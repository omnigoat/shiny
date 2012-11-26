#include <shiny/plumbing/device.hpp>

ID3D11Device* shiny::plumbing::device_t::device_ = nullptr;
ID3D11DeviceContext* shiny::plumbing::device_t::immediate_context_ = nullptr;

shiny::plumbing::device_t::device_t()
{
	if (!device_) {
		D3D11CreateDevice(
			nullptr, D3D_DRIVER_TYPE_HARDWARE, HMODULE(), 0, NULL, D3D_FEATURE_LEVEL_11_0, D3D11_SDK_VERSION,
			&device_,
			nullptr,
			&immediate_context_
		);
	}
	else {
		device_->AddRef();
		immediate_context_->AddRef();
	}
}

shiny::plumbing::device_t::device_t(const device_t& rhs)
{
	device_->AddRef();
	immediate_context_->AddRef();
}

shiny::plumbing::device_t::device_t(const device_t&& rhs)
{
}

shiny::plumbing::device_t::~device_t()
{
	device_->Release();
	immediate_context_->Release();
}

shiny::plumbing::device_t& shiny::plumbing::device_t::operator = (const device_t& rhs)
{
	device_->AddRef();
	immediate_context_->AddRef();
	return *this;
}

shiny::plumbing::device_t& shiny::plumbing::device_t::operator = (const device_t&&)
{
	return *this;
}

