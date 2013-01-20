#ifndef SHINY_PLUMBING_VERTEX_BUFFER_HPP
#define SHINY_PLUMBING_VERTEX_BUFFER_HPP
//======================================================================
#include <d3d11.h>
//======================================================================
#include <vector>
//======================================================================
#include <atma/assert.hpp>
//======================================================================
#include <shiny/plumbing/device.hpp>
#include <shiny/plumbing/lock.hpp>
//======================================================================
namespace shiny {
namespace plumbing {
//======================================================================
	

	//======================================================================
	// vertex_buffer_t
	//======================================================================
	struct vertex_buffer_t
	{
		enum class usage {
			general,
			updated_often,
			immutable,
			staging
		};
		
		template <typename T> auto lock(lock_type_t) -> lock_t<vertex_buffer_t, T>;
		template <typename T> auto lock(lock_type_t) const -> lock_t<vertex_buffer_t, T const>;

		auto reload_from_shadow_buffer() -> void;
		auto release_shadow_buffer() -> void;
		auto aquire_shadow_buffer(bool pull_from_hardware = true) -> void;

	private:
		vertex_buffer_t(usage use, unsigned int data_size, bool shadow);

	private:
		context_t& context_;
		
		usage use_;
		std::vector<char> data_;
		unsigned int data_size_;
		bool shadowing_;
		bool locked_;
		
		ID3D11Buffer* d3d_buffer_;

		friend struct context_t;
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
		lock_t(context_t&, vertex_buffer_t* owner, lock_type_t lock_type);
		// noncopyable
		lock_t(const lock_t&);

		context_t& context_;
		vertex_buffer_t* owner_;
		lock_type_t lock_type_;
		D3D11_MAPPED_SUBRESOURCE d3d_resource_;

		friend struct vertex_buffer_t;
	};







	//======================================================================
	// implementation - vertex_buffer_t
	//======================================================================
	template <typename T>
	auto vertex_buffer_t::lock(lock_type_t lock_type) -> lock_t<vertex_buffer_t, T> {
		return lock_t<vertex_buffer_t, T>(context_, this, lock_type);
	}






	//======================================================================
	// implementation - vertex_buffer_t::lock_t
	//======================================================================
	template <typename T>
	lock_t<vertex_buffer_t, T>::lock_t(context_t& context, vertex_buffer_t* owner, lock_type_t lock_type )
	: context_(context), owner_(owner), lock_type_(lock_type)
	{
		if (!owner_->shadowing_) {
			D3D11_MAP map_type;
			switch (lock_type_) {
				case lock_type_t::read: map_type = D3D11_MAP_READ; break;
				case lock_type_t::write: map_type = D3D11_MAP_WRITE; break;
				case lock_type_t::read_write: map_type = D3D11_MAP_READ_WRITE; break;
				case lock_type_t::write_discard: map_type = D3D11_MAP_WRITE_DISCARD; break;
			}

			detail::map_resource(owner_->d3d_buffer_, 0, map_type, 0, &d3d_resource_);
		}
		
		owner_->locked_ = true;
	}
	
	template <typename T>
	lock_t<vertex_buffer_t, T>::lock_t(lock_t&& rhs)
	: owner_(rhs.owner_), data_size_(rhs.data_size_), d3d_resource_(std::move(rhs.d3d_resource_))
	{
	}

	template <typename T>
	lock_t<vertex_buffer_t, T>::~lock_t()
	{
		detail::immediate_context_guard_t guard(detail::immediate_context_mutex_);

		// if we are shadowing, that means all data written was written into our shadow
		// buffer. we will now update the d3d buffer from our shadow buffer.
		if (owner_->shadowing_) {
			detail::map_resource(owner_->d3d_buffer_, 0, D3D11_MAP_WRITE, 0, &d3d_resource_);
			std::copy_n(&owner_->data_.front(), owner_->data_size_, reinterpret_cast<char*>(d3d_resource_.pData));
		}
		
		detail::unmap_resource(owner_->d3d_buffer_, 0);
		owner_->locked_ = false;
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
