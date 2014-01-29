#ifndef DUST_PLATFORM_WIN32_RUNTIME_HPP
#define DUST_PLATFORM_WIN32_RUNTIME_HPP
//======================================================================
#include <dust/platform/win32/dxgi_fwd.hpp>
#include <dust/platform/win32/d3d_fwd.hpp>
#include <dust/format.hpp>
#include <dust/output.hpp>

#include <fooey/widgets/window.hpp>
//======================================================================
#ifdef _DEBUG
#include <initguid.h>
#include <dxgidebug.h>
#endif
//======================================================================
#include <vector>
#include <map>
//======================================================================
namespace dust {
//======================================================================
	
	//======================================================================
	// runtime_t (win32/dxgi)
	// ------------------------
	//   scoped runtime, setting up global state for dxgi/d3d
	//======================================================================
	struct runtime_t
	{
		runtime_t();
		~runtime_t();
		
		runtime_t(runtime_t const&) = delete;
		runtime_t(runtime_t&&) = delete;

		// non-platform-specific functions
		auto output_for_window(fooey::window_ptr const&) -> output_ptr;

	
		platform::dxgi_factory_ptr factory;

#ifdef _DEBUG
		atma::com_ptr<IDXGIDebug> dxgi_debug;
#endif

		// dxgi adapters
		std::vector<platform::dxgi_adapter_ptr> dxgi_adapters;

		// outputs for adapters
		typedef std::vector<platform::dxgi_output_ptr> dxgi_outputs_t;
		typedef std::map<platform::dxgi_adapter_ptr, dxgi_outputs_t> dxgi_outputs_mapping_t;
		dxgi_outputs_mapping_t dxgi_outputs_mapping;

		// valid backbuffer formats for outputs
		typedef std::vector<display_mode_t> display_modes_t;
		typedef std::map<platform::dxgi_output_ptr, display_modes_t> dxgi_backbuffer_formats_t;
		dxgi_backbuffer_formats_t dxgi_backbuffer_formats;
	};


	namespace platform
	{
		auto dxgi_and_d3d_at(runtime_t&, uint32_t adapter_index) ->  std::tuple<dxgi_adapter_ptr, d3d_device_ptr, d3d_context_ptr>;
	}

//======================================================================
} // namespace dust
//======================================================================
#endif
//======================================================================
