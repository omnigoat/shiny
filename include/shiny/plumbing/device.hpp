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
		extern std::mutex immediate_context_mutex_;

		// which thread was the device created on
		extern std::thread::id device_creation_thread_id_;
	}

	enum class lock_type_t
	{
		read,
		write,
		read_write,
		write_discard
	};

	//======================================================================
	// a context is a per-thread instantiation that provides access to D3D
	//======================================================================
	struct context_t
	{
		context_t();
		~context_t();

		auto get() -> ID3D11DeviceContext* { return d3d_context_; }
		

	protected:
		auto map(vertex_buffer_t&, vertex_buffer_t::lock_t&) -> void;
		auto unmap(vertex_buffer_t&) -> void;

		friend struct vertex_buffer_t::lock_t;

	private:
		ID3D11DeviceContext* d3d_context_;
		
		// resources are not necessarily mapped with our d3d_context_, as some
		// operations must only be performed with the immediate context. this
		// maps resources to the context which mapped them initially
		std::map<ID3D11Resource*, ID3D11DeviceContext*> mapped_resources_;

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
