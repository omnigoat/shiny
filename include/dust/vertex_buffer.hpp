#pragma once
//======================================================================
#include <dust/lock.hpp>
#include <dust/context.hpp>

#include <atma/assert.hpp>
#include <atma/aligned_allocator.hpp>

#include <d3d11.h>

#include <vector>
#include <thread>
//======================================================================
namespace dust {
//======================================================================
	
	struct vertex_buffer_t;
	typedef atma::intrusive_ptr<vertex_buffer_t> vertex_buffer_ptr;

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
		auto size() const -> uint32_t;

		auto allocate_shadow_buffer() -> void;
		auto release_shadow_buffer() -> void;
		auto fill_shadow_buffer(data_t const&, uint32_t offset = 0) -> void;
		auto fill_shadow_buffer(data_t&&) -> void;
		auto upload_shadow_buffer(bool block = false) -> void;

	private:
		struct context_binding_t;

		vertex_buffer_t(context_ptr const&, vb_usage_t, bool shadow, uint32_t data_size, void* data);
#if 0
		vertex_buffer_t(gpu_access_t, cpu_access_t, bool shadow, uint32_t data_size);
		vertex_buffer_t(gpu_access_t, cpu_access_t, bool shadow, uint32_t data_size, void* data);
		vertex_buffer_t(gpu_access_t, cpu_access_t, bool shadow, data_t const& data);
		vertex_buffer_t(gpu_access_t, cpu_access_t, bool shadow, data_t&& data);
#endif

		~vertex_buffer_t();

	private:
		vb_usage_t usage_;
		platform::d3d_buffer_ptr d3d_buffer_;
		gpu_access_t gpu_access_;
		cpu_access_t cpu_access_;
		uint32_t data_size_;

		data_t data_;
		bool shadowing_;
		
		std::mutex mutex_;
		std::mutex inflight_mutex_;

		context_ptr context_;

		friend struct locked_vertex_buffer_t;
	};

	auto create_vertex_buffer(context_ptr const&, vb_usage_t, bool shadow, uint32_t data_size, void* data) -> vertex_buffer_ptr;



#if 0
	//======================================================================
	// locked_vertex_buffer_t
	//======================================================================
	struct locked_vertex_buffer_t
	{
		locked_vertex_buffer_t(vertex_buffer_t&, lock_type_t);
		~locked_vertex_buffer_t();

		template <typename T>
		auto begin() -> T* {
			return owner_->shadowing_
			  ? reinterpret_cast<T*>(&owner_->data_.front())
			  : reinterpret_cast<T*>(d3d_resource_.pData)
			  ;
		}

		template <typename T>
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
#endif


//======================================================================
} // namespace dust
//======================================================================
