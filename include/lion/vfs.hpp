#pragma once

#include <lion/streams.hpp>

#include <atma/intrusive_ptr.hpp>
#include <atma/bitmask.hpp>
#include <atma/string.hpp>

#include <filesystem>


namespace stdfs = std::experimental::filesystem;

namespace lion
{
	enum class path_type_t
	{
		dir,
		file,
		symlink
	};

	struct fs_path_t;
	using  fs_path_ptr  = atma::intrusive_ptr<fs_path_t>;
	
	enum class open_flags_t
	{
		read,
		write,
		exclusive,
		nonbacked,
	};

	using open_mask_t = atma::bitmask_t<open_flags_t>;

	struct filesystem_t : atma::ref_counted
	{
		//virtual auto physical_path() const -> stdfs::path const& = 0;
		virtual auto physical_path() const -> stdfs::path const& = 0;
		virtual auto root_path() const -> fs_path_ptr const& = 0;

		virtual auto cd(stdfs::path const& path) -> fs_path_ptr { return cd(root_path(), path); }
		virtual auto cd(fs_path_ptr const&, stdfs::path const&) -> fs_path_ptr { return fs_path_ptr::null; }
		virtual auto open(fs_path_ptr const&, open_mask_t) -> stream_ptr { return stream_ptr::null; }

	protected:
		auto impl_mkdir(fs_path_ptr const&, fs_path_ptr const&) -> bool;

	private:
		//virtual auto internal_cd(fs_path_t*, stdfs::path const&) -> fs_path_ptr { return fs_path_ptr::null; }

		friend struct fs_path_t;
	};

	using filesystem_ptr = atma::intrusive_ptr<filesystem_t>;

	inline auto split_path(atma::string const& str) -> atma::vector<atma::string>
	{
		char const* const delims = "\\/";

		atma::vector<atma::string> r;

		auto i = str.begin();
		while (i != str.end())
		{
			auto e = atma::find_first_of(str, i, delims);

			auto s = atma::string{i, e};
			s.push_back('/');
			r.push_back(s);
			i = ++e;
		}
		
		return r;
	}

	struct physical_filesystem_t : filesystem_t
	{
		physical_filesystem_t(stdfs::path const&);

		auto physical_path() const -> stdfs::path const& override { return physical_path_; }

		auto open(fs_path_ptr const&, open_mask_t) -> stream_ptr override;

		auto cd(fs_path_ptr const&, stdfs::path const&) -> fs_path_ptr override;

	private:
		stdfs::path physical_path_;
		fs_path_ptr root_;
	};

	using physical_filesystem_ptr = atma::intrusive_ptr<physical_filesystem_t>;

	struct fs_path_t : atma::ref_counted
	{
		using children_t = atma::vector<fs_path_ptr>;

		auto filesystem() const -> filesystem_ptr const& { return fs_; }
		auto children() const -> children_t const& { return children_; }
		auto path_type() const -> path_type_t { return type_; }
		auto logical_path() const -> stdfs::path const& { return logical_path_; }
		auto physical_path() const -> stdfs::path const& { return physical_path_; }

	private:
		fs_path_t(filesystem_ptr const&, fs_path_t* parent, path_type_t, stdfs::path const& logical, stdfs::path const& physical);

	private:

		filesystem_ptr fs_;
		fs_path_t* parent_;
		children_t children_;

		path_type_t type_;
		atma::string leaf_;
		stdfs::path logical_path_;
		stdfs::path physical_path_;

		friend struct filesystem_t;
		friend struct atma::intrusive_ptr_expose_constructor;
	};

	inline auto operator == (fs_path_t const& lhs, fs_path_t const& rhs) -> bool
	{
		//return lhs.to_string() == rhs.to_string();
		return true;
	}

	inline auto operator != (fs_path_t const& lhs, fs_path_t const& rhs) -> bool
	{
		return !operator == (lhs, rhs);
	}


	
	inline auto remove_toplevel(stdfs::path const& p) -> stdfs::path
	{
		auto const& k = p.native();
		auto i = k.find(stdfs::path::preferred_separator);

		if (i == k.npos)
			return stdfs::path{};
		else
			return stdfs::path{k.c_str() + i + 1, k.c_str() + k.size()};
	}
	


	// 
	struct vfs_t : filesystem_t
	{
		vfs_t();
		vfs_t(stdfs::path const& physical_path);

		auto physical_path() const -> stdfs::path const& override { return root_->physical_path(); }
		auto mount(stdfs::path const& logical, filesystem_ptr const&) -> void;

		auto open(stdfs::path const&) -> stream_ptr;
		auto open(stdfs::path const&, open_mask_t) -> stream_ptr;

		auto cd(fs_path_ptr const&, stdfs::path const&) -> fs_path_ptr override;

	private:
		auto logical_cd(stdfs::path const&) -> fs_path_ptr;

	private:
		fs_path_ptr root_;
	};
}
