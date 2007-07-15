/*
 * typelist.hpp - Copyright (C) 2007 by Nathan Reed
 * Implementation of C++ template type lists and related tools.
 */

/*
 * Type list and definition of nil type list, which is void.
 */

typedef void nil;

template <typename Head, typename Tail = nil>
struct typelist {};

/*
 * Type/value list.
 */

template <typename Typelist>
struct typevallist {};

template <typename Head, typename Tail>
struct typevallist <typelist<Head, Tail> >
{
	Head hd;
	typevallist<Tail> tl;
	typevallist(Head hd_, const typevallist<Tail> &tl_):
		hd(hd_), tl(tl_)
	{}
};

// Specializations of type/value list for head types that are references and
// const-references.  We need to handle these specially since we can't count
// on the referenced object hanging around for the lifetime of the list.

template <typename Head, typename Tail>
struct typevallist <typelist<Head &, Tail> >
{
	Head hd;
	typevallist<Tail> tl;
	typevallist(Head &hd_, const typevallist<Tail> &tl_):
		hd(hd_), tl(tl_)
	{}
};

template <typename Head, typename Tail>
struct typevallist <typelist<const Head &, Tail> >
{
	Head hd;
	typevallist<Tail> tl;
	typevallist(const Head &hd_, const typevallist<Tail> &tl_):
		hd(hd_), tl(tl_)
	{}
};

/*
 * Containers for function pointer types.  We have three kinds of containers:
 * one for global functions, one for non-const member functions, and one for
 * const member functions.  These containers allow the function pointer types
 * to be broken down into their components.
 *
 * Of course, because of the limitations of C++ templates, we can only define
 * this for up to a constant number of function parameters.  We give the
 * definitions for up to 8 parameters here, though this can be easily
 * increased if necessary.
 */

template <typename FnPtr>
struct fnptr {};

/* Ordinary function pointers. */

#define FNPTR_GLOBAL_TRAITS \
	static const bool mfp = false;\
	typedef Ret resulttype

template <typename Ret>
struct fnptr <Ret (*) ()>
{
	FNPTR_GLOBAL_TRAITS;
	typedef nil params;
	static Ret apply (Ret (*fp) (), const typevallist<params> &tvl)
	{
		(void)tvl;
		return fp();
	}
};

template <typename Ret, typename P1>
struct fnptr <Ret (*) (P1)>
{
	FNPTR_GLOBAL_TRAITS;
	typedef typelist<P1> params;
	static Ret apply (Ret (*fp) (P1), const typevallist<params> &tvl)
	{
		return fp(tvl.hd);
	}
};

template <typename Ret, typename P1, typename P2>
struct fnptr <Ret (*) (P1, P2)>
{
	FNPTR_GLOBAL_TRAITS;
	typedef typelist<P1, typelist<P2> > params;
	static Ret apply (Ret (*fp) (P1, P2), const typevallist<params> &tvl)
	{
		return fp(tvl.hd, tvl.tl.hd);
	}
};

