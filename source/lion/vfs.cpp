#include <lion/vfs.hpp>

#include <lion/file.hpp>

#include <atma/algorithm.hpp>
#include <atma/idxs.hpp>

#include <regex>

using namespace lion;
using lion::vfs_t;

static std::regex logical_path_regex{"/(\\w+/)+"};

auto is_logical_absolute(atma::string const& path) -> bool
{
	return !path.empty() && path.c_str()[0] == '/';
}

vfs_t::vfs_t()
	: root_{"/"}
{
}

auto path_is_valid_logical_path(atma::string const& path) -> bool
{
	return std::regex_match(path.c_str(), logical_path_regex);
}

auto vfs_t::mount(atma::string const& path, filesystem_ptr const& fs) -> void
{
	ATMA_ASSERT(path_is_valid_logical_path(path));

	mount_node_t* m = nullptr;

	for (auto const& x : path_split_range(path))
	{
		if (x == "/")
		{
			m = &root_;
		}
		else if (x == "../")
		{
			ATMA_HALT("no relative paths for mounting");
		}
		else
		{
			auto it = std::find_if(m->children.begin(), m->children.end(), [&x](mount_node_t const& c) {
				return c.name == x;
			});

			if (it != m->children.end())
				m = &*it;
			else {
				m->children.push_back({x});
				m = &m->children.back();
			}
		}
	}

	if (m)
		m->filesystem = fs;
}


auto vfs_t::open(atma::string const& path) -> stream_ptr
{
	ATMA_ASSERT(is_logical_absolute(path));

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
		else if (x == "../")
		{
			ATMA_HALT("no relative paths for the VFS");
		}
		else
		{
			auto it = std::find_if(m->children.begin(), m->children.end(), [&x](mount_node_t const& c) {
				return c.name == x;
			});

			if (it != m->children.end())
				m = &*it;
			else
			{
				
				for ( ; pi != r.end(); ++pi)
					fp = fp + atma::string(pi->begin(), pi->end());
				break;
			}
		}
	}

	if (m && m->filesystem)
		return m->filesystem->open(fp.c_str(), lion::open_flags_t::read);

	return stream_ptr::null;
}

#if 0
auto vfs_t::cd(fs_path_t* parent, stdfs::path const& path) -> fs_path_ptr
{
	ATMA_ASSERT(parent);

	auto lp = parent->logical_path_;
	auto pp = stdfs::path{};

	for (auto leaf : path)
	{
		lp /= leaf;

		auto child = fs_path_ptr::make(shared_from_this<filesystem_t>(), parent, path_type_t::dir, lp, pp);

		parent->children_.push_back(child);
		parent = parent->children_.back().get();
	}

	return parent->shared_from_this<fs_path_t>();
}

auto vfs_t::logical_cd(stdfs::path const& path) -> fs_path_ptr
{
	if (path.empty())
		return fs_path_ptr::null;
	else
		return root_->cd(path);
}
#endif
