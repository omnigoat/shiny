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
#include <atma/lockfree/queue.hpp>
//======================================================================
namespace shiny {
namespace plumbing {
//======================================================================
//template <typename resource_tm, typename element_tm> struct lock_t;
//struct vertex_buffer_t;
//======================================================================
	struct command_t;

	//======================================================================
	// the devil lies here.
	//======================================================================
	namespace detail {
		// the prime thread is the thread upon which all commands are consumed.
		// this thread is also where @d3d_immediate_context_ resides.
		extern std::thread prime_thread_;
		extern std::atomic_bool prime_thread_running_;

		// this is the command-queue which through all commands reach the prime thread.
		typedef atma::lockfree::queue_t<command_t*> command_queue_t;
		extern command_queue_t command_queue_;
		//extern std::unique_mutex command_queue_mutex_;

		// there is one global d3d device, and one immeidate context
		extern ID3D11Device* d3d_device_;
		extern ID3D11DeviceContext* d3d_immediate_context_;
	}

	
	//======================================================================
	// device management
	//======================================================================
	namespace detail
	{
		auto setup_d3d_device() -> void;
		auto teardown_d3d_device() -> void;
	}


	//======================================================================
	// context implementation
	//======================================================================
	/*namespace detail {
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
	}*/



//======================================================================
} // namespace plumbing
} // namespace shiny
//======================================================================
#endif
//======================================================================
