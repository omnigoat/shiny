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
	Bytef const* cur = reinterpret_cast<Bytef const*>(begin);
	do
	{
		strm.avail_in = ReadSize;
		strm.next_in = const_cast<Bytef*>(cur);
		cur += ReadSize;

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
				memcpy(acc + accoff, out+outoff, dhave);
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




void voxels_init(dust::context_ptr const& ctx)
{
	vd = dust::get_vertex_declaration({
		{dust::vertex_stream_semantic_t::position, 0, dust::element_format_t::f32x4}
	});

	vb = dust::create_vertex_buffer(ctx, dust::buffer_usage_t::immutable, vd, 8, vbd);
	
	auto f = atma::filesystem::file_t("../shaders/voxel.hlsl");
	auto fm = f.read_into_memory();
	vs = dust::create_vertex_shader(ctx, fm, false, "vs_main");
	ps = dust::create_pixel_shader(ctx, fm, false, "ps_main");


	blockpool = dust::create_texture3d(ctx, dust::texture_usage_t::streaming, dust::element_format_t::f16x4, 128);
	{
		// open file, read everything into memory
		// todo: memory-mapped files

		// inflate 16kb at a time, and call our function for each brick
		ctx->signal_map(blockpool, 0, dust::map_type_t::write_discard, [&](dust::mapped_subresource_t& sr)
		{
			auto f = atma::filesystem::file_t{"../data/dragon.oct"};
			auto m = atma::unique_memory_t(f.size());
			f.read(m.begin(), f.size());
			f.close();

			auto i = (char const*)m.begin();
			i += 4; // skip check
			int node_count = *((int const*)i);
			i += 4;
			i += 4; // brick-count
			i += 4; // zero

			//auto nodes = atma::unique_memory_t(64 * node_count);

			// create node buffer
			nodebuf = dust::create_generic_buffer(ctx, dust::buffer_usage_t::immutable, dust::element_format_t::u32x2, node_count, i, node_count);

			i += 64 * node_count;

			uint const bricksize = 8*8*8*sizeof(float)* 4;
			zl_for_each_chunk<bricksize, 16 * 1024>(i, m.end(), [&ctx, &sr, &bricksize](void const* buf) {
				memcpy(sr.data, buf, bricksize);
			});
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
