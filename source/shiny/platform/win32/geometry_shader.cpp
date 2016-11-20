#include <shiny/geometry_shader.hpp>

#include <shiny/context.hpp>
#include <shiny/logging.hpp>

#include <rose/file.hpp>


using namespace shiny;
using shiny::geometry_shader_t;


auto shiny::create_geometry_shader(renderer_ptr const& context, atma::string const& path, bool precompiled, atma::string const& entrypoint) -> geometry_shader_ptr
{
	auto f = rose::file_t{path};
	auto m = rose::read_into_memory(f);
	return geometry_shader_ptr::make(context, path, m.begin(), m.size(), precompiled, entrypoint);
}

geometry_shader_t::geometry_shader_t(renderer_ptr const& rndr, atma::string const& path, void const* data, size_t data_length, bool precompiled, atma::string const& entrypoint)
	: rndr_{rndr}
	, path_{path}
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
		ATMA_ENSURE_IS(S_OK, D3DCompile(data, data_length, path.c_str(), nullptr, nullptr, entrypoint.raw_begin(), "gs_5_0", D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, d3d_blob_.assign(), errors.assign()));
		if (errors)
		{
			SHINY_ERROR("geometry-shader compilation errors:\n", (char*)errors->GetBufferPointer());
		}
	}


	// create geometry-shader
	auto const& device = rndr_->d3d_device();
	ATMA_ENSURE_IS(S_OK, device->CreateGeometryShader(d3d_blob_->GetBufferPointer(), d3d_blob_->GetBufferSize(), nullptr, d3d_gs_.assign()));
}


