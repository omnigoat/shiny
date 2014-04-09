#include <dust/platform/win32/buffer.hpp>

#include <dust/context.hpp>
#include <dust/vertex_declaration.hpp>

#include <atma/assert.hpp>


using namespace dust;
using dust::buffer_t;

#if 0
//======================================================================
// create_buffer
//======================================================================
auto dust::create_buffer(context_ptr const& context, buffer_usage_t usage, vertex_declaration_t const& vd, uint vertex_count, void* data) -> vertex_buffer_ptr
{
	return buffer_ptr(new buffer_t(context, usage, vd, vertex_count, data));
}


//======================================================================
// buffer_t
//======================================================================
buffer_t::buffer_t(context_ptr const& context, buffer_usage_t usage, vertex_declaration_t const& vd, uint vertex_count, void* data)
: context_(context), gpu_access_(), cpu_access_(), usage_(usage), capacity_(vd.stride() * vertex_count), size_(capacity_), vertex_count_(vertex_count)
{
	ATMA_ASSERT(capacity_);
	
	switch (usage_)
	{
		case buffer_usage_t::vertex_immutable:
		{
			ATMA_ASSERT_MSG(data, "immutable buffers require data upon initialisation");

			gpu_access_ = gpu_access_t::read;
			cpu_access_ = cpu_access_t::none;

			context_->create_d3d_buffer(d3d_buffer_, buffer_type_t::vertex_buffer, gpu_access_t::read, cpu_access_t::none, capacity_, data);
			break;
		}

		case buffer_usage_t::vertex_longlived:
		{
			gpu_access_ = gpu_access_t::read;
			cpu_access_ = cpu_access_t::write;

			if (data) {
				shadow_buffer_.assign(reinterpret_cast<char*>(data), reinterpret_cast<char*>(data) + capacity_);
				upload_shadow_buffer();
			}
			else {
				shadow_buffer_.resize((uint32)capacity_);
				context_->create_d3d_buffer(d3d_buffer_, buffer_type_t::vertex_buffer, gpu_access_, cpu_access_, capacity_, data);
			}
			break;
		}
	}
}


buffer_t::~buffer_t()
{
}

auto buffer_t::upload_shadow_buffer() -> void
{
	//ATMA_ASSERT(!shadow_buffer_.empty());

	//context_->signal_d3d_buffer_upload(d3d_buffer_, &shadow_buffer_[0], shadow_buffer_.size(), 1);
	//context_->signal_block();
}

#endif