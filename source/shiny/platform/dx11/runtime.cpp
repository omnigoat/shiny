#include <shiny/platform/dx11/runtime.hpp>


using namespace shiny;
using namespace shiny::platform;

using shiny::runtime_t;


runtime_t::runtime_t()
{
	// create dxgi factory
	ATMA_ENSURE_IS(S_OK, CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&dxgi_factory_));

	// get all the adapters
	{
		dxgi_adapter_ptr adapter;
		UINT i = 0;
		while (dxgi_factory_->EnumAdapters1(i++, adapter.assign()) != DXGI_ERROR_NOT_FOUND)
			dxgi_adapters_.push_back(adapter);
	}

	// get all outputs for the adapters
	{
		for (auto& x : dxgi_adapters_)
		{
			dxgi_output_ptr output;
			UINT i = 0;
			while (x->EnumOutputs(i++, output.assign()) != DXGI_ERROR_NOT_FOUND) {
				dxgi_outputs_mapping_[x].push_back(output);
				enumerate_backbuffers(output);
			}
		}
	}


	ATMA_ASSERT(dxgi_factory_);

	// get debug thing
#if _DEBUG
	{
		auto hDll = LoadLibrary(L"dxgidebug.dll");
		typedef HRESULT(__stdcall *fPtr)(REFIID, void**);
		fPtr DXGIGetDebugInterface = (fPtr)GetProcAddress(hDll, "DXGIGetDebugInterface");

		DXGIGetDebugInterface(__uuidof(IDXGIDebug), (void**)&dxgi_debug);
	}
#endif
}

runtime_t::~runtime_t()
{
}

auto runtime_t::dxgi_factory() const -> platform::dxgi_factory_ptr const&
{
	return dxgi_factory_;
}

auto runtime_t::enumerate_backbuffers(platform::dxgi_output_ptr const& output) -> void
{
	UINT mode_count = 0;
	ATMA_ENSURE_IS(S_OK, output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &mode_count, nullptr));

	auto modes = std::vector<DXGI_MODE_DESC>(mode_count);
	ATMA_ENSURE_IS(S_OK, output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &mode_count, &modes[0]));

	// convert dxgi format to shiny's format
	auto& formats = dxgi_backbuffer_formats_[output];
	ATMA_ASSERT(formats.empty());
	for (auto const& x : modes)
	{
		formats.push_back({
			x.Width, x.Height,
			x.RefreshRate.Numerator, x.RefreshRate.Denominator,
			format_t::nu8x4
		});
	}
}

auto shiny::runtime_t::dxgid3d_for_adapter(uint32 adapter_index) -> std::tuple<dxgi_adapter_ptr, d3d_device_ptr, d3d_renderer_ptr>
{
	dxgi_adapter_ptr const& adapter = dxgi_adapters_[adapter_index];
	d3d_device_ptr device;
	d3d_renderer_ptr renderer;

	// get or craete device for adapter
	auto i = d3d_devices_.find(adapter);
	if (i != d3d_devices_.end()) {
		device = i->second;
		device->GetImmediateContext(renderer.assign());
	}
	else
	{
#if _DEBUG
		if (S_OK != D3D11CreateDevice(
			adapter.get(), D3D_DRIVER_TYPE_UNKNOWN,
			NULL, D3D11_CREATE_DEVICE_DEBUG,
			NULL, 0,
			D3D11_SDK_VERSION,
			device.assign(),
			NULL,
			renderer.assign()
		))
#endif
		{
			ATMA_ENSURE_IS(S_OK, D3D11CreateDevice(
				adapter.get(), D3D_DRIVER_TYPE_UNKNOWN,
				NULL, 0,
				NULL, 0,
				D3D11_SDK_VERSION,
				device.assign(),
				NULL,
				renderer.assign()
			));
		}

		d3d_devices_[adapter] = device;
	}

	return std::make_tuple(dxgi_adapters_[adapter_index], device, renderer);
}

auto runtime_t::dxgi_output_of(platform::dxgi_adapter_ptr const& adapter, uint output_index) -> platform::dxgi_output_ptr const&
{
	auto i = dxgi_outputs_mapping_.find(adapter);
	ATMA_ASSERT(i != dxgi_outputs_mapping_.end());
	ATMA_ASSERT(output_index < i->second.size());

	return i->second[output_index];
}

auto runtime_t::d3d_report_live_objects() -> void
{
#if ATMA_DEBUG
	dxgi_debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
#endif
}

auto runtime_t::make_data_declaration(shiny::data_streams_t const& streams) -> data_declaration_t const*
{
	auto i = data_declaration_cache_.find(streams);
	if (i == data_declaration_cache_.end()) {
		auto p = std::unique_ptr<data_declaration_t>(new data_declaration_t(streams));
		i = data_declaration_cache_.insert(std::make_pair(streams, std::move(p))).first;
	}

	return i->second.get();
}

auto runtime_t::geometry_declaration_of(shiny::geometry_streams_t const& streams) -> geometry_declaration_t const*
{
	auto i = geometry_declaration_cache_.find(streams);
	if (i == geometry_declaration_cache_.end()) {
		auto p = std::unique_ptr<geometry_declaration_t>(new geometry_declaration_t(streams));
		i = geometry_declaration_cache_.insert(std::make_pair(streams, std::move(p))).first;
	}

	return i->second.get();
}