#ifndef SHINY_PLUMBING_VERTEX_BUFFER_HPP
#define SHINY_PLUMBING_VERTEX_BUFFER_HPP
//======================================================================
#include <shiny/plumbing/lock.hpp>
#include <shiny/plumbing/prime_thread.hpp>
#include <shiny/plumbing/commands/map_unmap_copy.hpp>
//======================================================================
#include <shiny/voodoo/resources.hpp>
#include <shiny/context.hpp>
//======================================================================
#include <atma/assert.hpp>
//======================================================================
#include <d3d11.h>
//======================================================================
#include <vector>
#include <thread>
//======================================================================
namespace shiny {
namespace plumbing {
//======================================================================
	
	using voodoo::gpu_access_t;
	using voodoo::cpu_access_t;


	struct vertex_buffer_t;
	typedef atma::intrusive_ptr<vertex_buffer_t> vertex_buffer_ptr;

	//======================================================================
	// vertex_buffer_t
	//======================================================================
	struct vertex_buffer_t : atma::ref_counted<vertex_buffer_t>
	{
		typedef std::vector<char> data_t;

		static auto create(std::initializer_list<context_ptr>, gpu_access_t, cpu_access_t, bool shadow, uint32_t data_size) -> vertex_buffer_ptr;


		auto is_shadowing() const -> bool;
		auto reload_from_shadow_buffer() -> void;
		auto release_shadow_buffer() -> void;
		auto aquire_shadow_buffer(bool pull_from_hardware = true) -> void;
		auto rebase_from_buffer(data_t&&, bool upload_to_hardware = true) -> void;
		auto rebase_from_buffer(data_t const&, bool upload_to_hardware = true) -> void;

		//
		auto bind_to(context_ptr const&) -> void;

	private:
		struct context_binding_t;

		vertex_buffer_t(gpu_access_t, cpu_access_t, bool shadow, uint32_t data_size);
		vertex_buffer_t(gpu_access_t, cpu_access_t, bool shadow, uint32_t data_size, void* data);
		vertex_buffer_t(gpu_access_t, cpu_access_t, bool shadow, data_t const& data);
		vertex_buffer_t(gpu_access_t, cpu_access_t, bool shadow, data_t&& data);
		~vertex_buffer_t();

	private:
		voodoo::d3d_buffer_ptr d3d_buffer_;
		gpu_access_t gpu_access_;
		cpu_access_t cpu_access_;
		uint32_t data_size_;
		std::vector<char> data_;
		bool shadowing_;
		
		std::mutex mutex_;
		std::mutex inflight_mutex_;


		std::map<std::tuple<context_ptr, voodoo::dxgi_adapter_ptr>, voodoo::d3d_buffer_ptr> d3d_buffers_;

		friend struct locked_vertex_buffer_t;
	};



	//======================================================================
	// locked_vertex_buffer_t
	//======================================================================
	struct locked_vertex_buffer_t
	{
		locked_vertex_buffer_t(vertex_buffer_t&, lock_type_t);
		~locked_vertex_buffer_t();

		template <typename T>
		auto begin() -> T* {
			return owner_->shadowing_
			  ? reinterpret_cast<T*>(&owner_->data_.front())
			  : reinterpret_cast<T*>(d3d_resource_.pData)
			  ;
		}

		template <typename T>
		auto end() -> T* {
			return owner_->shadowing_
			  ? reinterpret_cast<T*>(&owner_->data_.front() + owner_->data_size_)
			  : reinterpret_cast<T*>(d3d_resource_.pData) + owner_->data_size_
			  ;
		}

	private:
		locked_vertex_buffer_t(locked_vertex_buffer_t const&);

		vertex_buffer_t* owner_;
		lock_type_t lock_type_;
		std::lock_guard<std::mutex> guard_;
		D3D11_MAPPED_SUBRESOURCE d3d_resource_;
	};



//======================================================================
} // namespace plumbing
} // namespace shiny
//======================================================================
#endif
//======================================================================
