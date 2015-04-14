#include <shiny/platform/win32/index_buffer.hpp>

#include <shiny/context.hpp>

#include <atma/assert.hpp>


using namespace shiny;
using shiny::index_buffer_t;


index_buffer_t::index_buffer_t(context_ptr const& context, buffer_usage_t usage, index_format_t format, uint index_count, void const* data, uint data_indexcount)
	: buffer_t(context, false, resource_usage_mask_t::none, usage, index_size(format), index_count, data, data_indexcount)
	, index_count_(index_count), index_format_(format)
{
}

index_buffer_t::~index_buffer_t()
{
}
