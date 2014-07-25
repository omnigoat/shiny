#include <dust/fragment_shader.hpp>

#include <dust/context.hpp>


using namespace dust;
using dust::pixel_shader_t;


auto dust::create_pixel_shader(context_ptr const& context, atma::unique_memory_t const& memory, bool precompiled, atma::string const& entrypoint) -> pixel_shader_ptr
{
	return pixel_shader_ptr(new pixel_shader_t(context, memory.begin(), memory.size(), precompiled, entrypoint));
}

pixel_shader_t::pixel_shader_t(context_ptr const& ctx, void const* data, size_t data_length, bool precompiled, atma::string const& entrypoint)
: context_(ctx)
{
	if (precompiled)
	{
		ATMA_ENSURE_IS(S_OK, D3DCreateBlob(data_length, d3d_blob_.assign()));
		memcpy(d3d_blob_->GetBufferPointer(), data, data_length);
	}
	else
	{
		platform::d3d_blob_ptr errors;
		ATMA_ENSURE_IS(S_OK, D3DCompile(data, data_length, nullptr, nullptr, nullptr, entrypoint.bytes_begin(), "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, d3d_blob_.assign(), errors.assign()));
		if (errors.get())
		{
			std::cout << (char*)errors->GetBufferPointer() << std::endl;
		}
	}


	auto const& device = context_->d3d_device();
	ATMA_ENSURE_IS(S_OK, device->CreatePixelShader(d3d_blob_->GetBufferPointer(), d3d_blob_->GetBufferSize(), nullptr, d3d_ps_.assign()));
}

