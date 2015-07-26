
void morton_encoding32(out uint morton, uint index1, uint index2, uint index3)
{
	index1 &= 0x000003ff;
	index2 &= 0x000003ff;
	index3 &= 0x000003ff;
	index1 |= (index1 << 16);
	index2 |= (index2 << 16);
	index3 |= (index3 << 16);
	index1 &= 0x030000ff;
	index2 &= 0x030000ff;
	index3 &= 0x030000ff;
	index1 |= (index1 << 8);
	index2 |= (index2 << 8);
	index3 |= (index3 << 8);
	index1 &= 0x0300f00f;
	index2 &= 0x0300f00f;
	index3 &= 0x0300f00f;
	index1 |= (index1 << 4);
	index2 |= (index2 << 4);
	index3 |= (index3 << 4);
	index1 &= 0x030c30c3;
	index2 &= 0x030c30c3;
	index3 &= 0x030c30c3;
	index1 |= (index1 << 2);
	index2 |= (index2 << 2);
	index3 |= (index3 << 2);
	index1 &= 0x09249249;
	index2 &= 0x09249249;
	index3 &= 0x09249249;

	morton = (uint)index1 | ((uint)index2 << 1) | ((uint)index3 << 2);
}


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

