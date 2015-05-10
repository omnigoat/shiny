cbuffer buf_main : register(b0)
{
	uint fragment_count;
	uint level;
	uint levels;
}

struct node_item_t
{
	uint child;
	uint brick;
};

struct node_t
{
	// offset to an 8-wide array of children nodes. offsets are
	// stored for 8-blocks of nodes, and the root node has 7
	// empty trailing children after it, so that a node-group is
	// always located at offset*8
	uint children_offset;

	uint brick_id;
};


RWStructuredBuffer<node_t> nodepool : register(u0);
//RWStructuredBuffer<uint> counter : register(u0);


[numthreads(2, 2, 2)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	
	uint offset = 0;
	uint child_idx = 0;
	node_t node = nodepool.Load(offset * 8 + child_idx);

	// navigate to node
	for (uint i = 0; i != level; ++i)
	{
		if (node.children_offset == 0)
			return;

		offset = node.children_offset;
		uint3 ldt = DTid / pow(2, level - i);
		child_idx = (ldt.x << 2) | (ldt.y << 2) | ldt.z;
		node = nodepool.Load(offset * 8 + child_idx);
	}

	if (node.children_offset & 0x80000000)
	{
		node.children_offset = nodepool.IncrementCounter() + 1;
	}
}
