#include <shiny/voodoo/device.hpp>
#include <atma/assert.hpp>

#ifdef _DEBUG
#include <initguid.h>
#include <dxgidebug.h>
#endif

//======================================================================
// externs
//======================================================================
atma::com_ptr<ID3D11Device> shiny::voodoo::detail::d3d_device_ = nullptr;
atma::com_ptr<ID3D11DeviceContext> shiny::voodoo::detail::d3d_immediate_context_ = nullptr;
std::mutex shiny::voodoo::detail::immediate_context_mutex_;

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

atma::com_ptr<IDXGIDebug> dxgi_debug_;


//======================================================================
// scoped_IC_lock
//======================================================================
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


//======================================================================
// device management
//======================================================================
auto shiny::voodoo::setup_dxgi() -> void
{
	// create dxgi factory
	ATMA_ENSURE_IS(S_OK, CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&detail::dxgi_factory_));

	// get all the adapters
	{
		atma::com_ptr<IDXGIAdapter1> adapter;
		uint32_t i = 0;
		while (detail::dxgi_factory_->EnumAdapters1(i++, adapter.assign()) != DXGI_ERROR_NOT_FOUND)
			detail::dxgi_adapters_.push_back(adapter);
		
		detail::dxgi_primary_adapter_ = detail::dxgi_adapters_[0];
	}

	// get all outputs for the primary adapter
	{
		atma::com_ptr<IDXGIOutput> output;
		uint32_t i = 0;
		while (detail::dxgi_primary_adapter_->EnumOutputs(i++, output.assign()) != DXGI_ERROR_NOT_FOUND)
			detail::dxgi_primary_adaptor_outputs_.push_back(output);

		// store primary output
		detail::dxgi_primary_output_ = detail::dxgi_primary_adaptor_outputs_[0];
	}


	ATMA_ASSERT(detail::dxgi_factory_);

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
	ATMA_ASSERT(detail::d3d_device_ == nullptr);

	ATMA_ENSURE_IS(S_OK, D3D11CreateDevice(
		detail::dxgi_primary_adapter_.get(), D3D_DRIVER_TYPE_UNKNOWN, NULL, 0, NULL, 0, D3D11_SDK_VERSION,
		detail::d3d_device_.assign(),
		NULL,
		detail::d3d_immediate_context_.assign()
	));


	// the local-context for the prime-thread doesn't get an AddRef, because it's special
	detail::d3d_local_context_ = detail::d3d_immediate_context_.get();
	
	// store dxgi-device of this
	detail::d3d_device_->QueryInterface(__uuidof(IDXGIDevice1), (void**)detail::dxgi_device_.assign());
}

auto shiny::voodoo::teardown_d3d_device() -> void
{
#if 1
	detail::d3d_immediate_context_.reset();
	detail::d3d_device_.reset();
#endif

	detail::dxgi_primary_output_.reset();
	detail::dxgi_primary_adaptor_outputs_.clear();

	detail::dxgi_primary_adapter_.reset();
	detail::dxgi_adapters_.clear();

	detail::dxgi_device_.reset();
	detail::dxgi_factory_.reset();
	

	

	OutputDebugString(L"DXGI Live Objects\n------------------------\n");
	dxgi_debug_->ReportLiveObjects(DXGI_DEBUG_DXGI, DXGI_DEBUG_RLO_ALL);
	OutputDebugString(L"END DXGI Live Objects\n");
	dxgi_debug_.reset();

}
