#include <atma/string.hpp>
#include <atma/intrusive_ptr.hpp>
#include <atma/math/vector4f.hpp>
#include <atma/algorithm.hpp>
#include <atma/enable_if.hpp>
#include <atma/math/intersection.hpp>
#include <atma/bind.hpp>


#include <filesystem>




namespace shelf
{
	enum class path_type_t
	{
		dir,
		file,
		symlink
	};

	struct path_t;
	using  path_ptr = std::unique_ptr<path_t>; // atma::intrusive_ptr<path_t>;

	// path
	struct path_t
	{
		path_t();
		path_t(atma::string const&);
		path_t(path_t const&);

		auto to_string() const -> atma::string;

		auto is_file() const -> bool;

	private:
		path_t(atma::string const&, atma::string::const_iterator const&);

	private:
		atma::string name_;
		path_type_t type_;
		path_ptr child_;
	};


	path_t::path_t()
	{
	}

	path_t::path_t(atma::string const& str)
		: path_t(str, str.begin())
	{
	}

	path_t::path_t(atma::string const& str, atma::string::const_iterator const& begin)
	{
		char const* delims = "/\\";

		auto end = atma::find_first_of(str, begin, delims);
		if (end == str.end()) {
			type_ = path_type_t::file;
		}
		else {
			type_ = path_type_t::dir;
			++end;
		}

		name_ = atma::string(begin, end);

		if (end == begin)
			return;

		child_ = path_ptr(new path_t(str, end));
	}

	path_t::path_t(path_t const& rhs)
		: name_(rhs.name_), type_(rhs.type_)
	{
		if (rhs.child_)
			child_ = path_ptr(new path_t(*rhs.child_));
	}

	auto path_t::to_string() const -> atma::string
	{
		auto result = atma::string();

		for (auto t = this; t != nullptr; t = t->child_.get())
		{
			result += t->name_;
		}

		return result;
	}

	inline auto operator == (path_t const& lhs, path_t const& rhs) -> bool
	{
		return lhs.to_string() == rhs.to_string();
	}

	inline auto operator != (path_t const& lhs, path_t const& rhs) -> bool
	{
		return !operator == (lhs, rhs);
	}


	struct filesystem_t
	{
		auto add_mount(path_t const& virtual_path, path_t const& host_path) -> void;

	private:
		struct node_t;
		using  node_ptr = atma::intrusive_ptr<node_t>;
		using  nodes_t = std::vector<node_ptr>;
		struct mount_t;

		struct node_t : atma::ref_counted
		{
			atma::string name;
			path_type_t type;

			node_t* parent;
			nodes_t children;
		};

		struct mount_t
		{
			path_t host_path;
			path_t mount_path;
			node_ptr mount_node;
		};


	private:
		auto merge_paths(node_t*, node_t*) -> node_ptr;

	private:
		typedef std::vector<mount_t> mounts_t;

		mounts_t mounts_;
	};

	auto filesystem_t::add_mount(path_t const& mount_path, path_t const& host_path) -> void
	{
#ifdef _DEBUG
		atma::for_each(mounts_, [&](mount_t const& x) {  ATMA_ASSERT(x.host_path != host_path);  });
#endif

		// do a thing?
		// host_path   = "relative/path/";
		// host_path2  = "/absolute/path";
		// host_path3  = "/absolute/different/path";

		mounts_.push_back({
			host_path,
			mount_path,
			// something??
		});
	}
}

namespace math = atma::math;

#include <atma/unique_memory.hpp>
#include <array>

template <typename T>
struct default_octree_subdivider_t
{
	auto subdivide(uint depth, atma::math::vector4f const& bounds, T const& x) -> void
	{
		
	}
};

struct octree_preallocate_tag
{
	uint levels;
};

struct octree_t
{
	octree_t() {}

#if 0
	octree_t(octree_preallocate_tag t)
	{
		while (t.levels-- != 0)
		{
			
		}
	};

	//auto add_triangle(T const&, math::vector4f const&, math::vector4f const&, math::vector4f const&) -> void;
#endif

	auto insert_point(math::vector4f const&) -> bool;
	struct node_t;
	//node_t root_;

private:

private:

private:
	
};


#if 0
struct box_t
{
	math::vector4f origin;
	math::vector4f half_extents;

