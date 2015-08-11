#include <shiny/format.hpp>

#include <atma/assert.hpp>


auto shiny::element_count(shiny::format_t f) -> uint
{
	return ((uint32)f & 0xf0) >> 4;
}

auto shiny::element_size(format_t f) -> size_t
{
	return element_count(f) * (((uint32)f & 0xff00) >> 8) / 8;
}

auto shiny::is_generic(format_t f) -> bool
{
	return ((uint32)f & 0x1) != 0;
}

auto shiny::format_depth_size(format_t f) -> size_t
{
	if ((uint32)f & 0x100)
		return element_size(f);
	else
		return 0;
}

auto shiny::format_stencil_size(format_t f) -> size_t
{
	return ((uint32)f & 0x200);
}

