#include <shiny/element_format.hpp>

#include <atma/assert.hpp>


auto shiny::element_count(shiny::element_format_t f) -> int
{
	return ((uint16)f & 0xf0) >> 4;
}

auto shiny::element_size(element_format_t f) -> size_t
{
	return element_count(f) * (((uint16)f & 0xff00) >> 8) / 8;
}

auto shiny::index_size(index_format_t fmt) -> size_t
{
	switch (fmt)
	{
		case index_format_t::index16:
			return 2;

		case index_format_t::index32:
			return 4;

		default:
			ATMA_HALT("bad index-format");
			return 0;
	}
}
