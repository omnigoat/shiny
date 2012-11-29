#ifndef SHINY_PLUMBING_VERTEX_BUFFER_HPP
#define SHINY_PLUMBING_VERTEX_BUFFER_HPP
//======================================================================
#include <d3d11.h>
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

		// lock_t, for accessing the data.
		template <typename T> struct lock_t;

		template <typename T>
		lock_t<T>&& lock();

	private:
		vertex_buffer_t(ID3D11Device* device, usage use, unsigned int data_size, bool shadow);


	private:
		usage use_;
		void* data_;
		unsigned int data_size_;
		bool shadowing_;
		bool locked_;

		ID3D11Device* d3d_device_;
		ID3D11Buffer* d3d_buffer_;
	};


	//======================================================================
	// vertex_buffer_t::lock_t
	//======================================================================
	template <typename T>
	struct vertex_buffer_t::lock_t
	{
		friend struct vertex_buffer_t;

		// lock_t is movable
		lock_t(lock_t&&);
		~lock_t();

		bool valid() const;

		T* begin();
		T* end();

	private:
		// constructable by friends
		lock_t(ID3D11Device* device, vertex_buffer_t* owner, unsigned int data_size);
		// noncopyable
		lock_t(const lock_t&);

		vertex_buffer_t* owner_;
		unsigned int data_size_;

		ID3D11Device* d3d_device_;
		D3D11_MAPPED_SUBRESOURCE d3d_resource_;
	};







	//======================================================================
	// implementation - vertex_buffer_t
	//======================================================================
	template <typename T>
	vertex_buffer_t::lock_t<T>&& vertex_buffer_t::lock()
	{
		return lock_t(d3d_device_, this);
	}






	//======================================================================
	// implementation - vertex_buffer_t::lock_t
	//======================================================================
	template <typename T>
	vertex_buffer_t::lock_t<T>::lock_t( ID3D11Device* device, vertex_buffer_t* owner, unsigned int data_size )
	 : owner_(owner), data_(data), data_size_(data_size), d3d_device_(device)
	{
		// ASSERT(!owner->locked_)
		
		if (d3d_device_->Map(owner_->d3d_buffer_, 0, D3D11_MAP_WRITE, 0, &d3d_resource_) != S_OK) {
			d3d_device_ = nullptr;
		}
		else {
			owner_->locked_ = true;
		}
	}
	
	template <typename T>
	vertex_buffer_t::lock_t<T>::lock_t(lock_t&& rhs)
	 : owner_(rhs.owner_), data_size_(rhs.data_size_), d3d_device_(rhs.d3d_device_), d3d_resource_(std::move(rhs.d3d_resource_))
	{
		rhs.d3d_device_ = nullptr;
	}

	template <typename T>
	vertex_buffer_t::lock_t<T>::~lock_t()
	{
		device_->Unmap(d3d_resource_, 0);
		owner_->locked_ = false;
	}

	template <typename T>
	bool vertex_buffer_t::lock_t<T>::valid() const
	{
		return d3d_device_;
	}

	template <typename T>
	T* vertex_buffer_t::lock_t<T>::begin() {
		return reinterpret_cast<T*>(d3d_resource_.pData);
	}

	template <typename T>
	T* vertex_buffer_t::lock_t<T>::end() {
		return reinterpret_cast<T*>(d3d_resource_.pData) + data_size_;
	}



//======================================================================
} // namespace plumbing
} // namespace shiny
//======================================================================
#endif
//======================================================================
