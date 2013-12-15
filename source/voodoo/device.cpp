#include <shiny/voodoo/device.hpp>
#include <atma/assert.hpp>

using shiny::voodoo::com_ptr;

//======================================================================
// externs
//======================================================================
ID3D11Device* shiny::voodoo::detail::d3d_device_ = nullptr;
ID3D11DeviceContext* shiny::voodoo::detail::d3d_immediate_context_ = nullptr;
std::mutex shiny::voodoo::detail::immediate_context_mutex_;

__declspec(thread) ID3D11DeviceContext* shiny::voodoo::detail::d3d_local_context_ = nullptr;

// dxgi factory
com_ptr<IDXGIFactory1> shiny::voodoo::detail::dxgi_factory_;

// dxgi device for the d3d-device
com_ptr<IDXGIDevice1> shiny::voodoo::detail::dxgi_device_;

// dxgi adapters
std::vector<com_ptr<IDXGIAdapter1>> shiny::voodoo::detail::dxgi_adapters_;
com_ptr<IDXGIAdapter1> shiny::voodoo::detail::dxgi_primary_adapter_;

// outputs/surface for primary adapter
std::vector<com_ptr<IDXGIOutput>> shiny::voodoo::detail::dxgi_primary_adaptor_outputs_;
com_ptr<IDXGIOutput> shiny::voodoo::detail::dxgi_primary_output_;
com_ptr<IDXGISurface> shiny::voodoo::detail::dxgi_primary_surface_;



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

auto scoped_async_immediate_context_t::operator -> () const -> ID3D11DeviceContext* {
	return d3d_immediate_context_;
}


//======================================================================
// device management
//======================================================================
auto shiny::voodoo::setup_d3d_device() -> void
{
	// create dxgi factory
	ATMA_ENSURE_IS(S_OK, CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&detail::dxgi_factory_));

	// get all the adapters
	{
		IDXGIAdapter1* adapter;
		uint32_t i = 0;
		while (detail::dxgi_factory_->EnumAdapters1(i++, &adapter) != DXGI_ERROR_NOT_FOUND)
			detail::dxgi_adapters_.push_back(make_com_ptr(adapter));
	}

	// get all outputs for the primary adapter
	{
		IDXGIOutput* output;
		uint32_t i = 0;
		while (detail::dxgi_adapters_[0]->EnumOutputs(i++, &output) != DXGI_ERROR_NOT_FOUND)
			detail::dxgi_primary_adaptor_outputs_.push_back(make_com_ptr(output));

		// store primary output and surface
		detail::dxgi_primary_output_ = detail::dxgi_primary_adaptor_outputs_[0];
		//detail::dxgi_primary_output_->GetDisplaySurfaceData(&detail::dxgi_primary_surface_);
	}


	ATMA_ASSERT(detail::dxgi_factory_);
	ATMA_ASSERT(detail::d3d_device_ == nullptr);

	ATMA_ENSURE_IS(S_OK, D3D11CreateDevice(
		detail::dxgi_primary_adapter_.get(), D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0, D3D11_SDK_VERSION,
		&detail::d3d_device_,
		NULL,
		&detail::d3d_immediate_context_
	));

	detail::d3d_local_context_ = detail::d3d_immediate_context_;

	// store dxgi-device of this
	detail::d3d_device_->QueryInterface(__uuidof(IDXGIDevice1), (void**)&detail::dxgi_device_);
}

auto shiny::voodoo::teardown_d3d_device() -> void
{
	detail::d3d_immediate_context_->Release();
	detail::d3d_device_->Release();
}
