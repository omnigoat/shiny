#include <shiny/runtime.hpp>
#include <shiny/context.hpp>
#include <shiny/vertex_buffer.hpp>
#include <shiny/vertex_declaration.hpp>
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
#include <shelf/file.hpp>

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

#include <atma/string.hpp>
#include <atma/intrusive_ptr.hpp>
#include <atma/algorithm.hpp>
#include <atma/enable_if.hpp>
#include <atma/bind.hpp>
#include <atma/filesystem/file.hpp>



namespace math = atma::math;


namespace shelf
{
	enum class path_type_t
	{
		dir,
		file,
		symlink
	};

	struct path_t;
	using  path_ptr = std::unique_ptr<path_t>; // atma::intrusive_ptr<path_t>;

	// path
	struct path_t
	{
		path_t();
		path_t(atma::string const&);
		path_t(path_t const&);

		auto to_string() const -> atma::string;

		auto is_file() const -> bool;

	private:
		path_t(atma::string const&, atma::string::const_iterator const&);

	private:
		atma::string name_;
		path_type_t type_;
		path_ptr child_;
	};


	path_t::path_t()
	{
	}

	path_t::path_t(atma::string const& str)
		: path_t(str, str.begin())
	{
	}

	path_t::path_t(atma::string const& str, atma::string::const_iterator const& begin)
	{
		char const* delims = "/\\";

		auto end = atma::find_first_of(str, begin, delims);
		if (end == str.end()) {
			type_ = path_type_t::file;
		}
		else {
			type_ = path_type_t::dir;
			++end;
		}

		name_ = atma::string(begin, end);

		if (end == begin)
			return;

		child_ = path_ptr(new path_t(str, end));
	}

	path_t::path_t(path_t const& rhs)
		: name_(rhs.name_), type_(rhs.type_)
	{
		if (rhs.child_)
			child_ = path_ptr(new path_t(*rhs.child_));
	}

	auto path_t::to_string() const -> atma::string
	{
		auto result = atma::string();

		for (auto t = this; t != nullptr; t = t->child_.get())
		{
			result += t->name_;
		}

		return result;
	}

	inline auto operator == (path_t const& lhs, path_t const& rhs) -> bool
	{
		return lhs.to_string() == rhs.to_string();
	}

	inline auto operator != (path_t const& lhs, path_t const& rhs) -> bool
	{
		return !operator == (lhs, rhs);
	}


	struct filesystem_t
	{
		auto add_mount(path_t const& virtual_path, path_t const& host_path) -> void;

	private:
		struct node_t;
		using  node_ptr = atma::intrusive_ptr<node_t>;
		using  nodes_t = std::vector<node_ptr>;
		struct mount_t;

		struct node_t : atma::ref_counted
		{
			atma::string name;
			path_type_t type;

			node_t* parent;
			nodes_t children;
		};

		struct mount_t
		{
			path_t host_path;
			path_t mount_path;
			node_ptr mount_node;
		};


	private:
		auto merge_paths(node_t*, node_t*) -> node_ptr;

	private:
		typedef std::vector<mount_t> mounts_t;

		mounts_t mounts_;
	};

	auto filesystem_t::add_mount(path_t const& mount_path, path_t const& host_path) -> void
	{
#ifdef _DEBUG
		atma::for_each(mounts_, [&](mount_t const& x) {  ATMA_ASSERT(x.host_path != host_path);  });
#endif

		// do a thing?
		// host_path   = "relative/path/";
		// host_path2  = "/absolute/path";
		// host_path3  = "/absolute/different/path";

		mounts_.push_back({
			host_path,
			mount_path,
			// something??
		});
	}



	template <size_t Bufsize, typename FN>
	auto for_each_line(abstract_input_stream_t& stream, size_t maxsize, FN&& fn) -> void
	{
		char buf[Bufsize];
		atma::string line;

		// read bytes into line until newline is found
		auto rr = shelf::read_result_t{shelf::stream_status_t::ok, 0};
		while (rr.status == shelf::stream_status_t::ok)
		{
			rr = stream.read(buf, Bufsize);
			auto bufend = buf + rr.bytes_read;
			auto bufp = buf;

			while (bufp != bufend)
			{
				auto newline = std::find(bufp, bufend, '\n');
				line.append(bufp, newline);

				if (newline != bufend) {
					fn(line.raw_begin(), line.raw_size());
					line.clear();
				}

				bufp = (newline == bufend) ? bufend : newline + 1;
			}
		}
	}


}

