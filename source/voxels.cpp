#include <dust/runtime.hpp>
#include <dust/context.hpp>
#include <dust/vertex_buffer.hpp>
#include <dust/vertex_declaration.hpp>
#include <dust/vertex_shader.hpp>
#include <dust/pixel_shader.hpp>
#include <dust/constant_buffer.hpp>
#include <dust/index_buffer.hpp>
#include <dust/camera.hpp>
#include <dust/scene.hpp>
#include <dust/texture2d.hpp>
#include <dust/compute_shader.hpp>
#include <dust/shader_resource2d.hpp>
#include <dust/texture3d.hpp>
#include <dust/platform/win32/generic_buffer.hpp>

#include <atma/filesystem/file.hpp>
#include <atma/unique_memory.hpp>

#include <zlib.h>


// vertex declaration
dust::vertex_declaration_t const* vd = nullptr;

// vertex-buffer
float vbd[] = {
	-1.f,  1.f,  0.f, 1.f,
	 1.f,  1.f,  0.f, 1.f,
	 1.f, -1.f,  0.f, 1.f,
	 1.f, -1.f,  0.f, 1.f,
	-1.f, -1.f,  0.f, 1.f,
	-1.f,  1.f,  0.f, 1.f,
};
auto vb = dust::vertex_buffer_ptr();

// shaders
auto vs = dust::vertex_shader_ptr();
auto ps = dust::pixel_shader_ptr();

// buffers/textures
auto nodebuf = dust::generic_buffer_ptr();
auto blockpool = dust::texture3d_ptr();
static FILE* fout;

template <size_t AccumulateSize, size_t ReadSize, typename FN>
auto zl_for_each_chunk(void const* begin, void const* end, FN const& fn) -> int
{
	auto strm = z_stream();
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	int ret = inflateInit(&strm);
	ATMA_ENSURE_IS(Z_OK, ret);

	uint8 acc[AccumulateSize];
	uint8 out[ReadSize];

	size_t accoff = 0;

	strm.avail_in = (uInt)((char const*)end - (char const*)begin);
	strm.next_in = const_cast<Bytef*>((Bytef const*)begin);

	do
	{
		
		do
		{
			strm.avail_out = ReadSize;
			strm.next_out = out;
			
			auto ret = inflate(&strm, Z_NO_FLUSH);
			switch (ret) {
				case Z_NEED_DICT:
					ret = Z_DATA_ERROR;
				case Z_DATA_ERROR:
				case Z_MEM_ERROR:
					goto zread_fail;
				case Z_FINISH:
				case Z_STREAM_END:
					goto zread_done;
				default:
					break;
			}

			size_t have = ReadSize - strm.avail_out;

			// pass data to loader
			size_t outoff = 0;
			if (have + accoff > AccumulateSize)
			{
				if (accoff)
				{
					outoff = AccumulateSize - accoff;
					memcpy(acc + accoff, out, outoff);
					fn(acc);
					accoff = 0;
				}
				for (; outoff + AccumulateSize <= have; outoff += AccumulateSize)
					fn(out + outoff);
			}

			if (outoff < have)
			{
				auto dhave = have - outoff;
				memcpy(acc + accoff, out + outoff, dhave);
				accoff += dhave;
				if (accoff >= AccumulateSize)
				{
					fn(acc);
					accoff = 0;
				}
			}

		} while (strm.avail_out == 0);

	} while (ret != Z_STREAM_END);

zread_done:
	inflateEnd(&strm);
	return 0;

zread_fail:
	inflateEnd(&strm);
	return ret;
}


class Float16Compressor
{
	union Bits
	{
		float f;
		int32_t si;
		uint32_t ui;
	};

	static int const shift = 13;
	static int const shiftSign = 16;

	static int32_t const infN = 0x7F800000; // flt32 infinity
	static int32_t const maxN = 0x477FE000; // max flt16 normal as a flt32
	static int32_t const minN = 0x38800000; // min flt16 normal as a flt32
	static int32_t const signN = 0x80000000; // flt32 sign bit

	static int32_t const infC = infN >> shift;
	static int32_t const nanN = (infC + 1) << shift; // minimum flt16 nan as a flt32
	static int32_t const maxC = maxN >> shift;
	static int32_t const minC = minN >> shift;
	static int32_t const signC = signN >> shiftSign; // flt16 sign bit

	static int32_t const mulN = 0x52000000; // (1 << 23) / minN
	static int32_t const mulC = 0x33800000; // minN / (1 << (23 - shift))

	static int32_t const subC = 0x003FF; // max flt32 subnormal down shifted
	static int32_t const norC = 0x00400; // min flt32 normal down shifted

	static int32_t const maxD = infC - maxC - 1;
	static int32_t const minD = minC - subC - 1;

public:

