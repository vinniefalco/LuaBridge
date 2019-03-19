#pragma once

#include <gtest/gtest.h>

#include <initializer_list>
#include <string>


using TestTypes = ::testing::Types <
  bool,
  char,
  unsigned char,
  short,
  unsigned short,
  int,
  unsigned int,
  long,
  unsigned long,
  long long,
  unsigned long long,
  float,
  double
>;

template <class T>
struct TypeTraits;

template <>
struct TypeTraits <bool>
{
  static std::initializer_list <bool> values () {return {true, false, true};}
  static std::string list () {return "true, false, true";}
};

template <>
struct TypeTraits <char>
{
  static std::initializer_list <char> values () {return {'a', 'b', 'c'};}
  static std::string list () {return "'a', 'b', 'c'";}
};

template <>
struct TypeTraits <unsigned char>
{
  static std::initializer_list <unsigned char> values () {return {1, 2, 3};}
  static std::string list () {return "1, 2, 3";}
};

template <>
struct TypeTraits <short>
{
  static std::initializer_list <short> values () {return {1, -2, 3};}
  static std::string list () {return "1, -2, 3";}
};

template <>
struct TypeTraits <unsigned short>
{
  static std::initializer_list <unsigned short> values () {return {1, 2, 3};}
  static std::string list () {return "1, 2, 3";}
};

template <>
struct TypeTraits <int>
{
  static std::initializer_list <int> values () {return {1, -2, 3};}
  static std::string list () {return "1, -2, 3";}
};

template <>
struct TypeTraits <unsigned int>
{
  static std::initializer_list <unsigned int> values () {return {1, 2, 3};}
  static std::string list () {return "1, 2, 3";}
};

template <>
struct TypeTraits <long>
{
  static std::initializer_list <long> values () {return {1, -2, 3};}
  static std::string list () {return "1, -2, 3";}
};

template <>
struct TypeTraits <unsigned long>
{
  static std::initializer_list <unsigned long> values () {return {1, 2, 3};}
  static std::string list () {return "1, 2, 3";}
};

template <>
struct TypeTraits <long long>
{
  static std::initializer_list <long long> values () {return {1, -2, 3};}
  static std::string list () {return "1, -2, 3";}
};

template <>
struct TypeTraits <unsigned long long>
{
  static std::initializer_list <unsigned long long> values () {return {1, 2, 3};}
  static std::string list () {return "1, 2, 3";}
};

template <>
struct TypeTraits <float>
{
  static std::initializer_list <float> values () {return {1.2f, -2.5f, 3.14f};}
  static std::string list () {return "1.2, -2.5, 3.14";}
};

template <>
struct TypeTraits <double>
{
  static std::initializer_list <double> values () {return {1.2, -2.5, 3.14};}
  static std::string list () {return "1.2, -2.5, 3.14";}
};
