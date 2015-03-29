#pragma once

#include <shiny/shiny_fwd.hpp>

#include <atma/types.hpp>
#include <atma/thread/engine.hpp>
#include <atma/math/vector4f.hpp>

namespace shiny {

	struct rendertarget_clear_t
	{
		rendertarget_clear_t()
			: clear_{}
		{}

		rendertarget_clear_t(aml::vector4f const& color)
			: color_{color}
			, clear_{true}
		{}

		rendertarget_clear_t(float r, float g, float b)
			: color_{r, g, b, 0.f}
			, clear_{true}
		{}

		auto clear() const -> bool { return clear_; }
		auto color() const -> aml::vector4f const& { return color_; }

	private:
		aml::vector4f color_;
		bool clear_;
	};




	struct scene_t
	{
		scene_t(context_ptr const&, camera_t const&, rendertarget_clear_t const& = rendertarget_clear_t{});

		auto scene_buffer() const -> constant_buffer_ptr const& { return scene_buffer_; }

		auto draw_batch() -> atma::thread::engine_t::queue_t::batch_t& { return batch_; }

	private:
		auto execute() -> void;

	private:
		context_ptr context_;
		constant_buffer_ptr scene_buffer_;

		atma::thread::engine_t::queue_t queue_;
		atma::thread::engine_t::queue_t::batch_t batch_;

		friend struct context_t;
	};

}
