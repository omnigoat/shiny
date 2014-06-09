
struct node_item_t
{
	uint child;
	uint brick;
};

struct node_t
{
	node_item_t items[8];
};

struct input_t
{
	float4 position : SV_Position;
	float4 texcoord : Texcoord;
};

cbuffer buf_octree : register(b3)
{
	StructuredBuffer<node_t> nodes;
}



// volume delta, because floating-point
static const float vdelta = 0.0000001f;

bool inside(float4 box, float3 position)
{
	return (position.x >= box.x - vdelta && position.x < box.x + box.w + vdelta)
		&& (position.y >= box.y - vdelta && position.y < box.y + box.w + vdelta)
		&& (position.z >= box.z - vdelta && position.z < box.z + box.w + vdelta)
		;
}

bool enter(float3 position, float3 normal, out float3 result)
{
	float3 near, far;
	result = float3(0, 0, 0);

	near.x = normal.x < 0.f ? 1.f : 0.f;
	near.y = normal.y < 0.f ? 1.f : 0.f;
	near.z = normal.z < 0.f ? 1.f : 0.f;
	far = float3(1, 1, 1) - near;

	float3 ratio_near = (near - position) / normal;
	return true;
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

float4 main(input_t input) : SV_Target
{
	return brick_path(input.position.xyz, normalize(input.texcoord).xyz, 0.00001f);
}
