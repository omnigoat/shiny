#pragma once

#include <shiny/compute_shader.hpp>

#include <atma/vector.hpp>
#include <atma/thread/engine.hpp>


namespace shiny
{
	namespace detail
	{
		
	}

	struct cs_dispatch_t
	{
		compute_shader_cptr shader;
		uint x, y, z;
	};

	struct bound_input_views_t
	{
		bound_input_views_t(std::initializer_list<bound_resource_view_t> list)
			: views(list.begin(), list.end())
		{}

		bound_resource_views_t views;
	};

	struct bound_compute_views_t
	{
		bound_compute_views_t(std::initializer_list<bound_resource_view_t> list)
			: views(list.begin(), list.end())
		{}

		bound_resource_views_t views;
	};

	namespace compute_commands
	{
		inline auto bind_input_views(std::initializer_list<bound_resource_view_t> views) -> bound_input_views_t
		{
			return bound_input_views_t{views};
		}

		inline auto bind_compute_views(std::initializer_list<bound_resource_view_t> views) -> bound_compute_views_t
		{
			return bound_compute_views_t{views};
		}

		inline auto dispatch(compute_shader_cptr const& cs, uint x, uint y, uint z) -> cs_dispatch_t
		{
			return cs_dispatch_t{cs, x, y, z};
		}
	}



	namespace detail
	{
		using queue_t = atma::thread::engine_t::queue_t;

		auto generate_compute_prelude(queue_t::batch_t&, context_ptr const&) -> void;
		auto generate_compute_command(queue_t::batch_t&, context_ptr const&, bound_input_views_t const&) -> void;
		auto generate_compute_command(queue_t::batch_t&, context_ptr const&, bound_compute_views_t const&) -> void;
		auto generate_compute_command(queue_t::batch_t&, context_ptr const&, cs_dispatch_t const&) -> void;
	}


	auto signal_compute(context_ptr const&, atma::thread::engine_t::queue_t::batch_t&) -> void;

	template <typename T, typename... Args>
	inline auto signal_compute(context_ptr const& ctx, atma::thread::engine_t::queue_t::batch_t& batch, T&& t, Args&&... args) -> void
	{
		detail::generate_compute_command(batch, ctx, std::forward<T>(t));
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
