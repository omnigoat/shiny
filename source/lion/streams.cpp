#include <lion/streams.hpp>

#include <atma/intrusive_ptr.hpp>

#include <algorithm>

using namespace lion;
using lion::mmap_bytestream_t;



mmap_bytestream_t::mmap_bytestream_t(rose::mmap_ptr const& mmap, mmap_bytestream_access_mask_t access)
	: mmap_bytestream_t{mmap, 0, mmap->size(), access}
{}

mmap_bytestream_t::mmap_bytestream_t(rose::mmap_ptr const& mmap, size_t offset, size_t size, mmap_bytestream_access_mask_t access)
	: memory_bytestream_t{nullptr, 0}
	, mmap_{mmap}
	, opers_{stream_opers_t::random_access, stream_opers_t::read}
{
	ATMA_ASSERT(mmap_);
	ATMA_ASSERT(mmap_->valid());

	if (size == 0)
		size = mmap->size();

	uint32 lo = offset & 0xffffffff;
	uint32 hi = (offset & 0xffffffff00000000) >> 32;

	if (access & mmap_bytestream_access_t::write_commit) {
		data_ = MapViewOfFile(mmap->handle(), FILE_MAP_WRITE, hi, lo, size);
		opers_ |= stream_opers_t::write;
	}
	else if (access & mmap_bytestream_access_t::write_copy) {
		data_ = MapViewOfFile(mmap->handle(), FILE_MAP_COPY, hi, lo, size);
		opers_ |= stream_opers_t::write;
	}
	else {
		data_ = MapViewOfFile(mmap->handle(), FILE_MAP_READ, hi, lo, size);
	}

	if (data_)
		memory_stream_reset(data_, size);
}

mmap_bytestream_t::~mmap_bytestream_t()
{
	if (data_)
		UnmapViewOfFile(data_);
}

auto mmap_bytestream_t::stream_status() const -> stream_status_t
{
	if (mmap_ == nullptr || data_ == nullptr)
		return stream_status_t::error;
	else
		return memory_bytestream_t::stream_status();
}

auto mmap_bytestream_t::stream_opers() const -> stream_opers_mask_t
{
	return opers_;
}




namespace
{
	auto read_input(input_stream_ptr const& stream) -> atma::unique_memory_t
	{
		atma::unique_memory_t mem;
		if (!stream)
			return mem;

		// start with 4kb, double every time
		size_t sz = 4096;

		atma::read_result_t rr;
		mem = atma::unique_memory_t{sz};
		for (size_t offset = 0;;)
		{
			rr = stream->read(mem.begin(), sz);
			if (rr.status == stream_status_t::exhausted)
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
		return read_input_random(atma::stream_cast<random_access_input_stream_t>(stream));
	}
	else
	{
		return read_input(atma::stream_cast<input_stream_t>(stream));
	}
}

auto lion::read_all(input_stream_ptr const& stream) -> atma::unique_memory_t
{
	ATMA_ASSERT(stream);

	if (stream->stream_opers() & stream_opers_t::random_access)
	{
		return read_input_random(atma::stream_cast<random_access_input_stream_t>(stream));
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

