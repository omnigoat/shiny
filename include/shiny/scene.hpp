#pragma once

#include <shiny/shiny_fwd.hpp>
#include <shiny/draw.hpp>

#include <shiny/rendertarget_clear.hpp>

#include <atma/types.hpp>
#include <atma/thread/engine.hpp>
#include <atma/math/vector4f.hpp>


namespace shiny
{
	struct scene_t
	{
		scene_t(context_ptr const&, camera_t const&, rendertarget_clear_t const&);

		auto scene_constant_buffer() const -> constant_buffer_ptr const& { return scene_constant_buffer_; }

		auto context() const -> context_ptr const& { return context_; }
		auto camera() const -> camera_t const& { return *camera_; }

		template <typename... Stages>
		auto draw(Stages&&... stages) -> void;

	private:
		context_ptr context_;
		constant_buffer_ptr scene_constant_buffer_;
		camera_t const* camera_;

		atma::thread::engine_t::queue_t queue_;
		atma::thread::engine_t::queue_t::batch_t batch_;

		friend struct context_t;
	};


	template <typename... Stages>
	auto scene_t::draw(Stages&&... stages) -> void
	{
		shiny::signal_draw(context_, batch_, std::forward<Stages>(stages)...);
	}

}
