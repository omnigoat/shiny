#include <sandbox/voxelization.hpp>

#include <shiny/context.hpp>
#include <shiny/scene.hpp>
#include <shiny/draw.hpp>
#include <shiny/compute.hpp>
#include <shiny/generic_buffer.hpp>
#include <shiny/resource_view.hpp>
#include <shiny/texture3d.hpp>
#include <shiny/compute_shader.hpp>
#include <shiny/constant_buffer.hpp>

#include <shelf/file.hpp>

#include <shox/morton.hpp>

#include <atma/math/matrix4f.hpp>
#include <atma/math/vector4f.hpp>
#include <atma/math/vector4i.hpp>
#include <atma/math/triangle.hpp>
#include <atma/math/aabc.hpp>
#include <atma/math/intersection.hpp>

#include <atma/filesystem/file.hpp>
#include <atma/vector.hpp>

using namespace sandbox;
using sandbox::voxelization_plugin_t;





struct obj_model_t
{
	using verts_t = std::vector<aml::vector4f>;
	using faces_t = std::vector<aml::vector4i>;

	obj_model_t(shelf::file_t&);

	auto vertices() const -> verts_t const& { return verts_; }
	auto faces() const -> faces_t const& { return faces_; }
	auto triangle_of(aml::vector4i const&) const -> aml::triangle_t;

private:
	verts_t verts_;
	faces_t faces_;
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
				verts_.push_back(aml::point4f(x, y, -z)); // reflect z because RH -> LH
				break;
			}

			case 'f': {
				int32 a, b, c;
				sscanf(str, "f %d %d %d", &a, &b, &c);
				faces_.push_back(aml::vector4i{a - 1, b - 1, c - 1, 0});
				break;
			}
		}
	});
}

auto obj_model_t::triangle_of(aml::vector4i const& f) const -> aml::triangle_t
{
	return aml::triangle_t{verts_[f.x], verts_[f.y], verts_[f.z]};
}

auto voxelization_plugin_t::gfx_setup(shiny::context_ptr const& ctx2) -> void
{
	ctx = ctx2;
}

struct node_t
{
	node_t()
		: children_offset{}
		, brick_idx{}
	{}

	uint32 children_offset;
	uint32 brick_idx;
};

struct level_t
{
	uint64 current_node_offset = std::numeric_limits<uint64>::max();
	node_t storage;
};

#include <atma/memory.hpp>

auto voxelization_plugin_t::main_setup() -> void
{
	setup_voxelization();
	setup_svo();
	setup_rendering();
}

auto voxelization_plugin_t::setup_voxelization() -> void
{
	auto sf = shelf::file_t{"../../data/dragon.obj"};
	auto obj = obj_model_t{sf};

	// try for 128^3 grid
	auto const gridsize = 128;
	
	auto numbers = atma::vector<int>{1, 2, 3, 4, 5};
	auto numbers2 = atma::vector<int>{};
	numbers2.attach_buffer(numbers.detach_buffer());
	auto mem = atma::unique_memory_t{};
	mem = numbers2.detach_buffer();


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
				for (int z = tgridmin.z; z <= tgridmax.z; ++z)
				{
					auto aabc = aml::aabc_t{
						bbmin.x + x * voxelwidth + voxelwidth / 2.f,
						bbmin.y + y * voxelwidth + voxelwidth / 2.f,
						bbmin.z + z * voxelwidth + voxelwidth / 2.f,
						voxelwidth};

					if (aml::intersect_aabb_triangle(aabc, info))
						fragments.push_back({moxi::morton_encoding32(x, y, z)});
				}
			}
		}
	}

	std::sort(fragments.begin(), fragments.end());
	
	fragments.erase(
		std::unique(fragments.begin(), fragments.end()),
		fragments.end());

	std::vector<aml::vector4f> vertices;
	std::vector<uint32> indices;

	if (false)
	{
		uint64 m = 0;
		int fragidx = 0;
		for (auto const& frag : fragments)
		{
			uint x, y, z;
			moxi::morton_decoding32(frag.morton, x, y, z);

			auto scale = aml::matrix4f::scale(0.1f);
			auto translate = aml::matrix4f::translate(aml::vector4f{(float)x, (float)y, (float)z});
			auto cmp = translate * scale;

			for (int i = 0; i != 8; ++i)
			{
				auto v = aml::vector4f{cube_vertices()[i * 8 + 0], cube_vertices()[i * 8 + 1], cube_vertices()[i * 8 + 2], cube_vertices()[i * 8 + 3]};
				vertices.push_back(v * cmp);
			}

			for (auto idx = cube_indices(); idx != cube_indices() + 36; ++idx)
				indices.push_back(fragidx * 8 + *idx);
			++fragidx;
		}

		this->vb = shiny::create_vertex_buffer(this->ctx, shiny::resource_storage_t::immutable, dd_position(), (uint)vertices.size(), &vertices[0]);
		this->ib = shiny::create_index_buffer(ctx, shiny::resource_storage_t::immutable, shiny::index_format_t::index32, (uint)indices.size(), &indices[0]);
	}

	voxelbuf = shiny::make_buffer(ctx,
		shiny::resource_type_t::structured_buffer,
		shiny::resource_usage_t::shader_resource,
		shiny::resource_storage_t::persistant,
		shiny::buffer_dimensions_t::infer(fragments),
		shiny::buffer_data_t{fragments.data(), fragments.size()});

	voxelbuf_view = shiny::make_resource_view(voxelbuf,
		shiny::resource_view_type_t::input,
		shiny::element_format_t::unknown);

