struct FSInput
{
	float4 position : SV_Position;
	nointerpolation float3 normal : Normal;
	nointerpolation float4 aabb : AABB;
};

cbuffer something
{
	
};


float4 main(FSInput input) : SV_Target
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
