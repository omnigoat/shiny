#include <memory>
#include <dust/runtime.hpp>
#include <dust/context.hpp>
#include <dust/vertex_buffer.hpp>
#include <dust/vertex_declaration.hpp>
#include <dust/vertex_shader.hpp>
#include <dust/fragment_shader.hpp>
#include <dust/constant_buffer.hpp>
#include <dust/index_buffer.hpp>
#include <dust/camera.hpp>
#include <dust/scene.hpp>
#include <dust/texture2d.hpp>
#include <dust/compute_shader.hpp>
#include <dust/shader_resource2d.hpp>
#include <dust/texture3d.hpp>
#include <dust/platform/win32/generic_buffer.hpp>

#include <fooey/widgets/window.hpp>
#include <fooey/fooey.hpp>
#include <fooey/events/resize.hpp>
#include <fooey/events/mouse.hpp>
#include <fooey/event_handler.hpp>
#include <fooey/keys.hpp>

#include <atma/math/vector4f.hpp>
#include <atma/math/matrix4f.hpp>
#include <atma/filesystem/file.hpp>
#include <atma/unique_memory.hpp>

#include <iostream>


#include <../vendor/DirectXTex/DirectXTex.h>



#define LOAD_VOXELS 1
#define RENDER_VOXELS 1
#define RENDER_CUBE 0
#define CS_TEST 0


