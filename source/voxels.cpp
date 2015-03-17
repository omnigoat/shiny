#include <shiny/runtime.hpp>
#include <shiny/context.hpp>
#include <shiny/vertex_buffer.hpp>
#include <shiny/data_declaration.hpp>
#include <shiny/vertex_shader.hpp>
#include <shiny/fragment_shader.hpp>
#include <shiny/constant_buffer.hpp>
#include <shiny/index_buffer.hpp>
#include <shiny/camera.hpp>
#include <shiny/scene.hpp>
#include <shiny/texture2d.hpp>
#include <shiny/compute_shader.hpp>
#include <shiny/shader_resource2d.hpp>
#include <shiny/texture3d.hpp>
#include <shiny/platform/win32/generic_buffer.hpp>
#include <shiny/constant_buffer.hpp>

#include <atma/filesystem/file.hpp>
#include <atma/unique_memory.hpp>
#include <atma/algorithm.hpp>


#include <zlib.h>


// vertex declaration
shiny::data_declaration_t const* vd = nullptr;

// vertex-buffer
float vbd[] = {
	-1.f,  1.f,  0.f, 1.f,
	 1.f,  1.f,  0.f, 1.f,
	 1.f, -1.f,  0.f, 1.f,
	 1.f, -1.f,  0.f, 1.f,
	-1.f, -1.f,  0.f, 1.f,
	-1.f,  1.f,  0.f, 1.f,
};
auto vb = shiny::vertex_buffer_ptr();

// shaders
auto vs = shiny::vertex_shader_ptr();
auto ps = shiny::fragment_shader_ptr();

struct voxel_cb
{
	atma::math::vector4f position;
	float x, y;
};

auto vcbd = voxel_cb();
auto vcb = shiny::constant_buffer_ptr();

// buffers/textures
auto nodebuf = shiny::generic_buffer_ptr();
auto bricktex = shiny::texture3d_ptr();
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

void voxels_init(shiny::context_ptr const& ctx)
{
	vd = ctx->runtime().make_data_declaration({
		{"position", 0, shiny::element_format_t::f32x4}
	});

	vb = shiny::create_vertex_buffer(ctx, shiny::buffer_usage_t::immutable, vd, 8, vbd);
	
	{
		auto f = atma::filesystem::file_t("../../shaders/vs_voxels.cso");
		auto fm = f.read_into_memory();
		vs = shiny::create_vertex_shader(ctx, vd, fm, true);
	}

	{
		auto f = atma::filesystem::file_t("../../shaders/ps_voxels.cso");
		auto fm = f.read_into_memory();
		ps = shiny::create_fragment_shader(ctx, fm, true);
	}

	vcb = shiny::create_constant_buffer(ctx, sizeof(voxel_cb), &vcbd);

	static uint const brick_edge_voxels = 8;
	static uint const brick_size = brick_edge_voxels*brick_edge_voxels*brick_edge_voxels*sizeof(float)*4;
	static uint const brick_count = 30;
	static uint const box_edge_size = brick_edge_voxels*brick_count;

	bricktex = shiny::create_texture3d(ctx, shiny::texture_usage_t::streaming, shiny::element_format_t::f32x4, box_edge_size);
	{
		// open file, read everything into memory
		// todo: memory-mapped files
		shiny::texture3d_ptr blam;

		// inflate 16kb at a time, and call our function for each brick
		ctx->signal_res_map(bricktex, 0, shiny::map_type_t::write_discard, [&](shiny::mapped_subresource_t& sr)
		{
			auto f = atma::filesystem::file_t{"../../data/bunny.oct"};
			auto m = atma::unique_memory_t(f.size());
			f.read(m.begin(), f.size());
			f.close();

			auto i = (char const*)m.begin();
			i += 4; // skip check
			int node_count = *((int const*)i);
			i += 4;
			int fbricks = *((int const*)i);
			i += 4; // brick-count
			i += 4; // zero


			//nodebuf = shiny::create_generic_buffer(ctx, shiny::buffer_usage_t::immutable, 64, 1, nodes_tiles, 1);
			nodebuf = shiny::create_generic_buffer(ctx, shiny::buffer_usage_t::immutable, 64, node_count, i, node_count);

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
				for (int z = 0; z != brick_edge_voxels; ++z)
					for (int y = 0; y != brick_edge_voxels; ++y)
						for (int x = 0; x != brick_edge_voxels; ++x)
							destbuf[brick_z + z][brick_y + y][brick_x + x] = srcbuf[srcoff++];

				++bricks;
			});

			fclose(fout);
		});
	}

	ctx->signal_block();
}

void voxels_update(shiny::context_ptr const& ctx, atma::math::vector4f const& camera_position, float yaw, float pitch)
{
	vcbd.position = camera_position;
	vcbd.x = yaw;
	vcbd.y = pitch;

	ctx->signal_res_update(vcb, sizeof(vcbd), &vcbd);
	ctx->signal_cs_upload_constant_buffer(2, vcb);
}

void voxels_render(shiny::context_ptr const& ctx)
{
#if 0
	ctx->signal_fs_upload_shader_resource(0, nodebuf);
	ctx->signal_fs_upload_shader_resource(1, bricktex);


	ctx->signal_draw(vd, vb, vs, ps);

#else
	ctx->signal_draw(
		shiny::shared_state_t{
			{{shiny::constant_buffer_index::user, vcb}}
		},

		shiny::vertex_stage_state_t{vs, vb},

		shiny::fragment_stage_state_t{
			ps,
			{{shiny::constant_buffer_index::user, vcb}},
			{{0, nodebuf}, {1, bricktex}}
		});
#endif
}
