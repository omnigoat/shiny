#include <lion/mmap.hpp>

using namespace lion;
using lion::mmap_t;
using lion::mmap_stream_t;

mmap_t::mmap_t(stdfs::path const& path)
	: path_(path)
	, data_()
	, size_()
{
	if (stdfs::exists(path_))
	{
#if defined(WIN32)
		auto file_handle = CreateFile(path_.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, nullptr);
		handle_ = CreateFileMapping(file_handle, nullptr, PAGE_READONLY, 0, 0, 0);
		data_ = MapViewOfFile(handle_, FILE_MAP_READ, 0, 0, 0);

		LARGE_INTEGER i;
		GetFileSizeEx(file_handle, &i);
		size_ = i.QuadPart;

		CloseHandle(file_handle);
#endif
	}
}

mmap_t::~mmap_t()
{
	UnmapViewOfFile(data_);
	CloseHandle(handle_);
}

auto mmap_t::valid() const -> bool
{
	return size_ != 0;
}

auto mmap_t::size() const -> size_t
{
	return size_;
}

auto mmap_t::data() const -> void const*
{
	ATMA_ASSERT(valid());

	return data_;
}



mmap_stream_t::mmap_stream_t(mmap_ptr const& mmap)
	: memory_stream_t{mmap->data(), mmap->size()}
	, mmap_(mmap)
{
}

auto mmap_stream_t::stream_opers() const -> stream_opers_mask_t
{
	return stream_opers_mask_t{stream_opers_t::random_access, stream_opers_t::read};
}
