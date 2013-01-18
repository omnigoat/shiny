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
//======================================================================
namespace shiny {
namespace plumbing {
//======================================================================

	//======================================================================
	// vertex_buffer_t
	//======================================================================
	struct vertex_buffer_t
	{
		// lock_t, for accessing the data.
		template <typename T> struct lock_t;

		enum class usage {
			general,
			updated_often,
			immutable,
			staging
		};

		
		template <typename T> auto lock() -> lock_t<T>;
		template <typename T> auto lock() const -> lock_t<T const>;

		auto reload_from_shadow_buffer() -> void;
		auto release_shadow_buffer() -> void;
		auto aquire_shadow_buffer() -> void;

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

		friend struct device_t;
	};


	//======================================================================
	// vertex_buffer_t::lock_t
	//======================================================================
	template <typename T>
	struct vertex_buffer_t::lock_t
	{
		// lock_t is movable
		lock_t(lock_t&&);
		~lock_t();

		auto valid() const -> bool;
		auto begin() -> T*;
		auto end() -> T*;

	private:
		// constructable by friends
		lock_t(context_t&, vertex_buffer_t* owner, unsigned int data_size);
		// noncopyable
		lock_t(const lock_t&);

		context_t& context_;
		vertex_buffer_t* owner_;
		unsigned int data_size_;
		//ID3D11Device* d3d_device_;
		D3D11_MAPPED_SUBRESOURCE d3d_resource_;

		friend struct vertex_buffer_t;
	};







	//======================================================================
	// implementation - vertex_buffer_t
	//======================================================================
	template <typename T>
	auto vertex_buffer_t::lock() -> vertex_buffer_t::lock_t<T>
	{
		return lock_t<T>(context_, this, data_size_);
	}






	//======================================================================
	// implementation - vertex_buffer_t::lock_t
	//======================================================================
	template <typename T>
	vertex_buffer_t::lock_t<T>::lock_t(context_t& context, vertex_buffer_t* owner, unsigned int data_size )
	: context_(context), owner_(owner), data_size_(data_size)
	{
		if (!owner_->shadowing_) {
			HRESULT hr = context_.get()->Map(owner_->d3d_buffer_, 0, D3D11_MAP_WRITE, 0, &d3d_resource_);
			ATMA_ASSERT(hr == S_OK);
		}
		
		owner_->locked_ = true;
	}
	
	template <typename T>
	vertex_buffer_t::lock_t<T>::lock_t(lock_t&& rhs)
	: owner_(rhs.owner_), data_size_(rhs.data_size_), d3d_resource_(std::move(rhs.d3d_resource_))
	{
	}

	template <typename T>
	vertex_buffer_t::lock_t<T>::~lock_t()
	{
		// if we are shadowing, that means all data written was written into our shadow
		// buffer. we will now update the d3d buffer from our shadow buffer.
		if (owner_->shadowing_) {
			HRESULT hr = context_.get()->Map(owner_->d3d_buffer_, 0, D3D11_MAP_WRITE, 0, &d3d_resource_);
			ATMA_ASSERT(hr == S_OK);
			memcpy(d3d_resource_.pData, &owner_->data_.front(), data_size_);
		}
		
		context_.get()->Unmap(owner_->d3d_buffer_, 0);
		
		owner_->locked_ = false;
	}

	template <typename T>
	auto vertex_buffer_t::lock_t<T>::valid() const -> bool {
		return d3d_device_;
	}

	template <typename T>
	auto vertex_buffer_t::lock_t<T>::begin() -> T* {
		return owner_->shadowing_
		  ? &owner_->data_.front()
		  : reinterpret_cast<T*>(d3d_resource_.pData)
		  ;
	}

	template <typename T>
	auto vertex_buffer_t::lock_t<T>::end() -> T* {
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
