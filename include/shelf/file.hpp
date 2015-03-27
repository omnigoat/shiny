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
		auto read(void*, size_t) -> read_result_t override;
		auto write(void const*, size_t) -> write_result_t override;

	private:
		// input-stream
		auto g_size() const -> size_t override;
		auto g_seek(size_t) -> stream_status_t override;
		auto g_move(int64) -> stream_status_t override;

		// output-stream
		auto p_size() const -> size_t override;
		auto p_seek(size_t) -> stream_status_t override;
		auto p_move(int64) -> stream_status_t override;

	private:
		atma::string filename_;
		file_access_t access_;

		FILE* handle_;
		size_t filesize_;
	};





	inline file_t::file_t()
		: filename_()
		, access_()
		, filesize_()
	{}

	inline file_t::file_t(atma::string const& filename, file_access_t access)
		: filename_(filename)
		, access_(access)
		, filesize_()
	{
		char const* fa[] = {"r", "w", "r+"};
		handle_ = fopen(filename.c_str(), fa[(uint)access]);
		if (handle_ == nullptr)
			return;

		// get filesize
		fseek(handle_, 0, SEEK_END);
		filesize_ = ftell(handle_);
		fseek(handle_, 0, SEEK_SET);
	}

	inline file_t::file_t(file_t&& rhs)
		: filename_(rhs.filename_)
		, access_(rhs.access_)
		, filesize_(rhs.filesize_)
	{}

	inline file_t::~file_t()
	{
		if (handle_)
			fclose(handle_);
	}

	inline auto file_t::valid() const -> bool
	{
		return handle_ != nullptr;
	}

	inline auto file_t::size() const -> size_t
	{
		return filesize_;
	}

	inline auto file_t::position() const -> size_t
	{
		return ftell(handle_);
	}

	inline auto file_t::seek(size_t x) -> stream_status_t
	{
		auto r = fseek(handle_, (long)x, SEEK_SET);
		if (r == 0)
			return stream_status_t::ok;
		else
			return stream_status_t::error;
	}

	inline auto file_t::move(int64 x) -> stream_status_t
	{
		auto r = fseek(handle_, (long)x, SEEK_CUR);
		if (r == 0)
			return stream_status_t::ok;
		else
			return stream_status_t::error;
	}

	inline auto file_t::read(void* buf, size_t size) -> read_result_t
	{
		size_t r = fread(buf, 1, size, handle_);

		if (r == size)
			return {stream_status_t::ok, r};
		else if (feof(handle_))
			return {stream_status_t::eof, r};
		else
			return {stream_status_t::error, r};
	}

	inline auto file_t::write(void const* data, size_t size) -> write_result_t
	{
		size_t r = fwrite(data, 1, size, handle_);

		if (r == size)
			return {stream_status_t::ok, r};
		else if (feof(handle_))
			return {stream_status_t::eof, r};
		else
			return {stream_status_t::error, r};
	}

	// input-stream
	inline auto file_t::g_size() const -> size_t
	{
		return filesize_;
	}

	inline auto file_t::g_seek(size_t x) -> stream_status_t
	{
		return seek(x);
	}

	inline auto file_t::g_move(int64 x) -> stream_status_t
	{
		return move(x);
	}

	// output-stream
	inline auto file_t::p_size() const -> size_t
	{
		return filesize_;
	}

	inline auto file_t::p_seek(size_t x) -> stream_status_t
	{
		return seek(x);
	}

	inline auto file_t::p_move(int64 x) -> stream_status_t
	{
		return move(x);
	}





	template <size_t Bufsize, typename FN>
	inline auto for_each_line(abstract_input_stream_t& stream, size_t maxsize, FN&& fn) -> void
	{
		char buf[Bufsize];
		atma::string line;

		// read bytes into line until newline is found
		auto rr = shelf::read_result_t{shelf::stream_status_t::ok, 0};
		while (rr.status == shelf::stream_status_t::ok)
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
