#include <lion/vfs.hpp>

#include <lion/file.hpp>

#include <atma/algorithm.hpp>
#include <atma/idxs.hpp>


using namespace lion;
using lion::vfs_t;

auto is_logical_absolute(atma::string const& path) -> bool
{
	return !path.empty() && path.c_str()[0] == '/';
}

vfs_t::vfs_t()
	: root_{"/"}
{
}

auto vfs_t::mount(atma::string const& logical_path, filesystem_ptr const& fs) -> void
{
	ATMA_ASSERT(is_logical_absolute(logical_path));

	mount_node_t* m = nullptr;

	for (auto const& x : path_split_range(logical_path))
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
					fp.append(pi->begin(), pi->end());
				break;
			}
		}
	}

	if (m && m->filesystem)
		return m->filesystem->open(fp, lion::open_flags_t::read);
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

auto vfs_t::logical_cd(stdfs::path const& logical_path) -> fs_path_ptr
{
	if (logical_path.empty())
		return fs_path_ptr::null;
	else
		return root_->cd(logical_path);
}
#endif
