#pragma once

#include <atma/intrusive_ptr.hpp>

#include <filesystem>

namespace stdfs = std::tr2::sys;

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
	
	struct abstract_filesystem_t : atma::ref_counted
	{
		virtual auto generate_path(atma::string const&) -> fs_path_ptr {return{};}

		virtual auto internal_cd(fs_path_t*, stdfs::path const&) -> fs_path_ptr {return{};}
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


	



	// 
	struct vfs_t : abstract_filesystem_t
	{
		vfs_t();

		auto mount(stdfs::path const& logical, abstract_filesystem_ptr const&) -> void;
		auto set_working_dir(stdfs::path const& logical) -> void;


		auto internal_cd(fs_path_t*, stdfs::path const&) -> fs_path_ptr override;

	private:
		fs_path_ptr root_;
	};
}