extern void voxels_init(dust::context_ptr const& ctx);
extern void voxels_render(dust::context_ptr const& ctx);





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
	auto f = atma::filesystem::file_t("../shaders/vs_basic.hlsl");
	auto fm = f.read_into_memory();
	auto vs = dust::create_vertex_shader(ctx, fm, false);

	auto f2 = atma::filesystem::file_t("../shaders/ps_basic.hlsl");
	auto fm2 = f2.read_into_memory();
	auto ps = dust::create_fragment_shader(ctx, fm2, false);

	// vertex declaration
	auto vd = dust::get_vertex_declaration({
		{dust::vertex_stream_semantic_t::position, 0, dust::element_format_t::f32x4},
		{dust::vertex_stream_semantic_t::color, 0, dust::element_format_t::f32x4}
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

	struct voxel_cb
	{
		math::vector4f position;
		float x, y;
	};

	auto vcbd = voxel_cb();
	auto vcb = dust::create_constant_buffer(ctx, sizeof(voxel_cb), &vcbd);

	// camera
	auto camera = dust::camera_t(
		math::look_at(math::point4f(0.f, 0.f, 2.f), math::point4f(0.f, 0.1f, 0.f), math::vector4f(0.f, 1.f, 0.f, 0.f)),
		math::perspective_fov(math::pi_over_two, (float)window->width() / window->height(), 0.03434f, 120.f)
	);

	
	// compute shader?
#if CS_TEST
	auto cs = dust::compute_shader_ptr();
	{
		namespace afs = atma::filesystem;

		auto f = afs::file_t{"Debug/cs_test.cso"};
		auto m = atma::unique_memory_t(f.size());
		f.read(m.begin(), f.size());

		cs = dust::create_compute_shader(ctx, m.begin(), m.size());
	}

	// surfaces for compute shader test
	auto sr = dust::create_shader_resource2d(ctx, dust::view_type_t::read_only, dust::element_format_t::un8x4, 128, 128);
	auto ur = dust::create_shader_resource2d(ctx, dust::view_type_t::read_write, dust::element_format_t::un8x4, 128, 128);

	// testing generic buffers too
	float color[4] = {.0f, .4f, .4f, 1.f};
	auto gb = dust::create_generic_buffer(ctx, dust::buffer_usage_t::immutable, dust::element_format_t::f32x4, 1, color, 1);
#endif



	// loading voxels?
#if LOAD_VOXELS
	voxels_init(ctx);
#endif

	

	bool running = true;

	window->key_state.on_key_down(fooey::key_t::Alt + fooey::key_t::Enter, [ctx]{
		ctx->signal_fullscreen_toggle(1);
	});

	window->key_state.on_key_down(fooey::key_t::Esc, [&running]{
		running = false;
	});

	float x = 0.f;
	float y = 0.f;

	atma::math::vector4f position ={0.f, 0.f, -2.f, 1.f};
	math::vector4f walk_direction;
	math::vector4f strafe_direction;
	math::vector4f rotation;
	float walk_speed = 0.002f;


	bool mouse_down = false;
	int ox = 0, oy = 0;
	window->on({
		{"close", [&running](fooey::event_t const&){
			running = false;
		}},

		{"mouse-move", [&](fooey::events::mouse_t const& e) {
			
			if (mouse_down)
			{
				auto dx = e.x() - ox;
				x -= dx * 0.01f;

				auto dy = e.y() - oy;
				y -= dy * 0.01f;
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

	bool W = false, A = false, S = false, D = false;
	window->key_state.on_key_down(fooey::key_t::W, [&] { W = true; });
	window->key_state.on_key_down(fooey::key_t::A, [&] { A = true; });
	window->key_state.on_key_down(fooey::key_t::S, [&] { S = true; });
	window->key_state.on_key_down(fooey::key_t::D, [&] { D = true; });

	window->key_state.on_key_up(fooey::key_t::W, [&] { W = false; });
	window->key_state.on_key_up(fooey::key_t::A, [&] { A = false; });
	window->key_state.on_key_up(fooey::key_t::S, [&] { S = false; });
	window->key_state.on_key_up(fooey::key_t::D, [&] { D = false; });

	//std::chrono::duration<std::chrono::milliseconds> elapsed;
	std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();
	int frames = 0;
	while (running)
	{
		t += 0.1f;

		if (y > atma::math::pi_over_two - 0.1)
			y = atma::math::pi_over_two - 0.1f;
		else if (y < -atma::math::pi_over_two + 0.1f)
			y = -atma::math::pi_over_two + 0.1f;

		if (W) position += walk_direction * walk_speed;
		if (A) position -= strafe_direction * walk_speed;
		if (S) position -= walk_direction * walk_speed;
		if (D) position += strafe_direction * walk_speed;

		walk_direction = math::point4f(sin(x) * cos(y), sin(y), cos(x) * cos(y));
		strafe_direction = math::cross_product(walk_direction, math::vector4f(0.f, 1.f, 0.f, 0.f));

		vcbd.position = position;
		vcbd.x = x;
		vcbd.y = y;
		ctx->signal_res_update(vcb, sizeof(vcbd), &vcbd);
		ctx->signal_cs_upload_constant_buffer(2, vcb);


		camera.move_to(math::point4f(sin(x) * cos(y) * 2.f, sin(y) * 2.f, cos(x) * cos(y) * 2.f));
		camera.look_at(math::point4f());

		
		
		camera.set_aspect(window->height() / (float)window->width());
		auto scene = dust::scene_t(ctx, camera);

		// do this just to upload the camera :P
		ctx->signal_draw_scene(scene);

		ctx->signal_clear();

#if RENDER_VOXELS
		//ctx->signal_cs_upload_constant_buffer(0, camera.)
		ctx->signal_res_update(cb, sizeof(world_matrix), &world_matrix);
		ctx->signal_vs_upload_constant_buffer(1, cb);
		ctx->signal_fs_upload_constant_buffer(1, cb);
		voxels_render(ctx);
#endif

#if RENDER_CUBE
		world_matrix = math::rotation_y(t * 0.002f);
		scene.signal_res_update(cb, sizeof(world_matrix), &world_matrix);
		scene.signal_cs_upload_constant_buffer(1, cb);
		scene.signal_draw(ib, vd, vb, vs, ps);
		ctx->signal_draw_scene(scene);
#endif

#if CS_TEST
		ctx->signal_cs_upload_shader_resource(dust::view_type_t::read_only, sr);
		ctx->signal_cs_upload_shader_resource(dust::view_type_t::read_write, ur);
		ctx->signal_cs_upload_generic_buffer(1, gb);
		ctx->signal_cs_set(cs);
		ctx->signal_cs_dispatch(4, 4, 1);
#endif


		ctx->signal_block();
		ctx->signal_present();
		++frames;

		if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() > 1000)
		{
			std::cout << "FPS: " << frames << std::endl;
			start = std::chrono::high_resolution_clock::now();
			frames = 0;
		}
	}
	ctx->signal_block();

#if CS_TEST
	auto img = DirectX::ScratchImage();
	DirectX::CaptureTexture(ctx->d3d_device().get(), ctx->d3d_immediate_context().get(), ur->backing_texture()->d3d_texture().get(), img);
	DirectX::SaveToTGAFile(*img.GetImage(0, 0, 0), L"someimg.tga");
#endif
}
