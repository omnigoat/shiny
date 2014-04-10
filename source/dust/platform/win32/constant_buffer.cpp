#include <dust/platform/win32/constant_buffer.hpp>

#include <dust/context.hpp>


using namespace dust;
using dust::constant_buffer_t;


constant_buffer_t::constant_buffer_t(context_ptr const& context, uint data_size, void const* data)
: buffer_t(context, buffer_type_t::constant_buffer, buffer_usage_t::dynamic, data_size, data)
{
}

#if 0
auto dust::create_constant_buffer(context_ptr const& context, uint data_size, void const* data) -> constant_buffer_ptr
{
	return constant_buffer_ptr(new constant_buffer_t(context, data_size, data));
}
#endif