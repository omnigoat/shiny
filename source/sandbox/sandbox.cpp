#include <sandbox/sandbox.hpp>
#include <sandbox/voxelization.hpp>

#include <shiny/runtime.hpp>
#include <shiny/renderer.hpp>
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
#include <shiny/texture3d.hpp>
#include <shiny/blend_state.hpp>
#include <shiny/draw_target.hpp>
#include <shiny/logging.hpp>

#include <lion/filesystem.hpp>
#include <lion/console_log_handler.hpp>
#include <lion/assets.hpp>

#include <pepper/freelook_camera_controller.hpp>

#include <fooey/widgets/window.hpp>
#include <fooey/fooey.hpp>
#include <fooey/events/resize.hpp>
#include <fooey/events/mouse.hpp>
#include <fooey/event_handler.hpp>
#include <fooey/keys.hpp>

#include <rose/runtime.hpp>
#include <rose/console.hpp>

#include <atma/math/matrix4f.hpp>
#include <atma/math/vector4f.hpp>
#include <atma/math/vector4i.hpp>
#include <atma/math/intersection.hpp>

#include <atma/algorithm.hpp>
#include <atma/function.hpp>
#include <atma/console.hpp>
#include <atma/atomic.hpp>
#include <atma/lockfree_queue.hpp>
#include <atma/threading.hpp>
#include <atma/logging.hpp>
#include <atma/string.hpp>
#include <atma/handle_table.hpp>

#include <regex>
#include <atomic>
#include <iomanip>
#include <limits>

#include <io.h>
#include <fcntl.h>

using namespace sandbox;
using sandbox::application_t;

float application_t::cube_vertices[] =
{
	0.5f, 0.5f, 0.5f, 1.f,    1.f, 0.f, 0.f, 0.2f,
	0.5f, 0.5f, -0.5f, 1.f,   0.f, 1.f, 0.f, 0.2f,
	0.5f, -0.5f, 0.5f, 1.f,   0.f, 0.f, 1.f, 0.2f,
	0.5f, -0.5f, -0.5f, 1.f,  1.f, 1.f, 0.f, 0.2f,
	-0.5f, 0.5f, 0.5f, 1.f,   1.f, 0.f, 1.f, 0.2f,
	-0.5f, 0.5f, -0.5f, 1.f,  0.f, 1.f, 1.f, 0.2f,
	-0.5f, -0.5f, 0.5f, 1.f,  1.f, 1.f, 1.f, 0.2f,
	-0.5f, -0.5f, -0.5f, 1.f, 1.f, 0.f, 0.f, 0.2f,
};

uint16 application_t::cube_indices[] =
{
	4, 5, 7, 7, 6, 4, // -x plane
	0, 2, 3, 3, 1, 0, // +x plane
	2, 6, 7, 7, 3, 2, // -y plane
	0, 1, 5, 5, 4, 0, // +y plane
	5, 1, 3, 3, 7, 5, // -z plane
	6, 2, 0, 0, 4, 6, // +z plane
};

extern int function_main();

template <typename T>
struct indirect_ptr
{
	indirect_ptr(T* const& ptr)
		: ptr_(&ptr)
	{}

	auto operator -> () const -> T*
	{
		return *ptr_;
	}

private:
	T* const* ptr_;
};

namespace lion
{
	//struct asset_t {};
	//
	//struct asset_storage_t
	//{
	//	//asset_ptr current;
	//	asset_t* current;
	//	//asset_ptr next;
	//	std::atomic_int32_t current_use_count;
	//	//std::atomic_int32_t next_use_count;
	//	std::atomic<asset_t*> ptr;
	//};
}

using asset_handle_t = intptr; //asset_storage_t const*;


#if 0
char buf[256];


struct stdio_catcher_t
{
	static int const bufsize = 4096;
	static int const stdin_idx = 0, stdout_idx = 1, stderr_idx = 2, stdmax = 3;
	static int const fd_read = 0, fd_write = 1;

