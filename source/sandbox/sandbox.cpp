#include <sandbox/sandbox.hpp>
#include <sandbox/voxelization.hpp>

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
#include <shiny/texture3d.hpp>
#include <shiny/blend_state.hpp>
#include <shiny/generic_buffer.hpp>
#include <shiny/draw_target.hpp>

#include <lion/filesystem.hpp>

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
#include <atma/algorithm.hpp>
#include <atma/function.hpp>

#include <regex>
#include <atomic>

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
	struct asset_t {};

	struct asset_storage_t
	{
		//asset_ptr current;
		asset_t* current;
		//asset_ptr next;
		std::atomic_int32_t current_use_count;
		//std::atomic_int32_t next_use_count;
		std::atomic<asset_t*> ptr;
	};
}

using asset_handle_t = intptr; //asset_storage_t const*;



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



struct log_system_t
{
	static size_t const buf_size = 128;
	static size_t const thread_buf_size = 2048;
	

	log_system_t()
	{
		memset(buf_, 0, buf_size);

		handle_ = std::thread([&] {
			while (running_)
			{
				while (read_position_ != written_position_)
				{
					write_command();
				}
			}
		});
	}

	~log_system_t()
	{
		running_ = false;
		handle_.join();
	}

	template <typename... Args>
	auto signal_log(Args&&... args) -> void
	{
		int k[] = {0, (encode(args), void(), 0)...};
	}

private:
	auto encode(byte color) -> void
	{
		encode_color(color);
	}

	auto encode(char const* string) -> void
	{
		size_t size = strlen(string);
		encode_string((byte const*)string, size);
	}

private:
	//--------------------------------
	//  buffer mutations
	//--------------------------------
	static int const header_size = sizeof(size_t);

	struct buf_allocation_t
	{
		size_t wp;
		size_t nwp;
		size_t p;
	};
	
	auto buf_allocate(size_t size) -> buf_allocation_t
	{
		size_t wp = 0, rp = 0;
		size_t nwp = 0;
		for (;;)
		{
			wp = write_position_.load();
			rp = read_position_.load();

			auto sz = (rp <= wp) ?
				rp + (buf_size - wp) :
				rp - wp;

			if (sz >= size)
			{
				nwp = (wp + size) % buf_size;

				if (write_position_.compare_exchange_strong(wp, nwp))
					break;
			}
		}

		return buf_allocation_t{wp, nwp, wp};
	}

	auto buf_commit(buf_allocation_t& a) -> void
	{
		while (!written_position_.compare_exchange_strong(a.wp, a.nwp))
			break;
	}

private:
	//--------------------------------
	//  encoding
	//--------------------------------
	enum class identifier_t : byte
	{
		string,
		color,
	};

	auto encode_byte(buf_allocation_t& A, byte b) -> void
	{
		ATMA_ASSERT(A.p != A.nwp);

		buf_[A.p % buf_size] = b;
		A.p = (A.p + 1) % buf_size;
	}

	auto encode_uint16(buf_allocation_t& A, uint16 i) -> void
	{
		encode_byte(A, i & 0xff);
		encode_byte(A, (i & 0xff00) >> 8);
	}

	auto encode_mem(buf_allocation_t& A, byte const* buf, size_t size) -> void
	{
		for (size_t i = 0; i != size; ++i) {
			encode_byte(A, buf[i]);
		}
	}

	auto encode_string(byte const* buf, size_t size) -> void
	{
		ATMA_ASSERT(size < buf_size);

		auto A = buf_allocate(size + 3);

		encode_byte   (A, (byte)identifier_t::string);
		encode_uint16 (A, uint16(size));
		encode_mem    (A, buf, size);

		buf_commit(A);
	}

	auto encode_color(byte color) -> void
	{
		auto A = buf_allocate(2);
		encode_byte(A, (byte)identifier_t::color);
		encode_byte(A, color);
	}

	//--------------------------------
	//  decoding
	//--------------------------------
	auto decode_byte(byte& x, size_t p) -> size_t 
	{
		x = buf_[p % buf_size];
		return (p + 1) % buf_size;
	}

