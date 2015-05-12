#include <shiny/compute_shader.hpp>

#include <shiny/context.hpp>


using namespace shiny;
using shiny::compute_shader_t;


auto shiny::make_compute_shader(context_ptr const& context, void const* data, size_t data_size) -> compute_shader_ptr
{
	return atma::make_intrusive_ptr<compute_shader_t>(context, data, data_size);
}

compute_shader_t::compute_shader_t(context_ptr const& context, void const* data, size_t data_size)
: context_(context)
{
	if (false)
	{
		ATMA_ENSURE_IS(S_OK, D3DCreateBlob(data_size, d3d_blob_.assign()));
		memcpy(d3d_blob_->GetBufferPointer(), data, data_size);
	}
	else
	{
		platform::d3d_blob_ptr errors;
		ATMA_ENSURE_IS(S_OK, D3DCompile(data, data_size, nullptr, nullptr, nullptr, "main", "cs_5_0", D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, d3d_blob_.assign(), errors.assign()));
		if (errors.get())
		{
			std::cout << (char*)errors->GetBufferPointer() << std::endl;
		}
	}


	auto const& device = context_->d3d_device();
	ATMA_ENSURE_IS(S_OK, device->CreateComputeShader(d3d_blob_->GetBufferPointer(), d3d_blob_->GetBufferSize(), nullptr, d3d_cs_.assign()));
}