template <typename Ret, typename P1, typename P2, typename P3>
struct fnptr <Ret (*) (P1, P2, P3)>
{
	FNPTR_GLOBAL_TRAITS;
	typedef typelist<P1, typelist<P2, typelist<P3> > > params;
	static Ret apply (Ret (*fp) (P1, P2, P3), const typevallist<params> &tvl)
	{
		return fp(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
	}
};

template <typename Ret, typename P1, typename P2, typename P3, typename P4>
struct fnptr <Ret (*) (P1, P2, P3, P4)>
{
	FNPTR_GLOBAL_TRAITS;
	typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4> > > > params;
	static Ret apply (Ret (*fp) (P1, P2, P3, P4),
		const typevallist<params> &tvl)
	{
		return fp(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
	}
};

template <typename Ret, typename P1, typename P2, typename P3, typename P4,
	typename P5>
struct fnptr <Ret (*) (P1, P2, P3, P4, P5)>
{
	FNPTR_GLOBAL_TRAITS;
	typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4,
		typelist<P5> > > > > params;
	static Ret apply (Ret (*fp) (P1, P2, P3, P4, P5),
		const typevallist<params> &tvl)
	{
		return fp(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
			tvl.tl.tl.tl.tl.hd);
	}
};

template <typename Ret, typename P1, typename P2, typename P3, typename P4,
	typename P5, typename P6>
struct fnptr <Ret (*) (P1, P2, P3, P4, P5, P6)>
{
	FNPTR_GLOBAL_TRAITS;
	typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4, typelist<P5, 
		typelist<P6> > > > > > params;
	static Ret apply (Ret (*fp) (P1, P2, P3, P4, P5, P6),
		const typevallist<params> &tvl)
	{
		return fp(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
			tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
	}
};

template <typename Ret, typename P1, typename P2, typename P3, typename P4,
	typename P5, typename P6, typename P7>
struct fnptr <Ret (*) (P1, P2, P3, P4, P5, P6, P7)>
{
	FNPTR_GLOBAL_TRAITS;
	typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4, typelist<P5,
		typelist<P6, typelist<P7> > > > > > > params;
	static Ret apply (Ret (*fp) (P1, P2, P3, P4, P5, P6, P7),
		const typevallist<params> &tvl)
	{
		return fp(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
			tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
			tvl.tl.tl.tl.tl.tl.tl.hd);
	}
};

template <typename Ret, typename P1, typename P2, typename P3, typename P4,
	typename P5, typename P6, typename P7, typename P8>
struct fnptr <Ret (*) (P1, P2, P3, P4, P5, P6, P7, P8)>
{
	FNPTR_GLOBAL_TRAITS;
	typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4, typelist<P5,
		typelist<P6, typelist<P7, typelist<P8> > > > > > > > params;
	static Ret apply (Ret (*fp) (P1, P2, P3, P4, P5, P6, P7, P8),
		const typevallist<params> &tvl)
	{
		return fp(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
			tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
			tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
	}
};

/* Non-const member function pointers. */

#define FNPTR_MFP_TRAITS \
	static const bool mfp = true;\
	static const bool const_mfp = false;\
	typedef T classtype;\
	typedef Ret resulttype

template <typename T, typename Ret>
struct fnptr <Ret (T::*) ()>
{
	FNPTR_MFP_TRAITS;
	typedef nil params;
	static Ret apply (T *obj, Ret (T::*fp) (), const typevallist<params> &tvl)
	{
		(void)tvl;
		return (obj->*fp)();
	}
};

template <typename T, typename Ret, typename P1>
struct fnptr <Ret (T::*) (P1)>
{
	FNPTR_MFP_TRAITS;
	typedef typelist<P1> params;
	static Ret apply (T *obj, Ret (T::*fp) (P1),
		const typevallist<params> &tvl)
	{
		return (obj->*fp)(tvl.hd);
	}
};

template <typename T, typename Ret, typename P1, typename P2>
struct fnptr <Ret (T::*) (P1, P2)>
{
	FNPTR_MFP_TRAITS;
	typedef typelist<P1, typelist<P2> > params;
	static Ret apply (T *obj, Ret (T::*fp) (P1, P2),
		const typevallist<params> &tvl)
	{
		return (obj->*fp)(tvl.hd, tvl.tl.hd);
	}
};

template <typename T, typename Ret, typename P1, typename P2, typename P3>
struct fnptr <Ret (T::*) (P1, P2, P3)>
{
	FNPTR_MFP_TRAITS;
	typedef typelist<P1, typelist<P2, typelist<P3> > > params;
	static Ret apply (T *obj, Ret (T::*fp) (P1, P2, P3),
		const typevallist<params> &tvl)
	{
		return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
	}
};

template <typename T, typename Ret, typename P1, typename P2, typename P3,
	typename P4>
struct fnptr <Ret (T::*) (P1, P2, P3, P4)>
{
	FNPTR_MFP_TRAITS;
	typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4> > > > params;
	static Ret apply (T *obj, Ret (T::*fp) (P1, P2, P3, P4),
		const typevallist<params> &tvl)
	{
		return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
	}
};

template <typename T, typename Ret, typename P1, typename P2, typename P3,
	typename P4, typename P5>
struct fnptr <Ret (T::*) (P1, P2, P3, P4, P5)>
{
	FNPTR_MFP_TRAITS;
	typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4,
		typelist<P5> > > > > params;
	static Ret apply (T *obj, Ret (T::*fp) (P1, P2, P3, P4, P5),
		const typevallist<params> &tvl)
	{
		return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
			tvl.tl.tl.tl.tl.hd);
	}
};

template <typename T, typename Ret, typename P1, typename P2, typename P3,
	typename P4, typename P5, typename P6>
struct fnptr <Ret (T::*) (P1, P2, P3, P4, P5, P6)>
{
	FNPTR_MFP_TRAITS;
	typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4, typelist<P5,
		typelist<P6> > > > > > params;
	static Ret apply (T *obj, Ret (T::*fp) (P1, P2, P3, P4, P5, P6),
		const typevallist<params> &tvl)
	{
		return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
			tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
	}
};

template <typename T, typename Ret, typename P1, typename P2, typename P3,
	typename P4, typename P5, typename P6, typename P7>
struct fnptr <Ret (T::*) (P1, P2, P3, P4, P5, P6, P7)>
{
	FNPTR_MFP_TRAITS;
	typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4, typelist<P5,
		typelist<P6, typelist<P7> > > > > > > params;
	static Ret apply (T *obj, Ret (T::*fp) (P1, P2, P3, P4, P5, P6, P7),
		const typevallist<params> &tvl)
	{
		return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
			tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
			tvl.tl.tl.tl.tl.tl.tl.hd);
	}
};

template <typename T, typename Ret, typename P1, typename P2, typename P3,
	typename P4, typename P5, typename P6, typename P7, typename P8>
struct fnptr <Ret (T::*) (P1, P2, P3, P4, P5, P6, P7, P8)>
{
	FNPTR_MFP_TRAITS;
	typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4, typelist<P5,
		typelist<P6, typelist<P7, typelist <P8> > > > > > > > params;
	static Ret apply (T *obj, Ret (T::*fp) (P1, P2, P3, P4, P5, P6, P7, P8),
		const typevallist<params> &tvl)
	{
		return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
			tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
			tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
	}
};

/* Const member function pointers. */

#define FNPTR_CONST_MFP_TRAITS \
	static const bool mfp = true;\
	static const bool const_mfp = true;\
	typedef T classtype;\
	typedef Ret resulttype

template <typename T, typename Ret>
struct fnptr <Ret (T::*) () const>
{
	FNPTR_CONST_MFP_TRAITS;
	typedef nil params;
	static Ret apply (const T *obj, Ret (T::*fp) () const,
		const typevallist<params> &tvl)
	{
		(void)tvl;
		return (obj->*fp)();
	}
};

template <typename T, typename Ret, typename P1>
struct fnptr <Ret (T::*) (P1) const>
{
	FNPTR_CONST_MFP_TRAITS;
	typedef typelist<P1> params;
	static Ret apply (const T *obj, Ret (T::*fp) (P1) const,
		const typevallist<params> &tvl)
	{
		return (obj->*fp)(tvl.hd);
	}
};

template <typename T, typename Ret, typename P1, typename P2>
struct fnptr <Ret (T::*) (P1, P2) const>
{
	FNPTR_CONST_MFP_TRAITS;
	typedef typelist<P1, typelist<P2> > params;
	static Ret apply (const T *obj, Ret (T::*fp) (P1, P2) const,
		const typevallist<params> &tvl)
	{
		return (obj->*fp)(tvl.hd, tvl.tl.hd);
	}
};

template <typename T, typename Ret, typename P1, typename P2, typename P3>
struct fnptr <Ret (T::*) (P1, P2, P3) const>
{
	FNPTR_CONST_MFP_TRAITS;
	typedef typelist<P1, typelist<P2, typelist<P3> > > params;
	static Ret apply (const T *obj, Ret (T::*fp) (P1, P2, P3) const,
		const typevallist<params> &tvl)
	{
		return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
	}
};

template <typename T, typename Ret, typename P1, typename P2, typename P3,
	typename P4>
struct fnptr <Ret (T::*) (P1, P2, P3, P4) const>
{
	FNPTR_CONST_MFP_TRAITS;
	typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4> > > > params;
	static Ret apply (const T *obj, Ret (T::*fp) (P1, P2, P3, P4) const,
		const typevallist<params> &tvl)
	{
		return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
	}
};

template <typename T, typename Ret, typename P1, typename P2, typename P3,
	typename P4, typename P5>
struct fnptr <Ret (T::*) (P1, P2, P3, P4, P5) const>
{
	FNPTR_CONST_MFP_TRAITS;
	typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4,
		typelist<P5> > > > > params;
	static Ret apply (const T *obj, Ret (T::*fp) (P1, P2, P3, P4, P5) const,
		const typevallist<params> &tvl)
	{
		return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
			tvl.tl.tl.tl.tl.hd);
	}
};

template <typename T, typename Ret, typename P1, typename P2, typename P3,
	typename P4, typename P5, typename P6>
struct fnptr <Ret (T::*) (P1, P2, P3, P4, P5, P6) const>
{
	FNPTR_CONST_MFP_TRAITS;
	typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4, typelist<P5,
		typelist<P6> > > > > > params;
	static Ret apply (const T *obj,
		Ret (T::*fp) (P1, P2, P3, P4, P5, P6) const,
		const typevallist<params> &tvl)
	{
		return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
			tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
	}
};

template <typename T, typename Ret, typename P1, typename P2, typename P3,
	typename P4, typename P5, typename P6, typename P7>
struct fnptr <Ret (T::*) (P1, P2, P3, P4, P5, P6, P7) const>
{
	FNPTR_CONST_MFP_TRAITS;
	typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4, typelist<P5,
		typelist<P6, typelist<P7> > > > > > > params;
	static Ret apply (const T *obj,
		Ret (T::*fp) (P1, P2, P3, P4, P5, P6, P7) const,
		const typevallist<params> &tvl)
	{
		return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
			tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
			tvl.tl.tl.tl.tl.tl.tl.hd);
	}
};

