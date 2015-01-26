#include <atma/string.hpp>
#include <atma/intrusive_ptr.hpp>
#include <atma/math/vector4f.hpp>
#include <atma/algorithm.hpp>
#include <atma/enable_if.hpp>
#include <atma/xtm/bind.hpp>

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

#include <array>

template <typename T>
struct default_octree_subdivider_t
{
	auto subdivide(uint depth, atma::math::vector4f const& bounds, T const& x) -> void
	{
		
	}
};

struct octree_t
{
	octree_t() {}

	//auto add_triangle(T const&, math::vector4f const&, math::vector4f const&, math::vector4f const&) -> void;

	auto insert_point(math::vector4f const&) -> bool;
	struct node_t;
	//node_t root_;

private:

private:

private:
	
};

struct triangle_t
{
	math::vector4f v0, v1, v2;

	auto edge0() const -> math::vector4f { return v1 - v0; }
	auto edge1() const -> math::vector4f { return v2 - v1; }
	auto edge2() const -> math::vector4f { return v0 - v2; }
};

struct box_t
{
	math::vector4f origin;
	math::vector4f half_extents;

	static auto from_minmax(math::vector4f const& min, math::vector4f const& max) -> box_t
	{
		return box_t{
			(min + max) / 2.f,
			(max - min) / 2.f};
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


auto project(math::vector4f const& axis, math::vector4f const* points, size_t point_count, float& min, float& max) -> void
{
	min = FLT_MAX;
	max = -FLT_MAX;

	for (auto p = points; p != points + point_count; ++p)
	{
		float v = math::dot_product(axis, *p);
		if (v < min) min = v;
		if (v > max) max = v;
	}
}


auto intersect_aabb_triangle(math::vector4f const& aabb, triangle_t const& tri) -> bool
{
	float box_min = 0.f, box_max = 0.f;
	float tri_min = 0.f, tri_max = 0.f;
	
	math::vector4f const tri_edges[3] = {
		tri.edge0(),
		tri.edge1(),
		tri.edge2()
	};

	math::vector4f const box_verts[] = {
		{aabb.x - aabb.w, aabb.y - aabb.w, aabb.z - aabb.w, 1.f},
		{aabb.x + aabb.w, aabb.y - aabb.w, aabb.z - aabb.w, 1.f},
		{aabb.x - aabb.w, aabb.y + aabb.w, aabb.z - aabb.w, 1.f},
		{aabb.x + aabb.w, aabb.y + aabb.w, aabb.z - aabb.w, 1.f},
		{aabb.x - aabb.w, aabb.y - aabb.w, aabb.z + aabb.w, 1.f},
		{aabb.x + aabb.w, aabb.y - aabb.w, aabb.z + aabb.w, 1.f},
		{aabb.x - aabb.w, aabb.y + aabb.w, aabb.z + aabb.w, 1.f},
		{aabb.x + aabb.w, aabb.y + aabb.w, aabb.z + aabb.w, 1.f},
	};

	math::vector4f const box_normals[3] = {
		math::vector4f{1.f, 0.f, 0.f, 1.f},
		math::vector4f{0.f, 1.f, 0.f, 1.f},
		math::vector4f{0.f, 0.f, 1.f, 1.f},
	};


	// test box normals
	{
		for (int i = 0; i != 3; ++i)
		{
			project(box_normals[i], &tri.v0, 3, tri_min, tri_max);
			if (tri_max < aabb.components[i] - aabb.w || tri_min > aabb.components[i] + aabb.w)
				return false;
		}
	}

	// test the triangle normal
	{
		auto tri_normal = math::cross_product(tri.v1 - tri.v0, tri.v2 - tri.v0);
		auto tri_offset = math::dot_product(tri_normal, tri.v0);

		project(tri_normal, box_verts, 8, box_min, box_max);

		if (box_max < tri_offset || box_min > tri_offset)
			return false;
	}


	// test nine-edges
	for (int i = 0; i != 3; ++i)
	{
		for (int j = 0; j != 3; ++j)
		{
			auto axis = math::cross_product(tri_edges[i], box_normals[j]);
			project(axis, box_verts, 8, box_min, box_max);
			project(axis, &tri.v0, 3, tri_min, tri_max);
			if (box_max <= tri_min || box_min >= tri_max)
				return false;
		}
	}


	return true;
}





#if 0



bool IsIntersecting(IAABox box, ITriangle triangle)
{
	double triangleMin, triangleMax;
	double boxMin, boxMax;

	// Test the box normals (x-, y- and z-axes)
	var boxNormals = new IVector[] {
		new Vector(1,0,0),
			new Vector(0,1,0),
			new Vector(0,0,1)
	};
	for (int i = 0; i < 3; i++)
	{
		IVector n = boxNormals[i];
		Project(triangle.Vertices, boxNormals[i], out triangleMin, out triangleMax);
		if (triangleMax < box.Start.Coords[i] || triangleMin > box.End.Coords[i])
			return false; // No intersection possible.
	}

	// Test the triangle normal
	double triangleOffset = triangle.Normal.Dot(triangle.A);
	Project(box.Vertices, triangle.Normal, out boxMin, out boxMax);
	if (boxMax < triangleOffset || boxMin > triangleOffset)
		return false; // No intersection possible.

	// Test the nine edge cross-products
	IVector[] triangleEdges = new IVector[] {
		triangle.A.Minus(triangle.B),
			triangle.B.Minus(triangle.C),
			triangle.C.Minus(triangle.A)
	};
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
		{
		// The box normals are the same as it's edge tangents
		IVector axis = triangleEdges[i].Cross(boxNormals[j]);
		Project(box.Vertices, axis, out boxMin, out boxMax);
		Project(triangle.Vertices, axis, out triangleMin, out triangleMax);
		if (boxMax <= triangleMin || boxMin >= triangleMax)
			return false; // No intersection possible
		}

	// No separating axis found.
	return true;
}

void Project(IEnumerable<IVector> points, IVector axis,
	out double min, out double max)
{
	double min = double.PositiveInfinity;
	double max = double.NegativeInfinity;
	foreach (var p in points)
	{
		double val = axis.Dot(p);
		if (val < min) min = val;
		if (val > max) max = val;
	}
}

interface IVector
{
	double X { get; }
	double Y { get; }
	double Z { get; }
	double[] Coords { get; }
	double Dot(IVector other);
	IVector Minus(IVector other);
	IVector Cross(IVector other);
}

interface IShape
{
	IEnumerable<IVector> Vertices { get; }
}

interface IAABox : IShape
{
	IVector Start { get; }
	IVector End { get; }
}

interface ITriangle : IShape {
	IVector Normal { get; }
	IVector A { get; }
	IVector B { get; }
	IVector C { get; }
}


#endif

















#if 0

struct octree_t::node_t
{
	node_t()
		: bounds(0.f, 0.f, 0.f, 0.5f)
		, children_()
		, datas_()
	{
	}

	node_t(math::vector4f const& bounds)
		: bounds(bounds)
		, children_()
		, datas_()
	{
	}

	atma::math::vector4f bounds;
	triangle_t data;

	auto inbounds(math::vector4f const& point) -> bool;
	auto insert(math::vector4f const& point, T const& data) -> bool;

private:
	auto imem_allocate() -> void
	{
		children_ = (node_t*)new char[8 * sizeof(node_t)];
	}

	auto imem_deallocate() -> void;


	auto oct_split() -> void
	{
		children_ = (node_t*)new char[8 * sizeof(node_t)];

		for (auto i = 0u; i != 8u; ++i)
			new (children_ + i) node_t( oct_subbound(bounds, i) );

		for (auto i = 0u; i != 8u; ++i)
			insert(dloc_[i], data_[i]);
	}

	auto oct_subbound(math::vector4f const&, uint) -> math::vector4f;

private:
	node_t* children_;

	std::array<T, 8> data_;
	std::array<math::vector4f, 8> dloc_;

	uint datas_;
};


auto octree_t::node_t::insert(triangle_t const& tri) -> bool
{
	
	if (!inbounds(point))
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
template<typename testType>
struct is_function_pointer
{
	static const bool value =
		std::is_pointer<testType>::value ?
		std::is_function<typename std::remove_pointer<testType>::type>::value :
		false;
};

int plus2(int x) { return x + 2; }

auto const plus2fn = std::make_tuple(&plus2);

template <typename F>
struct function_t
{
	function_t(F fn) : fn_(fn) {}

	template <typename... Curried>
	auto operator ()(Curried... curried)
	-> typename std::enable_if<atma::xtm::variadic_size<Curried...>::value == atma::xtm::function_traits<F>::arity, int>::type
	{
		return fn_(std::forward<Curried>(curried)...);
	}

	template <typename... Curried>
	auto operator ()(Curried... curried)
	-> typename std::enable_if<std::tuple_size<std::tuple<Curried...>>::value != atma::xtm::function_traits<F>::arity,
		function_t<decltype(atma::xtm::curry(fn_, std::forward<Curried>(curried)...))>>::type
	{
		return {atma::xtm::curry(fn_, std::forward<Curried>(curried)...)};
	}

	auto fn() const -> F { return fn_; }

private:
	F fn_;
};

template <typename R, typename... Args>
auto functionize(R(&fn)(Args...)) -> function_t<R(*)(Args...)>
{
	return {&fn};
}

template <typename F, typename G, typename X>
inline auto point(F f, G g, X x) -> typename atma::xtm::function_traits<F>::result_type
{
	return f(g(std::forward<X>(x)));
}

template <typename F, typename G>
struct curried_function_traits
{
	using result_type = typename atma::xtm::function_traits<F>::result_type;

	//using signature = typename atma::xtm::function_traits<F>::result_type(*)(typename atma::xtm::function_traits<G>::template arg<0>::type);

	using sg = typename atma::xtm::function_traits<F>::result_type(*)(function_t<F>, function_t<G>, typename atma::xtm::function_traits<G>::template arg<0>::type);
};



namespace atma { namespace xtm {

	template <typename F>
	struct function_traits<function_t<F>>
		: function_traits<F>
	{};
	
#if 0
	template <typename F, typename Bindings>
	struct function_traits<bind_t<F, Bindings>>
	{
		using result_type = typename function_traits<F>::result_type;
		
		enum { arity = function_traits<F>::arity - tuple_nonplaceholder_size_t<Bindings>::value };

		template <size_t i>
		struct arg {
			typedef typename function_traits<F>::template arg<i + tuple_nonplaceholder_size_t<Bindings>::value>::type type;
		};
	};
#endif
}}


#if 1
template <typename F, typename G>
inline auto operator * (function_t<F> f, function_t<G> g)
-> function_t<
	atma::xtm::bind_t<
		typename curried_function_traits<F, G>::sg,
		std::tuple<function_t<F>, function_t<G>, atma::xtm::placeholder_t<0>>
	>>
{
	static_assert(atma::xtm::function_traits<F>::arity == 1, "can not compose functions of arity greater than 1");
	static_assert(atma::xtm::function_traits<G>::arity == 1, "can not compose functions of arity greater than 1");

	auto fnptr = &point<
		function_t<F>,
		function_t<G>,
		typename atma::xtm::function_traits<G>::template arg<0>::type>;

	return {atma::xtm::curry(std::forward<decltype(fnptr)>(fnptr), std::forward<function_t<F>>(f), std::forward<function_t<G>>(g))};
}
#endif


int mul(int x, int y) { return x * y; }
int add(int x, int y) { return x + y; }

int mul2(int x) { return x * 2; }
int add4(int x) { return x + 4; }

struct giraffe
{
	int add(int x, int y) { return x + y; }
};

template <typename, typename, typename> struct Y;

template <size_t... A, size_t... B, typename Tuple>
struct Y<atma::xtm::idxs_t<A...>, atma::xtm::idxs_t<B...>, Tuple>
{ using type = std::tuple<typename std::tuple_element<A, typename std::tuple_element<B, Tuple>::type>::type...>; };







#if 0
template <typename FN>
struct base;

template <typename R, typename... Args>
struct base<R(Args...)>
{
	virtual auto operator ()(Args&&... args) -> R = 0;
};

template <typename FN>
struct member_function;

template <typename R, typename Args...>
struct member_function : base<R(Args...)>
{
	auto operator ()(Args&&... args) -> R override
	{
	}

private:
	R(*fn_)(Args...);
};

#endif



template <typename R, typename... Args>
auto mfn_dispatch(void* fn, void*, Args&&... args) -> R
{
	R(*fn2)(Args...) = fn;
	return (*fn2)(std::forward<Args>(args)...);
}





template <typename FN>
struct fna;

template <typename R, typename... Args>
struct fna<R (Args...)> 
{
	static auto apply(R (*fn)(Args...), Args&&... args) -> R
	{
		return (*fn)(std::forward<Args>(args)...);
	}
};

template <typename FN>
struct fn_t
{
	//template <typename R, typename C, typename... Args>
	//function_t(R (C::*fn)(Args...), C* c = nullptr)
	//	: instance_{c}, fn_{fn}
	//{}

	template <typename R, typename... Args>
	fn_t(R(*fn)(Args...))
		: instance_{}
		, dispatch_{&mfn_dispatch<R, Args...>}
	{}

	template <typename... Args>
	auto operator ()(Args&&... args) -> typename atma::xtm::function_traits<std::decay_t<FN>>::result_type
	{
		return fn_()
	}

private:
	void* instance_;
	void* fn_;
	FN* dispatch_;
};



struct dragon_t
{
	int age() { return 4; }
};

int four() { return 4; }

int main()
{
	auto f = fn_t<int()>{&four};
	//auto f2 = function_t<int()>{};

	auto numbers = std::vector<int>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	auto gt_2 =   [](int x) { return x > 2; };

	auto is_even = [](int x) { return x % 2 == 0; };
	auto is_odd = [](int x) { return x % 2 != 0; };
	auto filtered = atma::filter(is_even, numbers);
	auto numbers2 = std::vector<int>(filtered.begin(), filtered.end());
	auto plus_1 = [](int x) { return x + 1; };
	auto mapped = atma::map(plus_1, filtered);
	auto numbers3 = std::vector<int>(mapped.begin(), mapped.end());

	//auto fn_add = function_t<int (*)(int, int)>{&add};
	auto fn_add = functionize(add);
	auto fn_mul2 = functionize(mul2);
	auto am = fn_mul2 * fn_add(4) * fn_add(1);
	auto r = am(3);

	auto thing = atma::filter(is_even) <<= atma::map(plus_1) <<= atma::filter(is_odd) <<= numbers;
	auto thing2 = std::vector<int>(thing.begin(), thing.end());

	using T1 = std::tuple<int, char, double, float>;
	using T2 = std::tuple<long, short, uint>;

	auto tt = atma::xtm::tuple_push_back(T2(), int(4));
	auto tt2 = atma::xtm::tuple_cat(T2(), T1());

	{
		int f = 5;
		auto args = atma::xtm::bind_arguments(std::make_tuple(arg2, arg1), std::forward_as_tuple(f,4));
		//auto bngs = atma::xtm::bind(&add, f, arg1)(4, 8);
		giraffe g;
		auto cngs = atma::xtm::call_fn_bound_tuple(&giraffe::add, std::make_tuple(&g, arg2, arg1), std::make_tuple(2, 3));
	}

	using T3 = Y<atma::xtm::idxs_t<0, 1, 2>, atma::xtm::idxs_t<0, 0, 1>, std::tuple<T1, T2>>::type;
	auto t3i = T3();
}
