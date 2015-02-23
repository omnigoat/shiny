#pragma once

#include <atma/string.hpp>
//#include <

namespace shelf
{

	enum class stream_status_t
	{
		ok,
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

	struct abstract_input_stream_t
	{
		virtual auto read(void*, size_t) -> read_result_t = 0;
	};

	struct abstract_random_access_input_stream_t
		: virtual abstract_input_stream_t
	{
		virtual auto g_size() const -> size_t = 0;
		virtual auto g_seek(size_t) -> stream_status_t = 0;
		virtual auto g_move(int64) -> stream_status_t = 0;
	};

	struct abstract_output_stream_t
	{
		virtual auto write(void const*, size_t) -> write_result_t = 0;
	};

	struct abstract_random_access_output_stream_t
		: virtual abstract_output_stream_t
	{
		virtual auto p_size() const -> size_t = 0;
		virtual auto p_seek(size_t) -> stream_status_t = 0;
		virtual auto p_move(int64) -> stream_status_t = 0;
	};

	struct abstract_input_output_stream_t
		: virtual abstract_input_stream_t
		, virtual abstract_output_stream_t
	{
	};

	struct abstract_random_access_input_output_stream_t
		: virtual abstract_input_output_stream_t
		, virtual abstract_random_access_input_stream_t
		, virtual abstract_random_access_output_stream_t
	{
	};



	enum class file_access_t : uint
	{
		read,
		write,
		read_write,
	};



	struct file_t : abstract_random_access_input_output_stream_t
	{
		file_t() {}
		file_t(atma::string const&, file_access_t = file_access_t::read);

		auto size() const -> size_t;

		auto seek(size_t) -> stream_status_t;
		auto move(int64) -> stream_status_t;
		//auto read(void*, size_t) -> read_result_t;
		//auto write(void const*, size_t) -> write_result_t;
		auto read(void*, size_t) -> read_result_t override;
		auto write(void const*, size_t) -> write_result_t override;

	private:
		// input-stream
		auto g_size() const -> size_t override { return 0; }
		auto g_seek(size_t) -> stream_status_t override { return stream_status_t::ok; }
		auto g_move(int64) -> stream_status_t override { return stream_status_t::ok; }

		// output-stream
		auto p_size() const -> size_t override { return 0; }
		auto p_seek(size_t) -> stream_status_t override { return stream_status_t::ok; }
		auto p_move(int64) -> stream_status_t override { return stream_status_t::ok; }

	private:
		atma::string filename_;
		file_access_t access_;

		FILE* handle_;
		size_t filesize_;
	};
}