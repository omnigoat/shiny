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
		bool shadow_;

		ID3D11Buffer* d3d_buffer_;
	};

	struct vertex_buffer_t::lock_t
	{
		friend struct vertex_buffer_t;

		// noncopyable, but movable
		lock_t(const lock_t&) = delete;
		lock_t(lock_t&&);


	private:
		lock_t(vertex_buffer_t* owner, void* data, unsigned int data_size);

		vertex_buffer_t* owner_;
		void* data_;
		unsigned int data_size_;
	};

//======================================================================
} // namespace plumbing
} // namespace shiny
//======================================================================
#endif
//======================================================================
