#pragma once

#include <dust/dust_fwd.hpp>

#include <vector>


namespace dust
{
	struct geometry_stream_t
	{
		char const* tag;
		char index, start, count;
	};

	inline auto operator < (geometry_stream_t const& lhs, geometry_stream_t const& rhs) -> bool
	{
		auto x = strcmp(lhs.tag, rhs.tag);
		if (x != 0)
			return x < 0;
		else
			return lhs.index < rhs.index;
	}

	struct geometry_declaration_t
	{
		geometry_declaration_t() = delete;
		geometry_declaration_t(geometry_declaration_t const*) = delete;
		geometry_declaration_t(geometry_declaration_t&&) = delete;

		auto streams() const -> geometry_streams_t const& { return streams_; }

	private:
		geometry_declaration_t(geometry_streams_t const& streams)
			: streams_(streams)
		{}

	private:
		geometry_streams_t streams_;

		friend struct runtime_t;
	};
}