#include <sandbox/voxelization.hpp>

#include <shiny/renderer.hpp>
#include <shiny/scene.hpp>
#include <shiny/draw.hpp>
#include <shiny/compute.hpp>
#include <shiny/resource_view.hpp>
#include <shiny/texture3d.hpp>
#include <shiny/compute_shader.hpp>
#include <shiny/constant_buffer.hpp>
#include <shiny/camera.hpp>
#include <shiny/texture2d.hpp>
#include <shiny/draw_target.hpp>

#include <moxi/morton.hpp>

#include <rose/file.hpp>

#include <atma/math/matrix4f.hpp>
#include <atma/math/vector4f.hpp>
#include <atma/math/vector4i.hpp>
#include <atma/math/triangle.hpp>
#include <atma/math/aabc.hpp>
#include <atma/math/intersection.hpp>

#include <atma/vector.hpp>
#include <atma/memory.hpp>


using namespace sandbox;
using sandbox::voxelization_plugin_t;





struct obj_model_t
{
	using verts_t = atma::vector<aml::vector4f>;
	using faces_t = atma::vector<aml::vector4i>;

	obj_model_t(rose::file_t&);

	auto vertices() const -> verts_t const& { return verts_; }
	auto faces() const -> faces_t const& { return faces_; }
	auto triangle_of(aml::vector4i const&) const -> aml::triangle_t;

private:
	verts_t verts_;
	faces_t faces_;
};

