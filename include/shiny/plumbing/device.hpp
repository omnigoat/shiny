#ifndef SHINY_PLUMBING_DEVICE_HPP
#define SHINY_PLUMBING_DEVICE_HPP
//======================================================================
#include <d3d11.h>
//======================================================================
namespace shiny {
namespace plumbing {
//======================================================================
	
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

	private:
		
	private:
		static ID3D11Device* device_;
		static ID3D11DeviceContext* immediate_context_;
	};

//======================================================================
} // namespace plumbing
} // namespace shiny
//======================================================================
#endif
//======================================================================
