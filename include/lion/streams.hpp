#pragma once

#include <atma/types.hpp>
#include <atma/intrusive_ptr.hpp>

namespace lion
{
	enum class stream_status_t
	{
		good,
		bof,
		eof,
		error,
	};

	struct read_result_t
	{
		stream_status_t status;
		size_t bytes_read;
	};

	struct write_result_t
	{
		stream_status_t status;
		size_t bytes_written;
	};

	struct abstract_stream_t
		: atma::ref_counted
	{
	};

	struct abstract_input_stream_t
		: virtual abstract_stream_t
	{
		virtual auto read(void*, size_t) -> read_result_t = 0;
	};

	struct abstract_random_access_input_stream_t
		: abstract_input_stream_t
	{
		virtual auto g_size() const -> size_t = 0;
		virtual auto g_seek(size_t) -> stream_status_t = 0;
		virtual auto g_move(int64) -> stream_status_t = 0;
	};

	struct abstract_output_stream_t
		: virtual abstract_stream_t
	{
		virtual auto write(void const*, size_t) -> write_result_t = 0;
	};

	struct abstract_random_access_output_stream_t
		: abstract_output_stream_t
	{
		virtual auto p_size() const -> size_t = 0;
		virtual auto p_seek(size_t) -> stream_status_t = 0;
		virtual auto p_move(int64) -> stream_status_t = 0;
	};

	using abstract_stream_ptr        = atma::intrusive_ptr<abstract_stream_t>;
	using abstract_input_stream_ptr  = atma::intrusive_ptr<abstract_input_stream_t>;
	using abstract_output_stream_ptr = atma::intrusive_ptr<abstract_output_stream_t>;
}
