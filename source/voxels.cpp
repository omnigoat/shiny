#if 0


#include <dust/vertex_buffer.hpp>
#include <dust/vertex_declaration.hpp>
#include <dust/constant_buffer.hpp>
#include <dust/index_buffer.hpp>
#include <dust/camera.hpp>
#include <dust/scene.hpp>
#include <dust/texture2d.hpp>
#include <dust/compute_shader.hpp>
#include <dust/shader_resource2d.hpp>
#include <dust/texture3d.hpp>
#include <dust/platform/win32/generic_buffer.hpp>


// vertex declaration
auto vd = dust::vertex_declaration_t();

// vertex-buffer
float vbd[] ={
	0.5f, 0.5f, 0.5f, 1.f, 1.f, 0.f, 0.f, 1.f,
	0.5f, 0.5f, -0.5f, 1.f, 0.f, 1.f, 0.f, 1.f,
	0.5f, -0.5f, 0.5f, 1.f, 0.f, 0.f, 1.f, 1.f,
	0.5f, -0.5f, -0.5f, 1.f, 1.f, 1.f, 0.f, 1.f,
	-0.5f, 0.5f, 0.5f, 1.f, 1.f, 0.f, 1.f, 1.f,
	-0.5f, 0.5f, -0.5f, 1.f, 0.f, 1.f, 1.f, 1.f,
	-0.5f, -0.5f, 0.5f, 1.f, 1.f, 1.f, 1.f, 1.f,
	-0.5f, -0.5f, -0.5f, 1.f, 1.f, 0.f, 0.f, 1.f,
};
auto vb = dust::vertex_buffer_ptr();

// index-buffer
uint16 ibd[] = {
	4, 5, 7, 7, 6, 4, // -x plane
	0, 2, 3, 3, 1, 0, // +x plane
	2, 6, 7, 7, 3, 2, // -y plane
	0, 1, 5, 5, 4, 0, // +y plane
	5, 1, 3, 3, 7, 5, // -z plane
	6, 2, 0, 0, 4, 6, // +z plane
};
auto ib = dust::index_buffer_ptr();


void voxels_init(dust::context_ptr const& ctx)
{
	auto vd = dust::vertex_declaration_t(ctx, vs, {
		{dust::vertex_stream_t::usage_t::position, 0, dust::vertex_stream_t::element_type_t::float32, 4},
		{dust::vertex_stream_t::usage_t::color, 0, dust::vertex_stream_t::element_type_t::float32, 4}
	});

	vb = dust::create_vertex_buffer(ctx, dust::buffer_usage_t::immutable, vd, 8, vbd);
	ib = dust::create_index_buffer(ctx, dust::buffer_usage_t::immutable, 16, 36, ibd);
}

#endif
