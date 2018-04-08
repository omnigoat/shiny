#include <lion/filesystem.hpp>

#include <rose/mmap.hpp>
#include <atma/algorithm.hpp>

#include <regex>

#include <atma/config/platform.hpp>

using namespace lion;



namespace
{
	std::regex logical_path_regex{"/([ a-zA-Z0-9_.]+/)+([ a-zA-Z0-9_.]+)?"};
	std::regex extension_regex{R"&(.+?\.(.+)$)&"};

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

	for (auto const& x : rose::path_split_range(p))
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
	: physical_path_(stdfs::absolute(path.c_str()).generic_u8string().c_str())
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

auto physical_filesystem_t::open(path_t const& path, file_access_mask_t mask) -> atma::stream_ptr
{
	auto fsp = cd(root_, path.string());

	if (fsp->stream())
		return fsp->stream();

	rose::mmap_ptr mmap;
	if (mask & file_access_t::write) {
		mmap = rose::mmap_ptr::make(fsp->path(), file_access_t::write);
	}
	else {
		mmap = rose::mmap_ptr::make(fsp->path(), file_access_t::read);
	}

	if (mask & file_access_t::write) {
		if (mask & file_access_t::nonbacked)
			return mmap_bytestream_ptr::make(mmap, mmap_bytestream_access_t::write_copy);
		else
			return mmap_bytestream_ptr::make(mmap, mmap_bytestream_access_t::write_commit);
	}
	else {
		return mmap_bytestream_ptr::make(mmap, mmap_bytestream_access_t::read);
	}
}


vfs_t::vfs_t(rose::runtime_t* rr)
	: rose_runtime_{rr}
	, root_{"/"}
{
	ATMA_ASSERT(rose_runtime_);
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


auto vfs_t::open(path_t const& path, atma::string* filepath) -> atma::stream_ptr
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
	{
		if (filepath)
			*filepath = (m->filesystem->physical_path() / fp).string();

		return m->filesystem->open(fp, lion::file_access_t::read);
	}

	return atma::stream_ptr::null;
}


auto vfs_t::add_filewatch(path_t const& path, filewatch_callback_t const& callback) -> void
{
	mount_node_t* mount = nullptr;
	path_t physpath;
	std::tie(mount, physpath) = get_physical_path(path);

	rose_runtime_->register_directory_watch(physpath, false, rose::file_change_t::changed,
		[=](path_t const& filename, rose::file_change_t change) {
			auto stream = mount->filesystem->open(path/filename, lion::file_access_t::read);
			while (stream->stream_status() != stream_status_t::good)
				stream = mount->filesystem->open(path / filename, lion::file_access_t::read);
			callback(physpath/filename, change, atma::stream_cast<lion::input_stream_t>(stream));
		});

}

auto vfs_t::get_physical_path(path_t const& logical) -> std::tuple<mount_node_t*, path_t>
{
	ATMA_ASSERT(path_is_valid_logical_path(logical.string()));

	mount_node_t* m = nullptr;

	atma::string fp;
	auto r = path_split_range(logical);
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
	{
		return {m, m->filesystem->physical_path() / fp};
	}

	return {};
}

