#pragma once

#include <shiny/platform/win32/dxgi_fwd.hpp>
#include <shiny/platform/win32/d3d_fwd.hpp>

#include <shiny/element_format.hpp>
#include <shiny/output.hpp>
#include <shiny/data_declaration.hpp>
#include <shiny/geometry_declaration.hpp>

#include <fooey/fooey_fwd.hpp>

#ifdef _DEBUG
#pragma warning(push,3)
#include <initguid.h>
#include <dxgidebug.h>
#pragma warning(pop)
#endif

#include <vector>
#include <map>


namespace shiny
{
	struct runtime_t
	{
		runtime_t();
		~runtime_t();
		runtime_t(runtime_t const&) = delete;
		runtime_t(runtime_t&&) = delete;

		auto dxgi_factory() const -> platform::dxgi_factory_ptr const&;


		// generic functions
		auto output_for_window(fooey::window_ptr const&) -> output_ptr;
		auto output_of(adapter_ptr const&, uint output_index) -> output_ptr;
		auto make_data_declaration(data_streams_t const&) -> data_declaration_t const*;
		auto geometry_declaration_of(geometry_streams_t const&) -> geometry_declaration_t const*;

		// dxgi/d3d specific
		auto dxgid3d_for_adapter(uint adapter_index) -> std::tuple<platform::dxgi_adapter_ptr, platform::d3d_device_ptr, platform::d3d_context_ptr>;
		auto dxgi_output_of(platform::dxgi_adapter_ptr const&, uint output_index) -> platform::dxgi_output_ptr const&;

	private:
		auto enumerate_backbuffers(platform::dxgi_output_ptr const&) -> void;

	private:
		platform::dxgi_factory_ptr dxgi_factory_;

#ifdef _DEBUG
		atma::com_ptr<IDXGIDebug> dxgi_debug;
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

		std::map<platform::dxgi_adapter_ptr, platform::d3d_device_ptr> d3d_devices_;


		std::map<geometry_streams_t, std::unique_ptr<geometry_declaration_t>> geometry_declaration_cache_;
		std::map<data_streams_t, std::unique_ptr<data_declaration_t>> data_declaration_cache_;
	};
}

