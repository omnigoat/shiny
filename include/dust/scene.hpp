#pragma once
//=====================================================================
#include <dust/dust_fwd.hpp>

#include <atma/types.hpp>
#include <atma/thread/engine.hpp>
//=====================================================================
namespace dust {
//=====================================================================

	struct scene_t
	{
		scene_t(context_ptr const&, camera_t const&);

		auto signal_update_constant_buffer(constant_buffer_ptr const&, uint data_size, void*) -> void;
		auto signal_constant_buffer_upload(uint index, constant_buffer_ptr const&) -> void;

		auto signal_draw(index_buffer_ptr const&, vertex_declaration_t const&, vertex_buffer_ptr const&, vertex_shader_ptr const&, pixel_shader_ptr const&) -> void;

	private:
		auto execute() -> void;

	private:
		context_ptr context_;
		constant_buffer_ptr scene_buffer_;

		atma::thread::engine_t::queue_t queue_;

		friend struct context_t;
	};

//=====================================================================
}
//=====================================================================