	stdio_catcher_t()
		: stdio_filehandles_{
			_fileno(stdin),
			_fileno(stdout),
			_fileno(stderr)
		}
	{
		engine_.signal([&]
		{
			// pipe stdout, stdin, stderr to *new places*
			//ATMA_ENSURE_IS(0, _pipe(fds_[0], 4096, _O_BINARY));
			ATMA_ENSURE_IS(0, _pipe(fds_[stdout_idx], 4096, _O_BINARY));
			ATMA_ENSURE_IS(0, _pipe(fds_[stderr_idx], 4096, _O_BINARY));

			for (int i = stdout_idx; i != stdmax; ++i)
			{
				int r = _dup2(fds_[i][fd_write], stdio_filehandles_[i]);
				ATMA_ASSERT(r != -1);
			}
		});

		engine_.signal_evergreen([&]{
			// do stderr first, because in the case of horrifying errors, we want
			// to tell the world as fast as possible
			int sz = _read(fds_[stderr_idx][fd_read], buf_[stderr_idx], sizeof(buf_[stderr_idx]) - 1);

			//int sz = _read(fds_[0][0], buf_[0], bufsize);
			//fwrite(buf_[0], 1, sz, stderr);
		});
	}

private:
	int stdio_filehandles_[3];
	int fds_[3][2]; // stdout, stdin, stderr {read|write}
	char buf_[3][bufsize];

	atma::thread::engine_t engine_;
};


enum class log_levels_t : int
{
	fatal = 0,
	error = 1,
	warning = 2,
	info = 3,
	trace = 4,

	size = 5
};

#endif

static int plus(int a, int b) { return a + b; }

//atma::logsink_t soundlog;



application_t::application_t(rose::runtime_t* rr)
	: window_renderer(fooey::system_renderer())
	, window(fooey::window("Excitement!", 800 + 16, 600 + 38))
	, runtime{}
	, rose_runtime_{rr}
	, vfs_{rr}
{
	// virtual file system, mount res folder
	auto fs = lion::physical_filesystem_ptr::make("resources/published");
	vfs_.mount("/res/", fs);

	// Windows window
	window_renderer->add_window(window);

	// shiny contex
	rndr = shiny::create_context(runtime, window, shiny::primary_adapter);


	// CURRENT
	//auto vs_basic_file = atma::filesystem::file_t("../shaders/vs_basic.hlsl");
	//auto vs_basic_mem = vs_basic_file.read_into_memory();
	//vs_flat = shiny::create_vertex_shader(rndr, dd_position, vs_basic_mem, false);

	// DESIRED
	// auto vs_basic = library.load_asset_as<shiny::vertex_shader_t>("/res/shaders/vs_basic.hlsl");


	// geometry
	dd_position = runtime.make_data_declaration({
		{"position", 0, shiny::format_t::f32x4}
	});

	dd_position_color = runtime.make_data_declaration({
		{"position", 0, shiny::format_t::f32x4},
		{"color", 0, shiny::format_t::f32x4}
	});

	vb_cube = rndr->make_vertex_buffer(shiny::resource_storage_t::immutable, dd_position_color, 8, cube_vertices, 8);
	ib_cube = rndr->make_index_buffer(shiny::resource_storage_t::immutable, shiny::format_t::u16, 36, cube_indices, 36);


	//auto f = vfs.open("/res/shaders/vs_basic.hlsl");
	//auto m = lion::read_all(f);
	//auto sdf = shiny::create_vertex_shader(rndr, m, false);
	//shiny::vertex_shader_t::make()

	// shaders
	//vs_flat = shiny::vertex_shader_t::make(rndr, "../shaders/vs_basic.hlsl", false);
	//fs_flat = shiny::fragment_shader_t::make(rndr, "../shaders/ps_basic.hlsl", false);
}

auto application_t::run() -> int
{
	bool running = true;


	//
	//  generic input handling
	//
	window->on({
		{"close", [&running](fooey::event_t const&){
			running = false;
		}}
	});

	window->key_state.on_key_down(fooey::key_t::Alt + fooey::key_t::Enter, [&]{
		rndr->signal_fullscreen_toggle(1);
	});

	window->key_state.on_key_down(fooey::key_t::Esc, [&running]{
		running = false;
	});

	// camera-controller
	auto&& cc = pepper::freelook_camera_controller_t{window};
	cc.require_mousedown_for_rotation(true);


	// clear!
	shiny::scene_t clear_scene{rndr, cc.camera(), shiny::rendertarget_clear_t{aml::vector4f{0.2f, 0.2f, 0.2f}, 1.f}};
	rndr->signal_draw_scene(clear_scene);


	//
	//  initialize plugins
	//
	for (auto const& x : plugins_)
	{
		x->input_bind(window);
		x->gfx_setup(rndr);
		x->main_setup();
	}


	// timestep of 16ms = 60hz
	auto const timestep_uint = 16u;
	auto const timestep = std::chrono::milliseconds(timestep_uint);
	
	// main-loop
	auto time_frame_end = std::chrono::high_resolution_clock::time_point();
	while (running)
	{
		auto time_now = std::chrono::high_resolution_clock::now();
		
		if (std::chrono::duration_cast<std::chrono::milliseconds>(time_now - time_frame_end) <= timestep)
			continue;
		time_frame_end = time_now;

		cc.update(timestep_uint);

		for (auto const& x : plugins_) {
			x->input_update();
		}

		rndr->immediate_set_stage(shiny::renderer_stage_t::resource_upload);

		// all plugins draw to same scene
		rndr->immediate_set_stage(shiny::renderer_stage_t::render);

		auto&& scene = shiny::scene_t{rndr, cc.camera(), shiny::rendertarget_clear_t{aml::vector4f{0.2f, 0.2f, 0.2f}, 1.f}};
		for (auto const& x : plugins_) {
			x->gfx_ctx_draw(rndr);
			x->gfx_draw(scene);
		}
		rndr->signal_draw_scene(scene);

		rndr->signal_present();
		rndr->signal_block();
	}

	return 0;
}


