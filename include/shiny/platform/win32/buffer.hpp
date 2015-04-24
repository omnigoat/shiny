#pragma once

#include <shiny/shiny_fwd.hpp>
#include <shiny/resource.hpp>
#include <shiny/platform/win32/d3d_fwd.hpp>

#include <atma/aligned_allocator.hpp>
#include <atma/vector.hpp>


namespace shiny
{
	namespace detail
	{
		template <typename T>
		using typed_shadow_buffer_t = atma::vector<T, atma::aligned_allocator_t<T, 16>>;

		using shadow_buffer_t = typed_shadow_buffer_t<byte>;




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
		struct buffer_data_vtable_t;

		template <typename T>
		struct buffer_data_vtable_t<typed_shadow_buffer_t<T>>
		{
			static auto shadowbuffer_copy(shadow_buffer_t& dest, void const* srcptr, size_t size) -> void
			{
				auto const& src = *reinterpret_cast<typed_shadow_buffer_t<T> const*>(srcptr);
				dest.assign(src.begin(), src.end());
			}

			static auto shadowbuffer_move(shadow_buffer_t& dest, void const* srcptr, size_t size) -> void
			{
				auto const& csrc = *reinterpret_cast<typed_shadow_buffer_t<T> const*>(srcptr);
				auto& src = const_cast<typed_shadow_buffer_t<T>&>(csrc);
				dest.attach_buffer(src.detach_buffer());
			}

			static auto vram_upload(void const*& dest, void const* srcptr, size_t size) -> void
			{
				auto const& csrc = *reinterpret_cast<typed_shadow_buffer_t<T> const*>(srcptr);
				dest = csrc.data();
			}

			static auto vram_postupload_move(void const* srcptr, size_t size) -> void
			{
				auto const& csrc = *reinterpret_cast<typed_shadow_buffer_t<T> const*>(srcptr);
				auto& src = const_cast<typed_shadow_buffer_t<T>&>(csrc);
				src.detach_buffer();
			}
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


	struct buffer_dimensions_t
	{
		buffer_dimensions_t(size_t stride, size_t count)
			: stride(stride), count(count)
		{
		}

		size_t stride;
		size_t count;

		template <typename T>
		static auto infer(detail::typed_shadow_buffer_t<T> const& memory) -> buffer_dimensions_t
		{
			return buffer_dimensions_t{sizeof(T), memory.size()};
		}
	};

	struct buffer_data_t
	{
		enum class ownership_t
		{
			copy,
			move,
		};

		static auto copy(void const*, size_t) -> buffer_data_t
		{
			return buffer_data_t{ownership_t::copy, nullptr, 0};
		}

		template <typename T>
		static auto copy(detail::typed_shadow_buffer_t<T>& buf) -> buffer_data_t
		{
			auto r = buffer_data_t{ownership_t::move, &buf, buf.size()};
			r.fn_shadowbuffer_ = &detail::buffer_data_vtable_t<detail::typed_shadow_buffer_t<T>>::shadowbuffer_copy;
			r.fn_vram_ = &detail::buffer_data_vtable_t<detail::typed_shadow_buffer_t<T>>::vram_upload;
			r.fn_vram_post_ = &detail::buffer_data_vtable_t<detail::typed_shadow_buffer_t<T>>::vram_postupload_move;
			return r;
		}

		template <typename T>
		static auto move(detail::typed_shadow_buffer_t<T>& buf) -> buffer_data_t
		{
			auto r = buffer_data_t{ownership_t::move, &buf, buf.size()};
			r.fn_shadowbuffer_ = &detail::buffer_data_vtable_t<detail::typed_shadow_buffer_t<T>>::shadowbuffer_move;
			r.fn_vram_ = &detail::buffer_data_vtable_t<detail::typed_shadow_buffer_t<T>>::vram_upload;
			r.fn_vram_post_ = &detail::buffer_data_vtable_t<detail::typed_shadow_buffer_t<T>>::vram_postupload_move;
			return r;
		}

		auto apply_to_shadowbuffer(detail::shadow_buffer_t& buf) const -> void
		{
			fn_shadowbuffer_(buf, data, count);
		}

		auto apply_to_vram(void const*& dest) const -> void
		{
			fn_vram_(dest, data, count);
		}

		auto post_vram() const -> void
		{
			fn_vram_post_(data, count);
		}

	private:
		buffer_data_t(ownership_t ownership, void const* data, size_t count)
			: ownership(ownership), data(data), count(count)
			, fn_shadowbuffer_(), fn_vram_(), fn_vram_post_()
		{}

		ownership_t ownership;
		void const* data;
		size_t count;
		void (*fn_shadowbuffer_)(detail::shadow_buffer_t&, void const*, size_t);
		void (*fn_vram_)(void const*&, void const*, size_t);
		void (*fn_vram_post_)(void const*, size_t);

		friend struct buffer_t;
	};

	struct buffer_t : resource_t
	{
		//template <typename T>
		//using typed_shadow_buffer_t = atma::vector<T, atma::aligned_allocator_t<T, 16>>;
		//using shadow_buffer_t = typed_shadow_buffer_t<char>;
		template <typename T>
		using typed_shadow_buffer_t = detail::typed_shadow_buffer_t<T>;
		using shadow_buffer_t = detail::shadow_buffer_t;


		buffer_t(context_ptr const&, resource_type_t, resource_usage_mask_t, buffer_usage_t, buffer_dimensions_t const&, buffer_data_t const&);
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
		resource_type_t rt,
		resource_usage_mask_t ru,
		buffer_usage_t bu,
		buffer_dimensions_t const& bdm,
		buffer_data_t const& bdt,
		Args&&... req_views) -> buffer_ptr
	{
		auto b = atma::make_intrusive_ptr<buffer_t>(ctx, rt, ru, bu, bdm, bdt);
		int _[] = { 0, (b->bind(req_views), 0)... };
		
		return b;
	}

}
