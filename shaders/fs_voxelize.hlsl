struct FSInput
{
	float4 position : SV_Position;
	nointerpolation float3 normal : Normal;
	nointerpolation float4 aabb : AABB;
};


float4 main(FSInput input) : SV_Target
{
	return input.position;
}
