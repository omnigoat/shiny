#include <dust/output.hpp>

#include <dust/platform/win32/runtime.hpp>

auto dust::platform::output_at(runtime_t const& runtime, dxgi_adapter_ptr const& adapter, uint32 output_index) -> dxgi_output_ptr
{
	auto i = runtime.dxgi_outputs_mapping.find(adapter);
	ATMA_ASSERT(i != runtime.dxgi_outputs_mapping.end());
	ATMA_ASSERT(output_index < i->second.size());

	return i->second[output_index];
}