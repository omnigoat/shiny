#pragma once

#include <shiny/shiny_fwd.hpp>
#include <shiny/resource.hpp>
#include <shiny/platform/win32/d3d_fwd.hpp>

#include <atma/aligned_allocator.hpp>

#include <vector>


namespace shiny
{
	namespace detail
	{
		template <typename T>
		using typed_shadow_buffer_t = std::vector<T, atma::aligned_allocator_t<T, 16>>;

		using shadow_buffer_t = typed_shadow_buffer_t<char>;




		struct gen_view_t
		{
			gen_view_t(bool, gpu_access_t, element_format_t, resource_subset_t)
				{} // SERIOUSLY, do this

			bool generate;
			gpu_access_t gpu_access;
			element_format_t element_format;
			resource_subset_t subset;
		};


		template <typename T>
		struct buf_vtable_t
		{
			static auto data_ptr(void const* buf) -> void const*
			{
				auto buf2 = reinterpret_cast<typed_shadow_buffer_t<T>&>(*buf);
				return &buf2[0];
			}

			static auto copy(shadow_buffer_t& dest, void const* buf) -> void
			{
				auto const& buf2 = *reinterpret_cast<typed_shadow_buffer_t<T> const*>(buf);
				dest = buf2;
			}

			static auto move(shadow_buffer_t& dest, void const* buf) -> void
			{
				auto buf2 = reinterpret_cast<typed_shadow_buffer_t<T>&>(*buf);
				dest = std::move(buf2);
			}
		};

		struct buffer_allocation_t
		{
			template <typename T>
			buffer_allocation_t(size_t stride, size_t count, T&& t)
				: stride(stride), count(count), buf_(&t)
			{
				init_vtable(std::forward<T>(t));
			}

			size_t stride;
			size_t count;

		private:
			template <typename T>
			auto init_vtable(typed_shadow_buffer_t<T>& lvalue) -> void
			{
				xfer_ = &buf_vtable_t<T>::copy;
			}

		private:
			auto (*xfer_)(shadow_buffer_t&, void const*) -> void;
			//auto (*cp_)(shadow_buffer_t&, void const*) -> void;
			auto (*dp_)(void const*) -> void const*;

			void* buf_;
		};

		
	}

	struct gen_default_read_view_t : detail::gen_view_t
	{
		gen_default_read_view_t(element_format_t ef = element_format_t::unknown, resource_subset_t rs = resource_subset_t::whole)
			: detail::gen_view_t{true, gpu_access_t::read, ef, rs}
		{}
	};

	struct gen_default_read_write_view_t : detail::gen_view_t
	{
		gen_default_read_write_view_t(element_format_t ef = element_format_t::unknown, resource_subset_t const& rs = resource_subset_t::whole)
			: detail::gen_view_t{true, gpu_access_t::read_write, ef, rs}
		{}
	};






	struct buffer_t : resource_t
	{
		template <typename T>
		using typed_shadow_buffer_t = std::vector<T, atma::aligned_allocator_t<T, 16>>;
		using shadow_buffer_t = typed_shadow_buffer_t<char>;

		buffer_t(context_ptr const&, resource_type_t, resource_usage_mask_t, buffer_usage_t, size_t element_stride, size_t element_count, void const* data, size_t data_element_count);
		virtual ~buffer_t();

		auto buffer_usage() const -> buffer_usage_t { return buffer_usage_; }
		auto is_shadowing() const -> bool { return !shadow_buffer_.empty(); }
		
		auto default_read_view() const -> resource_view_ptr const& { return default_read_view_; }
		auto default_read_write_view() const -> resource_view_ptr const& { return default_read_write_view_; }

		auto bind(gen_default_read_view_t const&) -> void;
		auto bind(gen_default_read_write_view_t const&) -> void;

		auto d3d_buffer() const -> platform::d3d_buffer_ptr const& { return d3d_buffer_; }
		auto d3d_resource() const -> platform::d3d_resource_ptr override { return d3d_buffer_; }

	protected:
		auto upload_shadow_buffer() -> void;

	protected:
		buffer_usage_t buffer_usage_;

		shadow_buffer_t shadow_buffer_;

		resource_view_ptr default_read_view_;
		resource_view_ptr default_read_write_view_;

		platform::d3d_buffer_ptr d3d_buffer_;
	};


	template <typename... Args>
	inline auto make_buffer(context_ptr const& ctx,
		resource_type_t type,
		resource_usage_mask_t rum,
		buffer_usage_t bu,
		size_t element_stride, size_t element_count, void const* data, size_t data_element_count,
		Args&&... req_views) -> buffer_ptr
	{
		auto b = atma::make_intrusive_ptr<buffer_t>(ctx, type, rum, bu, element_stride, element_count, data, data_element_count);
		int _[] = { 0, (b->bind(req_views), 0)... };
		return b;
	}

}
