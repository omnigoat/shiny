#pragma once
//======================================================================
#include <dust/platform/win32/d3d_fwd.hpp>

#include <atma/intrusive_ptr.hpp>
//======================================================================
namespace dust {
	
	struct context_t;
	typedef atma::intrusive_ptr<context_t> context_ptr;


	struct vertex_shader_t : atma::ref_counted
	{
		vertex_shader_t(context_ptr const&);

		auto d3d_vs() const -> platform::d3d_vertex_shader_ptr const& { return d3d_vs_; }

	private:
		context_ptr context_;
		platform::d3d_vertex_shader_ptr d3d_vs_;
	};

	typedef atma::intrusive_ptr<vertex_shader_t> vertex_shader_ptr;
}


