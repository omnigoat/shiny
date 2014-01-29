#ifndef DUST_VOODOO_DEVICE_HPP
#define DUST_VOODOO_DEVICE_HPP
//======================================================================
#include <dust/format.hpp>
#include <dust/output.hpp>
//======================================================================
#include <fooey/widgets/window.hpp>
//======================================================================
#include <atma/com_ptr.hpp>
//======================================================================
#include <thread>
#include <atomic>
#include <mutex>
//======================================================================
namespace dust {
namespace voodoo {
//======================================================================
	
#if 0
	//======================================================================
	// the devil lies here.
	//======================================================================
	namespace detail
	{
		
	}

	auto dxgi_factory() -> dxgi_factory_ptr;

	auto dxgi_and_d3d_at(uint32_t adapter_index) -> std::tuple<dxgi_adapter_ptr, d3d_device_ptr, d3d_context_ptr>;
	auto adapter_and_output(uint32_t adapter, uint32_t output) -> std::tuple<dxgi_adapter_ptr, dxgi_output_ptr>;
	auto output_at(dxgi_adapter_ptr const&, uint32_t output_index) -> dxgi_output_ptr;
	auto closest_matching_format(dxgi_output_ptr const&, uint32_t width, uint32_t height) -> display_mode_t;

	auto setup_dxgi() -> void;
	auto teardown_dxgi() -> void;
	auto setup_d3d_device() -> void;
	auto teardown_d3d_device() -> void;
#endif

//======================================================================
} // namespace voodoo
} // namespace dust
//======================================================================
#endif
//======================================================================
