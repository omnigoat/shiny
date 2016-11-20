#pragma once

#include <atma/streams.hpp>

#include <rose/mmap.hpp>


// bring atma bytestsreams into lion dropping the byte
namespace lion
{
	using stream_status_t = atma::stream_status_t;
	using stream_opers_t = atma::stream_opers_t;
	using stream_opers_mask_t = atma::stream_opers_mask_t;

	using stream_t = atma::stream_t;
	using input_stream_t = atma::input_bytestream_t;
	using output_stream_t = atma::output_bytestream_t;
	using random_access_input_stream_t = atma::random_access_input_bytestream_t;
	using random_access_output_stream_t = atma::random_access_output_bytestream_t;

	using stream_ptr = atma::stream_ptr;
	using input_stream_ptr = atma::input_bytestream_ptr;
	using output_stream_ptr = atma::output_bytestream_ptr;
	using random_access_input_stream_ptr = atma::random_access_input_bytestream_ptr;
	using random_access_output_stream_ptr = atma::random_access_output_bytestream_ptr;
}


// mmap-bytestream
namespace lion
{
	enum class mmap_bytestream_access_t
	{
		read,
		write_copy,
		write_commit,
	};

	ATMA_BITMASK(mmap_bytestream_access_mask_t, mmap_bytestream_access_t);

	struct mmap_bytestream_t
		: atma::memory_bytestream_t
	{
		mmap_bytestream_t(rose::mmap_ptr const&, mmap_bytestream_access_mask_t);
		mmap_bytestream_t(rose::mmap_ptr const&, size_t offset, size_t size, mmap_bytestream_access_mask_t);
		~mmap_bytestream_t();

		auto stream_status() const -> stream_status_t override;
		auto stream_opers() const -> stream_opers_mask_t override;

	private:
		rose::mmap_ptr mmap_;
		stream_opers_mask_t opers_;

		void* data_;
	};

	using mmap_bytestream_ptr = atma::intrusive_ptr<mmap_bytestream_t>;
}


// functions
namespace lion
{
	auto read_all(stream_ptr const&) -> atma::unique_memory_t;
	auto read_all(input_stream_ptr const&) -> atma::unique_memory_t;
	auto read_all(random_access_input_stream_ptr const&) -> atma::unique_memory_t;
}
