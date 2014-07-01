#include <dust/vertex_shader.hpp>

#include <dust/context.hpp>


using namespace dust;
using dust::vertex_shader_t;


auto dust::create_vertex_shader(context_ptr const& context, atma::unique_memory_t const& memory, bool precompiled, atma::string const& entrypoint) -> vertex_shader_ptr
{
	return vertex_shader_ptr(new vertex_shader_t(context, memory.begin(), memory.size(), precompiled, entrypoint));
}

vertex_shader_t::vertex_shader_t(context_ptr const& ctx, void const* data, size_t data_length, bool precompiled, atma::string const& entrypoint)
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
		ATMA_ENSURE_IS(S_OK, D3DCompile(data, data_length, nullptr, nullptr, nullptr, entrypoint.bytes_begin(), "vs_4_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, d3d_blob_.assign(), errors.assign()));
		if (errors)
		{
			auto errs = (char*)errors->GetBufferPointer();
		}
	}
	
	
	auto const& device = context_->d3d_device();
	ATMA_ENSURE_IS(S_OK, device->CreateVertexShader(d3d_blob_->GetBufferPointer(), d3d_blob_->GetBufferSize(), nullptr, d3d_vs_.assign()));
}


