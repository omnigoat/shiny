#include <lion/file.hpp>

#include <lion/vfs.hpp>
#include <lion/mmap.hpp>

#include <atma/algorithm.hpp>


using namespace lion;
using lion::file_t;


auto toplevel_dir(stdfs::path const& p) -> stdfs::path
{
	auto const& k = p.native();
	auto i = k.find(stdfs::path::preferred_separator);

	if (i == k.npos)
		return {};
	else
		return {k.c_str(), k.c_str() + i};
}



auto cd(lion::fs_path_ptr const& p, atma::string const& s) -> fs_path_ptr const&
{
	return atma::find_if_else(p->children_, fs_path_ptr::null, [&s](fs_path_ptr const& x) {
		return x->type_ == path_type_t::dir && x->physical_path_.filename() == s.c_str();
	});
}

lion::physical_filesystem_t::physical_filesystem_t(stdfs::path const& pp)
	: physical_path_(pp)
{
	ATMA_ASSERT(stdfs::exists(physical_path_));
}

auto lion::physical_filesystem_t::cd(atma::string const& p) -> fs_path_ptr
{
	fs_path_ptr r;
	root_ = fs_path_ptr::make(shared_from_this<filesystem_t>(), nullptr, path_type_t::dir, stdfs::path{"/"}, stdfs::path{"."});

	stdfs::path rp = p.c_str();

	auto path_strings = split_path(p);
	if (path_strings.empty())
		return fs_path_ptr::null;

	stdfs::path lp;
	stdfs::path fp;
	for (auto si = path_strings.begin(), sie = path_strings.end() - 1; si != path_strings.end(); ++si)
	{
		auto const& s = *si;

		if (s == "/")
		{
			ATMA_ASSERT(si == path_strings.begin());
			r = root_;
			lp = "/";
			fp = ".";
		}
		else
		{
			lp /= s.c_str();
			fp /= s.c_str();
			
			if (auto r2 = cd(r, s))
			{
				r = r2;
			}
			else
			{
				auto type = s.raw_end()[-1] == '/' ? path_type_t::dir : path_type_t::file;
				auto np = fs_path_ptr::make(shared_from_this<filesystem_t>(), r.get(), type, lp, fp);
				r->children_.push_back(np);
				r = np;
			}
		}
	}

	return r;
}

auto lion::physical_filesystem_t::internal_cd(fs_path_t* parent, stdfs::path const& path) -> fs_path_ptr
{
	ATMA_ASSERT(parent);

	auto lp = parent->logical_path_;
	auto pp = parent->physical_path_;

	for (auto leaf : path)
	{
		lp /= leaf;
		pp /= leaf;

		if (stdfs::exists(pp))
		{
			auto child = fs_path_ptr::make(shared_from_this<filesystem_t>(), parent, path_type_t::dir, lp, pp);

			parent->children_.push_back(child);
			parent = parent->children_.back().get();
		}
		else
		{
			return fs_path_ptr::null;
		}
	}

	return parent->shared_from_this<fs_path_t>();
}

auto lion::fs_path_t::cd(stdfs::path const& p) -> fs_path_ptr
{
	auto tp = toplevel_dir(p);

	for (auto const& x : children_)
	{
		if (tp.string() == x->logical_path_.filename().string())
			return x->cd(remove_toplevel(p));
	}

	return fs_->internal_cd(this, p);
}

auto physical_filesystem_t::open(fs_path_ptr const& path, open_mask_t mask) -> stream_ptr
{
	// we can use a mmap for most cases, except where we need to write to a file
	// if we want to mutate the contents of a file, but don't care about having those
	// changes actually written to disk, then we can use 'nonbacked', which will create
	// a copy-on-write mmap (or something similar)
	if ((mask & open_flags_t::read) || ((mask & open_flags_t::write) && (mask & open_flags_t::nonbacked)))
	{
		auto mmap = mmap_ptr::make(path->physical_path_.string());
		return mmap_stream_ptr::make(mmap);
	}
	else
	{
	}

	return stream_ptr::null;
}


















lion::fs_path_t::fs_path_t(filesystem_ptr const& fs, fs_path_t* parent, lion::path_type_t type, stdfs::path const& logical, stdfs::path const& physical)
	: fs_(fs), parent_(parent), type_(type), logical_path_(logical), physical_path_(physical)
{

}

file_t::file_t()
	: filename_()
	, access_()
	, filesize_()
{}

file_t::file_t(atma::string const& filename, file_access_t access)
	: filename_(filename)
	, access_(access)
	, filesize_()
{
	char const* fa[] = {"r", "w", "r+"};
	handle_ = fopen(filename.c_str(), fa[(uint)access]);
	if (handle_ == nullptr)
		return;

	// get filesize
	fseek(handle_, 0, SEEK_END);
	filesize_ = ftell(handle_);
	fseek(handle_, 0, SEEK_SET);
}

file_t::file_t(file_t&& rhs)
	: filename_(rhs.filename_)
	, access_(rhs.access_)
	, filesize_(rhs.filesize_)
	, handle_(rhs.handle_)
{
	rhs.handle_ = nullptr;
}

file_t::~file_t()
{
	if (handle_)
		fclose(handle_);
}

auto file_t::valid() const -> bool
{
	return handle_ != nullptr;
}

auto file_t::size() const -> size_t
{
	return filesize_;
}

auto file_t::position() const -> size_t
{
	return ftell(handle_);
}

auto file_t::seek(size_t x) -> stream_status_t
{
	auto r = fseek(handle_, (long)x, SEEK_SET);
	if (r == 0)
		return stream_status_t::good;
	else
		return stream_status_t::error;
}

auto file_t::move(int64 x) -> stream_status_t
{
	auto r = fseek(handle_, (long)x, SEEK_CUR);
	if (r == 0)
		return stream_status_t::good;
	else
		return stream_status_t::error;
}

auto file_t::read(void* buf, size_t size) -> read_result_t
{
	size_t r = fread(buf, 1, size, handle_);

	if (r == size)
		return {stream_status_t::good, r};
	else if (feof(handle_))
		return {stream_status_t::eof, r};
	else
		return {stream_status_t::error, r};
}

auto file_t::write(void const* data, size_t size) -> write_result_t
{
	size_t r = fwrite(data, 1, size, handle_);

	if (r == size)
		return {stream_status_t::good, r};
	else if (feof(handle_))
		return {stream_status_t::eof, r};
	else
		return {stream_status_t::error, r};
}

// absract-stream
auto file_t::stream_opers() const -> stream_opers_mask_t
{
	return stream_opers_mask_t{stream_opers_t::read, stream_opers_t::write, stream_opers_t::random_access};
}

// input-stream
auto file_t::g_size() const -> size_t
{
	return filesize_;
}

auto file_t::g_seek(size_t x) -> stream_status_t
{
	return seek(x);
}

auto file_t::g_move(int64 x) -> stream_status_t
{
	return move(x);
}

// output-stream
auto file_t::p_size() const -> size_t
{
	return filesize_;
}

auto file_t::p_seek(size_t x) -> stream_status_t
{
	return seek(x);
}

auto file_t::p_move(int64 x) -> stream_status_t
{
	return move(x);
}
