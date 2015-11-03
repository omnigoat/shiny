cbuffer buf_main : register(b0)
{
	uint fragment_count;
	uint levels;
	uint level;
}

struct node_t
{
	// offset to an 8-wide array of children nodes. offsets are
	// stored for 8-blocks of nodes, and the root node has 7
	// empty trailing children after it, so that a node-group is
	// always located at offset*8
	uint children_offset;

	// offset to 
	uint brick_id;
};


struct tile_t
{
	node_t nodes[8];
};


// atomic counters
static const uint tile_counter = 0;
static const uint brick_counter = 1;

RWStructuredBuffer<uint> countbuf : register(u0);
RWStructuredBuffer<tile_t> nodepool : register(u1);


[numthreads(2, 2, 2)]
void main(uint3 gid : SV_GroupID, uint3 gtid : SV_GroupThreadID)
{
	uint offset = 0;
	
	// we need to traverse from level 0 to level N, where N is whatever
	// we've been dispatched to deal with. this is important because it means
	// that our group-id is a larger power-of-two [pow(2, level) groups were
	// dispatched in each dimension].
	for (uint i = 0; i != level; ++i)
	{
		uint3 lgid      = gid / pow(2, level - i - 1) % 2;
		uint  child_idx = (lgid.z << 2) | (lgid.y << 1) | lgid.x;

		node_t node = nodepool.Load(offset).nodes[child_idx];
		if (node.children_offset == 0)
			return;

		offset = node.children_offset;
	}

	// offset now points to the tile that houses the 8 children of our level.
	// for level 0, this would be tile 0. for level 1, this would be one of the
	// 8 tiles pointed at by the 8 children of the root node.
	//
	// use the group-thread-id to index into one of these 8 children.
	uint lgtid = (gtid.z << 2) | (gtid.y << 1) | gtid.x;

	node_t node = nodepool.Load(offset).nodes[lgtid];
	if (node.children_offset & 0x80000000)
	{
		// if we're allocating tiles
		if (level < levels)
		{
			// allocate new tile (note that the counter should be initialized to 1)
			uint tile;
			InterlockedAdd(countbuf[tile_counter], 1, tile);

			nodepool[offset].nodes[lgtid].children_offset = tile;
		}
		// if we're allocating bricks
		else
		{
			uint brick;
			InterlockedAdd(countbuf[brick_counter], 1, brick);

			nodepool[offset].nodes[lgtid].children_offset = 0;
			nodepool[offset].nodes[lgtid].brick_id = brick;
		}
	}
}
