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

	struct index_buffer_t;
	typedef atma::intrusive_ptr<index_buffer_t> index_buffer_ptr;

	struct camera_t;
	struct scene_t;

	struct texture2d_t;
	typedef atma::intrusive_ptr<texture2d_t> texture2d_ptr;

	struct texture3d_t;
	typedef atma::intrusive_ptr<texture3d_t> texture3d_ptr;

	struct compute_shader_t;
	typedef atma::intrusive_ptr<compute_shader_t> compute_shader_ptr;

	struct buffer_t;
	typedef atma::intrusive_ptr<buffer_t> buffer_ptr;


	enum class buffer_usage_t
	{
		immutable,
		long_lived,
		dynamic
	};

	enum class gpu_access_t
	{
		read,
		write,
		read_write
	};

	enum class cpu_access_t
	{
		none,
		read,
		write,
		read_write,
	};

	enum class buffer_type_t
	{
		vertex_buffer,
		index_buffer,
		constant_buffer,
		shader_resource,
	};

	enum class texture_usage_t
	{
		normal,
		render_target,
		depth_stencil
	};
}


