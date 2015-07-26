#include "morton.hlsli"

struct FSInput
{
	float4 position : SV_Position;
	//nointerpolation float3 normal : Normal;
	//nointerpolation float4 aabb : AABB;
};

RWStructuredBuffer<uint> countbuf : register(u1);
RWStructuredBuffer<uint> fragments : register(u2);

float4 main(FSInput input) : SV_Target
{
	uint idx;
	InterlockedAdd(countbuf[0], 1, idx);

	morton_encoding32(fragments[idx], input.position.x, input.position.y, input.position.z);
	fragments[idx] = idx;

	//discard;
	return float4(1.f, 0.f, 0.f, 0.f);
}
