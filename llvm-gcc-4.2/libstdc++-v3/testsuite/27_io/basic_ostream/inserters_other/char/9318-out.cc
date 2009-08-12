// 1999-10-11 bkoz

// Copyright (C) 1999, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
// USA.

// As a special exception, you may use this file as part of a free software
// library without restriction.  Specifically, if other files instantiate
// templates or use macros or inline functions from this file, or you compile
// this file and link it with other files to produce an executable, this
// file does not by itself cause the resulting executable to be covered by
// the GNU General Public License.  This exception does not however
// invalidate any other reasons why the executable file might be covered by
// the GNU General Public License.

// 27.5.2 template class basic_streambuf

#include <streambuf>
#include <sstream>
#include <ostream>
#include <testsuite_hooks.h>

// libstdc++/9318
class Outbuf : public std::streambuf
{
public:
  typedef std::streambuf::traits_type traits_type;

  std::string result() const { return str; }

protected:
  virtual int_type overflow(int_type c = traits_type::eof())
  {
    if (!traits_type::eq_int_type(c, traits_type::eof()))
      str.push_back(traits_type::to_char_type(c));
    return traits_type::not_eof(c);
  }

private:
  std::string str;
};

void test10()
{
  bool test __attribute__((unused)) = true;

  std::stringbuf sbuf("Bad Moon Rising", std::ios::in);
  Outbuf buf;
  std::ostream stream(&buf);
  stream << &sbuf;

  VERIFY( buf.result() == "Bad Moon Rising" );
}

int main() 
{
  test10();
  return 0;
}
