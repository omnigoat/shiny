#include <shiny/plumbing/device.hpp>
//#include <shiny/plumbing/vertex_buffer.hpp>

//======================================================================
// externs
//======================================================================
ID3D11Device* shiny::plumbing::detail::d3d_device_ = nullptr;
ID3D11DeviceContext* shiny::plumbing::detail::d3d_immediate_context_ = nullptr;

shiny::plumbing::detail::command_queue_t shiny::plumbing::detail::command_queue_;
std::thread shiny::plumbing::detail::prime_thread_;


//======================================================================
// device management
//======================================================================
auto shiny::plumbing::detail::setup_d3d_device() -> void
{
	ATMA_ASSERT(detail::d3d_device_ == nullptr);

	HRESULT hr = D3D11CreateDevice(
		NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0, D3D11_SDK_VERSION,
		&d3d_device_,
		NULL,
		&d3d_immediate_context_
	);

	ATMA_ASSERT(hr == S_OK);
}

auto shiny::plumbing::detail::teardown_d3d_device() -> void
{
	d3d_immediate_context_->Release();
	d3d_device_->Release();
}


