#include <lion/streams.hpp>

#include <algorithm>

using namespace lion;


namespace
{
	auto read_input(atma::input_stream_ptr const& stream) -> atma::unique_memory_t
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
			if (rr.status == atma::stream_status_t::eof)
				break;

			atma::unique_memory_t tmp{sz + sz * 2};
			memcpy(tmp.begin(), mem.begin(), sz);
			offset += sz;
			sz *= 2;
			mem = std::move(tmp);
		}

		return mem;
	}

	auto read_input_random(atma::random_access_input_stream_ptr const& rs) -> atma::unique_memory_t
	{
		if (!rs)
			return atma::unique_memory_t{};

		auto mem = atma::unique_memory_t{rs->g_size()};
		rs->read(mem.begin(), rs->g_size());
		return mem;
	}
}

auto lion::read_all(atma::stream_ptr const& stream) -> atma::unique_memory_t
{
	ATMA_ASSERT(stream);
	ATMA_ASSERT(stream->stream_opers() & atma::stream_opers_t::read, "non-input stream given");
	
	if (stream->stream_opers() & atma::stream_opers_t::random_access)
	{
		return read_input_random(atma::stream_cast<atma::random_access_input_stream_t>(stream));
	}
	else
	{
		return read_input(atma::stream_cast<atma::input_stream_t>(stream));
	}
}

auto lion::read_all(atma::input_stream_ptr const& stream) -> atma::unique_memory_t
{
	ATMA_ASSERT(stream);

	if (stream->stream_opers() & atma::stream_opers_t::random_access)
	{
		return read_input_random(atma::stream_cast<atma::random_access_input_stream_t>(stream));
	}
	else
	{
		return read_input(stream);
	}
}

auto lion::read_all(atma::random_access_input_stream_ptr const& stream) -> atma::unique_memory_t
{
	ATMA_ASSERT(stream);

	return read_input_random(stream);
}





