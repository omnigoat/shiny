#include <dust/platform/win32/shader_resource_view.hpp>

#include <dust/context.hpp>
#include <dust/vertex_declaration.hpp>

#include <atma/assert.hpp>


using namespace dust;
using dust::shader_resource_view_t;


shader_resource_view_t::shader_resource_view_t(context_ptr const& ctx, view_type_t view_type)
	: context_(ctx), view_type_(view_type)
{
	
}
