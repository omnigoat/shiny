#pragma once

#include <shiny/shiny_fwd.hpp>
#include <shiny/resource.hpp>
#include <shiny/platform/dx11/d3d_fwd.hpp>

#include <atma/aligned_allocator.hpp>
#include <atma/vector.hpp>


namespace shiny
{
	namespace detail
	{
		template <typename T>
		using aligned_data_t = atma::vector<T, atma::aligned_allocator_t<T, 16>>;

		struct gen_view_t
		{
			gen_view_t(bool g, gpu_access_t ga, format_t ef, resource_subset_t rs)
				: generate(g), gpu_access(ga), element_format(ef), subset(rs)
			{}

			bool generate;
			gpu_access_t gpu_access;
			format_t element_format;
			resource_subset_t subset;
		};
	}

	struct gen_primary_input_view_t : detail::gen_view_t
	{
		gen_primary_input_view_t(format_t ef = format_t::unknown, resource_subset_t rs = resource_subset_t::whole)
			: detail::gen_view_t{true, gpu_access_t::read, ef, rs}
		{}
	};

	struct gen_primary_compute_view_t : detail::gen_view_t
	{
		gen_primary_compute_view_t(format_t ef = format_t::unknown, resource_subset_t const& rs = resource_subset_t::whole)
			: detail::gen_view_t{true, gpu_access_t::read_write, ef, rs}
		{}
	};


	struct buffer_dimensions_t
	{
		buffer_dimensions_t(size_t stride, size_t count)
			: stride(stride), count(count)
		{
		}

		size_t stride;
		size_t count;

		template <typename T>
		static auto infer(detail::aligned_data_t<T> const& memory) -> buffer_dimensions_t
		{
			return buffer_dimensions_t{sizeof(T), memory.size()};
		}
	};

	struct buffer_data_t
	{
		buffer_data_t()
			: data(), size()
		{}

		buffer_data_t(void const* data, size_t size)
			: data(data), size(size)
		{}

		template <typename T>
		buffer_data_t(detail::aligned_data_t<T> const& x)
			: data(x.data()), size(x.size())
		{}

		void const* data;
		size_t size;
	};

	struct buffer_t : shiny_dx11::resource_dx11_t
	{
		template <typename T> using aligned_data_t = detail::aligned_data_t<T>;

		buffer_t(renderer_ptr const&, resource_type_t, resource_usage_mask_t, resource_storage_t, buffer_dimensions_t const&, buffer_data_t const&);
		virtual ~buffer_t();

		auto primary_input_view() const -> resource_view_ptr const& { return primary_input_view_; }
		auto primary_compute_view() const -> resource_view_ptr const& { return primary_compute_view_; }

		auto bind(gen_primary_input_view_t const&) -> void;
		auto bind(gen_primary_compute_view_t const&) -> void;

		auto d3d_buffer() const -> platform::d3d_buffer_ptr const& { return d3d_buffer_; }
		auto d3d_resource() const -> platform::d3d_resource_ptr override { return d3d_buffer_; }

	protected:
		resource_view_ptr primary_input_view_;
		resource_view_ptr primary_compute_view_;

		platform::d3d_buffer_ptr d3d_buffer_;
	};




	template <typename... Args>
	inline auto make_buffer(renderer_ptr const& rndr,
		resource_type_t rt,
		resource_usage_mask_t ru,
		resource_storage_t rs,
		buffer_dimensions_t const& bdm,
		buffer_data_t const& bdt,
		Args&&... req_views) -> buffer_ptr
	{
		auto b = atma::make_intrusive<buffer_t>(rndr, rt, ru, rs, bdm, bdt);
		int _[] = { 0, (b->bind(req_views), 0)... };
		
		return b;
	}

}
