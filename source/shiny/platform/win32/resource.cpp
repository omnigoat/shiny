#include <shiny/platform/win32/resource.hpp>
#include <shiny/platform/win32/context.hpp>

using namespace shiny;
using shiny::resource_t;


resource_t::resource_t(renderer_ptr const& rndr, resource_type_t rt, resource_usage_mask_t ru, resource_storage_t rs, size_t element_stride, size_t element_count)
	: rndr_(rndr), resource_type_(rt), resource_usage_(ru), resource_storage_(rs), element_stride_(element_stride), element_count_(element_count)
{
}

resource_t::~resource_t()
{
}
