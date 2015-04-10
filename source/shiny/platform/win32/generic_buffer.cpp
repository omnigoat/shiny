#include <shiny/platform/win32/generic_buffer.hpp>

#include <shiny/context.hpp>

using namespace shiny;
using shiny::generic_buffer_t;


generic_buffer_t::generic_buffer_t(context_ptr const& ctx, resource_usage_mask_t const& rs, buffer_usage_t usage, size_t stride, uint elements, void const* data, uint data_elemcount)
	: buffer_t(ctx, buffer_type_t::generic_buffer, rs, usage, stride, elements, data, data_elemcount)
{
	auto desc = D3D11_SHADER_RESOURCE_VIEW_DESC{
		DXGI_FORMAT_UNKNOWN,
		D3D11_SRV_DIMENSION_BUFFER,
		D3D11_BUFFER_SRV{0, element_count_}};

	ATMA_ENSURE_IS(S_OK, context()->d3d_device()->CreateShaderResourceView(d3d_resource().get(), &desc, d3d_srv_.assign()));
}


auto shiny::make_generic_buffer(
	context_ptr const& ctx,
	resource_usage_mask_t rmask,
	buffer_usage_t usage,
	size_t stride, uint elements,
	void const* data, uint data_elements) -> generic_buffer_ptr
{
	return atma::make_intrusive_ptr<generic_buffer_t>(ctx, rmask, usage, stride, elements, data, data_elements);
}
