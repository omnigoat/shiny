#pragma once

#include <atma/streams.hpp>

namespace lion
{
	auto read_all(atma::stream_ptr const&) -> atma::unique_memory_t;
	auto read_all(atma::input_stream_ptr const&) -> atma::unique_memory_t;
	auto read_all(atma::random_access_input_stream_ptr const&) -> atma::unique_memory_t;
}
