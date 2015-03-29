#include <sandbox/voxelization.hpp>

#include <shiny/context.hpp>
#include <shiny/scene.hpp>
#include <shiny/draw.hpp>

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

auto voxelization_plugin_t::main_setup() -> void
{
	auto sf = shelf::file_t{"../../data/dragon.obj"};
	auto obj = obj_model_t{sf};

	// try for 128^3 grid
	auto const gridsize = 128;

	
#if 1
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
		//auto info = aml::aabb_triangle_intersection_info_t{aml::vector4f{voxelwidth, voxelwidth, voxelwidth}, t.v0, t.v1, t.v2};

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

					if (aml::intersect_aabb_triangle(aabc, t))
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
#endif


	uint32* mi = new uint32[obj.faces().size() * 3];
	size_t i = 0;
	for (auto const& x : obj.faces()) {
		//memcpy(mi + i * 3, &x.x, 3 * sizeof(uint32));
		mi[i * 3 + 0] = x.x;
		mi[i * 3 + 1] = x.y;
		mi[i * 3 + 2] = x.z;
		++i;
	}

	vb = shiny::create_vertex_buffer(ctx, shiny::buffer_usage_t::immutable, dd_position(), (uint)vertices.size(), &vertices[0]);
	ib = shiny::create_index_buffer(ctx, shiny::buffer_usage_t::immutable, shiny::index_format_t::index32, (uint)indices.size(), &indices[0]);


	auto f2 = atma::filesystem::file_t("../../shaders/gs_normal.hlsl");
	auto fm2 = f2.read_into_memory();
	//auto gs = shiny::create_geometry_shader(ctx, fm2, false);


	//vb = shiny::create_vertex_buffer(ctx, shiny::buffer_usage_t::immutable, dd_position(), obj.vertices().size(), &obj.vertices()[0]);
	//ib = shiny::create_index_buffer(ctx, shiny::buffer_usage_t::immutable, sizeof(uint32), obj.faces().size() * 3, mi);
}

auto voxelization_plugin_t::gfx_draw(shiny::scene_t& scene) -> void
{
	namespace sdc = shiny::draw_commands;
	
	shiny::signal_draw(ctx, scene.draw_batch()
		, sdc::input_assembly_stage(dd_position(), vb, ib)
		, sdc::vertex_stage(vs_flat(), shiny::bound_constant_buffers_t{{0, scene.scene_buffer()}})
		, sdc::fragment_stage(fs_flat())
		);

#if 0
	shiny::signal_draw(scene.draw_sink()
		, sdc::input_stage(dd_position(), vb, ib)
		, sdc::geometry_stage(gs)
		, sdc::vertex_stage(vs)
		, sdc::fragment_stage(fs)
		);
#endif
	//scene.signal_draw(ib, dd_position(), vb, vs_flat(), fs_flat());
	//scene.signal_draw(ib, dd_position(), vb, fs_flat(), vs_flat(), gs_flat())
}
