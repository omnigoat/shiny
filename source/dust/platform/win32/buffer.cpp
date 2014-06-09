#include <dust/platform/win32/buffer.hpp>

#include <dust/context.hpp>
#include <dust/vertex_declaration.hpp>

#include <atma/assert.hpp>


using namespace dust;
using dust::buffer_t;


buffer_t::buffer_t(context_ptr const& ctx, buffer_type_t type, buffer_usage_t usage, uint data_size, void const* data)
: resource_t(ctx, {}), type_(type), usage_(usage), size_(data_size)
{
	ATMA_ASSERT(size_);
	
	switch (usage_)
	{
		case buffer_usage_t::immutable:
		{
			ATMA_ASSERT_MSG(data, "immutable buffers require data upon initialisation");

			context()->create_d3d_buffer(d3d_buffer_, type_, usage_, size_, data);
			break;
		}

		case buffer_usage_t::long_lived:
		case buffer_usage_t::dynamic:
		{
			if (data) {
				shadow_buffer_.assign(reinterpret_cast<char const*>(data), reinterpret_cast<char const*>(data) + size_);
				context()->create_d3d_buffer(d3d_buffer_, type_, usage_, size_, &shadow_buffer_[0]);
			}
			else {
				shadow_buffer_.resize((uint)size_);
				context()->create_d3d_buffer(d3d_buffer_, type_, usage_, size_, nullptr);
			}
			break;
		}

		default:
			ATMA_ASSERT_MSG(false, "buffer usage not optional");
	}
}


buffer_t::~buffer_t()
{
}

auto buffer_t::upload_shadow_buffer() -> void
{
	ATMA_ASSERT(is_shadowing());

	context()->signal_d3d_buffer_upload(d3d_buffer_, &shadow_buffer_[0], shadow_buffer_.size(), 1);
	context()->signal_block();
}

auto buffer_t::d3d_resource() const -> platform::d3d_resource_ptr
{
	return d3d_buffer_;
}
