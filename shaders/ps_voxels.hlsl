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
	float3(-1.f, -1.f,  1.f),
	float3(-1.f,  1.f, -1.f),
	float3(-1.f,  1.f,  1.f),
	float3( 1.f, -1.f, -1.f),
	float3( 1.f, -1.f,  1.f),
	float3( 1.f,  1.f, -1.f),
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
static const uint brick_count = 48;
static const float brick_sizef = 8.f;


uint find_brick(in aabb_t box, float3 pos, float size, out aabb_t leaf_box)
{
	uint node_index = 0;
	uint aabb_child = 0;
	uint result_brick = 0;

	for (float volume = 1.f / brick_sizef; volume > size; volume *= 0.5f)
	{
		aabb_child = box.child_index(pos);
		uint child_index = nodes[node_index].items[aabb_child].child;
		if (child_index == 0)
			break;

		result_brick = nodes[node_index].items[aabb_child].brick;
		node_index = child_index;
		leaf_box = child_aabb(box, aabb_child);
	}

	return result_brick;
}







bool inside(float4 box, float3 position)
{
	return (position.x >= box.x - box.w - vdelta && position.x < box.x + box.w + vdelta)
		&& (position.y >= box.y - box.w - vdelta && position.y < box.y + box.w + vdelta)
		&& (position.z >= box.z - box.w - vdelta && position.z < box.z + box.w + vdelta)
		;
}

/*
bool
intersection(box b, ray r)
{
	double tx1 = (b.min.x - r.x0.x)*r.n_inv.x;
	double tx2 = (b.max.x - r.x0.x)*r.n_inv.x;

	double tmin = min(tx1, tx2);
	double tmax = max(tx1, tx2);

	double ty1 = (b.min.y - r.x0.y)*r.n_inv.y;
	double ty2 = (b.max.y - r.x0.y)*r.n_inv.y;

	tmin = max(tmin, min(ty1, ty2));
	tmax = min(tmax, max(ty1, ty2));

	return tmax >= tmin;
}
*/
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



bool enter(in float3 position, in float3 normal, out float3 result)
{
	result = float3(0.f, 0.f, 0.f);

	float3 near = {
		normal.x < 0.f ? 1.f : 0.f,
		normal.y < 0.f ? 1.f : 0.f,
		normal.z < 0.f ? 1.f : 0.f
	};

	float3 ratio_near = (near - position) / normal;
	float near_min = min(min(ratio_near.x, ratio_near.y), ratio_near.z);
	float near_max = max(max(ratio_near.x, ratio_near.y), ratio_near.z);
	float near_mid = 
		ratio_near.x == near_min
			? (ratio_near.y == near_min ? ratio_near.z : ratio_near.y)
			: (ratio_near.z == near_min ? ratio_near.x : ratio_near.z)
			;
	
	
	float4 box = float4(0, 0, 0, 1.f + vdelta);
	if (inside(box, near_min * normal + position)) {
		result = near_min * normal + position;
		return true;
	}
	else if (inside(box, near_mid * normal + position))
	{
		result = normal * near_mid + position;
		return true;
	}
	else if (inside(box, near_max * normal + position))
	{
		result = normal * near_max + position;
		return true;
	}
	else {	
		return false;
	}
}

float3 escape(float4 box, float3 position, float3 normal)
{
	float3 far = {
		normal.x < 0 ? box.x - box.w : box.x + box.w,
		normal.y < 0 ? box.y - box.w : box.y + box.w,
		normal.z < 0 ? box.z - box.w : box.z + box.w
	};

	if (normal.x == 0) normal.x += vdelta * 100.f;
	if (normal.y == 0) normal.y += vdelta * 100.f;
	if (normal.z == 0) normal.z += vdelta * 100.f;

	float3 ratio = (far - position) / normal;
	float rmin = min(min(ratio.x, ratio.y), ratio.z);
	return rmin * normal + position;
}