#if 1
	// load .obj triangles into vb/ib so that we can see the original mesh
	{
		auto mi = atma::unique_memory_t{sizeof(uint32) * obj.faces().size() * 3};
		auto tmi = atma::memory_view_t<decltype(mi), uint32>{mi};

		size_t i = 0;
		for (auto const& f : obj.faces())
		{
			tmi[i * 3 + 2] = f.x;
			tmi[i * 3 + 1] = f.y;
			tmi[i * 3 + 0] = f.z;
			++i;
		}

		vb = shiny::create_vertex_buffer(ctx, shiny::resource_storage_t::immutable, dd_position(), (uint)obj.vertices().size(), &obj.vertices()[0]);
		ib = shiny::create_index_buffer(ctx, shiny::resource_storage_t::immutable, shiny::index_format_t::index32, (uint)obj.faces().size() * 3, mi.begin());
	}
#endif

	auto f2 = atma::filesystem::file_t("../../shaders/gs_normal.hlsl");
	auto fm2 = f2.read_into_memory();
	gs = shiny::create_geometry_shader(ctx, fm2, false);
}

auto voxelization_plugin_t::setup_svo() -> void
{
	auto cs_from_file = [&](atma::string const& filename) -> shiny::compute_shader_ptr
	{
		auto f = atma::filesystem::file_t(filename.c_str());
		auto fmem = f.read_into_memory();
		return shiny::make_compute_shader(ctx, fmem.begin(), fmem.size());
	};

	cs_clear           = cs_from_file("../../shaders/sparse_octree_clear.hlsl");
	cs_mark            = cs_from_file("../../shaders/sparse_octree_mark_cs.hlsl");
	cs_allocate        = cs_from_file("../../shaders/sparse_octree_allocate.hlsl");
	cs_write_fragments = cs_from_file("../../shaders/sparse_octree_write_fragments.hlsl");


	auto const gridsize = 128;
	auto const brick_edge_size = 8u;
	auto const brick_morton_width = brick_edge_size * brick_edge_size * brick_edge_size;
	auto const levels_required = aml::log2(gridsize / brick_edge_size) + 1;
	auto const empty = std::numeric_limits<uint64>::max();

	auto nodes_required = (gridsize / brick_edge_size) * (gridsize / brick_edge_size) * (gridsize / brick_edge_size);
	nodes_required += nodes_required / 3; // size for levels
	auto const node_size = sizeof(node_t);
	auto const fullsize = nodes_required * node_size;


	auto cmem = atma::unique_memory_t{fullsize};
	memset(cmem.begin(), 0, fullsize);

	// constant-buffer
	struct cbuf
	{
		uint32 fragment_count;
		uint32 level;
		uint32 levels;
		uint32 pad;
	};

	auto cb = shiny::make_constant_buffer(ctx,
		sizeof(cbuf),
		nullptr);

	// atomic counters
	uint32 counters[2] ={1, 1};
	countbuf = shiny::make_buffer(ctx,
		shiny::resource_type_t::structured_buffer,
		shiny::resource_usage_t::shader_resource | shiny::resource_usage_t::unordered_access,
		shiny::resource_storage_t::persistant,
		shiny::buffer_dimensions_t{sizeof(uint32), 2},
		shiny::buffer_data_t{&counters, 2});

	countbuf_view = shiny::make_resource_view(countbuf,
		shiny::resource_view_type_t::compute,
		shiny::element_format_t::unknown);

	// node-cache
	nodecache = shiny::make_buffer(ctx,
		shiny::resource_type_t::structured_buffer,
		shiny::resource_usage_t::shader_resource | shiny::resource_usage_t::unordered_access,
		shiny::resource_storage_t::persistant,
		shiny::buffer_dimensions_t{node_size, nodes_required},
		shiny::buffer_data_t{cmem.begin(), nodes_required},
			shiny::gen_primary_input_view_t{},
			shiny::gen_primary_compute_view_t{});

	nodecache_view = shiny::make_resource_view(nodecache,
		shiny::resource_view_type_t::compute,
		shiny::element_format_t::unknown);

	// brick-cache
	brickcache = shiny::make_texture3d(ctx,
		shiny::resource_usage_t::shader_resource | shiny::resource_usage_t::unordered_access,
		shiny::resource_storage_t::persistant,
		shiny::texture3d_dimensions_t::cube(shiny::element_format_t::u32x2, 512, 1));

	brickcache_view = shiny::make_resource_view(brickcache,
		shiny::resource_view_type_t::compute,
		shiny::element_format_t::unknown);

	brickcache_input_view = shiny::make_resource_view(brickcache,
		shiny::resource_view_type_t::input,
		shiny::element_format_t::unknown);

	// staging buffer
	stb = shiny::make_buffer(ctx,
		shiny::resource_type_t::staging_buffer,
		shiny::resource_usage_mask_t::none,
		shiny::resource_storage_t::staging,
		shiny::buffer_dimensions_t{node_size, nodes_required},
		shiny::buffer_data_t{});



	namespace scc = shiny::compute_commands;

	// setup bound resources
	auto bound_constant_buffers = scc::bind_constant_buffers({{0, cb}});
	auto bound_input_views      = scc::bind_input_views({{0, voxelbuf_view}});
	auto bound_compute_views    = scc::bind_compute_views({{0, countbuf_view}, {1, nodecache_view}, {2, brickcache_view}});

#if 0
	// reset atomic-counter
	shiny::signal_compute(ctx,
		scc::bind_compute_views({{0, nodecache_view, 0}}),
		scc::dispatch(cs_clear, 0, 0, 0));
#endif

	// mark & allocate tiles in the node-cache
	for (int i = 0; i != levels_required; ++i)
	{
		ctx->signal_rs_constant_buffer_upload(cb, cbuf{
			(uint32)fragments.size(),
			(uint32)i,
			(uint32)levels_required
		});

		auto d = aml::pow(2, i);

		shiny::signal_compute(ctx,
			bound_constant_buffers,
			bound_input_views,
			bound_compute_views,
			scc::dispatch(cs_mark, (uint)fragments.size() / 64, 1, 1),
			scc::dispatch(cs_allocate, d, d, d));
	}

	// 1) mark nodes' brick_ids
	// 2) allocate bricks from the brick-cache
	// 3) write fragments into 3d texture
	{
		ctx->signal_rs_constant_buffer_upload(cb, cbuf{
			(uint32)fragments.size(),
			(uint32)levels_required,
			(uint32)levels_required
		});

		auto dim = aml::pow(2, (int)levels_required);
		shiny::signal_compute(ctx,
			bound_constant_buffers,
			bound_input_views,
			bound_compute_views,
			scc::dispatch(cs_mark, (uint)fragments.size() / 64, 1, 1),
			scc::dispatch(cs_allocate, dim, dim, dim),
			scc::dispatch(cs_write_fragments, (uint)fragments.size() / 64, 1, 1));
	}

#if _DEBUG
	ctx->signal_copy_buffer(stb, nodecache);
	ctx->signal_res_map(stb, 0, shiny::map_type_t::read, [](shiny::mapped_subresource_t& sr){
		int breakpoint = 4;
	});
#endif
}

