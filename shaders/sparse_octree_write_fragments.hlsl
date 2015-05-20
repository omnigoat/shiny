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

struct voxel_t
{
	uint color;
	uint normal;
};

// input-buffer. list of voxel-fragments, only morton number
StructuredBuffer<uint> fragments : register(t0);

// atomic counters
static const uint tile_counter = 0;
static const uint brick_counter = 1;
RWStructuredBuffer<uint> countbuf : register(u0);

RWStructuredBuffer<node_t> nodepool : register(u1);
RWTexture3D<uint2> brickpool : register(u2);



void morton_decoding32(uint morton, out uint x, out uint y, out uint z)
{
	x = morton;
	y = morton >> 1;
	z = morton >> 2;
	x &= 0x09249249;
	y &= 0x09249249;
	z &= 0x09249249;
	x |= (x >> 2);
	y |= (y >> 2);
	z |= (z >> 2);
	x &= 0x030c30c3;
	y &= 0x030c30c3;
	z &= 0x030c30c3;
	x |= (x >> 4);
	y |= (y >> 4);
	z |= (z >> 4);
	x &= 0x0300f00f;
	y &= 0x0300f00f;
	z &= 0x0300f00f;
	x |= (x >> 8);
	y |= (y >> 8);
	z |= (z >> 8);
	x &= 0x030000ff;
	y &= 0x030000ff;
	z &= 0x030000ff;
	x |= (x >> 16);
	y |= (y >> 16);
	z |= (z >> 16);
	x &= 0x000003ff;
	y &= 0x000003ff;
	z &= 0x000003ff;
}


[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	// fragment-index, abandon if we're too large
	uint idx = DTid.x;
	if (idx >= fragment_count)
		return;

	uint voxel = fragments.Load(idx);

	uint offset = 0;
	uint child_idx = 0;
	for (uint i = 0; i != level; ++i)
	{
		node_t node = nodepool.Load(offset * 8 + child_idx);

		// get morton-code for this level, early out if zero?
		uint morton = voxel / pow(2, levels - i);
		if (morton == 0)
			return;

		offset = node.children_offset;
		child_idx = morton & 0x7;
	}

	node_t node = nodepool.Load(offset * 8 + child_idx);

	// brick position is in morton coordinates
	uint brick_morton = node.brick_id;
	uint3 brick_coords;
	morton_decoding32(brick_morton, brick_coords.x, brick_coords.y, brick_coords.z);
	brick_coords *= 8;

	// voxel coords within brick
	uint voxel_morton = voxel & 0x7;
	uint3 voxel_coords;
	morton_decoding32(voxel_morton, voxel_coords.x, voxel_coords.y, voxel_coords.z);

	// BLHABLHABLHK
	uint3 coords = brick_coords + voxel_coords;

	brickpool[coords] = uint2(0xffffffff, 0);
}
