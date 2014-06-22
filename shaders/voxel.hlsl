
struct ps_input_t
{
	float4 position : SV_Position;
	float4 texcoord : Texcoord;
};

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
StructuredBuffer<node_t> nodes;

// brick pool
texture3D bricks;
SamplerState brick_sampler
{
	
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Clamp;
	AddressV = Clamp;
};


static const uint brick_size = 8;
static const uint brick_count = 48;
static const float brick_sizef = 8.f;

uint oct_child(inout float4 box, float3 pos)
{
	uint child = 0;
	box.w *= 0.5f;

	if (box.x < pos.x) {
		child += 1;
		box.x += box.w;
	}
	else {
		box.x -= box.w;
	}

	if (box.y < pos.y) {
		child += 2;
		box.y += box.w;
	}
	else {
		box.y -= box.w;
	}

	if (box.z < pos.z) {
		child += 4;
		box.z += box.w;
	}
	else {
		box.z -= box.w;
	}

	return child;
}


uint find_brick(inout float4 box, float3 pos, float size)
{
	uint child = 0;
	uint index = 0;
	uint brick_size = 0;
	float volume = 1.f / brick_sizef;
	uint tmp = 0;

	while (volume > size)
	{
		volume *= 0.5f;
		child = oct_child(box, pos);
		uint chindex = nodes.Load(index).items[child].child;
		if (chindex == 0)
			break;

		index = chindex;
	}

	return nodes[index].items[child].brick;
}





// volume delta, because floating-point
static const float vdelta = 0.0000001f;

bool inside(float4 box, float3 position)
{
	return (position.x >= box.x - box.w - vdelta && position.x < box.x + box.w + vdelta)
		&& (position.y >= box.y - box.w - vdelta && position.y < box.y + box.w + vdelta)
		&& (position.z >= box.z - box.w - vdelta && position.z < box.z + box.w + vdelta)
		;
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
	
	
	float4 box = float4(.5f + -vdelta, .5f + -vdelta, .5f + -vdelta, .5f + vdelta);
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
	float4 box = float4(0, 0, 0, 1);
	float size = 0.00000001f;
	float distance = 0.f;

	float3 hit;
	if (inside(box, position))
		hit = position;
	else if (!enter(position, normal, hit))
		discard;

	float len = length(hit - position);
	distance += len;

	float4 colour = float4(0.0, 0.0, 0.0, 0.0);
	uint brick_id = find_brick(box, hit, size);
	float rem = 0.0;
	for (int i = 0; i < 50 && colour.w < 1.f; ++i)
	{
		float3 far = escape(box, hit, normal);
		len = length(far - hit);
		float3 step = normal * len;
		if (brick_id != 0)
		{
			float3 brick_near = (hit - box.xyz) / box.w;
			float3 brick_far = (far - box.xyz) / box.w;
			brick_ray(brick_id, brick_near, brick_far, colour, rem);
		}
		else rem = 0; // box.w / float(B_SIZE);

		float3 tmp = hit + step*1.010;
		if (!inside(float4(.5f, .5f, .5f, .5f), tmp))break;
		brick_id = find_brick(box, tmp, size);
		hit = tmp;
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
}

float4 main(ps_input_t input) : SV_Target
{
	return brick_path(input.position.xyz, normalize(input.texcoord).xyz, 0.00001f);
}
