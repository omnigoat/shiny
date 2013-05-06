#include <shiny/voodoo/device.hpp>
#include <atma/assert.hpp>

//======================================================================
// externs
//======================================================================
std::thread shiny::voodoo::detail::prime_thread_;
std::atomic_bool shiny::voodoo::detail::prime_thread_running_;


ID3D11Device* shiny::voodoo::detail::d3d_device_ = nullptr;
ID3D11DeviceContext* shiny::voodoo::detail::d3d_immediate_context_ = nullptr;
std::mutex shiny::voodoo::detail::immediate_context_mutex_;

__declspec(thread) ID3D11DeviceContext* shiny::voodoo::detail::d3d_local_context_ = nullptr;


//======================================================================
// scoped_IC_lock
//======================================================================
using shiny::voodoo::detail::scoped_IC_lock;
scoped_IC_lock::scoped_IC_lock()
{
	immediate_context_mutex_.lock();
}

scoped_IC_lock::~scoped_IC_lock()
{
	immediate_context_mutex_.unlock();
}

//======================================================================
// device management
//======================================================================
auto shiny::voodoo::setup_d3d_device() -> void
{
	ATMA_ASSERT(detail::d3d_device_ == nullptr);

	HRESULT hr = D3D11CreateDevice(
		NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0, D3D11_SDK_VERSION,
		&detail::d3d_device_,
		NULL,
		&detail::d3d_immediate_context_
	);

	ATMA_ASSERT(hr == S_OK);

	detail::d3d_local_context_ = detail::d3d_immediate_context_;
}

auto shiny::voodoo::teardown_d3d_device() -> void
{
	detail::d3d_immediate_context_->Release();
	detail::d3d_device_->Release();
}


