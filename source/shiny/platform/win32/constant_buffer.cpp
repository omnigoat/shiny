#include <shiny/platform/win32/constant_buffer.hpp>

#include <shiny/context.hpp>


using namespace shiny;
using shiny::constant_buffer_t;


constant_buffer_t::constant_buffer_t(context_ptr const& context, uint data_size, void const* data)
: buffer_t(context, buffer_type_t::constant_buffer, buffer_usage_t::dynamic, data_size, 1, data, 1)
{
}