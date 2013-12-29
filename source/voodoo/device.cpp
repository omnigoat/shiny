#include <shiny/voodoo/device.hpp>

#include <shiny/format.hpp>

#include <atma/assert.hpp>

#ifdef _DEBUG
#include <initguid.h>
#include <dxgidebug.h>
#endif

#include <map>
#include <tuple>

//======================================================================
// DXGI
//======================================================================
namespace
{
	using namespace shiny;

	// dxgi factory
	atma::com_ptr<IDXGIFactory1> dxgi_factory_;
	atma::com_ptr<IDXGIDebug> dxgi_debug_;

	// dxgi adapters
	std::vector<voodoo::dxgi_adapter_ptr> dxgi_adapters_;
	
	// outputs for adapters
	typedef std::vector<atma::com_ptr<IDXGIOutput>> dxgi_outputs_t;
	typedef std::map<atma::com_ptr<IDXGIAdapter1>, dxgi_outputs_t> dxgi_outputs_mapping_t;
	dxgi_outputs_mapping_t dxgi_outputs_mapping_;

	// valid backbuffer formats for outputs
	typedef std::vector<display_mode_t> display_modes_t;
	typedef std::map<voodoo::dxgi_output_ptr, display_modes_t> dxgi_backbuffer_formats_t;
	dxgi_backbuffer_formats_t dxgi_backbuffer_formats_;
	


	auto enumerate_backbuffers(display_modes_t& dest, voodoo::dxgi_output_ptr const& dxgi_output) -> void
	{
		auto format = DXGI_FORMAT_R8G8B8A8_UNORM;

		uint32_t mode_count = 0;
		ATMA_ENSURE_IS(S_OK, dxgi_output->GetDisplayModeList(format, 0, &mode_count, nullptr));

		auto modes = std::unique_ptr<DXGI_MODE_DESC[]>(new DXGI_MODE_DESC[mode_count]);
		ATMA_ENSURE_IS(S_OK, dxgi_output->GetDisplayModeList(format, 0, &mode_count, modes.get()));

		// convert dxgi format to shiny's format
		for (auto i = modes.get(); i != modes.get() + mode_count; ++i)
		{
			dest.push_back({
				i->Width, i->Height,
				i->RefreshRate.Numerator, i->RefreshRate.Denominator,
				display_format_t::r8g8b8a8_unorm
			});
		}
	}
}




//======================================================================
// D3D
//======================================================================
namespace
{
	using namespace shiny::voodoo;

	std::map<dxgi_adapter_ptr, d3d_device_ptr> d3d_devices_;
}

//======================================================================
// lol data
//======================================================================
#if 0
atma::com_ptr<ID3D11Device> d3d_device_ = nullptr;
atma::com_ptr<ID3D11DeviceContext> d3d_immediate_context_ = nullptr;
std::mutex immediate_context_mutex_;

__declspec(thread) ID3D11DeviceContext* shiny::voodoo::detail::d3d_local_context_ = nullptr;

// dxgi factory
atma::com_ptr<IDXGIFactory1> shiny::voodoo::detail::dxgi_factory_;

// dxgi device for the d3d-device
atma::com_ptr<IDXGIDevice1> shiny::voodoo::detail::dxgi_device_;

// dxgi adapters
std::vector<atma::com_ptr<IDXGIAdapter1>> shiny::voodoo::detail::dxgi_adapters_;
atma::com_ptr<IDXGIAdapter1> shiny::voodoo::detail::dxgi_primary_adapter_;

// outputs/surface for primary adapter
std::vector<atma::com_ptr<IDXGIOutput>> shiny::voodoo::detail::dxgi_primary_adaptor_outputs_;
atma::com_ptr<IDXGIOutput> shiny::voodoo::detail::dxgi_primary_output_;
#endif



//======================================================================
// scoped_IC_lock
//======================================================================
#if 0
using shiny::voodoo::detail::scoped_async_immediate_context_t;
scoped_async_immediate_context_t::scoped_async_immediate_context_t()
{
	immediate_context_mutex_.lock();
}

scoped_async_immediate_context_t::~scoped_async_immediate_context_t()
{
	immediate_context_mutex_.unlock();
}

auto scoped_async_immediate_context_t::operator -> () const -> atma::com_ptr<ID3D11DeviceContext> const& {
	return d3d_immediate_context_;
}
#endif

