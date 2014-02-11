#pragma once
//======================================================================
#include <dust/dust_fwd.hpp>

#pragma message("move vertex_buffer to platform/win32")
#include <dust/platform/win32/d3d_fwd.hpp>
#include <dust/adapter.hpp>

#include <atma/assert.hpp>
#include <atma/aligned_allocator.hpp>

#include <vector>
//======================================================================
namespace dust {
//======================================================================
	
	enum class vb_usage_t
	{
		immutable,
		long_lived
	};

	
	struct vertex_buffer_t : atma::ref_counted
	{
		typedef std::vector<char, atma::aligned_allocator_t<char, 4>> data_t;

		auto usage() const -> vb_usage_t { return usage_; }
		auto is_shadowing() const -> bool;
		auto capacity() const -> size_t;
		auto size() const -> size_t;
		auto d3d_buffer() const -> platform::d3d_buffer_ptr const& { return d3d_buffer_; }

		// maps the vertex buffer, and calls a function with the mapped range
		template <typename T, typename FN>
		auto with_map(FN) -> void;
		
		
	private:
		struct context_binding_t;

		vertex_buffer_t(context_ptr const&, vb_usage_t, size_t data_size, void* data);
#if 0
		vertex_buffer_t(gpu_access_t, cpu_access_t, bool shadow, uint32_t data_size);
		vertex_buffer_t(gpu_access_t, cpu_access_t, bool shadow, uint32_t data_size, void* data);
		vertex_buffer_t(gpu_access_t, cpu_access_t, bool shadow, data_t const& data);
		vertex_buffer_t(gpu_access_t, cpu_access_t, bool shadow, data_t&& data);
#endif

		~vertex_buffer_t();

	private:
		auto upload_shadow_buffer() -> void;

	private:
		vb_usage_t usage_;
		gpu_access_t gpu_access_;
		cpu_access_t cpu_access_;

		size_t capacity_;
		size_t size_;
		data_t shadow_buffer_;

		std::mutex mutex_;

		context_ptr context_;
		platform::d3d_buffer_ptr d3d_buffer_;


		friend auto create_vertex_buffer(context_ptr const&, vb_usage_t, size_t data_size, void* data) -> vertex_buffer_ptr;
	};

	




	template <typename T, typename FN>
	auto vertex_buffer_t::with_map(FN f) -> void
	{
		ATMA_ASSERT(usage_ != vb_usage_t::immutable);

		switch (usage_)
		{
			case vb_usage_t::long_lived:
				f( reinterpret_cast<T*>(&shadow_buffer_[0]), reinterpret_cast<T*>(&shadow_buffer_[0] + capacity_) );
				context_->signal_d3d_buffer_upload(d3d_buffer_, &shadow_buffer_[0], (uint32_t)size_, 1);
				break;
		}
	}

#if 0
	//======================================================================
	// locked_vertex_buffer_t
	//======================================================================
	template <typename T>
	struct locked_vertex_buffer_t
	{
		locked_vertex_buffer_t(vertex_buffer_t&, lock_type_t);
		~locked_vertex_buffer_t();

		auto begin() -> T* {
			return owner_->shadowing_
			  ? reinterpret_cast<T*>(&owner_->data_.front())
			  : reinterpret_cast<T*>(d3d_resource_.pData)
			  ;
		}

		auto end() -> T* {
			return owner_->shadowing_
			  ? reinterpret_cast<T*>(&owner_->data_.front() + owner_->data_size_)
			  : reinterpret_cast<T*>(d3d_resource_.pData) + owner_->data_size_
			  ;
		}

	private:
		locked_vertex_buffer_t(locked_vertex_buffer_t const&);

		vertex_buffer_t* owner_;
		lock_type_t lock_type_;
		std::lock_guard<std::mutex> guard_;
		D3D11_MAPPED_SUBRESOURCE d3d_resource_;
	};




	template <typename T>
	inline locked_vertex_buffer_t<T>::locked_vertex_buffer_t(vertex_buffer_t& vertex_buffer, lock_type_t lock_type)
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
#endif


//======================================================================
} // namespace dust
//======================================================================
