#pragma once

#include <shiny/index_buffer.hpp>

#include <shiny_dx11/buffer.hpp>


namespace shiny_dx11
{
	using index_buffer_bridge_t = shiny::index_buffer_bridge_t<buffer_t>;
	using index_buffer_bridge_ptr = atma::intrusive_ptr<index_buffer_bridge_t>;
}
