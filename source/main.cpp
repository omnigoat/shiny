#include <memory>
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

#include <fooey/widgets/window.hpp>
#include <fooey/fooey.hpp>
#include <fooey/events/resize.hpp>
#include <fooey/events/mouse.hpp>
#include <fooey/event_handler.hpp>

#include <atma/math/vector4f.hpp>
#include <atma/math/matrix4f.hpp>
#include <atma/filesystem/file.hpp>
#include <atma/unique_memory.hpp>

#include <atma/jflate/common.hpp>


#include <iostream>


#include <../vendor/DirectXTex/DirectXTex.h>

#include <zlib.h>

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
7		7	}

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

	inflateEnd(&strm);
	return 0;

zread_fail:
	inflateEnd(&strm);
	return ret;
}



int main()
{
	// setup gui
	auto renderer = fooey::system_renderer();
	auto window = fooey::window("Excitement.", 480, 360);
	renderer->add_window(window);

	// initialise dust
	auto dust_runtime = dust::runtime_t();
	auto ctx = dust::create_context(dust_runtime, window, dust::primary_adapter);
	
	// shaders
	auto vs = dust::create_vertex_shader(ctx);
	auto ps = dust::create_pixel_shader(ctx);

	// vertex declaration
	auto vd = dust::vertex_declaration_t(ctx, vs, {
		{dust::vertex_stream_t::usage_t::position, 0, dust::vertex_stream_t::element_type_t::float32, 4},
		{dust::vertex_stream_t::usage_t::color, 0, dust::vertex_stream_t::element_type_t::float32, 4}
	});
	
	// vertex-buffer
	float vbd[] = {
		 0.5f,  0.5f,  0.5f, 1.f,   1.f, 0.f, 0.f, 1.f,
		 0.5f,  0.5f, -0.5f, 1.f,   0.f, 1.f, 0.f, 1.f,
		 0.5f, -0.5f,  0.5f, 1.f,   0.f, 0.f, 1.f, 1.f,
		 0.5f, -0.5f, -0.5f, 1.f,   1.f, 1.f, 0.f, 1.f,
		-0.5f,  0.5f,  0.5f, 1.f,   1.f, 0.f, 1.f, 1.f,
		-0.5f,  0.5f, -0.5f, 1.f,   0.f, 1.f, 1.f, 1.f,
		-0.5f, -0.5f,  0.5f, 1.f,   1.f, 1.f, 1.f, 1.f,
		-0.5f, -0.5f, -0.5f, 1.f,   1.f, 0.f, 0.f, 1.f,
	};
	auto vb = dust::create_vertex_buffer(ctx, dust::buffer_usage_t::immutable, vd, 8, vbd);

	// index-buffer
	uint16 ibd[] = {
		4, 5, 7, 7, 6, 4, // -x plane
		0, 2, 3, 3, 1, 0, // +x plane
		2, 6, 7, 7, 3, 2, // -y plane
		0, 1, 5, 5, 4, 0, // +y plane
		5, 1, 3, 3, 7, 5, // -z plane
		6, 2, 0, 0, 4, 6, // +z plane
	};
	auto ib = dust::create_index_buffer(ctx, dust::buffer_usage_t::immutable, 16, 36, ibd);

	namespace math = atma::math;

	// constant buffer
	static float t = 0.f;
	atma::math::matrix4f world_matrix;
	auto cb = dust::create_constant_buffer(ctx, sizeof(world_matrix), &world_matrix);

	// texture
	auto tx = dust::create_texture2d(ctx, dust::surface_format_t::un8x4, 128, 128);

	
	// camera
	auto camera = dust::camera_t(
		math::look_at(math::point4f(0.f, 0.f, 2.f), math::point4f(0.f, 0.1f, 0.f), math::vector4f(0.f, 1.f, 0.f, 0.f)),
		math::perspective_fov(math::pi_over_two, (float)window->width() / window->height(), 0.03434f, 120.f)
	);

	// compute shader?
	//auto p = atma::filesystem::path_t{"blam/hooray/things/"};
	auto cs = dust::compute_shader_ptr(); //  dust::create_compute_shader(
	{
		namespace afs = atma::filesystem;

		auto f = afs::file_t{"Debug/cs_test.cso"};
		auto m = atma::unique_memory_t(f.size());
		f.read(m.begin(), f.size());

		cs = dust::create_compute_shader(ctx, m.begin(), m.size());

		// 128x128 texture for reading
	}
	auto sr = dust::create_shader_resource2d(ctx, dust::view_type_t::read_only, dust::surface_format_t::un8x4, 128, 128);
	auto ur = dust::create_shader_resource2d(ctx, dust::view_type_t::read_write, dust::surface_format_t::un8x4, 128, 128);

	// load voxels?
	{
		namespace afs = atma::filesystem;

		// open file, read everything into memory
		// todo: memory-mapped files
		auto f = afs::file_t{"../data/dragon.oct"};
		auto m = atma::unique_memory_t(f.size());
		f.read(m.begin(), f.size());


		// inflate 16kb at a time, and call our function for each brick
		uint const bricksize = 8*8*8*sizeof(float)* 4;
		zl_for_each_chunk<bricksize, 16 * 1024>(m.begin(), m.end(), [](void const* buf) {
			
		});

		auto tx3 = dust::create_texture3d(ctx, dust::surface_format_t::f32x4, 0, 128);
	}


	

	bool running = true;

	window->on({
		{"close", [&running](fooey::event_t const&){
			running = false;
		}}
	});

	window->key_state.on_key(fooey::key_t::Alt + fooey::key_t::Enter, [ctx]{
		ctx->signal_fullscreen_toggle(1);
	});

	window->key_state.on_key(fooey::key_t::Esc, [&running]{
		running = false;
	});

	float x = 0.f;
	float y = 0.f;

	bool mouse_down = false;
	int ox = 0, oy = 0;
	window->on({
		{"mouse-move", [&](fooey::events::mouse_t const& e) {
			//std::cout << "mouse-move " << e.x() << ":" << e.y() << std::endl;
			
			if (mouse_down)
			{
				auto dx = e.x() - ox;
				x += dx * 0.01f;

				auto dy = e.y() - oy;
				y += dy * 0.01f;
			}

			ox = e.x();
			oy = e.y();
		}},

		{"mouse-down.left", [&]{
			mouse_down = true;
		}},
		{"mouse-up.left", [&]{
			mouse_down = false;
		}},
		{"mouse-leave", [&]{
			mouse_down = false;
		}}
	});

	while (running)
	{
		t += 0.1f;

		if (y > atma::math::pi_over_two - 0.1)
			y = atma::math::pi_over_two - 0.1f;
		else if (y < -atma::math::pi_over_two + 0.1f)
			y = -atma::math::pi_over_two + 0.1f;

		camera.move_to(math::point4f(sin(x) * cos(y) * 2.f, sin(y) * 2.f, cos(x) * cos(y) * 2.f));
		camera.look_at(math::point4f());

		camera.set_aspect(window->height() / (float)window->width());
		auto scene = dust::scene_t(ctx, camera);

		world_matrix = math::rotation_y(t * 0.002f);
		scene.signal_update_constant_buffer(cb, sizeof(world_matrix), &world_matrix);
		scene.signal_constant_buffer_upload(1, cb);
		scene.signal_draw(ib, vd, vb, vs, ps);

		ctx->signal_upload_shader_resource(dust::view_type_t::read_only, sr);
		ctx->signal_upload_shader_resource(dust::view_type_t::read_write, ur);
		ctx->signal_upload_compute_shader(cs);
		ctx->signal_compute_shader_dispatch(4, 4, 1);

		//ctx->signal_upload_compute_shader(cs);
		//ctx->signal_execute_compute_shader(cs)


		ctx->signal_clear();
		ctx->signal_draw_scene(scene);
		ctx->signal_block();
		ctx->signal_present();
	}
	ctx->signal_block();

	auto img = DirectX::ScratchImage();
	DirectX::CaptureTexture(ctx->d3d_device().get(), ctx->d3d_immediate_context().get(), ur->backing_texture()->d3d_texture().get(), img);
	DirectX::SaveToTGAFile(*img.GetImage(0, 0, 0), L"someimg.tga");
}
