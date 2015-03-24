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
#include <shiny/blend_state.hpp>
#include <shiny/platform/win32/generic_buffer.hpp>
#include <shelf/file.hpp>

#include <pepper/freelook_camera_controller.hpp>

#include <fooey/widgets/window.hpp>
#include <fooey/fooey.hpp>
#include <fooey/events/resize.hpp>
#include <fooey/events/mouse.hpp>
#include <fooey/event_handler.hpp>
#include <fooey/keys.hpp>

#include <atma/math/matrix4f.hpp>
#include <atma/math/vector4f.hpp>
#include <atma/math/vector4i.hpp>
#include <atma/math/intersection.hpp>

#include <atma/filesystem/file.hpp>

#include <sandbox/sandbox.hpp>

auto intialize(shiny::context_ptr const& ctx) -> void
{
	auto&& shiny_runtime = ctx->runtime();


	// data declaration
	sandbox::dd_position_color = shiny_runtime.make_data_declaration({
		{"position", 0, shiny::element_format_t::f32x4},
		{"color", 0, shiny::element_format_t::f32x4}
	});

	// shaders
	auto f = atma::filesystem::file_t("../../shaders/vs_debug.hlsl");
	auto fm = f.read_into_memory();
	auto vs = shiny::create_vertex_shader(ctx, sandbox::dd_position_color, fm, false);

	auto f2 = atma::filesystem::file_t("../../shaders/ps_debug.hlsl");
	auto fm2 = f2.read_into_memory();
	auto ps = shiny::create_fragment_shader(ctx, fm2, false);

	auto vs_basic_file = atma::filesystem::file_t("../../shaders/vs_basic.hlsl");
	auto vs_basic_mem = vs_basic_file.read_into_memory();
	sandbox::vs_flat = shiny::create_vertex_shader(ctx, sandbox::dd_position_color, vs_basic_mem, false);

	auto ps_basic_file = atma::filesystem::file_t("../../shaders/ps_basic.hlsl");
	auto ps_basic_mem = ps_basic_file.read_into_memory();
	sandbox::fs_flat = shiny::create_fragment_shader(ctx, ps_basic_mem, false);

	// geometry
	float cube_verts[] ={
		0.5f, 0.5f, 0.5f, 1.f, 1.f, 0.f, 0.f, 1.f,
		0.5f, 0.5f, -0.5f, 1.f, 0.f, 1.f, 0.f, 1.f,
		0.5f, -0.5f, 0.5f, 1.f, 0.f, 0.f, 1.f, 1.f,
		0.5f, -0.5f, -0.5f, 1.f, 1.f, 1.f, 0.f, 1.f,
		-0.5f, 0.5f, 0.5f, 1.f, 1.f, 0.f, 1.f, 1.f,
		-0.5f, 0.5f, -0.5f, 1.f, 0.f, 1.f, 1.f, 1.f,
		-0.5f, -0.5f, 0.5f, 1.f, 1.f, 1.f, 1.f, 1.f,
		-0.5f, -0.5f, -0.5f, 1.f, 1.f, 0.f, 0.f, 1.f,
	};
	sandbox::vb_cube = shiny::create_vertex_buffer(ctx, shiny::buffer_usage_t::immutable, sandbox::dd_position_color, 8, cube_verts);

	uint16 ibd[] ={
		4, 5, 7, 7, 6, 4, // -x plane
		0, 2, 3, 3, 1, 0, // +x plane
		2, 6, 7, 7, 3, 2, // -y plane
		0, 1, 5, 5, 4, 0, // +y plane
		5, 1, 3, 3, 7, 5, // -z plane
		6, 2, 0, 0, 4, 6, // +z plane
	};
	sandbox::ib_cube = shiny::create_index_buffer(ctx, shiny::buffer_usage_t::immutable, 16, 36, ibd);

	// just do blending here
	auto sbs = shiny::blend_state_t{};
	auto blend = ctx->make_blender(sbs);
	ctx->signal_om_blending(blend);
}

auto bind_input(fooey::window_ptr const& window, shiny::context_ptr const& ctx) -> void
{
	window->key_state.on_key_down(fooey::key_t::Alt + fooey::key_t::Enter, [ctx]{
		ctx->signal_fullscreen_toggle(1);
	});

	window->key_state.on_key_down(fooey::key_t::Esc, [&running]{
		running = false;
	});


	window->on({
		{"close", [&running](fooey::event_t const&){
			running = false;
		}}
	});
}

auto update_input(fooey::window_ptr const& window, shiny::context_ptr const& ctx) -> void
{
}

auto runloop(shiny::context_ptr const& ctx) -> void
{
	bool running = true;
	while (running)
	{
		input_update(window, running);
	}

	// constant buffer
	static float t = 0.f;

	auto cbd = cb_t{aml::matrix4f::identity(), aml::vector4f{1.f, 0.f, 0.f, 1.f}};
	auto cb = shiny::create_constant_buffer(ctx, sizeof(cb_t), &cbd);

	bool running = true;

	

	bool mouse_down = false;
	int ox = 0, oy = 0;
	

	

	auto camera_controller = pepper::freelook_camera_controller_t{window};
	camera_controller.require_mousedown_for_rotation(true);

	//std::chrono::duration<std::chrono::milliseconds> elapsed;
	std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();
	int frames = 0;
	while (running)
	{
		camera_controller.update(1);

		auto scene = shiny::scene_t{ctx, camera_controller.camera(), shiny::rendertarget_clear_t{.2f, .2f, .2f}};

		int i = 0;
		oct.root_->for_each(0, [&](int level, octree_t::node_t const* x)
		{
			if (x->data().empty())
				return;
			if (level != 6)
				return;

			auto vb = shiny::create_vertex_buffer(ctx, shiny::buffer_usage_t::immutable, vd, 8, vbd);
			auto ib = shiny::create_index_buffer(ctx, shiny::buffer_usage_t::immutable, 16, 36, ibd);

			auto scale = aml::matrix4f::scale(x->aabc.diameter());
			auto move = aml::matrix4f::translate(x->aabc.center());
			auto transform = scale * move;

			auto cbd = cb_t{transform, aml::vector4f{1.f, 1.f - (level / 6.f), 0.f, .15f}};
			auto cb = shiny::create_constant_buffer(ctx, sizeof(cb_t), &cbd);
			scene.signal_cs_upload_constant_buffer(1, cb);
			scene.signal_draw(ib, vd, vb, vs, ps);
		});

		ctx->signal_draw_scene(scene);

		debug_draw_triangle(ctx, tri);

		ctx->signal_block();
		ctx->signal_present();
	}

	ctx->signal_block();
}

int main()
{
	// setup gui
	auto renderer = fooey::system_renderer();
	auto window = fooey::window("Excitement.", 480, 360);
	renderer->add_window(window);

	// initialise shiny
	auto shiny_runtime = shiny::runtime_t();
	auto ctx = shiny::create_context(shiny_runtime, window, shiny::primary_adapter);

	initialize(ctx);
	bind_input(window, ctx);
	runloop(ctx);
}