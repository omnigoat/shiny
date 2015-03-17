#include <shiny/vertex_shader.hpp>

#include <shiny/platform/win32/dxgid3d_convert.hpp>

#include <shiny/data_declaration.hpp>
#include <shiny/context.hpp>


using namespace shiny;
using shiny::vertex_shader_t;


auto shiny::create_vertex_shader(context_ptr const& context, data_declaration_t const* vd, atma::unique_memory_t const& memory, bool precompiled, atma::string const& entrypoint) -> vertex_shader_ptr
{
	return vertex_shader_ptr(new vertex_shader_t(context, vd, memory.begin(), memory.size(), precompiled, entrypoint));
}

vertex_shader_t::vertex_shader_t(context_ptr const& ctx, data_declaration_t const* vd, void const* data, size_t data_length, bool precompiled, atma::string const& entrypoint)
: context_(ctx), data_declaration_(vd)
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
		ATMA_ENSURE_IS(S_OK, D3DCompile(data, data_length, nullptr, nullptr, nullptr, entrypoint.raw_begin(), "vs_4_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, d3d_blob_.assign(), errors.assign()));
		if (errors)
		{
			auto errs = (char*)errors->GetBufferPointer();
		}
	}


	// create vertex-shader
	auto const& device = context_->d3d_device();
	ATMA_ENSURE_IS(S_OK, device->CreateVertexShader(d3d_blob_->GetBufferPointer(), d3d_blob_->GetBufferSize(), nullptr, d3d_vs_.assign()));


	// create input-layout
	uint offset = 0;
	std::vector<D3D11_INPUT_ELEMENT_DESC> d3d_elements;
	for (auto const& x : data_declaration_->streams())
	{
		d3d_elements.push_back({
			x.semantic().c_str(),
			0, platform::dxgi_format_of(x.element_format()), 0, offset, D3D11_INPUT_PER_VERTEX_DATA, 0
		});

		offset += x.size();
	}

	ATMA_ENSURE_IS(S_OK, context_->d3d_device()->CreateInputLayout(&d3d_elements[0], (uint)d3d_elements.size(),
		d3d_blob_->GetBufferPointer(), d3d_blob_->GetBufferSize(), d3d_input_layout_.assign()));
}


