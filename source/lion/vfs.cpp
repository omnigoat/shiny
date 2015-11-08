#include <lion/vfs.hpp>

#include <lion/file.hpp>

#include <atma/algorithm.hpp>


using namespace lion;
using lion::vfs_t;

vfs_t::vfs_t()
{
	root_ = fs_path_ptr::make(shared_from_this<abstract_filesystem_t>(), nullptr, path_type_t::dir, "/", "");
}

auto vfs_t::mount(stdfs::path const& logical, abstract_filesystem_ptr const& fs) -> void
{
	fs_path_ptr lp;

	if (logical.empty())
		return;
	else if (logical.c_str()[0] == stdfs::path::preferred_separator)
		lp = root_->cd(remove_toplevel(logical));
	else
		lp = root_->cd(logical);

	lp->fs_ = fs;
	lp->physical_path_ = fs->working_dir();
}

auto vfs_t::open(stdfs::path const& path) -> abstract_stream_ptr
{
	auto lp = logical_cd(path);

	return lp->fs_->open(lp, open_mask_t{open_flags_t::read});
}

auto vfs_t::internal_cd(fs_path_t* parent, stdfs::path const& path) -> fs_path_ptr
{
	ATMA_ASSERT(parent);

	auto lp = parent->logical_path_;
	auto pp = stdfs::path{};

	for (auto leaf : path)
	{
		lp /= leaf;

		auto child = fs_path_ptr::make(shared_from_this<abstract_filesystem_t>(), parent, path_type_t::dir, lp, pp);

		parent->children_.push_back(child);
		parent = parent->children_.back().get();
	}

	return parent->shared_from_this<fs_path_t>();
}

auto vfs_t::logical_cd(stdfs::path const& logical) -> fs_path_ptr
{
	if (logical.empty())
		return fs_path_ptr::null;
	else if (logical.c_str()[0] == stdfs::path::preferred_separator)
		return root_->cd(remove_toplevel(logical));
	else
		return root_->cd(logical);
}
