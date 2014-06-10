#include <dust/platform/win32/vertex_buffer.hpp>

#include <dust/context.hpp>
#include <dust/vertex_declaration.hpp>

#include <atma/assert.hpp>


using namespace dust;
using dust::vertex_buffer_t;


//======================================================================
// vertex_buffer_t
//======================================================================
vertex_buffer_t::vertex_buffer_t(context_ptr const& context, buffer_usage_t usage, vertex_declaration_t const& vd, uint vertex_count, void* data, uint data_vertcount)
: buffer_t(context, buffer_type_t::vertex_buffer, usage, vd.stride() * vertex_count, data, vd.stride() * data_vertcount),
  vertex_count_(vertex_count)
{
}

vertex_buffer_t::~vertex_buffer_t()
{
}
