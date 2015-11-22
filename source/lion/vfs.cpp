#include <lion/vfs.hpp>

#include <lion/file.hpp>

#include <atma/algorithm.hpp>


using namespace lion;
using lion::vfs_t;


vfs_t::vfs_t()
{
	root_ = fs_path_ptr::make(shared_from_this<filesystem_t>(), nullptr, path_type_t::dir, "/", stdfs::current_path());
}

auto vfs_t::mount(stdfs::path const& logical_path, filesystem_ptr const& fs) -> void
{
	ATMA_ASSERT(logical_path.is_absolute());

	fs_path_ptr lp;

	if (auto lp = logical_cd(logical_path))
	{
		lp->fs_ = fs;
		lp->physical_path_ = fs->working_dir();
	}
}

auto vfs_t::open(stdfs::path const& logical_path) -> stream_ptr
{
	ATMA_ASSERT(logical_path.is_absolute());

	auto lp = logical_cd(logical_path);

	return lp->fs_->open(lp, open_mask_t{open_flags_t::read});
}

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
