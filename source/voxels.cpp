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
#include <dust/vertex_shader.hpp>
#include <dust/pixel_shader.hpp>
#include <dust/context.hpp>

#include <atma/filesystem/file.hpp>
#include <atma/unique_memory.hpp>


// vertex declaration
dust::vertex_declaration_t const* vd = nullptr;

// vertex-buffer
float vbd[] = {
	-1.f,  1.f,  1.f, 1.f,
	 1.f,  1.f, -1.f, 1.f,
	 1.f, -1.f,  1.f, 1.f,
	 1.f, -1.f,  1.f, 1.f,
	-1.f, -1.f,  1.f, 1.f,
	-1.f,  1.f,  1.f, 1.f,
};
auto vb = dust::vertex_buffer_ptr();

// shaders
auto vs = dust::vertex_shader_ptr();
auto ps = dust::pixel_shader_ptr();

void voxels_init(dust::context_ptr const& ctx)
{
	vd = dust::get_vertex_declaration({
		{dust::vertex_stream_semantic_t::position, 0, dust::element_format_t::f32x4},
		{dust::vertex_stream_semantic_t::color, 0, dust::element_format_t::f32x4}
	});

	vb = dust::create_vertex_buffer(ctx, dust::buffer_usage_t::immutable, vd, 8, vbd);
	
	auto f = atma::filesystem::file_t("../shaders/vs_voxels.hlsl");
	auto fm = f.read_into_memory();
	vs = dust::create_vertex_shader(ctx, fm, false, "vs_main");
	ps = dust::create_pixel_shader(ctx, fm, false, "ps_main");

}

void voxels_render(dust::context_ptr const& ctx)
{
	ctx->signal_draw(vd, vb, vs, ps);


}