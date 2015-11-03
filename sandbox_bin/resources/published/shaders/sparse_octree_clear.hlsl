cbuffer buf_main : register(b0)
{
	uint nodes;
}

struct node_t
{
	uint child;
	uint brick;
};


RWStructuredBuffer<node_t> nodepool : register(u1);



[numthreads(64, 1, 1)]
void main( uint3 dtid : SV_DispatchThreadID )
{
	nodepool[dtid.x].child = 0;
	nodepool[dtid.x].brick = 0;
}
