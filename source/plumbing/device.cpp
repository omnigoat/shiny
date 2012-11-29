#include <shiny/plumbing/device.hpp>

ID3D11Device* shiny::plumbing::device_t::d3d_device_ = nullptr;
ID3D11DeviceContext* shiny::plumbing::device_t::d3d_immediate_context_ = nullptr;

shiny::plumbing::device_t::device_t()
{
	if (!device_) {
		D3D11CreateDevice(
			nullptr, D3D_DRIVER_TYPE_HARDWARE, HMODULE(), 0, NULL, D3D_FEATURE_LEVEL_11_0, D3D11_SDK_VERSION,
			&d3d_device_,
			nullptr,
			&d3d_immediate_context_
		);
	}
	else {
		d3d_device_->AddRef();
		d3d_immediate_context_->AddRef();
	}
}

shiny::plumbing::device_t::device_t(const device_t& rhs)
{
	d3d_device_->AddRef();
	d3d_immediate_context_->AddRef();
}

shiny::plumbing::device_t::device_t(const device_t&& rhs)
{
}

shiny::plumbing::device_t::~device_t()
{
	d3d_device_->Release();
	d3d_immediate_context_->Release();
}

shiny::plumbing::device_t& shiny::plumbing::device_t::operator = (const device_t& rhs)
{
	d3d_device_->AddRef();
	d3d_immediate_context_->AddRef();
	return *this;
}

shiny::plumbing::device_t& shiny::plumbing::device_t::operator = (const device_t&&)
{
	return *this;
}

shiny::plumbing::vertex_buffer_t shiny::plumbing::device_t::create_vertex_buffer( unsigned int size )
{
	return shiny::plumbing::vertex_buffer_t(device_, vertex_buffer_t::usage::general, size, true);
}

