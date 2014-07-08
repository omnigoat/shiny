#include <dust/platform/win32/runtime.hpp>


using namespace dust;
using namespace dust::platform;

using dust::runtime_t;


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
#ifdef _DEBUGj
	{
		typedef HRESULT(__stdcall *fPtr)(const IID&, void**);
		HMODULE hDll = GetModuleHandleW(L"dxgidebug.dll");
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

	// convert dxgi format to dust's format
	auto& formats = dxgi_backbuffer_formats_[output];
	ATMA_ASSERT(formats.empty());
	for (auto const& x : modes)
	{
		formats.push_back({
			x.Width, x.Height,
			x.RefreshRate.Numerator, x.RefreshRate.Denominator,
			element_format_t::un8x4
		});
	}
}

auto dust::runtime_t::dxgid3d_for_adapter(uint32 adapter_index) -> std::tuple<dxgi_adapter_ptr, d3d_device_ptr, d3d_context_ptr>
{
	dxgi_adapter_ptr const& adapter = dxgi_adapters_[adapter_index];
	d3d_device_ptr device;
	d3d_context_ptr context;

	// get or craete device for adapter
	auto i = d3d_devices_.find(adapter);
	if (i != d3d_devices_.end()) {
		device = i->second;
		device->GetImmediateContext(context.assign());
	}
	else
	{
		ATMA_ENSURE_IS(S_OK, D3D11CreateDevice(
			adapter.get(), D3D_DRIVER_TYPE_UNKNOWN,
			NULL, 0, NULL, 0, D3D11_SDK_VERSION,
			device.assign(),
			NULL,
			context.assign()
		));

		d3d_devices_[adapter] = device;
	}

	return std::make_tuple(dxgi_adapters_[adapter_index], device, context);
}

auto runtime_t::dxgi_output_of(platform::dxgi_adapter_ptr const& adapter, uint output_index) -> platform::dxgi_output_ptr const&
{
	auto i = dxgi_outputs_mapping_.find(adapter);
	ATMA_ASSERT(i != dxgi_outputs_mapping_.end());
	ATMA_ASSERT(output_index < i->second.size());

	return i->second[output_index];
}


