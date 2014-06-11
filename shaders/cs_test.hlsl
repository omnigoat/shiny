Texture2D<float4> Input : register(t0);
RWTexture2D<float4> Output : register(u0);
StructuredBuffer<float4> Color : register(u1);

[numthreads(32, 32, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
	Output[threadID.xy] = float4(threadID.x / 128.f, threadID.y / 128.f, 0.f, 1.f);
}