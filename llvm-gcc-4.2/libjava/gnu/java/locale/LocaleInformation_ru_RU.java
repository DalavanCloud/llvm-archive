/* LocaleInformation_ru_RU.java
   Copyright (C) 2002 Free Software Foundation, Inc.

This file is part of GNU Classpath.

GNU Classpath is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU Classpath is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Classpath; see the file COPYING.  If not, write to the
Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301 USA.

Linking this library statically or dynamically with other modules is
making a combined work based on this library.  Thus, the terms and
conditions of the GNU General Public License cover the whole
combination.

As a special exception, the copyright holders of this library give you
permission to link this library with independent modules to produce an
executable, regardless of the license terms of these independent
modules, and to copy and distribute the resulting executable under
terms of your choice, provided that you also meet, for each linked
independent module, the terms and conditions of the license of that
module.  An independent module is a module which is not derived from
or based on this library.  If you modify this library, you may extend
this exception to your version of the library, but you are not
obligated to do so.  If you do not wish to do so, delete this
exception statement from your version. */


// This file was automatically generated by localedef.

package gnu.java.locale;

import java.util.ListResourceBundle;

public class LocaleInformation_ru_RU extends ListResourceBundle
{
  static final String decimalSeparator = ",";
  static final String groupingSeparator = ".";
  static final String numberFormat = "#,###,##0.###";
  static final String percentFormat = "#,###,##0%";
  static final String[] weekdays = { null, "\u0412\u043E\u0441\u043A\u0440\u0435\u0441\u0435\u043D\u044C\u0435", "\u041F\u043E\u043D\u0435\u0434\u0435\u043B\u044C\u043D\u0438\u043A", "\u0412\u0442\u043E\u0440\u043D\u0438\u043A", "\u0421\u0440\u0435\u0434\u0430", "\u0427\u0435\u0442\u0432\u0435\u0440\u0433", "\u041F\u044F\u0442\u043D\u0438\u0446\u0430", "\u0421\u0443\u0431\u0431\u043E\u0442\u0430" };

  static final String[] shortWeekdays = { null, "\u0412\u0441\u043A", "\u041F\u043D\u0434", "\u0412\u0442\u0440", "\u0421\u0440\u0434", "\u0427\u0442\u0432", "\u041F\u0442\u043D", "\u0421\u0431\u0442" };

  static final String[] shortMonths = { "\u042F\u043D\u0432", "\u0424\u0435\u0432", "\u041C\u0430\u0440", "\u0410\u043F\u0440", "\u041C\u0430\u0439", "\u0418\u044E\u043D", "\u0418\u044E\u043B", "\u0410\u0432\u0433", "\u0421\u0435\u043D", "\u041E\u043A\u0442", "\u041D\u043E\u044F", "\u0414\u0435\u043A", null };

  static final String[] months = { "\u042F\u043D\u0432\u0430\u0440\u044F", "\u0424\u0435\u0432\u0440\u0430\u043B\u044F", "\u041C\u0430\u0440\u0442\u0430", "\u0410\u043F\u0440\u0435\u043B\u044F", "\u041C\u0430\u044F", "\u0418\u044E\u043D\u044F", "\u0418\u044E\u043B\u044F", "\u0410\u0432\u0433\u0443\u0441\u0442\u0430", "\u0421\u0435\u043D\u0442\u044F\u0431\u0440\u044F", "\u041E\u043A\u0442\u044F\u0431\u0440\u044F", "\u041D\u043E\u044F\u0431\u0440\u044F", "\u0414\u0435\u043A\u0430\u0431\u0440\u044F", null };

  static final String[] ampms = { "", "" };

  static final String shortDateFormat = "dd.MM.yyyy";
  static final String defaultTimeFormat = "";
  static final String currencySymbol = "\u0440\u0443\u0431";
  static final String intlCurrencySymbol = "RUR";
  static final String currencyFormat = "#,###,##0.00 $;-#,###,##0.00 $";

  private static final Object[][] contents =
  {
    { "weekdays", weekdays },
    { "shortWeekdays", shortWeekdays },
    { "shortMonths", shortMonths },
    { "months", months },
    { "ampms", ampms },
    { "shortDateFormat", shortDateFormat },
    { "defaultTimeFormat", defaultTimeFormat },
    { "currencySymbol", currencySymbol },
    { "intlCurrencySymbol", intlCurrencySymbol },
    { "currencyFormat", currencyFormat },
    { "decimalSeparator", decimalSeparator },
    { "groupingSeparator", groupingSeparator },
    { "numberFormat", numberFormat },
    { "percentFormat", percentFormat },
  };

  public Object[][] getContents () { return contents; }
}
