#include <lion/streams.hpp>

#include <algorithm>

using namespace lion;


namespace
{
	auto read_input(input_stream_ptr const& stream) -> atma::unique_memory_t
	{
		atma::unique_memory_t mem;
		if (!stream)
			return mem;

		// start with 4kb, double every time
		size_t sz = 4096;

		read_result_t rr;
		mem = atma::unique_memory_t{sz};
		for (size_t offset = 0;;)
		{
			rr = stream->read(mem.begin(), sz);
			if (rr.status == stream_status_t::eof)
				break;

			atma::unique_memory_t tmp{sz + sz * 2};
			memcpy(tmp.begin(), mem.begin(), sz);
			offset += sz;
			sz *= 2;
			mem = std::move(tmp);
		}

		return mem;
	}

	auto read_input_random(random_access_input_stream_ptr const& rs) -> atma::unique_memory_t
	{
		if (!rs)
			return atma::unique_memory_t{};

		auto mem = atma::unique_memory_t{rs->g_size()};
		rs->read(mem.begin(), rs->g_size());
		return mem;
	}
}

auto lion::read_all(stream_ptr const& stream) -> atma::unique_memory_t
{
	ATMA_ASSERT(stream);
	ATMA_ASSERT(stream->stream_opers() & stream_opers_t::read, "non-input stream given");
	
	if (stream->stream_opers() & stream_opers_t::random_access)
	{
		return read_input_random(stream_cast<random_access_input_stream_t>(stream));
	}
	else
	{
		return read_input(stream_cast<input_stream_t>(stream));
	}
}

auto lion::read_all(input_stream_ptr const& stream) -> atma::unique_memory_t
{
	ATMA_ASSERT(stream);

	if (stream->stream_opers() & stream_opers_t::random_access)
	{
		return read_input_random(stream_cast<random_access_input_stream_t>(stream));
	}
	else
	{
		return read_input(stream);
	}
}

auto lion::read_all(random_access_input_stream_ptr const& stream) -> atma::unique_memory_t
{
	ATMA_ASSERT(stream);

	return read_input_random(stream);
}





memory_stream_t::memory_stream_t(void* data, size_t size)
	: data_(reinterpret_cast<byte*>(data))
	, size_(size)
	, position_()
{
}

auto memory_stream_t::valid() const -> bool
{
	return data_ != nullptr;
}

auto memory_stream_t::size() const -> size_t
{
	return size_;
}

auto memory_stream_t::position() const -> size_t
{
	return position_;
}

auto memory_stream_t::seek(size_t x) -> stream_status_t
{
	if (x < size_) {
		position_ = x;
		return stream_status_t::good;
	}
	else {
		return stream_status_t::error;
	}
}

auto memory_stream_t::move(int64 x) -> stream_status_t
{
	if (position_ + x < size_) {
		position_ += x;
		return stream_status_t::good;
	}
	else {
		return stream_status_t::error;
	}
}

auto memory_stream_t::read(void* buf, size_t size) -> read_result_t
{
	size_t r = std::min(size, size_ - position_);
	memcpy(buf, data_ + position_, r);

	if (r == size)
		return {stream_status_t::good, r};
	else
		return {stream_status_t::eof, r};
}

auto memory_stream_t::write(void const* data, size_t size) -> write_result_t
{
	size_t r = std::min(size, size_ - position_);
	memcpy(data_ + position_, data, r);

	if (r == size)
		return{stream_status_t::good, r};
	else
		return{stream_status_t::eof, r};
}

// absract-stream
auto memory_stream_t::stream_opers() const -> stream_opers_mask_t
{
	return stream_opers_t::read | stream_opers_t::write | stream_opers_t::random_access;
}

// input-stream
auto memory_stream_t::g_size() const -> size_t
{
	return size();
}

auto memory_stream_t::g_seek(size_t x) -> stream_status_t
{
	return seek(x);
}

auto memory_stream_t::g_move(int64 x) -> stream_status_t
{
	return move(x);
}

// output-stream
auto memory_stream_t::p_size() const -> size_t
{
	return size();
}

auto memory_stream_t::p_seek(size_t x) -> stream_status_t
{
	return seek(x);
}

auto memory_stream_t::p_move(int64 x) -> stream_status_t
{
	return move(x);
}

auto memory_stream_t::memory_stream_reset(void* data, size_t size) -> void
{
	data_ = reinterpret_cast<byte*>(data);
	size_ = size;
	position_ = 0;
}