namespace math = atma::math;


#include <atma/math/aabc.hpp>
#include <atma/unique_memory.hpp>
#include <array>


template <typename T>
struct default_octree_subdivider_t
{
	auto subdivide(uint depth, atma::math::vector4f const& bounds, T const& x) -> void
	{
		
	}
};

struct octree_allocate_tag
{
	explicit octree_allocate_tag(uint levels) : levels(levels) {}
	uint levels;
};

struct octree_t
{
	octree_t() {}

	octree_t(octree_allocate_tag t);

	auto insert(math::triangle_t const&) -> bool;

	auto insert_point(math::vector4f const&) -> bool;
	struct node_t;


	node_t* root_;

private:

private:
	
};

struct octree_t::node_t
{
	node_t()
		: view_(buf_)
	{}

	node_t(math::aabc_t const& aabc)
		: aabc(aabc), view_(buf_)
	{}

	node_t(octree_allocate_tag tag, math::aabc_t const& aabc)
		: aabc(aabc), view_(buf_)
	{
		imem_allocate(tag);
	}

	using data_t = std::vector<math::triangle_t>;

	atma::math::aabc_t aabc;
	
	auto data() const -> data_t const& { return data_; }

	auto inbounds(math::vector4f const& point) -> bool;
	auto insert(math::triangle_t const&) -> bool;

	template <typename FN>
	auto for_each(int level, FN const& fn) -> void
	{
		fn(level, this);
		for (auto i = buf_.begin<node_t>(); i != buf_.end<node_t>(); ++i)
			i->for_each(level + 1, fn);
	}

private:
	auto child_ref(uint idx) -> node_t& { return reinterpret_cast<node_t*>(buf_.begin())[idx]; }

	auto imem_allocate(octree_allocate_tag tag) -> void
	{
		if (tag.levels == 0)
			return;

		ATMA_ASSERT(buf_.empty());
		auto newbuf = atma::unique_memory_t{8 * sizeof(node_t)};
		std::swap(buf_, newbuf);

		auto subtag = octree_allocate_tag{tag.levels - 1};
		for (auto i = 0u; i != 8u; ++i)
			new (&child_ref(i)) node_t {subtag, aabc.octant_of(i)};
	}

#if 0
	auto oct_split() -> void
	{
		imem_allocate(octree_allocate_tag{1});

		for (auto const& x : data_)
			insert(x);
	}
#endif

	auto is_leaf() const -> bool { return buf_.empty(); }

	template <typename FN>
	auto imem_for_each(FN&& fn) -> void
	{
		for (auto i = buf_.begin<node_t>(); i != buf_.end<node_t>(); ++i)
			fn(*i);
	}

private:
	atma::unique_memory_t buf_;
	atma::memory_view_t<atma::unique_memory_t, node_t> view_;

	data_t data_;
};




octree_t::octree_t(octree_allocate_tag t)
	: root_( new node_t{t, math::aabc_t{}} )
{};

auto octree_t::insert(math::triangle_t const& tri) -> bool
{
	return root_->insert(tri);
}

auto octree_t::node_t::insert(math::triangle_t const& tri) -> bool
{
	if (!atma::math::intersect_aabc_triangle2(aabc, tri))
	{
		return false;
	}



	if (!buf_.empty())
	{
		imem_for_each([&tri](node_t& x) {
			x.insert(tri);
		});
	}
	else
	{
		data_.push_back(tri);
	}

	return true;
}

auto octree_t::node_t::inbounds(math::vector4f const& point) -> bool
{
	return aabc.inside(point);
}




auto go() -> void
{
	auto oct = octree_t{octree_allocate_tag{4}};

	auto tri = math::triangle_t{
		math::point4f(0.1f, 0.1f, 0.1f),
		math::point4f(0.4f, 0.2f, 0.15f),
		math::point4f(0.23f, 0.56f, 0.44f)};

	auto tri2 = math::triangle_t {
		math::point4f(0.6f, 0.6f, 0.6f),
		math::point4f(0.6f, 0.2f, 0.1f),
		math::point4f(0.1f, 0.4f, 0.6f)};

	auto r = oct.insert(tri);
	auto r2 = oct.insert(tri2);
}


struct obj_model_t
{
	using verts_t = std::vector<math::vector4f>;
	using faces_t = std::vector<math::vector4i>;

	obj_model_t(shelf::file_t&);

	auto vertices() const -> verts_t const& { return verts_; }
	auto faces() const -> faces_t const& { return faces_; }

private:
	verts_t verts_;
	faces_t faces_;
};

