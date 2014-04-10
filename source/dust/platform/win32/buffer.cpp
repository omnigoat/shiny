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
#endif

//======================================================================
// buffer_t
//======================================================================
buffer_t::buffer_t(context_ptr const& context, buffer_type_t type, buffer_usage_t usage, uint data_size, void const* data)
: context_(context), type_(type), usage_(usage), size_(data_size)
{
	ATMA_ASSERT(size_);
	
	switch (usage_)
	{
		case buffer_usage_t::immutable:
		{
			ATMA_ASSERT_MSG(data, "immutable buffers require data upon initialisation");

			context_->create_d3d_buffer(d3d_buffer_, type_, usage_, size_, data);
			break;
		}

		case buffer_usage_t::long_lived:
		{
			if (data) {
				shadow_buffer_.assign(reinterpret_cast<char const*>(data), reinterpret_cast<char const*>(data) + size_);
				upload_shadow_buffer();
			}
			else {
				shadow_buffer_.resize(size_);
				context_->create_d3d_buffer(d3d_buffer_, type_, usage_, size_, nullptr);
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
	ATMA_ASSERT(is_shadowing());

	context_->signal_d3d_buffer_upload(d3d_buffer_, &shadow_buffer_[0], shadow_buffer_.size(), 1);
	context_->signal_block();
}
