#include <dust/vertex_buffer.hpp>

#include <dust/context.hpp>

#include <atma/assert.hpp>

using namespace dust;
using dust::vertex_buffer_t;


//======================================================================
// vertex_buffer_t
//======================================================================
auto dust::create_vertex_buffer(context_ptr const& context, vb_usage_t usage, size_t size, void* data) -> vertex_buffer_ptr
{
	return vertex_buffer_ptr(new vertex_buffer_t(context, usage, size, data));
}

vertex_buffer_t::vertex_buffer_t(context_ptr const& context, vb_usage_t usage, size_t data_size, void* data)
: context_(context), gpu_access_(), cpu_access_(), usage_(usage), capacity_(data_size), size_(data_size)
{
	ATMA_ASSERT(data_size > 0);

	switch (usage_)
	{
		case vb_usage_t::immutable:
		{
			ATMA_ASSERT_MSG(data, "immutable buffers require data upon initialisation");

			gpu_access_ = gpu_access_t::read;
			cpu_access_ = cpu_access_t::none;

			context_->create_d3d_buffer(d3d_buffer_, gpu_access_t::read, cpu_access_t::none, data_size, data);
			break;
		}

		case vb_usage_t::long_lived:
		{
			gpu_access_ = gpu_access_t::read;
			cpu_access_ = cpu_access_t::write;

			if (data) {
				shadow_buffer_.assign(reinterpret_cast<char*>(data), reinterpret_cast<char*>(data) + capacity_);
				upload_shadow_buffer();
			}
			else {
				shadow_buffer_.resize((uint32_t)capacity_);
				context_->create_d3d_buffer(d3d_buffer_, gpu_access_, cpu_access_, data_size, data);
			}
			break;
		}
	}
}


vertex_buffer_t::~vertex_buffer_t()
{
}

auto vertex_buffer_t::is_shadowing() const -> bool
{
	return !shadow_buffer_.empty();
}

auto vertex_buffer_t::upload_shadow_buffer() -> void
{
	ATMA_ASSERT(!shadow_buffer_.empty());

	context_->signal_d3d_buffer_upload(d3d_buffer_, &shadow_buffer_[0], shadow_buffer_.size(), 1);
	context_->signal_block();
}



#if 0
//======================================================================
// locked_vertex_buffer_t
//======================================================================
locked_vertex_buffer_t::locked_vertex_buffer_t(vertex_buffer_t& vertex_buffer, lock_type_t lock_type)
	: owner_(&vertex_buffer), lock_type_(lock_type), guard_(owner_->mutex_)
{
	ATMA_ASSERT_SWITCH(lock_type_,
		(lock_type_t::read,          ATMA_ASSERT_ONE_OF(owner_->cpu_access_, cpu_access_t::read, cpu_access_t::read_write))
		(lock_type_t::write,
		 lock_type_t::write_discard, ATMA_ASSERT_ONE_OF(owner_->cpu_access_, cpu_access_t::write, cpu_access_t::read_write))
		(lock_type_t::read_write,    ATMA_ASSERT(owner_->cpu_access_ == cpu_access_t::read_write))
	);

	
	// if we're not shadowing, we ask the prime thread to map our buffer, and
	// then we block until it's done so.
	if (!owner_->shadowing_)
	{
		switch (lock_type_)
		{
			// write-discard can be performed on this thread
			case lock_type_t::write_discard:
				owner_->context_->signal_d3d_map(owner_->d3d_buffer_, &d3d_resource_, D3D11_MAP_WRITE_DISCARD, 0);
				owner_->context_->signal_block();
				break;

			// other types must be performed on the prime-thread
			default:
			{
				D3D11_MAP map_type
					= (lock_type_ == lock_type_t::read) ? D3D11_MAP_READ
					: (lock_type_ == lock_type_t::read_write) ? D3D11_MAP_READ_WRITE
					: D3D11_MAP_WRITE
					;

				owner_->context_->signal_d3d_map(owner_->d3d_buffer_, &d3d_resource_, map_type, 0);
				owner_->context_->signal_block();
				//owner_->context_->signal_d3d_map(owner_->d3d_buffer_, &d3d_resource_, map_type, 0);
				//owner_->context_->signal_block();
			}
		}
	}
}

locked_vertex_buffer_t::~locked_vertex_buffer_t()
{
	// if we are shadowing, that means all data written was written into our shadow
	// buffer. we will now update the d3d buffer from our shadow buffer.
	auto owner = owner_;
	auto d3d_resource = new D3D11_MAPPED_SUBRESOURCE;
	
	if (owner_->shadowing_) {
		owner_->context_->signal_d3d_map(owner_->d3d_buffer_, d3d_resource, D3D11_MAP_WRITE_DISCARD, 0, [&](D3D11_MAPPED_SUBRESOURCE* sr) {
			std::memcpy(d3d_resource->pData, &owner->shadow_buffer_.front(), owner->size_);
		});
	}

	owner_->context_->signal_d3d_unmap(owner_->d3d_buffer_, 0);
	owner_->context_->signal_block();
}
#endif

