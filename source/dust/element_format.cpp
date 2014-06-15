#include <dust/element_format.hpp>

#include <atma/assert.hpp>


auto dust::element_count(dust::element_format_t f) -> int
{
	using ef = dust::element_format_t;

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

auto dust::element_size(element_format_t fmt) -> int
{
	static int const mapping[] =
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
