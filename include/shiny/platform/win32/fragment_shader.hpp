#pragma once

#include <shiny/shiny_fwd.hpp>
#include <shiny/platform/win32/d3d_fwd.hpp>

#include <lion/assets.hpp>

#include <atma/string.hpp>


namespace shiny
{
	struct fragment_shader_t : lion::asset_t
	{
		static auto make(shiny::renderer_ptr const&, lion::path_t const& path, bool precompiled, atma::string const& entrypoint = "main") -> fragment_shader_ptr;
		static auto make(shiny::renderer_ptr const&, lion::path_t const& path, void const* data, size_t data_size, bool precompiled, atma::string const& entrypoint = "main") -> fragment_shader_ptr;

		auto d3d_blob() const -> platform::d3d_blob_ptr const& { return d3d_blob_; }
		auto d3d_fs() const -> platform::d3d_fragment_shader_ptr const& { return d3d_fs_; }

	protected:
		fragment_shader_t(renderer_ptr const&, lion::path_t const&, void const*, size_t, bool, atma::string const&);
		fragment_shader_t(renderer_ptr const&, lion::path_t const&, platform::d3d_blob_ptr const&, platform::d3d_fragment_shader_ptr const&);

	private:
		renderer_ptr rndr_;

		platform::d3d_blob_ptr d3d_blob_;
		platform::d3d_fragment_shader_ptr d3d_fs_;

		friend struct atma::intrusive_ptr_make<fragment_shader_t>;
	};
}

namespace atma
{
	template <>
	struct intrusive_ptr_make<shiny::fragment_shader_t>
	{
		static auto make(shiny::renderer_ptr const&, lion::path_t const&, void const*, size_t, bool, atma::string const&) -> shiny::fragment_shader_t*;
	};
}
