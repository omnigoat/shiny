#include <shiny/vertex_shader.hpp>

#include <shiny/context.hpp>
#include <shiny/logging.hpp>

#include <rose/file.hpp>


using namespace shiny;
using shiny::vertex_shader_t;


auto vertex_shader_t::make(renderer_ptr const& ctx, lion::path_t const& path, bool precompiled, atma::string const& entrypoint) -> vertex_shader_ptr
{
	auto f = rose::file_t{path.string()};
	auto m = rose::read_into_memory(f);
	return vertex_shader_ptr::make(ctx, path, m.begin(), m.size(), precompiled, entrypoint);
}

auto vertex_shader_t::make(shiny::renderer_ptr const& ctx, lion::path_t const& path, void const* data, size_t data_size, bool precompiled, atma::string const& entrypoint) -> vertex_shader_ptr
{
	return vertex_shader_ptr::make(ctx, path, data, data_size, precompiled, entrypoint);
}


vertex_shader_t::vertex_shader_t(renderer_ptr const& ctx, lion::path_t const& path, platform::d3d_blob_ptr const& blob, platform::d3d_vertex_shader_ptr const& shader)
	: asset_t{path}
	, context_{ctx}
	, d3d_blob_{blob}
	, d3d_vs_{shader}
{}


auto atma::intrusive_ptr_make<shiny::vertex_shader_t>::make(renderer_ptr const& ctx, lion::path_t const& path, void const* data, size_t data_size, bool precompiled, atma::string const& entrypoint) -> vertex_shader_t*
{
	using namespace shiny;
	namespace platform = shiny::platform;

	platform::d3d_blob_ptr d3d_blob;
	platform::d3d_vertex_shader_ptr d3d_vs;

	if (precompiled)
	{
		ATMA_ENSURE_IS(S_OK, D3DCreateBlob(data_size, d3d_blob.assign()));
		memcpy(d3d_blob->GetBufferPointer(), data, data_size);
	}
	else
	{
		platform::d3d_blob_ptr errors;
		ATMA_ENSURE_IS(S_OK, D3DCompile(data, data_size, path.c_str(), nullptr, nullptr, entrypoint.raw_begin(), "vs_5_0", D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, d3d_blob.assign(), errors.assign()));
		if (errors)
		{
			SHINY_ERROR("vertex-shader errors:\n", (char*)errors->GetBufferPointer());
		}
	}

	// create vertex-shader
	auto const& device = ctx->d3d_device();
	if (S_OK == device->CreateVertexShader(d3d_blob->GetBufferPointer(), d3d_blob->GetBufferSize(), nullptr, d3d_vs.assign()))
	{
		return new vertex_shader_t{ctx, path, d3d_blob, d3d_vs};
	}
	else
	{
		return nullptr;
	}
}

