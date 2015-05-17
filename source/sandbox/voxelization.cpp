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
	auto sf = shelf::file_t{"../../data/dragon.obj"};
	auto obj = obj_model_t{sf};

	// try for 128^3 grid
	auto const gridsize = 128;
	
	auto numbers = atma::vector<int>{1, 2, 3, 4, 5};
	auto numbers2 = atma::vector<int>{};
	numbers2.attach_buffer(numbers.detach_buffer());
	auto mem = atma::unique_memory_t{};
	mem = numbers2.detach_buffer();

#if 1
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

#if 0
	auto it = fragments.begin() + 2000;
	auto i1 = it ->morton; uint i1x, i1y, i1z; moxi::morton_decoding32(i1, i1x, i1y, i1z);
	auto i2 = i1 / 8;      uint i2x, i2y, i2z; moxi::morton_decoding32(i2, i2x, i2y, i2z);
	auto m2 = (i2 & 0x7); uint m2x, m2y, m2z; moxi::morton_decoding32(m2, m2x, m2y, m2z);
	auto i3 = i2 / 8;      uint i3x, i3y, i3z; moxi::morton_decoding32(i3, i3x, i3y, i3z);
	auto i4 = i3 / 8;      uint i4x, i4y, i4z; moxi::morton_decoding32(i4, i4x, i4y, i4z);
	auto i5 = i4 / 8;      uint i5x, i5y, i5z; moxi::morton_decoding32(i5, i5x, i5y, i5z);
	auto i6 = i5 / 8;      uint i6x, i6y, i6z; moxi::morton_decoding32(i6, i6x, i6y, i6z);
#endif

	auto const brick_edge_size = 8u;
	auto const brick_morton_width = brick_edge_size * brick_edge_size * brick_edge_size;
	auto const levels_required = aml::log2(gridsize / brick_edge_size) + 1;
	auto const empty = std::numeric_limits<uint64>::max();

	auto nodes_required = (gridsize / brick_edge_size) * (gridsize / brick_edge_size) * (gridsize / brick_edge_size);
	nodes_required += nodes_required / 3; // size for levels
	auto const node_size = sizeof(node_t);
	auto const fullsize = nodes_required * node_size;



	voxelbuf = shiny::make_buffer(ctx,
		shiny::resource_type_t::structured_buffer,
		shiny::resource_usage_t::shader_resource | shiny::resource_usage_t::unordered_access,
		shiny::resource_storage_t::persistant,
		shiny::buffer_dimensions_t::infer(fragments),
		shiny::buffer_data_t{fragments});

	voxelbuf_view = shiny::make_resource_view(voxelbuf,
		shiny::resource_view_type_t::input,
		shiny::element_format_t::unknown);

	brickpool = shiny::make_texture3d(ctx,
		shiny::resource_usage_t::shader_resource | shiny::resource_usage_t::unordered_access,
		shiny::resource_storage_t::persistant,
		shiny::texture3d_dimensions_t::cube(shiny::element_format_t::u8x4, gridsize, 1));

#if 0
	nodecache = shiny::make_buffer(ctx,
		shiny::resource_type_t::structured_buffer,
		shiny::resource_usage_t::shader_resource | shiny::resource_usage_t::unordered_access,
		shiny::resource_storage_t::persistant,
		shiny::buffer_dimensions_t{node_size, nodes_required},
		shiny::buffer_data_t{},
			shiny::gen_default_read_write_view_t{});

	nodecache_view = shiny::make_resource_view(nodecache,
		shiny::resource_view_type_t::compute,
		shiny::element_format_t::unknown);
#endif

	stb = shiny::make_buffer(ctx,
		shiny::resource_type_t::staging_buffer,
		shiny::resource_usage_mask_t::none,
		shiny::resource_storage_t::staging,
		shiny::buffer_dimensions_t{node_size, nodes_required},
		shiny::buffer_data_t{});
