#pragma once
//======================================================================
#include <atma/intrusive_ptr.hpp>
//======================================================================
namespace dust {
	
	struct runtime_t;

	struct adapter_t;
	typedef atma::intrusive_ptr<adapter_t> adapter_ptr;

	struct output_t;
	typedef atma::intrusive_ptr<output_t> output_ptr;

	struct context_t;
	typedef atma::intrusive_ptr<context_t> context_ptr;

	struct vertex_shader_t;
	typedef atma::intrusive_ptr<vertex_shader_t> vertex_shader_ptr;

	struct pixel_shader_t;
	typedef atma::intrusive_ptr<pixel_shader_t> pixel_shader_ptr;

	struct vertex_declaration_t;

	struct vertex_buffer_t;
	typedef atma::intrusive_ptr<vertex_buffer_t> vertex_buffer_ptr;

	struct constant_buffer_t;
	typedef atma::intrusive_ptr<constant_buffer_t> constant_buffer_ptr;
}


