#include <shelf/file.hpp>


using namespace shelf;
using shelf::file_t;

file_t::file_t(atma::string const& filename, file_access_t access)
	: filename_(filename), access_(access)
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

auto file_t::size() const -> size_t
{
	return filesize_;
}

auto file_t::seek(size_t x) -> stream_status_t
{
	auto r = fseek(handle_, x, SEEK_SET);
	if (r == 0)
		return stream_status_t::ok;
	else
		return stream_status_t::error;
}

auto file_t::move(int64 x) -> stream_status_t
{
	auto r = fseek(handle_, x, SEEK_CUR);
	if (r == 0)
		return stream_status_t::ok;
	else
		return stream_status_t::error;
}

auto file_t::read(void* buf, size_t size) -> read_result_t
{
	size_t r = fread(buf, 1, size, handle_);
	if (r == size)
		return {stream_status_t::ok, r};
	else if (feof(handle_))
		return {stream_status_t::eof, r};
	else
		return {stream_status_t::error, r};
}

auto write(void const*, size_t) -> write_result_t
{
}
