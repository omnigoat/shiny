#pragma once

#include <atma/types.hpp>
#include <atma/intrusive_ptr.hpp>
#include <atma/bitmask.hpp>
#include <atma/unique_memory.hpp>

namespace lion
{
	enum class stream_status_t
	{
		good,
		eof,
		error,
	};

	enum class stream_opers_t
	{
		read,
		write,
		random_access,
	};

	using stream_opers_mask_t = atma::bitmask_t<stream_opers_t>;

	struct read_result_t
	{
		read_result_t() : status(), bytes_read() {}
		read_result_t(stream_status_t s, size_t b) : status(s), bytes_read(b) {}

		stream_status_t status;
		size_t bytes_read;
	};

	struct write_result_t
	{
		stream_status_t status;
		size_t bytes_written;
	};

	struct stream_t
		: atma::ref_counted
	{
		virtual auto stream_opers() const -> stream_opers_mask_t = 0;
	};

	struct input_stream_t
		: virtual stream_t
	{
		virtual auto read(void*, size_t) -> read_result_t = 0;
	};

	struct random_access_input_stream_t
		: input_stream_t
	{
		virtual auto g_size() const -> size_t = 0;
		virtual auto g_seek(size_t) -> stream_status_t = 0;
		virtual auto g_move(int64) -> stream_status_t = 0;
	};

	struct output_stream_t
		: virtual stream_t
	{
		virtual auto write(void const*, size_t) -> write_result_t = 0;
	};

	struct random_access_output_stream_t
		: output_stream_t
	{
		virtual auto p_size() const -> size_t = 0;
		virtual auto p_seek(size_t) -> stream_status_t = 0;
		virtual auto p_move(int64) -> stream_status_t = 0;
	};

	using stream_ptr                      = atma::intrusive_ptr<stream_t>;
	using input_stream_ptr                = atma::intrusive_ptr<input_stream_t>;
	using output_stream_ptr               = atma::intrusive_ptr<output_stream_t>;
	using random_access_input_stream_ptr  = atma::intrusive_ptr<random_access_input_stream_t>;
	using random_access_output_stream_ptr = atma::intrusive_ptr<random_access_output_stream_t>;

	template <typename T, typename Y>
	inline auto stream_cast(atma::intrusive_ptr<Y> const& stream) -> atma::intrusive_ptr<T>
	{
		return stream.cast_dynamic<T>();
	}

	auto read_all(stream_ptr const&) -> atma::unique_memory_t;
	auto read_all(input_stream_ptr const&) -> atma::unique_memory_t;
	auto read_all(random_access_input_stream_ptr const&) -> atma::unique_memory_t;
}
