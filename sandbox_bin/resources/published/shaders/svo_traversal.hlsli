
struct node_t
{
	// offset to an 8-wide array of children nodes. offsets are
	// stored for 8-blocks of nodes, and the root node has 7
	// empty trailing children after it, so that a node-group is
	// always located at offset*8
	uint children_offset;

	uint brick_id;
};

struct tile_t
{
	node_t nodes[8];
};

RWStructuredBuffer<tile_t> nodepool : register(u1);

// the full morton code looks like this, where 'f' is the fragment,
// and each ascending letter represents a higher-level node in the 
// svo. we don't necessarily use all levels. the highest two bits 
// are always unused.
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
//
uint2 svo_traversal(uint levels, uint level, uint brick_morton)
{
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
	for (uint i = levels; i > levels - level; --i)
	{
		node_t node = nodepool[offset].nodes[child_idx];
		offset = node.children_offset;
		child_idx = level_mortons[i];
	}

	return uint2(offset, child_idx);
}

uint2 svo_traversal_from_fragment(uint levels, uint level, uint fragment_morton)
{
	// truncate the fragment-level part of the morton-code
	uint brick_morton = fragment_morton >> 9;

	return svo_traversal(levels, level, brick_morton);
}