float3 brick_origin(uint brick_id)
{
	float3 brick_pos;
	uint brick_tmp;

	brick_pos.x = float(brick_id % brick_count);
	brick_tmp = (brick_id - uint(brick_pos.x)) / brick_count;
	brick_pos.y = float(brick_tmp % brick_count);
	brick_pos.z = float((brick_tmp - uint(brick_pos.y))/brick_count);

	return brick_pos / float(brick_count);
}

uint find_child(inout aabb_t box, float3 pos, float size)
{
	uint child = 0;
	uint index = 0;
	uint brick_size = 0;
	float volume = 1.f / brick_sizef;
	uint tmp = 0;
	uint count = 0;
	while (count < 5 && volume > size)
	{
		volume *= 0.5f;
		child = box.child_index(pos);
		uint chindex = nodes[index].items[child].child;
		if (chindex == 0)
			break;

		box = child_aabb(box, pos);
		index = chindex;
		count++;
	}

	return count;
}


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

float4 brick_path(float3 position, float3 normal, float ratio)
{
	aabb_t box = {0, 0, 0, 1.f};

	float size = 0.00000001f;
	float distance = 0.f;
	float3 hit_enter, hit_exit;


	aabb_t leaf_box;
	if (box.contains(position)) //false) //inside(box, position))
	{
		hit_enter = position;
	}
	else if (!intersection(box, position, normal, hit_enter, hit_exit))
	{
		discard;
	}

	//uint ch = find_child(jbox, hit_enter, 0.00001);
	//return float4((float3)ch, 1.f);

	float len = length(hit_enter - position);
	distance += len;

	float4 colour = float4(0.0, 0.0, 0.0, 0.0);
	
	

	uint reps = 0;
	float4 color = {0.f, 0.f, 0.f, 0.f};
	float rem = 0.f;
	do
	{
		aabb_t leaf_box;
		float3 leaf_enter;
		float3 leaf_exit;

		uint brick_id = find_brick(box, hit_enter, size, leaf_box);
		intersection(leaf_box, position, normal, leaf_enter, leaf_exit);

		if (brick_id != 0)
		{
			// our 3d-texture is addressed in [0,0,0] -> [1,1,1]
			float3 brick_enter = (hit_enter - box.min()) / box.width();
			float3 brick_exit = (hit_exit - box.min()) / box.width();
			brick_accumulate(brick_id, brick_enter, brick_exit, color, rem);
		}

		hit_enter = leaf_exit + normal * 0.0000001f;
		if (!box.contains(hit_enter))
			break;

		++reps;
	} while (reps < 50 && colour.w < 1.f);

#if 0
	for (int i = 0; ; ++i)
	{
		float brick_internal_length = length(hit_exit - hit_enter);
		float3 step = normal * brick_internal_length;

		if (brick_id != 0)
		{
			// our 3d-texture is addressed in [0,0,0] -> [1,1,1]
			float3 brick_near = (hit_enter - box.min()) / box.width();
			float3 brick_far = (hit_exit - box.min()) / box.width();
			brick_accumulate(brick_id, brick_near, brick_far, color, rem);
		}

		float3 tmp = hit_enter + step;
		if (box.contains(tmp))
			break;

		brick_id = find_brick(box, tmp, size);
		hit_enter = tmp;
	}
#endif

	return float4(0.f, 0.f, 0.f, 1.f); //bricks.Load(brick_id);

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
		brick_id = find_brick(box, tmp, size);
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



float4 ps_main(ps_input_t input) : SV_Target
{
#if 0
	float4 box = {0, 0, 0, 1.f};
	float3 position = {0, 0, 0};

	float4 colors[5] = {
		float4(1, 0, 0, 1),
		float4(0, 1, 0, 1),
		float4(0, 0, 1, 1),
		float4(1, 0, 1, 1),
		float4(1, 1, 0, 1)
	};

	uint depth = find_child(box, position, 0.00000001f);

	return colors[depth];
#endif
	
	//float2 n = normalize(input.texcoord);
	
	return brick_path(float3(0.f, 0.f, -2.f), normalize(input.texcoord), 0.00001f);
}
