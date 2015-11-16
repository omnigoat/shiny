#include <lion/streams.hpp>

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

