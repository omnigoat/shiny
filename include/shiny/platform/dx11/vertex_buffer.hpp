#pragma once

#include <shiny/vertex_buffer.hpp>

#include <shiny/platform/dx11/buffer.hpp>


namespace shiny_dx11
{
	using vertex_buffer_bridge_t = shiny::vertex_buffer_bridge_t<buffer_t>;
	using vertex_buffer_bridge_ptr = atma::intrusive_ptr<vertex_buffer_bridge_t>;
}



