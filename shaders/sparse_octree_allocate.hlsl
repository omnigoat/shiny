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


[numthreads(2, 2, 2)]
void main(uint3 gid : SV_GroupID, uint3 gtid : SV_GroupThreadID)
{
	uint offset = 0;
	
	// level 0: gid: [0, 0, 0], gtid: [2, 2, 2]
	// level 1: gid: [0-1, 0-1, 0-1], gtid: [2, 2, 2]
	// level 2: gid: [0-3, 0-3, 0-3]
	// level 3: gid: [0-7], gtid: [0-1]
	for (uint i = 0; i != level; ++i)
	{
		uint3 lgid      = gid / pow(2, level - i - 1) % 2;
		uint  child_idx = (lgid.x << 2) | (lgid.y << 1) | lgid.z;

		node_t node = nodepool.Load(offset * 8 + child_idx);
		if (node.children_offset == 0)
			return;

		offset = node.children_offset;
	}

	uint lgtid = (gtid.x << 2) | (gtid.y << 1) | gtid.z;

	node_t node = nodepool.Load(offset * 8 + lgtid);
	if (node.children_offset & 0x80000000)
	{
		uint prev;
		InterlockedExchange(nodepool[offset * 8 + lgtid].children_offset, nodepool.IncrementCounter() + 1, prev);
	}
}
