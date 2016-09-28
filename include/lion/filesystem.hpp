#pragma once


#include <lion/streams.hpp>

#include <rose/path.hpp>
#include <rose/runtime.hpp>

#include <atma/intrusive_ptr.hpp>
#include <atma/bitmask.hpp>
#include <atma/string.hpp>
#include <atma/streams.hpp>

#include <filesystem>




//=====================================================================
// fs_path_t
//=====================================================================
namespace lion
{
	enum class path_type_t
	{
		unknown,
		dir,
		file,
		symlink
	};

	using path_t = rose::path_t;

	struct fs_path_t;
	using  fs_path_ptr = atma::intrusive_ptr<fs_path_t>;

	struct filesystem_t;
	using  filesystem_ptr = atma::intrusive_ptr<filesystem_t>;

	//struct asset_t;
	//using  asset_ptr = atma::intrusive_ptr<asset_t>;


	struct fs_path_t : atma::ref_counted
	{
		using children_t = atma::vector<fs_path_ptr>;

		auto filesystem() const -> filesystem_ptr const& { return fs_; }
		auto parent() const -> fs_path_t* { return parent_; }
		auto children() const -> children_t const& { return children_; }
		auto path_type() const -> path_type_t { return type_; }
		auto leaf() const -> atma::string const& { return leaf_; }

		auto path() const -> path_t const& { return path_; }
		auto stream() const -> atma::stream_ptr const& { return stream_; }
		//auto asset() const -> asset_ptr const& { return asset_; }

	private:
		fs_path_t(filesystem_ptr const&, fs_path_t* parent, path_type_t, atma::string const& pathleaf);

		auto mk_path(path_t&) const -> void;

	private:
		filesystem_ptr fs_;
		fs_path_t* parent_;
		children_t children_;

		path_type_t type_;
		path_t path_;
		atma::string leaf_;

		atma::stream_ptr stream_;
		//asset_ptr asset_;

		friend struct filesystem_t;
		friend struct atma::enable_intrusive_ptr_make;
	};

}




//=====================================================================
// filesystem_t
//=====================================================================
namespace lion
{
	using file_access_t = rose::file_access_t;
	using file_access_mask_t = rose::file_access_mask_t;


	//
	// filesystem_t
	//
	struct filesystem_t : atma::ref_counted
	{
		virtual auto physical_path() const -> path_t const& = 0;
		virtual auto root_path() const -> fs_path_ptr const& = 0;

		auto cd(fs_path_ptr const&, atma::string const&) -> fs_path_ptr;
		virtual auto cd(atma::string const& path) -> fs_path_ptr { return cd(root_path(), path); }
		virtual auto open(path_t const&, file_access_mask_t) -> atma::stream_ptr { return atma::stream_ptr::null; }

	protected:
		// returns a fs_path_ptr to a 
		virtual auto impl_subpath(fs_path_ptr const&, char const*) -> fs_path_ptr { return fs_path_ptr::null; }

	private:
		friend struct fs_path_t;
	};




	//
	// physical_filesystem_t
	//
	struct physical_filesystem_t : filesystem_t
	{
		physical_filesystem_t(atma::string const&);

		auto physical_path() const -> path_t const& override { return physical_path_; }
		auto root_path() const -> fs_path_ptr const& override { return root_; }

		auto open(path_t const&, file_access_mask_t) -> atma::stream_ptr override;

	private:
		auto impl_subpath(fs_path_ptr const&, char const*) -> fs_path_ptr override;

	private:
		path_t physical_path_;
		fs_path_ptr root_;
	};

	using physical_filesystem_ptr = atma::intrusive_ptr<physical_filesystem_t>;




	//
	// vfs_t
	//
	struct vfs_t
	{
		using filewatch_callback_t = atma::function<void(rose::path_t const&, rose::file_change_t, lion::input_stream_ptr const&)>;

		vfs_t(rose::runtime_t*);

		auto mount(path_t const&, filesystem_ptr const&) -> void;

		auto open(path_t const&, atma::string* filepath = nullptr) -> atma::stream_ptr;
		auto open(path_t const&, file_access_mask_t) -> atma::stream_ptr;

		auto add_filewatch(path_t const&, filewatch_callback_t const&) -> void;

	private:
		struct mount_node_t
		{
			using children_t = atma::vector<mount_node_t>;

			atma::string name;
			children_t children;

			filesystem_ptr filesystem;
		};

		auto get_physical_path(path_t const&) -> std::tuple<mount_node_t*, path_t>;

	private:
		rose::runtime_t* rose_runtime_;
		mount_node_t root_;
	};

}
