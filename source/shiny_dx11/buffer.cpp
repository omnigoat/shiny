#include <shiny_dx11/buffer.hpp>

#include <shiny_dx11/dxgid3d_convert.hpp>
#include <shiny/renderer.hpp>

#include <atma/assert.hpp>


using namespace shiny;
using shiny_dx11::buffer_t;


shiny_dx11::buffer_t::buffer_t(
	renderer_ptr const& rndr,
	resource_type_t rt, resource_usage_mask_t ru, resource_storage_t rs,
	buffer_dimensions_t const& bdm, buffer_data_t const& bdt)
{
	// no zero-size buffers
	ATMA_ASSERT(bdm.stride * bdm.count > 0);
	
	ATMA_ASSERT(!(ru & resource_usage_t::unordered_access) || rs == resource_storage_t::persistant,
		"unordered-access buffers must be placed into persistant storage");


	auto resource_size = bdm.stride * bdm.count;

	// fixup default element-count, figure out size of data
	auto data_element_count = bdt.size;
	if (data_element_count == 0)
		data_element_count = bdm.count;
	auto data_size = bdm.stride * data_element_count;
	data_size = aml::alignby((uint)data_size, 16);

	// determine resource-storage and cpu-access
	auto d3d_rs = D3D11_USAGE();
	auto d3d_ca = D3D11_CPU_ACCESS_FLAG();
	switch (rs)
	{
		case resource_storage_t::immutable:
			d3d_rs = D3D11_USAGE_IMMUTABLE;
			break;

		case resource_storage_t::persistant:
			d3d_rs = D3D11_USAGE_DEFAULT;
			break;

		case resource_storage_t::temporary:
		case resource_storage_t::transient:
		case resource_storage_t::constant:
			d3d_rs = D3D11_USAGE_DYNAMIC;
			d3d_ca = D3D11_CPU_ACCESS_WRITE;
			break;

		case resource_storage_t::staging:
			d3d_rs = D3D11_USAGE_STAGING;
			d3d_ca = D3D11_CPU_ACCESS_READ;
			break;

		default:
			ATMA_HALT("resource-storage is ill-formed");
			break;
	}


	// structured buffer?
	auto misc_flags = D3D11_RESOURCE_MISC_FLAG{};
	if (rt == resource_type_t::structured_buffer)
	{
		(uint&)misc_flags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	}


	// determine binding
	auto binding = platform::d3dbind_of(rt);
	if (ru & resource_usage_t::render_target)
		(uint&)binding |= D3D11_BIND_RENDER_TARGET;
	if (ru & resource_usage_t::depth_stencil)
		(uint&)binding |= D3D11_BIND_DEPTH_STENCIL;
	if (ru & resource_usage_t::shader_resource)
		(uint&)binding |= D3D11_BIND_SHADER_RESOURCE;
	if (ru & resource_usage_t::unordered_access)
		(uint&)binding |= D3D11_BIND_UNORDERED_ACCESS;


	// create buffer
	auto rs_size = aml::alignby((uint)resource_size, 16);
	auto buffer_desc = D3D11_BUFFER_DESC{(UINT)rs_size, d3d_rs, (UINT)binding, (UINT)d3d_ca, (UINT)misc_flags, (UINT)bdm.stride};
	switch (rs)
	{
		case resource_storage_t::immutable:
		{
			ATMA_ASSERT(bdt.data, "immutable buffers require data upon initialisation");
			//ATMA_ASSERT(resource_size() == data_size, "immutable buffer: allocation size != data size");
			ATMA_ASSERT(d3d_ca == 0, "immutable buffer with cpu access? silly.");

			auto d3d_data = D3D11_SUBRESOURCE_DATA{bdt.data, (UINT)data_size, 1};
			ATMA_ENSURE_IS(S_OK, rndr->d3d_device()->CreateBuffer(&buffer_desc, &d3d_data, d3d_buffer_.assign()));
			break;
		}

		// constant-buffers must be 16byte sized/aligned
		case resource_storage_t::constant:
			buffer_desc.ByteWidth += (16 - buffer_desc.ByteWidth % 16) % 16;
			data_size += (16 - data_size % 16) % 16;
			// fallthrough!

		case resource_storage_t::persistant:
		case resource_storage_t::temporary:
		case resource_storage_t::transient:
		case resource_storage_t::staging:
		{
			if (bdt.data)
			{
				auto d3d_data = D3D11_SUBRESOURCE_DATA{bdt.data, (UINT)data_size, 1};
				ATMA_ENSURE_IS(S_OK, rndr->d3d_device()->CreateBuffer(&buffer_desc, &d3d_data, d3d_buffer_.assign()));
			}
			else
			{
				ATMA_ENSURE_IS(S_OK, rndr->d3d_device()->CreateBuffer(&buffer_desc, nullptr, d3d_buffer_.assign()));
			}

			break;
		}

		default:
		{
			ATMA_HALT("not supported");
			break;
		}
	}
}

