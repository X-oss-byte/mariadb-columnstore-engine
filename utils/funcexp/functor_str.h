/* Copyright (C) 2014 InfiniDB, Inc.
   Copyright (C) 2019 MariaDB Corporation

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; version 2 of
   the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
   MA 02110-1301, USA. */

//  $Id: functor_str.h 3923 2013-06-19 21:43:06Z bwilkinson $

/** @file */

#pragma once

#include "functor.h"
#include "sql_crypt.h"

namespace funcexp
{
/** @brief Func_Str class
 *    For function that returns a string result.
 *        Must implement getStrVal()
 *        Implement any other methods that behave differently from the default.
 */
class Func_Str : public Func
{
 public:
  Func_Str()
  {
  }
  Func_Str(const std::string& funcName) : Func(funcName)
  {
  }
  virtual ~Func_Str()
  {
  }

  int64_t getIntVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                    execplan::CalpontSystemCatalog::ColType& op_ct)
  {
    return atoll(getStrVal(row, fp, isNull, op_ct).c_str());
  }

  double getDoubleVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                      execplan::CalpontSystemCatalog::ColType& op_ct)
  {
    return strtod(getStrVal(row, fp, isNull, op_ct).c_str(), NULL);
  }

  long double getLongDoubleVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                               execplan::CalpontSystemCatalog::ColType& op_ct)
  {
    return strtold(getStrVal(row, fp, isNull, op_ct).c_str(), NULL);
  }

#if 0
    std::string getStrVal(rowgroup::Row& row,
                          FunctionParm& fp,
                          bool& isNull,
                          execplan::CalpontSystemCatalog::ColType& op_ct) = 0;
#endif

  execplan::IDB_Decimal getDecimalVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                                      execplan::CalpontSystemCatalog::ColType& op_ct)
  {
    return execplan::IDB_Decimal(atoll(getStrVal(row, fp, isNull, op_ct).c_str()), 0, 0);
  }

  int32_t getDateIntVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct)
  {
    auto str = getStrVal(row, fp, isNull, op_ct);
    return (isNull ? 0 : stringToDate(str));
  }

  int64_t getDatetimeIntVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                            execplan::CalpontSystemCatalog::ColType& op_ct)
  {
    auto str = getStrVal(row, fp, isNull, op_ct);
    return (isNull ? 0 : stringToDatetime(str));
  }

  int64_t getTimestampIntVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                             execplan::CalpontSystemCatalog::ColType& op_ct)
  {
    auto str = getStrVal(row, fp, isNull, op_ct);
    return (isNull ? 0 : stringToTimestamp(str, op_ct.getTimeZone()));
  }

  int64_t getTimeIntVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct)
  {
    auto str = getStrVal(row, fp, isNull, op_ct);
    return (isNull ? 0 : stringToTime(str));
  }

 protected:
  void stringValue(execplan::SPTP& fp, rowgroup::Row& row, bool& isNull, std::string& fFloatStr)
  {
    // Bug3788, use the shorter of fixed or scientific notation for floating point values.
    // [ the default format in treenode.h is fixed-point notation ]
    char buf[20];
    long double floatVal;
    int exponent;
    long double base;

    switch (fp->data()->resultType().colDataType)
    {
      case execplan::CalpontSystemCatalog::LONGDOUBLE:
        floatVal = fp->data()->getLongDoubleVal(row, isNull);
        break;

      case execplan::CalpontSystemCatalog::DOUBLE: floatVal = fp->data()->getDoubleVal(row, isNull); break;

      case execplan::CalpontSystemCatalog::FLOAT: floatVal = fp->data()->getFloatVal(row, isNull); break;

      default:
        fFloatStr = fp->data()->getStrVal(row, isNull).safeString("");
        return;
        break;
    }

    if (isNull)
    {
      return;
    }

    exponent = (int)floor(log10(fabsl(floatVal)));
    base = floatVal * pow(10, -1.0 * exponent);

    if (std::isnan(exponent) || std::isnan(base))
    {
      snprintf(buf, 20, "%Lf", floatVal);
      fFloatStr = execplan::removeTrailing0(buf, 20);
    }
    else
    {
      snprintf(buf, 20, "%.5Lf", base);
      fFloatStr = execplan::removeTrailing0(buf, 20);
      snprintf(buf, 20, "e%02d", exponent);
      fFloatStr += buf;
    }
  }
};

/** @brief Func_concat class
 */
class Func_concat : public Func_Str
{
 public:
  Func_concat() : Func_Str("concat")
  {
  }
  virtual ~Func_concat()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_concat_oracle class
 */
class Func_concat_oracle : public Func_Str
{
 public:
  Func_concat_oracle() : Func_Str("concat_operator_oracle")
  {
  }
  virtual ~Func_concat_oracle()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_substr class
 */
class Func_substr : public Func_Str
{
 public:
  Func_substr() : Func_Str("substr")
  {
  }
  virtual ~Func_substr()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_date_format class
 */
class Func_date_format : public Func_Str
{
 public:
  Func_date_format() : Func_Str("date_format")
  {
  }
  virtual ~Func_date_format()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);