//======================================================================
// device management
//======================================================================
auto shiny::voodoo::setup_dxgi() -> void
{
	// create dxgi factory
	ATMA_ENSURE_IS(S_OK, CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&dxgi_factory_));

	// get all the adapters
	{
		atma::com_ptr<IDXGIAdapter1> adapter;
		uint32_t i = 0;
		while (dxgi_factory_->EnumAdapters1(i++, adapter.assign()) != DXGI_ERROR_NOT_FOUND)
			dxgi_adapters_.push_back(adapter);
	}

	// get all outputs for the adapters
	{
		for (auto& x : dxgi_adapters_)
		{
			atma::com_ptr<IDXGIOutput> output;
			uint32_t i = 0;
			while (x->EnumOutputs(i++, output.assign()) != DXGI_ERROR_NOT_FOUND) {
				dxgi_outputs_mapping_[x].push_back(output);
				enumerate_backbuffers(dxgi_backbuffer_formats_[output], output);
			}
		}
	}


	ATMA_ASSERT(dxgi_factory_);

	// get debug thing
#ifdef _DEBUG
	{
		typedef HRESULT(__stdcall *fPtr)(const IID&, void**);
		HMODULE hDll = GetModuleHandleW(L"dxgidebug.dll");
		fPtr DXGIGetDebugInterface = (fPtr)GetProcAddress(hDll, "DXGIGetDebugInterface");

		DXGIGetDebugInterface(__uuidof(IDXGIDebug), (void**)&dxgi_debug_);
	}
#endif
}


auto shiny::voodoo::setup_d3d_device() -> void
{
#if 0
	ATMA_ASSERT(detail::d3d_device_ == nullptr);

	


	// the local-context for the prime-thread doesn't get an AddRef, because it's special
	detail::d3d_local_context_ = detail::d3d_immediate_context_.get();
	
	// store dxgi-device of this
	detail::d3d_device_->QueryInterface(__uuidof(IDXGIDevice1), (void**)detail::dxgi_device_.assign());
#endif
}

auto shiny::voodoo::teardown_d3d_device() -> void
{
#if 0
	detail::d3d_immediate_context_.reset();
	detail::d3d_device_.reset();
#endif

#if 0
	detail::dxgi_primary_output_.reset();
	detail::dxgi_primary_adaptor_outputs_.clear();

	detail::dxgi_primary_adapter_.reset();
	detail::dxgi_adapters_.clear();

	detail::dxgi_factory_.reset();
	

	

	OutputDebugString(L"DXGI Live Objects\n------------------------\n");
	dxgi_debug_->ReportLiveObjects(DXGI_DEBUG_DXGI, DXGI_DEBUG_RLO_ALL);
	OutputDebugString(L"END DXGI Live Objects\n");
	dxgi_debug_.reset();
#endif
}

auto shiny::voodoo::dxgi_factory() -> dxgi_factory_ptr
{
	return dxgi_factory_;
}

auto shiny::voodoo::output_at(dxgi_adapter_ptr const& adapter, uint32_t index) -> dxgi_output_ptr
{
	return dxgi_outputs_mapping_[adapter][index];
}

auto shiny::voodoo::dxgi_and_d3d_at(uint32_t adapter_index) -> std::tuple<dxgi_adapter_ptr, d3d_device_ptr, d3d_context_ptr>
{
	dxgi_adapter_ptr adapter = dxgi_adapters_[adapter_index];
	d3d_device_ptr device;
	d3d_context_ptr context;

	auto i = d3d_devices_.find(adapter);
	if (i != d3d_devices_.end()) {
		device = i->second;
		device->GetImmediateContext(context.assign());
	}
	else {
		// we can't specify the primary adapter ourselves, because for some reason
		// the transition to fullscreen sends another WM_SIZE message
		ATMA_ENSURE_IS(S_OK, D3D11CreateDevice(
			adapter.get(), D3D_DRIVER_TYPE_UNKNOWN, NULL, 0, NULL, 0, D3D11_SDK_VERSION,
			device.assign(),
			NULL,
			context.assign()
		));

		d3d_devices_[adapter] = device;
	}

	return std::make_tuple(dxgi_adapters_[adapter_index], device, context);
}