#include <dust/platform/win32/index_buffer.hpp>

#include <dust/context.hpp>

#include <atma/assert.hpp>


using namespace dust;
using dust::index_buffer_t;


index_buffer_t::index_buffer_t(context_ptr const& context, buffer_usage_t usage, uint32 data_size, void* data)
: context_(context), usage_(usage), capacity_(data_size), size_(data_size)
{
	ATMA_ASSERT(capacity_);

	switch (usage_)
	{
		case buffer_usage_t::immutable:
		{
			ATMA_ASSERT_MSG(data, "immutable buffers require data upon initialisation");

			gpu_access_ = gpu_access_t::read;
			cpu_access_ = cpu_access_t::none;

			context_->create_d3d_buffer(d3d_buffer_, buffer_type_t::index_buffer, gpu_access_t::read, cpu_access_t::none, capacity_, data);
			break;
		}

		case buffer_usage_t::long_lived:
		{
			gpu_access_ = gpu_access_t::read;
			cpu_access_ = cpu_access_t::write;

			if (data) {
				shadow_buffer_.assign(reinterpret_cast<char*>(data), reinterpret_cast<char*>(data) + capacity_);
				upload_shadow_buffer();
			}
			else {
				shadow_buffer_.resize((uint32)capacity_);
				context_->create_d3d_buffer(d3d_buffer_, buffer_type_t::index_buffer, gpu_access_, cpu_access_, capacity_, data);
			}
			break;
		}
	}
}

index_buffer_t::~index_buffer_t()
{
}

auto index_buffer_t::is_shadowing() const -> bool
{
	return !shadow_buffer_.empty();
}

auto index_buffer_t::upload_shadow_buffer() -> void
{
	ATMA_ASSERT(!shadow_buffer_.empty());

	context_->signal_d3d_buffer_upload(d3d_buffer_, &shadow_buffer_[0], shadow_buffer_.size(), 1);
	context_->signal_block();
}


#if 0
//======================================================================
// locked_index_buffer_t
//======================================================================
locked_index_buffer_t::locked_index_buffer_t(index_buffer_t& vertex_buffer, lock_type_t lock_type)
	: owner_(&vertex_buffer), lock_type_(lock_type), guard_(owner_->mutex_)
{
	ATMA_ASSERT_SWITCH(lock_type_,
		(lock_type_t::read, ATMA_ASSERT_ONE_OF(owner_->cpu_access_, cpu_access_t::read, cpu_access_t::read_write))
		(lock_type_t::write,
		lock_type_t::write_discard, ATMA_ASSERT_ONE_OF(owner_->cpu_access_, cpu_access_t::write, cpu_access_t::read_write))
		(lock_type_t::read_write, ATMA_ASSERT(owner_->cpu_access_ == cpu_access_t::read_write))
		);


	// if we're not shadowing, we ask the prime thread to map our buffer, and
	// then we block until it's done so.
	if (!owner_->shadowing_)
	{
		switch (lock_type_)
		{
			// write-discard can be performed on this thread
			case lock_type_t::write_discard: //map_type = D3D11_MAP_WRITE_DISCARD; break;
				voodoo::map(owner_->d3d_buffer_, &d3d_resource_, D3D11_MAP_WRITE_DISCARD, 0);
				break;

				// other types must be performed on the prime-thread
			default:
			{
				D3D11_MAP map_type
					= (lock_type_ == lock_type_t::read) ? D3D11_MAP_READ
					: (lock_type_ == lock_type_t::read_write) ? D3D11_MAP_READ_WRITE
					: D3D11_MAP_WRITE
					;

				prime_thread::enqueue(std::bind(&voodoo::map_vb, owner_->d3d_buffer_, &d3d_resource_, map_type, 0U));
				prime_thread::enqueue_block();
			}
		}
	}
}

locked_index_buffer_t::~locked_index_buffer_t()
{
	// if we are shadowing, that means all data written was written into our shadow
	// buffer. we will now update the d3d buffer from our shadow buffer.
	if (owner_->shadowing_) {
		prime_thread::enqueue(std::bind(&voodoo::map_vb, owner_->d3d_buffer_, &d3d_resource_, D3D11_MAP_WRITE_DISCARD, 0U));
		prime_thread::enqueue([&] { std::memcpy(d3d_resource_.pData, &owner_->data_.front(), owner_->data_size_); });
	}

	prime_thread::enqueue(std::bind(&voodoo::unmap, owner_->d3d_buffer_, 0));
	prime_thread::enqueue_block();
}
#endif
