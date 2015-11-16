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

	struct abstract_filesystem_t : atma::ref_counted
	{
		virtual auto working_dir() const -> stdfs::path const& = 0;

		virtual auto generate_path(atma::string const&) -> fs_path_ptr
		{
			return fs_path_ptr::null;
		}

		virtual auto open(fs_path_ptr const&, open_mask_t) -> stream_ptr
		{
			return stream_ptr::null;
		}

		virtual auto internal_cd(fs_path_t*, stdfs::path const&) -> fs_path_ptr
		{
			return fs_path_ptr::null;
		}
	};

	using abstract_filesystem_ptr = atma::intrusive_ptr<abstract_filesystem_t>;

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

	struct physical_filesystem_t : abstract_filesystem_t
	{
		physical_filesystem_t(stdfs::path const&);

		auto working_dir() const -> stdfs::path const& override { return physical_path_; }

		auto open(fs_path_ptr const&, open_mask_t) -> stream_ptr override;

		auto generate_path(atma::string const& p) -> fs_path_ptr override;

		auto internal_cd(fs_path_t*, stdfs::path const&) -> fs_path_ptr override;

	private:
		stdfs::path physical_path_;
		fs_path_ptr root_;
	};

	using physical_filesystem_ptr = atma::intrusive_ptr<physical_filesystem_t>;

	struct fs_path_t : atma::ref_counted
	{
		auto cd(stdfs::path const&) -> fs_path_ptr;

	private:
		fs_path_t(abstract_filesystem_ptr const&, fs_path_t* parent, path_type_t, stdfs::path const& logical, stdfs::path const& physical);

	public:
		using children_t = atma::vector<fs_path_ptr>;

		abstract_filesystem_ptr fs_;
		fs_path_t* parent_;
		children_t children_;

		path_type_t type_;
		atma::string leaf_;
		stdfs::path logical_path_;
		stdfs::path physical_path_;

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
	struct vfs_t : abstract_filesystem_t
	{
		vfs_t();
		vfs_t(stdfs::path const& working_dir);

		auto working_dir() const -> stdfs::path const& override { return root_->physical_path_; }
		auto mount(stdfs::path const& logical, abstract_filesystem_ptr const&) -> void;

		auto open(stdfs::path const&) -> stream_ptr;
		auto open(stdfs::path const&, open_mask_t) -> stream_ptr;

	public:
		auto internal_cd(fs_path_t*, stdfs::path const&) -> fs_path_ptr override;

	private:
		auto logical_cd(stdfs::path const&) -> fs_path_ptr;

	private:
		fs_path_ptr root_;
	};
}