	static auto from_minmax(math::vector4f const& min, math::vector4f const& max) -> box_t
	{
		return box_t{(min + max) / 2.f, (max - min) / 2.f};
	}
};

auto intersect_aabb_box(math::vector4f const& aabb, box_t const& box) -> bool
{
	return !(
		aabb.x + aabb.w < box.origin.x - box.half_extents.x ||
		aabb.x - aabb.w > box.origin.x + box.half_extents.x ||
		aabb.y + aabb.w < box.origin.y - box.half_extents.y ||
		aabb.y - aabb.w > box.origin.y + box.half_extents.y ||
		aabb.z + aabb.w < box.origin.z - box.half_extents.z ||
		aabb.z - aabb.w > box.origin.z + box.half_extents.z)
		;
}
#endif



#if 1

struct octree_t::node_t
{
	node_t()
		: bounds(0.f, 0.f, 0.f, 0.5f)
	{
	}

	node_t(math::vector4f const& bounds)
		: bounds(bounds)
	{
	}

	atma::math::vector4f bounds;
	atma::math::triangle_t data;

	auto inbounds(math::vector4f const& point) -> bool;
	auto insert(math::triangle_t const&) -> bool;

private:
	auto child_ref(uint idx) -> node_t& { return reinterpret_cast<node_t*>(buf_.begin())[idx]; }

	auto oct_split() -> void
	{
		auto newbuf = atma::unique_memory_t{8 * sizeof(node_t)};
		std::swap(buf_, newbuf);

		for (auto i = 0u; i != 8u; ++i)
			new (&child_ref(i)) node_t{oct_subbound(bounds, i)};

		for (auto const& x : data_)
			insert(x);
	}

	auto oct_subbound(math::vector4f const&, uint) -> math::vector4f;

private:
	atma::unique_memory_t buf_;

	std::vector<math::triangle_t> data_;

	uint datas_;
};


auto octree_t::node_t::insert(math::triangle_t const& tri) -> bool
{
	if (!atma::math::intersect_aabb_triangle(tri))
	{
		return false;
	}

	// we are a full leaf node
	if (!children_ && datas_ == 8)
	{
		oct_split();
	}

	if (children_)
	{
		return std::any_of(children_, children_ + 8, [&point, &data](node_t& node) {
			return node.insert(point, data);
		});
	}

	data_[datas_] = data;
	dloc_[datas_] = point;
	++datas_;

	return true;
}

auto octree_t::node_t::inbounds(math::vector4f const& point) -> bool
{
	return
		point.x < bounds.x + bounds.w && point.x > bounds.x - bounds.w &&
		point.y < bounds.y + bounds.w && point.y > bounds.y - bounds.w &&
		point.z < bounds.z + bounds.w && point.z > bounds.z - bounds.w
		;
}

auto octree_t::node_t::oct_subbound(math::vector4f const& bounds, uint idx) -> math::vector4f
{
	return math::vector4f(
		(0.5f - ((idx & 1)     ) * 1.f) * bounds.w + bounds.x,
		(0.5f - ((idx & 2) >> 1) * 1.f) * bounds.w + bounds.y,
		(0.5f - ((idx & 4) >> 2) * 1.f) * bounds.w + bounds.z,
		bounds.w * 0.5f);
}

#endif

#if 0
template <typename F, typename G>
inline auto operator * (function_t<F> f, function_t<G> g)
-> function_t<
	atma::bind_t<
		typename curried_function_traits<F, G>::sg,
		std::tuple<function_t<F>, function_t<G>, atma::placeholder_t<0>>
	>>
{
	static_assert(atma::function_traits<F>::arity == 1, "can not compose functions of arity greater than 1");
	static_assert(atma::function_traits<G>::arity == 1, "can not compose functions of arity greater than 1");

	auto fnptr = &point<
		function_t<F>,
		function_t<G>,
		typename atma::function_traits<G>::template arg<0>::type>;

	return {atma::curry(std::forward<decltype(fnptr)>(fnptr), std::forward<function_t<F>>(f), std::forward<function_t<G>>(g))};
}
#endif


#include <dust/runtime.hpp>
#include <dust/context.hpp>

auto go(dust::runtime_t& runtime, dust::context_ptr const& ctx) -> void
{
	auto gd = runtime.geometry_declaration_of({
		{"position", 0, 0, 4}
	});


}


int main()
{

}
