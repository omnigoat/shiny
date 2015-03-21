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

#include <atma/string.hpp>
#include <atma/intrusive_ptr.hpp>
#include <atma/algorithm.hpp>
#include <atma/enable_if.hpp>
#include <atma/bind.hpp>
#include <atma/filesystem/file.hpp>

#include <shox/morton.hpp>



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
	if (!atma::math::intersect_aabc_triangle(aabc, tri))
	{
		return false;
	}



	if (!buf_.empty())
	{
		imem_for_each([&tri](node_t& x) {
			x.insert(tri);
		});
	}
	//else
	{
		data_.push_back(tri);
	}

	return true;
}

auto octree_t::node_t::inbounds(math::vector4f const& point) -> bool
{
	return aabc.inside(point);
}



struct obj_model_t
{
	using verts_t = std::vector<math::vector4f>;
	using faces_t = std::vector<math::vector4i>;

	obj_model_t(shelf::file_t&);

	auto vertices() const -> verts_t const& { return verts_; }
	auto faces() const -> faces_t const& { return faces_; }
	auto triangle_of(aml::vector4i const&) const -> aml::triangle_t;

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

auto obj_model_t::triangle_of(aml::vector4i const& f) const -> aml::triangle_t
{
	return aml::triangle_t{verts_[f.x - 1], verts_[f.y - 1], verts_[f.z - 1]};
}


struct cb_t
{
	aml::matrix4f world_matrix;
	aml::vector4f color;
};


shiny::vertex_shader_ptr vs_basic, vs;
shiny::fragment_shader_ptr ps_basic, ps;
shiny::data_declaration_t const* vd;

auto debug_draw_triangle(shiny::context_ptr const& ctx, math::triangle_t const& tri) -> void
{
	using namespace atma::math;

	auto n = aml::normalize(math::vector4f{math::cross_product(tri.v1 - tri.v0, tri.v2 - tri.v0)});


#if 0
	// triangle bounding-box
	auto tmin2 = point4f(std::min(tri.v0.x, tri.v1.x), std::min(tri.v0.y, tri.v1.y), std::min(tri.v0.z, tri.v1.z));
	auto tmin  = point4f(std::min(tri.v2.x, tmin2.x), std::min(tri.v2.y, tmin2.y), std::min(tri.v2.z, tmin2.z));
	auto tmax2 = point4f(std::max(tri.v0.x, tri.v1.x), std::max(tri.v0.y, tri.v1.y), std::max(tri.v0.z, tri.v1.z));
	auto tmax  = point4f(std::max(tri.v2.x, tmax2.x), std::max(tri.v2.y, tmax2.y), std::max(tri.v2.z, tmax2.z));
#endif
	//auto tmid  = vector4f{(tmax + tmin) / 2.f};
	

	// vertex-buffer
	float tvbd[] = {
		tri.v0.x, tri.v0.y, tri.v0.z, 1.f, 1.f, .6f, 0.f, .2f,
		tri.v1.x, tri.v1.y, tri.v1.z, 1.f, 0.f, .6f, 0.f, .2f,
		tri.v2.x, tri.v2.y, tri.v2.z, 1.f, 0.f, 0.f, .6f, .2f,
	};

	auto tvb = shiny::create_vertex_buffer(ctx, shiny::buffer_usage_t::immutable, vd, 3, tvbd);

	uint16 tibd[] = {0, 1, 2};
	auto tib = shiny::create_index_buffer(ctx, shiny::buffer_usage_t::immutable, 16, 3, tibd);

	auto tcbd = cb_t{aml::matrix4f::identity(), aml::vector4f{1.f, 1.f, 1.f, 0.3f}};
	auto tcb = shiny::create_constant_buffer(ctx, sizeof(cb_t), &tcbd);
	
	ctx->signal_cs_upload_constant_buffer(1, tcb);
	ctx->signal_draw(tib, vd, tvb, vs_basic, ps_basic);

	// middle-position of triangle
	auto tmid = point4f((tri.v0.x + tri.v1.x + tri.v2.x) / 3.f, (tri.v0.y + tri.v1.y + tri.v2.y) / 3.f, (tri.v0.z + tri.v1.z + tri.v2.z) / 3.f);

	auto edge_normal = [](int a, int b, vector4f const& v0, vector4f const& v1, vector4f const& v2) -> vector4f
	{
		auto AB = vector4f{v1 - v0};
		auto AC = vector4f{v2 - v0};
		auto SN = cross_product(AB, AC);
		auto N = vector4f{cross_product(AB, SN)};
		//auto AC2 = vector4f{AC[b], AC[a], 0.f, 0.f};
		//if (v1[a]*v2[b] - v0[b]*v1[a] - v0[a]*v2[b] - v1[b]*v2[a] + v0[a]*v2[b] + v0[b]*v2[a] < 0.f)
		//if (dot_product(N, AC2) <= 0.f)
		N = N * -1;
		return N;
	};

	// centroids of edges
	auto e0c = vector4f{(tri.v0 + tri.v1) / 2.f};
	auto e1c = vector4f{(tri.v1 + tri.v2) / 2.f};
	auto e2c = vector4f{(tri.v2 + tri.v0) / 2.f};

	auto e0n = edge_normal(0,1, tri.v0, tri.v1, tri.v2); //vector4f{tmid - e0c};
	auto e1n = edge_normal(0,1, tri.v1, tri.v2, tri.v0); //vector4f{tmid - e0c};
	auto e2n = edge_normal(0,1, tri.v2, tri.v0, tri.v1); //vector4f{tmid - e0c};


	// debug-lines
	float dtvbd[] = {
		tmid.x, tmid.y, tmid.z, 1.f, 1.f, 1.f, 1.f, 1.f,
		tmid.x + n.x, tmid.y + n.y, tmid.z + n.z, 1.f, 1.f, 1.f, 1.f, 1.f,
		e0c.x, e0c.y, e0c.z, 1.f, 1.f, 1.f, 1.f, 1.f,
		e0c.x + e0n.x, e0c.y + e0n.y, e0c.z + e0n.z, 1.f,  1.f, 1.f, 1.f, 1.f,
		e1c.x, e1c.y, e1c.z, 1.f, 1.f, 1.f, 1.f, 1.f,
		e1c.x + e1n.x, e1c.y + e1n.y, e1c.z + e1n.z, 1.f, 1.f, 1.f, 1.f, 1.f,
		e2c.x, e2c.y, e2c.z, 1.f, 1.f, 1.f, 1.f, 1.f,
		e2c.x + e2n.x, e2c.y + e2n.y, e2c.z + e2n.z, 1.f, 1.f, 1.f, 1.f, 1.f,
	};

	auto dtvb = shiny::create_vertex_buffer(ctx, shiny::buffer_usage_t::immutable, vd, 8, dtvbd);
	ctx->signal_ia_topology(shiny::topology_t::line);
	ctx->signal_draw(vd, dtvb, vs, ps);
	ctx->signal_ia_topology(shiny::topology_t::triangle);
}


int debug_everything()
{
	auto tri = math::triangle_t
		{ math::point4f(0.15f, 0.f, 0.4f)
		, math::point4f(0.f, 0.3f, 0.15f)
		, math::point4f(-0.35f, 0.05f, 0.f)
		};

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
	vd = shiny_runtime.make_data_declaration({
		{"position", 0, shiny::element_format_t::f32x4},
		{"color", 0, shiny::element_format_t::f32x4}
	});

	// shaders
	auto f = atma::filesystem::file_t("../../shaders/vs_debug.hlsl");
	auto fm = f.read_into_memory();
	vs = shiny::create_vertex_shader(ctx, vd, fm, false);

	auto f2 = atma::filesystem::file_t("../../shaders/ps_debug.hlsl");
	auto fm2 = f2.read_into_memory();
	ps = shiny::create_fragment_shader(ctx, fm2, false);

	auto vs_basic_file = atma::filesystem::file_t("../../shaders/vs_basic.hlsl");
	auto vs_basic_mem = vs_basic_file.read_into_memory();
	vs_basic = shiny::create_vertex_shader(ctx, vd, vs_basic_mem, false);

	auto ps_basic_file = atma::filesystem::file_t("../../shaders/ps_basic.hlsl");
	auto ps_basic_mem = ps_basic_file.read_into_memory();
	ps_basic = shiny::create_fragment_shader(ctx, ps_basic_mem, false);

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
	
	auto cbd = cb_t{aml::matrix4f::identity(), aml::vector4f{1.f, 0.f, 0.f, 1.f}};
	auto cb = shiny::create_constant_buffer(ctx, sizeof(cb_t), &cbd);

	bool running = true;

	window->key_state.on_key_down(fooey::key_t::Alt + fooey::key_t::Enter, [ctx]{
		ctx->signal_fullscreen_toggle(1);
	});

	window->key_state.on_key_down(fooey::key_t::Esc, [&running]{
		running = false;
	});

	bool mouse_down = false;
	int ox = 0, oy = 0;
	window->on({
		{"close", [&running](fooey::event_t const&){
			running = false;
		}}
	});

	auto sbs = shiny::blend_state_t{};
	
	auto blend = ctx->make_blender(sbs);
	ctx->signal_om_blending(blend);

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
	return 0;
}

int main()
{
	auto sf = shelf::file_t{"../../data/dragon.obj"};
	auto obj = obj_model_t{sf};

#if 0
	auto f2 = shelf::file_t{"../../data/dragon2.msh", shelf::file_access_t::write};

	uint64 verts = obj.vertices().size();
	uint64 faces = obj.faces().size();
	f2.write(&verts, sizeof(uint64));
	f2.write(&faces, sizeof(uint64));

	f2.write(&obj.vertices()[0], sizeof(math::vector4f) * verts);
	f2.write(&obj.faces()[0], sizeof(math::vector4i) * faces);

	auto msh_file = shelf::file_t {"../../data/dragon2.msh"};
	auto msh = msh_model_t{msh_file};
#endif

	// try for 128^3 grid
	auto const gridsize = 128;

	struct voxel_t
	{
		uint64 morton;
	};

	auto fragments = std::vector<voxel_t>{};

	// get the real-world bounding box of the model
	auto bbmin = aml::point4f(FLT_MAX, FLT_MAX, FLT_MAX), bbmax = aml::point4f(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (auto const& v : obj.vertices())
	{
		if (v.x < bbmin.x) bbmin.x = v.x;
		if (v.y < bbmin.y) bbmin.y = v.y;
		if (v.z < bbmin.z) bbmin.z = v.z;

		if (bbmax.x < v.x) bbmax.x = v.x;
		if (bbmax.y < v.y) bbmax.y = v.y;
		if (bbmax.z < v.z) bbmax.z = v.z;
	}


	auto tri_dp = bbmax - bbmin;
	//auto mid = (bbmin + bbmax) / 2.f;


	// expand to a cube
	auto gridwidth = std::max({tri_dp.x, tri_dp.y, tri_dp.z});
	auto halfgridwidth = gridwidth / 2.f;

	auto voxelwidth = gridwidth / gridsize;

	for (auto const& f : obj.faces())
	{
		auto t = obj.triangle_of(f);
		auto tbb = t.aabb();
		auto info = aml::aabb_triangle_intersection_info_t{aml::vector4f{voxelwidth, voxelwidth, voxelwidth}, t.v0, t.v1, t.v2};

		auto tgridmin = aml::vector4i{
			(int)(gridsize * ((tbb.min_point().x - bbmin.x) / gridwidth)),
			(int)(gridsize * ((tbb.min_point().y - bbmin.y) / gridwidth)),
			(int)(gridsize * ((tbb.min_point().z - bbmin.z) / gridwidth)),
			0};

		auto tgridmax = aml::vector4i{
			(int)(gridsize * ((tbb.max_point().x - bbmin.x) / gridwidth)),
			(int)(gridsize * ((tbb.max_point().y - bbmin.y) / gridwidth)),
			(int)(gridsize * ((tbb.max_point().z - bbmin.z) / gridwidth)),
			0};

		for (int x = tgridmin.x; x <= tgridmax.x; ++x)
		{
			for (int y = tgridmin.y; y <= tgridmax.y; ++y)
			{
				for (int z = tgridmin.z; z <= tgridmin.z; ++z)
				{
					auto aabc = aml::aabc_t{
						bbmin.x + x * voxelwidth + voxelwidth / 2.f,
						bbmin.y + y * voxelwidth + voxelwidth / 2.f,
						bbmin.z + z * voxelwidth + voxelwidth / 2.f,
						voxelwidth};

					if (aml::intersect_aabb_triangle(aabc, info))
						fragments.push_back({shox::morton_encoding(x, y, z)});
				}
			}
		}
	}

}


