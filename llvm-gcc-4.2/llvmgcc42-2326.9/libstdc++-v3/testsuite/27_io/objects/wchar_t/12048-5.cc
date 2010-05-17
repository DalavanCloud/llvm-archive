// Copyright (C) 2003 Free Software Foundation, Inc.
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

// DR 49 states that cin.rdbuf()->sbumpc() and fgetc(stdin) should be
// equivalent and interchangable. Currently however, cin.rdbuf()->sungetc()
// only returns characters that were read with cin.rdbuf()->sbumpc()

// { dg-do run { xfail *-*-* } }

#include <iostream>
#include <cstdio>
#include <cwchar>
#include <testsuite_hooks.h>

void
test01()
{
  std::freopen("cin_unget-1.txt", "r", stdin);

  wchar_t c1;
  std::wint_t c2;
  wchar_t c3;
  std::wcin.get(c1);
  c2 = std::fgetwc(stdin);
  std::wcin.unget();
  if (std::wcin.good())
    {
      std::wcin.get(c3);
      VERIFY( std::wcin.good() );
      VERIFY( c3 == std::char_traits<wchar_t>::to_char_type(c2) );
    }
}

int main(void)
{
  test01();
  return 0;
}
