#include <lion/filesystem.hpp>

#include <lion/mmap.hpp>
#include <lion/file.hpp>

#include <atma/algorithm.hpp>

#include <regex>

using namespace lion;



namespace
{
	std::regex logical_path_regex{"/([ a-zA-Z0-9_.]+/)+([ a-zA-Z0-9_.]+)?"};

	auto path_is_valid_logical_path(atma::string const& path) -> bool
	{
		return std::regex_match(path.c_str(), logical_path_regex);
	}
}


fs_path_t::fs_path_t(filesystem_ptr const& fs, fs_path_t* parent, path_type_t type, atma::string const& pathleaf)
	: fs_(fs), parent_(parent), type_(type), leaf_(pathleaf)
{
	mk_path(path_);
}

auto fs_path_t::mk_path(path_t& dest) const -> void
{
	if (parent_)
		parent_->mk_path(dest);
	dest /= leaf_;
}


auto filesystem_t::cd(fs_path_ptr const& fsp, atma::string const& p) -> fs_path_ptr
{
	fs_path_t* r = root_path().get();

	for (auto const& x : path_split_range(p))
	{
		if (x == "/")
		{
			r = root_path().get();
		}
		else if (x == "../")
		{
			r = r->parent();
		}
		else
		{
			bool found = false;
			for (auto const& child : fsp->children_)
			{
				if (child->leaf() == x) {
					found = true;
					r = child.get();
					break;
				}
			}

			if (!found)
			{
				if (auto c = impl_subpath(r->shared_from_this<fs_path_t>(), atma::string{x}.c_str()))
				{
					r->children_.push_back(c);
					r = r->children_.back().get();
				}
			}
		}
	}

	if (r)
		return r->shared_from_this<fs_path_t>();
	else
		return fs_path_ptr::null;
}




physical_filesystem_t::physical_filesystem_t(atma::string const& path)
	: physical_path_(path)
{
	ATMA_ASSERT(stdfs::exists(physical_path_.c_str()));

	root_ = fs_path_ptr::make(shared_from_this<filesystem_t>(), nullptr, path_type_t::dir, path);
}

auto physical_filesystem_t::impl_subpath(fs_path_ptr const& fsp, char const* name) -> fs_path_ptr
{
	ATMA_ASSERT(fsp);
	ATMA_ASSERT(fsp->path_type() == path_type_t::dir);

	auto fp = stdfs::path{fsp->path() / name};

	std::error_code err;
	auto status = stdfs::status(fp, err);
	if (err) {
		ATMA_HALT("OS-level error for filesystem");
		return fs_path_ptr::null;
	}

	auto type = path_type_t::unknown;
	switch (status.type())
	{
		case stdfs::file_type::directory:
			type = path_type_t::dir;
			break;

		case stdfs::file_type::regular:
			type = path_type_t::file;
			break;

		case stdfs::file_type::symlink:
			type = path_type_t::symlink;
			break;

		default:
			break;
	}
	
	return fs_path_ptr::make(shared_from_this<filesystem_t>(), fsp.get(), type, name);
}

auto physical_filesystem_t::open(path_t const& path, open_mask_t mask) -> stream_ptr
{
	auto fsp = cd(root_, path.string());

	if (fsp->stream())
		return fsp->stream();

	// we can use a mmap for most cases, except where we need to write to a file.
	// if we want to mutate the contents of a file, but don't care about having those
	// changes actually written to disk, then we can use 'nonbacked', which will create
	// a copy-on-write mmap (or something similar)
	if ((mask & open_flags_t::read) || ((mask & open_flags_t::write) && (mask & open_flags_t::nonbacked)))
	{
		auto mmap = mmap_ptr::make(fsp->path());
		return mmap_stream_ptr::make(mmap);
	}
	else
	{
		return stream_ptr{new file_t{path.string(), file_access_t::write}};
	}

	return stream_ptr::null;
}


vfs_t::vfs_t()
	: root_{"/"}
{
}

auto vfs_t::mount(path_t const& path, filesystem_ptr const& fs) -> void
{
	ATMA_ASSERT(path_is_valid_logical_path(path.string()));

	mount_node_t* m = nullptr;

	for (auto const& x : path_split_range(path))
	{
		if (x == "/")
		{
			m = &root_;
		}
		else
		{
			auto it = std::find_if(m->children.begin(), m->children.end(), [&x](mount_node_t const& c) {
				return c.name == x;
			});

			if (it != m->children.end()) {
				m = &*it;
			}
			else {
				m->children.push_back({x});
				m = &m->children.back();
			}
		}
	}

	if (m)
		m->filesystem = fs;
}


auto vfs_t::open(path_t const& path) -> stream_ptr
{
	ATMA_ASSERT(path_is_valid_logical_path(path.string()));

	mount_node_t* m = nullptr;

	atma::string fp;
	auto r = path_split_range(path);
	for (auto pi = r.begin(); pi != r.end(); ++pi)
	{
		auto const& x = *pi;

		if (x == "/")
		{
			m = &root_;
		}
		else
		{
			auto it = std::find_if(m->children.begin(), m->children.end(), [&x](mount_node_t const& c) {
				return c.name == x;
			});

			if (it != m->children.end()) {
				m = &*it;
			}
			else {
				atma::for_each(pi, r.end(), atma::utf8_appender_t{fp});
				break;
			}
		}
	}

	if (m && m->filesystem)
		return m->filesystem->open(fp, lion::open_flags_t::read);

	return stream_ptr::null;
}


