#pragma once

#include <shiny/compute_shader.hpp>
#include <shiny/resource_view.hpp>

#include <atma/vector.hpp>
#include <atma/thread/engine.hpp>


namespace shiny
{
	struct compute_context_t
	{
		compute_context_t(renderer_ptr const& rndr, bound_constant_buffers_t const& cbs, bound_input_views_t const& ivs, bound_compute_views_t const& cvs)
			: rndr_{rndr}, cbs_{cbs}, ivs_{ivs}, cvs_{cvs}
		{}

		//compute_context_t(compute_context_t const&) = delete;
		//compute_context_t(compute_context_t&&) = delete;

		~compute_context_t();

		auto constant_buffers() const -> bound_constant_buffers_t const& { return cbs_; }
		auto input_views() const -> bound_input_views_t const& { return ivs_; }
		auto compute_views() const -> bound_compute_views_t const& { return cvs_; }

		auto signal_dispatch(compute_shader_cptr const&, uint32 dims) -> compute_context_t&;
		auto signal_dispatch(compute_shader_cptr const&, uint32 x, uint32 y, uint32 z) -> compute_context_t&;

	private:
		renderer_ptr rndr_;

		bound_constant_buffers_t cbs_;
		bound_input_views_t ivs_;
		bound_compute_views_t cvs_;
	};
}
