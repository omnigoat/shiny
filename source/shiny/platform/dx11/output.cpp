#include <shiny/output.hpp>

#include <shiny/platform/dx11/runtime.hpp>

#if 0
auto shiny::platform::output_at(runtime_t const& runtime, dxgi_adapter_ptr const& adapter, uint32 output_index) -> dxgi_output_ptr
{
	auto i = runtime.output_of(adapter, output_index);
	ATMA_ASSERT(i);
	ATMA_ASSERT(output_index < i->second.size());

	return i->second[output_index];
}
#endif