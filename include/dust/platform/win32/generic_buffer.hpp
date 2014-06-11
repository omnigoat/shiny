#pragma once
//======================================================================
#include <dust/buffer.hpp>
#include <dust/platform/win32/dxgid3d_convert.hpp>
//======================================================================
namespace dust
{
	struct generic_buffer_t : dust::buffer_t
	{
		generic_buffer_t(context_ptr const& ctx, buffer_usage_t usage, surface_format_t format, uint elements, void const* data, uint data_elemcount)
			: buffer_t(ctx, buffer_type_t::generic_buffer, usage, platform::dxgi_size_of(format), elements, data, data_elemcount),
			  element_count_(elements)
		{
			auto desc = D3D11_SHADER_RESOURCE_VIEW_DESC{};
			desc.Format = DXGI_FORMAT_UNKNOWN;
			desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			desc.Buffer = {0, data_elemcount};

			ATMA_ENSURE_IS(S_OK, context()->d3d_device()->CreateShaderResourceView(d3d_resource().get(), &desc, d3d_srv_.assign()));
		}

		// todo: constructor that shares the data of another genric-buffer but has a different view

		auto element_count() const -> uint { return element_count_; }

		auto d3d_shader_resource_view() const -> platform::d3d_shader_resource_view_ptr const& { return d3d_srv_; }

	private:
		uint element_count_;

		platform::d3d_shader_resource_view_ptr d3d_srv_;
	};

	inline auto create_generic_buffer(
		context_ptr const& ctx, buffer_usage_t usage, surface_format_t format,
		uint elements, void const* data, uint data_elemcount)
	-> generic_buffer_ptr
	{
		return generic_buffer_ptr(new generic_buffer_t(ctx, usage, format, elements, data, data_elemcount));
	}
}