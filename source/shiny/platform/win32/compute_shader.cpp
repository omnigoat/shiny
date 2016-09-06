#include <shiny/compute_shader.hpp>

#include <shiny/context.hpp>
#include <shiny/logging.hpp>

#include <rose/file.hpp>

using namespace shiny;
using shiny::compute_shader_t;


auto shiny::create_compute_shader(context_ptr const& context, atma::string const& path, bool precompiled, atma::string const& entrypoint) -> compute_shader_ptr
{
	auto f = rose::file_t{path};
	auto m = rose::read_into_memory(f);
	return compute_shader_ptr::make(context, path, m.begin(), m.size(), precompiled, entrypoint);
}

compute_shader_t::compute_shader_t(context_ptr const& context, atma::string const& path, void const* data, size_t data_size, bool precompiled, atma::string const& entrypoint)
	: context_(context)
{
	if (precompiled)
	{
		ATMA_ENSURE_IS(S_OK, D3DCreateBlob(data_size, d3d_blob_.assign()));
		memcpy(d3d_blob_->GetBufferPointer(), data, data_size);
	}
	else
	{
		platform::d3d_blob_ptr errors;
		D3DCompile(data, data_size, path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entrypoint.c_str(), "cs_5_0", D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, d3d_blob_.assign(), errors.assign());
		if (errors.get())
		{
			SHINY_ERROR("compute-shader errors:\n", (char*)errors->GetBufferPointer());
			ATMA_HALT("bad compute-shader");
		}
	}


	auto const& device = context_->d3d_device();
	ATMA_ENSURE_IS(S_OK, device->CreateComputeShader(d3d_blob_->GetBufferPointer(), d3d_blob_->GetBufferSize(), nullptr, d3d_cs_.assign()));
}

