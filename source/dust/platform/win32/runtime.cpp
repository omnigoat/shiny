#include <dust/platform/win32/runtime.hpp>

using namespace dust;
using namespace dust::platform;

using dust::runtime_t;


//======================================================================
// enumerate-backbuffers
//======================================================================
namespace
{
	auto enumerate_backbuffers(runtime_t::display_modes_t& dest, platform::dxgi_output_ptr const& dxgi_output) -> void
	{
		auto format = DXGI_FORMAT_R8G8B8A8_UNORM;


		UINT mode_count = 0;
		ATMA_ENSURE_IS(S_OK, dxgi_output->GetDisplayModeList(format, 0, &mode_count, nullptr));

		//auto modes = std::unique_ptr<DXGI_MODE_DESC[]>(new DXGI_MODE_DESC[mode_count]);
		auto modes = std::vector<DXGI_MODE_DESC>(mode_count);
		ATMA_ENSURE_IS(S_OK, dxgi_output->GetDisplayModeList(format, 0, &mode_count, &modes[0]));

		// convert dxgi format to dust's format
		for (auto const& x : modes)
		{
			dest.push_back({
				x.Width, x.Height,
				x.RefreshRate.Numerator, x.RefreshRate.Denominator,
				element_format_t::un8x4
			});
		}
	}
}




//======================================================================
// runtime_t
//======================================================================
runtime_t::runtime_t()
{
	// create dxgi factory
	ATMA_ENSURE_IS(S_OK, CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&dxgi_factory));

	// get all the adapters
	{
		dxgi_adapter_ptr adapter;
		UINT i = 0;
		while (dxgi_factory->EnumAdapters1(i++, adapter.assign()) != DXGI_ERROR_NOT_FOUND)
			dxgi_adapters.push_back(adapter);
	}

	// get all outputs for the adapters
	{
		for (auto& x : dxgi_adapters)
		{
			dxgi_output_ptr output;
			UINT i = 0;
			while (x->EnumOutputs(i++, output.assign()) == S_OK) { //DXGI_ERROR_NOT_FOUND) {
				dxgi_outputs_mapping[x].push_back(output);
				enumerate_backbuffers(dxgi_backbuffer_formats[output], output);
			}
		}
	}


	ATMA_ASSERT(dxgi_factory);

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


auto dust::platform::dxgi_and_d3d_at(runtime_t& runtime, uint32 adapter_index) -> std::tuple<dxgi_adapter_ptr, d3d_device_ptr, d3d_context_ptr>
{
	dxgi_adapter_ptr adapter = runtime.dxgi_adapters[adapter_index];
	d3d_device_ptr device;
	d3d_context_ptr context;

	// get or craete device for adapter
	auto i = runtime.d3d_devices.find(adapter);
	if (i != runtime.d3d_devices.end()) {
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

		runtime.d3d_devices[adapter] = device;
	}

	return std::make_tuple(runtime.dxgi_adapters[adapter_index], device, context);
}



