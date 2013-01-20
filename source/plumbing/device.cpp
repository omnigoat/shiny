#include <shiny/plumbing/device.hpp>
#include <shiny/plumbing/vertex_buffer.hpp>
//======================================================================
using shiny::plumbing::context_t;
//======================================================================
ID3D11Device* shiny::plumbing::detail::d3d_device_ = nullptr;
ID3D11DeviceContext* shiny::plumbing::detail::d3d_immediate_context_ = nullptr;
std::thread::id shiny::plumbing::detail::device_creation_thread_id_;
std::mutex shiny::plubming::detail::immediate_context_mutex_;

std::map<std::thread::id, context_t*> context_t::bound_contexts_;
std::mutex context_t::mutex_;



context_t::context_t()
{
	mutex_.lock();

	//ATMA_ASSERT(detail::d3d_device_ == nullptr);
	//ATMA_ASSERT(detail::d3d_immediate_context_ == nullptr);
	ATMA_ASSERT(context_t::bound_contexts_.find(std::this_thread::get_id()) == context_t::bound_contexts_.end());
	
	// if the thread we've been created in is the same thread that the device was
	// created in, then we're an immediate context.
	if (detail::d3d_device_ == nullptr) {
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

	bound_contexts_[std::this_thread::get_id()] = this;

	mutex_.unlock();
}

context_t::~context_t()
{
	mutex_.lock();

	ATMA_ASSERT(context_t::bound_contexts_.find(std::this_thread::get_id()) != context_t::bound_contexts_.end());

	// if we're the immediate context and we're being destructed, then we destroy the
	// device, but only if there are no other contexts around.
	if (d3d_context_ == detail::d3d_immediate_context_) {
		ATMA_ASSERT(bound_contexts_.size() == 1);
		detail::d3d_immediate_context_->Release();
		detail::d3d_device_->Release();
	}

	bound_contexts_.erase(std::this_thread::get_id());
	ATMA_ASSERT(bound_contexts_.empty());

	mutex_.unlock();
}

using shiny::plumbing::vertex_buffer_t;
using shiny::plumbing::lock_type_t;

auto context_t::map(vertex_buffer_t& vb, vertex_buffer_t::lock_t& lock) -> void
{
	D3D11_MAP map;
	switch (lock.lock_type()) {
		case lock_type_t::read: map = D3D11_MAP_READ; break;
		case lock_type_t::write = D3D11_MAP_WRITE; break;
		case lock_type_t::read_write = D3D11_MAP_READ_WRITE; break;
		case lock_type_t::write_discard = D3D11_MAP_WRITE_DISCARD; break;
	}

	// single-threaded
	{
		std::lock_guard<std::mutex> SL(detail::immediate_context_mutex_);
		detail::d3d_immediate_context_->Map(vb.d3d_buffer_, 0, map, 0, lock.d3d_resource_);
	}
}

auto context_t::unmap(shiny::plumbing::vertex_buffer_t& vb) -> void
{
	
}

auto shiny::plumbing::this_context() -> context_t&
{
	context_t::mutex_.lock();
	auto i = context_t::bound_contexts_.find(std::this_thread::get_id());
	ATMA_ASSERT(i != context_t::bound_contexts_.end());
	context_t* c = i->second;
	context_t::mutex_.unlock();

	return *c;
}

auto shiny::plumbing::device() -> ID3D11Device* {
	ATMA_ASSERT(detail::d3d_device_ != nullptr);
	return detail::d3d_device_;
}



