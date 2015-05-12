#include <shiny/geometry_shader.hpp>

#include <shiny/platform/win32/dxgid3d_convert.hpp>

#include <shiny/geometry_declaration.hpp>
#include <shiny/context.hpp>


using namespace shiny;
using shiny::geometry_shader_t;


auto shiny::create_geometry_shader(context_ptr const& context, atma::unique_memory_t const& memory, bool precompiled, atma::string const& entrypoint) -> geometry_shader_ptr
{
	return geometry_shader_ptr(new geometry_shader_t(context, memory.begin(), memory.size(), precompiled, entrypoint));
}

geometry_shader_t::geometry_shader_t(context_ptr const& ctx, void const* data, size_t data_length, bool precompiled, atma::string const& entrypoint)
	: context_(ctx)
{
	// create blob
	if (precompiled)
	{
		ATMA_ENSURE_IS(S_OK, D3DCreateBlob(data_length, d3d_blob_.assign()));
		memcpy(d3d_blob_->GetBufferPointer(), data, data_length);
	}
	else
	{
		platform::d3d_blob_ptr errors;
		ATMA_ENSURE_IS(S_OK, D3DCompile(data, data_length, nullptr, nullptr, nullptr, entrypoint.raw_begin(), "gs_5_0", D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, d3d_blob_.assign(), errors.assign()));
		if (errors)
		{
			auto errs = (char*)errors->GetBufferPointer();
		}
	}


	// create geometry-shader
	auto const& device = context_->d3d_device();
	ATMA_ENSURE_IS(S_OK, device->CreateGeometryShader(d3d_blob_->GetBufferPointer(), d3d_blob_->GetBufferSize(), nullptr, d3d_gs_.assign()));
}


