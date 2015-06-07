
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
	float3 pixel_delta : PixelDelta;
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
StructuredBuffer<tile_t> nodes : register(t0);
texture3D<float2> bricks : register(t1);

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

//static const float inv_brick_countf = 1.f / brickcache_width;

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
		tile_t n = nodes[node_index];
		
		uint aabb_child = leaf_box.child_index(pos);
		result_brick = n.nodes[aabb_child].brick;
		leaf_box = child_aabb(leaf_box, aabb_child); 

		node_index = n.nodes[aabb_child].child;
		if (node_index == 0)
			break;
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
	return r * (float)brick_size / brickcache_width;
}


//
//  brick_ray
//  -----------
//    brick_enter/brick_exit: [0.f, 8.f) coordinates of enter/exit positions
void brick_ray(in uint brick_id, in float3 brick_enter, in float3 brick_exit, inout float4 colour, inout float remainder)
{
	float4 result = colour;

	float  inv_brick_size       = 1.f / brick_size;
	float  inv_brickcache_width = 1.f / brickcache_width;
	float  half_size            = inv_brick_size * 0.5;
	float3 half_size_xyz        = float3(half_size, half_size, half_size);
	float3 brick_multi          = inv_brick_size * (brick_size-1);
	float3 brick_position       = brick_origin(brick_id);

	// take coordinates into [0.5f, 7.5f) range, then squash into [0.0625f, 0.9375f),
	// so that the edges of our sample ranges (0.f, and 1.f), sample the middle of the
	// outermost voxels
	float3 fragment_enter  = brick_enter * brick_multi + half_size_xyz;
	float3 fragment_exit   = brick_exit  * brick_multi + half_size_xyz;
	float  fragment_length = length(fragment_exit - fragment_enter);

	// I'm not sure what this's for
	if (fragment_length < 0.1)
		return;


	// x/(y/z) === (xz)/y
	// x/(y/(z/w)) === x/(wy/z) === xz/wy

	// length of vector in fragment-space
	//float steps = 1.f / (len * brick_size);
	float steps = fragment_length * inv_brick_size;

	float pos;
	for (pos = steps*remainder; pos < 1.f && result.w < 1.f; pos += steps)
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

		float4 voxel = voxelfull.xxxx;
		result.xyz += ((1.0-result.w)*(1.0-result.w) * voxel.xyz)/(1.0 - result.xyz * voxel.xyz);
		result.w = result.w + (1.0-result.w) * voxel.w;
	}

	remainder = (pos - 1.f) / steps;
	colour = result;
}

float4 brick_path(float3 position, float3 normal, float ratio)
{
	aabb_t box = {0.f, 0.f, 0.f, 1.f + vdelta};

	float size = 0.005f; //* length(0.f - position);
	float3 hit_enter, hit_exit;

	if (box.contains(position))
		hit_enter = position;
	else if (!intersection(box, position, normal, hit_enter, hit_exit))
		discard;

	float len = length(hit_enter - position);
	uint reps = 0;
	float4 color = {0.f, 0.f, 0.f, 0.f};
	float rem = 0.f;

	// debug: hit-enter
	//return float4(hit_enter, 1.f);

	while (reps != 50 && color.w < 1.f && box.contains(hit_enter))
	{
		aabb_t leaf_box;
		float3 leaf_enter;
		float3 leaf_exit;

		uint brick_id = brick_index(box, hit_enter, size, leaf_box);
		if (!intersection(leaf_box, position, normal, leaf_enter, leaf_exit))
			break;
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
	color.w = 1.f;

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
	// debug: position
	//return position;

	// debug: pixel-delta
	//return float4(input.pixel_delta.xy, 0.f, 1.f);

	// debug: pitch/yaw
	//return float4(x, y, 0.f, 1.f);
	float yaw2 = yaw + input.pixel_delta.x;
	float pitch2 = pitch + input.pixel_delta.y * 1.33333f;
	float3 dir = {sin(yaw2) * cos(pitch2), sin(pitch2), cos(yaw2) * cos(pitch2)};
	
	// debug: direction
	//return float4(dir, 1.f);

	return brick_path(position.xyz, dir, 0.00001f);
}
