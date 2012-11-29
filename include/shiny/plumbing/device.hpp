#ifndef SHINY_PLUMBING_DEVICE_HPP
#define SHINY_PLUMBING_DEVICE_HPP
//======================================================================
#include <d3d11.h>
//======================================================================
namespace shiny {
namespace plumbing {
//======================================================================
	
	struct vertex_buffer_t;

	struct context_t
	{
		friend struct device_t;

	private:
		context_t(device_t* device)
		 : device_(device)
		{
		}

		device_t* device_;
	};

	struct device_t
	{
		device_t();
		device_t(const device_t&);
		device_t(const device_t&&);
		device_t& operator = (const device_t&);
		device_t& operator = (const device_t&&);
		~device_t();

		vertex_buffer_t create_vertex_buffer(unsigned int size);
		
	private:
		static ID3D11Device* d3d_device_;
		static ID3D11DeviceContext* d3d_immediate_context_;
	};

//======================================================================
} // namespace plumbing
} // namespace shiny
//======================================================================
#endif
//======================================================================
