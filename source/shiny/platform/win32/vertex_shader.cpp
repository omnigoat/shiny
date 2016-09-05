#include <shiny/vertex_shader.hpp>

#include <shiny/platform/win32/dxgid3d_convert.hpp>

#include <shiny/data_declaration.hpp>
#include <shiny/context.hpp>

using namespace shiny;
using shiny::vertex_shader_t;


auto shiny::create_vertex_shader(context_ptr const& context, atma::unique_memory_t const& memory, bool precompiled, atma::string const& entrypoint) -> vertex_shader_ptr
{
	return vertex_shader_ptr(new vertex_shader_t(context, memory.begin(), memory.size(), precompiled, entrypoint));
}

vertex_shader_t::vertex_shader_t(context_ptr const& ctx, void const* data, size_t data_length, bool precompiled, atma::string const& entrypoint)
	: context_(ctx)
{
	// create blob
	if (precompiled)
	{
		ATMA_ENSURE_IS(S_OK, D3DCreateBlob(data_length, d3d_blob_.assign()));
		memcpy(d3d_blob_->GetBufferPointer(), data, data_length);
#if 0
		wchar_t buf[1234]{};
		auto s = mbstowcs(buf, entrypoint.c_str(), entrypoint.raw_size());
		buf[s] = '\0';
		auto hr = D3DReadFileToBlob(buf, d3d_blob_.assign());
#endif
	}
	else
	{
		platform::d3d_blob_ptr errors;
		char* errs = nullptr;
		ATMA_ENSURE_IS(S_OK, D3DCompile(data, data_length, nullptr, nullptr, nullptr, entrypoint.raw_begin(), "vs_5_0", D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, d3d_blob_.assign(), errors.assign()));
		if (errors)
		{
			errs = (char*)errors->GetBufferPointer();
		}
	}

	// create vertex-shader
	auto const& device = context_->d3d_device();
	auto size =  d3d_blob_->GetBufferSize();
	ATMA_ENSURE_IS(S_OK, device->CreateVertexShader(d3d_blob_->GetBufferPointer(), size, nullptr, d3d_vs_.assign()));
}