  int32_t getDateIntVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);

  int64_t getDatetimeIntVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                            execplan::CalpontSystemCatalog::ColType& op_ct);

  int64_t getTimestampIntVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                             execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_lcase class
 */
class Func_lcase : public Func_Str
{
 public:
  Func_lcase() : Func_Str("lcase")
  {
  }
  virtual ~Func_lcase()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_ucase class
 */
class Func_ucase : public Func_Str
{
 public:
  Func_ucase() : Func_Str("ucase")
  {
  }
  virtual ~Func_ucase()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_left class
 */
class Func_left : public Func_Str
{
 public:
  Func_left() : Func_Str("left")
  {
  }
  virtual ~Func_left()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_ltrim class
 */
class Func_ltrim : public Func_Str
{
 public:
  Func_ltrim() : Func_Str("ltrim")
  {
  }
  virtual ~Func_ltrim()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_rtrim class
 */
class Func_rtrim : public Func_Str
{
 public:
  Func_rtrim() : Func_Str("rtrim")
  {
  }
  virtual ~Func_rtrim()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_trim class
 */
class Func_trim : public Func_Str
{
 public:
  Func_trim() : Func_Str("trim")
  {
  }
  virtual ~Func_trim()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_ltrim class
 */
class Func_ltrim_oracle : public Func_Str
{
 public:
  Func_ltrim_oracle() : Func_Str("ltrim_oracle")
  {
  }
  virtual ~Func_ltrim_oracle()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_rtrim class
 */
class Func_rtrim_oracle : public Func_Str
{
 public:
  Func_rtrim_oracle() : Func_Str("rtrim_oracle")
  {
  }
  virtual ~Func_rtrim_oracle()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_trim class
 */
class Func_trim_oracle : public Func_Str
{
 public:
  Func_trim_oracle() : Func_Str("trim_oracle")
  {
  }
  virtual ~Func_trim_oracle()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_lpad class
 */
class Func_lpad : public Func_Str
{
  static const std::string fPad;

 public:
  Func_lpad() : Func_Str("lpad")
  {
  }
  virtual ~Func_lpad()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_rpad class
 */
class Func_rpad : public Func_Str
{
  static const std::string fPad;

 public:
  Func_rpad() : Func_Str("rpad")
  {
  }
  virtual ~Func_rpad()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_replace class
 */
class Func_replace : public Func_Str
{
 public:
  Func_replace() : Func_Str("replace")
  {
  }
  virtual ~Func_replace()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

class Func_replace_oracle : public Func_Str
{
 public:
  Func_replace_oracle() : Func_Str("replace_oracle")
  {
  }
  virtual ~Func_replace_oracle()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_right class
 */
class Func_right : public Func_Str
{
 public:
  Func_right() : Func_Str("right")
  {
  }
  virtual ~Func_right()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_char class
 */
class Func_char : public Func_Str
{
 public:
  Func_char() : Func_Str("char")
  {
  }
  virtual ~Func_char()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_cast_char class
 */
class Func_cast_char : public Func_Str
{
 public:
  Func_cast_char() : Func_Str("char")
  {
  }
  virtual ~Func_cast_char()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_format class
 */
class Func_format : public Func_Str
{
 public:
  Func_format() : Func_Str("format")
  {
  }
  virtual ~Func_format()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_conv class
 */
class Func_conv : public Func_Str
{
 public:
  Func_conv() : Func_Str("conv")
  {
  }
  virtual ~Func_conv()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_md5 class
 */
class Func_md5 : public Func_Str
{
 public:
  Func_md5() : Func_Str("md5")
  {
  }
  virtual ~Func_md5()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_unhex class
 */
class Func_unhex : public Func_Str
{
 public:
  Func_unhex() : Func_Str("unhex")
  {
  }
  virtual ~Func_unhex()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_concat_ws class
 */
class Func_concat_ws : public Func_Str
{
 public:
  Func_concat_ws() : Func_Str("concat_ws")
  {
  }
  virtual ~Func_concat_ws()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_monthname class
 */
class Func_monthname : public Func_Str
{
 public:
  Func_monthname() : Func_Str("monthname")
  {
  }
  virtual ~Func_monthname()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  int64_t getIntVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                    execplan::CalpontSystemCatalog::ColType& op_ct);
  int64_t getIntValInternal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                            execplan::CalpontSystemCatalog::ColType& op_ct);

  double getDoubleVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                      execplan::CalpontSystemCatalog::ColType& op_ct);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);

  execplan::IDB_Decimal getDecimalVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                                      execplan::CalpontSystemCatalog::ColType& op_ct);

  int32_t getDateIntVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);

  int64_t getDatetimeIntVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                            execplan::CalpontSystemCatalog::ColType& op_ct);

