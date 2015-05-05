#pragma once

#include <shiny/compute_shader.hpp>

#include <atma/vector.hpp>


namespace shiny
{
	namespace detail
	{
		
	}

	struct cs_execute_t
	{
		compute_shader_cptr shader_;
		uint x, y, z;
	};

	struct bound_shader_views_t
	{
		bound_shader_views_t(std::initializer_list<bound_resource_view_t> list)
			: views_(list.begin(), list.end())
		{}

		bound_resource_views_t views_;
	};

	struct bound_compute_views_t
	{
		bound_shader_views_t(std::initializer_list<bound_resource_view_t> list)
			: views_(list.begin(), list.end())
		{}

		bound_resource_views_t views_;
	};




	auto signal_compute(context_ptr const&, atma::thread::engine_t::queue_t::batch_t&) -> void;

	template <typename T, typename... Args>
	inline auto signal_compute(context_ptr const& ctx, atma::thread::engine_t::queue_t::batch_t& batch, T&& t, Args&&... args) -> void
	{
		detail::generate_command(batch, ctx, std::forward<T>(t));
		signal_compute(ctx, batch, std::forward<Args>(args)...);
	}

	template <typename... Args>
	inline auto signal_compute(context_ptr const& ctx, Args&&... args) -> void
	{
		atma::thread::engine_t::queue_t::batch_t batch;
		detail::generate_draw_prelude(batch, ctx);
		signal_compute(ctx, batch, std::forward<Args>(args)...);
	}
}