auto application_t::register_plugin(plugin_ptr const& plugin) -> void
{
	plugins_.push_back(plugin);
}

auto plugin_t::dd_position() const -> shiny::data_declaration_t const*
{
	return app_->dd_position;
}

auto plugin_t::dd_position_color() const -> shiny::data_declaration_t const*
{
	return app_->dd_position_color;
}

auto plugin_t::cube_vertices() const -> float const*
{
	return app_->cube_vertices;
}

auto plugin_t::cube_indices() const -> uint16 const*
{
	return app_->cube_indices;
}

auto plugin_t::vs_flat() const -> shiny::vertex_shader_ptr const&
{
	return app_->vs_flat;
}

auto plugin_t::fs_flat() const -> shiny::fragment_shader_ptr const&
{
	return app_->fs_flat;
}


#if 0
namespace atma
{
	template <typename T, size_t N>
	struct arena_allocator_t
	{
		using value_type = T;
		using pointer = value_type*;
		using const_pointer = value_type const*;

		template <typename U>
		struct rebind { using other = arena_allocator_t<U, N>; };


		auto allocate(size_t n) -> T*;

	private:
		struct block_t;
		using  block_ptr = std::unique_ptr<block_t>;

		std::vector<block_ptr> blocks_;
		size_t idx_ = 0;
	};


	template <typename T, size_t N>
	struct arena_allocator_t<T, N>::block_t
	{
		block_t()
			: data{reinterpret_cast<T*>(new char[sizeof(T) * N])}
			, freelist{{0, (uint16)N}}
		{}

		auto allocate(size_t) -> T*;

		size_t max_contiguous = N;
		std::vector<std::pair<uint16, uint16>> freelist;
		T* data = nullptr;
	};




	template <typename T, size_t N>
	inline auto arena_allocator_t<T, N>::allocate(size_t n) -> T*
	{
		for (auto const& b : blocks_)
		{
			if (n <= b->max_contiguous)
				return b->allocate(n);
		}

		blocks_.emplace_back(new block_t);
		return blocks_.back()->allocate(n);
	}


	template <typename T, size_t N>
	inline auto arena_allocator_t<T, N>::block_t::allocate(size_t n) -> T*
	{
		uint16 begin, end;
		for (auto& x : freelist)
		{
			auto const xs = x.second - x.first;
			if (n <= xs)
			{
				bool iseqcont = xs == max_contiguous;
				auto r = x.second - n;
				x.second -= n;

				if (iseqcont)
				{
					max_contiguous = 0;
					for (auto const& y : freelist)
						max_contiguous = (y.second - y.first) > max_contiguous ? (y.second - y.first) : max_contiguous;
				}

				if (x.first == x.second) {
					std::swap(x, freelist.back());
					freelist.pop_back();
				}

				return data + r;
			}
		}

		return nullptr;
	}
}
#endif


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	// thread pool for various systems
	atma::thread_pool_t global_thread_pool{8};

	// platform runtime & console-logging-handler
	atma::inplace_engine_t fs_engine_{4096};
	rose::runtime_t RR{&fs_engine_};
	lion::console_log_handler_t console_log{RR.console()};

	// shiny runtime & logging through console
	shiny::logging::runtime_t SLR;
	SLR.attach_handler(&console_log);
	shiny::logging::set_runtime(&SLR);

	sandbox::application_t app{&RR};

	app.register_plugin(plugin_ptr(new sandbox::voxelization_plugin_t{&app}));

	return app.run();
}
