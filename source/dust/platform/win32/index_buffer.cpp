#include <dust/platform/win32/index_buffer.hpp>

#include <dust/context.hpp>

#include <atma/assert.hpp>


using namespace dust;
using dust::index_buffer_t;


index_buffer_t::index_buffer_t(context_ptr const& context, buffer_usage_t usage, uint index_size, uint index_count, void* data)
: buffer_t(context, buffer_type_t::index_buffer, usage, index_size * index_count, data),
  index_count_(index_count)
{
}

index_buffer_t::~index_buffer_t()
{
}