struct mesh_t
{
	using vertices_t = std::vector<math::vector4f>;
	using indices_t = std::vector<math::vector4i>;

	mesh_t();
	mesh_t(size_t vertices, size_t indices);

private:
	vertices_t vertices_;
	indices_t indices_;
};

obj_model_t::obj_model_t(shelf::file_t& file)
{
	shelf::for_each_line<256>(file, 128, [&](char const* str, size_t size)
	{
		switch (str[0])
		{
			case 'v': {
				float x, y, z;
				sscanf(str, "v %f %f %f", &x, &y, &z);
				verts_.push_back(math::point4f(x, y, z));
				break;
			}

			case 'f': {
				int32 a, b, c;
				sscanf(str, "f %d %d %d", &a, &b, &c);
				faces_.push_back(math::vector4i{a, b, c, 0});
				break;
			}
		}
	});
}


int main()
{
	auto tri = math::triangle_t
		{ math::point4f(0.35f, 0.f, 0.f)
		, math::point4f(0.f, 0.3f, 0.0f)
		, math::point4f(0.f, 0.f, 0.4f)
		};

	//auto r = math::intersect_aabc_triangle2(math::aabc_t{0.125f, 0.125f, 0.375f, 0.25f}, tri);

	auto oct = octree_t{octree_allocate_tag{6}};
	oct.insert(tri);


	// setup gui
	auto renderer = fooey::system_renderer();
	auto window = fooey::window("Excitement.", 480, 360);
	renderer->add_window(window);

	// initialise shiny
	auto shiny_runtime = shiny::runtime_t();
	auto ctx = shiny::create_context(shiny_runtime, window, shiny::primary_adapter);

	// vertex declaration
	auto vd = shiny_runtime.vertex_declaration_of({
		{shiny::vertex_stream_semantic_t::position, 0, shiny::element_format_t::f32x4},
		{shiny::vertex_stream_semantic_t::color, 0, shiny::element_format_t::f32x4}
	});

	// shaders
	auto f = atma::filesystem::file_t("../../shaders/vs_debug.hlsl");
	auto fm = f.read_into_memory();
	auto vs = shiny::create_vertex_shader(ctx, vd, fm, false);

	auto f2 = atma::filesystem::file_t("../../shaders/ps_debug.hlsl");
	auto fm2 = f2.read_into_memory();
	auto ps = shiny::create_fragment_shader(ctx, fm2, false);

	auto vs_basic_file = atma::filesystem::file_t("../../shaders/vs_basic.hlsl");
	auto vs_basic_mem = vs_basic_file.read_into_memory();
	auto vs_basic = shiny::create_vertex_shader(ctx, vd, vs_basic_mem, false);

	auto ps_basic_file = atma::filesystem::file_t("../../shaders/ps_basic.hlsl");
	auto ps_basic_mem = ps_basic_file.read_into_memory();
	auto ps_basic = shiny::create_fragment_shader(ctx, ps_basic_mem, false);

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
	auto vb = shiny::create_vertex_buffer(ctx, shiny::buffer_usage_t::immutable, vd, 8, vbd);

	// index-buffer
	uint16 ibd[] = {
		4, 5, 7, 7, 6, 4, // -x plane
		0, 2, 3, 3, 1, 0, // +x plane
		2, 6, 7, 7, 3, 2, // -y plane
		0, 1, 5, 5, 4, 0, // +y plane
		5, 1, 3, 3, 7, 5, // -z plane
		6, 2, 0, 0, 4, 6, // +z plane
	};
	auto ib = shiny::create_index_buffer(ctx, shiny::buffer_usage_t::immutable, 16, 36, ibd);




	namespace math = atma::math;

	// constant buffer
	static float t = 0.f;
	struct cb_t
	{
		aml::matrix4f world_matrix;
		aml::vector4f color;
	};
	
	auto cbd = cb_t{aml::matrix4f::identity(), aml::vector4f{1.f, 0.f, 0.f, 1.f}};
	auto cb = shiny::create_constant_buffer(ctx, sizeof(cb_t), &cbd);

	bool running = true;

	window->key_state.on_key_down(fooey::key_t::Alt + fooey::key_t::Enter, [ctx]{
		ctx->signal_fullscreen_toggle(1);
	});

	window->key_state.on_key_down(fooey::key_t::Esc, [&running]{
		running = false;
	});

	float x = 0.f;
	float y = 0.f;

	auto position         = math::vector4f{0.f, 0.f, -2.f, 1.f};
	auto walk_direction   = math::vector4f{0.f, 0.f, 1.f, 0.f};
	auto strafe_direction = math::vector4f{1.f, 0.f, 0.f, 0.f};

	math::vector4f rotation;
	float walk_speed = 0.02f;


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
				x += dx * 0.01f;

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
		if (A) position += strafe_direction * walk_speed;
		if (S) position -= walk_direction * walk_speed;
		if (D) position -= strafe_direction * walk_speed;

		walk_direction = math::point4f(sin(x) * cos(y), sin(y), cos(x) * cos(y));
		strafe_direction = math::cross_product(walk_direction, math::vector4f(0.f, 1.f, 0.f, 0.f));

		auto camera = shiny::camera_t(
			math::look_at(position, position + walk_direction, math::vector4f {0.f, 1.f, 0.f, 0.f}),
			math::perspective_fov(math::pi_over_two, (float)window->height() / window->width(), 0.03434f, 120.f));

		auto scene = shiny::scene_t{ctx, camera, shiny::rendertarget_clear_t{.2f, .2f, .2f}};
		
		int i = 0;
		oct.root_->for_each(0, [&](int level, octree_t::node_t const* x)
		{
			if (x->data().empty())
				return;
			if (level != 6)
				return;
			//++i;
			// setup box
			auto vb = shiny::create_vertex_buffer(ctx, shiny::buffer_usage_t::immutable, vd, 8, vbd);
			auto ib = shiny::create_index_buffer(ctx, shiny::buffer_usage_t::immutable, 16, 36, ibd);

			auto scale = aml::matrix4f::scale(x->aabc.diameter());
			auto move = aml::matrix4f::translate(x->aabc.origin());
			auto transform = move * scale;

			auto cbd = cb_t{transform, aml::vector4f{1.f, 1.f - (level / 6.f), 0.f, .15f}};
			auto cb = shiny::create_constant_buffer(ctx, sizeof(cb_t), &cbd);
			scene.signal_cs_upload_constant_buffer(1, cb);
			scene.signal_draw(ib, vd, vb, vs, ps);
		});

		// triangle
		// vertex-buffer
		float tvbd[] = {
			tri.v0.x, tri.v0.y, tri.v0.z, 1.f, .3f,0.f,0.f,.2f,
			tri.v1.x, tri.v1.y, tri.v1.z, 1.f, 0.f,.3f,0.f,.2f,
			tri.v2.x, tri.v2.y, tri.v2.z, 1.f, 0.f,0.f,.3f,.2f,
		};

		auto tvb = shiny::create_vertex_buffer(ctx, shiny::buffer_usage_t::immutable, vd, 3, tvbd);

		// index-buffer
		uint16 tibd[] = {
			0, 1, 2
		};
		auto tib = shiny::create_index_buffer(ctx, shiny::buffer_usage_t::immutable, 16, 3, tibd);

		//scene.signal_res_update(cb, sizeof(cb_t), &cbd);
		//ctx->signal_vs_upload_constant_buffer(1, cb);
		//ctx->signal_cs_upload_constant_buffer(1, cb);

		//scene.signal_draw(ib, vd, vb, vs, ps);
		namespace SD = shiny::draw_dsl;

		auto tcbd = cb_t{aml::matrix4f::identity(), aml::vector4f{0.f, 0.f, 1.f, 0.3f}};
		auto tcb = shiny::create_constant_buffer(ctx, sizeof(cb_t), &tcbd);
		scene.signal_cs_upload_constant_buffer(1, tcb);
		scene.signal_draw(tib, vd, tvb, vs_basic, ps_basic);

		ctx->signal_draw_scene(scene);

		ctx->signal_block();
		ctx->signal_present();
	}

	ctx->signal_block();


#if 0
	auto sf = shelf::file_t{"../../data/dragon.obj"};
	auto obj = obj_model_t{sf};

	auto f2 = shelf::file_t{"../../data/dragon2.msh", shelf::file_access_t::write};

	uint64 verts = obj.vertices().size();
	uint64 faces = obj.faces().size();
	f2.write(&verts, sizeof(uint64));
	f2.write(&faces, sizeof(uint64));

	f2.write(&obj.vertices()[0], sizeof(math::vector4f) * verts);
	f2.write(&obj.faces()[0], sizeof(math::vector4i) * faces);
#elif 0
	auto msh_file = shelf::file_t {"../../data/dragon2.msh"};
	auto msh = msh_model_t{msh_file};
#endif
}
