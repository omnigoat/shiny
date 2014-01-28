#ifndef DUST_PLATFORM_WIN32_RUNTIME_HPP
#define DUST_PLATFORM_WIN32_RUNTIME_HPP
//======================================================================
#include <dust/platform/win32/dxgi_fwd.hpp>
#include <dust/platform/win32/d3d_fwd.hpp>
#include <dust/format.hpp>
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
		runtime_t(runtime_t&) = delete;

		auto dxgi_and_d3d_at(uint32_t adapter_index) -> std::tuple<platform::dxgi_adapter_ptr, platform::d3d_device_ptr, platform::d3d_context_ptr>;

	private:
		platform::dxgi_factory_ptr factory_;

#ifdef _DEBUG
		atma::com_ptr<IDXGIDebug> dxgi_debug_;
#endif

		// dxgi adapters
		std::vector<platform::dxgi_adapter_ptr> dxgi_adapters_;

		// outputs for adapters
		typedef std::vector<platform::dxgi_output_ptr> dxgi_outputs_t;
		typedef std::map<platform::dxgi_adapter_ptr, dxgi_outputs_t> dxgi_outputs_mapping_t;
		dxgi_outputs_mapping_t dxgi_outputs_mapping_;

		// valid backbuffer formats for outputs
		typedef std::vector<display_mode_t> display_modes_t;
		typedef std::map<platform::dxgi_output_ptr, display_modes_t> dxgi_backbuffer_formats_t;
		dxgi_backbuffer_formats_t dxgi_backbuffer_formats_;
	};
	
//======================================================================
} // namespace dust
//======================================================================
#endif
//======================================================================
