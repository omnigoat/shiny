#include <shiny/plumbing/device.hpp>
#include <shiny/plumbing/vertex_buffer.hpp>
//======================================================================
using shiny::plumbing::context_t;
//======================================================================
ID3D11Device* shiny::plumbing::detail::d3d_device_ = nullptr;

ID3D11DeviceContext* shiny::plumbing::detail::d3d_immediate_context_ = nullptr;
std::recursive_mutex shiny::plumbing::detail::immediate_context_mutex_;

std::map<std::thread::id, context_t*> shiny::plumbing::detail::bound_contexts_;
std::mutex shiny::plumbing::detail::bound_contexts_mutex_;
__declspec(thread) context_t* shiny::plumbing::detail::context_handle_ = nullptr;


context_t::context_t()
{
	std::lock_guard<std::mutex> guard(detail::bound_contexts_mutex_);

	ATMA_ASSERT(detail::bound_contexts_.find(std::this_thread::get_id()) == detail::bound_contexts_.end());
	
	// if there is no device, then create one, and make us an immediate context.
	// otherwise, we'll be a deferred context.
	if (detail::d3d_device_ == nullptr)
	{
		HRESULT hr = D3D11CreateDevice(
			NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0, D3D11_SDK_VERSION,
			&detail::d3d_device_,
			NULL,
			&detail::d3d_immediate_context_
		);

		ATMA_ASSERT(hr == S_OK);
		d3d_context_ = detail::d3d_immediate_context_;
	}
	else {
		detail::d3d_device_->CreateDeferredContext(0, &d3d_context_);
	}

	detail::context_handle_ = this;
	detail::bound_contexts_[std::this_thread::get_id()] = this;
}

context_t::~context_t()
{
	std::lock_guard<std::mutex> guard(detail::bound_contexts_mutex_);

	ATMA_ASSERT(detail::bound_contexts_.find(std::this_thread::get_id()) != detail::bound_contexts_.end());

	// if we're the immediate context and we're being destructed, then we destroy the device
	if (d3d_context_ == detail::d3d_immediate_context_) {
		ATMA_ASSERT(detail::bound_contexts_.size() == 1);
		detail::d3d_immediate_context_->Release();
		detail::d3d_device_->Release();
	}
	else {
		d3d_context_->Release();
	}

	detail::context_handle_ = nullptr;
	detail::bound_contexts_.erase(std::this_thread::get_id());
}

auto shiny::plumbing::this_context() -> context_t&
{
	ATMA_ASSERT(detail::context_handle_ != nullptr);
	return *detail::context_handle_;
}

auto shiny::plumbing::device() -> ID3D11Device* {
	ATMA_ASSERT(detail::d3d_device_ != nullptr);
	return detail::d3d_device_;
}



