#pragma once

#include <atma/streams.hpp>

#include <rose/mmap.hpp>

namespace lion
{
	enum class mmap_stream_access_t
	{
		read,
		write_copy,
		write_commit,
	};

	ATMA_BITMASK(mmap_stream_access_mask_t, mmap_stream_access_t);

	struct mmap_stream_t : atma::memory_stream_t
	{
		mmap_stream_t(rose::mmap_ptr const&, mmap_stream_access_mask_t);
		mmap_stream_t(rose::mmap_ptr const&, size_t offset, size_t size, mmap_stream_access_mask_t);
		~mmap_stream_t();

		auto stream_opers() const -> atma::stream_opers_mask_t override;

	private:
		rose::mmap_ptr mmap_;
		atma::stream_opers_mask_t opers_;

		void* data_;
	};

	using mmap_stream_ptr = atma::intrusive_ptr<mmap_stream_t>;



	auto read_all(atma::stream_ptr const&) -> atma::unique_memory_t;
	auto read_all(atma::input_stream_ptr const&) -> atma::unique_memory_t;
	auto read_all(atma::random_access_input_stream_ptr const&) -> atma::unique_memory_t;
}
