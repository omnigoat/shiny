
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

StructuredBuffer<node_t> nodes;


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
	float volume = 1.f / brick_size;
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
	float3 f = {0.f, 0.f, 90.f};
	return f;
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

	return float4(0, 0, 0, 1);
}

float4 main(ps_input_t input) : SV_Target
{
	return brick_path(input.position.xyz, normalize(input.texcoord).xyz, 0.00001f);
}
