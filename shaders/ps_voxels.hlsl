struct ps_input_t
{
	float4 position : SV_Position;
	float3 texcoord : Texcoord;
};


//
// Pixel Shader
//
struct node_item_t
{
	uint child;
	uint brick;
};

struct node_t
{
	node_item_t items[8];
};

// node pool
StructuredBuffer<node_t> nodes : register(t0);
texture3D bricks : register(t1);

SamplerState brick_sampler
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Clamp;
	AddressV = Clamp;
};


// volume delta, because floating-point
static const float vdelta = 0.0000001f;

static const float3 axis_lookup[] = {
	float3(-1.f, -1.f, -1.f),
	float3( 1.f, -1.f, -1.f),
	float3(-1.f,  1.f, -1.f),
	float3( 1.f,  1.f, -1.f),
	float3(-1.f, -1.f,  1.f),
	float3( 1.f, -1.f,  1.f),
	float3(-1.f,  1.f,  1.f),
	float3( 1.f,  1.f,  1.f),
};


//
//  axis-aligned bounding-box
//  ---------------------------
//     yay shader-model 5!
//
class aabb_t
{
	float3 center() { return data.xyz; }
	float3 min() { return data.xyz + axis_lookup[0] * radius(); }
	float3 max() { return data.xyz + axis_lookup[7] * radius(); }

	float width() { return data.w; }
	float radius() { return data.w * .5f; }

	bool contains(in float3 pos)
	{
		return (pos.x >= data.x - radius() - vdelta && pos.x < data.x + radius() + vdelta)
			&& (pos.y >= data.y - radius() - vdelta && pos.y < data.y + radius() + vdelta)
			&& (pos.z >= data.z - radius() - vdelta && pos.z < data.z + radius() + vdelta)
			;
	}

	uint child_index(in float3 pos)
	{
		uint child = 0;
		child += (bool)(data.x < pos.x);
		child += (bool)(data.y < pos.y) * 2;
		child += (bool)(data.z < pos.z) * 4;
		return child;
	}

#if 0
	aabb_t child_aabb(int index)
	{
		
	}
#endif

	

	float4 data;
};


aabb_t child_aabb(in aabb_t box, float3 pos)
{
	//uint index = child_index(pos);
	uint index = 0;
	aabb_t r ={box.data.xyz + axis_lookup[index] * box.data.w * .25f, box.data.w * .5f};
	return r;
}

aabb_t child_aabb(in aabb_t box, uint index)
{
	aabb_t r ={box.data.xyz + axis_lookup[index] * box.data.w * .25f, box.data.w * .5f};
	return r;
}

static const aabb_t box = {0.f, 0.f, 0.f, 1.f};

static const uint brick_size = 8;
static const uint brick_count = 30;
static const float brick_sizef = 8.f;
static const float inv_brick_countf = 1.f / brick_count;

bool intersection(in aabb_t box, in float3 position, in float3 dir, out float3 enter, out float3 exit)
{
	float3 inv_dir = float3(1.f / dir.x, 1.f / dir.y, 1.f / dir.z);

	float tx1 = (box.min().x - position.x) * inv_dir.x;
	float tx2 = (box.center().x + box.radius() - position.x) * inv_dir.x;
	float tx_min = min(tx1, tx2);
	float tx_max = max(tx1, tx2);

	float ty1 = (box.min().y - position.y) * inv_dir.y;
	float ty2 = (box.center().y + box.radius() - position.y) * inv_dir.y;
	float ty_min = min(ty1, ty2);
	float ty_max = max(ty1, ty2);

	float tz1 = (box.min().z - position.z) * inv_dir.z;
	float tz2 = (box.center().z + box.radius() - position.z) * inv_dir.z;
	float tz_min = min(tz1, tz2);
	float tz_max = max(tz1, tz2);

	float tmin = max(tx_min, max(ty_min, tz_min));
	float tmax = min(tx_max, min(ty_max, tz_max));

	enter = position + dir * tmin;
	exit  = position + dir * tmax;

	return tmin < tmax && 0.f < tmax;
}

uint brick_index(in aabb_t box, float3 pos, float size, out aabb_t leaf_box)
{
	uint result_brick = 0;
	leaf_box = box;

	uint node_index = 0;
	for (float volume = 1.f / brick_sizef; volume > size; volume *= 0.5f)
	{
		node_t n = nodes[node_index];
		
		uint aabb_child = leaf_box.child_index(pos);
		node_index = n.items[aabb_child].child;
		if (node_index == 0)
			break;

		result_brick = n.items[aabb_child].brick;
		leaf_box = child_aabb(leaf_box, aabb_child);
	}

	return result_brick;
}

float3 brick_origin(uint brick_id)
{
	return float3(
		inv_brick_countf * (float)((brick_id % brick_count)),
		inv_brick_countf * (float)((brick_id / brick_count) % brick_count),
		inv_brick_countf * (float)((brick_id / (brick_count * brick_count)) % brick_count)
	);
}