auto voxelization_plugin_t::setup_rendering() -> void
{
	// vertex-buffer
	float vbd[] ={
		-1.f,  1.f, 0.f, 1.f,
		 1.f,  1.f, 0.f, 1.f,
		 1.f, -1.f, 0.f, 1.f,
		 1.f, -1.f, 0.f, 1.f,
		-1.f, -1.f, 0.f, 1.f,
		-1.f,  1.f, 0.f, 1.f,
	};

	auto dd = ctx->runtime().make_data_declaration({
		{"position", 0, shiny::element_format_t::f32x4}
	});

	vb_quad = shiny::create_vertex_buffer(ctx, shiny::resource_storage_t::persistant, dd, 8, vbd);


	{
		auto f = atma::filesystem::file_t("../shiny/x64/Debug/vs_voxels.cso");
		//auto f = atma::filesystem::file_t{"../../shaders/vs_voxels.hlsl"};
		auto fm = f.read_into_memory();
		vs_voxels = shiny::create_vertex_shader(ctx, dd, fm, true);
	}

	{
		auto f = atma::filesystem::file_t("../shiny/x64/Debug/ps_voxels.cso");
		auto fm = f.read_into_memory();
		fs_voxels = shiny::create_fragment_shader(ctx, fm, true);
	}
}

auto voxelization_plugin_t::gfx_ctx_draw(shiny::context_ptr const& ctx) -> void
{
	
}

auto voxelization_plugin_t::gfx_draw(shiny::scene_t& scene) -> void
{
	namespace sdc = shiny::draw_commands;

#if 0
	scene.draw(
		sdc::input_assembly_stage(dd_position(), vb, ib),
		sdc::vertex_stage(vs_flat(), shiny::bound_constant_buffers_t{
			{0, scene.scene_constant_buffer()}
		}),
		sdc::geometry_stage(gs),
		sdc::fragment_stage(fs_flat()));
#else
	scene.draw(
		sdc::input_assembly_stage(vb->data_declaration(), vb),
		sdc::vertex_stage(vs_voxels),
		sdc::fragment_stage(fs_voxels, shiny::bound_input_views_t{
			{0, nodecache->primary_input_view()},
			{1, brickcache_input_view}
		})
	);
#endif
}