	auto decode_uint16(uint16& x, size_t p) -> size_t
	{
		byte h, l;
		p = decode_byte(l, p);
		p = decode_byte(h, p);
		x = (uint16(h) << 8) | l;
		return p;
	}

	auto decode_string(atma::string& x, size_t p, size_t s) -> size_t
	{
		for (size_t i = 0; i != s; ++i)
			x.push_back(buf_[(p + i) % buf_size]);

		return (p + s) % buf_size;
	}


	auto write_command() -> void
	{
		auto rp = read_position_.load();

		byte identifier;
		uint16 size;
		atma::string str;

		rp = decode_byte(identifier, rp);
		rp = decode_uint16(size, rp);
		rp = decode_string(str, rp, size);

		std::cout << "size: " << size << " \"" << str << "\"" << std::endl;

		read_position_ = (read_position_ + size + 3) % buf_size;
	}

private:
	atma::vector<void*> writers_;

	std::thread handle_;
	bool running_ = true;

	byte buf_[buf_size];
	std::atomic_size_t write_position_ = 0;
	std::atomic_size_t written_position_ = 0;
	std::atomic_size_t read_position_ = 0;
};

static int plus(int a, int b) { return a + b; }

application_t::application_t()
	: window_renderer(fooey::system_renderer())
	, window(fooey::window("Excitement!", 800 + 16, 600 + 38))
	, runtime{}
{
 {
	log_system_t shiny_log_system{
		//atma::log_color_t{log_system_t::fatal, 0b11001111},
		//atma::log_color_t{log_system_t::error, 0b11001111},
		//atma::log_color_t{log_system_t::warning, 0b00001110},
	};

	shiny_log_system.signal_log("here is a story about dragons.");
	shiny_log_system.signal_log("once upon a time, they were everywhere.");
	shiny_log_system.signal_log("then they learnt how to brew gin.");
	shiny_log_system.signal_log("so now they're mostly,");
	shiny_log_system.signal_log("at the bar.");
	shiny_log_system.signal_log("but they're also everywhere. alcohol helps with the procreating.");
 }
	//auto shiny_logpipe_handle = atma::log::new_pipe("shiny");


#if 0
	int fds[2];
	
	int res = _pipe(fds, 4096, _O_BINARY);
	ATMA_ASSERT(res == 0);

	int so = _fileno(stdout);
	res = _dup2(fds[1], so);
	//ATMA_ASSERT(res != -1);
	
	char ttry[128];
	setvbuf(stdout, ttry, _IOLBF, 128);
	printf("blamalam\n");
	printf("yay\n");
	//fflush(stdout);
	
	res = _read(fds[0], buf, sizeof(buf) - 1);
#endif
	
	
	shiny::vertex_shader_t
	  * vs_basic = nullptr,
	  * vs_debug = nullptr,
	  * vs_voxel = nullptr;

	//LION_SCOPE_LOCK_ASSETS(
	//	(h1, vs_basic),
	//	(h2, vs_debug),
	//	(h2, vs_debug));

	lion::vfs_t vfs;
	auto fs = lion::physical_filesystem_ptr::make("./resources/published");
	vfs.mount("/res/", fs);

	auto f = vfs.open("/res/shaders/vs_basic.hlsl");
	auto m = lion::read_all(f);
	//std::cout << (char*)m.begin() << std::endl;
	printf("%.*s", (int)m.size(), m.begin());

	struct asset_pattern_t
	{
		std::regex regex;
		std::function<void()> callback;
	};

	//std::regex R{"^vs_.+\\.hlsl"};
	//bool b = std::regex_match("vs_love.hlsl", R);

	//char buf[8000];
	//auto r = f2->read(buf, 1200);
	
	//auto f = vfs.open("/res/shaders/vs_basic.hlsl", file_bind_flags::read_only);
#if 0
	lion::asset_library_t library{vfs};
	library.register_asset_thing("/res/shaders/",
		lion::open_flags_t::read,
		lion::file_watching_flags_t::yes,
		{ lion::asset_pattern{"vs_.+\\.hlsl", &load_vertex_shader},
		  lion::asset_pattern{"fs_.+\\.hlsl", &load_fragment_shader},
		  lion::asset_pattern{"cs_.+\\.hlsl", &load_compute_shader} });
		
	[](lion::input_stream_t const& stream) {
		
	});
