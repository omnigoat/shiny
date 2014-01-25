#ifndef DUST_PLUMBING_INDEX_BUFFER_HPP
#define DUST_PLUMBING_INDEX_BUFFER_HPP
//======================================================================
#include <dust/lock.hpp>
//======================================================================
#include <dust/resources.hpp>
//======================================================================
#include <atma/assert.hpp>
//======================================================================
#include <d3d11.h>
//======================================================================
#include <vector>
#include <thread>
//======================================================================
namespace dust {
namespace plumbing {
//======================================================================
	
	using voodoo::gpu_access_t;
	using voodoo::cpu_access_t;

	//======================================================================
	// index_buffer_t
	//======================================================================
	struct index_buffer_t
	{
		typedef std::vector<char> data_t;

		index_buffer_t(gpu_access_t, cpu_access_t, bool shadow, uint32_t data_size);
		index_buffer_t(gpu_access_t, cpu_access_t, bool shadow, uint32_t data_size, void* data);
		index_buffer_t(gpu_access_t, cpu_access_t, bool shadow, data_t const& data);
		index_buffer_t(gpu_access_t, cpu_access_t, bool shadow, data_t&& data);
		~index_buffer_t();

		template <typename T> auto lock(lock_type_t) -> lock_t<index_buffer_t, T>;
		template <typename T> auto lock(lock_type_t) const -> lock_t<index_buffer_t, T const>;

		auto is_shadowing() const -> bool;
		auto reload_from_shadow_buffer() -> void;
		auto release_shadow_buffer() -> void;
		auto aquire_shadow_buffer(bool pull_from_hardware = true) -> void;
		auto rebase_from_buffer(data_t&&, bool upload_to_hardware = true) -> void;
		auto rebase_from_buffer(data_t const&, bool upload_to_hardware = true) -> void;

	private:
		ID3D11Buffer* d3d_buffer_;
		gpu_access_t gpu_access_;
		cpu_access_t cpu_access_;
		uint32_t data_size_;
		std::vector<char> data_;
		bool shadowing_;
		std::mutex mutex_;

		friend struct locked_index_buffer_t;
	};



	//======================================================================
	// locked_index_buffer_t
	//======================================================================
	struct locked_index_buffer_t
	{
		locked_index_buffer_t(index_buffer_t&, lock_type_t);
		~locked_index_buffer_t();

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
		locked_index_buffer_t(locked_index_buffer_t const&);

		index_buffer_t* owner_;
		lock_type_t lock_type_;
		std::lock_guard<std::mutex> guard_;
		D3D11_MAPPED_SUBRESOURCE d3d_resource_;
	};



//======================================================================
} // namespace plumbing
} // namespace dust
//======================================================================
#endif
//======================================================================
