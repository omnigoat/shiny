#include <sandbox/voxelization.hpp>

#include <shiny/context.hpp>
#include <shiny/scene.hpp>
#include <shiny/draw.hpp>
#include <shiny/generic_buffer.hpp>

#include <shelf/file.hpp>

#include <shox/morton.hpp>

#include <atma/math/matrix4f.hpp>
#include <atma/math/vector4f.hpp>
#include <atma/math/vector4i.hpp>
#include <atma/math/triangle.hpp>
#include <atma/math/aabc.hpp>
#include <atma/math/intersection.hpp>

#include <atma/filesystem/file.hpp>


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

struct voxel_t
{
	uint64 morton;
};

auto operator < (voxel_t const& lhs, voxel_t const& rhs) -> bool
{
	return lhs.morton < rhs.morton;
}

auto operator == (voxel_t const& lhs, voxel_t const& rhs) -> bool
{
	return lhs.morton == rhs.morton;
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


auto voxelization_plugin_t::main_setup() -> void
{
	auto sf = shelf::file_t{"../../data/dragon.obj"};
	auto obj = obj_model_t{sf};

	// try for 128^3 grid
	auto const gridsize = 128;

	
#if 1
	using fragments_t = std::vector<voxel_t>;
	auto fragments = fragments_t{};

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
						fragments.push_back({moxi::morton_encoding(x, y, z)});
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
		moxi::morton_decoding(frag.morton, x, y, z);

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

	auto const block_edge_size = uint64(8);
	auto const brick_morton_width = block_edge_size * block_edge_size * block_edge_size;
	auto const levels_required = aml::log2(gridsize / block_edge_size) + 1;
	auto const empty = std::numeric_limits<uint64>::max();


#if 0
	// load voxel data into GPU. in the future, when the fragments are generated GPU-side, this
	// won't be necessary anymore.
	//ctx->make_buffer(
		//shiny::resource_usage_t::standard,
		//shiny::buffer_type_t::generic_buffer,
		//shiny::buffer_access_t::cpu_none_gpu_readwrite,

		
	auto voxelbuf = ctx->make_generic_buffer(shiny::resource_usage_t::unordered_access, shiny::buffer_usage_t::persistant, sizeof(uint64), (uint)fragments.size(), &fragments[0], (uint)fragments.size());
	//auto voxelbufview = ctx->make_resource_view(voxelbuf, shiny::element_format_t::u64, )
	// one node is a 32-bit value for the brick, 32-bit value for the children-offset
	auto const nodepool_size = 2*(gridsize*gridsize*gridsize) / (brick_morton_width);

	auto nodepool = ctx->make_generic_buffer(shiny::resource_usage_t::unordered_access, shiny::buffer_usage_t::persistant, sizeof(uint32)*2, nodepool_size, nullptr, 0);

	//atma::thread::engine_t::queue_t::batch_t batch;
	namespace sdc = shiny::draw_commands;
	//shiny::signal_draw(ctx, sdc::input_assembly_stage()
	//ctx->signal_cs_upload_shader_resource(shiny::view_type_t::read_only, )
	
#if 0
	shiny::signal_compute(ctx,
		shiny::bound_resources_t{
			{0, voxelbuf->d3d_srv()},
			{1, nodepool->d3d_srv()}
		});
#endif

#endif

	


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

	this->vb = shiny::create_vertex_buffer(this->ctx, shiny::buffer_usage_t::immutable, dd_position(), (uint)vertices.size(), &vertices[0]);
	this->ib = shiny::create_index_buffer(ctx, shiny::buffer_usage_t::immutable, shiny::index_format_t::index32, (uint)indices.size(), &indices[0]);

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


	vb = shiny::create_vertex_buffer(ctx, shiny::buffer_usage_t::immutable, dd_position(), obj.vertices().size(), &obj.vertices()[0]);
	ib = shiny::create_index_buffer(ctx, shiny::buffer_usage_t::immutable, shiny::index_format_t::index32, obj.faces().size() * 3, mi);

#endif

	auto f2 = atma::filesystem::file_t("../../shaders/gs_normal.hlsl");
	auto fm2 = f2.read_into_memory();
	gs = shiny::create_geometry_shader(ctx, fm2, false);


}

auto voxelization_plugin_t::gfx_draw(shiny::scene_t& scene) -> void
{
	namespace sdc = shiny::draw_commands;

	scene.draw(
		sdc::input_assembly_stage(dd_position(), vb, ib),
		sdc::vertex_stage(vs_flat(), shiny::bound_constant_buffers_t{
			{0, scene.scene_constant_buffer()}
		}),
		sdc::geometry_stage(gs),
		sdc::fragment_stage(fs_flat()));
}
