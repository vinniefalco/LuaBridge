/*
 * typelist.hpp - Copyright (C) 2007 by Nathan Reed
 * Implementation of C++ template type lists and related tools.
 */

/*
 * Things we need to do with typelists:
 *   1. Construct a run-time data structure with a value for each type in a
          type list (a type/value list).
 *   2. Iterate over a type list so as to fill a type/value list.
 *   3. Convert a type/value list to a call of a given function pointer.
 *   4. Convert a type/value list to a call of a given member function
          pointer on a given object.
 *   5. Convert a type/value list to a constructor call of a given type.
 *   6. Convert a function pointer type into a list of parameter types.
 */

/*
 * Type list and definition of nil type list, which is void.
 */

typedef void nil;

template <typename Head, typename Tail = nil>
struct typelist {};

/*
 * Iteration over a type list.  This is a bit subtle as it allows for both
 * type dispatch and runtime dispatch.  We take a template Op, and for each
 * type T in the typelist, an object of Op<T> is constructed and the
 * constructor is passed an object of type Init (if one is given).  This
 * allows for runtime information to be communicated to the Op<T> object.
 * Then, the Op<T> is called using the () operator.  The whole process is
 * started by passing the desired Init object to the execute() method.
 */

/*template <typename Typelist, template <typename> class Op>
struct for_each_type {};

// Base case: for a nil typelist

template <template <typename> class Op>
struct for_each_type <nil, Op>
{
	template <typename Init>
	static void execute (Init &i) {}

	static void execute () {}
};

// Recursive case: for a non-nil typelist

template <typename Head, typename Tail, template <typename> class Op>
struct for_each_type <typelist<Head, Tail>, Op>
{
	template <typename Init>
	static void execute (Init &i)
	{
		Op<Head> op(i);
		op();
		for_each_type<Tail, Op>::execute(i);
	}

	static void execute ()
	{
		Op<Head> op;
		op();
		for_each_type<Tail, Op>::execute();
	}
};*/

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
 * Iteration over a type value list.  This works basically the same way as
 * the iteration over the type list, but an additional parameter is passed to
 * execute(), which is the type value list.  For each type, a reference to the
 * corresponding element is passed to Op.
 */

/*template <typename TypeValList, template <typename> class Op>
struct for_each_value {};

// Base case: for a nil typelist

template <template <typename> class Op>
struct for_each_value <typevallist<nil>, Op>
{
	template <typename Init>
	static void execute (Init &i, typevallist<nil> &tvl) {}

	static void execute (typevallist<nil> &tvl) {}
};

// Recursive case: for a non-nil typelist

template <typename Head, typename Tail, template <typename> class Op>
struct for_each_value <typevallist<typelist<Head, Tail> >, Op>
{
	template <typename Init>
	static void execute (Init &i, typevallist<typelist<Head, Tail> > &tvl)
	{
		Op<Head> op(i);
		op(tvl.hd);
		for_each_value<typevallist<Tail>, Op>::execute(i, tvl.tl);
	}

	static void execute (typevallist<typelist<Head, Tail> > &tvl)
	{
		Op<Head> op;
		op(tvl.hd);
		for_each_value<typevallist<Tail>, Op>::execute(tvl.tl);
	}
};*/

/*
 * Containers for function pointer types.  We have three kinds of containers:
 * one for global functions, one for non-const member functions, and one for
 * const member functions.  These containers allow the function pointer types
 * to be broken down into their components.
 *
 * Of course, because of the limitations of C++ templates, we can only define
 * this for up to a constant number of function parameters.  We give the
 * definitions for up to 5 parameters here, though this can be easily
 * increased if necessary.
 */

template <typename FnPtr>
struct fnptr {};

/* Ordinary function pointers. */

#define FNPTR_GLOBAL_TRAITS() \
	static const bool mfp = false;\
	typedef Ret resulttype

template <typename Ret>
struct fnptr <Ret (*) ()>
{
	FNPTR_GLOBAL_TRAITS();
	typedef nil params;
	static Ret apply (Ret (*fp) (), typevallist<params> &tvl)
	{
		tvl;
		return fp();
	}
};

template <typename Ret, typename P1>
struct fnptr <Ret (*) (P1)>
{
	FNPTR_GLOBAL_TRAITS();
	typedef typelist<P1> params;
	static Ret apply (Ret (*fp) (P1), typevallist<params> &tvl)
	{
		return fp(tvl.hd);
	}
};

template <typename Ret, typename P1, typename P2>
struct fnptr <Ret (*) (P1, P2)>
{
	FNPTR_GLOBAL_TRAITS();
	typedef typelist<P1, typelist<P2> > params;
	static Ret apply (Ret (*fp) (P1, P2), typevallist<params> &tvl)
	{
		return fp(tvl.hd, tvl.tl.hd);
	}
};