obj_model_t::obj_model_t(rose::file_t& file)
{
	rose::for_each_line<256>(file, 128, [&](char const* str, size_t size)
	{
		switch (str[0])
		{
			case 'v': {
				float x, y, z;
				sscanf(str, "v %f %f %f", &x, &y, &z);
				verts_.push_back(aml::point4f(x, y, z)); // reflect z because RH -> LH
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

auto load_geometry_shader(shiny::renderer_ptr const& rndr, rose::path_t const& path, lion::input_stream_ptr const& stream) -> lion::asset_ptr
{
	bool precompiled = path.extension() == "cso";
	auto f = rose::file_t{path.c_str()};
	auto m = rose::read_into_memory(f);
	auto r = rndr->make_geometry_shader(path, m, "main", precompiled);
	return r;
}

auto load_vertex_shader(shiny::renderer_ptr const& rndr, rose::path_t const& path, lion::input_stream_ptr const& stream) -> lion::asset_ptr
{
	bool precompiled = path.extension() == "cso";
	auto f = rose::file_t{path.c_str()};
	auto m = rose::read_into_memory(f);
	auto r = rndr->make_vertex_shader(path, m, "main", precompiled);
	return r;
}

auto load_fragment_shader(shiny::renderer_ptr const& rndr, rose::path_t const& path, lion::input_stream_ptr const& stream) -> lion::asset_ptr
{
	bool precompiled = path.extension() == "cso";
	auto f = rose::file_t{path.c_str()};
	auto m = rose::read_into_memory(f);
	auto r = rndr->make_fragment_shader(path, m, "main", precompiled);
	return r;
}

voxelization_plugin_t::voxelization_plugin_t(application_t* app)
	: plugin_t{app}
	, library_{&app->vfs()}
{
	library_.register_asset_type<shiny::fragment_shader_t>({
		lion::asset_pattern_t{"/res/shaders/(f|p)s_.+\\.hlsl", atma::curry(&load_fragment_shader, std::ref(rndr)), atma::curry(&load_fragment_shader, std::ref(rndr))}
	});

	library_.register_asset_type<shiny::vertex_shader_t>({
		lion::asset_pattern_t{"/res/shaders/vs_.+\\.hlsl", atma::curry(&load_vertex_shader, std::ref(rndr)), atma::curry(&load_vertex_shader, std::ref(rndr))}
	});

	library_.register_asset_type<shiny::geometry_shader_t>({
		lion::asset_pattern_t{"/res/shaders/gs_.+\\.hlsl", atma::curry(&load_geometry_shader, std::ref(rndr)), atma::curry(&load_geometry_shader, std::ref(rndr))}
	});
}

auto voxelization_plugin_t::gfx_setup(shiny::renderer_ptr const& ctx2) -> void
{
	rndr = ctx2;
}



struct level_t
{
	uint64 current_node_offset = std::numeric_limits<uint64>::max();
	node_t storage;
};


auto voxelization_plugin_t::main_setup() -> void
{
	setup_voxelization();
	setup_svo();
	setup_rendering();
}

auto const gridsize = 512;

auto fit_linear_dispatch_to_group(uint xs, uint ys, uint zs, uint s) -> std::tuple<uint, uint, uint>
{
	// s = 10,000
	// x,y,z = 8
	// id = x * 64 + y * 8 + z
	// ------
	//  [1, 1, 1] -> [8, 8, 8]
	//  [8, 1, 1] -> [64, 8, 8]

	uint x = std::max(1u, (uint)std::ceil((float)s / (xs * ys * zs)));
	uint y = 1;
	uint z = 1;

	return {x, y, z};
}

auto voxelization_plugin_t::setup_voxelization() -> void
{
	auto sf = rose::file_t{"../data/dragon.obj"};
	auto obj = obj_model_t{sf};
	
	{
		auto mi = atma::unique_memory_t{sizeof(uint32) * obj.faces().size() * 3};
		auto tmi = atma::memory_view_t<uint32>{mi};

		size_t i = 0;
		for (auto const& f : obj.faces())
		{
			tmi[i * 3 + 0] = f.x;
			tmi[i * 3 + 1] = f.y;
			tmi[i * 3 + 2] = f.z;
			++i;
		}

		vb = rndr->make_vertex_buffer(shiny::resource_storage_t::immutable, dd_position(), obj.vertices());
		ib = rndr->make_index_buffer(shiny::resource_storage_t::immutable, shiny::format_t::u32, (uint)obj.faces().size() * 3, mi.begin());
	}

	

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


	// expand to a cube
	auto gridwidth = std::max({tri_dp.x, tri_dp.y, tri_dp.z});
	auto voxelwidth = (gridwidth / gridsize);
	//auto voxel_halfwidth = std::sqrtf(voxelwidth * voxelwidth) / 0.5f;


	auto render_target = rndr->make_texture2d(
		shiny::resource_usage_t::render_target,
		shiny::resource_storage_t::persistant,
		shiny::format_t::nu8x4,
		gridsize, gridsize, 1);

	auto render_target_view = shiny::make_resource_view(render_target,
		shiny::resource_view_type_t::render_target,
		shiny::format_t::nu8x4);

	auto draw_target = shiny::draw_target_t{
		render_target_view};

	shiny::scene_t voxelization_scene{rndr, draw_target};



#if 0
	for (auto const& f : obj.faces())
	{
		auto t = obj.triangle_of(f);

		// expand triangle edges by half voxel-width
		auto tmid = (t.v0 + t.v1 + t.v2) / 3.f;
		t.v0 = tmid + (t.v0 - tmid) * (1.f + voxel_halfwidth);
		t.v1 = tmid + (t.v1 - tmid) * (1.f + voxel_halfwidth);
		t.v2 = tmid + (t.v2 - tmid) * (1.f + voxel_halfwidth);

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

		for (int z = tgridmin.z; z <= tgridmax.z; ++z)
		{
			for (int y = tgridmin.y; y <= tgridmax.y; ++y)
			{
				for (int x = tgridmin.x; x <= tgridmax.x; ++x)
				{
					auto aabc = aml::aabc_t{
						bbmin.x + x * voxelwidth + voxelwidth / 2.f,
						bbmin.y + y * voxelwidth + voxelwidth / 2.f,
						bbmin.z + z * voxelwidth + voxelwidth / 2.f,
						voxelwidth};

					// bounding-box test
					if (intersect_aabbs(aabc, t.aabb()) || aml::intersect_aabb_triangle(aabc, info))
						fragments.push_back({moxi::morton_encoding32(x, y, z)});
				}
			}
		}
	}

	std::sort(fragments.begin(), fragments.end());
	
	fragments.erase(
		std::unique(fragments.begin(), fragments.end()),
		fragments.end());

#if 0
	voxelbuf = shiny::make_buffer(rndr,
		shiny::resource_type_t::structured_buffer,
		shiny::resource_usage_t::shader_resource,
		shiny::resource_storage_t::persistant,
		shiny::buffer_dimensions_t::infer(fragments),
		shiny::buffer_data_t{fragments.data(), fragments.size()});

	voxelbuf_view = shiny::make_resource_view(voxelbuf,
		shiny::resource_view_type_t::input,
		shiny::format_t::unknown);
#endif
#endif

#if 1
	// load .obj triangles into vb/ib so that we can see the original mesh
	{
		auto mi = atma::unique_memory_t{sizeof(uint32) * obj.faces().size() * 3};
		auto tmi = atma::memory_view_t<uint32>{mi};

		size_t i = 0;
		for (auto const& f : obj.faces())
		{
			tmi[i * 3 + 0] = f.x;
			tmi[i * 3 + 1] = f.y;
			tmi[i * 3 + 2] = f.z;
			++i;
		}

		vb = rndr->make_vertex_buffer(shiny::resource_storage_t::immutable, dd_position(), obj.vertices());
		ib = rndr->make_index_buffer(shiny::resource_storage_t::immutable, shiny::format_t::u32, (uint)obj.faces().size() * 3, mi.begin());
	}
#endif



	//gs_voxelize = shiny::create_geometry_shader(rndr, "resources/published/shaders/gs_voxelization.hlsl", false);
	gs_voxelize = library_.load_as<shiny::geometry_shader_t>("/res/shaders/gs_voxelization.hlsl");
	vs_voxelize = library_.load_as<shiny::vertex_shader_t>("/res/shaders/vs_voxelize.hlsl");
	fs_voxelize = library_.load_as<shiny::fragment_shader_t>("/res/shaders/fs_voxelize.hlsl");
	ATMA_ASSERT(vs_voxelize);
	ATMA_ASSERT(fs_voxelize);

	// fragments buffer (64mb)
	fragments_buf = rndr->make_buffer(
		shiny::resource_type_t::structured_buffer,
		shiny::resource_usage_t::shader_resource | shiny::resource_usage_t::unordered_access,
		shiny::resource_storage_t::persistant,
		shiny::buffer_dimensions_t{sizeof(uint32), 16 * 1024 * 1024},
		shiny::buffer_data_t{});

	fragments_view = shiny::make_resource_view(fragments_buf,
		shiny::resource_view_type_t::compute,
		shiny::format_t::unknown);

	fragments_srv_view = shiny::make_resource_view(fragments_buf,
		shiny::resource_view_type_t::read,
		shiny::format_t::unknown);

	{
		namespace sdc = shiny::draw_commands;


		uint32 counters[2] = {0, 0};
		auto countbuf = rndr->make_buffer(
			shiny::resource_type_t::structured_buffer,
			shiny::resource_usage_t::shader_resource | shiny::resource_usage_t::unordered_access,
			shiny::resource_storage_t::persistant,
			shiny::buffer_dimensions_t{sizeof(uint32), 2},
			shiny::buffer_data_t{&counters, 2});

		auto countbuf_view = shiny::make_resource_view(countbuf,
			shiny::resource_view_type_t::compute,
			shiny::format_t::unknown);

		auto mid = (bbmin + bbmax) / 2.f;
		mid.w = gridwidth * 0.5f;
		auto cb = rndr->make_constant_buffer_for(mid);

		aml::vector4f dimensions{(float)gridsize, (float)gridsize, (float)gridsize, 0.f};
		auto cb2 = rndr->make_constant_buffer_for(dimensions);

		voxelization_scene.draw
		(
			shiny::input_assembly_stage_t{
				vb->data_declaration(),
				vb, ib},

			shiny::vertex_stage_t{
				vs_voxelize,
				shiny::bound_constant_buffers_t{
					{0, cb}
				}
			},

			shiny::geometry_stage_t{
				gs_voxelize,
				shiny::bound_constant_buffers_t{
					{0, cb2}
				}
			},

			shiny::fragment_stage_t{
				fs_voxelize,
				shiny::bound_constant_buffers_t{
					{0, cb},
					{1, cb2}
				},

				shiny::bound_compute_views_t{
					{0, countbuf_view},
					{1, fragments_view}
				}
			},

			shiny::output_merger_stage_t{
				shiny::depth_stencil_state_t::off
			}
		);

		rndr->signal_draw_scene(voxelization_scene);

		auto counter_scratch = rndr->make_buffer(
			shiny::resource_type_t::staging_buffer,
			shiny::resource_usage_mask_t::none,
			shiny::resource_storage_t::staging,
			shiny::buffer_dimensions_t{sizeof(uint32), 2},
			shiny::buffer_data_t{});

		rndr->signal_copy_buffer(counter_scratch, countbuf);

		rndr->signal_rs_map(counter_scratch, 0, shiny::map_type_t::read, [&](shiny::mapped_subresource_t const& ms) {
			fragments_count = *(uint*)ms.data;
		});

		rndr->signal_block();
		
		std::cout << "fragments_count: " << fragments_count << std::endl;

#if 0

		auto scratch = shiny::make_buffer(rndr,
			shiny::resource_type_t::staging_buffer,
			shiny::resource_usage_mask_t::none,
			shiny::resource_storage_t::staging,
			shiny::buffer_dimensions_t{sizeof(uint32), 5 * 1024 * 1024},
			shiny::buffer_data_t{});

		rndr->signal_copy_buffer(scratch, fragments_buf);

		shiny::buffer_t::aligned_data_t<uint> gpu_fragments(fragments_count);
		rndr->signal_rs_map(scratch, 0, shiny::map_type_t::read, [&](shiny::mapped_subresource_t const& ms) {
			memcpy(gpu_fragments.data(), ms.data, sizeof(uint) * fragments_count);
		});

		rndr->signal_block();
#endif

#if 0
		std::sort(gpu_fragments.begin(), gpu_fragments.end());

		gpu_fragments.erase(
			std::unique(gpu_fragments.begin(), gpu_fragments.end()),
			gpu_fragments.end());
#endif

#if 0
		voxelbuf = shiny::make_buffer(rndr,
			shiny::resource_type_t::structured_buffer,
			shiny::resource_usage_t::shader_resource,
			shiny::resource_storage_t::persistant,
			shiny::buffer_dimensions_t::infer(gpu_fragments),
			shiny::buffer_data_t{gpu_fragments.data(), gpu_fragments.size()});

		voxelbuf_view = shiny::make_resource_view(voxelbuf,
			shiny::resource_view_type_t::input,
			shiny::format_t::unknown);
#endif
	}
}

auto voxelization_plugin_t::setup_svo() -> void
{
	cs_clear           = shiny::create_compute_shader(rndr, "resources/published/shaders/sparse_octree_clear.hlsl", false);
	cs_mark            = shiny::create_compute_shader(rndr, "resources/published/shaders/sparse_octree_mark.hlsl", false);
	cs_allocate        = shiny::create_compute_shader(rndr, "resources/published/shaders/sparse_octree_allocate.hlsl", false);
	cs_write_fragments = shiny::create_compute_shader(rndr, "resources/published/shaders/sparse_octree_write_fragments.hlsl", false);


	auto const brick_edge_size = 8u;
	auto const brick_morton_width = brick_edge_size * brick_edge_size * brick_edge_size;
	auto const levels_required = aml::log2(gridsize / brick_edge_size);
	auto const empty = std::numeric_limits<uint64>::max();

	auto nodes_required = (gridsize / brick_edge_size) * (gridsize / brick_edge_size) * (gridsize / brick_edge_size);
	nodes_required += nodes_required / 3; // size for levels
	auto const tiles_required = (7 + nodes_required) / 8;
	auto const node_size = sizeof(node_t);
	auto const tile_size = node_size * 8;
	auto const fullsize = tiles_required * tile_size;


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

	auto cb = rndr->make_constant_buffer(nullptr, sizeof(cbuf));

	// atomic counters
	uint32 counters[2] ={1, 1};
	countbuf = rndr->make_buffer(
		shiny::resource_type_t::structured_buffer,
		shiny::resource_usage_t::shader_resource | shiny::resource_usage_t::unordered_access,
		shiny::resource_storage_t::persistant,
		shiny::buffer_dimensions_t{sizeof(uint32), 2},
		shiny::buffer_data_t{&counters, 2});

	countbuf_view = shiny::make_resource_view(countbuf,
		shiny::resource_view_type_t::compute,
		shiny::format_t::unknown);

	// node-cache
	nodecache = rndr->make_buffer(
		shiny::resource_type_t::structured_buffer,
		shiny::resource_usage_t::shader_resource | shiny::resource_usage_t::unordered_access,
		shiny::resource_storage_t::persistant,
		shiny::buffer_dimensions_t{tile_size, tiles_required},
		shiny::buffer_data_t{cmem.begin(), tiles_required});

	nodecache_view = shiny::make_resource_view(nodecache,
		shiny::resource_view_type_t::compute,
		shiny::format_t::unknown);

	nodecache_input_view = shiny::make_resource_view(nodecache,
		shiny::resource_view_type_t::read,
		shiny::format_t::unknown);

	auto const brickcachesize = 400;

	// brick-cache
	brickcache = rndr->make_texture3d(
		shiny::resource_usage_t::shader_resource | shiny::resource_usage_t::unordered_access,
		shiny::resource_storage_t::persistant,
		shiny::format_t::f32x2, brickcachesize, brickcachesize, brickcachesize, 1);

	brickcache_view = shiny::make_resource_view(brickcache,
		shiny::resource_view_type_t::compute,
		shiny::format_t::unknown);

	brickcache_input_view = shiny::make_resource_view(brickcache,
		shiny::resource_view_type_t::read,
		shiny::format_t::f32x2);

#if 0
	auto brick_readback = shiny::make_texture3d(rndr,
		shiny::resource_usage_mask_t::none,
		shiny::resource_storage_t::staging,
		shiny::texture3d_dimensions_t::cube(shiny::format_t::f32x2, brickcache->width() / 2, 1));

	// staging buffer
	stb = shiny::make_buffer(rndr,
		shiny::resource_type_t::staging_buffer,
		shiny::resource_usage_mask_t::none,
		shiny::resource_storage_t::staging,
		shiny::buffer_dimensions_t{tile_size, tiles_required},
		shiny::buffer_data_t{});
#endif


	// setup bound resources
	auto bound_constant_buffers = shiny::bound_constant_buffers_t{{0, cb}};
	auto bound_input_views      = shiny::bound_input_views_t{{0, fragments_srv_view}};

	auto bound_compute_views = shiny::bound_compute_views_t{
		{0, countbuf_view},
		{1, nodecache_view},
		{2, brickcache_view}};

	auto [kx, ky, kz] = fit_linear_dispatch_to_group(8, 8, 8, fragments_count);

	auto cc = rndr->make_compute_context(
		bound_constant_buffers,
		bound_input_views,
		bound_compute_views);

	// mark & allocate tiles in the node-cache
	for (int i = 0; i != levels_required; ++i)
	{
		rndr->signal_rs_upload(cb, cbuf{
			(uint32)fragments_count,
			(uint32)levels_required,
			(uint32)i});

		cc.signal_dispatch(cs_mark, kx, ky, kz)
		  .signal_dispatch(cs_allocate, aml::pow(2, i))
		  ;
	}

	// 1) mark nodes' brick_ids
	// 2) allocate bricks from the brick-cache
	// 3) write fragments into 3d texture
	{
		rndr->signal_rs_upload(cb, cbuf{
			(uint32)fragments_count,
			(uint32)levels_required,
			(uint32)levels_required});

		cc.signal_dispatch(cs_mark, kx, ky, kz)
		  .signal_dispatch(cs_allocate, aml::pow(2, (int)levels_required))
		  .signal_dispatch(cs_write_fragments, kx, ky, kz)
		  ;
	}
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

	auto dd = rndr->runtime().make_data_declaration({
		{"position", 0, shiny::format_t::f32x4}
	});

	vb_quad = rndr->make_vertex_buffer(shiny::resource_storage_t::persistant, dd, 8, vbd, 8);

	vs_voxels = library_.load_as<shiny::vertex_shader_t>("/res/shaders/vs_voxels.hlsl");
	fs_voxels = library_.load_as<shiny::fragment_shader_t>("/res/shaders/ps_voxels.hlsl");
}

auto voxelization_plugin_t::gfx_ctx_draw(shiny::renderer_ptr const& rndr) -> void
{
	
}

auto voxelization_plugin_t::gfx_draw(shiny::scene_t& scene) -> void
{
	namespace sdc = shiny::draw_commands;

	struct brick_cb_t
	{
		aml::vector4f position;
		float x, y;
		uint32 brickcache_width, brick_size;
	};

	auto cb = scene.renderer()->make_constant_buffer_for(brick_cb_t{
		scene.camera().position(),
		scene.camera().yaw(), scene.camera().pitch(),
		(uint32)brickcache->width() / 8, 8
	});

	auto bound_buffers = shiny::bound_constant_buffers_t{
		{0, scene.scene_constant_buffer()},
		{2, cb}};

	scene.draw(
		shiny::input_assembly_stage_t{
			vb_quad->data_declaration(),
			vb_quad},

		shiny::vertex_stage_t{
			vs_voxels,
			bound_buffers
		},

		shiny::fragment_stage_t{
			fs_voxels, 
			bound_buffers,
		
			shiny::bound_input_views_t{
				{0, nodecache_input_view},
				{1, brickcache_input_view}
			}
		}
	);

}
