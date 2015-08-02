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


[numthreads(8, 8, 8)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint idx = DTid.x * 64 + DTid.y * 8 + DTid.z;
	if (idx >= fragment_count)
		return;

	uint fragment_morton = fragments.Load(idx);

	uint2 node_idx = svo_traversal_from_fragment(levels, level, fragment_morton);
	
	InterlockedOr(nodepool[node_idx.x].nodes[node_idx.y].children_offset, 0x80000000);
}