template <typename T, typename Ret, typename P1, typename P2, typename P3,
	typename P4, typename P5, typename P6, typename P7, typename P8>
struct fnptr <Ret (T::*) (P1, P2, P3, P4, P5, P6, P7, P8) const>
{
	FNPTR_CONST_MFP_TRAITS;
	typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4, typelist<P5,
		typelist<P6, typelist<P7, typelist<P8> > > > > > > > params;
	static Ret apply (const T *obj,
		Ret (T::*fp) (P1, P2, P3, P4, P5, P6, P7, P8) const,
		const typevallist<params> &tvl)
	{
		return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
			tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
			tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
	}
};

/*
 * Constructor generators.  These templates allow you to call operator new and
 * pass the contents of a type/value list to the constructor.  Like the
 * function pointer containers, these are only defined up to 8 parameters.
 */

template <typename T, typename Typelist>
struct constructor {};

template <typename T>
struct constructor <T, nil>
{
	static T* apply (const typevallist<nil> &tvl)
	{
		(void)tvl;
		return new T;
	}
};

template <typename T, typename P1>
struct constructor <T, typelist<P1> >
{
	static T* apply (const typevallist<typelist<P1> > &tvl)
	{
		return new T(tvl.hd);
	}
};

template <typename T, typename P1, typename P2>
struct constructor <T, typelist<P1, typelist<P2> > >
{
	static T* apply (const typevallist<typelist<P1, typelist<P2> > > &tvl)
	{
		return new T(tvl.hd, tvl.tl.hd);
	}
};

