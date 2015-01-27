#include <atma/xtm/bind.hpp>

#include <memory>
#include <functional>


namespace detail
{
	uint const fn_size = 32;
	uint const fn_bufsize = fn_size - sizeof(void*) - sizeof(void*);

	using functor_buf_t = char[fn_bufsize];

	template <typename T>
	struct is_inplace_allowed_t
	{
		static bool const value = sizeof(T) <= fn_bufsize;
	};

	template <typename FN>
	struct base_dispatch_t;

	template <typename R, typename... Params>
	struct base_dispatch_t<R(Params...)>
	{
		virtual auto operator ()(Params&&... args) -> R = 0;
	};

	template <typename FN>
	using base_dispatch_ptr = std::unique_ptr<base_dispatch_t<FN>>;

	template <typename FN>
	struct memfnptr_dispatch_t;

	template <typename R, typename C, typename... Params>
	struct memfnptr_dispatch_t<R (C::*)(Params...)>
		: base_dispatch_t<R(Params...)>
	{
		auto operator ()(C* c, Params&&... args) -> R override
		{
			return (c->*fn_)(std::forward<Params>(args)...);
		}

	private:
		R (C::*fn_)(Params...);
	};

	template <typename R, typename... Params>
	struct fnptr_dispatch_t
		: base_dispatch_t<R(Params...)>
	{
		fnptr_dispatch_t(R (*fn)(Params...))
			: fn_{fn}
		{}

		auto operator ()(Params&&... args) -> R override
		{
			return (*fn_)(std::forward<Params>(args)...);
		}

	private:
		R (*fn_)(Params...);
	};


	template <typename T>
	auto functorize(T&& t) -> T
	{
		return std::forward<T>(t);
	}

	template <typename R, typename C, typename... Params>
	auto functorize(R (C::*fn)(Params...)) -> decltype(std::mem_fn(fn))
	{
		return std::mem_fn(fn);
	}

	template <typename R, typename C, typename... Params>
	auto functorize(R(C::*fn)(Params...) const) -> decltype(std::mem_fn(fn))
	{
		return std::mem_fn(fn);
	}



	template <typename FN, bool = is_inplace_allowed_t<FN>::value>
	struct dispatch_central_t
	{
		static auto store(functor_buf_t& buf, FN const& fn) -> void
		{
			new (&buf) FN(fn);
		}

		static auto destruct(functor_buf_t& fn) -> void
		{
			reinterpret_cast<FN&>(fn).~FN();
		}

		template <typename R, typename... Args>
		static auto call(functor_buf_t& buf, Args... args) -> R
		{
			auto&& fn = reinterpret_cast<FN&>(buf);
			return fn(std::forward<Args>(args)...);
		}
	};

	template <typename FN>
	struct dispatch_central_t<FN, false>
	{
		static auto store(functor_buf_t& buf, FN const& fn) -> void
		{
			reinterpret_cast<FN*&>(buf) = new FN(fn);
		}

		static auto destruct(functor_buf_t& buf) -> void
		{
			delete reinterpret_cast<FN*&>(buf);
		}

		template <typename R, typename... Args>
		static auto call(functor_buf_t& buf, Args... args) -> R
		{
			return reinterpret_cast<FN*&>(buf)->operator ()(std::forward<Args>(args)...);
		}
	};



	struct functor_vtable_t
	{
		void (*const destruct)(detail::functor_buf_t&);
	};

	template <typename FN>
	auto destruct_functor(functor_buf_t& buf) -> void
	{
		dispatch_central_t<FN>::destruct(buf);
	}

	template <typename FN>
	auto generate_vtable() -> functor_vtable_t&
	{
		static functor_vtable_t _ = {&destruct_functor<FN>};
		return _;
	}
}






template <typename> struct fn_t;

template <typename R, typename... Params>
struct fn_t<R(Params...)>
{
	template <typename FN>
	fn_t(FN&& fn)
		: vtable_()
		, dispatch_()
	{
		functor_init(detail::functorize(std::forward<FN>(fn)));
	}

	~fn_t()
	{
		vtable_->destruct(buf_);
	}

	template <typename... Args>
	auto operator ()(Args&&... args) -> R
	{
		return dispatch_(buf_, std::forward<Args>(args)...);
	}

private:
	template <typename FN>
	auto functor_init(FN&& fn) -> void
	{
		vtable_ = &detail::generate_vtable<FN>();
		dispatch_ = &detail::dispatch_central_t<FN>::template call<R, Params...>;
		detail::dispatch_central_t<FN>::store(buf_, std::forward<FN>(fn));
	}

private:
	detail::functor_vtable_t* vtable_;
	R (*dispatch_)(detail::functor_buf_t&, Params...);
	detail::functor_buf_t buf_;
};





int plus(int a, int b) { return a + b; }

struct dragon_t
{
	int plus(int a, int b) { return a + b; }
};

int function_main()
{
	auto f = fn_t<int(int, int)>{&plus};
	auto r = f(3, 4);


	auto d = dragon_t();


	//auto x = atma::xtm::curry(&dragon_t::plus, &d);
	//auto x2 = new decltype(x)(x);
	//auto xr = (*x2)(3, 4);

	std::function<int(int, int)> sf = atma::xtm::curry(&dragon_t::plus, &d);
	//auto r2 = sf(4, 5);

	auto f2 = fn_t<int(int, int)>{atma::xtm::curry(&dragon_t::plus, &d)};
	auto r2 = f2(4, 5);

	auto f3 = fn_t<int(dragon_t&, int, int)>{std::mem_fn(&dragon_t::plus)};
	auto r3 = f3(d, 3, 3);

	return sizeof(atma::xtm::curry(&dragon_t::plus, &d));
}