#if 0
void brick_ray(uint brick_id, float3 near, float3 far, inout float4 color, inout float rem)
{
	float inv_size = 1.f / brick_size;
	float inv_count = 1.f / brick_count;
	float half_size = 0.5f * inv_size;
	near = near * (inv_size * (brick_size - 1)) + half_size;
	far = far * (inv_size * (brick_size - 1)) + half_size;
	float len = length(far - near);
	float3 brick_pos = brick_origin(brick_id);
	float4 result = color;

	if (len < 0.1f)
		return;

	float steps = 1.f / (len * brick_size);

	float pos;
	for (pos = steps * rem; pos < 1.f; pos += steps)
	{
		float3 sample_loc = lerp(near, far, pos) * inv_count;
		float4 voxel = bricks.SampleLevel(brick_sampler, sample_loc + brick_pos, 0);
		result.xyz += ((1.f - result.a) * (1.f - result.a) * voxel.xyz) / (1.f - result.xyz * voxel.xyz);
		result.w = result.w + (1.f - result.w) * voxel.w;
		if (result.w > 1.f)
			break;
	}

	rem = (pos - 1.f) * (len * brick_size);
	color = result;
}
#endif

void brick_accumulate(inout float4 color, uint brick_id, float3 near, float3 far)
{
	float inv_size = 1.f / brick_size;
	float inv_count = 1.f / brick_count;
	float half_size = inv_size * 0.5f;

	// translate begin and end positions to the centres of the voxels
	near = near * (inv_size * (brick_size - 1)) + half_size;
	far = far * (inv_size * (brick_size - 1)) + half_size;

	float len = length(far - near);
	float3 brick_pos = brick_origin(brick_id);
	float rem = 0.f;
	
	float4 result = color;

	if (len < 0.1f)
		return;

	float steps = 1.f / (len * brick_size);

	float pos;
	for (pos = steps * rem; pos < 1.f; pos += steps)
	{
		float3 sample_loc = lerp(near, far, pos) * inv_count;
		float4 voxel = bricks.SampleLevel(brick_sampler, sample_loc + brick_pos, 0);
		result.xyz += ((1.f - result.a) * (1.f - result.a) * voxel.xyz) / (1.f - result.xyz * voxel.xyz);
		result.w = result.w + (1.f - result.w) * voxel.w;
		if (result.w > 1.f)
			break;
	}
	
	// don't bother with rem
	//rem = (pos - 1.f) * (len * brick_size);
	color = result;
}

float4 brick_path(float3 position, float3 normal, float ratio)
{
	aabb_t box = {0, 0, 0, 1.f};

	float size = 0.0000001f;
	float3 hit_enter, hit_exit;


	aabb_t leaf_box;
	if (box.contains(position))
		hit_enter = position;
	else if (!intersection(box, position, normal, hit_enter, hit_exit))
		discard;

	float len = length(hit_enter - position);
	uint reps = 0;
	float4 color = {0.f, 0.f, 0.f, 0.f};
	float rem = 0.f;
	
	do
	{
		aabb_t leaf_box;
		float3 leaf_enter;
		float3 leaf_exit;

		uint brick_id = brick_index(box, hit_enter, size, leaf_box);
		intersection(leaf_box, position, normal, leaf_enter, leaf_exit);

		if (brick_id != 0)
		{
			// our 3d-texture is addressed in [0,0,0] -> [1,1,1]
			float3 brick_enter = (leaf_enter - box.min()) / box.width();
			float3 brick_exit = (leaf_exit - box.min()) / box.width();
			brick_accumulate(color, brick_id, brick_enter, brick_exit);
		}

		/*
		float3 brick_enter = (leaf_enter - box.min()) / box.width();
		float4 voxel = bricks.SampleLevel(brick_sampler, brick_enter, 0);
		color += voxel;
		*/

		hit_enter = leaf_exit + normal * 0.01f;
		if (!box.contains(hit_enter))
			break;

		++reps;
	} while (reps < 50 && color.w < 1.f);


	return color;

#if 0
	float rem = 0.0;
	for (int i = 0; i < 50 && colour.w < 1.f; ++i)
	{
		float3 far = escape(box, hit_enter, normal);
		len = length(far - hit_enter);
		float3 step = normal * len;
		if (brick_id != 0)
		{
			float3 brick_near = (hit_enter - box.xyz) / box.w;
			float3 brick_far = (far - box.xyz) / box.w;
			brick_ray(brick_id, brick_near, brick_far, colour, rem);
		}
		else rem = 0;

		float3 tmp = hit_enter + step*1.010;
		if (!inside(float4(.5f, .5f, .5f, .5f), tmp))break;
		brick_id = brick_index(box, tmp, size);
		hit_enter = tmp;
	}
	float3 n = normalize(colour.xyz);

	float3 lamb = float3(.4f, .4f, .4f);
	float3 lpwr = float3(.3f, .3f, .3f);
	float3 ldir = float3(0, -1, 0);
	// diffuse lighting
	float i2  = dot(ldir, n);
	float4 color = float4(lamb+ i * lpwr, colour.w);
	// specular lighting
	float3 h = normalize(ldir + normal);
	i = clamp(pow(dot(n, h), 15), 0, 1);
	color += float4(i * lpwr, 0);
	
	return color;
#endif
}



float4 main(ps_input_t input) : SV_Target
{
	return brick_path(float3(0.f, 0.f, -1.4f), normalize(input.texcoord), 0.00001f);
}
