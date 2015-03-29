#include <shiny/element_format.hpp>

#include <atma/assert.hpp>


auto shiny::element_count(shiny::element_format_t f) -> int
{
	using ef = shiny::element_format_t;

	switch (f)
	{
		default:
			break;

		// 4-component
		case ef::g8x4:
		case ef::s8x4:
		case ef::u8x4:
		case ef::sn8x4:
		case ef::un8x4:
		case ef::f16x4:
		case ef::f32x4:
			return 4;

		// 2-component
		case ef::u32x2:
			return 2;

		// 1-component
		case ef::g32:
			return 1;
	}

	ATMA_HALT("component-count of non-recognised element_format_t");
	return -1;
}

auto shiny::element_size(element_format_t fmt) -> size_t
{
	static size_t const mapping[] =
	{
		0,

		4,
		4,
		4,
		4,
		4,
		8,
		16,

		8,

		4
	};

	ATMA_ASSERT((int)fmt < std::extent<decltype(mapping)>::value);
	return mapping[(int)fmt];
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
