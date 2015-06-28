#include "svo_traversal.hlsli"

cbuffer buf_main : register(b0)
{
	uint fragment_count;
	uint levels;
	uint level;
}


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
RWTexture3D<float2> brickpool : register(u2);



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

	uint fragment_morton = fragments.Load(idx);

	uint2 node_idx = svo_traversal_from_fragment(levels, level, fragment_morton);
	node_t node = nodepool[node_idx.x].nodes[node_idx.y];

	// brick position is in morton coordinates
	uint brick_morton = node.brick_id;
	uint3 brick_coords;
	morton_decoding32(brick_morton, brick_coords.x, brick_coords.y, brick_coords.z);
	brick_coords *= 8;

	// voxel coords within brick (lowest 9 bits == 8*8*8)
	uint voxel_morton = voxel & 0x1ff;
	uint3 fragment_coords;
	morton_decoding32(voxel_morton, fragment_coords.x, fragment_coords.y, fragment_coords.z);

	// exact coordinates inside texture3d
	uint3 coords = brick_coords + fragment_coords;

	
	
	uint3 thing;
	morton_decoding32(voxel, thing.x, thing.y, thing.z);
	//thing *= 2.f;
	uint cc = (thing.x & 0xff) | ((thing.y & 0xff) << 8) | ((thing.z & 0xff) << 16) | (0xff << 24);

	// encode color to f32
	brickpool[coords] = float2(asfloat(cc), asfloat(voxel));
}
