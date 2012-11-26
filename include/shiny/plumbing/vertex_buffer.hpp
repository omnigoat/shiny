#ifndef SHINY_PLUMBING_VERTEX_BUFFER_HPP
#define SHINY_PLUMBING_VERTEX_BUFFER_HPP
//======================================================================
#include <d3d11.h>
//======================================================================
namespace shiny {
namespace plumbing {
//======================================================================
	
	struct vertex_buffer_t
	{
		enum class usage {
			general,
			updated_often,
			immutable,
			staging
		};

		struct lock_t;

		vertex_buffer_t(usage use, void* data, unsigned int data_size, bool shadow);


	private:
		usage use_;
		void* data_;
		unsigned int data_size_;
		bool shadowing_;

		ID3D11Buffer* d3d_buffer_;
	};

	struct vertex_buffer_t::lock_t
	{
		friend struct vertex_buffer_t;

		// lock_t is movable
		lock_t(lock_t&&);

		bool valid() const;

	private:
		// constructable by friends
		lock_t(vertex_buffer_t* owner, void* data, unsigned int data_size);
		// noncopyable
		lock_t(const lock_t&);

		vertex_buffer_t* owner_;
		void* data_;
		unsigned int data_size_;
		D3D11_MAPPED_SUBRESOURCE d3d_resource_;
	};

//======================================================================
} // namespace plumbing
} // namespace shiny
//======================================================================
#endif
//======================================================================
