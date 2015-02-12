#pragma once

#include <dust/dust_fwd.hpp>

#include <vector>


namespace dust
{
	struct geometry_stream_t
	{
		char const* tag;
		char start, count;
	};

	struct geometry_declaration_t
	{
		geometry_declaration_t() = delete;
		geometry_declaration_t(geometry_declaration_t const*) = delete;
		geometry_declaration_t(geometry_declaration_t&&) = delete;

		auto streams() const -> geometry_streams_t const&;

	private:
		geometry_declaration_t(geometry_streams_t const&);

	private:
		geometry_streams_t streams_;

		friend struct runtime_t;
	};
}