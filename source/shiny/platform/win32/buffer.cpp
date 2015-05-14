#include <shiny/platform/win32/buffer.hpp>

#include <shiny/platform/win32/dxgid3d_convert.hpp>
#include <shiny/context.hpp>

#include <atma/assert.hpp>


using namespace shiny;
using shiny::buffer_t;


buffer_t::buffer_t(context_ptr const& ctx,
			resource_type_t type, resource_usage_mask_t rs, resource_storage_t usage,
			buffer_dimensions_t const& bdm, buffer_data_t const& bdt)
	: resource_t(ctx, type, rs, bdm.stride, bdm.count)
	, buffer_usage_(usage)
{
	// no zero-size buffers
	ATMA_ASSERT(resource_size());
	// unordered-access buffers must be placed into persistant storage
	ATMA_ASSERT(!(rs & resource_usage_t::unordered_access) || (usage == resource_storage_t::persistant || usage == resource_storage_t::persistant_shadowed));



	// fixup default element-count, figure out size of data
	auto data_element_count = bdt.count;
	if (data_element_count == 0)
		data_element_count = bdm.count;
	auto data_size = bdm.stride * data_element_count;


	// determine buffer-usage and cpu-access
	auto d3d_bu = D3D11_USAGE();
	auto d3d_ca = D3D11_CPU_ACCESS_FLAG();
	switch (buffer_usage_)
	{
		case resource_storage_t::immutable:
			d3d_bu = D3D11_USAGE_IMMUTABLE;
			break;

		case resource_storage_t::persistant:
		case resource_storage_t::persistant_shadowed:
			d3d_bu = D3D11_USAGE_DEFAULT;
			break;

		case resource_storage_t::temporary:
		case resource_storage_t::temporary_shadowed:
		case resource_storage_t::transient:
		case resource_storage_t::transient_shadowed:
		case resource_storage_t::constant:
		case resource_storage_t::constant_shadowed:
			d3d_bu = D3D11_USAGE_DYNAMIC;
			d3d_ca = D3D11_CPU_ACCESS_WRITE;
			break;

		case resource_storage_t::staging:
			d3d_bu = D3D11_USAGE_STAGING;
			d3d_ca = D3D11_CPU_ACCESS_READ;
			break;

		default:
			ATMA_HALT("buffer usage is ill-formed");
			break;
	}


	// structured buffer?
	auto misc_flags = D3D11_RESOURCE_MISC_FLAG{};
	if (resource_type() == resource_type_t::structured_buffer)
	{
		(uint&)misc_flags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	}


	bool shadowed =
		buffer_usage_ == resource_storage_t::persistant_shadowed ||
		buffer_usage_ == resource_storage_t::temporary_shadowed ||
		buffer_usage_ == resource_storage_t::transient_shadowed ||
		buffer_usage_ == resource_storage_t::constant_shadowed
		;


	// determine binding
	auto binding = platform::d3dbind_of(resource_type());
	if (resource_usage() & resource_usage_t::render_target)
		(uint&)binding |= D3D11_BIND_RENDER_TARGET;
	if (resource_usage() & resource_usage_t::depth_stencil)
		(uint&)binding |= D3D11_BIND_DEPTH_STENCIL;
	if (resource_usage() & resource_usage_t::shader_resource)
		(uint&)binding |= D3D11_BIND_SHADER_RESOURCE;
	if (resource_usage() & resource_usage_t::unordered_access) {
		(uint&)binding |= D3D11_BIND_UNORDERED_ACCESS;
	}

	// cpu-access for unordered-access-buffers
	//if (resource_usage() & resource_usage_t::unordered_access)
		//(uint&)d3d_ca |= D3D11_CPU_ACCESS_READ;

	// create buffer
	auto buffer_desc = D3D11_BUFFER_DESC{(UINT)resource_size(), d3d_bu, binding, d3d_ca, misc_flags, (UINT)bdm.stride};
	switch (buffer_usage_)
	{
		case resource_storage_t::immutable:
		{
			ATMA_ASSERT_MSG(bdt.data, "immutable buffers require data upon initialisation");
			ATMA_ASSERT_MSG(resource_size() == data_size, "immutable buffer: allocation size != data size");
			ATMA_ASSERT_MSG(d3d_ca == 0, "immutable buffer with cpu access? silly.");

			auto d3d_data = D3D11_SUBRESOURCE_DATA{bdt.data, (UINT)data_size, 1};
			ATMA_ENSURE_IS(S_OK, context()->d3d_device()->CreateBuffer(&buffer_desc, &d3d_data, d3d_buffer_.assign()));
			break;
		}

		// constant-buffers must be 16byte sized
		case resource_storage_t::constant:
			data_size = ((data_size / 16) + 1) * 16;
			//buffer_desc.StructureByteStride = (UINT)((bdm.stride / 16) + 1) * 16;
			// fallthrough

		case resource_storage_t::persistant:
		case resource_storage_t::temporary:
		case resource_storage_t::transient:
		case resource_storage_t::staging:
		{
			if (bdt.data)
			{
				auto d3d_data = D3D11_SUBRESOURCE_DATA{bdt.data, (UINT)data_size, 1};
				ATMA_ENSURE_IS(S_OK, context()->d3d_device()->CreateBuffer(&buffer_desc, &d3d_data, d3d_buffer_.assign()));
			}
			else
			{
				ATMA_ENSURE_IS(S_OK, context()->d3d_device()->CreateBuffer(&buffer_desc, nullptr, d3d_buffer_.assign()));
			}

			break;
		}

		case resource_storage_t::persistant_shadowed:
		case resource_storage_t::temporary_shadowed:
		case resource_storage_t::transient_shadowed:
		case resource_storage_t::constant_shadowed:
		{
			ATMA_HALT("not supported");
			break;
		}
	}
}


buffer_t::~buffer_t()
{
}

auto buffer_t::bind(gen_default_read_view_t const& v) -> void
{
	ATMA_ASSERT(!default_read_view_);

	//default_read_view_ = make_resource_view(shared_from_this<resource_t>(), gpu_access_t::read, v.element_format, v.subset);
}

auto buffer_t::bind(gen_default_read_write_view_t const& v) -> void
{
	ATMA_ASSERT(!default_read_write_view_);

	//default_read_write_view_ = make_resource_view(shared_from_this<resource_t>(), gpu_access_t::read_write, v.element_format, v.subset);
}

