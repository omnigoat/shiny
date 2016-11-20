#include <shiny/platform/win32/constant_buffer.hpp>

#include <shiny/context.hpp>


using namespace shiny;
using shiny::constant_buffer_t;


constant_buffer_t::constant_buffer_t(renderer_ptr const& context, size_t data_size, void const* data)
	: buffer_t(context, resource_type_t::constant_buffer, resource_usage_mask_t::none, resource_storage_t::constant, buffer_dimensions_t{data_size, 1}, buffer_data_t{data, 1})
{
}
