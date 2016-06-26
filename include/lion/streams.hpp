#pragma once

#include <atma/streams.hpp>

#include <rose/mmap.hpp>

namespace lion
{
	enum class mmap_bytestream_access_t
	{
		read,
		write_copy,
		write_commit,
	};

	ATMA_BITMASK(mmap_bytestream_access_mask_t, mmap_bytestream_access_t);

	struct mmap_bytestream_t : atma::memory_bytestream_t
	{
		mmap_bytestream_t(rose::mmap_ptr const&, mmap_bytestream_access_mask_t);
		mmap_bytestream_t(rose::mmap_ptr const&, size_t offset, size_t size, mmap_bytestream_access_mask_t);
		~mmap_bytestream_t();

		auto stream_opers() const -> atma::stream_opers_mask_t override;

	private:
		rose::mmap_ptr mmap_;
		atma::stream_opers_mask_t opers_;

		void* data_;
	};

	using mmap_bytestream_ptr = atma::intrusive_ptr<mmap_bytestream_t>;



	auto read_all(atma::stream_ptr const&) -> atma::unique_memory_t;
	auto read_all(atma::input_bytestream_ptr const&) -> atma::unique_memory_t;
	auto read_all(atma::random_access_input_bytestream_ptr const&) -> atma::unique_memory_t;
}
