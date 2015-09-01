#pragma once

#include <atma/string.hpp>


namespace lion
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





	enum class file_access_t : uint
	{
		read,
		write,
		read_write,
	};




	struct file_t
		: abstract_random_access_input_stream_t
		, abstract_random_access_output_stream_t
	{
		file_t();
		file_t(atma::string const&, file_access_t = file_access_t::read);
		file_t(file_t&&);
		file_t(file_t const&) = delete;
		~file_t();

		auto valid() const -> bool;
		auto size() const -> size_t;
		auto position() const -> size_t;

		auto seek(size_t) -> stream_status_t;
		auto move(int64) -> stream_status_t;

		// input-stream
		auto read(void*, size_t) -> read_result_t override;

		// output-stream
		auto write(void const*, size_t) -> write_result_t override;

	private:
		// random-access-input-stream
		auto g_size() const -> size_t override;
		auto g_seek(size_t) -> stream_status_t override;
		auto g_move(int64) -> stream_status_t override;

		// random-access-output-stream
		auto p_size() const -> size_t override;
		auto p_seek(size_t) -> stream_status_t override;
		auto p_move(int64) -> stream_status_t override;

	private:
		atma::string filename_;
		file_access_t access_;

		FILE* handle_;
		size_t filesize_;
	};


	template <size_t Bufsize, typename FN>
	inline auto for_each_line(abstract_input_stream_t& stream, size_t maxsize, FN&& fn) -> void
	{
		char buf[Bufsize];
		atma::string line;

		// read bytes into line until newline is found
		auto rr = read_result_t{stream_status_t::ok, 0};
		while (rr.status == stream_status_t::ok)
		{
			rr = stream.read(buf, Bufsize);
			auto bufend = buf + rr.bytes_read;
			auto bufp = buf;

			while (bufp != bufend)
			{
				auto newline = std::find(bufp, bufend, '\n');
				line.append(bufp, newline);

				if (newline != bufend) {
					fn(line.raw_begin(), line.raw_size());
					line.clear();
				}

				bufp = (newline == bufend) ? bufend : newline + 1;
			}
		}
	}
}
