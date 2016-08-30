
// put this in a header some day
float4 u32x1_to_f(uint x)
{
	return float4(
		(x & 0xff) / 255.f,
		((x >> 8) & 0xff) / 255.f,
		((x >> 16) & 0xff) / 255.f,
		((x >> 24) & 0xff) / 255.f
	);
}

cbuffer buf_scene : register(b0)
{
	matrix view;
	matrix inverse_view;
	matrix proj;
	matrix inverse_vp;
	float time;
};

cbuffer buf_voxel : register(b2)
{
	float4 position;
	float yaw, pitch;
	uint brickcache_width;
	uint brick_size;
}

struct ps_input_t
{
	float4 blah : SV_Position;
	float3 view_dir : ViewDirection;
};


//
// Pixel Shader
//
struct node_t
{
	uint child;
	uint brick;
};

struct tile_t
{
	node_t nodes[8];
};

// node pool
StructuredBuffer<tile_t> nodepool : register(t0);
texture3D<float2> bricks : register(t1);

SamplerState brick_sampler
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Clamp;
	AddressV = Clamp;
	AddressW = Clamp;
};


// volume delta, because floating-point
static const float vdelta = 0.0001f;

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
	float radius() { return data.w * .5f + vdelta; }

	bool contains(in float3 pos)
	{
		return (pos.x >= data.x - radius() - vdelta && pos.x <= data.x + radius() + vdelta)
			&& (pos.y >= data.y - radius() - vdelta && pos.y <= data.y + radius() + vdelta)
			&& (pos.z >= data.z - radius() - vdelta && pos.z <= data.z + radius() + vdelta)
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


aabb_t child_aabb(in aabb_t box, uint index)
{
	aabb_t r = {box.data.xyz + axis_lookup[index] * box.data.w * .25f, box.data.w * .5f};
	return r;
}

bool intersection(in aabb_t box, in float3 position, in float3 dir, out float3 enter, out float3 exit)
{
	float3 inv_dir = 1.f / dir;

	float3 near = (box.min() - position) * inv_dir;
	float3 far  = (box.max() - position) * inv_dir;
	float3 tmin = min(near, far);
	float3 tmax = max(near, far);
	float rmin = max(tmin.x, max(tmin.y, tmin.z));
	float rmax = min(tmax.x, min(tmax.y, tmax.z));

	enter = position + dir * max(rmin, 0.f);
	exit  = position + dir * max(rmax, 0.f);

	return rmin <= rmax && 0.f < rmax;
}

uint brick_index(in aabb_t box, float3 pos, float size, out aabb_t leaf_box)
{
	leaf_box = box;

	uint result_brick = 0;
	uint tile = 0;
	uint child_offset = 0;

	for (float volume = 1.f; volume > size; volume *= 0.5f)
	{
		node_t n = nodepool[tile].nodes[child_offset];

		tile         = n.child;
		child_offset = leaf_box.child_index(pos);

		result_brick = n.brick;

		if (tile == 0)
			break;

		leaf_box     = child_aabb(leaf_box, child_offset);
	}

	return result_brick;
}


void morton_decoding32(uint morton, out uint x, out uint y, out uint z)
{
	x = morton;
	y = morton >> 1;
	z = morton >> 2;
	x &= 0x09249249;
	y &= 0x09249249;
	z &= 0x09249249;
	x |= (x >> 2);
	y |= (y >> 2);
	z |= (z >> 2);
	x &= 0x030c30c3;
	y &= 0x030c30c3;
	z &= 0x030c30c3;
	x |= (x >> 4);
	y |= (y >> 4);
	z |= (z >> 4);
	x &= 0x0300f00f;
	y &= 0x0300f00f;
	z &= 0x0300f00f;
	x |= (x >> 8);
	y |= (y >> 8);
	z |= (z >> 8);
	x &= 0x030000ff;
	y &= 0x030000ff;
	z &= 0x030000ff;
	x |= (x >> 16);
	y |= (y >> 16);
	z |= (z >> 16);
	x &= 0x000003ff;
	y &= 0x000003ff;
	z &= 0x000003ff;
}



float3 brick_origin(uint brick_id)
{
	float3 r;
	morton_decoding32(brick_id, r.x, r.y, r.z);
	return r / brickcache_width;
}


//
//  brick_ray
//  -----------
//    brick_enter/brick_exit: [0.f, 1.f) coordinates of enter/exit positions.
//    there are in relation to the brick itself
//
//    remainder: the percentage of a step we "overstepped"
//
void brick_ray(in uint brick_id, in float3 brick_enter, in float3 brick_exit, inout float4 colour)
{
	float4 result = colour;

	// debug: brick_enter
	//colour = float4(brick_enter, 1.f);
	//return;

	float  inv_brick_size       = 1.f / brick_size;
	float  inv_brickcache_width = 1.f / brickcache_width;
	float  half_size            = inv_brick_size * .5f;
	float3 half_size_xyz        = float3(half_size, half_size, half_size);
	float3 brick_multi          = (brick_size - 1.f) / brick_size;
	float3 brick_position       = brick_origin(brick_id);

	// take coordinates into [0.0625f, 0.9375f) range, so that the edges of our
	// sample ranges (0.f, and 1.f), sample the middle of the outermost voxels
	float3 fragment_enter  = brick_enter * brick_multi + half_size_xyz;
	float3 fragment_exit   = brick_exit  * brick_multi + half_size_xyz;
	float  fragment_length = length(fragment_exit - fragment_enter);

	// debug: fragment_enter
	//colour = float4(fragment_enter, 1.f);
	//return;

	// STEPPING
	// ----------
	//   : 1/(3 * brick-size) is a nice step for isotropic voxels.
	//   : we take less steps when we intersect less of the brick
	//   : everything is over 1.f, because we need to lerp between our fragment enter & exit
	//
	float stepping_percentage = fragment_length / (1.4142f * brick_multi.x);

	float step = 1.f / (3 * brick_size * stepping_percentage);
	
	
	float pos = 0.f;
	for ( ; pos < 1.f; pos += step)
	{
		float3 fragment_position = lerp(fragment_enter, fragment_exit, pos) * inv_brickcache_width;
		float2 voxelfull = bricks.SampleLevel(brick_sampler, brick_position + fragment_position, 0);

		uint c = asuint(voxelfull.x);

		if (c != 0)
		{
			result = float4(
				((c >> 0)  & 0xff) / 255.f,
				((c >> 8)  & 0xff) / 255.f,
				((c >> 16) & 0xff) / 255.f,
				1.f);

			break;
		}
	}

	colour = result;
}

float4 brick_path(float3 position, float3 normal, float ratio)
{
	aabb_t box = {0.f, 0.f, 0.f, 1.f};

	float size = 0.005f; //* length(0.f - position);
	float3 hit_enter, hit_exit;

	if (!intersection(box, position, normal, hit_enter, hit_exit))
		discard;

	// debug: hit-enter
	//return float4(hit_enter, 1.f);

	// debug: hit-exit
	//return float4(hit_exit, 1.f);

	float4 color = {0.f, 0.f, 0.f, 0.f};

	uint reps = 0;
	for ( ; reps != 64 && color.w < 1.f && box.contains(hit_enter); ++reps)
	{
		aabb_t leaf_box;
		float3 leaf_enter;
		float3 leaf_exit;

		uint brick_id = brick_index(box, hit_enter, size, leaf_box);
		if (!intersection(leaf_box, hit_enter, normal, leaf_enter, leaf_exit))
			break;

		float len = length(leaf_exit - hit_enter);

		if (brick_id != 0)
		{
			float3 brick_enter = (leaf_enter - leaf_box.min()) / leaf_box.width();
			float3 brick_exit  = (leaf_exit  - leaf_box.min()) / leaf_box.width();
			brick_ray(brick_id, brick_enter, brick_exit, color);
		}

		hit_enter = hit_enter + len * normal * 1.001f;
	}

	// debug: number of nodes traversed
	//return float4(reps / 16.f, 0.f, 0.f, 1.f);

#if 0
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
	return brick_path(position.xyz, normalize(input.view_dir), 0.00001f);
}
