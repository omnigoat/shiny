#ifndef SHINY_PLUMBING_DEVICE_HPP
#define SHINY_PLUMBING_DEVICE_HPP
//======================================================================
#include <thread>
#include <map>
#include <mutex>
//======================================================================
#include <d3d11.h>
//======================================================================
namespace shiny {
namespace plumbing {
//======================================================================
struct vertex_buffer_t;
//======================================================================
	

	//======================================================================
	// the devil lies here.
	//======================================================================
	namespace detail {
		// there is one global d3d device, and one immeidate context
		extern ID3D11Device* d3d_device_;
		extern ID3D11DeviceContext* d3d_immediate_context_;

		// which thread was the device created on
		extern std::thread::id device_creation_thread_id_;
	}


	//======================================================================
	// a context is a per-thread instantiation that provides access to D3D
	//======================================================================
	struct context_t
	{
		context_t();
		~context_t();

		auto get() -> ID3D11DeviceContext* { return d3d_context_; }

	private:
		ID3D11DeviceContext* d3d_context_;
		
		// contexts are strictly bound to threads. 
		static std::map<std::thread::id, context_t*> bound_contexts_;
		static std::mutex mutex_;

		friend auto this_context() -> context_t&;
	};



	//======================================================================
	// free-functions
	//======================================================================
	auto this_context() -> context_t&;
	auto device() -> ID3D11Device*;

//======================================================================
} // namespace plumbing
} // namespace shiny
//======================================================================
#endif
//======================================================================
