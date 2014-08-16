cbuffer buf_scene : register(b0)
{
	matrix view;
	matrix proj;
	matrix inverse_vp;
	float time;
};

cbuffer buf_voxel : register(b2)
{
	float4 position;
	float x, y;
}

struct ps_input_t
{
	float4 blah : SV_Position;
	float3 pixel_delta : PixelDelta;
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
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Clamp;
	AddressV = Clamp;
	AddressW = Clamp;
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
static const float brick_sizef = 8.f;
static const uint brick_count = 30;
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
	for (float volume = 1.f; volume > size; volume *= 0.5f)
	{
		node_t n = nodes[node_index];
		
		uint aabb_child = leaf_box.child_index(pos);
		result_brick = n.items[aabb_child].brick;
		leaf_box = child_aabb(leaf_box, aabb_child); 

		node_index = n.items[aabb_child].child;
		if (node_index == 0)
			break;
	}

	return result_brick;
}

float3 brick_origin(uint brick_id)
{
	return float3(
		inv_brick_countf * (float)((brick_id) % brick_count),
		inv_brick_countf * (float)((brick_id / brick_count) % brick_count),
		inv_brick_countf * (float)((brick_id / (brick_count * brick_count)) % brick_count)
	);
}

void brick_ray(in uint brick_id, in float3 near, in float3 far, inout float4 colour, inout float remainder)
{
	// float3 delta = far - near;
	float iSize = 1.0 / float(brick_size);
	float iCount = 1.0 / float(brick_count);
	float hs = iSize * 0.5;
	near = near * (iSize * (brick_size-1)) + float3(hs, hs, hs);
	far = far * (iSize * (brick_size-1)) + float3(hs, hs, hs);
	float len = length(far - near);
	float3 brick_pos = brick_origin(brick_id);
	float4 result = colour;

	if (len < 0.1)return;

	float steps = 1.0/(len / iSize);

	float pos;
	for (pos=steps*remainder; pos < 1.0; pos += steps)
	{
		float3 sample_loc = lerp(near, far, pos)*iCount;
		float4 voxel = bricks.SampleLevel(brick_sampler, sample_loc + brick_pos, 0);
		result.xyz += ((1.0-result.w)*(1.0-result.w) * voxel.xyz)/(1.0 - result.xyz * voxel.xyz);
		result.w = result.w + (1.0-result.w) * voxel.w;
		// result.xyz = result.xyz + (1.0-result.w) * voxel.xyz;
		// result.w = result.w + (1.0-result.w) * voxel.w;
		if (result.w > 1.0)break;
	}
	remainder = (pos - 1.0) / steps;
	colour = result;
	// TODO: interpolate between different resolutions
}

float4 brick_path(float3 position, float3 normal, float ratio)
{
	aabb_t box ={-vdelta, -vdelta, -vdelta, 1.f + vdelta};

	float size = 0.005f; //* length(0.f - position);
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
	
	while (reps != 50 && color.w < 1.f && box.contains(hit_enter))
	{
		aabb_t leaf_box;
		float3 leaf_enter;
		float3 leaf_exit;

		uint brick_id = brick_index(box, hit_enter, size, leaf_box);
		intersection(leaf_box, position, normal, leaf_enter, leaf_exit);
		float len = length(leaf_exit - hit_enter);

		if (brick_id != 0)
		{
			float3 brick_enter = (leaf_enter - leaf_box.min()) / leaf_box.width();
			float3 brick_exit = (leaf_exit - leaf_box.min()) / leaf_box.width();
			brick_ray(brick_id, brick_enter, brick_exit, color, rem);
		}
		else
		{
			rem = 0.f;
		}

		hit_enter = hit_enter + len * normal * 1.01f;
		++reps;
	} 

#if 1
	float3 n = normalize(color.xyz);

	float3 lamb = (0.4);
	float3 lpwr = (0.3);
	float3 ldir = float3(0, -1, 0);
	// diffuse lighting
	float i  = dot(ldir, n);
	color = float4(lamb+ i * lpwr, color.w);
	// specular lighting
	float3 h = normalize(ldir + normal);
	i = clamp(pow(dot(n, h), 15), 0, 1);
	color += float4(i * lpwr, 0);
#endif

	return color;
}

static const float pi = 3.14159265f;

float4 main(ps_input_t input) : SV_Target
{
	//float4 pj_position = mul(inverse_vp, float4(0.f, 0.f, 0.f, 1.f));
	//float3 position = pj_position.xyz / pj_position.w;

	float3 dir ={sin(x) * cos(y), sin(y), cos(x) * cos(y)};
	float guessup = float3(0.f, 1.f, 0.f);
	float3 right = cross(dir, float3(0.f, 1.f, 0.f));
	float3 up = cross(right, dir);

	float yd = acos(position.y);
	
	float3 p = dir + up * input.pixel_delta.y + right * input.pixel_delta.x;


	return brick_path(position.xyz, p, 0.00001f);
}
