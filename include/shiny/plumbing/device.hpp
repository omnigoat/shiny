#ifndef SHINY_PLUMBING_DEVICE_HPP
#define SHINY_PLUMBING_DEVICE_HPP
//======================================================================
#include <thread>
#include <map>
#include <mutex>
//======================================================================
#include <d3d11.h>
//======================================================================
#include <atma/assert.hpp>
//======================================================================
namespace shiny {
namespace plumbing {
//======================================================================
template <typename resource_tm, typename element_tm> struct lock_t;
struct vertex_buffer_t;
struct context_t;
//======================================================================
	

	//======================================================================
	// the devil lies here.
	//======================================================================
	namespace detail {
		// there is one global d3d device, and one immeidate context
		extern ID3D11Device* d3d_device_;
		extern ID3D11DeviceContext* d3d_immediate_context_;

		// the immediate context is not thread-safe, so it's guarded by a mutex.
		// it's a recursive mutex, because user code may call many detail::context_xxxx
		// functions, each of which calls .lock() - thus, to avoid locking a million
		// times in a row, user code can lock the mutex themselves for the duration
		// required.
		extern std::recursive_mutex immediate_context_mutex_;
		typedef std::lock_guard<decltype(immediate_context_mutex_)> immediate_context_guard_t;

		// contexts are strictly bound to threads. 
		extern std::map<std::thread::id, context_t*> bound_contexts_;
		extern std::mutex bound_contexts_mutex_;

		// handle to the context for this thread. this is thread-local storage. wish
		// to switch to C++11 thread_local when possible.
		__declspec(thread) extern context_t* context_handle_;
	}

	
	//======================================================================
	// context implementation
	//======================================================================
	namespace detail {
		inline auto map_resource(ID3D11Resource* resource, unsigned int subresource, D3D11_MAP map_type, unsigned int map_flags,
		  D3D11_MAPPED_SUBRESOURCE* mapped_resource) -> void {
			std::lock_guard<std::recursive_mutex> guard(immediate_context_mutex_);
			HRESULT hr = d3d_immediate_context_->Map(resource, subresource, map_type, map_flags, mapped_resource);
			ATMA_ASSERT(hr == S_OK);
		}

		inline auto unmap_resource(ID3D11Resource* resource, unsigned int subresource) -> void {
			immediate_context_guard_t guard(immediate_context_mutex_);
			d3d_immediate_context_->Unmap(resource, subresource);
		}
	}

	//======================================================================
	// a context is a per-thread instantiation that provides access to D3D
	//======================================================================
	struct context_t
	{
		context_t();
		~context_t();

	private:
		ID3D11DeviceContext* d3d_context_;
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