#if 0
	



	auto fragments3d = std::vector<aml::vector4f>{};
	auto nodes = std::vector<node_t>{};

	// root node
	nodes.push_back(node_t{});


	//
	// sublevel_morton: the morton id of the item "below" us, for nodes > 0, this
	// is a node N-1, for node0, this is a brick id
	//
	std::function<node_t&(int, uint64)> publish_node = [&](int level_idx, uint64 sublevel_morton) -> node_t&
	{
		uint64 level_morton = sublevel_morton >> 3;
		auto self_morton = level_morton & 0x7;

		if (level_idx == levels_required - 1)
			return nodes[0];

		auto* parent = &publish_node(level_idx + 1, level_morton);

		if (parent->children_offset == 0)
		{
			parent->children_offset = (uint32)nodes.size();
			node_t nds[8];
			nodes.insert(nodes.end(), nds, nds + 8);
			parent = &publish_node(level_idx + 1, level_morton);
		}

		auto& node = nodes[parent->children_offset + self_morton];

		return node;
	};

	auto publish_brick = [&](fragments_t::const_iterator const& begin, fragments_t::const_iterator const& end) -> void
	{
		auto brick_morton = begin->morton / brick_morton_width;
		auto& node = publish_node(0, brick_morton);

		node.brick_idx = (uint32)brick_morton;
	};

	
	// node: node in octree, has 2x2x2 children, or a 
	// node0: lowest-level node in our octree
	// brick: an 8x8x8 collection of fragments
	for (auto fragment_iter = fragments.begin(); fragment_iter != fragments.end(); )
	{
		auto brick_morton = fragment_iter->morton / brick_morton_width;

		// traverse all fragments in the brick
		auto brick_end = fragment_iter;
		while (brick_end != fragments.end() && brick_end->morton / brick_morton_width == brick_morton)
			++brick_end;

		publish_brick(fragment_iter, brick_end);
		fragment_iter = brick_end;
	}
#endif

	this->vb = shiny::create_vertex_buffer(this->ctx, shiny::resource_storage_t::immutable, dd_position(), (uint)vertices.size(), &vertices[0]);
	this->ib = shiny::create_index_buffer(ctx, shiny::resource_storage_t::immutable, shiny::index_format_t::index32, (uint)indices.size(), &indices[0]);

#else
	uint32* mi = new uint32[obj.faces().size() * 3];
	size_t i = 0;
	for (auto const& x : obj.faces()) {
		//memcpy(mi + i * 3, &x.x, 3 * sizeof(uint32));
		mi[i * 3 + 2] = x.x;
		mi[i * 3 + 1] = x.y;
		mi[i * 3 + 0] = x.z;
		++i;
	}


	vb = shiny::create_vertex_buffer(ctx, shiny::resource_storage_t::immutable, dd_position(), obj.vertices().size(), &obj.vertices()[0]);
	ib = shiny::create_index_buffer(ctx, shiny::resource_storage_t::immutable, shiny::index_format_t::index32, obj.faces().size() * 3, mi);

#endif

	auto f2 = atma::filesystem::file_t("../../shaders/gs_normal.hlsl");
	auto fm2 = f2.read_into_memory();
	gs = shiny::create_geometry_shader(ctx, fm2, false);

	auto cs_from_file = [&](atma::string const& filename) -> shiny::compute_shader_ptr
	{
		auto f = atma::filesystem::file_t(filename.c_str());
		auto fmem = f.read_into_memory();
		return shiny::make_compute_shader(ctx, fmem.begin(), fmem.size());
	};

	cs_clear = cs_from_file("../../shaders/sparse_octree_clear.hlsl");
	cs_mark = cs_from_file("../../shaders/sparse_octree_mark_cs.hlsl");
	cs_allocate = cs_from_file("../../shaders/sparse_octree_allocate.hlsl");
	cs_write_fragments = cs_from_file("../../shaders/sparse_octree_write_fragments.hlsl");
}

