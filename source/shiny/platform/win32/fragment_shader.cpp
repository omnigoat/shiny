#include <shiny/fragment_shader.hpp>

#include <shiny/renderer.hpp>
#include <shiny/logging.hpp>

#include <lion/streams.hpp>

#include <rose/file.hpp>


using namespace shiny;
using shiny::fragment_shader_t;


auto fragment_shader_t::make(renderer_ptr const& context, lion::path_t const& path, bool precompiled, atma::string const& entrypoint) -> fragment_shader_ptr
{
	auto f = rose::file_t{path.string()};
	auto m = rose::read_into_memory(f);
	return fragment_shader_ptr::make(context, path, m.begin(), m.size(), precompiled, entrypoint);
}

auto fragment_shader_t::make(shiny::renderer_ptr const& rndr, lion::path_t const& path, void const* data, size_t data_size, bool precompiled, atma::string const& entrypoint) -> fragment_shader_ptr
{
	return fragment_shader_ptr::make(rndr, path, data, data_size, precompiled, entrypoint);
}


fragment_shader_t::fragment_shader_t(renderer_ptr const& rndr, lion::path_t const& path, platform::d3d_blob_ptr const& blob, platform::d3d_fragment_shader_ptr const& shader)
	: asset_t{path}
	, rndr_{rndr}
	, d3d_blob_{blob}
	, d3d_fs_{shader}
{}


auto atma::intrusive_ptr_make<shiny::fragment_shader_t>::make(renderer_ptr const& rndr, lion::path_t const& path, void const* data, size_t data_size, bool precompiled, atma::string const& entrypoint) -> fragment_shader_t*
{
	using namespace shiny;
	namespace platform = shiny::platform;

	platform::d3d_blob_ptr d3d_blob;
	platform::d3d_fragment_shader_ptr d3d_vs;

	if (precompiled)
	{
		ATMA_ENSURE_IS(S_OK, D3DCreateBlob(data_size, d3d_blob.assign()));
		memcpy(d3d_blob->GetBufferPointer(), data, data_size);
	}
	else
	{
		platform::d3d_blob_ptr errors;
		if (S_OK != D3DCompile(data, data_size, path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entrypoint.raw_begin(), "ps_5_0", D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, d3d_blob.assign(), errors.assign()))
		{
			SHINY_ERROR("fragment-shader errors:\n", (char*)errors->GetBufferPointer());
			return nullptr;
		}
		else if (errors)
		{
			SHINY_WARN("fragment-shader warnings:\n", (char*)errors->GetBufferPointer());
		}
	}

	// create fragment-shader
	auto const& device = rndr->d3d_device();
	if (S_OK == device->CreatePixelShader(d3d_blob->GetBufferPointer(), d3d_blob->GetBufferSize(), nullptr, d3d_vs.assign()))
	{
		return new fragment_shader_t{rndr, path, d3d_blob, d3d_vs};
	}
	else
	{
		return nullptr;
	}
}

