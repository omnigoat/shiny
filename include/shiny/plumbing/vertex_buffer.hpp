#ifndef SHINY_PLUMBING_VERTEX_BUFFER_HPP
#define SHINY_PLUMBING_VERTEX_BUFFER_HPP
//======================================================================
#include <d3d11.h>
//======================================================================
#include <vector>
//======================================================================
#include <atma/assert.hpp>
//======================================================================
#include <shiny/voodoo/resources.hpp>
//======================================================================
#include <shiny/plumbing/lock.hpp>
#include <shiny/plumbing/prime_thread.hpp>
#include <shiny/plumbing/commands/map_unmap_copy.hpp>
//======================================================================
namespace shiny {
namespace plumbing {
//======================================================================
	
	using voodoo::gpu_access_t;
	using voodoo::cpu_access_t;

	//======================================================================
	// vertex_buffer_t
	//======================================================================
	struct vertex_buffer_t
	{
		typedef std::vector<char> data_t;

		vertex_buffer_t(gpu_access_t, cpu_access_t, bool shadow, unsigned int data_size);
		vertex_buffer_t(gpu_access_t, cpu_access_t, bool shadow, unsigned int data_size, void* data);
		vertex_buffer_t(gpu_access_t, cpu_access_t, bool shadow, data_t const& data);
		vertex_buffer_t(gpu_access_t, cpu_access_t, bool shadow, data_t&& data);
		~vertex_buffer_t();

		template <typename T> auto lock(lock_type_t) -> lock_t<vertex_buffer_t, T>;
		template <typename T> auto lock(lock_type_t) const -> lock_t<vertex_buffer_t, T const>;

		auto is_shadowing() const -> bool;
		auto reload_from_shadow_buffer() -> void;
		auto release_shadow_buffer() -> void;
		auto aquire_shadow_buffer(bool pull_from_hardware = true) -> void;
		auto rebase_from_buffer(data_t&&) -> void;
		auto rebase_from_buffer(data_t const&) -> void;

	private:
		ID3D11Buffer* d3d_buffer_;
		gpu_access_t gpu_access_;
		cpu_access_t cpu_access_;
		unsigned int data_size_;
		std::vector<char> data_;
		bool shadowing_;
		std::atomic_bool locked_;
		
		
		friend struct lock_t<vertex_buffer_t, char>;
	};


	//======================================================================
	// specialisation of lock_t for vertex buffers
	//======================================================================
	template <typename T>
	struct lock_t<vertex_buffer_t, T>
	{
		// lock_t is movable
		lock_t(lock_t&&);
		~lock_t();

		auto valid() const -> bool;
		auto begin() -> T*;
		auto end() -> T*;

	private:
		// constructable by friends
		lock_t(vertex_buffer_t* owner, lock_type_t lock_type);
		// noncopyable
		lock_t(const lock_t&);

		vertex_buffer_t* owner_;
		lock_type_t lock_type_;
		D3D11_MAPPED_SUBRESOURCE d3d_resource_;

		friend struct vertex_buffer_t;
	};







	//======================================================================
	// implementation - vertex_buffer_t
	//======================================================================
	template <typename T>
	auto vertex_buffer_t::lock(lock_type_t lock_type) -> lock_t<vertex_buffer_t, T>
	{
		ATMA_ASSERT(!locked_.load());
		return lock_t<vertex_buffer_t, T>(this, lock_type);
	}






	//======================================================================
	// implementation - vertex_buffer_t::lock_t
	//======================================================================
	template <typename T>
	lock_t<vertex_buffer_t, T>::lock_t(vertex_buffer_t* owner, lock_type_t lock_type )
	: owner_(owner), lock_type_(lock_type)
	{
		owner_->locked_.store(true);

		if (!owner_->shadowing_) {
			D3D11_MAP map_type;
			switch (lock_type_) {
				case lock_type_t::read: map_type = D3D11_MAP_READ; break;
				case lock_type_t::write: map_type = D3D11_MAP_WRITE; break;
				case lock_type_t::read_write: map_type = D3D11_MAP_READ_WRITE; break;
				case lock_type_t::write_discard: map_type = D3D11_MAP_WRITE_DISCARD; break;
			}

			voodoo::map(owner_->d3d_buffer_, &d3d_resource_, map_type, 0);
		}
	}

	template <typename T>
	lock_t<vertex_buffer_t, T>::~lock_t()
	{
		// if we are shadowing, that means all data written was written into our shadow
		// buffer. we will now update the d3d buffer from our shadow buffer.
		if (owner_->shadowing_) {
			voodoo::map(owner_->d3d_buffer_, &d3d_resource_, D3D11_MAP_WRITE_DISCARD, 0);
			memcpy(&owner_->data_.front(), d3d_resource_.pData, owner_->data_size_);
		}
		
		voodoo::unmap(owner_->d3d_buffer_, 0);
	}

	template <typename T>
	auto lock_t<vertex_buffer_t, T>::valid() const -> bool {
		return d3d_device_;
	}

	template <typename T>
	auto lock_t<vertex_buffer_t, T>::begin() -> T* {
		return owner_->shadowing_
		  ? &owner_->data_.front()
		  : reinterpret_cast<T*>(d3d_resource_.pData)
		  ;
	}

	template <typename T>
	auto lock_t<vertex_buffer_t, T>::end() -> T* {
		return owner_->shadowing_
		  ? &owner_->data_.front() + data_size_
		  : reinterpret_cast<T*>(d3d_resource_.pData) + data_size_
		  ;
	}



//======================================================================
} // namespace plumbing
} // namespace shiny
//======================================================================
#endif
//======================================================================