auto voxelization_plugin_t::gfx_ctx_draw(shiny::context_ptr const& ctx) -> void
{

	struct cs_cbuf
	{
		uint32 fragment_count;
		uint32 level;
		uint32 levels;
		uint32 pad;
	};


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

	// atomic counters
	uint32 counters[2] = {1, 1};
	auto countbuf = shiny::make_buffer(ctx,
		shiny::resource_type_t::structured_buffer,
		shiny::resource_usage_t::shader_resource | shiny::resource_usage_t::unordered_access,
		shiny::resource_storage_t::persistant,
		shiny::buffer_dimensions_t{sizeof(counters), 1},
		shiny::buffer_data_t{&counters, 1});

	auto countbuf_view = shiny::make_resource_view(countbuf,
		shiny::resource_view_type_t::compute,
		shiny::element_format_t::unknown);

	// node-cache
	auto nodecache = shiny::make_buffer(ctx,
		shiny::resource_type_t::structured_buffer,
		shiny::resource_usage_t::shader_resource | shiny::resource_usage_t::unordered_access,
		shiny::resource_storage_t::persistant,
		shiny::buffer_dimensions_t{node_size, nodes_required},
		shiny::buffer_data_t{cmem.begin(), nodes_required});

	auto nodecache_view = shiny::make_resource_view(nodecache,
		shiny::resource_view_type_t::compute,
		shiny::element_format_t::unknown);

	// brick-cache
	auto brickcache = shiny::make_texture3d(ctx,
		shiny::resource_usage_t::shader_resource | shiny::resource_usage_t::unordered_access,
		shiny::resource_storage_t::persistant,
		shiny::texture3d_dimensions_t::cube(shiny::element_format_t::u32, 512, 1));

	auto brickcache_view = shiny::make_resource_view(brickcache,
		shiny::resource_view_type_t::compute,
		shiny::element_format_t::unknown);

	namespace scc = shiny::compute_commands;

	struct whatever { uint32 n; uint32 pad[3]; };

	// reset atomic-counter
	shiny::signal_compute(ctx,
		scc::bind_compute_views({{0, nodecache_view, 0}}),
		scc::dispatch(cs_clear, 0, 0, 0));

	for (int i = 0; i != levels_required; ++i)
	{
		auto cb = shiny::make_constant_buffer(ctx, cs_cbuf{
			(uint32)fragments.size(),
			(uint32)i,
			(uint32)levels_required
		});

		// pow(2, i - j);
		auto dim = 1 << i;

		shiny::signal_compute(ctx,
			scc::bind_constant_buffers({{0, cb}}),
			scc::bind_input_views({{0, voxelbuf_view}}),
			scc::bind_compute_views({{0, nodecache_view}}),
			scc::dispatch(cs_mark, (uint)fragments.size() / 64, 1, 1));

		ctx->signal_copy_buffer(stb, nodecache);

		ctx->signal_res_map(stb, 0, shiny::map_type_t::read, [](shiny::mapped_subresource_t& sr){
			int breakpoint = 4;
		});

		shiny::signal_compute(ctx,
			scc::bind_constant_buffers({{0, cb}}),
			scc::bind_compute_views({{0, countbuf_view}, {0, nodecache_view}}),
			scc::dispatch(cs_allocate, dim, dim, dim));
	}

	auto cb = shiny::make_constant_buffer(ctx, cs_cbuf{
		(uint32)fragments.size(),
		(uint32)levels_required,
		(uint32)levels_required
	});

	// write fragments into 3d texture
	shiny::signal_compute(ctx,
		scc::bind_constant_buffers({{0, cb}}),
		scc::bind_input_views({{0, voxelbuf_view}}),
		scc::bind_compute_views({{0, countbuf_view}, {1, nodecache_view}, {2, brickcache_view}}),
		scc::dispatch(cs_write_fragments, (uint)fragments.size() / 64, 1, 1));

	
	ctx->signal_copy_buffer(stb, nodecache);

	ctx->signal_res_map(stb, 0, shiny::map_type_t::read, [](shiny::mapped_subresource_t& sr){
		int breakpoint = 4;
	});
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
#endif
}