	static uint16 compress(float value)
	{
		Bits v, s;
		v.f = value;
		uint32_t sign = v.si & signN;
		v.si ^= sign;
		sign >>= shiftSign; // logical shift
		s.si = mulN;
		s.si = (int32_t)(s.f * v.f); // correct subnormals
		v.si ^= (s.si ^ v.si) & -(minN > v.si);
		v.si ^= (infN ^ v.si) & -((infN > v.si) & (v.si > maxN));
		v.si ^= (nanN ^ v.si) & -((nanN > v.si) & (v.si > infN));
		v.ui >>= shift; // logical shift
		v.si ^= ((v.si - maxD) ^ v.si) & -(v.si > maxC);
		v.si ^= ((v.si - minD) ^ v.si) & -(v.si > subC);
		return v.ui | sign;
	}

	static float decompress(uint16 value)
	{
		Bits v;
		v.ui = value;
		int32_t sign = v.si & signC;
		v.si ^= sign;
		sign <<= shiftSign;
		v.si ^= ((v.si , minD) ^ v.si) & -(v.si > subC);
		v.si ^= ((v.si , maxD) ^ v.si) & -(v.si > maxC);
		Bits s;
		s.si = mulC;
		s.f *= v.si;
		int32_t mask = -(norC > v.si);
		v.si <<= shift;
		v.si ^= (s.si ^ v.si) & mask;
		v.si |= sign;
		return v.f;
	}
};

void voxels_init(dust::context_ptr const& ctx)
{
	vd = dust::get_vertex_declaration({
		{dust::vertex_stream_semantic_t::position, 0, dust::element_format_t::f32x4}
	});

	vb = dust::create_vertex_buffer(ctx, dust::buffer_usage_t::immutable, vd, 8, vbd);
	
	{
		auto f = atma::filesystem::file_t("../shaders/vs_voxels.cso");
		auto fm = f.read_into_memory();
		vs = dust::create_vertex_shader(ctx, fm, true);
	}

	{
		auto f = atma::filesystem::file_t("../shaders/ps_voxels.cso");
		auto fm = f.read_into_memory();
		ps = dust::create_pixel_shader(ctx, fm, true);
	}
	
	
	//D3DXFloat32To16Array();

	static uint const brick_edge_voxels = 8;
	static uint const brick_size = brick_edge_voxels*brick_edge_voxels*brick_edge_voxels*sizeof(float)*4;
	static uint const brick_count = 30;
	static uint const box_edge_size = brick_edge_voxels*brick_count;

	blockpool = dust::create_texture3d(ctx, dust::texture_usage_t::streaming, dust::element_format_t::f32x4, box_edge_size);
	{
		// open file, read everything into memory
		// todo: memory-mapped files

		// inflate 16kb at a time, and call our function for each brick
		ctx->signal_map(blockpool, 0, dust::map_type_t::write_discard, [&](dust::mapped_subresource_t& sr)
		{
			auto f = atma::filesystem::file_t{"../data/bunny.oct"};
			auto m = atma::unique_memory_t(f.size());
			f.read(m.begin(), f.size());
			f.close();

			auto i = (char const*)m.begin();
			i += 4; // skip check
			int node_count = *((int const*)i);
			i += 4;
			i += 4; // brick-count
			i += 4; // zero

			// create node buffer
			nodebuf = dust::create_generic_buffer(ctx, dust::buffer_usage_t::immutable, 64, node_count, i, node_count);

			i += 64 * node_count;

			struct float4
			{
				float x, y, z, w;
			};

			fout = fopen("verts.txt", "w+");

			// 256kb chunks of data at a time
			static uint const chunk_size = 256 * 1024;
			auto cbuf = (char*)sr.data;
			auto destbuf = reinterpret_cast<float4(&)[box_edge_size][box_edge_size][box_edge_size]>(*(float4*)sr.data);

			uint bricks = 0;
			zl_for_each_chunk<brick_size, chunk_size>(i, m.end(), [&ctx, &destbuf, &bricks](void const* buf)
			{
				auto srcbuf = reinterpret_cast<float4 const*>(buf);

				// 3d-position of block
				int brick_x = brick_edge_voxels * (bricks % brick_count);
				int brick_y = brick_edge_voxels * ((bricks / brick_count) % brick_count);
				int brick_z = brick_edge_voxels * ((bricks / (brick_count * brick_count)) % brick_count);

				size_t srcoff = 0;
				for (int z = 0; z != brick_edge_voxels; ++z) {
					for (int y = 0; y != brick_edge_voxels; ++y) {
						for (int x = 0; x != brick_edge_voxels; ++x) {
#if 0
							if (srcbuf[srcoff].x + srcbuf[srcoff].y + srcbuf[srcoff].z + srcbuf[srcoff].w != 0.f)
								fprintf(fout, "b: %d, o: %d, v: %f %f %f %f\n", bricks, srcoff, srcbuf[srcoff].x, srcbuf[srcoff].y, srcbuf[srcoff].z, srcbuf[srcoff].w);
#endif
							destbuf[brick_z + z][brick_y + y][brick_x + x] = srcbuf[srcoff++];
						}
					}
				}

				++bricks;
			});

			fclose(fout);
		});
	}

	ctx->signal_block();
}

void voxels_render(dust::context_ptr const& ctx)
{
	ctx->signal_ps_upload_shader_resource(0, nodebuf);
	ctx->signal_ps_upload_shader_resource(1, blockpool);
	
	ctx->signal_draw(vd, vb, vs, ps);
}