#endif

	//library.register_asset_type("*\\.hlsl$", [](lion::input_stream_t const& stream) {
		// do things with f, return an asset_ptr
	//});

	//auto sh = library.load_asset_as<shiny::vertex_shader_t>("/res/shaders/);

	struct vertex_shader_backend_t
	{
		auto d3d_vs() const -> shiny::platform::d3d_vertex_shader_ptr { return {}; }
	};

	using vertex_shader_backend_ptr = indirect_ptr<vertex_shader_backend_t>;

	struct vertex_shader_tx
	{
		auto d3d_vs() const -> shiny::platform::d3d_vertex_shader_ptr
		{
			return backend_->d3d_vs();
		}

	private:
		//vertex_shader_backend_t const* const* backend_;
		vertex_shader_backend_ptr backend_;
	};

	//auto vs = lion::lock_asset_ptr(vertex_shader_handle);

	exit(0);

	function_main();

	window_renderer->add_window(window);
	ctx = shiny::create_context(runtime, window, shiny::primary_adapter);

	// geometry
	dd_position = runtime.make_data_declaration({
		{"position", 0, shiny::format_t::f32x4}
	});

	dd_position_color = runtime.make_data_declaration({
		{"position", 0, shiny::format_t::f32x4},
		{"color", 0, shiny::format_t::f32x4}
	});

	vb_cube = shiny::create_vertex_buffer(ctx, shiny::resource_storage_t::immutable, dd_position_color, 8, cube_vertices);
	ib_cube = shiny::create_index_buffer(ctx, shiny::resource_storage_t::immutable, shiny::format_t::u16, 36, cube_indices);


	// shaders
	auto vs_basic_file = atma::filesystem::file_t("../../shaders/vs_basic.hlsl");
	auto vs_basic_mem = vs_basic_file.read_into_memory();
	vs_flat = shiny::create_vertex_shader(ctx, dd_position, vs_basic_mem, false);

	auto ps_basic_file = atma::filesystem::file_t("../../shaders/ps_basic.hlsl");
	auto ps_basic_mem = ps_basic_file.read_into_memory();
	fs_flat = shiny::create_fragment_shader(ctx, ps_basic_mem, false);
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
		ctx->signal_fullscreen_toggle(1);
	});

	window->key_state.on_key_down(fooey::key_t::Esc, [&running]{
		running = false;
	});

	// camera-controller
	auto&& cc = pepper::freelook_camera_controller_t{window};
	cc.require_mousedown_for_rotation(true);


	// clear!
	ctx->signal_draw_scene(shiny::scene_t{ctx, cc.camera(), shiny::rendertarget_clear_t{aml::vector4f{0.2f, 0.2f, 0.2f}, 1.f}});


	//
	//  initialize plugins
	//
	for (auto const& x : plugins_)
	{
		x->input_bind(window);
		x->gfx_setup(ctx);
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

		ctx->immediate_set_stage(shiny::renderer_stage_t::resource_upload);

		// all plugins draw to same scene
		ctx->immediate_set_stage(shiny::renderer_stage_t::render);
#if 1
		auto&& scene = shiny::scene_t{ctx, cc.camera(), shiny::rendertarget_clear_t{aml::vector4f{0.2f, 0.2f, 0.2f}, 1.f}};
		for (auto const& x : plugins_) {
			x->gfx_ctx_draw(ctx);
			x->gfx_draw(scene);
		}
		ctx->signal_draw_scene(scene);
#endif
		ctx->signal_present();
		ctx->signal_block();
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

int main()
{
	sandbox::application_t app;

	app.register_plugin(plugin_ptr(new sandbox::voxelization_plugin_t{&app}));

	return app.run();
}


