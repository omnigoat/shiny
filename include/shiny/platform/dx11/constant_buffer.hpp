#pragma once

#include <shiny/constant_buffer.hpp>

#include <shiny/platform/dx11/buffer.hpp>


namespace shiny_dx11
{
	using constant_buffer_bridge_t = shiny::constant_buffer_bridge_t<buffer_t>;
	using constant_buffer_bridge_ptr = atma::intrusive_ptr<constant_buffer_bridge_t>;
}

