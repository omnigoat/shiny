#pragma once

#include "streams.hpp"

#include <atma/unique_memory.hpp>
#include <atma/assert.hpp>
#include <atma/intrusive_ptr.hpp>

#include <filesystem>


namespace stdfs = std::experimental::filesystem;

namespace lion
{
	enum class access_flags_t
	{
		read,
		write,
	};

	ATMA_BITMASK(access_mask_t, access_flags_t);

	struct mmap_t : atma::ref_counted
	{
		mmap_t(stdfs::path const&, access_mask_t = access_flags_t::read);
		~mmap_t();

		auto valid() const -> bool;
		auto size() const -> size_t;

		template <typename T>
		auto data_view() const -> atma::memory_view_t<T const>
		{
			ATMA_ASSERT(valid());
			return {reinterpret_cast<T const*>(data_), reinterpret_cast<T const*>(data_ + size_)};
		}

	private:
		stdfs::path path_;
		access_mask_t access_mask_;

		HANDLE handle_ = 0;
		size_t size_ = 0;

		friend struct mmap_stream_t;
	};

	using mmap_ptr = atma::intrusive_ptr<mmap_t>;




	enum class mmap_stream_access_t
	{
		read,
		write_copy,
		write_commit,
	};

	ATMA_BITMASK(mmap_stream_access_mask_t, mmap_stream_access_t);

	struct mmap_stream_t : atma::memory_stream_t
	{
		mmap_stream_t(mmap_ptr const&, mmap_stream_access_mask_t);
		mmap_stream_t(mmap_ptr const&, size_t offset, size_t size, mmap_stream_access_mask_t);
		~mmap_stream_t();

		auto stream_opers() const -> atma::stream_opers_mask_t override;

	private:
		mmap_ptr mmap_;
		atma::stream_opers_mask_t opers_;

		void* data_;
	};

	using mmap_stream_ptr = atma::intrusive_ptr<mmap_stream_t>;
}
