#include <shiny/fragment_shader.hpp>

#include <shiny/context.hpp>
#include <shiny/logging.hpp>

#include <lion/streams.hpp>

#include <rose/file.hpp>


using namespace shiny;
using shiny::fragment_shader_t;


auto shiny::create_fragment_shader(context_ptr const& context, atma::string const& path, bool precompiled, atma::string const& entrypoint) -> fragment_shader_ptr
{
	auto f = rose::file_t{path};
	auto m = rose::read_into_memory(f);
	return fragment_shader_ptr::make(context, path, m.begin(), m.size(), precompiled, entrypoint);
}

auto shiny::create_fragment_shader(context_ptr const& context, atma::string const& path, atma::input_bytestream_ptr const& stream, bool precompiled, atma::string const& entrypoint) -> fragment_shader_ptr
{
	auto m = lion::read_all(stream);
	return fragment_shader_ptr::make(context, path, m.begin(), m.size(), precompiled, entrypoint);
}


fragment_shader_t::fragment_shader_t(context_ptr const& ctx, atma::string const& path, void const* data, size_t data_length, bool precompiled, atma::string const& entrypoint)
	: context_(ctx)
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
		ATMA_ENSURE_IS(S_OK, D3DCompile(data, data_length, path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entrypoint.c_str(), "ps_5_0", D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, d3d_blob_.assign(), errors.assign()));
		if (errors)
		{
			SHINY_ERROR("fragment-shader errors:\n", (char*)errors->GetBufferPointer());
		}
	}

	auto const& device = context_->d3d_device();
	ATMA_ENSURE_IS(S_OK, device->CreatePixelShader(d3d_blob_->GetBufferPointer(), d3d_blob_->GetBufferSize(), nullptr, d3d_fs_.assign()));
}
