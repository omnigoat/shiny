#pragma once

#include <lion/streams.hpp>

#include <atma/intrusive_ptr.hpp>
#include <atma/bitmask.hpp>
#include <atma/string.hpp>

#include <filesystem>


namespace stdfs = std::experimental::filesystem;

namespace lion
{
	struct asset_t : atma::ref_counted
	{
	};

	using asset_ptr = atma::intrusive_ptr<asset_t>;

	enum class path_type_t
	{
		unknown,
		dir,
		file,
		symlink
	};


	struct fs_path_t;
	using  fs_path_ptr = atma::intrusive_ptr<fs_path_t>;

	struct filesystem_t;
	using filesystem_ptr = atma::intrusive_ptr<filesystem_t>;


	//
	// fs_path_t
	//  - unique per VFS (so if two filesystems have identical directory structure, thus
	//    returning two fs_path_t of same layout, they will still be differentiated)
	//
	struct fs_path_t : atma::ref_counted
	{
		using children_t = atma::vector<fs_path_ptr>;

		auto filesystem() const -> filesystem_ptr const& { return fs_; }
		auto parent() const -> fs_path_t* { return parent_; }
		auto children() const -> children_t const& { return children_; }
		auto path_type() const -> path_type_t { return type_; }
		auto leaf() const -> atma::string const& { return leaf_; }

		auto path_string() const -> atma::string;
		auto stream() const -> stream_ptr const& { return stream_; }
		auto asset() const -> asset_ptr const& { return asset_; }

		auto set_filesystem(filesystem_ptr const& fs) -> void { fs_ = fs; }

	private:
		fs_path_t(filesystem_ptr const&, fs_path_t* parent, path_type_t, stdfs::path const& logical, stdfs::path const& physical);

		auto path_string_impl(atma::string&) const -> void;

	private:
		atma::string vfs_path_;
		filesystem_ptr fs_;
		fs_path_t* parent_;
		children_t children_;

		path_type_t type_;
		atma::string leaf_;

		stream_ptr stream_;
		asset_ptr asset_;

		friend struct filesystem_t;
		friend struct atma::intrusive_ptr_expose_constructor;
	};

	

#if 0
	inline auto operator == (fs_path_t const& lhs, fs_path_t const& rhs) -> bool
	{
		//return lhs.to_string() == rhs.to_string();
		return true;
	}

	inline auto operator != (fs_path_t const& lhs, fs_path_t const& rhs) -> bool
	{
		return !operator == (lhs, rhs);
	}
#endif




	enum class open_flags_t
	{
		read,
		write,
		exclusive,
		nonbacked,
	};

	ATMA_BITMASK(open_mask_t, open_flags_t);


	//
	// filesystem_t
	//
	struct filesystem_t : atma::ref_counted
	{
		virtual auto physical_path() const -> stdfs::path const& = 0;
		virtual auto root_path() const -> fs_path_ptr const& = 0;

		auto cd(fs_path_ptr const&, atma::string const&) -> fs_path_ptr;
		virtual auto cd(atma::string const& path) -> fs_path_ptr { return cd(root_path(), path); }
		virtual auto open(atma::string const&, open_mask_t) -> stream_ptr { return stream_ptr::null; }

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
		physical_filesystem_t(stdfs::path const&);

		auto physical_path() const -> stdfs::path const& override { return physical_path_; }
		auto root_path() const -> fs_path_ptr const& override { return root_; }

		auto open(atma::string const&, open_mask_t) -> stream_ptr override;

	private:
		auto impl_subpath(fs_path_ptr const&, char const*) -> fs_path_ptr override;

	private:
		stdfs::path physical_path_;
		fs_path_ptr root_;
	};

	using physical_filesystem_ptr = atma::intrusive_ptr<physical_filesystem_t>;

	




	inline auto findinc_path_separator(char const* begin, char const* end) -> char const*
	{
		auto i = begin;
		while (i != end && *i != '/')
			++i;

		return (i == end) ? i : i + 1;
	}

	struct path_range_iter_t
	{
		path_range_iter_t(char const* begin, char const* end, char const* terminal)
			:range_{begin, end}, terminal_(terminal)
		{}

		auto operator * () const -> atma::utf8_string_range_t const&
		{
			return range_;
		}

		auto operator -> () const -> atma::utf8_string_range_t const*
		{
			return &range_;
		}

		auto operator ++ () -> path_range_iter_t&
		{
			range_ = atma::utf8_string_range_t{range_.end(), findinc_path_separator(range_.end(), terminal_)};
			return *this;
		}

		auto operator != (path_range_iter_t const& rhs) const -> bool
		{
			return range_ != rhs.range_ || terminal_ != rhs.terminal_;
		}

	private:
		atma::utf8_string_range_t range_;
		char const* terminal_;
	};

	struct path_range_t
	{
		path_range_t(char const* str, size_t size)
			: str_(str), size_(size)
		{}

		auto begin() -> path_range_iter_t
		{
			auto r = findinc_path_separator(str_, str_ + size_);
			return{str_, r, str_ + size_};
		}

		auto end() -> path_range_iter_t
		{
			return{str_ + size_, str_ + size_, str_ + size_};
		}

	private:
		char const* str_;
		size_t size_;
	};


	inline auto path_split_range(atma::string const& p) -> path_range_t
	{
		return path_range_t{p.c_str(), p.raw_size()};
	}
	
	// 
	struct vfs_t
	{
		vfs_t();

		auto mount(atma::string const&, filesystem_ptr const&) -> void;

		auto open(atma::string const&) -> stream_ptr;
		auto open(atma::string const&, open_mask_t) -> stream_ptr;

	private:
		struct mount_node_t
		{
			using children_t = atma::vector<mount_node_t>;

			atma::string name;
			children_t children;

			filesystem_ptr filesystem;
		};

	private:
		mount_node_t root_;
	};
}
