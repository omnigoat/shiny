#include "svo_traversal.hlsli"

cbuffer buf_main : register(b0)
{
	uint fragment_count;

	// total number of levels in this octree
	uint levels;

	// level we're marking
	uint level;
}


// input-buffer. list of voxel-fragments, only morton number
StructuredBuffer<uint> fragments : register(t0);

// 




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

	uint fragment_morton = fragments.Load(idx);

	// the morton code looks like this, where 'f' is the fragment,
	// and each ascending letter represents a higher-level node in
	// the svo. we don't necessarily use all levels. the highest
	// two bits are always unused.
	//
	//  10987654321098765432109876543210
	//                         fffffffff
	// 0                   aaa
	// 1                 bbb
	// 2              ccc
	// 3           ddd
	// 4        eee
	// 5     fff
	// 6  ggg
	//  xx


	// truncate the fragment-level part of the morton-code
	uint brick_morton = fragment_morton >> 9;

	uint level_mortons[8] =
	{
		0, // root-node
		(brick_morton >> (3 * 0)) & 0x7,
		(brick_morton >> (3 * 1)) & 0x7,
		(brick_morton >> (3 * 2)) & 0x7,
		(brick_morton >> (3 * 3)) & 0x7,
		(brick_morton >> (3 * 4)) & 0x7,
		(brick_morton >> (3 * 5)) & 0x7,
		(brick_morton >> (3 * 6)) & 0x7,
	};

	uint offset = 0;
	uint child_idx = 0;
	for (uint i = levels - 1; i > levels - level - 1; --i)
	{
		node_t node = nodepool.Load(offset).nodes[child_idx];

		offset = node.children_offset;
		child_idx = level_mortons[i];
	}

	uint2 node_idx = svo_traversal_from_fragment(levels, level, fragment_morton);
	
	InterlockedOr(nodepool[node_idx.x].nodes[node_idx.y].children_offset, 0x80000000);
}
