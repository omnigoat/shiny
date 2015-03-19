#pragma once

#include <shiny/shiny_fwd.hpp>
#include <shiny/data_stream.hpp>


namespace shiny
{
	struct data_declaration_t
	{
		data_declaration_t() = delete;
		data_declaration_t(data_declaration_t const*) = delete;
		data_declaration_t(data_declaration_t&&) = delete;

		auto streams() const -> data_streams_t const&;
		auto stride() const -> uint;

	private:
		data_declaration_t(data_streams_t const&);

	private:
		data_streams_t streams_;
		uint stride_;

		friend struct runtime_t;
	};
}