template <typename T, typename P1, typename P2, typename P3>
struct constructor <T, typelist<P1, typelist<P2, typelist<P3> > > >
{
	static T* apply (const typevallist<typelist<P1, typelist<P2,
		typelist<P3> > > > &tvl)
	{
		return new T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
	}
};

template <typename T, typename P1, typename P2, typename P3, typename P4>
struct constructor <T, typelist<P1, typelist<P2, typelist<P3,
	typelist<P4> > > > >
{
	static T* apply (const typevallist<typelist<P1, typelist<P2,
		typelist<P3, typelist<P4> > > > > &tvl)
	{
		return new T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
	}
};

template <typename T, typename P1, typename P2, typename P3, typename P4,
	typename P5>
struct constructor <T, typelist<P1, typelist<P2, typelist<P3,
	typelist<P4, typelist<P5> > > > > >
{
	static T* apply (const typevallist<typelist<P1, typelist<P2,
		typelist<P3, typelist<P4, typelist<P5> > > > > > &tvl)
	{
		return new T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
			tvl.tl.tl.tl.tl.hd);
	}
};

template <typename T, typename P1, typename P2, typename P3, typename P4,
	typename P5, typename P6>
struct constructor <T, typelist<P1, typelist<P2, typelist<P3,
	typelist<P4, typelist<P5, typelist<P6> > > > > > >
{
	static T* apply (const typevallist<typelist<P1, typelist<P2,
		typelist<P3, typelist<P4, typelist<P5, typelist<P6> > > > > > > &tvl)
	{
		return new T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
			tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
	}
};

template <typename T, typename P1, typename P2, typename P3, typename P4,
	typename P5, typename P6, typename P7>
struct constructor <T, typelist<P1, typelist<P2, typelist<P3,
	typelist<P4, typelist<P5, typelist<P6, typelist<P7> > > > > > > >
{
	static T* apply (const typevallist<typelist<P1, typelist<P2,
		typelist<P3, typelist<P4, typelist<P5, typelist<P6,
		typelist<P7> > > > > > > > &tvl)
	{
		return new T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
			tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
			tvl.tl.tl.tl.tl.tl.tl.hd);
	}
};

template <typename T, typename P1, typename P2, typename P3, typename P4,
	typename P5, typename P6, typename P7, typename P8>
struct constructor <T, typelist<P1, typelist<P2, typelist<P3,
	typelist<P4, typelist<P5, typelist<P6, typelist<P7, 
	typelist<P8> > > > > > > > >
{
	static T* apply (const typevallist<typelist<P1, typelist<P2,
		typelist<P3, typelist<P4, typelist<P5, typelist<P6,
		typelist<P7, typelist<P8> > > > > > > > > &tvl)
	{
		return new T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
			tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
			tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
	}
};
