#include <dust/platform/win32/generic_buffer.hpp>

#include <dust/context.hpp>

using namespace dust;
using dust::generic_buffer_t;


generic_buffer_t::generic_buffer_t(context_ptr const& ctx, buffer_usage_t usage, element_format_t format, uint elements, void const* data, uint data_elemcount)
: generic_buffer_t(ctx, usage, element_size(format), elements, data, data_elemcount)
{
}

generic_buffer_t::generic_buffer_t(context_ptr const& ctx, buffer_usage_t usage, uint stride, uint elements, void const* data, uint data_elemcount)
: buffer_t(ctx, buffer_type_t::generic_buffer, usage, stride, elements, data, data_elemcount), element_count_(elements)
{
	auto desc = D3D11_SHADER_RESOURCE_VIEW_DESC{};
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	desc.Buffer ={0, data_elemcount};

	ATMA_ENSURE_IS(S_OK, context()->d3d_device()->CreateShaderResourceView(d3d_resource().get(), &desc, d3d_srv_.assign()));
}
