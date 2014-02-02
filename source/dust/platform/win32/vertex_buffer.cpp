#include <dust/vertex_buffer.hpp>

#include <atma/assert.hpp>


using dust::vertex_buffer_t;
using dust::vertex_buffer_ptr;
using dust::locked_vertex_buffer_t;
using dust::gpu_access_t;
using dust::cpu_access_t;

using dust::context_ptr;


//======================================================================
// vertex_buffer_t
//======================================================================
#if 0
auto vertex_buffer_t::create(std::initializer_list<context_ptr> contexts, gpu_access_t gpua, cpu_access_t cpua, bool shadow, uint32_t data_size) -> vertex_buffer_ptr
{
	auto vb = vertex_buffer_ptr(new vertex_buffer_t(gpua, cpua, shadow, data_size));

	for (auto const& x : contexts) {
		//vb->bind_to(x);
	}

	return vb;
}
#endif

vertex_buffer_t::vertex_buffer_t(context_ptr const& context, usage_t usage, bool shadow, uint32_t data_size, void* data)
: context_(context), gpu_access_(), cpu_access_(), usage_(usage), shadowing_(shadow), data_size_(data_size)
{
	ATMA_ASSERT(data_size > 0);

	if (usage_ == usage_t::immutable)
	{
		ATMA_ASSERT_MSG(data, "immutable buffers require data upon initialisation");

		gpu_access_ = gpu_access_t::read;
		cpu_access_ = cpu_access_t::none;

		context_->create_d3d_buffer(d3d_buffer_, gpu_access_t::read, cpu_access_t::none, data_size, data);
	}
	else if (usage_ == usage_t::long_lived)
	{
		gpu_access_ = gpu_access_t::read;
		cpu_access_ = cpu_access_t::write;

		// if we're shadowing, then stick the data in the shadow buffer, and then upload
		// it, because the shadow-buffer is aligned correctly, and guaranteed to be fastest.
		if (shadowing_ && data) {
			data_.assign(reinterpret_cast<char*>(data), reinterpret_cast<char*>(data)+ data_size_);

			upload_shadow_buffer();
			return;
		}

		if (shadowing_)
			data_.resize(data_size);

		context_->create_d3d_buffer(d3d_buffer_, gpu_access_, cpu_access_, data_size, data);
	}
}

#if 0
vertex_buffer_t::vertex_buffer_t(gpu_access_t gpua, cpu_access_t cpua, bool shadow, uint32_t data_size)
: vertex_buffer_t(gpua, cpua, shadow, data_size, nullptr)
{
}

vertex_buffer_t::vertex_buffer_t(gpu_access_t gpua, cpu_access_t cpua, bool shadow, uint32_t data_size, void* data)
: gpu_access_(gpua), cpu_access_(cpua), data_size_(data_size), shadowing_(shadow)
{
	
	//context_->create_d3d_buffer(d3d_buffer_, gpu_access_, cpu_access_, data_size, data);
	//prime_thread::enqueue(std::bind(&voodoo::create_buffer, &d3d_buffer_, gpu_access_, cpu_access_, data_size, data));
	
	if (shadowing_) {
		ATMA_ASSERT(data);
		data_.assign(reinterpret_cast<char*>(data), reinterpret_cast<char*>(data) + data_size_);
	}

	//context_->signal_block();

	if (data) {
		
		//prime_thread::enqueue(std::bind(&vertex_buffer_t::reload_from_shadow_buffer, this));
		//context_->signal_

		//prime_thread::enqueue_block();
		
	}
}
#endif

vertex_buffer_t::~vertex_buffer_t()
{
}

auto vertex_buffer_t::is_shadowing() const -> bool
{
	return shadowing_;
}

auto vertex_buffer_t::allocate_shadow_buffer() -> void
{
	ATMA_ASSERT(!shadowing_);
	data_.resize(data_size_);
	shadowing_ = true;
}

auto vertex_buffer_t::release_shadow_buffer() -> void
{
	ATMA_ASSERT(shadowing_);

	data_.clear();
	data_.shrink_to_fit();
	shadowing_ = false;
}

auto vertex_buffer_t::fill_shadow_buffer(vertex_buffer_t::data_t const& buffer, uint32_t offset) -> void
{
	ATMA_ASSERT(offset + buffer.size() <= data_size_);

	memcpy(&data_[0] + offset, &buffer[0], buffer.size());
}

auto vertex_buffer_t::fill_shadow_buffer(vertex_buffer_t::data_t&& buffer) -> void
{
	ATMA_ASSERT(data_size_ == buffer.size());

	data_.swap(buffer);
}

auto vertex_buffer_t::upload_shadow_buffer(bool block) -> void
{
	ATMA_ASSERT(shadowing_);

	context_->signal_d3d_buffer_upload(d3d_buffer_, &data_[0], data_.size(), 1);
	if (block)
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
			std::memcpy(d3d_resource->pData, &owner->data_.front(), owner->data_size_);
		});
	}

	owner_->context_->signal_d3d_unmap(owner_->d3d_buffer_, 0);
	owner_->context_->signal_block();
}
#endif

