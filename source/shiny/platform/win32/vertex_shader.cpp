#include <shiny/vertex_shader.hpp>

#include <shiny/context.hpp>
#include <shiny/logging.hpp>

#include <rose/file.hpp>


using namespace shiny;
using shiny::vertex_shader_t;


auto shiny::create_vertex_shader(context_ptr const& context, atma::string const& path, bool precompiled, atma::string const& entrypoint) -> vertex_shader_ptr
{
	auto f = rose::file_t{path};
	auto m = rose::read_into_memory(f);
	return vertex_shader_ptr::make(context, path, m.begin(), m.size(), precompiled, entrypoint);
}

vertex_shader_t::vertex_shader_t(context_ptr const& ctx, atma::string const& path, void const* data, size_t data_length, bool precompiled, atma::string const& entrypoint)
	: context_{ctx}
	, path_{path}
{
	if (precompiled)
	{
		ATMA_ENSURE_IS(S_OK, D3DCreateBlob(data_length, d3d_blob_.assign()));
		memcpy(d3d_blob_->GetBufferPointer(), data, data_length);
	}
	else
	{
		platform::d3d_blob_ptr errors;
		ATMA_ENSURE_IS(S_OK, D3DCompile(data, data_length, path.c_str(), nullptr, nullptr, entrypoint.raw_begin(), "vs_5_0", D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, d3d_blob_.assign(), errors.assign()));
		if (errors)
		{
			SHINY_ERROR("vertex-shader errors:\n", (char*)errors->GetBufferPointer());
		}
	}

	// create vertex-shader
	auto const& device = context_->d3d_device();
	auto size =  d3d_blob_->GetBufferSize();
	ATMA_ENSURE_IS(S_OK, device->CreateVertexShader(d3d_blob_->GetBufferPointer(), size, nullptr, d3d_vs_.assign()));
}


