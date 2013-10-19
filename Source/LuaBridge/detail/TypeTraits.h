//------------------------------------------------------------------------------
/*
  https://github.com/vinniefalco/LuaBridge

  Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>

  License: The MIT License (http://www.opensource.org/licenses/mit-license.php)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
//==============================================================================

#ifndef LUABRIDGE_TYPEINFO_HEADER
#define LUABRIDGE_TYPEINFO_HEADER

//------------------------------------------------------------------------------
/**
    Container traits.

    Unspecialized ContainerTraits has the isNotContainer typedef for SFINAE.
    All user defined containers must supply an appropriate specialization for
    ContinerTraits (without the typedef isNotContainer). The containers that
    come with LuaBridge also come with the appropriate ContainerTraits
    specialization. See the corresponding declaration for details.

    A specialization of ContainerTraits for some generic type ContainerType
    looks like this:

        template <class T>
        struct ContainerTraits <ContainerType <T> >
        {
          typedef typename T Type;

          static T* get (ContainerType <T> const& c)
          {
            return c.get (); // Implementation-dependent on ContainerType
          }
        };
*/
template <class T>
struct ContainerTraits
{
  typedef bool isNotContainer;
};

/**
    Container Construction traits.

    Unspecialized ContainerConstructionTraits implement default container
    construction with the containers constructor. Specializing this trait
    can be used to tell LuaBridge how to obtain a container from raw pointers
    in cases where special care has to be taken such as in the case of
    std::shared_ptr.

    Example:

		class A : public std::enable_shared_from_this<A> {
		public:
		  std::shared_ptr<A> get_shared() { return shared_from_this(); }
		  ...
		}

		namespace luabridge {
		  // register shared_ptr as container
		  template <class T>
		  struct ContainerTraits <std::shared_ptr<T> >
		  {
		    static T* get (std::shared_ptr<T> const& c)
			{
			  return c.get();
			}
		  };

	      // make sure shared_ptr isn't usable with not instrumented types
		  template <class T>
		  struct ContainerConstructionTraits< std::shared_ptr<T> > { };

		  // creation traits specify how to obtain a shared_ptr from a raw pointer
		  template <>
		  struct ContainerConstructionTraits< std::shared_ptr<A> >
		  {
			static std::shared_ptr<A> constructContainer(A *t)
			{
			  return t->get_shared();
			}
		  };
		}
*/
template <class C>
struct ContainerConstructionTraits
{
  typedef typename ContainerTraits<C>::Type T;
  static C constructContainer(T *t)
  {
    return C(t);
  }
};

//------------------------------------------------------------------------------
/**
    Type traits.

    Specializations return information about a type.
*/
struct TypeTraits
{
  /** Determine if type T is a container.

      To be considered a container, there must be a specialization of
      ContainerTraits with the required fields.
  */
  template <typename T>
  class isContainer
  {
  private:
    typedef char yes[1]; // sizeof (yes) == 1
    typedef char no [2]; // sizeof (no)  == 2

    template <typename C>
    static no& test (typename C::isNotContainer*);

    template <typename>
    static yes& test (...);

  public:
    static const bool value = sizeof (test <ContainerTraits <T> >(0)) == sizeof (yes);
  };

  /** Determine if T is const qualified.
  */
  /** @{ */
  template <class T>
  struct isConst
  {
    static bool const value = false;
  };

  template <class T>
  struct isConst <T const>
  {
    static bool const value = true;
  };
  /** @} */

  /** Remove the const qualifier from T.
  */
  /** @{ */
  template <class T>
  struct removeConst
  {
    typedef T Type;
  };

  template <class T>
  struct removeConst <T const>
  {
    typedef T Type;
  };
  /**@}*/
};

#endif
