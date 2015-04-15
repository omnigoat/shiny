#include <shiny/platform/win32/resource.hpp>
#include <shiny/platform/win32/context.hpp>

using namespace shiny;
using shiny::resource_t;


resource_t::resource_t(context_ptr const& ctx, resource_type_t rt, resource_usage_mask_t ru, size_t element_stride, size_t element_count)
	: context_(ctx), resource_type_(rt), resource_usage_(ru), element_stride_(element_stride), element_count_(element_count)
{
}

resource_t::~resource_t()
{
}
