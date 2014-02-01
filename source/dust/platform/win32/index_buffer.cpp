#include <dust/index_buffer.hpp>
#include <dust/thread.hpp>
#include <atma/assert.hpp>

using dust::plumbing::index_buffer_t;
using dust::plumbing::locked_index_buffer_t;
using dust::plumbing::lock_type_t;
using dust::plumbing::gpu_access_t;
using dust::plumbing::cpu_access_t;

#if 0
//======================================================================
// index_buffer_t
//======================================================================
index_buffer_t::index_buffer_t(gpu_access_t gpua, cpu_access_t cpua, bool shadow, uint32_t data_size)
: index_buffer_t(gpua, cpua, shadow, data_size, nullptr)
{
}

index_buffer_t::index_buffer_t(gpu_access_t gpua, cpu_access_t cpua, bool shadow, uint32_t data_size, void* data)
: d3d_buffer_(), gpu_access_(gpua), cpu_access_(cpua), data_size_(data_size), shadowing_(shadow)
{
	prime_thread::enqueue(std::bind(&voodoo::create_index_buffer, &d3d_buffer_, gpu_access_, cpu_access_, data_size, data));

	if (shadowing_) {
		ATMA_ASSERT(data);
		data_.assign(reinterpret_cast<char*>(data), reinterpret_cast<char*>(data) + data_size_);
	}

	if (data) {
		prime_thread::enqueue(std::bind(&index_buffer_t::reload_from_shadow_buffer, this));
	}

	prime_thread::enqueue_block();
}

index_buffer_t::~index_buffer_t()
{
	mutex_.lock();

	if (d3d_buffer_) {
		d3d_buffer_->Release();
	}

	mutex_.unlock();
}

auto index_buffer_t::is_shadowing() const -> bool
{
	return shadowing_;
}

auto index_buffer_t::reload_from_shadow_buffer() -> void
{
	ATMA_ASSERT(shadowing_);

	locked_index_buffer_t L(*this, lock_type_t::write_discard);
	std::copy_n(&data_.front(), data_size_, L.begin<char>());
}

auto index_buffer_t::release_shadow_buffer() -> void
{
	ATMA_ASSERT(shadowing_);

	data_.clear();
	data_.shrink_to_fit();
	shadowing_ = false;
}

auto index_buffer_t::aquire_shadow_buffer(bool pull_from_hardware) -> void
{
	ATMA_ASSERT(!shadowing_);
	ATMA_ASSERT(false);
	shadowing_ = true;
}

auto index_buffer_t::rebase_from_buffer(index_buffer_t::data_t&& buffer, bool upload_to_hardware) -> void
{
	ATMA_ASSERT(data_size_ == buffer.size());

	data_.swap(buffer);
	if (upload_to_hardware)
		reload_from_shadow_buffer();
}

auto index_buffer_t::rebase_from_buffer(index_buffer_t::data_t const& buffer, bool upload_to_hardware) -> void
{
	ATMA_ASSERT(data_size_ == buffer.size());

	std::copy(buffer.begin(), buffer.end(), data_.begin());

	if (upload_to_hardware)
		reload_from_shadow_buffer();
}


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
