#pragma once

#include "streams.hpp"

#include <atma/unique_memory.hpp>
#include <atma/assert.hpp>
#include <atma/intrusive_ptr.hpp>

#include <filesystem>


namespace stdfs = std::experimental::filesystem;

namespace lion
{
	struct mmap_t : atma::ref_counted
	{
		mmap_t(stdfs::path const&);
		~mmap_t();

		auto valid() const -> bool;
		auto size() const -> size_t;
		auto data() const -> void const*;

		template <typename T>
		auto data_view() const -> atma::memory_view_t<T const>
		{
			ATMA_ASSERT(valid());
			return {reinterpret_cast<T const*>(data_), reinterpret_cast<T const*>(data_ + size_)};
		}

	private:
		stdfs::path path_;
		void* data_;
		size_t size_;

#if defined(WIN32)
		HANDLE handle_;
#endif
	};

	using mmap_ptr = atma::intrusive_ptr<mmap_t>;




	struct mmap_stream_t
		: abstract_random_access_input_stream_t
	{
		mmap_stream_t(mmap_ptr const&);

		auto stream_opers() const -> stream_opers_mask_t override;
		auto read(void*, size_t) -> read_result_t override;

	private:
		auto g_size() const -> size_t override;
		auto g_seek(size_t) -> stream_status_t override;
		auto g_move(int64) -> stream_status_t override;

	private:
		mmap_ptr mmap_;
		size_t position_;
	};

	using mmap_stream_ptr = atma::intrusive_ptr<mmap_stream_t>;
}