template <typename Ret, typename P1, typename P2, typename P3>
struct fnptr <Ret (*) (P1, P2, P3)>
{
	FNPTR_GLOBAL_TRAITS();
	typedef typelist<P1, typelist<P2, typelist<P3> > > params;
	static Ret apply (Ret (*fp) (P1, P2, P3), typevallist<params> &tvl)
	{
		return fp(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
	}
};

template <typename Ret, typename P1, typename P2, typename P3, typename P4>
struct fnptr <Ret (*) (P1, P2, P3, P4)>
{
	FNPTR_GLOBAL_TRAITS();
	typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4> > > > params;
	static Ret apply (Ret (*fp) (P1, P2, P3, P4), typevallist<params> &tvl)
	{
		return fp(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
	}
};

template <typename Ret, typename P1, typename P2, typename P3, typename P4,
	typename P5>
struct fnptr <Ret (*) (P1, P2, P3, P4, P5)>
{
	FNPTR_GLOBAL_TRAITS();
	typedef typelist<P1, typelist<P2, typelist<P3,
		typelist<P4, typelist<P5> > > > > params;
	static Ret apply (Ret (*fp) (P1, P2, P3, P4, P5),
		typevallist<params> &tvl)
	{
		return fp(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
			tvl.tl.tl.tl.tl.hd);
	}
};

/* Non-const member function pointers. */

#define FNPTR_MFP_TRAITS() \
	static const bool mfp = true;\
	static const bool const_mfp = false;\
	typedef T classtype;\
	typedef Ret resulttype

template <typename T, typename Ret>
struct fnptr <Ret (T::*) ()>
{
	FNPTR_MFP_TRAITS();
	typedef nil params;
	static Ret apply (T *obj, Ret (T::*fp) (), typevallist<params> &tvl)
	{
		tvl;
		return obj->fp();
	}
};

template <typename T, typename Ret, typename P1>
struct fnptr <Ret (T::*) (P1)>
{
	FNPTR_MFP_TRAITS();
	typedef typelist<P1> params;
	static Ret apply (T *obj, Ret (T::*fp) (P1), typevallist<params> &tvl)
	{
		return obj->fp(tvl.hd);
	}
};

template <typename T, typename Ret, typename P1, typename P2>
struct fnptr <Ret (T::*) (P1, P2)>
{
	FNPTR_MFP_TRAITS();
	typedef typelist<P1, typelist<P2> > params;
	static Ret apply (T *obj, Ret (T::*fp) (P1, P2), typevallist<params> &tvl)
	{
		return obj->fp(tvl.hd, tvl.tl.hd);
	}
};

template <typename T, typename Ret, typename P1, typename P2, typename P3>
struct fnptr <Ret (T::*) (P1, P2, P3)>
{
	FNPTR_MFP_TRAITS();
	typedef typelist<P1, typelist<P2, typelist<P3> > > params;
	static Ret apply (T *obj, Ret (T::*fp) (P1, P2, P3),
		typevallist<params> &tvl)
	{
		return obj->fp(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
	}
};

template <typename T, typename Ret, typename P1, typename P2, typename P3,
	typename P4>
struct fnptr <Ret (T::*) (P1, P2, P3, P4)>
{
	FNPTR_MFP_TRAITS();
	typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4> > > > params;
	static Ret apply (T *obj, Ret (T::*fp) (P1, P2, P3, P4),
		typevallist<params> &tvl)
	{
		return obj->fp(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
	}
};

template <typename T, typename Ret, typename P1, typename P2, typename P3,
	typename P4, typename P5>
struct fnptr <Ret (T::*) (P1, P2, P3, P4, P5)>
{
	FNPTR_MFP_TRAITS();
	typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4,
		typelist<P5> > > > > params;
	static Ret apply (T *obj, Ret (T::*fp) (P1, P2, P3, P4, P5),
		typevallist<params> &tvl)
	{
		return obj->fp(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
			tvl.tl.tl.tl.tl.hd);
	}
};

/* Const member function pointers. */

#define FNPTR_CONST_MFP_TRAITS() \
	static const bool mfp = true;\
	static const bool const_mfp = true;\
	typedef T classtype;\
	typedef Ret ret

template <typename T, typename Ret>
struct fnptr <Ret (T::*) () const>
{
	FNPTR_CONST_MFP_TRAITS();
	typedef nil params;
	static Ret apply (const T *obj, Ret (T::*fp) () const,
		typevallist<params> &tvl)
	{
		tvl;
		return obj->fp();
	}
};

template <typename T, typename Ret, typename P1>
struct fnptr <Ret (T::*) (P1) const>
{
	FNPTR_CONST_MFP_TRAITS();
	typedef typelist<P1> params;
	static Ret apply (const T *obj, Ret (T::*fp) (P1) const,
		typevallist<params> &tvl)
	{
		return obj->fp(tvl.hd);
	}
};

template <typename T, typename Ret, typename P1, typename P2>
struct fnptr <Ret (T::*) (P1, P2) const>
{
	FNPTR_CONST_MFP_TRAITS();
	typedef typelist<P1, typelist<P2> > params;
	static Ret apply (const T *obj, Ret (T::*fp) (P1, P2) const,
		typevallist<params> &tvl)
	{
		return obj->fp(tvl.hd, tvl.tl.hd);
	}
};

template <typename T, typename Ret, typename P1, typename P2, typename P3>
struct fnptr <Ret (T::*) (P1, P2, P3) const>
{
	FNPTR_CONST_MFP_TRAITS();
	typedef typelist<P1, typelist<P2, typelist<P3> > > params;
	static Ret apply (const T *obj, Ret (T::*fp) (P1, P2, P3) const,
		typevallist<params> &tvl)
	{
		return obj->fp(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
	}
};

template <typename T, typename Ret, typename P1, typename P2, typename P3,
	typename P4>
struct fnptr <Ret (T::*) (P1, P2, P3, P4) const>
{
	FNPTR_CONST_MFP_TRAITS();
	typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4> > > > params;
	static Ret apply (const T *obj, Ret (T::*fp) (P1, P2, P3, P4) const,
		typevallist<params> &tvl)
	{
		return obj->fp(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
	}
};

template <typename T, typename Ret, typename P1, typename P2, typename P3,
	typename P4, typename P5>
struct fnptr <Ret (T::*) (P1, P2, P3, P4, P5) const>
{
	FNPTR_CONST_MFP_TRAITS();
	typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4,
		typelist<P5> > > > > params;
	static Ret apply (const T *obj, Ret (T::*fp) (P1, P2, P3, P4, P5) const,
		typevallist<params> &tvl)
	{
		return obj->fp(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
			tvl.tl.tl.tl.tl.hd);
	}
};