  int64_t getTimestampIntVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                             execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_time_format class
 */
class Func_time_format : public Func_Str
{
 public:
  Func_time_format() : Func_Str("time_format")
  {
  }
  virtual ~Func_time_format()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_sec_to_time class
 */
class Func_sec_to_time : public Func_Str
{
 public:
  Func_sec_to_time() : Func_Str("sec_to_time")
  {
  }
  virtual ~Func_sec_to_time()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  int64_t getIntVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                    execplan::CalpontSystemCatalog::ColType& op_ct);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);

  double getDoubleVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                      execplan::CalpontSystemCatalog::ColType& op_ct);

  execplan::IDB_Decimal getDecimalVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                                      execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_substring_index class
 */
class Func_substring_index : public Func_Str
{
 public:
  Func_substring_index() : Func_Str("substring_index")
  {
  }
  virtual ~Func_substring_index()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_hex class
 */
class Func_hex : public Func_Str
{
 public:
  Func_hex() : Func_Str("hex")
  {
  }
  virtual ~Func_hex()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_repeat class
 */
class Func_repeat : public Func_Str
{
 public:
  Func_repeat() : Func_Str("repeat")
  {
  }
  virtual ~Func_repeat()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_inet_ntoa class to convert big-indian (network ordered) int to
 * an ascii IP address
 */
class Func_inet_ntoa : public Func_Str
{
 public:
  Func_inet_ntoa() : Func_Str("inet_ntoa")
  {
  }
  virtual ~Func_inet_ntoa()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  int64_t getIntVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                    execplan::CalpontSystemCatalog::ColType& op_ct);

  double getDoubleVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                      execplan::CalpontSystemCatalog::ColType& op_ct);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);

  bool getBoolVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                  execplan::CalpontSystemCatalog::ColType& op_ct);

  execplan::IDB_Decimal getDecimalVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                                      execplan::CalpontSystemCatalog::ColType& op_ct);

  int32_t getDateIntVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);

  int64_t getDatetimeIntVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                            execplan::CalpontSystemCatalog::ColType& op_ct);

  int64_t getTimestampIntVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                             execplan::CalpontSystemCatalog::ColType& op_ct);

  int64_t getTimeIntVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);

 private:
  void convertNtoa(int64_t ipNum, std::string& ipString);
};

/** @brief Func_reverse class
 */
class Func_reverse : public Func_Str
{
 public:
  Func_reverse() : Func_Str("reverse")
  {
  }
  virtual ~Func_reverse()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_insert class
 */
class Func_insert : public Func_Str
{
 public:
  Func_insert() : Func_Str("insert")
  {
  }
  virtual ~Func_insert()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_maketime class
 */
class Func_maketime : public Func_Str
{
 public:
  Func_maketime() : Func_Str("maketime")
  {
  }
  virtual ~Func_maketime()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_get_format class
 */
class Func_get_format : public Func_Str
{
 public:
  Func_get_format() : Func_Str("get_format")
  {
  }
  virtual ~Func_get_format()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_elt class
 */
class Func_elt : public Func_Str
{
 public:
  Func_elt() : Func_Str("elt")
  {
  }
  virtual ~Func_elt()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_sha class
 */
class Func_sha : public Func_Str
{
 public:
  Func_sha() : Func_Str("sha")
  {
  }
  virtual ~Func_sha()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_idbpartition class
 */
class Func_idbpartition : public Func_Str
{
 public:
  Func_idbpartition() : Func_Str("idbpartition")
  {
  }
  virtual ~Func_idbpartition()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_space class
 */
class Func_space : public Func_Str
{
 public:
  Func_space() : Func_Str("space")
  {
  }
  virtual ~Func_space()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_quote class
 */
class Func_quote : public Func_Str
{
 public:
  Func_quote() : Func_Str("quote")
  {
  }
  virtual ~Func_quote()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);
};

/** @brief Func_encode class
 */

class Func_encode : public Func_Str
{
 public:
  Func_encode() : Func_Str("encode"), fSeeded(false), fSeeds{0, 0}
  {
  }
  virtual ~Func_encode()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);

  void resetSeed()
  {
    fSeeded = false;
    fSeeds[0] = 0;
    fSeeds[1] = 0;
  }

 private:
  void hash_password(ulong* result, const char* password, uint password_len);
  bool fSeeded;
  SQL_CRYPT sql_crypt;
  ulong fSeeds[2];
};

/** @brief Func_encode class
 */

class Func_decode : public Func_Str
{
 public:
  Func_decode() : Func_Str("decode"), fSeeded(false), fSeeds{0, 0}
  {
  }
  virtual ~Func_decode()
  {
  }

  execplan::CalpontSystemCatalog::ColType operationType(FunctionParm& fp,
                                                        execplan::CalpontSystemCatalog::ColType& resultType);

  std::string getStrVal(rowgroup::Row& row, FunctionParm& fp, bool& isNull,
                        execplan::CalpontSystemCatalog::ColType& op_ct);

  void resetSeed()
  {
    fSeeded = false;
    fSeeds[0] = 0;
    fSeeds[1] = 0;
  }

 private:
  void hash_password(ulong* result, const char* password, uint password_len);
  bool fSeeded;
  SQL_CRYPT sql_crypt;
  ulong fSeeds[2];
};

}  // namespace funcexp
