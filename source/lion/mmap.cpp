#include <lion/mmap.hpp>

using namespace lion;
using lion::mmap_t;
using lion::mmap_stream_t;

mmap_t::mmap_t(stdfs::path const& path, access_mask_t am)
	: path_(path), access_mask_(am)
{
	if (!stdfs::exists(path_))
		return;

	DWORD file_access = (am & access_flags_t::read ? GENERIC_READ : 0) | (am & access_flags_t::write ? GENERIC_WRITE : 0);
	auto file_handle = CreateFile(path_.c_str(), file_access, 0, nullptr, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, nullptr);

	DWORD map_access = am & access_flags_t::write ? PAGE_READWRITE : PAGE_READONLY;
	handle_ = CreateFileMapping(file_handle, nullptr, map_access, 0, 0, 0);

	//data_ = MapViewOfFile(handle_, FILE_MAP_READ, 0, 0, 0);

	LARGE_INTEGER i;
	GetFileSizeEx(file_handle, &i);
	size_ = i.QuadPart;

	CloseHandle(file_handle);
}

mmap_t::~mmap_t()
{
	//UnmapViewOfFile(data_);
	CloseHandle(handle_);
}

auto mmap_t::valid() const -> bool
{
	return handle_ != nullptr;
}

auto mmap_t::size() const -> size_t
{
	return size_;
}




mmap_stream_t::mmap_stream_t(mmap_ptr const& mmap, mmap_stream_access_mask_t access)
	: mmap_stream_t{mmap, 0, 0, access}
{}

mmap_stream_t::mmap_stream_t(mmap_ptr const& mmap, size_t offset, size_t size, mmap_stream_access_mask_t access)
	: memory_stream_t{nullptr, 0}
	, mmap_{mmap}
	, opers_{stream_opers_t::random_access, stream_opers_t::read}
{
	ATMA_ASSERT(mmap_);
	ATMA_ASSERT(mmap_->valid());

	if (size == 0)
		size = mmap->size();

	uint32 lo = offset & 0xffffffff;
	uint32 hi = (offset & 0xffffffff00000000) >> 32;

	if (access & mmap_stream_access_t::write_commit) {
		data_ = MapViewOfFile(mmap->handle_, FILE_MAP_WRITE, hi, lo, size);
		opers_ |= stream_opers_t::write;
	}
	else if (access & mmap_stream_access_t::write_copy) {
		data_ = MapViewOfFile(mmap->handle_, FILE_MAP_COPY, hi, lo, size);
		opers_ |= stream_opers_t::write;
	}
	else {
		data_ = MapViewOfFile(mmap->handle_, FILE_MAP_READ, hi, lo, size);
	}

	if (data_)
		memory_stream_reset(data_, size);
}

auto mmap_stream_t::stream_opers() const -> stream_opers_mask_t
{
	return opers_;
}
