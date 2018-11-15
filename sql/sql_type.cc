/*
   Copyright (c) 2015  MariaDB Foundation.

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; version 2 of the License.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "mariadb.h"
#include "sql_type.h"
#include "sql_const.h"
#include "sql_class.h"
#include "sql_time.h"
#include "item.h"
#include "log.h"
#include "tztime.h"

Type_handler_row         type_handler_row;

Type_handler_null        type_handler_null;

Type_handler_bool        type_handler_bool;
Type_handler_tiny        type_handler_tiny;
Type_handler_short       type_handler_short;
Type_handler_long        type_handler_long;
Type_handler_int24       type_handler_int24;
Type_handler_longlong    type_handler_longlong;
Type_handler_longlong    type_handler_ulonglong; // Only used for CAST() for now
Type_handler_vers_trx_id type_handler_vers_trx_id;
Type_handler_float       type_handler_float;
Type_handler_double      type_handler_double;
Type_handler_bit         type_handler_bit;

Type_handler_olddecimal  type_handler_olddecimal;
Type_handler_newdecimal  type_handler_newdecimal;

Type_handler_year        type_handler_year;
Type_handler_year        type_handler_year2;
Type_handler_time        type_handler_time;
Type_handler_date        type_handler_date;
Type_handler_timestamp   type_handler_timestamp;
Type_handler_timestamp2  type_handler_timestamp2;
Type_handler_datetime    type_handler_datetime;
Type_handler_time2       type_handler_time2;
Type_handler_newdate     type_handler_newdate;
Type_handler_datetime2   type_handler_datetime2;

Type_handler_enum        type_handler_enum;
Type_handler_set         type_handler_set;

Type_handler_string      type_handler_string;
Type_handler_var_string  type_handler_var_string;
Type_handler_varchar     type_handler_varchar;
Type_handler_hex_hybrid  type_handler_hex_hybrid;
static Type_handler_varchar_compressed type_handler_varchar_compressed;

Type_handler_tiny_blob   type_handler_tiny_blob;
Type_handler_medium_blob type_handler_medium_blob;
Type_handler_long_blob   type_handler_long_blob;
Type_handler_blob        type_handler_blob;
static Type_handler_blob_compressed type_handler_blob_compressed;

#ifdef HAVE_SPATIAL
Type_handler_geometry    type_handler_geometry;
#endif


bool Type_handler_data::init()
{
#ifdef HAVE_SPATIAL

#ifndef DBUG_OFF
  if (m_type_aggregator_non_commutative_test.add(&type_handler_geometry,
                                                 &type_handler_geometry,
                                                 &type_handler_geometry) ||
      m_type_aggregator_non_commutative_test.add(&type_handler_geometry,
                                                 &type_handler_varchar,
                                                 &type_handler_long_blob))
    return true;
#endif

  return
    m_type_aggregator_for_result.add(&type_handler_geometry,
                                     &type_handler_null,
                                     &type_handler_geometry) ||
    m_type_aggregator_for_result.add(&type_handler_geometry,
                                     &type_handler_geometry,
                                     &type_handler_geometry) ||
    m_type_aggregator_for_result.add(&type_handler_geometry,
                                     &type_handler_hex_hybrid,
                                     &type_handler_long_blob) ||
    m_type_aggregator_for_result.add(&type_handler_geometry,
                                     &type_handler_tiny_blob,
                                     &type_handler_long_blob) ||
    m_type_aggregator_for_result.add(&type_handler_geometry,
                                     &type_handler_blob,
                                     &type_handler_long_blob) ||
    m_type_aggregator_for_result.add(&type_handler_geometry,
                                     &type_handler_medium_blob,
                                     &type_handler_long_blob) ||
    m_type_aggregator_for_result.add(&type_handler_geometry,
                                     &type_handler_long_blob,
                                     &type_handler_long_blob) ||
    m_type_aggregator_for_result.add(&type_handler_geometry,
                                     &type_handler_varchar,
                                     &type_handler_long_blob) ||
    m_type_aggregator_for_result.add(&type_handler_geometry,
                                     &type_handler_string,
                                     &type_handler_long_blob) ||
    m_type_aggregator_for_comparison.add(&type_handler_geometry,
                                         &type_handler_geometry,
                                         &type_handler_geometry) ||
    m_type_aggregator_for_comparison.add(&type_handler_geometry,
                                         &type_handler_null,
                                         &type_handler_geometry) ||
    m_type_aggregator_for_comparison.add(&type_handler_geometry,
                                         &type_handler_long_blob,
                                         &type_handler_long_blob);
#endif
  return false;
}


Type_handler_data *type_handler_data= NULL;



void VDec::set(Item *item)
{
  m_ptr= item->val_decimal(&m_buffer);
  DBUG_ASSERT((m_ptr == NULL) == item->null_value);
}


VDec::VDec(Item *item)
{
  m_ptr= item->val_decimal(&m_buffer);
  DBUG_ASSERT((m_ptr == NULL) == item->null_value);
}


VDec_op::VDec_op(Item_func_hybrid_field_type *item)
{
  m_ptr= item->decimal_op(&m_buffer);
  DBUG_ASSERT((m_ptr == NULL) == item->null_value);
}


date_mode_t Temporal::sql_mode_for_dates(THD *thd)
{
  return ::sql_mode_for_dates(thd);
}


bool Dec_ptr::to_datetime_with_warn(THD *thd, MYSQL_TIME *to,
                                    date_mode_t fuzzydate, Item *item)
{
  if (to_datetime_with_warn(thd, to, fuzzydate, item->field_name_or_null()))
    return item->null_value|= item->make_zero_date(to, fuzzydate);
  return item->null_value= false;
}


my_decimal *Temporal::to_decimal(my_decimal *to) const
{
  return date2my_decimal(this, to);
}


my_decimal *Temporal::bad_to_decimal(my_decimal *to) const
{
  my_decimal_set_zero(to);
  return NULL;
}


Temporal_hybrid::Temporal_hybrid(THD *thd, Item *item, date_mode_t fuzzydate)
{
  if (item->get_date(thd, this, fuzzydate))
    time_type= MYSQL_TIMESTAMP_NONE;
}


void Sec6::make_from_decimal(const my_decimal *d)
{
  m_neg= my_decimal2seconds(d, &m_sec, &m_usec);
  m_truncated= (m_sec >= LONGLONG_MAX);
}


void Sec6::make_from_double(double nr)
{
  if ((m_neg= nr < 0))
    nr= -nr;
  if ((m_truncated= nr > (double) LONGLONG_MAX))
  {
    m_sec= LONGLONG_MAX;
    m_usec= 0;
  }
  else
  {
    m_sec= (ulonglong) nr;
    m_usec= (ulong) ((nr - floor(nr)) * 1000000);
  }
}


void Sec6::make_truncated_warning(THD *thd, const char *type_str) const
{
  char buff[1 + MAX_BIGINT_WIDTH + 1 + 6 + 1]; // '-' int '.' frac '\0'
  to_string(buff, sizeof(buff));
  thd->push_warning_truncated_wrong_value(type_str, buff);
}


bool Sec6::convert_to_mysql_time(THD *thd, MYSQL_TIME *ltime,
                                 date_mode_t fuzzydate, const ErrConv *str,
                                 const char *field_name) const
{
  int warn;
  bool is_time= bool(fuzzydate & TIME_TIME_ONLY);
  const char *typestr= is_time ? "time" : "datetime";
  bool rc= is_time ? to_time(ltime, &warn) :
                     to_datetime(ltime, fuzzydate, &warn);
  if (truncated())
  {
    // The value was already truncated at the constructor call time
    thd->push_warning_wrong_or_truncated_value(Sql_condition::WARN_LEVEL_WARN,
                                               !is_time, typestr,
                                               str->ptr(), field_name);
  }
  else if (rc || MYSQL_TIME_WARN_HAVE_WARNINGS(warn))
    thd->push_warning_wrong_or_truncated_value(Sql_condition::WARN_LEVEL_WARN,
                                               rc, typestr, str->ptr(),
                                               field_name);
  else if (MYSQL_TIME_WARN_HAVE_NOTES(warn))
    thd->push_warning_wrong_or_truncated_value(Sql_condition::WARN_LEVEL_NOTE,
                                               rc, typestr, str->ptr(),
                                               field_name);
  return rc;
}


VSec6::VSec6(THD *thd, Item *item, const char *type_str, ulonglong limit)
{
  if (item->decimals == 0)
  { // optimize for an important special case
    Longlong_hybrid nr(item->val_int(), item->unsigned_flag);
    make_from_int(nr);
    m_is_null= item->null_value;
    if (!m_is_null && m_sec > limit)
    {
      m_sec= limit;
      m_truncated= true;
      ErrConvInteger err(nr);
      thd->push_warning_truncated_wrong_value(type_str, err.ptr());
    }
  }
  else if (item->cmp_type() == REAL_RESULT)
  {
    double nr= item->val_real();
    make_from_double(nr);
    m_is_null= item->null_value;
    if (!m_is_null && m_sec > limit)
    {
      m_sec= limit;
      m_truncated= true;
    }
    if (m_truncated)
    {
      ErrConvDouble err(nr);
      thd->push_warning_truncated_wrong_value(type_str, err.ptr());
    }   
  }
  else
  {
    VDec tmp(item);
    (m_is_null= tmp.is_null()) ? reset() : make_from_decimal(tmp.ptr());
    if (!m_is_null && m_sec > limit)
    {
      m_sec= limit;
      m_truncated= true;
    }
    if (m_truncated)
    {
      ErrConvDecimal err(tmp.ptr());
      thd->push_warning_truncated_wrong_value(type_str, err.ptr());
    }
  }
}


Year::Year(longlong value, bool unsigned_flag, uint length)
{
  if ((m_truncated= (value < 0))) // Negative or huge unsigned
    m_year= unsigned_flag ? 9999 : 0;
  else if (value > 9999)
  {
    m_truncated= true;
    m_year= 9999;
  }
  else if (length == 2)
  {
    m_year= value < 70 ? (uint) value + 2000 :
             value <= 1900 ? (uint) value + 1900 :
             (uint) value;
  }
  else
    m_year= (uint) value;
  DBUG_ASSERT(m_year <= 9999);
}


uint Year::year_precision(const Item *item) const
{
  return item->type_handler() == &type_handler_year2 ? 2 : 4;
}


VYear::VYear(Item *item)
 :Year_null(Year(item->val_int(), item->unsigned_flag,
                 year_precision(item)), item->null_value)
{ }


VYear_op::VYear_op(Item_func_hybrid_field_type *item)
 :Year_null(Year(item->int_op(), item->unsigned_flag,
                 year_precision(item)), item->null_value)
{ }


const LEX_CSTRING Interval_DDhhmmssff::m_type_name=
  {STRING_WITH_LEN("INTERVAL DAY TO SECOND")};


Interval_DDhhmmssff::Interval_DDhhmmssff(THD *thd, MYSQL_TIME_STATUS *st,
                                         bool push_warnings,
                                         Item *item, ulong max_hour)
{
  my_time_status_init(st);
  switch (item->cmp_type()) {
  case ROW_RESULT:
    DBUG_ASSERT(0);
    time_type= MYSQL_TIMESTAMP_NONE;
    break;
  case TIME_RESULT:
    {
      if (item->get_date(thd, this, TIME_TIME_ONLY))
        time_type= MYSQL_TIMESTAMP_NONE;
      else if (time_type != MYSQL_TIMESTAMP_TIME)
      {
        st->warnings|= MYSQL_TIME_WARN_OUT_OF_RANGE;
        push_warning_wrong_or_truncated_value(thd, ErrConvTime(this),
                                              st->warnings);
        time_type= MYSQL_TIMESTAMP_NONE;
      }
      break;
    }
  case INT_RESULT:
  case REAL_RESULT:
  case DECIMAL_RESULT:
  case STRING_RESULT:
    {
      StringBuffer<STRING_BUFFER_USUAL_SIZE> tmp;
      String *str= item->val_str(&tmp);
      if (!str)
        time_type= MYSQL_TIMESTAMP_NONE;
      else if (str_to_DDhhmmssff(st, str->ptr(), str->length(), str->charset(),
                                 UINT_MAX32))
      {
        if (push_warnings)
          thd->push_warning_wrong_value(Sql_condition::WARN_LEVEL_WARN,
                                        m_type_name.str,
                                        ErrConvString(str).ptr());
        time_type= MYSQL_TIMESTAMP_NONE;
      }
      else
      {
        if (hour > max_hour)
        {
          st->warnings|= MYSQL_TIME_WARN_OUT_OF_RANGE;
          time_type= MYSQL_TIMESTAMP_NONE;
        }
        // Warn if hour or nanosecond truncation happened
        if (push_warnings)
          push_warning_wrong_or_truncated_value(thd, ErrConvString(str),
                                                st->warnings);
      }
    }
    break;
  }
  DBUG_ASSERT(is_valid_value_slow());
}


void
Interval_DDhhmmssff::push_warning_wrong_or_truncated_value(THD *thd,
                                                           const ErrConv &str,
                                                           int warnings)
{
  if (warnings & MYSQL_TIME_WARN_OUT_OF_RANGE)
  {
    thd->push_warning_wrong_value(Sql_condition::WARN_LEVEL_WARN,
                                  m_type_name.str, str.ptr());
  }
  else if (MYSQL_TIME_WARN_HAVE_WARNINGS(warnings))
  {
    thd->push_warning_truncated_wrong_value(Sql_condition::WARN_LEVEL_WARN,
                                            m_type_name.str, str.ptr());
  }
  else if (MYSQL_TIME_WARN_HAVE_NOTES(warnings))
  {
    thd->push_warning_truncated_wrong_value(Sql_condition::WARN_LEVEL_NOTE,
                                            m_type_name.str, str.ptr());
  }
}


uint Interval_DDhhmmssff::fsp(THD *thd, Item *item)
{
  MYSQL_TIME_STATUS st;
  switch (item->cmp_type()) {
  case INT_RESULT:
  case TIME_RESULT:
    return item->decimals;
  case REAL_RESULT:
  case DECIMAL_RESULT:
    return MY_MIN(item->decimals, TIME_SECOND_PART_DIGITS);
  case ROW_RESULT:
    DBUG_ASSERT(0);
    return 0;
  case STRING_RESULT:
    break;
  }
  if (!item->const_item() || item->is_expensive())
    return TIME_SECOND_PART_DIGITS;
  Interval_DDhhmmssff it(thd, &st, false/*no warnings*/, item, UINT_MAX32);
  return it.is_valid_interval_DDhhmmssff() ? st.precision :
                                             TIME_SECOND_PART_DIGITS;
}


void Time::make_from_item(THD *thd, int *warn, Item *item, const Options opt)
{
  *warn= 0;
  if (item->get_date(thd, this, opt.get_date_flags()))
    time_type= MYSQL_TIMESTAMP_NONE;
  else
    valid_MYSQL_TIME_to_valid_value(thd, warn, opt);
}


/**
  Create from a DATETIME by subtracting a given number of days,
  implementing an optimized version of calc_time_diff().
*/
void Time::make_from_datetime_with_days_diff(int *warn, const MYSQL_TIME *from,
                                             long days)
{
  *warn= 0;
  DBUG_ASSERT(from->time_type == MYSQL_TIMESTAMP_DATETIME ||
              from->time_type == MYSQL_TIMESTAMP_DATE);
  long daynr= calc_daynr(from->year, from->month, from->day);
  long daydiff= daynr - days;
  if (!daynr) // Zero date
  {
    set_zero_time(this, MYSQL_TIMESTAMP_TIME);
    neg= true;
    hour= TIME_MAX_HOUR + 1; // to report "out of range" in "warn"
  }
  else if (daydiff >=0)
  {
    neg= false;
    year= month= day= 0;
    hhmmssff_copy(from);
    hour+= daydiff * 24;
    time_type= MYSQL_TIMESTAMP_TIME;
  }
  else
  {
    longlong timediff= ((((daydiff * 24LL +
                           from->hour)   * 60LL +
                           from->minute) * 60LL +
                           from->second) * 1000000LL +
                           from->second_part);
    unpack_time(timediff, this, MYSQL_TIMESTAMP_TIME);
    if (year || month)
    {
      *warn|= MYSQL_TIME_WARN_OUT_OF_RANGE;
      year= month= day= 0;
      hour= TIME_MAX_HOUR + 1;
    }
  }
  // The above code can generate TIME values outside of the valid TIME range.
  adjust_time_range_or_invalidate(warn);
}


void Time::make_from_datetime_move_day_to_hour(int *warn,
                                               const MYSQL_TIME *from)
{
  *warn= 0;
  DBUG_ASSERT(from->time_type == MYSQL_TIMESTAMP_DATE ||
              from->time_type == MYSQL_TIMESTAMP_DATETIME);
  time_type= MYSQL_TIMESTAMP_TIME;
  neg= false;
  year= month= day= 0;
  hhmmssff_copy(from);
  datetime_to_time_YYYYMMDD_000000DD_mix_to_hours(warn, from->year,
                                                  from->month, from->day);
  adjust_time_range_or_invalidate(warn);
}


void Time::make_from_datetime(int *warn, const MYSQL_TIME *from, long curdays)
{
  if (!curdays)
    make_from_datetime_move_day_to_hour(warn, from);
  else
    make_from_datetime_with_days_diff(warn, from, curdays);
}


void Time::make_from_time(int *warn, const MYSQL_TIME *from)
{
  DBUG_ASSERT(from->time_type == MYSQL_TIMESTAMP_TIME);
  if (from->year || from->month)
    make_from_out_of_range(warn);
  else
  {
    *warn= 0;
    DBUG_ASSERT(from->day == 0);
    *(static_cast<MYSQL_TIME*>(this))= *from;
    adjust_time_range_or_invalidate(warn);
  }
}


Time::Time(int *warn, const MYSQL_TIME *from, long curdays)
{
  switch (from->time_type) {
  case MYSQL_TIMESTAMP_NONE:
  case MYSQL_TIMESTAMP_ERROR:
    make_from_out_of_range(warn);
    break;
  case MYSQL_TIMESTAMP_DATE:
  case MYSQL_TIMESTAMP_DATETIME:
    make_from_datetime(warn, from, curdays);
    break;
  case MYSQL_TIMESTAMP_TIME:
    make_from_time(warn, from);
    break;
  }
  DBUG_ASSERT(is_valid_value_slow());
}


void Temporal_with_date::make_from_item(THD *thd, Item *item, date_mode_t flags)
{
  flags&= ~TIME_TIME_ONLY;
  /*
    Some TIME type items return error when trying to do get_date()
    without TIME_TIME_ONLY set (e.g. Item_field for Field_time).
    In the SQL standard time->datetime conversion mode we add TIME_TIME_ONLY.
    In the legacy time->datetime conversion mode we do not add TIME_TIME_ONLY
    and leave it to get_date() to check date.
  */
  date_mode_t time_flag= (item->field_type() == MYSQL_TYPE_TIME &&
              !(thd->variables.old_behavior & OLD_MODE_ZERO_DATE_TIME_CAST)) ?
              TIME_TIME_ONLY : date_mode_t(0);
  if (item->get_date(thd, this, flags | time_flag))
    time_type= MYSQL_TIMESTAMP_NONE;
  else if (time_type == MYSQL_TIMESTAMP_TIME)
  {
    MYSQL_TIME tmp;
    if (time_to_datetime_with_warn(thd, this, &tmp, flags))
      time_type= MYSQL_TIMESTAMP_NONE;
    else
      *(static_cast<MYSQL_TIME*>(this))= tmp;
  }
}


void Temporal_with_date::make_from_item(THD *thd, Item *item)
{
  return make_from_item(thd, item, sql_mode_for_dates(thd));
}


void Temporal_with_date::check_date_or_invalidate(int *warn, date_mode_t flags)
{
  if (check_date(this, pack_time(this) != 0,
                 ulonglong(flags & TIME_MODE_FOR_XXX_TO_DATE), warn))
    time_type= MYSQL_TIMESTAMP_NONE;
}


void Datetime::make_from_time(THD *thd, int *warn, const MYSQL_TIME *from,
                              date_mode_t flags)
{
  DBUG_ASSERT(from->time_type == MYSQL_TIMESTAMP_TIME);
  if (time_to_datetime(thd, from, this))
    make_from_out_of_range(warn);
  else
  {
    *warn= 0;
    check_date_or_invalidate(warn, flags);
  }
}


void Datetime::make_from_datetime(THD *thd, int *warn, const MYSQL_TIME *from,
                                  date_mode_t flags)
{
  DBUG_ASSERT(from->time_type == MYSQL_TIMESTAMP_DATE ||
              from->time_type == MYSQL_TIMESTAMP_DATETIME);
  if (from->neg || check_datetime_range(from))
    make_from_out_of_range(warn);
  else
  {
    *warn= 0;
    *(static_cast<MYSQL_TIME*>(this))= *from;
    date_to_datetime(this);
    check_date_or_invalidate(warn, flags);
  }
}


Datetime::Datetime(THD *thd, int *warn, const MYSQL_TIME *from,
                   date_mode_t flags)
{
  DBUG_ASSERT(bool(flags & TIME_TIME_ONLY) == false);
  switch (from->time_type) {
  case MYSQL_TIMESTAMP_ERROR:
  case MYSQL_TIMESTAMP_NONE:
    make_from_out_of_range(warn);
    break;
  case MYSQL_TIMESTAMP_TIME:
    make_from_time(thd, warn, from, flags);
    break;
  case MYSQL_TIMESTAMP_DATETIME:
  case MYSQL_TIMESTAMP_DATE:
    make_from_datetime(thd, warn, from, flags);
    break;
  }
  DBUG_ASSERT(is_valid_value_slow());
}


uint Type_std_attributes::count_max_decimals(Item **item, uint nitems)
{
  uint res= 0;
  for (uint i= 0; i < nitems; i++)
    set_if_bigger(res, item[i]->decimals);
  return res;
}


/**
  Set max_length/decimals of function if function is fixed point and
  result length/precision depends on argument ones.
*/

void Type_std_attributes::count_decimal_length(Item **item, uint nitems)
{
  int max_int_part= 0;
  decimals= 0;
  unsigned_flag= 1;
  for (uint i=0 ; i < nitems ; i++)
  {
    set_if_bigger(decimals, item[i]->decimals);
    set_if_bigger(max_int_part, item[i]->decimal_int_part());
    set_if_smaller(unsigned_flag, item[i]->unsigned_flag);
  }
  int precision= MY_MIN(max_int_part + decimals, DECIMAL_MAX_PRECISION);
  fix_char_length(my_decimal_precision_to_length_no_truncation(precision,
                                                               (uint8) decimals,
                                                               unsigned_flag));
}


/**
  Set max_length of if it is maximum length of its arguments.
*/

void Type_std_attributes::count_only_length(Item **item, uint nitems)
{
  uint32 char_length= 0;
  unsigned_flag= 0;
  for (uint i= 0; i < nitems ; i++)
  {
    set_if_bigger(char_length, item[i]->max_char_length());
    set_if_bigger(unsigned_flag, item[i]->unsigned_flag);
  }
  fix_char_length(char_length);
}


void Type_std_attributes::count_octet_length(Item **item, uint nitems)
{
  max_length= 0;
  unsigned_flag= 0;
  for (uint i= 0; i < nitems ; i++)
  {
    set_if_bigger(max_length, item[i]->max_length);
    set_if_bigger(unsigned_flag, item[i]->unsigned_flag);
  }
}


/**
  Set max_length/decimals of function if function is floating point and
  result length/precision depends on argument ones.
*/

void Type_std_attributes::count_real_length(Item **items, uint nitems)
{
  uint32 length= 0;
  decimals= 0;
  max_length= 0;
  unsigned_flag= false;
  for (uint i=0 ; i < nitems ; i++)
  {
    if (decimals < FLOATING_POINT_DECIMALS)
    {
      set_if_bigger(decimals, items[i]->decimals);
      /* Will be ignored if items[i]->decimals >= FLOATING_POINT_DECIMALS */
      set_if_bigger(length, (items[i]->max_length - items[i]->decimals));
    }
    set_if_bigger(max_length, items[i]->max_length);
  }
  if (decimals < FLOATING_POINT_DECIMALS)
  {
    max_length= length;
    length+= decimals;
    if (length < max_length)  // If previous operation gave overflow
      max_length= UINT_MAX32;
    else
      max_length= length;
  }
  // Corner case: COALESCE(DOUBLE(255,4), DOUBLE(255,3)) -> FLOAT(255, 4)
  set_if_smaller(max_length, MAX_FIELD_CHARLENGTH);
}


/**
  Calculate max_length and decimals for string functions.

  @param field_type  Field type.
  @param items       Argument array.
  @param nitems      Number of arguments.

  @retval            False on success, true on error.
*/
bool Type_std_attributes::count_string_length(const char *func_name,
                                              Item **items, uint nitems)
{
  if (agg_arg_charsets_for_string_result(collation, func_name,
                                         items, nitems, 1))
    return true;
  if (collation.collation == &my_charset_bin)
    count_octet_length(items, nitems);
  else
    count_only_length(items, nitems);
  decimals= max_length ? NOT_FIXED_DEC : 0;
  return false;
}


/*
  Find a handler by its ODBC literal data type.

  @param type_str  - data type name, not necessarily 0-terminated
  @retval          - a pointer to data type handler if type_str points
                     to a known ODBC literal data type, or NULL otherwise
*/
const Type_handler *
Type_handler::odbc_literal_type_handler(const LEX_CSTRING *type_str)
{
  if (type_str->length == 1)
  {
    if (type_str->str[0] == 'd')      // {d'2001-01-01'}
      return &type_handler_newdate;
    else if (type_str->str[0] == 't') // {t'10:20:30'}
      return &type_handler_time2;
  }
  else if (type_str->length == 2)     // {ts'2001-01-01 10:20:30'}
  {
    if (type_str->str[0] == 't' && type_str->str[1] == 's')
      return &type_handler_datetime2;
  }
  return NULL; // Not a known ODBC literal type
}


/**
  This method is used by:
  - Item_user_var_as_out_param::field_type()
  - Item_func_udf_str::field_type()
  - Item_empty_string::make_send_field()

  TODO: type_handler_adjusted_to_max_octet_length() and string_type_handler()
  provide very similar functionality, to properly choose between
  VARCHAR/VARBINARY vs TEXT/BLOB variations taking into accoung maximum
  possible octet length.

  We should probably get rid of either of them and use the same method
  all around the code.
*/
const Type_handler *
Type_handler::string_type_handler(uint max_octet_length)
{
  if (max_octet_length >= 16777216)
    return &type_handler_long_blob;
  else if (max_octet_length >= 65536)
    return &type_handler_medium_blob;
  return &type_handler_varchar;
}


const Type_handler *
Type_handler::varstring_type_handler(const Item *item)
{
  if (!item->max_length)
    return &type_handler_string;
  if (item->too_big_for_varchar())
    return blob_type_handler(item->max_length);
  return &type_handler_varchar;
}


const Type_handler *
Type_handler::blob_type_handler(uint max_octet_length)
{
  if (max_octet_length <= 255)
    return &type_handler_tiny_blob;
  if (max_octet_length <= 65535)
    return &type_handler_blob;
  if (max_octet_length <= 16777215)
    return &type_handler_medium_blob;
  return &type_handler_long_blob;
}


const Type_handler *
Type_handler::blob_type_handler(const Item *item)
{
  return blob_type_handler(item->max_length);
}

/**
  This method is used by:
  - Item_sum_hybrid, e.g. MAX(item), MIN(item).
  - Item_func_set_user_var
*/
const Type_handler *
Type_handler_string_result::type_handler_adjusted_to_max_octet_length(
                                                        uint max_octet_length,
                                                        CHARSET_INFO *cs) const
{
  if (max_octet_length / cs->mbmaxlen <= CONVERT_IF_BIGGER_TO_BLOB)
    return &type_handler_varchar; // See also Item::too_big_for_varchar()
  if (max_octet_length >= 16777216)
    return &type_handler_long_blob;
  else if (max_octet_length >= 65536)
    return &type_handler_medium_blob;
  return &type_handler_blob;
}


CHARSET_INFO *Type_handler::charset_for_protocol(const Item *item) const
{
  /*
    For backward compatibility, to make numeric
    data types return "binary" charset in client-side metadata.
  */
  return &my_charset_bin;
}


bool
Type_handler::Item_func_or_sum_illegal_param(const char *funcname) const
{
  my_error(ER_ILLEGAL_PARAMETER_DATA_TYPE_FOR_OPERATION, MYF(0),
           name().ptr(), funcname);
  return true;
}


bool
Type_handler::Item_func_or_sum_illegal_param(const Item_func_or_sum *it) const
{
  return Item_func_or_sum_illegal_param(it->func_name());
}


CHARSET_INFO *
Type_handler_string_result::charset_for_protocol(const Item *item) const
{
  return item->collation.collation;
}


const Type_handler *
Type_handler::get_handler_by_cmp_type(Item_result type)
{
  switch (type) {
  case REAL_RESULT:       return &type_handler_double;
  case INT_RESULT:        return &type_handler_longlong;
  case DECIMAL_RESULT:    return &type_handler_newdecimal;
  case STRING_RESULT:     return &type_handler_long_blob;
  case TIME_RESULT:       return &type_handler_datetime;
  case ROW_RESULT:        return &type_handler_row;
  }
  DBUG_ASSERT(0);
  return &type_handler_string;
}


Type_handler_hybrid_field_type::Type_handler_hybrid_field_type()
  :m_type_handler(&type_handler_double)
{
}


/***************************************************************************/

/* number of bytes to store second_part part of the TIMESTAMP(N) */
uint Type_handler_timestamp::m_sec_part_bytes[MAX_DATETIME_PRECISION + 1]=
     { 0, 1, 1, 2, 2, 3, 3 };

/* number of bytes to store DATETIME(N) */
uint Type_handler_datetime::m_hires_bytes[MAX_DATETIME_PRECISION + 1]=
     { 5, 6, 6, 7, 7, 7, 8 };

/* number of bytes to store TIME(N) */
uint Type_handler_time::m_hires_bytes[MAX_DATETIME_PRECISION + 1]=
     { 3, 4, 4, 5, 5, 5, 6 };

/***************************************************************************/
const Name Type_handler_row::m_name_row(STRING_WITH_LEN("row"));

const Name Type_handler_null::m_name_null(STRING_WITH_LEN("null"));

const Name
  Type_handler_string::m_name_char(STRING_WITH_LEN("char")),
  Type_handler_var_string::m_name_var_string(STRING_WITH_LEN("varchar")),
  Type_handler_varchar::m_name_varchar(STRING_WITH_LEN("varchar")),
  Type_handler_hex_hybrid::m_name_hex_hybrid(STRING_WITH_LEN("hex_hybrid")),
  Type_handler_tiny_blob::m_name_tinyblob(STRING_WITH_LEN("tinyblob")),
  Type_handler_medium_blob::m_name_mediumblob(STRING_WITH_LEN("mediumblob")),
  Type_handler_long_blob::m_name_longblob(STRING_WITH_LEN("longblob")),
  Type_handler_blob::m_name_blob(STRING_WITH_LEN("blob"));

const Name
  Type_handler_enum::m_name_enum(STRING_WITH_LEN("enum")),
  Type_handler_set::m_name_set(STRING_WITH_LEN("set"));

const Name
  Type_handler_bool::m_name_bool(STRING_WITH_LEN("boolean")),
  Type_handler_tiny::m_name_tiny(STRING_WITH_LEN("tinyint")),
  Type_handler_short::m_name_short(STRING_WITH_LEN("smallint")),
  Type_handler_long::m_name_int(STRING_WITH_LEN("int")),
  Type_handler_longlong::m_name_longlong(STRING_WITH_LEN("bigint")),
  Type_handler_int24::m_name_mediumint(STRING_WITH_LEN("mediumint")),
  Type_handler_year::m_name_year(STRING_WITH_LEN("year")),
  Type_handler_bit::m_name_bit(STRING_WITH_LEN("bit"));

const Name
  Type_handler_float::m_name_float(STRING_WITH_LEN("float")),
  Type_handler_double::m_name_double(STRING_WITH_LEN("double"));

const Name
  Type_handler_olddecimal::m_name_decimal(STRING_WITH_LEN("decimal")),
  Type_handler_newdecimal::m_name_decimal(STRING_WITH_LEN("decimal"));

const Name
  Type_handler_time_common::m_name_time(STRING_WITH_LEN("time")),
  Type_handler_date_common::m_name_date(STRING_WITH_LEN("date")),
  Type_handler_datetime_common::m_name_datetime(STRING_WITH_LEN("datetime")),
  Type_handler_timestamp_common::m_name_timestamp(STRING_WITH_LEN("timestamp"));

const Name
  Type_handler::m_version_default(STRING_WITH_LEN("")),
  Type_handler::m_version_mariadb53(STRING_WITH_LEN("mariadb-5.3")),
  Type_handler::m_version_mysql56(STRING_WITH_LEN("mysql-5.6"));


const Type_limits_int
  Type_handler_tiny::m_limits_sint8=      Type_limits_sint8(),
  Type_handler_tiny::m_limits_uint8=      Type_limits_uint8(),
  Type_handler_short::m_limits_sint16=    Type_limits_sint16(),
  Type_handler_short::m_limits_uint16=    Type_limits_uint16(),
  Type_handler_int24::m_limits_sint24=    Type_limits_sint24(),
  Type_handler_int24::m_limits_uint24=    Type_limits_uint24(),
  Type_handler_long::m_limits_sint32=     Type_limits_sint32(),
  Type_handler_long::m_limits_uint32=     Type_limits_uint32(),
  Type_handler_longlong::m_limits_sint64= Type_limits_sint64(),
  Type_handler_longlong::m_limits_uint64= Type_limits_uint64();


/***************************************************************************/

const Type_handler *Type_handler_null::type_handler_for_comparison() const
{
  return &type_handler_null;
}


const Type_handler *Type_handler_int_result::type_handler_for_comparison() const
{
  return &type_handler_longlong;
}


const Type_handler *Type_handler_string_result::type_handler_for_comparison() const
{
  return &type_handler_long_blob;
}


const Type_handler *Type_handler_decimal_result::type_handler_for_comparison() const
{
  return &type_handler_newdecimal;
}


const Type_handler *Type_handler_real_result::type_handler_for_comparison() const
{
  return &type_handler_double;
}


const Type_handler *Type_handler_time_common::type_handler_for_comparison() const
{
  return &type_handler_time;
}

const Type_handler *Type_handler_date_common::type_handler_for_comparison() const
{
  return &type_handler_newdate;
}


const Type_handler *Type_handler_datetime_common::type_handler_for_comparison() const
{
  return &type_handler_datetime;
}


const Type_handler *Type_handler_timestamp_common::type_handler_for_comparison() const
{
  return &type_handler_datetime;
}


const Type_handler *Type_handler_row::type_handler_for_comparison() const
{
  return &type_handler_row;
}

/***************************************************************************/

const Type_handler *Type_handler_typelib::type_handler_for_item_field() const
{
  return &type_handler_string;
}


const Type_handler *Type_handler_typelib::cast_to_int_type_handler() const
{
  return &type_handler_longlong;
}


/***************************************************************************/

bool
Type_handler_hybrid_field_type::aggregate_for_result(const Type_handler *other)
{
  if (m_type_handler->is_traditional_type() && other->is_traditional_type())
  {
    m_type_handler=
      Type_handler::aggregate_for_result_traditional(m_type_handler, other);
    return false;
  }
  other= type_handler_data->
         m_type_aggregator_for_result.find_handler(m_type_handler, other);
  if (!other)
    return true;
  m_type_handler= other;
  return false;
}


const Type_handler *
Type_handler::type_handler_long_or_longlong(uint max_char_length)
{
  if (max_char_length <= MY_INT32_NUM_DECIMAL_DIGITS - 2)
    return &type_handler_long;
  return &type_handler_longlong;
}

/*
  This method is called for CASE (and its abbreviations) and LEAST/GREATEST
  when data type aggregation returned LONGLONG and there were some BIT
  expressions. This helps to adjust the data type from LONGLONG to LONG
  if all expressions fit.
*/
const Type_handler *
Type_handler::bit_and_int_mixture_handler(uint max_char_length)
{
  if (max_char_length <= MY_INT32_NUM_DECIMAL_DIGITS)
    return &type_handler_long;
  return &type_handler_longlong;
}


/**
  @brief Aggregates field types from the array of items.

  @param[in] items  array of items to aggregate the type from
  @param[in] nitems number of items in the array
  @param[in] treat_bit_as_number - if BIT should be aggregated to a non-BIT
             counterpart as a LONGLONG number or as a VARBINARY string.

             Currently behaviour depends on the function:
             - LEAST/GREATEST treat BIT as VARBINARY when
               aggregating with a non-BIT counterpart.
               Note, UNION also works this way.

             - CASE, COALESCE, IF, IFNULL treat BIT as LONGLONG when
               aggregating with a non-BIT counterpart;

             This inconsistency may be changed in the future. See MDEV-8867.

             Note, independently from "treat_bit_as_number":
             - a single BIT argument gives BIT as a result
             - two BIT couterparts give BIT as a result

  @details This function aggregates field types from the array of items.
    Found type is supposed to be used later as the result field type
    of a multi-argument function.
    Aggregation itself is performed by Type_handler::aggregate_for_result().

  @note The term "aggregation" is used here in the sense of inferring the
    result type of a function from its argument types.

  @retval false - on success
  @retval true  - on error
*/

bool
Type_handler_hybrid_field_type::aggregate_for_result(const char *funcname,
                                                     Item **items, uint nitems,
                                                     bool treat_bit_as_number)
{
  bool bit_and_non_bit_mixture_found= false;
  uint32 max_display_length;
  if (!nitems || items[0]->result_type() == ROW_RESULT)
  {
    DBUG_ASSERT(0);
    set_handler(&type_handler_null);
    return true;
  }
  set_handler(items[0]->type_handler());
  max_display_length= items[0]->max_display_length();
  for (uint i= 1 ; i < nitems ; i++)
  {
    const Type_handler *cur= items[i]->type_handler();
    set_if_bigger(max_display_length, items[i]->max_display_length());
    if (treat_bit_as_number &&
        ((type_handler() == &type_handler_bit) ^ (cur == &type_handler_bit)))
    {
      bit_and_non_bit_mixture_found= true;
      if (type_handler() == &type_handler_bit)
        set_handler(&type_handler_longlong); // BIT + non-BIT
      else
        cur= &type_handler_longlong; // non-BIT + BIT
    }
    if (aggregate_for_result(cur))
    {
      my_error(ER_ILLEGAL_PARAMETER_DATA_TYPES2_FOR_OPERATION, MYF(0),
               type_handler()->name().ptr(), cur->name().ptr(), funcname);
      return true;
    }
  }
  if (bit_and_non_bit_mixture_found && type_handler() == &type_handler_longlong)
    set_handler(Type_handler::bit_and_int_mixture_handler(max_display_length));
  return false;
}

/**
  Collect built-in data type handlers for comparison.
  This method is very similar to item_cmp_type() defined in item.cc.
  Now they coexist. Later item_cmp_type() will be removed.
  In addition to item_cmp_type(), this method correctly aggregates
  TIME with DATETIME/TIMESTAMP/DATE, so no additional find_date_time_item()
  is needed after this call.
*/

bool
Type_handler_hybrid_field_type::aggregate_for_comparison(const Type_handler *h)
{
  DBUG_ASSERT(m_type_handler == m_type_handler->type_handler_for_comparison());
  DBUG_ASSERT(h == h->type_handler_for_comparison());

  if (!m_type_handler->is_traditional_type() ||
      !h->is_traditional_type())
  {
    h= type_handler_data->
       m_type_aggregator_for_comparison.find_handler(m_type_handler, h);
    if (!h)
      return true;
    m_type_handler= h;
    DBUG_ASSERT(m_type_handler == m_type_handler->type_handler_for_comparison());
    return false;
  }

  Item_result a= cmp_type();
  Item_result b= h->cmp_type();
  if (a == STRING_RESULT && b == STRING_RESULT)
    m_type_handler= &type_handler_long_blob;
  else if (a == INT_RESULT && b == INT_RESULT)
    m_type_handler= &type_handler_longlong;
  else if (a == ROW_RESULT || b == ROW_RESULT)
    m_type_handler= &type_handler_row;
  else if (a == TIME_RESULT || b == TIME_RESULT)
  {
    if ((a == TIME_RESULT) + (b == TIME_RESULT) == 1)
    {
      /*
        We're here if there's only one temporal data type:
        either m_type_handler or h.
      */
      if (b == TIME_RESULT)
        m_type_handler= h; // Temporal types bit non-temporal types
    }
    else
    {
      /*
        We're here if both m_type_handler and h are temporal data types.
        - If both data types are TIME, we preserve TIME.
        - If both data types are DATE, we preserve DATE.
          Preserving DATE is needed for EXPLAIN FORMAT=JSON,
          to print DATE constants using proper format:
          'YYYY-MM-DD' rather than 'YYYY-MM-DD 00:00:00'.
      */
      if (m_type_handler->field_type() != h->field_type())
        m_type_handler= &type_handler_datetime;
    }
  }
  else if ((a == INT_RESULT || a == DECIMAL_RESULT) &&
           (b == INT_RESULT || b == DECIMAL_RESULT))
  {
    m_type_handler= &type_handler_newdecimal;
  }
  else
    m_type_handler= &type_handler_double;
  DBUG_ASSERT(m_type_handler == m_type_handler->type_handler_for_comparison());
  return false;
}


/**
  Aggregate data type handler for LEAST/GRATEST.
  aggregate_for_min_max() is close to aggregate_for_comparison(),
  but tries to preserve the exact type handler for string, int and temporal
  data types (instead of converting to super-types).
  FLOAT is not preserved and is converted to its super-type (DOUBLE).
  This should probably fixed eventually, for symmetry.
*/

bool
Type_handler_hybrid_field_type::aggregate_for_min_max(const Type_handler *h)
{
  if (!m_type_handler->is_traditional_type() ||
      !h->is_traditional_type())
  {
    /*
      If at least one data type is non-traditional,
      do aggregation for result immediately.
      For now we suppose that these two expressions:
        - LEAST(type1, type2)
        - COALESCE(type1, type2)
      return the same data type (or both expressions return error)
      if type1 and/or type2 are non-traditional.
      This may change in the future.
    */
    h= type_handler_data->
       m_type_aggregator_for_result.find_handler(m_type_handler, h);
    if (!h)
      return true;
    m_type_handler= h;
    return false;
  }

  Item_result a= cmp_type();
  Item_result b= h->cmp_type();
  DBUG_ASSERT(a != ROW_RESULT); // Disallowed by check_cols() in fix_fields()
  DBUG_ASSERT(b != ROW_RESULT); // Disallowed by check_cols() in fix_fields()

  if (a == STRING_RESULT && b == STRING_RESULT)
    m_type_handler=
      Type_handler::aggregate_for_result_traditional(m_type_handler, h);
  else if (a == INT_RESULT && b == INT_RESULT)
  {
    // BIT aggregates with non-BIT as BIGINT
    if (m_type_handler != h)
    {
      if (m_type_handler == &type_handler_bit)
        m_type_handler= &type_handler_longlong;
      else if (h == &type_handler_bit)
        h= &type_handler_longlong;
    }
    m_type_handler=
      Type_handler::aggregate_for_result_traditional(m_type_handler, h);
  }
  else if (a == TIME_RESULT || b == TIME_RESULT)
  {
    if ((a == TIME_RESULT) + (b == TIME_RESULT) == 1)
    {
      /*
        We're here if there's only one temporal data type:
        either m_type_handler or h.
      */
      if (b == TIME_RESULT)
        m_type_handler= h; // Temporal types bit non-temporal types
    }
    else
    {
      /*
        We're here if both m_type_handler and h are temporal data types.
      */
      m_type_handler=
        Type_handler::aggregate_for_result_traditional(m_type_handler, h);
    }
  }
  else if ((a == INT_RESULT || a == DECIMAL_RESULT) &&
           (b == INT_RESULT || b == DECIMAL_RESULT))
  {
    m_type_handler= &type_handler_newdecimal;
  }
  else
  {
    // Preserve FLOAT if two FLOATs, set to DOUBLE otherwise.
    if (m_type_handler != &type_handler_float || h != &type_handler_float)
      m_type_handler= &type_handler_double;
  }
  return false;
}


bool
Type_handler_hybrid_field_type::aggregate_for_min_max(const char *funcname,
                                                      Item **items, uint nitems)
{
  bool bit_and_non_bit_mixture_found= false;
  uint32 max_display_length;
  // LEAST/GREATEST require at least two arguments
  DBUG_ASSERT(nitems > 1);
  set_handler(items[0]->type_handler());
  max_display_length= items[0]->max_display_length();
  for (uint i= 1; i < nitems;  i++)
  {
    const Type_handler *cur= items[i]->type_handler();
    set_if_bigger(max_display_length, items[i]->max_display_length());
    // Check if BIT + non-BIT, or non-BIT + BIT
    bit_and_non_bit_mixture_found|= (m_type_handler == &type_handler_bit) !=
                                    (cur == &type_handler_bit);
    if (aggregate_for_min_max(cur))
    {
      my_error(ER_ILLEGAL_PARAMETER_DATA_TYPES2_FOR_OPERATION, MYF(0),
               type_handler()->name().ptr(), cur->name().ptr(), funcname);
      return true;
    }
  }
  if (bit_and_non_bit_mixture_found && type_handler() == &type_handler_longlong)
    set_handler(Type_handler::bit_and_int_mixture_handler(max_display_length));
  return false;
}


const Type_handler *
Type_handler::aggregate_for_num_op_traditional(const Type_handler *h0,
                                               const Type_handler *h1)
{
  Item_result r0= h0->cmp_type();
  Item_result r1= h1->cmp_type();

  if (r0 == REAL_RESULT || r1 == REAL_RESULT ||
      r0 == STRING_RESULT || r1 ==STRING_RESULT)
    return &type_handler_double;

  if (r0 == TIME_RESULT || r1 == TIME_RESULT)
    return &type_handler_datetime;

  if (r0 == DECIMAL_RESULT || r1 == DECIMAL_RESULT)
    return &type_handler_newdecimal;

  DBUG_ASSERT(r0 == INT_RESULT && r1 == INT_RESULT);
  return &type_handler_longlong;
}


const Type_aggregator::Pair*
Type_aggregator::find_pair(const Type_handler *handler1,
                           const Type_handler *handler2) const
{
  for (uint i= 0; i < m_array.elements(); i++)
  {
    const Pair& el= m_array.at(i);
    if (el.eq(handler1, handler2) ||
        (m_is_commutative && el.eq(handler2, handler1)))
      return &el;
  }
  return NULL;
}


bool
Type_handler_hybrid_field_type::aggregate_for_num_op(const Type_aggregator *agg,
                                                     const Type_handler *h0,
                                                     const Type_handler *h1)
{
  const Type_handler *hres;
  if (h0->is_traditional_type() && h1->is_traditional_type())
  {
    set_handler(Type_handler::aggregate_for_num_op_traditional(h0, h1));
    return false;
  }
  if ((hres= agg->find_handler(h0, h1)))
  {
    set_handler(hres);
    return false;
  }
  return true;
}


/***************************************************************************/

const Type_handler *
Type_handler::get_handler_by_field_type(enum_field_types type)
{
  switch (type) {
  case MYSQL_TYPE_DECIMAL:     return &type_handler_olddecimal;
  case MYSQL_TYPE_NEWDECIMAL:  return &type_handler_newdecimal;
  case MYSQL_TYPE_TINY:        return &type_handler_tiny;
  case MYSQL_TYPE_SHORT:       return &type_handler_short;
  case MYSQL_TYPE_LONG:        return &type_handler_long;
  case MYSQL_TYPE_LONGLONG:    return &type_handler_longlong;
  case MYSQL_TYPE_INT24:       return &type_handler_int24;
  case MYSQL_TYPE_YEAR:        return &type_handler_year;
  case MYSQL_TYPE_BIT:         return &type_handler_bit;
  case MYSQL_TYPE_FLOAT:       return &type_handler_float;
  case MYSQL_TYPE_DOUBLE:      return &type_handler_double;
  case MYSQL_TYPE_NULL:        return &type_handler_null;
  case MYSQL_TYPE_VARCHAR:     return &type_handler_varchar;
  case MYSQL_TYPE_TINY_BLOB:   return &type_handler_tiny_blob;
  case MYSQL_TYPE_MEDIUM_BLOB: return &type_handler_medium_blob;
  case MYSQL_TYPE_LONG_BLOB:   return &type_handler_long_blob;
  case MYSQL_TYPE_BLOB:        return &type_handler_blob;
  case MYSQL_TYPE_VAR_STRING:  return &type_handler_varchar; // Map to VARCHAR 
  case MYSQL_TYPE_STRING:      return &type_handler_string;
  case MYSQL_TYPE_ENUM:        return &type_handler_varchar; // Map to VARCHAR
  case MYSQL_TYPE_SET:         return &type_handler_varchar; // Map to VARCHAR
  case MYSQL_TYPE_GEOMETRY:
#ifdef HAVE_SPATIAL
    return &type_handler_geometry;
#else
    return NULL;
#endif
  case MYSQL_TYPE_TIMESTAMP:   return &type_handler_timestamp2;// Map to timestamp2
  case MYSQL_TYPE_TIMESTAMP2:  return &type_handler_timestamp2;
  case MYSQL_TYPE_DATE:        return &type_handler_newdate;   // Map to newdate
  case MYSQL_TYPE_TIME:        return &type_handler_time2;     // Map to time2
  case MYSQL_TYPE_TIME2:       return &type_handler_time2;
  case MYSQL_TYPE_DATETIME:    return &type_handler_datetime2; // Map to datetime2
  case MYSQL_TYPE_DATETIME2:   return &type_handler_datetime2;
  case MYSQL_TYPE_NEWDATE:
    /*
      NEWDATE is actually a real_type(), not a field_type(),
      but it's used around the code in field_type() context.
      We should probably clean up the code not to use MYSQL_TYPE_NEWDATE
      in field_type() context and add DBUG_ASSERT(0) here.
    */
    return &type_handler_newdate;
  case MYSQL_TYPE_VARCHAR_COMPRESSED:
  case MYSQL_TYPE_BLOB_COMPRESSED:
    break;
  };
  DBUG_ASSERT(0);
  return &type_handler_string;
}


const Type_handler *
Type_handler::get_handler_by_real_type(enum_field_types type)
{
  switch (type) {
  case MYSQL_TYPE_DECIMAL:     return &type_handler_olddecimal;
  case MYSQL_TYPE_NEWDECIMAL:  return &type_handler_newdecimal;
  case MYSQL_TYPE_TINY:        return &type_handler_tiny;
  case MYSQL_TYPE_SHORT:       return &type_handler_short;
  case MYSQL_TYPE_LONG:        return &type_handler_long;
  case MYSQL_TYPE_LONGLONG:    return &type_handler_longlong;
  case MYSQL_TYPE_INT24:       return &type_handler_int24;
  case MYSQL_TYPE_YEAR:        return &type_handler_year;
  case MYSQL_TYPE_BIT:         return &type_handler_bit;
  case MYSQL_TYPE_FLOAT:       return &type_handler_float;
  case MYSQL_TYPE_DOUBLE:      return &type_handler_double;
  case MYSQL_TYPE_NULL:        return &type_handler_null;
  case MYSQL_TYPE_VARCHAR:     return &type_handler_varchar;
  case MYSQL_TYPE_VARCHAR_COMPRESSED: return &type_handler_varchar_compressed;
  case MYSQL_TYPE_TINY_BLOB:   return &type_handler_tiny_blob;
  case MYSQL_TYPE_MEDIUM_BLOB: return &type_handler_medium_blob;
  case MYSQL_TYPE_LONG_BLOB:   return &type_handler_long_blob;
  case MYSQL_TYPE_BLOB:        return &type_handler_blob;
  case MYSQL_TYPE_BLOB_COMPRESSED: return &type_handler_blob_compressed;
  case MYSQL_TYPE_VAR_STRING:
    /*
      VAR_STRING is actually a field_type(), not a real_type(),
      but it's used around the code in real_type() context.
      We should clean up the code and add DBUG_ASSERT(0) here.
    */
    return &type_handler_string;
  case MYSQL_TYPE_STRING:      return &type_handler_string;
  case MYSQL_TYPE_ENUM:        return &type_handler_enum;
  case MYSQL_TYPE_SET:         return &type_handler_set;
  case MYSQL_TYPE_GEOMETRY:
#ifdef HAVE_SPATIAL
    return &type_handler_geometry;
#else
    return NULL;
#endif
  case MYSQL_TYPE_TIMESTAMP:   return &type_handler_timestamp;
  case MYSQL_TYPE_TIMESTAMP2:  return &type_handler_timestamp2;
  case MYSQL_TYPE_DATE:        return &type_handler_date;
  case MYSQL_TYPE_TIME:        return &type_handler_time;
  case MYSQL_TYPE_TIME2:       return &type_handler_time2;
  case MYSQL_TYPE_DATETIME:    return &type_handler_datetime;
  case MYSQL_TYPE_DATETIME2:   return &type_handler_datetime2;
  case MYSQL_TYPE_NEWDATE:     return &type_handler_newdate;
  };
  DBUG_ASSERT(0);
  return &type_handler_string;
}


/**
  Create a DOUBLE field by default.
*/
Field *
Type_handler::make_num_distinct_aggregator_field(MEM_ROOT *mem_root,
                                                 const Item *item) const
{
  return new(mem_root)
         Field_double(NULL, item->max_length,
                      (uchar *) (item->maybe_null ? "" : 0),
                      item->maybe_null ? 1 : 0, Field::NONE,
                      &item->name, (uint8) item->decimals,
                      0, item->unsigned_flag);
}


Field *
Type_handler_float::make_num_distinct_aggregator_field(MEM_ROOT *mem_root,
                                                       const Item *item)
                                                       const
{
  return new(mem_root)
         Field_float(NULL, item->max_length,
                     (uchar *) (item->maybe_null ? "" : 0),
                     item->maybe_null ? 1 : 0, Field::NONE,
                     &item->name, (uint8) item->decimals,
                     0, item->unsigned_flag);
}


Field *
Type_handler_decimal_result::make_num_distinct_aggregator_field(
                                                            MEM_ROOT *mem_root,
                                                            const Item *item)
                                                            const
{
  DBUG_ASSERT(item->decimals <= DECIMAL_MAX_SCALE);
  return new (mem_root)
         Field_new_decimal(NULL, item->max_length,
                           (uchar *) (item->maybe_null ? "" : 0),
                           item->maybe_null ? 1 : 0, Field::NONE,
                           &item->name, (uint8) item->decimals,
                           0, item->unsigned_flag);
}


Field *
Type_handler_int_result::make_num_distinct_aggregator_field(MEM_ROOT *mem_root,
                                                            const Item *item)
                                                            const
{
  /**
    Make a longlong field for all INT-alike types. It could create
    smaller fields for TINYINT, SMALLINT, MEDIUMINT, INT though.
  */
  return new(mem_root)
         Field_longlong(NULL, item->max_length,
                        (uchar *) (item->maybe_null ? "" : 0),
                        item->maybe_null ? 1 : 0, Field::NONE,
                        &item->name, 0, item->unsigned_flag);
}


/***********************************************************************/

Field *Type_handler_tiny::make_conversion_table_field(TABLE *table,
                                                      uint metadata,
                                                      const Field *target)
                                                      const
{
  /*
    As we don't know if the integer was signed or not on the master,
    assume we have same sign on master and slave.  This is true when not
    using conversions so it should be true also when using conversions.
  */
  bool unsigned_flag= ((Field_num*) target)->unsigned_flag;
  return new (table->in_use->mem_root)
         Field_tiny(NULL, 4 /*max_length*/, (uchar *) "", 1, Field::NONE,
                    &empty_clex_str, 0/*zerofill*/, unsigned_flag);
}


Field *Type_handler_short::make_conversion_table_field(TABLE *table,
                                                       uint metadata,
                                                       const Field *target)
                                                       const
{
  bool unsigned_flag= ((Field_num*) target)->unsigned_flag;
  return new (table->in_use->mem_root)
         Field_short(NULL, 6 /*max_length*/, (uchar *) "", 1, Field::NONE,
                     &empty_clex_str, 0/*zerofill*/, unsigned_flag);
}


Field *Type_handler_int24::make_conversion_table_field(TABLE *table,
                                                       uint metadata,
                                                       const Field *target)
                                                       const
{
  bool unsigned_flag= ((Field_num*) target)->unsigned_flag;
  return new (table->in_use->mem_root)
         Field_medium(NULL, 9 /*max_length*/, (uchar *) "", 1, Field::NONE,
                      &empty_clex_str, 0/*zerofill*/, unsigned_flag);
}


Field *Type_handler_long::make_conversion_table_field(TABLE *table,
                                                      uint metadata,
                                                      const Field *target)
                                                      const
{
  bool unsigned_flag= ((Field_num*) target)->unsigned_flag;
  return new (table->in_use->mem_root)
         Field_long(NULL, 11 /*max_length*/, (uchar *) "", 1, Field::NONE,
         &empty_clex_str, 0/*zerofill*/, unsigned_flag);
}


Field *Type_handler_longlong::make_conversion_table_field(TABLE *table,
                                                          uint metadata,
                                                          const Field *target)
                                                          const
{
  bool unsigned_flag= ((Field_num*) target)->unsigned_flag;
  return new (table->in_use->mem_root)
         Field_longlong(NULL, 20 /*max_length*/,(uchar *) "", 1, Field::NONE,
                        &empty_clex_str, 0/*zerofill*/, unsigned_flag);
}



Field *Type_handler_float::make_conversion_table_field(TABLE *table,
                                                       uint metadata,
                                                       const Field *target)
                                                       const
{
  return new (table->in_use->mem_root)
         Field_float(NULL, 12 /*max_length*/, (uchar *) "", 1, Field::NONE,
                     &empty_clex_str, 0/*dec*/, 0/*zerofill*/, 0/*unsigned_flag*/);
}


Field *Type_handler_double::make_conversion_table_field(TABLE *table,
                                                        uint metadata,
                                                        const Field *target)
                                                        const
{
  return new (table->in_use->mem_root)
         Field_double(NULL, 22 /*max_length*/, (uchar *) "", 1, Field::NONE,
                      &empty_clex_str, 0/*dec*/, 0/*zerofill*/, 0/*unsigned_flag*/);
}


Field *Type_handler_newdecimal::make_conversion_table_field(TABLE *table,
                                                            uint metadata,
                                                            const Field *target)
                                                            const
{
  int  precision= metadata >> 8;
  uint8 decimals= metadata & 0x00ff;
  uint32 max_length= my_decimal_precision_to_length(precision, decimals, false);
  DBUG_ASSERT(decimals <= DECIMAL_MAX_SCALE);
  return new (table->in_use->mem_root)
         Field_new_decimal(NULL, max_length, (uchar *) "", 1, Field::NONE,
                           &empty_clex_str, decimals, 0/*zerofill*/, 0/*unsigned*/);
}


Field *Type_handler_olddecimal::make_conversion_table_field(TABLE *table,
                                                            uint metadata,
                                                            const Field *target)
                                                            const
{
  sql_print_error("In RBR mode, Slave received incompatible DECIMAL field "
                  "(old-style decimal field) from Master while creating "
                  "conversion table. Please consider changing datatype on "
                  "Master to new style decimal by executing ALTER command for"
                  " column Name: %s.%s.%s.",
                  target->table->s->db.str,
                  target->table->s->table_name.str,
                  target->field_name.str);
  return NULL;
}


Field *Type_handler_year::make_conversion_table_field(TABLE *table,
                                                      uint metadata,
                                                      const Field *target)
                                                      const
{
  return new(table->in_use->mem_root)
         Field_year(NULL, 4, (uchar *) "", 1, Field::NONE, &empty_clex_str);
}


Field *Type_handler_null::make_conversion_table_field(TABLE *table,
                                                      uint metadata,
                                                      const Field *target)
                                                      const
{
  return new(table->in_use->mem_root)
         Field_null(NULL, 0, Field::NONE, &empty_clex_str, target->charset());
}


Field *Type_handler_timestamp::make_conversion_table_field(TABLE *table,
                                                           uint metadata,
                                                           const Field *target)
                                                           const
{
  return new_Field_timestamp(table->in_use->mem_root, NULL, (uchar *) "", 1,
                           Field::NONE, &empty_clex_str, table->s, target->decimals());
}


Field *Type_handler_timestamp2::make_conversion_table_field(TABLE *table,
                                                            uint metadata,
                                                            const Field *target)
                                                            const
{
  return new(table->in_use->mem_root)
         Field_timestampf(NULL, (uchar *) "", 1, Field::NONE,
                          &empty_clex_str, table->s, metadata);
}


Field *Type_handler_newdate::make_conversion_table_field(TABLE *table,
                                                         uint metadata,
                                                         const Field *target)
                                                         const
{
  return new(table->in_use->mem_root)
         Field_newdate(NULL, (uchar *) "", 1, Field::NONE, &empty_clex_str);
}


Field *Type_handler_date::make_conversion_table_field(TABLE *table,
                                                      uint metadata,
                                                      const Field *target)
                                                      const
{
  return new(table->in_use->mem_root)
         Field_date(NULL, (uchar *) "", 1, Field::NONE, &empty_clex_str);
}


Field *Type_handler_time::make_conversion_table_field(TABLE *table,
                                                      uint metadata,
                                                      const Field *target)
                                                      const
{
  return new_Field_time(table->in_use->mem_root, NULL, (uchar *) "", 1,
                        Field::NONE, &empty_clex_str, target->decimals());
}


Field *Type_handler_time2::make_conversion_table_field(TABLE *table,
                                                       uint metadata,
                                                       const Field *target)
                                                       const
{
  return new(table->in_use->mem_root)
         Field_timef(NULL, (uchar *) "", 1, Field::NONE, &empty_clex_str, metadata);
}


Field *Type_handler_datetime::make_conversion_table_field(TABLE *table,
                                                          uint metadata,
                                                          const Field *target)
                                                          const
{
  return new_Field_datetime(table->in_use->mem_root, NULL, (uchar *) "", 1,
                            Field::NONE, &empty_clex_str, target->decimals());
}


Field *Type_handler_datetime2::make_conversion_table_field(TABLE *table,
                                                           uint metadata,
                                                           const Field *target)
                                                           const
{
  return new(table->in_use->mem_root)
         Field_datetimef(NULL, (uchar *) "", 1,
                         Field::NONE, &empty_clex_str, metadata);
}


Field *Type_handler_bit::make_conversion_table_field(TABLE *table,
                                                     uint metadata,
                                                     const Field *target)
                                                     const
{
  DBUG_ASSERT((metadata & 0xff) <= 7);
  uint32 max_length= 8 * (metadata >> 8U) + (metadata & 0x00ff);
  return new(table->in_use->mem_root)
         Field_bit_as_char(NULL, max_length, (uchar *) "", 1,
                           Field::NONE, &empty_clex_str);
}


Field *Type_handler_string::make_conversion_table_field(TABLE *table,
                                                        uint metadata,
                                                        const Field *target)
                                                        const
{
  /* This is taken from Field_string::unpack. */
  uint32 max_length= (((metadata >> 4) & 0x300) ^ 0x300) + (metadata & 0x00ff);
  return new(table->in_use->mem_root)
         Field_string(NULL, max_length, (uchar *) "", 1,
                      Field::NONE, &empty_clex_str, target->charset());
}


Field *Type_handler_varchar::make_conversion_table_field(TABLE *table,
                                                         uint metadata,
                                                         const Field *target)
                                                         const
{
  return new(table->in_use->mem_root)
         Field_varstring(NULL, metadata, HA_VARCHAR_PACKLENGTH(metadata),
                         (uchar *) "", 1, Field::NONE, &empty_clex_str,
                         table->s, target->charset());
}


Field *Type_handler_varchar_compressed::make_conversion_table_field(TABLE *table,
                                                         uint metadata,
                                                         const Field *target)
                                                         const
{
  return new(table->in_use->mem_root)
         Field_varstring_compressed(NULL, metadata,
                                    HA_VARCHAR_PACKLENGTH(metadata),
                                    (uchar *) "", 1, Field::NONE,
                                    &empty_clex_str,
                                    table->s, target->charset(),
                                    zlib_compression_method);
}



Field *Type_handler_blob_compressed::make_conversion_table_field(TABLE *table,
                                                      uint metadata,
                                                      const Field *target)
                                                      const
{
  uint pack_length= metadata & 0x00ff;
  if (pack_length < 1 || pack_length > 4)
    return NULL; // Broken binary log?
  return new(table->in_use->mem_root)
         Field_blob_compressed(NULL, (uchar *) "", 1, Field::NONE,
                               &empty_clex_str,
                               table->s, pack_length, target->charset(),
                               zlib_compression_method);
}


#ifdef HAVE_SPATIAL
const Name Type_handler_geometry::m_name_geometry(STRING_WITH_LEN("geometry"));


const Type_handler *Type_handler_geometry::type_handler_for_comparison() const
{
  return &type_handler_geometry;
}


Field *Type_handler_geometry::make_conversion_table_field(TABLE *table,
                                                          uint metadata,
                                                          const Field *target)
                                                          const
{
  DBUG_ASSERT(target->type() == MYSQL_TYPE_GEOMETRY);
  /*
    We do not do not update feature_gis statistics here:
    status_var_increment(target->table->in_use->status_var.feature_gis);
    as this is only a temporary field.
    The statistics was already incremented when "target" was created.
  */
  return new(table->in_use->mem_root)
         Field_geom(NULL, (uchar *) "", 1, Field::NONE, &empty_clex_str, table->s, 4,
                    ((const Field_geom*) target)->geom_type,
                    ((const Field_geom*) target)->srid);
}
#endif

Field *Type_handler_enum::make_conversion_table_field(TABLE *table,
                                                      uint metadata,
                                                      const Field *target)
                                                      const
{
  DBUG_ASSERT(target->type() == MYSQL_TYPE_STRING);
  DBUG_ASSERT(target->real_type() == MYSQL_TYPE_ENUM);
  return new(table->in_use->mem_root)
         Field_enum(NULL, target->field_length,
                    (uchar *) "", 1, Field::NONE, &empty_clex_str,
                    metadata & 0x00ff/*pack_length()*/,
                    ((const Field_enum*) target)->typelib, target->charset());
}


Field *Type_handler_set::make_conversion_table_field(TABLE *table,
                                                     uint metadata,
                                                     const Field *target)
                                                     const
{
  DBUG_ASSERT(target->type() == MYSQL_TYPE_STRING);
  DBUG_ASSERT(target->real_type() == MYSQL_TYPE_SET);
  return new(table->in_use->mem_root)
         Field_set(NULL, target->field_length,
                   (uchar *) "", 1, Field::NONE, &empty_clex_str,
                   metadata & 0x00ff/*pack_length()*/,
                   ((const Field_enum*) target)->typelib, target->charset());
}

/*************************************************************************/
bool Type_handler_null::
       Column_definition_fix_attributes(Column_definition *def) const
{
  return false;
}

bool Type_handler_tiny::
       Column_definition_fix_attributes(Column_definition *def) const
{
  return def->fix_attributes_int(MAX_TINYINT_WIDTH + def->sign_length());
}

bool Type_handler_short::
       Column_definition_fix_attributes(Column_definition *def) const
{
  return def->fix_attributes_int(MAX_SMALLINT_WIDTH + def->sign_length());
}

bool Type_handler_int24::
       Column_definition_fix_attributes(Column_definition *def) const
{
  return def->fix_attributes_int(MAX_MEDIUMINT_WIDTH + def->sign_length());
}

bool Type_handler_long::
       Column_definition_fix_attributes(Column_definition *def) const
{
  return def->fix_attributes_int(MAX_INT_WIDTH + def->sign_length());
}

bool Type_handler_longlong::
       Column_definition_fix_attributes(Column_definition *def) const
{
  return def->fix_attributes_int(MAX_BIGINT_WIDTH/*no sign_length() added*/);
}

bool Type_handler_newdecimal::
       Column_definition_fix_attributes(Column_definition *def) const
{
  return def->fix_attributes_decimal();
}

bool Type_handler_olddecimal::
       Column_definition_fix_attributes(Column_definition *def) const
{
  DBUG_ASSERT(0); // Obsolete
  return true;
}

bool Type_handler_var_string::
       Column_definition_fix_attributes(Column_definition *def) const
{
  DBUG_ASSERT(0); // Obsolete
  return true;
}

bool Type_handler_varchar::
       Column_definition_fix_attributes(Column_definition *def) const
{
  /*
    Long VARCHAR's are automaticly converted to blobs in mysql_prepare_table
    if they don't have a default value
  */
  return def->check_length(ER_TOO_BIG_DISPLAYWIDTH, MAX_FIELD_BLOBLENGTH);
}

bool Type_handler_blob_common::
       Column_definition_fix_attributes(Column_definition *def) const
{
  def->flags|= BLOB_FLAG;
  return def->check_length(ER_TOO_BIG_DISPLAYWIDTH, MAX_FIELD_BLOBLENGTH);
}

#ifdef HAVE_SPATIAL
bool Type_handler_geometry::
       Column_definition_fix_attributes(Column_definition *def) const
{
  def->flags|= BLOB_FLAG;
  return false;
}
#endif

bool Type_handler_year::
       Column_definition_fix_attributes(Column_definition *def) const
{
  if (!def->length || def->length != 2)
    def->length= 4; // Default length
  def->flags|= ZEROFILL_FLAG | UNSIGNED_FLAG;
  return false;
}

bool Type_handler_float::
       Column_definition_fix_attributes(Column_definition *def) const
{
  return def->fix_attributes_real(MAX_FLOAT_STR_LENGTH);
}


bool Type_handler_double::
       Column_definition_fix_attributes(Column_definition *def) const
{
  return def->fix_attributes_real(DBL_DIG + 7);
}

bool Type_handler_timestamp_common::
       Column_definition_fix_attributes(Column_definition *def) const
{
  def->flags|= UNSIGNED_FLAG;
  return def->fix_attributes_temporal_with_time(MAX_DATETIME_WIDTH);
}

bool Type_handler_date_common::
       Column_definition_fix_attributes(Column_definition *def) const
{
  // We don't support creation of MYSQL_TYPE_DATE anymore
  def->set_handler(&type_handler_newdate);
  def->length= MAX_DATE_WIDTH;
  return false;
}

bool Type_handler_time_common::
       Column_definition_fix_attributes(Column_definition *def) const
{
  return def->fix_attributes_temporal_with_time(MIN_TIME_WIDTH);
}

bool Type_handler_datetime_common::
       Column_definition_fix_attributes(Column_definition *def) const
{
  return def->fix_attributes_temporal_with_time(MAX_DATETIME_WIDTH);
}

bool Type_handler_set::
       Column_definition_fix_attributes(Column_definition *def) const
{
  def->pack_length= get_set_pack_length(def->interval_list.elements);
  return false;
}

bool Type_handler_enum::
       Column_definition_fix_attributes(Column_definition *def) const
{
  def->pack_length= get_enum_pack_length(def->interval_list.elements);
  return false;
}

bool Type_handler_bit::
       Column_definition_fix_attributes(Column_definition *def) const
{
  return def->fix_attributes_bit();
}

/*************************************************************************/

void Type_handler_blob_common::
       Column_definition_reuse_fix_attributes(THD *thd,
                                              Column_definition *def,
                                              const Field *field) const
{
  DBUG_ASSERT(def->key_length == 0);
}


void Type_handler_typelib::
       Column_definition_reuse_fix_attributes(THD *thd,
                                              Column_definition *def,
                                              const Field *field) const
{
  DBUG_ASSERT(def->flags & (ENUM_FLAG | SET_FLAG));
  def->interval= field->get_typelib();
}


#ifdef HAVE_SPATIAL
void Type_handler_geometry::
       Column_definition_reuse_fix_attributes(THD *thd,
                                              Column_definition *def,
                                              const Field *field) const
{
  def->geom_type= ((Field_geom*) field)->geom_type;
  def->srid= ((Field_geom*) field)->srid;
}
#endif


void Type_handler_year::
       Column_definition_reuse_fix_attributes(THD *thd,
                                              Column_definition *def,
                                              const Field *field) const
{
  if (def->length != 4)
  {
    char buff[sizeof("YEAR()") + MY_INT64_NUM_DECIMAL_DIGITS + 1];
    my_snprintf(buff, sizeof(buff), "YEAR(%llu)", def->length);
    push_warning_printf(thd, Sql_condition::WARN_LEVEL_NOTE,
                        ER_WARN_DEPRECATED_SYNTAX,
                        ER_THD(thd, ER_WARN_DEPRECATED_SYNTAX),
                        buff, "YEAR(4)");
  }
}


void Type_handler_real_result::
       Column_definition_reuse_fix_attributes(THD *thd,
                                              Column_definition *def,
                                              const Field *field) const
{
  /*
    Floating points are stored with FLOATING_POINT_DECIMALS but internally
    in MariaDB used with NOT_FIXED_DEC, which is >= FLOATING_POINT_DECIMALS.
  */
  if (def->decimals >= FLOATING_POINT_DECIMALS)
    def->decimals= NOT_FIXED_DEC;
}


/*************************************************************************/

bool Type_handler::
       Column_definition_prepare_stage1(THD *thd,
                                        MEM_ROOT *mem_root,
                                        Column_definition *def,
                                        handler *file,
                                        ulonglong table_flags) const
{
  def->create_length_to_internal_length_simple();
  return false;
}

bool Type_handler_null::
       Column_definition_prepare_stage1(THD *thd,
                                        MEM_ROOT *mem_root,
                                        Column_definition *def,
                                        handler *file,
                                        ulonglong table_flags) const
{
  def->create_length_to_internal_length_null();
  return false;
}

bool Type_handler_row::
       Column_definition_prepare_stage1(THD *thd,
                                        MEM_ROOT *mem_root,
                                        Column_definition *def,
                                        handler *file,
                                        ulonglong table_flags) const
{
  def->create_length_to_internal_length_null();
  return false;
}

bool Type_handler_newdecimal::
       Column_definition_prepare_stage1(THD *thd,
                                        MEM_ROOT *mem_root,
                                        Column_definition *def,
                                        handler *file,
                                        ulonglong table_flags) const
{
  def->create_length_to_internal_length_newdecimal();
  return false;
}

bool Type_handler_bit::
       Column_definition_prepare_stage1(THD *thd,
                                        MEM_ROOT *mem_root,
                                        Column_definition *def,
                                        handler *file,
                                        ulonglong table_flags) const
{
  return def->prepare_stage1_bit(thd, mem_root, file, table_flags);
}

bool Type_handler_typelib::
       Column_definition_prepare_stage1(THD *thd,
                                        MEM_ROOT *mem_root,
                                        Column_definition *def,
                                        handler *file,
                                        ulonglong table_flags) const
{
  return def->prepare_stage1_typelib(thd, mem_root, file, table_flags);
}


bool Type_handler_string_result::
       Column_definition_prepare_stage1(THD *thd,
                                        MEM_ROOT *mem_root,
                                        Column_definition *def,
                                        handler *file,
                                        ulonglong table_flags) const
{
  return def->prepare_stage1_string(thd, mem_root, file, table_flags);
}


#ifdef HAVE_SPATIAL
bool Type_handler_geometry::
       Column_definition_prepare_stage1(THD *thd,
                                        MEM_ROOT *mem_root,
                                        Column_definition *def,
                                        handler *file,
                                        ulonglong table_flags) const
{
  def->create_length_to_internal_length_string();
  return def->prepare_blob_field(thd);
}
#endif


/*************************************************************************/

bool Type_handler::
       Column_definition_redefine_stage1(Column_definition *def,
                                         const Column_definition *dup,
                                         const handler *file,
                                         const Schema_specification_st *schema)
                                         const
{
  def->redefine_stage1_common(dup, file, schema);
  def->create_length_to_internal_length_simple();
  return false;
}


bool Type_handler_null::
       Column_definition_redefine_stage1(Column_definition *def,
                                         const Column_definition *dup,
                                         const handler *file,
                                         const Schema_specification_st *schema)
                                         const
{
  def->redefine_stage1_common(dup, file, schema);
  def->create_length_to_internal_length_null();
  return false;
}


bool Type_handler_newdecimal::
       Column_definition_redefine_stage1(Column_definition *def,
                                         const Column_definition *dup,
                                         const handler *file,
                                         const Schema_specification_st *schema)
                                         const
{
  def->redefine_stage1_common(dup, file, schema);
  def->create_length_to_internal_length_newdecimal();
  return false;
}


bool Type_handler_string_result::
       Column_definition_redefine_stage1(Column_definition *def,
                                         const Column_definition *dup,
                                         const handler *file,
                                         const Schema_specification_st *schema)
                                         const
{
  def->redefine_stage1_common(dup, file, schema);
  def->set_compression_method(dup->compression_method());
  def->create_length_to_internal_length_string();
  return false;
}


bool Type_handler_typelib::
       Column_definition_redefine_stage1(Column_definition *def,
                                         const Column_definition *dup,
                                         const handler *file,
                                         const Schema_specification_st *schema)
                                         const
{
  def->redefine_stage1_common(dup, file, schema);
  def->create_length_to_internal_length_typelib();
  return false;
}


bool Type_handler_bit::
       Column_definition_redefine_stage1(Column_definition *def,
                                         const Column_definition *dup,
                                         const handler *file,
                                         const Schema_specification_st *schema)
                                         const
{
  def->redefine_stage1_common(dup, file, schema);
  /*
    If we are replacing a field with a BIT field, we need
    to initialize pack_flag.
  */
  def->pack_flag= FIELDFLAG_NUMBER;
  if (!(file->ha_table_flags() & HA_CAN_BIT_FIELD))
    def->pack_flag|= FIELDFLAG_TREAT_BIT_AS_CHAR;
  def->create_length_to_internal_length_bit();
  return false;
}


/*************************************************************************/

bool Type_handler::
       Column_definition_prepare_stage2_legacy(Column_definition *def,
                                               enum_field_types type) const
{
  def->pack_flag= f_settype((uint) type);
  return false;
}

bool Type_handler::
       Column_definition_prepare_stage2_legacy_num(Column_definition *def,
                                                   enum_field_types type) const
{
  def->pack_flag= def->pack_flag_numeric(def->decimals) |
                  f_settype((uint) type);
  return false;
}

bool Type_handler::
       Column_definition_prepare_stage2_legacy_real(Column_definition *def,
                                                    enum_field_types type) const
{
  uint dec= def->decimals;
  /*
    User specified FLOAT() or DOUBLE() without precision. Change to
    FLOATING_POINT_DECIMALS to keep things compatible with earlier MariaDB
    versions.
  */
  if (dec >= FLOATING_POINT_DECIMALS)
    dec= FLOATING_POINT_DECIMALS;
  def->pack_flag= def->pack_flag_numeric(dec) | f_settype((uint) type);
  return false;
}

bool Type_handler_newdecimal::
       Column_definition_prepare_stage2(Column_definition *def,
                                        handler *file,
                                        ulonglong table_flags) const
{
  def->pack_flag= def->pack_flag_numeric(def->decimals);
  return false;
}

bool Type_handler_blob_common::
       Column_definition_prepare_stage2(Column_definition *def,
                                        handler *file,
                                        ulonglong table_flags) const
{
  return def->prepare_stage2_blob(file, table_flags, FIELDFLAG_BLOB);
}

#ifdef HAVE_SPATIAL
bool Type_handler_geometry::
       Column_definition_prepare_stage2(Column_definition *def,
                                        handler *file,
                                        ulonglong table_flags) const
{
  if (!(table_flags & HA_CAN_GEOMETRY))
  {
    my_error(ER_CHECK_NOT_IMPLEMENTED, MYF(0), "GEOMETRY");
    return true;
  }
  return def->prepare_stage2_blob(file, table_flags, FIELDFLAG_GEOM);
}
#endif

bool Type_handler_varchar::
       Column_definition_prepare_stage2(Column_definition *def,
                                        handler *file,
                                        ulonglong table_flags) const
{
  return def->prepare_stage2_varchar(table_flags);
}

bool Type_handler_string::
       Column_definition_prepare_stage1(THD *thd,
                                        MEM_ROOT *mem_root,
                                        Column_definition *c,
                                        handler *file,
                                        ulonglong table_flags) const
{
  if (c->check_length2(ER_TOO_BIG_FIELDLENGTH,
                       (table_flags & HA_EXTENDED_TYPES_CONVERSION) ?
                       MAX_FIELD_VARCHARLENGTH : MAX_FIELD_CHARLENGTH))
    return true;
  return Type_handler_longstr::
    Column_definition_prepare_stage1(thd, mem_root, c, file, table_flags);
}

bool Type_handler_string::
       Column_definition_prepare_stage2(Column_definition *def,
                                        handler *file,
                                        ulonglong table_flags) const
{
  def->pack_flag= (def->charset->state & MY_CS_BINSORT) ? FIELDFLAG_BINARY : 0;
  return false;
}

bool Type_handler_enum::
       Column_definition_prepare_stage2(Column_definition *def,
                                        handler *file,
                                        ulonglong table_flags) const
{
  uint dummy;
  return def->prepare_stage2_typelib("ENUM", FIELDFLAG_INTERVAL, &dummy);
}

bool Type_handler_set::
       Column_definition_prepare_stage2(Column_definition *def,
                                        handler *file,
                                        ulonglong table_flags) const
{
  uint dup_count;
  if (def->prepare_stage2_typelib("SET", FIELDFLAG_BITFIELD, &dup_count))
    return true;
  /* Check that count of unique members is not more then 64 */
  if (def->interval->count - dup_count > sizeof(longlong)*8)
  {
     my_error(ER_TOO_BIG_SET, MYF(0), def->field_name.str);
     return true;
  }
  return false;
}

bool Type_handler_bit::
       Column_definition_prepare_stage2(Column_definition *def,
                                        handler *file,
                                        ulonglong table_flags) const
{
  /* 
    We have sql_field->pack_flag already set here, see
    mysql_prepare_create_table().
  */
  return false;
}

/*************************************************************************/

uint32 Type_handler_time::calc_pack_length(uint32 length) const
{
  return length > MIN_TIME_WIDTH ?
         hires_bytes(length - 1 - MIN_TIME_WIDTH) : 3;
}

uint32 Type_handler_time2::calc_pack_length(uint32 length) const
{
  return length > MIN_TIME_WIDTH ?
         my_time_binary_length(length - MIN_TIME_WIDTH - 1) : 3;
}

uint32 Type_handler_timestamp::calc_pack_length(uint32 length) const
{
  return length > MAX_DATETIME_WIDTH ?
         4 + sec_part_bytes(length - 1 - MAX_DATETIME_WIDTH) : 4;
}

uint32 Type_handler_timestamp2::calc_pack_length(uint32 length) const
{
  return length > MAX_DATETIME_WIDTH ?
         my_timestamp_binary_length(length - MAX_DATETIME_WIDTH - 1) : 4;
}

uint32 Type_handler_datetime::calc_pack_length(uint32 length) const
{
  return length > MAX_DATETIME_WIDTH ?
         hires_bytes(length - 1 - MAX_DATETIME_WIDTH) : 8;
}

uint32 Type_handler_datetime2::calc_pack_length(uint32 length) const
{
  return length > MAX_DATETIME_WIDTH ?
         my_datetime_binary_length(length - MAX_DATETIME_WIDTH - 1) : 5;
}

uint32 Type_handler_tiny_blob::calc_pack_length(uint32 length) const
{
  return 1 + portable_sizeof_char_ptr;
}

uint32 Type_handler_blob::calc_pack_length(uint32 length) const
{
  return 2 + portable_sizeof_char_ptr;
}

uint32 Type_handler_medium_blob::calc_pack_length(uint32 length) const
{
  return 3 + portable_sizeof_char_ptr;
}

uint32 Type_handler_long_blob::calc_pack_length(uint32 length) const
{
  return 4 + portable_sizeof_char_ptr;
}

#ifdef HAVE_SPATIAL
uint32 Type_handler_geometry::calc_pack_length(uint32 length) const
{
  return 4 + portable_sizeof_char_ptr;
}
#endif

uint32 Type_handler_newdecimal::calc_pack_length(uint32 length) const
{
  abort();  // This shouldn't happen
  return 0;
}

uint32 Type_handler_set::calc_pack_length(uint32 length) const
{
  abort();  // This shouldn't happen
  return 0;
}

uint32 Type_handler_enum::calc_pack_length(uint32 length) const
{
  abort();  // This shouldn't happen
  return 0;
}


/*************************************************************************/
Field *Type_handler::make_and_init_table_field(const LEX_CSTRING *name,
                                               const Record_addr &addr,
                                               const Type_all_attributes &attr,
                                               TABLE *table) const
{
  Field *field= make_table_field(name, addr, attr, table);
  if (field)
    field->init(table);
  return field;
}


Field *Type_handler_tiny::make_table_field(const LEX_CSTRING *name,
                                           const Record_addr &addr,
                                           const Type_all_attributes &attr,
                                           TABLE *table) const
{
  return new (table->in_use->mem_root)
         Field_tiny(addr.ptr(), attr.max_char_length(),
                    addr.null_ptr(), addr.null_bit(),
                    Field::NONE, name, 0/*zerofill*/, attr.unsigned_flag);
}


Field *Type_handler_short::make_table_field(const LEX_CSTRING *name,
                                           const Record_addr &addr,
                                           const Type_all_attributes &attr,
                                           TABLE *table) const

{
  return new (table->in_use->mem_root)
         Field_short(addr.ptr(), attr.max_char_length(),
                     addr.null_ptr(), addr.null_bit(),
                     Field::NONE, name, 0/*zerofill*/, attr.unsigned_flag);
}


Field *Type_handler_int24::make_table_field(const LEX_CSTRING *name,
                                            const Record_addr &addr,
                                            const Type_all_attributes &attr,
                                            TABLE *table) const
{
  return new (table->in_use->mem_root)
         Field_medium(addr.ptr(), attr.max_char_length(),
                      addr.null_ptr(), addr.null_bit(),
                      Field::NONE, name,
                      0/*zerofill*/, attr.unsigned_flag);
}


Field *Type_handler_long::make_table_field(const LEX_CSTRING *name,
                                           const Record_addr &addr,
                                           const Type_all_attributes &attr,
                                           TABLE *table) const
{
  return new (table->in_use->mem_root)
         Field_long(addr.ptr(), attr.max_char_length(),
                    addr.null_ptr(), addr.null_bit(),
                    Field::NONE, name, 0/*zerofill*/, attr.unsigned_flag);
}


Field *Type_handler_longlong::make_table_field(const LEX_CSTRING *name,
                                               const Record_addr &addr,
                                               const Type_all_attributes &attr,
                                               TABLE *table) const
{
  return new (table->in_use->mem_root)
         Field_longlong(addr.ptr(), attr.max_char_length(),
                        addr.null_ptr(), addr.null_bit(),
                        Field::NONE, name,
                        0/*zerofill*/, attr.unsigned_flag);
}


Field *Type_handler_vers_trx_id::make_table_field(const LEX_CSTRING *name,
                                               const Record_addr &addr,
                                               const Type_all_attributes &attr,
                                               TABLE *table) const
{
  return new (table->in_use->mem_root)
         Field_vers_trx_id(addr.ptr(), attr.max_char_length(),
                        addr.null_ptr(), addr.null_bit(),
                        Field::NONE, name,
                        0/*zerofill*/, attr.unsigned_flag);
}


Field *Type_handler_float::make_table_field(const LEX_CSTRING *name,
                                            const Record_addr &addr,
                                            const Type_all_attributes &attr,
                                            TABLE *table) const
{
  return new (table->in_use->mem_root)
         Field_float(addr.ptr(), attr.max_char_length(),
                     addr.null_ptr(), addr.null_bit(),
                     Field::NONE, name,
                     (uint8) attr.decimals, 0/*zerofill*/, attr.unsigned_flag);
}


Field *Type_handler_double::make_table_field(const LEX_CSTRING *name,
                                             const Record_addr &addr,
                                             const Type_all_attributes &attr,
                                             TABLE *table) const
{
  return new (table->in_use->mem_root)
         Field_double(addr.ptr(), attr.max_char_length(),
                      addr.null_ptr(), addr.null_bit(),
                      Field::NONE, name,
                      (uint8) attr.decimals, 0/*zerofill*/, attr.unsigned_flag);
}


Field *
Type_handler_olddecimal::make_table_field(const LEX_CSTRING *name,
                                          const Record_addr &addr,
                                          const Type_all_attributes &attr,
                                          TABLE *table) const
{
  /*
    Currently make_table_field() is used for Item purpose only.
    On Item level we have type_handler_newdecimal only.
    For now we have DBUG_ASSERT(0).
    It will be removed when we reuse Type_handler::make_table_field()
    in make_field() in field.cc, to open old tables with old decimal.
  */
  DBUG_ASSERT(0);
  return new (table->in_use->mem_root)
         Field_decimal(addr.ptr(), attr.max_length,
                       addr.null_ptr(), addr.null_bit(),
                       Field::NONE, name, (uint8) attr.decimals,
                       0/*zerofill*/,attr.unsigned_flag);
}


Field *
Type_handler_newdecimal::make_table_field(const LEX_CSTRING *name,
                                          const Record_addr &addr,
                                          const Type_all_attributes &attr,
                                          TABLE *table) const
{
  uint8 dec= (uint8) attr.decimals;
  uint8 intg= (uint8) (attr.decimal_precision() - dec);
  uint32 len= attr.max_char_length();

  /*
    Trying to put too many digits overall in a DECIMAL(prec,dec)
    will always throw a warning. We must limit dec to
    DECIMAL_MAX_SCALE however to prevent an assert() later.
  */

  if (dec > 0)
  {
    signed int overflow;

    dec= MY_MIN(dec, DECIMAL_MAX_SCALE);

    /*
      If the value still overflows the field with the corrected dec,
      we'll throw out decimals rather than integers. This is still
      bad and of course throws a truncation warning.
      +1: for decimal point
      */

    const int required_length=
      my_decimal_precision_to_length(intg + dec, dec, attr.unsigned_flag);

    overflow= required_length - len;

    if (overflow > 0)
      dec= MY_MAX(0, dec - overflow);            // too long, discard fract
    else
      /* Corrected value fits. */
      len= required_length;
  }
  return new (table->in_use->mem_root)
         Field_new_decimal(addr.ptr(), len, addr.null_ptr(), addr.null_bit(),
                           Field::NONE, name,
                           dec, 0/*zerofill*/, attr.unsigned_flag);
}


Field *Type_handler_year::make_table_field(const LEX_CSTRING *name,
                                           const Record_addr &addr,
                                           const Type_all_attributes &attr,
                                           TABLE *table) const
{
  return new (table->in_use->mem_root)
         Field_year(addr.ptr(), attr.max_length,
                    addr.null_ptr(), addr.null_bit(),
                    Field::NONE, name);
}


Field *Type_handler_null::make_table_field(const LEX_CSTRING *name,
                                           const Record_addr &addr,
                                           const Type_all_attributes &attr,
                                           TABLE *table) const

{
  return new (table->in_use->mem_root)
         Field_null(addr.ptr(), attr.max_length,
                    Field::NONE, name, attr.collation.collation);
}


Field *Type_handler_timestamp::make_table_field(const LEX_CSTRING *name,
                                                const Record_addr &addr,
                                                const Type_all_attributes &attr,
                                                TABLE *table) const

{
  return new_Field_timestamp(table->in_use->mem_root,
                             addr.ptr(), addr.null_ptr(), addr.null_bit(),
                             Field::NONE, name, table->s, attr.decimals);
}


Field *Type_handler_timestamp2::make_table_field(const LEX_CSTRING *name,
                                                 const Record_addr &addr,
                                                 const Type_all_attributes &attr,
                                                 TABLE *table) const

{
  /*
    Will be changed to "new Field_timestampf" when we reuse
    make_table_field() for make_field() purposes in field.cc.
  */
  return new_Field_timestamp(table->in_use->mem_root,
                             addr.ptr(), addr.null_ptr(), addr.null_bit(),
                             Field::NONE, name, table->s, attr.decimals);
}


Field *Type_handler_newdate::make_table_field(const LEX_CSTRING *name,
                                              const Record_addr &addr,
                                              const Type_all_attributes &attr,
                                              TABLE *table) const

{
  return new (table->in_use->mem_root)
         Field_newdate(addr.ptr(), addr.null_ptr(), addr.null_bit(),
                       Field::NONE, name);
}


Field *Type_handler_date::make_table_field(const LEX_CSTRING *name,
                                           const Record_addr &addr,
                                           const Type_all_attributes &attr,
                                           TABLE *table) const

{
  /*
    DBUG_ASSERT will be removed when we reuse make_table_field()
    for make_field() in field.cc
  */
  DBUG_ASSERT(0);
  return new (table->in_use->mem_root)
         Field_date(addr.ptr(), addr.null_ptr(), addr.null_bit(),
                    Field::NONE, name);
}


Field *Type_handler_time::make_table_field(const LEX_CSTRING *name,
                                           const Record_addr &addr,
                                           const Type_all_attributes &attr,
                                           TABLE *table) const

{
  return new_Field_time(table->in_use->mem_root,
                        addr.ptr(), addr.null_ptr(), addr.null_bit(),
                        Field::NONE, name, attr.decimals);
}


Field *Type_handler_time2::make_table_field(const LEX_CSTRING *name,
                                            const Record_addr &addr,
                                            const Type_all_attributes &attr,
                                            TABLE *table) const


{
  /*
    Will be changed to "new Field_timef" when we reuse
    make_table_field() for make_field() purposes in field.cc.
  */
  return new_Field_time(table->in_use->mem_root,
                        addr.ptr(), addr.null_ptr(), addr.null_bit(),
                        Field::NONE, name, attr.decimals);
}


Field *Type_handler_datetime::make_table_field(const LEX_CSTRING *name,
                                               const Record_addr &addr,
                                               const Type_all_attributes &attr,
                                               TABLE *table) const

{
  return new_Field_datetime(table->in_use->mem_root,
                            addr.ptr(), addr.null_ptr(), addr.null_bit(),
                            Field::NONE, name, attr.decimals);
}


Field *Type_handler_datetime2::make_table_field(const LEX_CSTRING *name,
                                                const Record_addr &addr,
                                                const Type_all_attributes &attr,
                                                TABLE *table) const
{
  /*
    Will be changed to "new Field_datetimef" when we reuse
    make_table_field() for make_field() purposes in field.cc.
  */
  return new_Field_datetime(table->in_use->mem_root,
                            addr.ptr(), addr.null_ptr(), addr.null_bit(),
                            Field::NONE, name, attr.decimals);
}


Field *Type_handler_bit::make_table_field(const LEX_CSTRING *name,
                                          const Record_addr &addr,
                                          const Type_all_attributes &attr,
                                          TABLE *table) const

{
  return new (table->in_use->mem_root)
         Field_bit_as_char(addr.ptr(), attr.max_length,
                           addr.null_ptr(), addr.null_bit(),
                           Field::NONE, name);
}


Field *Type_handler_string::make_table_field(const LEX_CSTRING *name,
                                             const Record_addr &addr,
                                             const Type_all_attributes &attr,
                                             TABLE *table) const

{
  return new (table->in_use->mem_root)
         Field_string(addr.ptr(), attr.max_length,
                      addr.null_ptr(), addr.null_bit(),
                      Field::NONE, name, attr.collation);
}


Field *Type_handler_varchar::make_table_field(const LEX_CSTRING *name,
                                              const Record_addr &addr,
                                              const Type_all_attributes &attr,
                                              TABLE *table) const

{
  return new (table->in_use->mem_root)
         Field_varstring(addr.ptr(), attr.max_length,
                         HA_VARCHAR_PACKLENGTH(attr.max_length),
                         addr.null_ptr(), addr.null_bit(),
                         Field::NONE, name,
                         table->s, attr.collation);
}


Field *Type_handler_tiny_blob::make_table_field(const LEX_CSTRING *name,
                                                const Record_addr &addr,
                                                const Type_all_attributes &attr,
                                                TABLE *table) const

{
  return new (table->in_use->mem_root)
         Field_blob(addr.ptr(), addr.null_ptr(), addr.null_bit(),
                    Field::NONE, name, table->s,
                    1, attr.collation);
}


Field *Type_handler_blob::make_table_field(const LEX_CSTRING *name,
                                           const Record_addr &addr,
                                           const Type_all_attributes &attr,
                                           TABLE *table) const

{
  return new (table->in_use->mem_root)
         Field_blob(addr.ptr(), addr.null_ptr(), addr.null_bit(),
                    Field::NONE, name, table->s,
                    2, attr.collation);
}


Field *
Type_handler_medium_blob::make_table_field(const LEX_CSTRING *name,
                                           const Record_addr &addr,
                                           const Type_all_attributes &attr,
                                           TABLE *table) const

{
  return new (table->in_use->mem_root)
         Field_blob(addr.ptr(), addr.null_ptr(), addr.null_bit(),
                    Field::NONE, name, table->s,
                    3, attr.collation);
}


Field *Type_handler_long_blob::make_table_field(const LEX_CSTRING *name,
                                                const Record_addr &addr,
                                                const Type_all_attributes &attr,
                                                TABLE *table) const

{
  return new (table->in_use->mem_root)
         Field_blob(addr.ptr(), addr.null_ptr(), addr.null_bit(),
                    Field::NONE, name, table->s,
                    4, attr.collation);
}



#ifdef HAVE_SPATIAL
Field *Type_handler_geometry::make_table_field(const LEX_CSTRING *name,
                                               const Record_addr &addr,
                                               const Type_all_attributes &attr,
                                               TABLE *table) const
{
  return new (table->in_use->mem_root)
         Field_geom(addr.ptr(), addr.null_ptr(), addr.null_bit(),
                    Field::NONE, name, table->s, 4,
                    (Field::geometry_type) attr.uint_geometry_type(),
                    0);
}
#endif


Field *Type_handler_enum::make_table_field(const LEX_CSTRING *name,
                                           const Record_addr &addr,
                                           const Type_all_attributes &attr,
                                           TABLE *table) const
{
  TYPELIB *typelib= attr.get_typelib();
  DBUG_ASSERT(typelib);
  return new (table->in_use->mem_root)
         Field_enum(addr.ptr(), attr.max_length,
                    addr.null_ptr(), addr.null_bit(),
                    Field::NONE, name,
                    get_enum_pack_length(typelib->count), typelib,
                    attr.collation);
}


Field *Type_handler_set::make_table_field(const LEX_CSTRING *name,
                                          const Record_addr &addr,
                                          const Type_all_attributes &attr,
                                          TABLE *table) const

{
  TYPELIB *typelib= attr.get_typelib();
  DBUG_ASSERT(typelib);
  return new (table->in_use->mem_root)
         Field_set(addr.ptr(), attr.max_length,
                   addr.null_ptr(), addr.null_bit(),
                   Field::NONE, name,
                   get_enum_pack_length(typelib->count), typelib,
                   attr.collation);
}

/*************************************************************************/

/*
   If length is not specified for a varchar parameter, set length to the
   maximum length of the actual argument. Goals are:
   - avoid to allocate too much unused memory for m_var_table
   - allow length check inside the callee rather than during copy of
     returned values in output variables.
   - allow varchar parameter size greater than 4000
   Default length has been stored in "decimal" member during parse.
*/
bool Type_handler_varchar::adjust_spparam_type(Spvar_definition *def,
                                               Item *from) const
{
  if (def->decimals)
  {
    uint def_max_char_length= MAX_FIELD_VARCHARLENGTH / def->charset->mbmaxlen;
    uint arg_max_length= from->max_char_length();
    set_if_smaller(arg_max_length, def_max_char_length);
    def->length= arg_max_length > 0 ? arg_max_length : def->decimals;
    def->create_length_to_internal_length_string();
  }
  return false;
}

/*************************************************************************/

uint32 Type_handler_decimal_result::max_display_length(const Item *item) const
{
  return item->max_length;
}


uint32 Type_handler_temporal_result::max_display_length(const Item *item) const
{
  return item->max_length;
}


uint32 Type_handler_string_result::max_display_length(const Item *item) const
{
  return item->max_length;
}


uint32 Type_handler_year::max_display_length(const Item *item) const
{
  return item->max_length;
}


uint32 Type_handler_bit::max_display_length(const Item *item) const
{
  return item->max_length;
}


uint32 Type_handler_general_purpose_int::max_display_length(const Item *item)
                                                            const
{
  return type_limits_int_by_unsigned_flag(item->unsigned_flag)->char_length();
}


/*************************************************************************/

void Type_handler_row::Item_update_null_value(Item *item) const
{
  DBUG_ASSERT(0);
  item->null_value= true;
}


void Type_handler_time_common::Item_update_null_value(Item *item) const
{
  MYSQL_TIME ltime;
  (void) item->get_date(current_thd, &ltime, TIME_TIME_ONLY);
}


void Type_handler_temporal_with_date::Item_update_null_value(Item *item) const
{
  MYSQL_TIME ltime;
  (void) item->get_date(current_thd, &ltime, sql_mode_for_dates(current_thd));
}


void Type_handler_string_result::Item_update_null_value(Item *item) const
{
  StringBuffer<MAX_FIELD_WIDTH> tmp;
  (void) item->val_str(&tmp);
}


void Type_handler_real_result::Item_update_null_value(Item *item) const
{
  (void) item->val_real();
}


void Type_handler_decimal_result::Item_update_null_value(Item *item) const
{
  my_decimal tmp;
  (void) item->val_decimal(&tmp);
}


void Type_handler_int_result::Item_update_null_value(Item *item) const
{
  (void) item->val_int();
}


void Type_handler_bool::Item_update_null_value(Item *item) const
{
  (void) item->val_bool();
}


/*************************************************************************/

int Type_handler_time_common::Item_save_in_field(Item *item, Field *field,
                                                 bool no_conversions) const
{
  return item->save_time_in_field(field, no_conversions);
}

int Type_handler_temporal_with_date::Item_save_in_field(Item *item,
                                                        Field *field,
                                                        bool no_conversions)
                                                        const
{
  return item->save_date_in_field(field, no_conversions);
}


int Type_handler_string_result::Item_save_in_field(Item *item, Field *field,
                                                   bool no_conversions) const
{
  return item->save_str_in_field(field, no_conversions);
}


int Type_handler_real_result::Item_save_in_field(Item *item, Field *field,
                                                 bool no_conversions) const
{
  return item->save_real_in_field(field, no_conversions);
}


int Type_handler_decimal_result::Item_save_in_field(Item *item, Field *field,
                                                    bool no_conversions) const
{
  return item->save_decimal_in_field(field, no_conversions);
}


int Type_handler_int_result::Item_save_in_field(Item *item, Field *field,
                                                bool no_conversions) const
{
  return item->save_int_in_field(field, no_conversions);
}


/***********************************************************************/

bool Type_handler_row::set_comparator_func(Arg_comparator *cmp) const
{
  return cmp->set_cmp_func_row();
}

bool Type_handler_int_result::set_comparator_func(Arg_comparator *cmp) const
{
  return cmp->set_cmp_func_int();
}

bool Type_handler_real_result::set_comparator_func(Arg_comparator *cmp) const
{
  return cmp->set_cmp_func_real();
}

bool Type_handler_decimal_result::set_comparator_func(Arg_comparator *cmp) const
{
  return cmp->set_cmp_func_decimal();
}

bool Type_handler_string_result::set_comparator_func(Arg_comparator *cmp) const
{
  return cmp->set_cmp_func_string();
}

bool Type_handler_time_common::set_comparator_func(Arg_comparator *cmp) const
{
  return cmp->set_cmp_func_time();
}

bool
Type_handler_temporal_with_date::set_comparator_func(Arg_comparator *cmp) const
{
  return cmp->set_cmp_func_datetime();
}


/*************************************************************************/

bool Type_handler_temporal_result::
       can_change_cond_ref_to_const(Item_bool_func2 *target,
                                    Item *target_expr, Item *target_value,
                                    Item_bool_func2 *source,
                                    Item *source_expr, Item *source_const)
                                    const
{
  if (source->compare_type_handler()->cmp_type() != TIME_RESULT)
    return false;

  /*
    Can't rewrite:
      WHERE COALESCE(time_column)='00:00:00'
        AND COALESCE(time_column)=DATE'2015-09-11'
    to
      WHERE DATE'2015-09-11'='00:00:00'
        AND COALESCE(time_column)=DATE'2015-09-11'
    because the left part will erroneously try to parse '00:00:00'
    as DATE, not as TIME.

    TODO: It could still be rewritten to:
      WHERE DATE'2015-09-11'=TIME'00:00:00'
        AND COALESCE(time_column)=DATE'2015-09-11'
    i.e. we need to replace both target_expr and target_value
    at the same time. This is not supported yet.
  */
  return target_value->cmp_type() == TIME_RESULT;
}


bool Type_handler_string_result::
       can_change_cond_ref_to_const(Item_bool_func2 *target,
                                    Item *target_expr, Item *target_value,
                                    Item_bool_func2 *source,
                                    Item *source_expr, Item *source_const)
                                    const
{
  if (source->compare_type_handler()->cmp_type() != STRING_RESULT)
    return false;
  /*
    In this example:
      SET NAMES utf8 COLLATE utf8_german2_ci;
      DROP TABLE IF EXISTS t1;
      CREATE TABLE t1 (a CHAR(10) CHARACTER SET utf8);
      INSERT INTO t1 VALUES ('o-umlaut'),('oe');
      SELECT * FROM t1 WHERE a='oe' COLLATE utf8_german2_ci AND a='oe';

    the query should return only the row with 'oe'.
    It should not return 'o-umlaut', because 'o-umlaut' does not match
    the right part of the condition: a='oe'
    ('o-umlaut' is not equal to 'oe' in utf8_general_ci,
     which is the collation of the field "a").

    If we change the right part from:
       ... AND a='oe'
    to
       ... AND 'oe' COLLATE utf8_german2_ci='oe'
    it will be evalulated to TRUE and removed from the condition,
    so the overall query will be simplified to:

      SELECT * FROM t1 WHERE a='oe' COLLATE utf8_german2_ci;

    which will erroneously start to return both 'oe' and 'o-umlaut'.
    So changing "expr" to "const" is not possible if the effective
    collations of "target" and "source" are not exactly the same.

    Note, the code before the fix for MDEV-7152 only checked that
    collations of "source_const" and "target_value" are the same.
    This was not enough, as the bug report demonstrated.
  */
  return
    target->compare_collation() == source->compare_collation() &&
    target_value->collation.collation == source_const->collation.collation;
}


bool Type_handler_numeric::
       can_change_cond_ref_to_const(Item_bool_func2 *target,
                                    Item *target_expr, Item *target_value,
                                    Item_bool_func2 *source,
                                    Item *source_expr, Item *source_const)
                                    const
{
  /*
   The collations of "target" and "source" do not make sense for numeric
   data types.
  */
  return target->compare_type_handler() == source->compare_type_handler();
}


/*************************************************************************/

Item_cache *
Type_handler_row::Item_get_cache(THD *thd, const Item *item) const
{
  return new (thd->mem_root) Item_cache_row(thd);
}

Item_cache *
Type_handler_int_result::Item_get_cache(THD *thd, const Item *item) const
{
  return new (thd->mem_root) Item_cache_int(thd, item->type_handler());
}

Item_cache *
Type_handler_year::Item_get_cache(THD *thd, const Item *item) const
{
  return new (thd->mem_root) Item_cache_year(thd, item->type_handler());
}

Item_cache *
Type_handler_real_result::Item_get_cache(THD *thd, const Item *item) const
{
  return new (thd->mem_root) Item_cache_real(thd);
}

Item_cache *
Type_handler_decimal_result::Item_get_cache(THD *thd, const Item *item) const
{
  return new (thd->mem_root) Item_cache_decimal(thd);
}

Item_cache *
Type_handler_string_result::Item_get_cache(THD *thd, const Item *item) const
{
  return new (thd->mem_root) Item_cache_str(thd, item);
}

Item_cache *
Type_handler_timestamp_common::Item_get_cache(THD *thd, const Item *item) const
{
  return new (thd->mem_root) Item_cache_datetime(thd);
}

Item_cache *
Type_handler_datetime_common::Item_get_cache(THD *thd, const Item *item) const
{
  return new (thd->mem_root) Item_cache_datetime(thd);
}

Item_cache *
Type_handler_time_common::Item_get_cache(THD *thd, const Item *item) const
{
  return new (thd->mem_root) Item_cache_time(thd);
}

Item_cache *
Type_handler_date_common::Item_get_cache(THD *thd, const Item *item) const
{
  return new (thd->mem_root) Item_cache_date(thd);
}

/*************************************************************************/

bool Type_handler_int_result::
       Item_hybrid_func_fix_attributes(THD *thd,
                                       const char *func_name,
                                       Type_handler_hybrid_field_type *handler,
                                       Type_all_attributes *func,
                                       Item **items, uint nitems) const
{
  bool unsigned_flag= items[0]->unsigned_flag;
  for (uint i= 1; i < nitems; i++)
  {
    if (unsigned_flag != items[i]->unsigned_flag)
    {
      // Convert a mixture of signed and unsigned int to decimal
      handler->set_handler(&type_handler_newdecimal);
      func->aggregate_attributes_decimal(items, nitems);
      return false;
    }
  }
  func->aggregate_attributes_int(items, nitems);
  return false;
}


bool Type_handler_real_result::
       Item_hybrid_func_fix_attributes(THD *thd,
                                       const char *func_name,
                                       Type_handler_hybrid_field_type *handler,
                                       Type_all_attributes *func,
                                       Item **items, uint nitems) const
{
  func->aggregate_attributes_real(items, nitems);
  return false;
}


bool Type_handler_decimal_result::
       Item_hybrid_func_fix_attributes(THD *thd,
                                       const char *func_name,
                                       Type_handler_hybrid_field_type *handler,
                                       Type_all_attributes *func,
                                       Item **items, uint nitems) const
{
  func->aggregate_attributes_decimal(items, nitems);
  return false;
}


bool Type_handler_string_result::
       Item_hybrid_func_fix_attributes(THD *thd,
                                       const char *func_name,
                                       Type_handler_hybrid_field_type *handler,
                                       Type_all_attributes *func,
                                       Item **items, uint nitems) const
{
  return func->aggregate_attributes_string(func_name, items, nitems);
}



/*
  We can have enum/set type after merging only if we have one enum|set
  field (or MIN|MAX(enum|set field)) and number of NULL fields
*/
bool Type_handler_typelib::
       Item_hybrid_func_fix_attributes(THD *thd,
                                       const char *func_name,
                                       Type_handler_hybrid_field_type *handler,
                                       Type_all_attributes *func,
                                       Item **items, uint nitems) const
{
  TYPELIB *typelib= NULL;
  for (uint i= 0; i < nitems; i++)
  {
    TYPELIB *typelib2;
    if ((typelib2= items[i]->get_typelib()))
    {
      if (typelib)
      {
        /*
          Two ENUM/SET columns found. We convert such combinations to VARCHAR.
          This may change in the future to preserve ENUM/SET
          if typelib definitions are equal.
        */
        handler->set_handler(&type_handler_varchar);
        return func->aggregate_attributes_string(func_name, items, nitems);
      }
      typelib= typelib2;
    }
  }
  DBUG_ASSERT(typelib); // There must be at least one typelib
  func->set_typelib(typelib);
  return func->aggregate_attributes_string(func_name, items, nitems);
}


bool Type_handler_blob_common::
       Item_hybrid_func_fix_attributes(THD *thd,
                                       const char *func_name,
                                       Type_handler_hybrid_field_type *handler,
                                       Type_all_attributes *func,
                                       Item **items, uint nitems) const
{
  if (func->aggregate_attributes_string(func_name, items, nitems))
    return true;
  handler->set_handler(blob_type_handler(func->max_length));
  return false;
}


bool Type_handler_date_common::
       Item_hybrid_func_fix_attributes(THD *thd,
                                       const char *func_name,
                                       Type_handler_hybrid_field_type *handler,
                                       Type_all_attributes *func,
                                       Item **items, uint nitems) const
{
  func->fix_attributes_date();
  return false;
}


bool Type_handler_time_common::
       Item_hybrid_func_fix_attributes(THD *thd,
                                       const char *func_name,
                                       Type_handler_hybrid_field_type *handler,
                                       Type_all_attributes *func,
                                       Item **items, uint nitems) const
{
  func->aggregate_attributes_temporal(MIN_TIME_WIDTH, items, nitems);
  return false;
}


bool Type_handler_datetime_common::
       Item_hybrid_func_fix_attributes(THD *thd,
                                       const char *func_name,
                                       Type_handler_hybrid_field_type *handler,
                                       Type_all_attributes *func,
                                       Item **items, uint nitems) const
{
  func->aggregate_attributes_temporal(MAX_DATETIME_WIDTH, items, nitems);
  return false;
}


bool Type_handler_timestamp_common::
       Item_hybrid_func_fix_attributes(THD *thd,
                                       const char *func_name,
                                       Type_handler_hybrid_field_type *handler,
                                       Type_all_attributes *func,
                                       Item **items, uint nitems) const
{
  func->aggregate_attributes_temporal(MAX_DATETIME_WIDTH, items, nitems);
  return false;
}

#ifdef HAVE_SPATIAL
bool Type_handler_geometry::
       Item_hybrid_func_fix_attributes(THD *thd,
                                       const char *func_name,
                                       Type_handler_hybrid_field_type *handler,
                                       Type_all_attributes *func,
                                       Item **items, uint nitems) const
{
  DBUG_ASSERT(nitems > 0);
  Type_geometry_attributes gattr(items[0]->type_handler(), items[0]);
  for (uint i= 1; i < nitems; i++)
    gattr.join(items[i]);
  func->set_geometry_type(gattr.get_geometry_type());
  func->collation.set(&my_charset_bin);
  func->unsigned_flag= false;
  func->decimals= 0;
  func->max_length= (uint32) UINT_MAX32;
  func->set_maybe_null(true);
  return false;
}
#endif


/*************************************************************************/

bool Type_handler::
       Item_func_min_max_fix_attributes(THD *thd, Item_func_min_max *func,
                                        Item **items, uint nitems) const
{
  /*
    Aggregating attributes for LEAST/GREATES is exactly the same
    with aggregating for CASE-alike functions (e.g. COALESCE)
    for the majority of data type handlers.
  */
  return Item_hybrid_func_fix_attributes(thd, func->func_name(),
                                         func, func, items, nitems);
}


bool Type_handler_temporal_result::
       Item_func_min_max_fix_attributes(THD *thd, Item_func_min_max *func,
                                        Item **items, uint nitems) const
{
  bool rc= Type_handler::Item_func_min_max_fix_attributes(thd, func,
                                                          items, nitems);
  bool is_time= func->field_type() == MYSQL_TYPE_TIME;
  func->decimals= 0;
  for (uint i= 0; i < nitems; i++)
  {
    uint deci= is_time ? items[i]->time_precision(thd) :
                         items[i]->datetime_precision(thd);
    set_if_bigger(func->decimals, deci);
  }

  if (rc || func->maybe_null)
    return rc;
  /*
    LEAST/GREATES(non-temporal, temporal) can return NULL.
    CAST functions Item_{time|datetime|date}_typecast always set maybe_full
    to true. Here we try to detect nullability more thoroughly.
    Perhaps CAST functions should also reuse this idea eventually.
  */
  const Type_handler *hf= func->type_handler();
  for (uint i= 0; i < nitems; i++)
  {
    /*
      If items[i] does not need conversion to the current temporal data
      type, then we trust items[i]->maybe_null, which was already ORred
      to func->maybe_null in the argument loop in fix_fields().
      If items[i] requires conversion to the current temporal data type,
      then conversion can fail and return NULL even for NOT NULL items.
    */
    const Type_handler *ha= items[i]->type_handler();
    if (hf == ha)
      continue; // No conversion.
    if (ha->cmp_type() != TIME_RESULT)
    {
      func->maybe_null= true; // Conversion from non-temporal is not safe
      break;
    }
    timestamp_type tf= hf->mysql_timestamp_type();
    timestamp_type ta= ha->mysql_timestamp_type();
    if (tf == ta ||
        (tf == MYSQL_TIMESTAMP_DATETIME && ta == MYSQL_TIMESTAMP_DATE))
    {
      /*
        If handlers have the same mysql_timestamp_type(),
        then conversion is NULL safe. Conversion from DATE to DATETIME
        is also safe. This branch includes data type pairs:
        Function return type Argument type  Comment
        -------------------- -------------  -------------
        TIMESTAMP            TIMESTAMP      no conversion
        TIMESTAMP            DATETIME       not possible
        TIMESTAMP            DATE           not possible
        DATETIME             DATETIME       no conversion
        DATETIME             TIMESTAMP      safe conversion
        DATETIME             DATE           safe conversion
        DATE                 DATE           no conversion
        TIME                 TIME           no conversion

        Note, a function cannot return TIMESTAMP if it has non-TIMESTAMP
        arguments (it would return DATETIME in such case).
      */
      DBUG_ASSERT(hf->field_type() != MYSQL_TYPE_TIMESTAMP || tf == ta);
      continue;
    }
    /*
      Here we have the following data type pairs that did not match
      the condition above:

      Function return type Argument type Comment
      -------------------- ------------- -------
      TIMESTAMP            TIME          Not possible
      DATETIME             TIME          depends on OLD_MODE_ZERO_DATE_TIME_CAST
      DATE                 TIMESTAMP     Not possible
      DATE                 DATETIME      Not possible
      DATE                 TIME          Not possible
      TIME                 TIMESTAMP     Not possible
      TIME                 DATETIME      Not possible
      TIME                 DATE          Not possible

      Most pairs are not possible, because the function data type
      would be DATETIME (according to LEAST/GREATEST aggregation rules).
      Conversion to DATETIME from TIME is not safe when
      OLD_MODE_ZERO_DATE_TIME_CAST is set:
      - negative TIME values cannot be converted to not-NULL DATETIME values
      - TIME values can produce DATETIME values that do not pass
        NO_ZERO_DATE and NO_ZERO_IN_DATE tests.
    */
    DBUG_ASSERT(hf->field_type() == MYSQL_TYPE_DATETIME);
    if (!(thd->variables.old_behavior & OLD_MODE_ZERO_DATE_TIME_CAST))
      continue;
    func->maybe_null= true;
    break;
  }
  return rc;
}


bool Type_handler_real_result::
       Item_func_min_max_fix_attributes(THD *thd, Item_func_min_max *func,
                                        Item **items, uint nitems) const
{
  /*
    DOUBLE is an exception and aggregates attributes differently
    for LEAST/GREATEST vs CASE-alike functions. See the comment in
    Item_func_min_max::aggregate_attributes_real().
  */
  func->aggregate_attributes_real(items, nitems);
  return false;
}

/*************************************************************************/

/**
  MAX/MIN for the traditional numeric types preserve the exact data type
  from Fields, but do not preserve the exact type from Items:
    MAX(float_field)              -> FLOAT
    MAX(smallint_field)           -> LONGLONG
    MAX(COALESCE(float_field))    -> DOUBLE
    MAX(COALESCE(smallint_field)) -> LONGLONG
  QQ: Items should probably be fixed to preserve the exact type.
*/
bool Type_handler_numeric::
       Item_sum_hybrid_fix_length_and_dec_numeric(Item_sum_hybrid *func,
                                                  const Type_handler *handler)
                                                  const
{
  Item *item= func->arguments()[0];
  Item *item2= item->real_item();
  func->Type_std_attributes::set(item);
  /* MIN/MAX can return NULL for empty set indepedent of the used column */
  func->maybe_null= func->null_value= true;
  if (item2->type() == Item::FIELD_ITEM)
    func->set_handler(item2->type_handler());
  else
    func->set_handler(handler);
  return false;
}


bool Type_handler_int_result::
       Item_sum_hybrid_fix_length_and_dec(Item_sum_hybrid *func) const
{
  return Item_sum_hybrid_fix_length_and_dec_numeric(func,
                                                    &type_handler_longlong);
}


bool Type_handler_bool::
       Item_sum_hybrid_fix_length_and_dec(Item_sum_hybrid *func) const
{
  return Item_sum_hybrid_fix_length_and_dec_numeric(func, &type_handler_bool);
}


bool Type_handler_real_result::
       Item_sum_hybrid_fix_length_and_dec(Item_sum_hybrid *func) const
{
  (void) Item_sum_hybrid_fix_length_and_dec_numeric(func,
                                                    &type_handler_double);
  func->max_length= func->float_length(func->decimals);
  return false;
}


bool Type_handler_decimal_result::
       Item_sum_hybrid_fix_length_and_dec(Item_sum_hybrid *func) const
{
  return Item_sum_hybrid_fix_length_and_dec_numeric(func,
                                                    &type_handler_newdecimal);
}


/**
   MAX(str_field) converts ENUM/SET to CHAR, and preserve all other types
   for Fields.
   QQ: This works differently from UNION, which preserve the exact data
   type for ENUM/SET if the joined ENUM/SET fields are equally defined.
   Perhaps should be fixed.
   MAX(str_item) chooses the best suitable string type.
*/
bool Type_handler_string_result::
       Item_sum_hybrid_fix_length_and_dec(Item_sum_hybrid *func) const
{
  Item *item= func->arguments()[0];
  Item *item2= item->real_item();
  func->Type_std_attributes::set(item);
  func->maybe_null= func->null_value= true;
  if (item2->type() == Item::FIELD_ITEM)
  {
    // Fields: convert ENUM/SET to CHAR, preserve the type otherwise.
    func->set_handler(item->type_handler());
  }
  else
  {
    // Items: choose VARCHAR/BLOB/MEDIUMBLOB/LONGBLOB, depending on length.
    func->set_handler(type_handler_varchar.
          type_handler_adjusted_to_max_octet_length(func->max_length,
                                                    func->collation.collation));
  }
  return false;
}


/**
  Traditional temporal types always preserve the type of the argument.
*/
bool Type_handler_temporal_result::
       Item_sum_hybrid_fix_length_and_dec(Item_sum_hybrid *func) const
{
  Item *item= func->arguments()[0];
  func->Type_std_attributes::set(item);
  func->maybe_null= func->null_value= true;
  func->set_handler(item->type_handler());
  return false;
}


/*************************************************************************/

bool Type_handler_int_result::
       Item_sum_sum_fix_length_and_dec(Item_sum_sum *item) const
{
  item->fix_length_and_dec_decimal();
  return false;
}


bool Type_handler_decimal_result::
       Item_sum_sum_fix_length_and_dec(Item_sum_sum *item) const
{
  item->fix_length_and_dec_decimal();
  return false;
}


bool Type_handler_temporal_result::
       Item_sum_sum_fix_length_and_dec(Item_sum_sum *item) const
{
  item->fix_length_and_dec_decimal();
  return false;
}


bool Type_handler_real_result::
       Item_sum_sum_fix_length_and_dec(Item_sum_sum *item) const
{
  item->fix_length_and_dec_double();
  return false;
}


bool Type_handler_string_result::
       Item_sum_sum_fix_length_and_dec(Item_sum_sum *item) const
{
  item->fix_length_and_dec_double();
  return false;
}


#ifdef HAVE_SPATIAL
bool Type_handler_geometry::
       Item_sum_sum_fix_length_and_dec(Item_sum_sum *item) const
{
  return Item_func_or_sum_illegal_param("sum");
}
#endif


/*************************************************************************/

bool Type_handler_int_result::
       Item_sum_avg_fix_length_and_dec(Item_sum_avg *item) const
{
  item->fix_length_and_dec_decimal();
  return false;
}


bool Type_handler_decimal_result::
       Item_sum_avg_fix_length_and_dec(Item_sum_avg *item) const
{
  item->fix_length_and_dec_decimal();
  return false;
}


bool Type_handler_temporal_result::
       Item_sum_avg_fix_length_and_dec(Item_sum_avg *item) const
{
  item->fix_length_and_dec_decimal();
  return false;
}


bool Type_handler_real_result::
       Item_sum_avg_fix_length_and_dec(Item_sum_avg *item) const
{
  item->fix_length_and_dec_double();
  return false;
}


bool Type_handler_string_result::
       Item_sum_avg_fix_length_and_dec(Item_sum_avg *item) const
{
  item->fix_length_and_dec_double();
  return false;
}


#ifdef HAVE_SPATIAL
bool Type_handler_geometry::
       Item_sum_avg_fix_length_and_dec(Item_sum_avg *item) const
{
  return Item_func_or_sum_illegal_param("avg");
}
#endif


/*************************************************************************/

bool Type_handler_int_result::
       Item_sum_variance_fix_length_and_dec(Item_sum_variance *item) const
{
  item->fix_length_and_dec_decimal();
  return false;
}


bool Type_handler_decimal_result::
       Item_sum_variance_fix_length_and_dec(Item_sum_variance *item) const
{
  item->fix_length_and_dec_decimal();
  return false;
}


bool Type_handler_temporal_result::
       Item_sum_variance_fix_length_and_dec(Item_sum_variance *item) const
{
  item->fix_length_and_dec_decimal();
  return false;
}


bool Type_handler_real_result::
       Item_sum_variance_fix_length_and_dec(Item_sum_variance *item) const
{
  item->fix_length_and_dec_double();
  return false;
}


bool Type_handler_string_result::
       Item_sum_variance_fix_length_and_dec(Item_sum_variance *item) const
{
  item->fix_length_and_dec_double();
  return false;
}


#ifdef HAVE_SPATIAL
bool Type_handler_geometry::
       Item_sum_variance_fix_length_and_dec(Item_sum_variance *item) const
{
  return Item_func_or_sum_illegal_param(item);
}
#endif


/*************************************************************************/

bool Type_handler_real_result::Item_val_bool(Item *item) const
{
  return item->val_real() != 0.0;
}

bool Type_handler_int_result::Item_val_bool(Item *item) const
{
  return item->val_int() != 0;
}

bool Type_handler_temporal_result::Item_val_bool(Item *item) const
{
  return item->val_real() != 0.0;
}

bool Type_handler_string_result::Item_val_bool(Item *item) const
{
  return item->val_real() != 0.0;
}


/*************************************************************************/

bool Type_handler_int_result::Item_get_date(THD *thd, Item *item,
                                            MYSQL_TIME *ltime,
                                            date_mode_t fuzzydate) const
{
  return item->get_date_from_int(thd, ltime, fuzzydate);
}


bool Type_handler_year::Item_get_date(THD *thd, Item *item, MYSQL_TIME *ltime,
                                      date_mode_t fuzzydate) const
{
  return item->null_value=
    VYear(item).to_mysql_time_with_warn(thd, ltime, fuzzydate,
                                        item->field_name_or_null());
}


bool Type_handler_real_result::Item_get_date(THD *thd, Item *item,
                                             MYSQL_TIME *ltime,
                                             date_mode_t fuzzydate) const
{
  return item->get_date_from_real(thd, ltime, fuzzydate);
}


bool Type_handler_string_result::Item_get_date(THD *thd, Item *item,
                                               MYSQL_TIME *ltime,
                                               date_mode_t fuzzydate) const
{
  return item->get_date_from_string(thd, ltime, fuzzydate);
}


bool Type_handler_temporal_result::Item_get_date(THD *thd, Item *item,
                                                 MYSQL_TIME *ltime,
                                                 date_mode_t fuzzydate) const
{
  DBUG_ASSERT(0); // Temporal type items must implement native get_date()
  item->null_value= true;
  set_zero_time(ltime, mysql_timestamp_type());
  return true;
}


/*************************************************************************/

longlong Type_handler_real_result::
           Item_val_int_signed_typecast(Item *item) const
{
  return item->val_int();
}

longlong Type_handler_int_result::
           Item_val_int_signed_typecast(Item *item) const
{
  return item->val_int();
}

longlong Type_handler_decimal_result::
           Item_val_int_signed_typecast(Item *item) const
{
  return item->val_int();
}

longlong Type_handler_temporal_result::
           Item_val_int_signed_typecast(Item *item) const
{
  return item->val_int();
}

longlong Type_handler_string_result::
           Item_val_int_signed_typecast(Item *item) const
{
  return item->val_int_signed_typecast_from_str();
}

/*************************************************************************/

longlong Type_handler_real_result::
           Item_val_int_unsigned_typecast(Item *item) const
{
  return item->val_int_unsigned_typecast_from_int();
}

longlong Type_handler_int_result::
           Item_val_int_unsigned_typecast(Item *item) const
{
  return item->val_int_unsigned_typecast_from_int();
}

longlong Type_handler_temporal_result::
           Item_val_int_unsigned_typecast(Item *item) const
{
  return item->val_int_unsigned_typecast_from_int();
}

longlong Type_handler_string_result::
           Item_val_int_unsigned_typecast(Item *item) const
{
  return item->val_int_unsigned_typecast_from_str();
}

/*************************************************************************/

String *
Type_handler_real_result::Item_func_hex_val_str_ascii(Item_func_hex *item,
                                                      String *str) const
{
  return item->val_str_ascii_from_val_real(str);
}


String *
Type_handler_decimal_result::Item_func_hex_val_str_ascii(Item_func_hex *item,
                                                         String *str) const
{
  return item->val_str_ascii_from_val_real(str);
}


String *
Type_handler_int_result::Item_func_hex_val_str_ascii(Item_func_hex *item,
                                                     String *str) const
{
  return item->val_str_ascii_from_val_int(str);
}


String *
Type_handler_temporal_result::Item_func_hex_val_str_ascii(Item_func_hex *item,
                                                          String *str) const
{
  return item->val_str_ascii_from_val_str(str);
}


String *
Type_handler_string_result::Item_func_hex_val_str_ascii(Item_func_hex *item,
                                                        String *str) const
{
  return item->val_str_ascii_from_val_str(str);
}

/***************************************************************************/

String *
Type_handler_decimal_result::Item_func_hybrid_field_type_val_str(
                                              Item_func_hybrid_field_type *item,
                                              String *str) const
{
  return VDec_op(item).to_string_round(str, item->decimals);
}


double
Type_handler_decimal_result::Item_func_hybrid_field_type_val_real(
                                              Item_func_hybrid_field_type *item)
                                              const
{
  return VDec_op(item).to_double();
}


longlong
Type_handler_decimal_result::Item_func_hybrid_field_type_val_int(
                                              Item_func_hybrid_field_type *item)
                                              const
{
  return VDec_op(item).to_longlong(item->unsigned_flag);
}


my_decimal *
Type_handler_decimal_result::Item_func_hybrid_field_type_val_decimal(
                                              Item_func_hybrid_field_type *item,
                                              my_decimal *dec) const
{
  return VDec_op(item).to_decimal(dec);
}


bool
Type_handler_decimal_result::Item_func_hybrid_field_type_get_date(
                                             THD *thd,
                                             Item_func_hybrid_field_type *item,
                                             MYSQL_TIME *ltime,
                                             date_mode_t fuzzydate) const
{
  return VDec_op(item).to_datetime_with_warn(thd, ltime, fuzzydate, item);
}


bool
Type_handler_year::Item_func_hybrid_field_type_get_date(
                                             THD *thd,
                                             Item_func_hybrid_field_type *item,
                                             MYSQL_TIME *ltime,
                                             date_mode_t fuzzydate) const
{
  return item->null_value=
    VYear_op(item).to_mysql_time_with_warn(thd, ltime, fuzzydate, NULL);
}


/***************************************************************************/


String *
Type_handler_int_result::Item_func_hybrid_field_type_val_str(
                                          Item_func_hybrid_field_type *item,
                                          String *str) const
{
  return item->val_str_from_int_op(str);
}


double
Type_handler_int_result::Item_func_hybrid_field_type_val_real(
                                          Item_func_hybrid_field_type *item)
                                          const
{
  return item->val_real_from_int_op();
}


longlong
Type_handler_int_result::Item_func_hybrid_field_type_val_int(
                                          Item_func_hybrid_field_type *item)
                                          const
{
  return item->val_int_from_int_op();
}


my_decimal *
Type_handler_int_result::Item_func_hybrid_field_type_val_decimal(
                                          Item_func_hybrid_field_type *item,
                                          my_decimal *dec) const
{
  return item->val_decimal_from_int_op(dec);
}


bool
Type_handler_int_result::Item_func_hybrid_field_type_get_date(
                                          THD *thd,
                                          Item_func_hybrid_field_type *item,
                                          MYSQL_TIME *ltime,
                                          date_mode_t fuzzydate) const
{
  return item->get_date_from_int_op(thd, ltime, fuzzydate);
}



/***************************************************************************/

String *
Type_handler_real_result::Item_func_hybrid_field_type_val_str(
                                           Item_func_hybrid_field_type *item,
                                           String *str) const
{
  return item->val_str_from_real_op(str);
}


double
Type_handler_real_result::Item_func_hybrid_field_type_val_real(
                                           Item_func_hybrid_field_type *item)
                                           const
{
  return item->val_real_from_real_op();
}


longlong
Type_handler_real_result::Item_func_hybrid_field_type_val_int(
                                           Item_func_hybrid_field_type *item)
                                           const
{
  return item->val_int_from_real_op();
}


my_decimal *
Type_handler_real_result::Item_func_hybrid_field_type_val_decimal(
                                           Item_func_hybrid_field_type *item,
                                           my_decimal *dec) const
{
  return item->val_decimal_from_real_op(dec);
}


bool
Type_handler_real_result::Item_func_hybrid_field_type_get_date(
                                             THD *thd,
                                             Item_func_hybrid_field_type *item,
                                             MYSQL_TIME *ltime,
                                             date_mode_t fuzzydate) const
{
  return item->get_date_from_real_op(thd, ltime, fuzzydate);
}


/***************************************************************************/

String *
Type_handler_temporal_result::Item_func_hybrid_field_type_val_str(
                                        Item_func_hybrid_field_type *item,
                                        String *str) const
{
  return item->val_str_from_date_op(str);
}


double
Type_handler_temporal_result::Item_func_hybrid_field_type_val_real(
                                        Item_func_hybrid_field_type *item)
                                        const
{
  return item->val_real_from_date_op();
}


longlong
Type_handler_temporal_result::Item_func_hybrid_field_type_val_int(
                                        Item_func_hybrid_field_type *item)
                                        const
{
  return item->val_int_from_date_op();
}


my_decimal *
Type_handler_temporal_result::Item_func_hybrid_field_type_val_decimal(
                                        Item_func_hybrid_field_type *item,
                                        my_decimal *dec) const
{
  return item->val_decimal_from_date_op(dec);
}


bool
Type_handler_temporal_result::Item_func_hybrid_field_type_get_date(
                                        THD *thd,
                                        Item_func_hybrid_field_type *item,
                                        MYSQL_TIME *ltime,
                                        date_mode_t fuzzydate) const
{
  return item->date_op(thd, ltime, fuzzydate);
}


/***************************************************************************/

String *
Type_handler_time_common::Item_func_hybrid_field_type_val_str(
                                    Item_func_hybrid_field_type *item,
                                    String *str) const
{
  return item->val_str_from_time_op(str);
}


double
Type_handler_time_common::Item_func_hybrid_field_type_val_real(
                                    Item_func_hybrid_field_type *item)
                                    const
{
  return item->val_real_from_time_op();
}


longlong
Type_handler_time_common::Item_func_hybrid_field_type_val_int(
                                    Item_func_hybrid_field_type *item)
                                    const
{
  return item->val_int_from_time_op();
}


my_decimal *
Type_handler_time_common::Item_func_hybrid_field_type_val_decimal(
                                    Item_func_hybrid_field_type *item,
                                    my_decimal *dec) const
{
  return item->val_decimal_from_time_op(dec);
}


bool
Type_handler_time_common::Item_func_hybrid_field_type_get_date(
                                    THD *thd,
                                    Item_func_hybrid_field_type *item,
                                    MYSQL_TIME *ltime,
                                    date_mode_t fuzzydate) const
{
  return item->time_op(thd, ltime);
}


/***************************************************************************/

String *
Type_handler_string_result::Item_func_hybrid_field_type_val_str(
                                             Item_func_hybrid_field_type *item,
                                             String *str) const
{
  return item->val_str_from_str_op(str);
}


double
Type_handler_string_result::Item_func_hybrid_field_type_val_real(
                                             Item_func_hybrid_field_type *item)
                                             const
{
  return item->val_real_from_str_op();
}


longlong
Type_handler_string_result::Item_func_hybrid_field_type_val_int(
                                             Item_func_hybrid_field_type *item)
                                             const
{
  return item->val_int_from_str_op();
}


my_decimal *
Type_handler_string_result::Item_func_hybrid_field_type_val_decimal(
                                              Item_func_hybrid_field_type *item,
                                              my_decimal *dec) const
{
  return item->val_decimal_from_str_op(dec);
}


bool
Type_handler_string_result::Item_func_hybrid_field_type_get_date(
                                             THD *thd,
                                             Item_func_hybrid_field_type *item,
                                             MYSQL_TIME *ltime,
                                             date_mode_t fuzzydate) const
{
  return item->get_date_from_str_op(thd, ltime, fuzzydate);
}

/***************************************************************************/

bool Type_handler_numeric::
       Item_func_between_fix_length_and_dec(Item_func_between *func) const
{
  return func->fix_length_and_dec_numeric(current_thd);
}

bool Type_handler_temporal_result::
       Item_func_between_fix_length_and_dec(Item_func_between *func) const
{
  return func->fix_length_and_dec_temporal(current_thd);
}

bool Type_handler_string_result::
       Item_func_between_fix_length_and_dec(Item_func_between *func) const
{
  return func->fix_length_and_dec_string(current_thd);
}


longlong Type_handler_row::
           Item_func_between_val_int(Item_func_between *func) const
{
  DBUG_ASSERT(0);
  func->null_value= true;
  return 0;
}

longlong Type_handler_string_result::
           Item_func_between_val_int(Item_func_between *func) const
{
  return func->val_int_cmp_string();
}

longlong Type_handler_temporal_with_date::
           Item_func_between_val_int(Item_func_between *func) const
{
  return func->val_int_cmp_datetime();
}

longlong Type_handler_time_common::
           Item_func_between_val_int(Item_func_between *func) const
{
  return func->val_int_cmp_time();
}

longlong Type_handler_int_result::
           Item_func_between_val_int(Item_func_between *func) const
{
  return func->val_int_cmp_int();
}

longlong Type_handler_real_result::
           Item_func_between_val_int(Item_func_between *func) const
{
  return func->val_int_cmp_real();
}

longlong Type_handler_decimal_result::
           Item_func_between_val_int(Item_func_between *func) const
{
  return func->val_int_cmp_decimal();
}

/***************************************************************************/

cmp_item *Type_handler_int_result::make_cmp_item(THD *thd,
                                                 CHARSET_INFO *cs) const
{
  return new (thd->mem_root) cmp_item_int;
}

cmp_item *Type_handler_real_result::make_cmp_item(THD *thd,
                                                 CHARSET_INFO *cs) const
{
  return new (thd->mem_root) cmp_item_real;
}

cmp_item *Type_handler_decimal_result::make_cmp_item(THD *thd,
                                                     CHARSET_INFO *cs) const
{
  return new (thd->mem_root) cmp_item_decimal;
}


cmp_item *Type_handler_string_result::make_cmp_item(THD *thd,
                                                    CHARSET_INFO *cs) const
{
  return new (thd->mem_root) cmp_item_sort_string(cs);
}

cmp_item *Type_handler_row::make_cmp_item(THD *thd,
                                                    CHARSET_INFO *cs) const
{
  return new (thd->mem_root) cmp_item_row;
}

cmp_item *Type_handler_time_common::make_cmp_item(THD *thd,
                                                    CHARSET_INFO *cs) const
{
  return new (thd->mem_root) cmp_item_time;
}

cmp_item *Type_handler_temporal_with_date::make_cmp_item(THD *thd,
                                                    CHARSET_INFO *cs) const
{
  return new (thd->mem_root) cmp_item_datetime;
}

/***************************************************************************/

static int srtcmp_in(CHARSET_INFO *cs, const String *x,const String *y)
{
  return cs->coll->strnncollsp(cs,
                               (uchar *) x->ptr(),x->length(),
                               (uchar *) y->ptr(),y->length());
}

in_vector *Type_handler_string_result::make_in_vector(THD *thd,
                                                      const Item_func_in *func,
                                                      uint nargs) const
{
  return new (thd->mem_root) in_string(thd, nargs, (qsort2_cmp) srtcmp_in,
                                       func->compare_collation());

}


in_vector *Type_handler_int_result::make_in_vector(THD *thd,
                                                   const Item_func_in *func,
                                                   uint nargs) const
{
  return new (thd->mem_root) in_longlong(thd, nargs);
}


in_vector *Type_handler_real_result::make_in_vector(THD *thd,
                                                    const Item_func_in *func,
                                                    uint nargs) const
{
  return new (thd->mem_root) in_double(thd, nargs);
}


in_vector *Type_handler_decimal_result::make_in_vector(THD *thd,
                                                       const Item_func_in *func,
                                                       uint nargs) const
{
  return new (thd->mem_root) in_decimal(thd, nargs);
}


in_vector *Type_handler_time_common::make_in_vector(THD *thd,
                                                    const Item_func_in *func,
                                                    uint nargs) const
{
  return new (thd->mem_root) in_time(thd, nargs);
}


in_vector *
Type_handler_temporal_with_date::make_in_vector(THD *thd,
                                                const Item_func_in *func,
                                                uint nargs) const
{
  return new (thd->mem_root) in_datetime(thd, nargs);
}


in_vector *Type_handler_row::make_in_vector(THD *thd,
                                            const Item_func_in *func,
                                            uint nargs) const
{
  return new (thd->mem_root) in_row(thd, nargs, 0);
}

/***************************************************************************/

bool Type_handler_string_result::
       Item_func_in_fix_comparator_compatible_types(THD *thd,
                                                    Item_func_in *func) const
{
  if (func->agg_all_arg_charsets_for_comparison())
    return true;
  if (func->compatible_types_scalar_bisection_possible())
  {
    return func->value_list_convert_const_to_int(thd) ||
           func->fix_for_scalar_comparison_using_bisection(thd);
  }
  return
    func->fix_for_scalar_comparison_using_cmp_items(thd,
                                                    1U << (uint) STRING_RESULT);
}


bool Type_handler_int_result::
       Item_func_in_fix_comparator_compatible_types(THD *thd,
                                                    Item_func_in *func) const
{
  /*
     Does not need to call value_list_convert_const_to_int()
     as already handled by int handler.
  */
  return func->compatible_types_scalar_bisection_possible() ?
    func->fix_for_scalar_comparison_using_bisection(thd) :
    func->fix_for_scalar_comparison_using_cmp_items(thd,
                                                    1U << (uint) INT_RESULT);
}


bool Type_handler_real_result::
       Item_func_in_fix_comparator_compatible_types(THD *thd,
                                                    Item_func_in *func) const
{
  return func->compatible_types_scalar_bisection_possible() ?
    (func->value_list_convert_const_to_int(thd) ||
     func->fix_for_scalar_comparison_using_bisection(thd)) :
    func->fix_for_scalar_comparison_using_cmp_items(thd,
                                                    1U << (uint) REAL_RESULT);
}


bool Type_handler_decimal_result::
       Item_func_in_fix_comparator_compatible_types(THD *thd,
                                                    Item_func_in *func) const
{
  return func->compatible_types_scalar_bisection_possible() ?
    (func->value_list_convert_const_to_int(thd) ||
     func->fix_for_scalar_comparison_using_bisection(thd)) :
    func->fix_for_scalar_comparison_using_cmp_items(thd,
                                                    1U << (uint) DECIMAL_RESULT);
}


bool Type_handler_temporal_result::
       Item_func_in_fix_comparator_compatible_types(THD *thd,
                                                    Item_func_in *func) const
{
  return func->compatible_types_scalar_bisection_possible() ?
    (func->value_list_convert_const_to_int(thd) ||
     func->fix_for_scalar_comparison_using_bisection(thd)) :
    func->fix_for_scalar_comparison_using_cmp_items(thd,
                                                    1U << (uint) TIME_RESULT);
}


bool Type_handler_row::Item_func_in_fix_comparator_compatible_types(THD *thd,
                                              Item_func_in *func) const
{
  return func->compatible_types_row_bisection_possible() ?
         func->fix_for_row_comparison_using_bisection(thd) :
         func->fix_for_row_comparison_using_cmp_items(thd);
}

/***************************************************************************/

String *Type_handler_string_result::
          Item_func_min_max_val_str(Item_func_min_max *func, String *str) const
{
  return func->val_str_native(str);
}


String *Type_handler_time_common::
          Item_func_min_max_val_str(Item_func_min_max *func, String *str) const
{
  return Time(func).to_string(str, func->decimals);
}


String *Type_handler_date_common::
          Item_func_min_max_val_str(Item_func_min_max *func, String *str) const
{
  return Date(func).to_string(str);
}


String *Type_handler_datetime_common::
          Item_func_min_max_val_str(Item_func_min_max *func, String *str) const
{
  return Datetime(func).to_string(str, func->decimals);
}


String *Type_handler_timestamp_common::
          Item_func_min_max_val_str(Item_func_min_max *func, String *str) const
{
  return Datetime(func).to_string(str, func->decimals);
}


String *Type_handler_int_result::
          Item_func_min_max_val_str(Item_func_min_max *func, String *str) const
{
  return func->val_string_from_int(str);
}


String *Type_handler_decimal_result::
          Item_func_min_max_val_str(Item_func_min_max *func, String *str) const
{
  return VDec(func).to_string_round(str, func->decimals);
}


String *Type_handler_real_result::
          Item_func_min_max_val_str(Item_func_min_max *func, String *str) const
{
  return func->val_string_from_real(str);
}


double Type_handler_string_result::
         Item_func_min_max_val_real(Item_func_min_max *func) const
{
  return func->val_real_native();
}


double Type_handler_time_common::
         Item_func_min_max_val_real(Item_func_min_max *func) const
{
  return Time(current_thd, func).to_double();
}


double Type_handler_date_common::
         Item_func_min_max_val_real(Item_func_min_max *func) const
{
  return Date(current_thd, func).to_double();
}


double Type_handler_datetime_common::
         Item_func_min_max_val_real(Item_func_min_max *func) const
{
  return Datetime(current_thd, func).to_double();
}

double Type_handler_timestamp_common::
         Item_func_min_max_val_real(Item_func_min_max *func) const
{
  return Datetime(current_thd, func).to_double();
}


double Type_handler_numeric::
         Item_func_min_max_val_real(Item_func_min_max *func) const
{
  return func->val_real_native();
}


longlong Type_handler_string_result::
         Item_func_min_max_val_int(Item_func_min_max *func) const
{
  return func->val_int_native();
}


longlong Type_handler_time_common::
         Item_func_min_max_val_int(Item_func_min_max *func) const
{
  return Time(current_thd, func).to_longlong();
}


longlong Type_handler_date_common::
         Item_func_min_max_val_int(Item_func_min_max *func) const
{
  return Date(current_thd, func).to_longlong();
}


longlong Type_handler_datetime_common::
         Item_func_min_max_val_int(Item_func_min_max *func) const
{
  return Datetime(current_thd, func).to_longlong();
}


longlong Type_handler_timestamp_common::
         Item_func_min_max_val_int(Item_func_min_max *func) const
{
  return Datetime(current_thd, func).to_longlong();
}


longlong Type_handler_numeric::
         Item_func_min_max_val_int(Item_func_min_max *func) const
{
  return func->val_int_native();
}


my_decimal *Type_handler_string_result::
            Item_func_min_max_val_decimal(Item_func_min_max *func,
                                          my_decimal *dec) const
{
  return func->val_decimal_native(dec);
}


my_decimal *Type_handler_numeric::
            Item_func_min_max_val_decimal(Item_func_min_max *func,
                                          my_decimal *dec) const
{
  return func->val_decimal_native(dec);
}


my_decimal *Type_handler_time_common::
            Item_func_min_max_val_decimal(Item_func_min_max *func,
                                          my_decimal *dec) const
{
  return Time(current_thd, func).to_decimal(dec);
}


my_decimal *Type_handler_date_common::
            Item_func_min_max_val_decimal(Item_func_min_max *func,
                                          my_decimal *dec) const
{
  return Date(current_thd, func).to_decimal(dec);
}


my_decimal *Type_handler_datetime_common::
            Item_func_min_max_val_decimal(Item_func_min_max *func,
                                          my_decimal *dec) const
{
  return Datetime(current_thd, func).to_decimal(dec);
}


my_decimal *Type_handler_timestamp_common::
            Item_func_min_max_val_decimal(Item_func_min_max *func,
                                          my_decimal *dec) const
{
  return Datetime(current_thd, func).to_decimal(dec);
}


bool Type_handler_string_result::
       Item_func_min_max_get_date(THD *thd, Item_func_min_max *func,
                                  MYSQL_TIME *ltime, date_mode_t fuzzydate) const
{
  /*
    just like ::val_int() method of a string item can be called,
    for example, SELECT CONCAT("10", "12") + 1,
    ::get_date() can be called for non-temporal values,
    for example, SELECT MONTH(GREATEST("2011-11-21", "2010-10-09"))
  */
  return func->get_date_from_string(thd, ltime, fuzzydate);
}


bool Type_handler_numeric::
       Item_func_min_max_get_date(THD *thd, Item_func_min_max *func,
                                  MYSQL_TIME *ltime, date_mode_t fuzzydate) const
{
  return Item_get_date(thd, func, ltime, fuzzydate);
}


bool Type_handler_temporal_result::
       Item_func_min_max_get_date(THD *thd, Item_func_min_max *func,
                                  MYSQL_TIME *ltime, date_mode_t fuzzydate) const
{
  /*
    - If the caller specified TIME_TIME_ONLY, then it's going to convert
      a DATETIME or DATE to TIME. So we pass the default flags for date. This is
      exactly the same with what Item_func_min_max_val_{int|real|decimal|str} or
      Item_send_datetime() do. We return the value in accordance with the
      current session date flags and let the caller further convert it to TIME.
    - If the caller did not specify TIME_TIME_ONLY, then return the value
      according to the flags supplied by the caller.
  */
  return func->get_date_native(thd, ltime,
                               fuzzydate & TIME_TIME_ONLY ?
                               sql_mode_for_dates(thd) :
                               fuzzydate);
}

bool Type_handler_time_common::
       Item_func_min_max_get_date(THD *thd, Item_func_min_max *func,
                                  MYSQL_TIME *ltime, date_mode_t fuzzydate) const
{
  return func->get_time_native(thd, ltime);
}

/***************************************************************************/

/**
  Get a string representation of the Item value.
  See sql_type.h for details.
*/
String *Type_handler_row::
          print_item_value(THD *thd, Item *item, String *str) const
{
  CHARSET_INFO *cs= thd->variables.character_set_client;
  StringBuffer<STRING_BUFFER_USUAL_SIZE> val(cs);
  str->append(STRING_WITH_LEN("ROW("));
  for (uint i= 0 ; i < item->cols(); i++)
  {
    if (i > 0)
      str->append(',');
    Item *elem= item->element_index(i);
    String *tmp= elem->type_handler()->print_item_value(thd, elem, &val);
    if (tmp)
      str->append(*tmp);
    else
      str->append(STRING_WITH_LEN("NULL"));
  }
  str->append(STRING_WITH_LEN(")"));
  return str;
}


/**
  Get a string representation of the Item value,
  using the character string format with its charset and collation, e.g.
    latin1 'string' COLLATE latin1_german2_ci
*/
String *Type_handler::
          print_item_value_csstr(THD *thd, Item *item, String *str) const
{
  String *result= item->val_str(str);

  if (!result)
    return NULL;

  StringBuffer<STRING_BUFFER_USUAL_SIZE> buf(result->charset());
  CHARSET_INFO *cs= thd->variables.character_set_client;

  buf.append('_');
  buf.append(result->charset()->csname);
  if (cs->escape_with_backslash_is_dangerous)
    buf.append(' ');
  append_query_string(cs, &buf, result->ptr(), result->length(),
                     thd->variables.sql_mode & MODE_NO_BACKSLASH_ESCAPES);
  buf.append(" COLLATE '");
  buf.append(item->collation.collation->name);
  buf.append('\'');
  str->copy(buf);

  return str;
}


String *Type_handler_numeric::
          print_item_value(THD *thd, Item *item, String *str) const
{
  return item->val_str(str);
}


String *Type_handler::
          print_item_value_temporal(THD *thd, Item *item, String *str,
                                    const Name &type_name, String *buf) const
{
  String *result= item->val_str(buf);
  return !result ||
         str->realloc(type_name.length() + result->length() + 2) ||
         str->copy(type_name.ptr(), type_name.length(), &my_charset_latin1) ||
         str->append('\'') ||
         str->append(result->ptr(), result->length()) ||
         str->append('\'') ?
         NULL :
         str;
}


String *Type_handler_time_common::
          print_item_value(THD *thd, Item *item, String *str) const
{
  StringBuffer<MAX_TIME_FULL_WIDTH+1> buf;
  return print_item_value_temporal(thd, item, str,
                                   Name(STRING_WITH_LEN("TIME")), &buf);
}


String *Type_handler_date_common::
          print_item_value(THD *thd, Item *item, String *str) const
{
  StringBuffer<MAX_DATE_WIDTH+1> buf;
  return print_item_value_temporal(thd, item, str,
                                   Name(STRING_WITH_LEN("DATE")), &buf);
}


String *Type_handler_datetime_common::
          print_item_value(THD *thd, Item *item, String *str) const
{
  StringBuffer<MAX_DATETIME_FULL_WIDTH+1> buf;
  return print_item_value_temporal(thd, item, str,
                                   Name(STRING_WITH_LEN("TIMESTAMP")), &buf);
}


String *Type_handler_timestamp_common::
          print_item_value(THD *thd, Item *item, String *str) const
{
  StringBuffer<MAX_DATETIME_FULL_WIDTH+1> buf;
  return print_item_value_temporal(thd, item, str,
                                   Name(STRING_WITH_LEN("TIMESTAMP")), &buf);
}


/***************************************************************************/

bool Type_handler_row::
       Item_func_round_fix_length_and_dec(Item_func_round *item) const
{
  DBUG_ASSERT(0);
  return false;
}


bool Type_handler_int_result::
       Item_func_round_fix_length_and_dec(Item_func_round *item) const
{
  item->fix_arg_int();
  return false;
}


bool Type_handler_real_result::
       Item_func_round_fix_length_and_dec(Item_func_round *item) const
{
  item->fix_arg_double();
  return false;
}


bool Type_handler_decimal_result::
       Item_func_round_fix_length_and_dec(Item_func_round *item) const
{
  item->fix_arg_decimal();
  return false;
}


bool Type_handler_temporal_result::
       Item_func_round_fix_length_and_dec(Item_func_round *item) const
{
  item->fix_arg_double();
  return false;
}


bool Type_handler_string_result::
       Item_func_round_fix_length_and_dec(Item_func_round *item) const
{
  item->fix_arg_double();
  return false;
}


#ifdef HAVE_SPATIAL
bool Type_handler_geometry::
       Item_func_round_fix_length_and_dec(Item_func_round *item) const
{
  return Item_func_or_sum_illegal_param(item);
}
#endif

/***************************************************************************/

bool Type_handler_row::
       Item_func_int_val_fix_length_and_dec(Item_func_int_val *item) const
{
  DBUG_ASSERT(0);
  return false;
}


bool Type_handler_int_result::
       Item_func_int_val_fix_length_and_dec(Item_func_int_val *item) const
{
  item->fix_length_and_dec_int_or_decimal();
  return false;
}


bool Type_handler_real_result::
       Item_func_int_val_fix_length_and_dec(Item_func_int_val *item) const
{
  item->fix_length_and_dec_double();
  return false;
}


bool Type_handler_decimal_result::
       Item_func_int_val_fix_length_and_dec(Item_func_int_val *item) const
{
  item->fix_length_and_dec_int_or_decimal();
  return false;
}


bool Type_handler_temporal_result::
       Item_func_int_val_fix_length_and_dec(Item_func_int_val *item) const
{
  item->fix_length_and_dec_int_or_decimal();
  return false;
}


bool Type_handler_string_result::
       Item_func_int_val_fix_length_and_dec(Item_func_int_val *item) const
{
  item->fix_length_and_dec_double();
  return false;
}


#ifdef HAVE_SPATIAL
bool Type_handler_geometry::
       Item_func_int_val_fix_length_and_dec(Item_func_int_val *item) const
{
  return Item_func_or_sum_illegal_param(item);
}
#endif

/***************************************************************************/

bool Type_handler_row::
       Item_func_abs_fix_length_and_dec(Item_func_abs *item) const
{
  DBUG_ASSERT(0);
  return false;
}


bool Type_handler_int_result::
       Item_func_abs_fix_length_and_dec(Item_func_abs *item) const
{
  item->fix_length_and_dec_int();
  return false;
}


bool Type_handler_real_result::
       Item_func_abs_fix_length_and_dec(Item_func_abs *item) const
{
  item->fix_length_and_dec_double();
  return false;
}


bool Type_handler_decimal_result::
       Item_func_abs_fix_length_and_dec(Item_func_abs *item) const
{
  item->fix_length_and_dec_decimal();
  return false;
}


bool Type_handler_temporal_result::
       Item_func_abs_fix_length_and_dec(Item_func_abs *item) const
{
  item->fix_length_and_dec_decimal();
  return false;
}


bool Type_handler_string_result::
       Item_func_abs_fix_length_and_dec(Item_func_abs *item) const
{
  item->fix_length_and_dec_double();
  return false;
}


#ifdef HAVE_SPATIAL
bool Type_handler_geometry::
       Item_func_abs_fix_length_and_dec(Item_func_abs *item) const
{
  return Item_func_or_sum_illegal_param(item);
}
#endif

/***************************************************************************/

bool Type_handler_row::
       Item_func_neg_fix_length_and_dec(Item_func_neg *item) const
{
  DBUG_ASSERT(0);
  return false;
}


bool Type_handler_int_result::
       Item_func_neg_fix_length_and_dec(Item_func_neg *item) const
{
  item->fix_length_and_dec_int();
  return false;
}


bool Type_handler_real_result::
       Item_func_neg_fix_length_and_dec(Item_func_neg *item) const
{
  item->fix_length_and_dec_double();
  return false;
}


bool Type_handler_decimal_result::
       Item_func_neg_fix_length_and_dec(Item_func_neg *item) const
{
  item->fix_length_and_dec_decimal();
  return false;
}


bool Type_handler_temporal_result::
       Item_func_neg_fix_length_and_dec(Item_func_neg *item) const
{
  item->fix_length_and_dec_decimal();
  return false;
}


bool Type_handler_string_result::
       Item_func_neg_fix_length_and_dec(Item_func_neg *item) const
{
  item->fix_length_and_dec_double();
  return false;
}


#ifdef HAVE_SPATIAL
bool Type_handler_geometry::
       Item_func_neg_fix_length_and_dec(Item_func_neg *item) const
{
  return Item_func_or_sum_illegal_param(item);
}
#endif


/***************************************************************************/

bool Type_handler::
       Item_func_signed_fix_length_and_dec(Item_func_signed *item) const
{
  item->fix_length_and_dec_generic();
  return false;
}


bool Type_handler::
       Item_func_unsigned_fix_length_and_dec(Item_func_unsigned *item) const
{
  const Item *arg= item->arguments()[0];
  if (!arg->unsigned_flag && arg->val_int_min() < 0)
  {
    /*
      Negative arguments produce long results:
        CAST(1-2 AS UNSIGNED) -> 18446744073709551615
    */
    item->max_length= MAX_BIGINT_WIDTH;
    return false;
  }
  item->fix_length_and_dec_generic();
  return false;
}


bool Type_handler_string_result::
       Item_func_signed_fix_length_and_dec(Item_func_signed *item) const
{
  item->fix_length_and_dec_string();
  return false;
}


bool Type_handler_string_result::
       Item_func_unsigned_fix_length_and_dec(Item_func_unsigned *item) const
{
  const Item *arg= item->arguments()[0];
  if (!arg->unsigned_flag &&       // Not HEX hybrid
      arg->max_char_length() > 1)  // Can be negative
  {
    // String arguments can give long results: '-1' -> 18446744073709551614
    item->max_length= MAX_BIGINT_WIDTH;
    return false;
  }
  item->fix_length_and_dec_string();
  return false;
}

bool Type_handler_real_result::
       Item_func_signed_fix_length_and_dec(Item_func_signed *item) const
{
  item->fix_length_and_dec_double();
  return false;
}


bool Type_handler_real_result::
       Item_func_unsigned_fix_length_and_dec(Item_func_unsigned *item) const
{
  item->fix_length_and_dec_double();
  return false;
}


bool Type_handler::
       Item_double_typecast_fix_length_and_dec(Item_double_typecast *item) const
{
  item->fix_length_and_dec_generic();
  return false;
}


bool Type_handler::
       Item_decimal_typecast_fix_length_and_dec(Item_decimal_typecast *item) const
{
  item->fix_length_and_dec_generic();
  return false;
}


bool Type_handler::
       Item_char_typecast_fix_length_and_dec(Item_char_typecast *item) const
{
  item->fix_length_and_dec_generic();
  return false;
}


bool Type_handler_numeric::
       Item_char_typecast_fix_length_and_dec(Item_char_typecast *item) const
{
  item->fix_length_and_dec_numeric();
  return false;
}


bool Type_handler_string_result::
       Item_char_typecast_fix_length_and_dec(Item_char_typecast *item) const
{
  item->fix_length_and_dec_str();
  return false;
}


bool Type_handler::
       Item_time_typecast_fix_length_and_dec(Item_time_typecast *item) const
{
  uint dec= item->decimals == NOT_FIXED_DEC ?
            item->arguments()[0]->time_precision(current_thd) :
            item->decimals;
  item->fix_attributes_temporal(MIN_TIME_WIDTH, dec);
  item->maybe_null= true;
  return false;
}


bool Type_handler::
       Item_date_typecast_fix_length_and_dec(Item_date_typecast *item) const
{
  item->fix_attributes_temporal(MAX_DATE_WIDTH, 0);
  item->maybe_null= true;
  return false;
}


bool Type_handler::
       Item_datetime_typecast_fix_length_and_dec(Item_datetime_typecast *item)
                                                 const
{
  uint dec= item->decimals == NOT_FIXED_DEC ?
            item->arguments()[0]->datetime_precision(current_thd) :
            item->decimals;
  item->fix_attributes_temporal(MAX_DATETIME_WIDTH, dec);
  item->maybe_null= true;
  return false;
}


#ifdef HAVE_SPATIAL

bool Type_handler_geometry::
       Item_func_signed_fix_length_and_dec(Item_func_signed *item) const
{
  return Item_func_or_sum_illegal_param(item);
}


bool Type_handler_geometry::
       Item_func_unsigned_fix_length_and_dec(Item_func_unsigned *item) const
{
  return Item_func_or_sum_illegal_param(item);
}


bool Type_handler_geometry::
       Item_double_typecast_fix_length_and_dec(Item_double_typecast *item) const
{
  return Item_func_or_sum_illegal_param(item);
}


bool Type_handler_geometry::
       Item_decimal_typecast_fix_length_and_dec(Item_decimal_typecast *item) const
{
  return Item_func_or_sum_illegal_param(item);
}


bool Type_handler_geometry::
       Item_char_typecast_fix_length_and_dec(Item_char_typecast *item) const
{
  if (item->cast_charset() != &my_charset_bin)
    return Item_func_or_sum_illegal_param(item); // CAST(geom AS CHAR)
  item->fix_length_and_dec_str();
  return false; // CAST(geom AS BINARY)
}


bool Type_handler_geometry::
       Item_time_typecast_fix_length_and_dec(Item_time_typecast *item) const
{
  return Item_func_or_sum_illegal_param(item);
}



bool Type_handler_geometry::
       Item_date_typecast_fix_length_and_dec(Item_date_typecast *item) const
{
  return Item_func_or_sum_illegal_param(item);
}


bool Type_handler_geometry::
       Item_datetime_typecast_fix_length_and_dec(Item_datetime_typecast *item)
                                                 const
{
  return Item_func_or_sum_illegal_param(item);

}

#endif /* HAVE_SPATIAL */

/***************************************************************************/

bool Type_handler_row::
       Item_func_plus_fix_length_and_dec(Item_func_plus *item) const
{
  DBUG_ASSERT(0);
  return true;
}


bool Type_handler_int_result::
       Item_func_plus_fix_length_and_dec(Item_func_plus *item) const
{
  item->fix_length_and_dec_int();
  return false;
}


bool Type_handler_real_result::
       Item_func_plus_fix_length_and_dec(Item_func_plus *item) const
{
  item->fix_length_and_dec_double();
  return false;
}


bool Type_handler_decimal_result::
       Item_func_plus_fix_length_and_dec(Item_func_plus *item) const
{
  item->fix_length_and_dec_decimal();
  return false;
}


bool Type_handler_temporal_result::
       Item_func_plus_fix_length_and_dec(Item_func_plus *item) const
{
  item->fix_length_and_dec_temporal();
  return false;
}


bool Type_handler_string_result::
       Item_func_plus_fix_length_and_dec(Item_func_plus *item) const
{
  item->fix_length_and_dec_double();
  return false;
}

/***************************************************************************/

bool Type_handler_row::
       Item_func_minus_fix_length_and_dec(Item_func_minus *item) const
{
  DBUG_ASSERT(0);
  return true;
}


bool Type_handler_int_result::
       Item_func_minus_fix_length_and_dec(Item_func_minus *item) const
{
  item->fix_length_and_dec_int();
  return false;
}


bool Type_handler_real_result::
       Item_func_minus_fix_length_and_dec(Item_func_minus *item) const
{
  item->fix_length_and_dec_double();
  return false;
}


bool Type_handler_decimal_result::
       Item_func_minus_fix_length_and_dec(Item_func_minus *item) const
{
  item->fix_length_and_dec_decimal();
  return false;
}


bool Type_handler_temporal_result::
       Item_func_minus_fix_length_and_dec(Item_func_minus *item) const
{
  item->fix_length_and_dec_temporal();
  return false;
}


bool Type_handler_string_result::
       Item_func_minus_fix_length_and_dec(Item_func_minus *item) const
{
  item->fix_length_and_dec_double();
  return false;
}

/***************************************************************************/

bool Type_handler_row::
       Item_func_mul_fix_length_and_dec(Item_func_mul *item) const
{
  DBUG_ASSERT(0);
  return true;
}


bool Type_handler_int_result::
       Item_func_mul_fix_length_and_dec(Item_func_mul *item) const
{
  item->fix_length_and_dec_int();
  return false;
}


bool Type_handler_real_result::
       Item_func_mul_fix_length_and_dec(Item_func_mul *item) const
{
  item->fix_length_and_dec_double();
  return false;
}


bool Type_handler_decimal_result::
       Item_func_mul_fix_length_and_dec(Item_func_mul *item) const
{
  item->fix_length_and_dec_decimal();
  return false;
}


bool Type_handler_temporal_result::
       Item_func_mul_fix_length_and_dec(Item_func_mul *item) const
{
  item->fix_length_and_dec_temporal();
  return false;
}


bool Type_handler_string_result::
       Item_func_mul_fix_length_and_dec(Item_func_mul *item) const
{
  item->fix_length_and_dec_double();
  return false;
}

/***************************************************************************/

bool Type_handler_row::
       Item_func_div_fix_length_and_dec(Item_func_div *item) const
{
  DBUG_ASSERT(0);
  return true;
}


bool Type_handler_int_result::
       Item_func_div_fix_length_and_dec(Item_func_div *item) const
{
  item->fix_length_and_dec_int();
  return false;
}


bool Type_handler_real_result::
       Item_func_div_fix_length_and_dec(Item_func_div *item) const
{
  item->fix_length_and_dec_double();
  return false;
}


bool Type_handler_decimal_result::
       Item_func_div_fix_length_and_dec(Item_func_div *item) const
{
  item->fix_length_and_dec_decimal();
  return false;
}


bool Type_handler_temporal_result::
       Item_func_div_fix_length_and_dec(Item_func_div *item) const
{
  item->fix_length_and_dec_temporal();
  return false;
}


bool Type_handler_string_result::
       Item_func_div_fix_length_and_dec(Item_func_div *item) const
{
  item->fix_length_and_dec_double();
  return false;
}

/***************************************************************************/

bool Type_handler_row::
       Item_func_mod_fix_length_and_dec(Item_func_mod *item) const
{
  DBUG_ASSERT(0);
  return true;
}


bool Type_handler_int_result::
       Item_func_mod_fix_length_and_dec(Item_func_mod *item) const
{
  item->fix_length_and_dec_int();
  return false;
}


bool Type_handler_real_result::
       Item_func_mod_fix_length_and_dec(Item_func_mod *item) const
{
  item->fix_length_and_dec_double();
  return false;
}


bool Type_handler_decimal_result::
       Item_func_mod_fix_length_and_dec(Item_func_mod *item) const
{
  item->fix_length_and_dec_decimal();
  return false;
}


bool Type_handler_temporal_result::
       Item_func_mod_fix_length_and_dec(Item_func_mod *item) const
{
  item->fix_length_and_dec_temporal();
  return false;
}


bool Type_handler_string_result::
       Item_func_mod_fix_length_and_dec(Item_func_mod *item) const
{
  item->fix_length_and_dec_double();
  return false;
}

/***************************************************************************/

uint Type_handler::Item_time_precision(THD *thd, Item *item) const
{
  return MY_MIN(item->decimals, TIME_SECOND_PART_DIGITS);
}


uint Type_handler::Item_datetime_precision(THD *thd, Item *item) const
{
  return MY_MIN(item->decimals, TIME_SECOND_PART_DIGITS);
}


uint Type_handler_string_result::Item_temporal_precision(THD *thd, Item *item,
                                                         bool is_time) const
{
  StringBuffer<64> buf;
  String *tmp;
  MYSQL_TIME_STATUS status;
  DBUG_ASSERT(item->is_fixed());
  if ((tmp= item->val_str(&buf)) &&
      (is_time ?
       Time(thd, &status, tmp->ptr(), tmp->length(), tmp->charset(),
            Time::Options(TIME_TIME_ONLY,
                          Time::DATETIME_TO_TIME_YYYYMMDD_TRUNCATE)).
         is_valid_time() :
       Datetime(&status, tmp->ptr(), tmp->length(), tmp->charset(),
                TIME_FUZZY_DATES).
         is_valid_datetime()))
    return MY_MIN(status.precision, TIME_SECOND_PART_DIGITS);
  return MY_MIN(item->decimals, TIME_SECOND_PART_DIGITS);
}

/***************************************************************************/

uint Type_handler::Item_decimal_scale(const Item *item) const
{
  return item->decimals < NOT_FIXED_DEC ?
         item->decimals :
         MY_MIN(item->max_length, DECIMAL_MAX_SCALE);
}

uint Type_handler_temporal_result::
       Item_decimal_scale_with_seconds(const Item *item) const
{
  return item->decimals < NOT_FIXED_DEC ?
         item->decimals :
         TIME_SECOND_PART_DIGITS;
}

uint Type_handler::Item_divisor_precision_increment(const Item *item) const
{
  return item->decimals;
}

uint Type_handler_temporal_result::
       Item_divisor_precision_increment_with_seconds(const Item *item) const
{
  return item->decimals <  NOT_FIXED_DEC ?
         item->decimals :
         TIME_SECOND_PART_DIGITS;
}

/***************************************************************************/

uint Type_handler_string_result::Item_decimal_precision(const Item *item) const
{
  uint res= item->max_char_length();
  /*
    Return at least one decimal digit, even if Item::max_char_length()
    returned  0. This is important to avoid attempts to create fields of types
    INT(0) or DECIMAL(0,0) when converting NULL or empty strings to INT/DECIMAL:
      CREATE TABLE t1 AS SELECT CONVERT(NULL,SIGNED) AS a;
  */
  return res ? MY_MIN(res, DECIMAL_MAX_PRECISION) : 1;
}

uint Type_handler_real_result::Item_decimal_precision(const Item *item) const
{
  uint res= item->max_char_length();
  return res ? MY_MIN(res, DECIMAL_MAX_PRECISION) : 1;
}

uint Type_handler_decimal_result::Item_decimal_precision(const Item *item) const
{
  uint prec= my_decimal_length_to_precision(item->max_char_length(),
                                            item->decimals,
                                            item->unsigned_flag);
  return MY_MIN(prec, DECIMAL_MAX_PRECISION);
}

uint Type_handler_int_result::Item_decimal_precision(const Item *item) const
{
 uint prec= my_decimal_length_to_precision(item->max_char_length(),
                                           item->decimals,
                                           item->unsigned_flag);
 return MY_MIN(prec, DECIMAL_MAX_PRECISION);
}

uint Type_handler_time_common::Item_decimal_precision(const Item *item) const
{
  return 7 + MY_MIN(item->decimals, TIME_SECOND_PART_DIGITS);
}

uint Type_handler_date_common::Item_decimal_precision(const Item *item) const
{
  return 8;
}

uint Type_handler_datetime_common::Item_decimal_precision(const Item *item) const
{
  return 14 + MY_MIN(item->decimals, TIME_SECOND_PART_DIGITS);
}

uint Type_handler_timestamp_common::Item_decimal_precision(const Item *item) const
{
  return 14 + MY_MIN(item->decimals, TIME_SECOND_PART_DIGITS);
}

/***************************************************************************/

bool Type_handler_real_result::
       subquery_type_allows_materialization(const Item *inner,
                                            const Item *outer) const
{
  DBUG_ASSERT(inner->cmp_type() == REAL_RESULT);
  return outer->cmp_type() == REAL_RESULT;
}


bool Type_handler_int_result::
       subquery_type_allows_materialization(const Item *inner,
                                            const Item *outer) const
{
  DBUG_ASSERT(inner->cmp_type() == INT_RESULT);
  return outer->cmp_type() == INT_RESULT;
}


bool Type_handler_decimal_result::
       subquery_type_allows_materialization(const Item *inner,
                                            const Item *outer) const
{
  DBUG_ASSERT(inner->cmp_type() == DECIMAL_RESULT);
  return outer->cmp_type() == DECIMAL_RESULT;
}


bool Type_handler_string_result::
       subquery_type_allows_materialization(const Item *inner,
                                            const Item *outer) const
{
  DBUG_ASSERT(inner->cmp_type() == STRING_RESULT);
  return outer->cmp_type() == STRING_RESULT &&
         outer->collation.collation == inner->collation.collation &&
         /*
           Materialization also is unable to work when create_tmp_table() will
           create a blob column because item->max_length is too big.
           The following test is copied from varstring_type_handler().
         */
         !inner->too_big_for_varchar();
}


bool Type_handler_temporal_result::
       subquery_type_allows_materialization(const Item *inner,
                                            const Item *outer) const
{
  DBUG_ASSERT(inner->cmp_type() == TIME_RESULT);
  return mysql_timestamp_type() ==
         outer->type_handler()->mysql_timestamp_type();
}

/***************************************************************************/


const Type_handler *
Type_handler_null::type_handler_for_tmp_table(const Item *item) const
{
  return &type_handler_string;
}


const Type_handler *
Type_handler_null::type_handler_for_union(const Item *item) const
{
  return &type_handler_string;
}


const Type_handler *
Type_handler_olddecimal::type_handler_for_tmp_table(const Item *item) const
{
  return &type_handler_newdecimal;
}

const Type_handler *
Type_handler_olddecimal::type_handler_for_union(const Item *item) const
{
  return &type_handler_newdecimal;
}


/***************************************************************************/

bool Type_handler::check_null(const Item *item, st_value *value) const
{
  if (item->null_value)
  {
    value->m_type= DYN_COL_NULL;
    return true;
  }
  return false;
}


bool Type_handler_null::
       Item_save_in_value(THD *thd, Item *item, st_value *value) const
{
  value->m_type= DYN_COL_NULL;
  return true;
}


bool Type_handler_row::
       Item_save_in_value(THD *thd, Item *item, st_value *value) const
{
  DBUG_ASSERT(0);
  value->m_type= DYN_COL_NULL;
  return true;
}


bool Type_handler_int_result::
       Item_save_in_value(THD *thd, Item *item, st_value *value) const
{
  value->m_type= item->unsigned_flag ? DYN_COL_UINT : DYN_COL_INT;
  value->value.m_longlong= item->val_int();
  return check_null(item, value);
}


bool Type_handler_real_result::
       Item_save_in_value(THD *thd, Item *item, st_value *value) const
{
  value->m_type= DYN_COL_DOUBLE;
  value->value.m_double= item->val_real();
  return check_null(item, value);
}


bool Type_handler_decimal_result::
       Item_save_in_value(THD *thd, Item *item, st_value *value) const
{
  value->m_type= DYN_COL_DECIMAL;
  my_decimal *dec= item->val_decimal(&value->m_decimal);
  if (dec != &value->m_decimal && !item->null_value)
    my_decimal2decimal(dec, &value->m_decimal);
  return check_null(item, value);
}


bool Type_handler_string_result::
       Item_save_in_value(THD *thd, Item *item, st_value *value) const
{
  value->m_type= DYN_COL_STRING;
  String *str= item->val_str(&value->m_string);
  if (str != &value->m_string && !item->null_value)
    value->m_string.set(str->ptr(), str->length(), str->charset());
  return check_null(item, value);
}


bool Type_handler_temporal_with_date::
       Item_save_in_value(THD *thd, Item *item, st_value *value) const
{
  value->m_type= DYN_COL_DATETIME;
  item->get_date(thd, &value->value.m_time, sql_mode_for_dates(thd));
  return check_null(item, value);
}


bool Type_handler_time_common::
       Item_save_in_value(THD *thd, Item *item, st_value *value) const
{
  value->m_type= DYN_COL_DATETIME;
  item->get_time(thd, &value->value.m_time);
  return check_null(item, value);
}

/***************************************************************************/

bool Type_handler_row::
  Item_param_set_from_value(THD *thd,
                            Item_param *param,
                            const Type_all_attributes *attr,
                            const st_value *val) const
{
  DBUG_ASSERT(0);
  param->set_null();
  return true;
}


bool Type_handler_real_result::
  Item_param_set_from_value(THD *thd,
                            Item_param *param,
                            const Type_all_attributes *attr,
                            const st_value *val) const
{
  param->unsigned_flag= attr->unsigned_flag;
  param->set_double(val->value.m_double);
  return false;
}


bool Type_handler_int_result::
  Item_param_set_from_value(THD *thd,
                            Item_param *param,
                            const Type_all_attributes *attr,
                            const st_value *val) const
{
  param->unsigned_flag= attr->unsigned_flag;
  param->set_int(val->value.m_longlong, attr->max_length);
  return false;
}


bool Type_handler_decimal_result::
  Item_param_set_from_value(THD *thd,
                            Item_param *param,
                            const Type_all_attributes *attr,
                            const st_value *val) const
{
  param->unsigned_flag= attr->unsigned_flag;
  param->set_decimal(&val->m_decimal, attr->unsigned_flag);
  return false;
}


bool Type_handler_string_result::
  Item_param_set_from_value(THD *thd,
                            Item_param *param,
                            const Type_all_attributes *attr,
                            const st_value *val) const
{
  param->unsigned_flag= false;
  param->setup_conversion_string(thd, attr->collation.collation);
  /*
    Exact value of max_length is not known unless data is converted to
    charset of connection, so we have to set it later.
  */
  return param->set_str(val->m_string.ptr(), val->m_string.length(),
                        attr->collation.collation,
                        attr->collation.collation);
}


bool Type_handler_temporal_result::
  Item_param_set_from_value(THD *thd,
                            Item_param *param,
                            const Type_all_attributes *attr,
                            const st_value *val) const
{
  param->unsigned_flag= attr->unsigned_flag;
  param->set_time(&val->value.m_time, attr->max_length, attr->decimals);
  return false;
}


#ifdef HAVE_SPATIAL
bool Type_handler_geometry::
  Item_param_set_from_value(THD *thd,
                            Item_param *param,
                            const Type_all_attributes *attr,
                            const st_value *val) const
{
  param->unsigned_flag= false;
  param->setup_conversion_blob(thd);
  param->set_geometry_type(attr->uint_geometry_type());
  return param->set_str(val->m_string.ptr(), val->m_string.length(),
                        &my_charset_bin, &my_charset_bin);
}
#endif

/***************************************************************************/

bool Type_handler_null::
      Item_send(Item *item, Protocol *protocol, st_value *buf) const
{
  return protocol->store_null();
}


bool Type_handler::
       Item_send_str(Item *item, Protocol *protocol, st_value *buf) const
{
  String *res;
  if ((res= item->val_str(&buf->m_string)))
  {
    DBUG_ASSERT(!item->null_value);
    return protocol->store(res->ptr(), res->length(), res->charset());
  }
  DBUG_ASSERT(item->null_value);
  return protocol->store_null();
}


bool Type_handler::
       Item_send_tiny(Item *item, Protocol *protocol, st_value *buf) const
{
  longlong nr= item->val_int();
  if (!item->null_value)
    return protocol->store_tiny(nr);
  return protocol->store_null();
}


bool Type_handler::
       Item_send_short(Item *item, Protocol *protocol, st_value *buf) const
{
  longlong nr= item->val_int();
  if (!item->null_value)
    return protocol->store_short(nr);
  return protocol->store_null();
}


bool Type_handler::
       Item_send_long(Item *item, Protocol *protocol, st_value *buf) const
{
  longlong nr= item->val_int();
  if (!item->null_value)
    return protocol->store_long(nr);
  return protocol->store_null();
}

bool Type_handler::
       Item_send_longlong(Item *item, Protocol *protocol, st_value *buf) const
{
  longlong nr= item->val_int();
  if (!item->null_value)
    return protocol->store_longlong(nr, item->unsigned_flag);
  return protocol->store_null();
}


bool Type_handler::
       Item_send_float(Item *item, Protocol *protocol, st_value *buf) const
{
  float nr= (float) item->val_real();
  if (!item->null_value)
    return protocol->store(nr, item->decimals, &buf->m_string);
  return protocol->store_null();
}


bool Type_handler::
       Item_send_double(Item *item, Protocol *protocol, st_value *buf) const
{
  double nr= item->val_real();
  if (!item->null_value)
    return protocol->store(nr, item->decimals, &buf->m_string);
  return protocol->store_null();
}


bool Type_handler::
       Item_send_datetime(Item *item, Protocol *protocol, st_value *buf) const
{
  item->get_date(protocol->thd, &buf->value.m_time,
                 sql_mode_for_dates(protocol->thd));
  if (!item->null_value)
    return protocol->store(&buf->value.m_time, item->decimals);
  return protocol->store_null();
}


bool Type_handler::
       Item_send_date(Item *item, Protocol *protocol, st_value *buf) const
{
  item->get_date(protocol->thd, &buf->value.m_time,
                 sql_mode_for_dates(protocol->thd));
  if (!item->null_value)
    return protocol->store_date(&buf->value.m_time);
  return protocol->store_null();
}


bool Type_handler::
       Item_send_time(Item *item, Protocol *protocol, st_value *buf) const
{
  item->get_time(protocol->thd, &buf->value.m_time);
  if (!item->null_value)
    return protocol->store_time(&buf->value.m_time, item->decimals);
  return protocol->store_null();
}

/***************************************************************************/

Item *Type_handler_int_result::
  make_const_item_for_comparison(THD *thd, Item *item, const Item *cmp) const
{
  longlong result= item->val_int();
  if (item->null_value)
    return new (thd->mem_root) Item_null(thd, item->name.str);
  return  new (thd->mem_root) Item_int(thd, item->name.str, result,
                                       item->max_length);
}


Item *Type_handler_real_result::
  make_const_item_for_comparison(THD *thd, Item *item, const Item *cmp) const
{
  double result= item->val_real();
  if (item->null_value)
    return new (thd->mem_root) Item_null(thd, item->name.str);
  return new (thd->mem_root) Item_float(thd, item->name.str, result,
                                        item->decimals, item->max_length);
}


Item *Type_handler_decimal_result::
  make_const_item_for_comparison(THD *thd, Item *item, const Item *cmp) const
{
  VDec result(item);
  if (result.is_null())
    return new (thd->mem_root) Item_null(thd, item->name.str);
  return new (thd->mem_root) Item_decimal(thd, item->name.str, result.ptr(),
                                          item->max_length, item->decimals);
}


Item *Type_handler_string_result::
  make_const_item_for_comparison(THD *thd, Item *item, const Item *cmp) const
{
  StringBuffer<MAX_FIELD_WIDTH> tmp;
  String *result= item->val_str(&tmp);
  if (item->null_value)
    return new (thd->mem_root) Item_null(thd, item->name.str);
  uint length= result->length();
  char *tmp_str= thd->strmake(result->ptr(), length);
  return new (thd->mem_root) Item_string(thd, item->name.str,
                                         tmp_str, length, result->charset());
}


Item *Type_handler_time_common::
  make_const_item_for_comparison(THD *thd, Item *item, const Item *cmp) const
{
  Item_cache_temporal *cache;
  longlong value= item->val_time_packed(thd);
  if (item->null_value)
    return new (thd->mem_root) Item_null(thd, item->name.str);
  cache= new (thd->mem_root) Item_cache_time(thd);
  if (cache)
    cache->store_packed(value, item);
  return cache;
}


Item *Type_handler_temporal_with_date::
  make_const_item_for_comparison(THD *thd, Item *item, const Item *cmp) const
{
  Item_cache_temporal *cache;
  longlong value= item->val_datetime_packed(thd);
  if (item->null_value)
    return new (thd->mem_root) Item_null(thd, item->name.str);
  cache= new (thd->mem_root) Item_cache_datetime(thd);
  if (cache)
    cache->store_packed(value, item);
  return cache;
}


Item *Type_handler_row::
  make_const_item_for_comparison(THD *thd, Item *item, const Item *cmp) const
{
  if (item->type() == Item::ROW_ITEM && cmp->type() == Item::ROW_ITEM)
  {
    /*
      Substitute constants only in Item_row's. Don't affect other Items
      with ROW_RESULT (eg Item_singlerow_subselect).

      For such Items more optimal is to detect if it is constant and replace
      it with Item_row. This would optimize queries like this:
      SELECT * FROM t1 WHERE (a,b) = (SELECT a,b FROM t2 LIMIT 1);
    */
    Item_row *item_row= (Item_row*) item;
    Item_row *comp_item_row= (Item_row*) cmp;
    uint col;
    /*
      If item and comp_item are both Item_row's and have same number of cols
      then process items in Item_row one by one.
      We can't ignore NULL values here as this item may be used with <=>, in
      which case NULL's are significant.
    */
    DBUG_ASSERT(item->result_type() == cmp->result_type());
    DBUG_ASSERT(item_row->cols() == comp_item_row->cols());
    col= item_row->cols();
    while (col-- > 0)
      resolve_const_item(thd, item_row->addr(col),
                         comp_item_row->element_index(col));
  }
  return NULL;
}

/***************************************************************************/

static const char* item_name(Item *a, String *str)
{
  if (a->name.str)
    return a->name.str;
  str->length(0);
  a->print(str, QT_ORDINARY);
  return str->c_ptr_safe();
}


static void wrong_precision_error(uint errcode, Item *a,
                                  ulonglong number, uint maximum)
{
  StringBuffer<1024> buf(system_charset_info);
  my_error(errcode, MYF(0), number, item_name(a, &buf), maximum);
}


/**
  Get precision and scale for a declaration
 
  return
    0  ok
    1  error
*/

bool get_length_and_scale(ulonglong length, ulonglong decimals,
                          uint *out_length, uint *out_decimals,
                          uint max_precision, uint max_scale,
                          Item *a)
{
  if (length > (ulonglong) max_precision)
  {
    wrong_precision_error(ER_TOO_BIG_PRECISION, a, length, max_precision);
    return 1;
  }
  if (decimals > (ulonglong) max_scale)
  {
    wrong_precision_error(ER_TOO_BIG_SCALE, a, decimals, max_scale);
    return 1;
  }

  *out_decimals=  (uint) decimals;
  my_decimal_trim(&length, out_decimals);
  *out_length=  (uint) length;
  
  if (*out_length < *out_decimals)
  {
    my_error(ER_M_BIGGER_THAN_D, MYF(0), "");
    return 1;
  }
  return 0;
}


Item *Type_handler_longlong::
        create_typecast_item(THD *thd, Item *item,
                             const Type_cast_attributes &attr) const
{
  if (this != &type_handler_ulonglong)
    return new (thd->mem_root) Item_func_signed(thd, item);
  return new (thd->mem_root) Item_func_unsigned(thd, item);

}


Item *Type_handler_date_common::
        create_typecast_item(THD *thd, Item *item,
                             const Type_cast_attributes &attr) const
{
  return new (thd->mem_root) Item_date_typecast(thd, item);
}



Item *Type_handler_time_common::
        create_typecast_item(THD *thd, Item *item,
                             const Type_cast_attributes &attr) const
{
  if (attr.decimals() > MAX_DATETIME_PRECISION)
  {
    wrong_precision_error(ER_TOO_BIG_PRECISION, item, attr.decimals(),
                          MAX_DATETIME_PRECISION);
    return 0;
  }
  return new (thd->mem_root)
         Item_time_typecast(thd, item, (uint) attr.decimals());
}


Item *Type_handler_datetime_common::
        create_typecast_item(THD *thd, Item *item,
                             const Type_cast_attributes &attr) const
{
  if (attr.decimals() > MAX_DATETIME_PRECISION)
  {
    wrong_precision_error(ER_TOO_BIG_PRECISION, item, attr.decimals(),
                          MAX_DATETIME_PRECISION);
    return 0;
  }
  return new (thd->mem_root)
         Item_datetime_typecast(thd, item, (uint) attr.decimals());

}


Item *Type_handler_decimal_result::
        create_typecast_item(THD *thd, Item *item,
                             const Type_cast_attributes &attr) const
{
  uint len, dec;
  if (get_length_and_scale(attr.length(), attr.decimals(), &len, &dec,
                           DECIMAL_MAX_PRECISION, DECIMAL_MAX_SCALE, item))
    return NULL;
  return new (thd->mem_root) Item_decimal_typecast(thd, item, len, dec);
}


Item *Type_handler_double::
        create_typecast_item(THD *thd, Item *item,
                             const Type_cast_attributes &attr) const
{
  uint len, dec;
  if (!attr.length_specified())
    return new (thd->mem_root) Item_double_typecast(thd, item,
                                                    DBL_DIG + 7,
                                                    NOT_FIXED_DEC);

  if (get_length_and_scale(attr.length(), attr.decimals(), &len, &dec,
                           DECIMAL_MAX_PRECISION, NOT_FIXED_DEC - 1, item))
    return NULL;
  return new (thd->mem_root) Item_double_typecast(thd, item, len, dec);
}


Item *Type_handler_long_blob::
        create_typecast_item(THD *thd, Item *item,
                             const Type_cast_attributes &attr) const
{
  int len= -1;
  CHARSET_INFO *real_cs= attr.charset() ?
                         attr.charset() :
                         thd->variables.collation_connection;
  if (attr.length_specified())
  {
    if (attr.length() > MAX_FIELD_BLOBLENGTH)
    {
      char buff[1024];
      String buf(buff, sizeof(buff), system_charset_info);
      my_error(ER_TOO_BIG_DISPLAYWIDTH, MYF(0), item_name(item, &buf),
               MAX_FIELD_BLOBLENGTH);
      return NULL;
    }
    len= (int) attr.length();
  }
  return new (thd->mem_root) Item_char_typecast(thd, item, len, real_cs);
}

/***************************************************************************/

void Type_handler_string_result::Item_param_setup_conversion(THD *thd,
                                                             Item_param *param)
                                                             const
{
  param->setup_conversion_string(thd, thd->variables.character_set_client);
}


void Type_handler_blob_common::Item_param_setup_conversion(THD *thd,
                                                           Item_param *param)
                                                           const
{
  param->setup_conversion_blob(thd);
}


void Type_handler_tiny::Item_param_set_param_func(Item_param *param,
                                                  uchar **pos, ulong len) const
{
  param->set_param_tiny(pos, len);
}


void Type_handler_short::Item_param_set_param_func(Item_param *param,
                                                   uchar **pos, ulong len) const
{
  param->set_param_short(pos, len);
}


void Type_handler_long::Item_param_set_param_func(Item_param *param,
                                                  uchar **pos, ulong len) const
{
  param->set_param_int32(pos, len);
}


void Type_handler_longlong::Item_param_set_param_func(Item_param *param,
                                                      uchar **pos,
                                                      ulong len) const
{
  param->set_param_int64(pos, len);
}


void Type_handler_float::Item_param_set_param_func(Item_param *param,
                                                   uchar **pos,
                                                   ulong len) const
{
  param->set_param_float(pos, len);
}


void Type_handler_double::Item_param_set_param_func(Item_param *param,
                                                   uchar **pos,
                                                   ulong len) const
{
  param->set_param_double(pos, len);
}


void Type_handler_decimal_result::Item_param_set_param_func(Item_param *param,
                                                            uchar **pos,
                                                            ulong len) const
{
  param->set_param_decimal(pos, len);
}


void Type_handler_string_result::Item_param_set_param_func(Item_param *param,
                                                           uchar **pos,
                                                           ulong len) const
{
  param->set_param_str(pos, len);
}


void Type_handler_time_common::Item_param_set_param_func(Item_param *param,
                                                         uchar **pos,
                                                         ulong len) const
{
  param->set_param_time(pos, len);
}


void Type_handler_date_common::Item_param_set_param_func(Item_param *param,
                                                         uchar **pos,
                                                         ulong len) const
{
  param->set_param_date(pos, len);
}


void Type_handler_datetime_common::Item_param_set_param_func(Item_param *param,
                                                             uchar **pos,
                                                             ulong len) const
{
  param->set_param_datetime(pos, len);
}

Field *Type_handler_blob_common::make_conversion_table_field(TABLE *table,
                                                            uint metadata,
                                                            const Field *target)
                                                            const
{
  uint pack_length= metadata & 0x00ff;
  if (pack_length < 1 || pack_length > 4)
    return NULL; // Broken binary log?
  return new(table->in_use->mem_root)
         Field_blob(NULL, (uchar *) "", 1, Field::NONE, &empty_clex_str,
                    table->s, pack_length, target->charset());
}


void Type_handler_timestamp_common::Item_param_set_param_func(Item_param *param,
                                                              uchar **pos,
                                                              ulong len) const
{
  param->set_param_datetime(pos, len);
}


void Type_handler::Item_param_set_param_func(Item_param *param,
                                             uchar **pos,
                                             ulong len) const
{
  param->set_null(); // Not possible type code in the client-server protocol
}


void Type_handler_typelib::Item_param_set_param_func(Item_param *param,
                                                     uchar **pos,
                                                     ulong len) const
{
  param->set_null(); // Not possible type code in the client-server protocol
}


#ifdef HAVE_SPATIAL
void Type_handler_geometry::Item_param_set_param_func(Item_param *param,
                                                      uchar **pos,
                                                      ulong len) const
{
  param->set_null(); // Not possible type code in the client-server protocol
}
#endif

/***************************************************************************/

Field *Type_handler_row::
  make_table_field_from_def(TABLE_SHARE *share, MEM_ROOT *mem_root,
                            const LEX_CSTRING *name,
                            const Record_addr &rec, const Bit_addr &bit,
                            const Column_definition_attributes *attr,
                            uint32 flags) const
{
  DBUG_ASSERT(attr->length == 0);
  DBUG_ASSERT(f_maybe_null(attr->pack_flag));
  return new (mem_root) Field_row(rec.ptr(), name);
}


Field *Type_handler_olddecimal::
  make_table_field_from_def(TABLE_SHARE *share, MEM_ROOT *mem_root,
                            const LEX_CSTRING *name,
                            const Record_addr &rec, const Bit_addr &bit,
                            const Column_definition_attributes *attr,
                            uint32 flags) const
{
  return new (mem_root)
    Field_decimal(rec.ptr(), (uint32) attr->length,
                  rec.null_ptr(), rec.null_bit(),
                  attr->unireg_check, name,
                  f_decimals(attr->pack_flag),
                  f_is_zerofill(attr->pack_flag) != 0,
                  f_is_dec(attr->pack_flag) == 0);
}


Field *Type_handler_newdecimal::
  make_table_field_from_def(TABLE_SHARE *share, MEM_ROOT *mem_root,
                            const LEX_CSTRING *name,
                            const Record_addr &rec, const Bit_addr &bit,
                            const Column_definition_attributes *attr,
                            uint32 flags) const
{
  return new (mem_root)
    Field_new_decimal(rec.ptr(), (uint32) attr->length,
                      rec.null_ptr(), rec.null_bit(),
                      attr->unireg_check, name,
                      f_decimals(attr->pack_flag),
                      f_is_zerofill(attr->pack_flag) != 0,
                      f_is_dec(attr->pack_flag) == 0);
}


Field *Type_handler_float::
  make_table_field_from_def(TABLE_SHARE *share, MEM_ROOT *mem_root,
                            const LEX_CSTRING *name,
                            const Record_addr &rec, const Bit_addr &bit,
                            const Column_definition_attributes *attr,
                            uint32 flags) const
{
  int decimals= f_decimals(attr->pack_flag);
  if (decimals == FLOATING_POINT_DECIMALS)
    decimals= NOT_FIXED_DEC;
  return new (mem_root)
    Field_float(rec.ptr(), (uint32) attr->length,
                rec.null_ptr(), rec.null_bit(),
                attr->unireg_check, name, decimals,
                f_is_zerofill(attr->pack_flag) != 0,
                f_is_dec(attr->pack_flag)== 0);
}


Field *Type_handler_double::
  make_table_field_from_def(TABLE_SHARE *share, MEM_ROOT *mem_root,
                            const LEX_CSTRING *name,
                            const Record_addr &rec, const Bit_addr &bit,
                            const Column_definition_attributes *attr,
                            uint32 flags) const
{
  int decimals= f_decimals(attr->pack_flag);
  if (decimals == FLOATING_POINT_DECIMALS)
    decimals= NOT_FIXED_DEC;
  return new (mem_root)
    Field_double(rec.ptr(), (uint32) attr->length,
                 rec.null_ptr(), rec.null_bit(),
                 attr->unireg_check, name, decimals,
                 f_is_zerofill(attr->pack_flag) != 0,
                 f_is_dec(attr->pack_flag)== 0);
}


Field *Type_handler_tiny::
  make_table_field_from_def(TABLE_SHARE *share, MEM_ROOT *mem_root,
                            const LEX_CSTRING *name,
                            const Record_addr &rec, const Bit_addr &bit,
                            const Column_definition_attributes *attr,
                            uint32 flags) const
{
  return new (mem_root)
    Field_tiny(rec.ptr(), (uint32) attr->length, rec.null_ptr(), rec.null_bit(),
               attr->unireg_check, name,
               f_is_zerofill(attr->pack_flag) != 0,
               f_is_dec(attr->pack_flag) == 0);
}


Field *Type_handler_short::
  make_table_field_from_def(TABLE_SHARE *share, MEM_ROOT *mem_root,
                            const LEX_CSTRING *name,
                            const Record_addr &rec, const Bit_addr &bit,
                            const Column_definition_attributes *attr,
                            uint32 flags) const
{
  return new (mem_root)
    Field_short(rec.ptr(), (uint32) attr->length,
                rec.null_ptr(), rec.null_bit(),
                attr->unireg_check, name,
                f_is_zerofill(attr->pack_flag) != 0,
                f_is_dec(attr->pack_flag) == 0);
}


Field *Type_handler_int24::
  make_table_field_from_def(TABLE_SHARE *share, MEM_ROOT *mem_root,
                            const LEX_CSTRING *name,
                            const Record_addr &rec, const Bit_addr &bit,
                            const Column_definition_attributes *attr,
                            uint32 flags) const
{
  return new (mem_root)
    Field_medium(rec.ptr(), (uint32) attr->length,
                 rec.null_ptr(), rec.null_bit(),
                 attr->unireg_check, name,
                 f_is_zerofill(attr->pack_flag) != 0,
                 f_is_dec(attr->pack_flag) == 0);
}


Field *Type_handler_long::
  make_table_field_from_def(TABLE_SHARE *share, MEM_ROOT *mem_root,
                            const LEX_CSTRING *name,
                            const Record_addr &rec, const Bit_addr &bit,
                            const Column_definition_attributes *attr,
                            uint32 flags) const
{
  return new (mem_root)
    Field_long(rec.ptr(), (uint32) attr->length, rec.null_ptr(), rec.null_bit(),
               attr->unireg_check, name,
               f_is_zerofill(attr->pack_flag) != 0,
               f_is_dec(attr->pack_flag) == 0);
}


Field *Type_handler_longlong::
  make_table_field_from_def(TABLE_SHARE *share, MEM_ROOT *mem_root,
                            const LEX_CSTRING *name,
                            const Record_addr &rec, const Bit_addr &bit,
                            const Column_definition_attributes *attr,
                            uint32 flags) const
{
  if (flags & (VERS_SYS_START_FLAG|VERS_SYS_END_FLAG))
    return new (mem_root)
      Field_vers_trx_id(rec.ptr(), (uint32) attr->length,
                        rec.null_ptr(), rec.null_bit(),
                        attr->unireg_check, name,
                        f_is_zerofill(attr->pack_flag) != 0,
                        f_is_dec(attr->pack_flag) == 0);
  return new (mem_root)
    Field_longlong(rec.ptr(), (uint32) attr->length,
                   rec.null_ptr(), rec.null_bit(),
                   attr->unireg_check, name,
                   f_is_zerofill(attr->pack_flag) != 0,
                   f_is_dec(attr->pack_flag) == 0);
}


Field *Type_handler_timestamp::
  make_table_field_from_def(TABLE_SHARE *share, MEM_ROOT *mem_root,
                            const LEX_CSTRING *name,
                            const Record_addr &rec, const Bit_addr &bit,
                            const Column_definition_attributes *attr,
                            uint32 flags) const
{
  return new_Field_timestamp(mem_root,
                             rec.ptr(), rec.null_ptr(), rec.null_bit(),
                             attr->unireg_check, name, share,
                             attr->temporal_dec(MAX_DATETIME_WIDTH));
}


Field *Type_handler_timestamp2::
  make_table_field_from_def(TABLE_SHARE *share, MEM_ROOT *mem_root,
                            const LEX_CSTRING *name,
                            const Record_addr &rec, const Bit_addr &bit,
                            const Column_definition_attributes *attr,
                            uint32 flags) const
{
  return new (mem_root)
    Field_timestampf(rec.ptr(), rec.null_ptr(), rec.null_bit(),
                     attr->unireg_check,
                     name, share, attr->temporal_dec(MAX_DATETIME_WIDTH));
}


Field *Type_handler_year::
  make_table_field_from_def(TABLE_SHARE *share, MEM_ROOT *mem_root,
                            const LEX_CSTRING *name,
                            const Record_addr &rec, const Bit_addr &bit,
                            const Column_definition_attributes *attr,
                                   uint32 flags) const
{
  return new (mem_root)
    Field_year(rec.ptr(), (uint32) attr->length, rec.null_ptr(), rec.null_bit(),
               attr->unireg_check, name);
}


Field *Type_handler_date::
  make_table_field_from_def(TABLE_SHARE *share, MEM_ROOT *mem_root,
                            const LEX_CSTRING *name,
                            const Record_addr &rec, const Bit_addr &bit,
                            const Column_definition_attributes *attr,
                            uint32 flags) const
{
  return new (mem_root)
    Field_date(rec.ptr(),rec.null_ptr(),rec.null_bit(),
               attr->unireg_check, name);
}


Field *Type_handler_newdate::
  make_table_field_from_def(TABLE_SHARE *share, MEM_ROOT *mem_root,
                            const LEX_CSTRING *name,
                            const Record_addr &rec, const Bit_addr &bit,
                            const Column_definition_attributes *attr,
                            uint32 flags) const
{
  return new (mem_root)
    Field_newdate(rec.ptr(), rec.null_ptr(), rec.null_bit(),
                  attr->unireg_check, name);
}


Field *Type_handler_time::
  make_table_field_from_def(TABLE_SHARE *share, MEM_ROOT *mem_root,
                            const LEX_CSTRING *name,
                            const Record_addr &rec, const Bit_addr &bit,
                            const Column_definition_attributes *attr,
                            uint32 flags) const
{
  return new_Field_time(mem_root, rec.ptr(), rec.null_ptr(), rec.null_bit(),
                        attr->unireg_check, name,
                        attr->temporal_dec(MIN_TIME_WIDTH));
}


Field *Type_handler_time2::
  make_table_field_from_def(TABLE_SHARE *share,  MEM_ROOT *mem_root,
                            const LEX_CSTRING *name,
                            const Record_addr &rec, const Bit_addr &bit,
                            const Column_definition_attributes *attr,
                            uint32 flags) const
{
  return new (mem_root)
    Field_timef(rec.ptr(), rec.null_ptr(), rec.null_bit(),
                attr->unireg_check, name,
                attr->temporal_dec(MIN_TIME_WIDTH));
}


Field *Type_handler_datetime::
  make_table_field_from_def(TABLE_SHARE *share, MEM_ROOT *mem_root,
                            const LEX_CSTRING *name,
                            const Record_addr &rec, const Bit_addr &bit,
                            const Column_definition_attributes *attr,
                            uint32 flags) const
{
  return new_Field_datetime(mem_root, rec.ptr(), rec.null_ptr(), rec.null_bit(),
                            attr->unireg_check, name,
                            attr->temporal_dec(MAX_DATETIME_WIDTH));
}


Field *Type_handler_datetime2::
  make_table_field_from_def(TABLE_SHARE *share, MEM_ROOT *mem_root,
                            const LEX_CSTRING *name,
                            const Record_addr &rec, const Bit_addr &bit,
                            const Column_definition_attributes *attr,
                            uint32 flags) const
{
  return new (mem_root)
    Field_datetimef(rec.ptr(), rec.null_ptr(), rec.null_bit(),
                    attr->unireg_check, name,
                    attr->temporal_dec(MAX_DATETIME_WIDTH));
}


Field *Type_handler_null::
  make_table_field_from_def(TABLE_SHARE *share, MEM_ROOT *mem_root,
                            const LEX_CSTRING *name,
                            const Record_addr &rec, const Bit_addr &bit,
                            const Column_definition_attributes *attr,
                            uint32 flags) const
{
  return new (mem_root)
    Field_null(rec.ptr(), (uint32) attr->length, attr->unireg_check,
               name, attr->charset);
}


Field *Type_handler_bit::
  make_table_field_from_def(TABLE_SHARE *share, MEM_ROOT *mem_root,
                            const LEX_CSTRING *name,
                            const Record_addr &rec, const Bit_addr &bit,
                            const Column_definition_attributes *attr,
                            uint32 flags) const
{
  return f_bit_as_char(attr->pack_flag) ?
     new (mem_root) Field_bit_as_char(rec.ptr(), (uint32) attr->length,
                                      rec.null_ptr(), rec.null_bit(),
                                      attr->unireg_check, name) :
     new (mem_root) Field_bit(rec.ptr(), (uint32) attr->length,
                              rec.null_ptr(), rec.null_bit(),
                              bit.ptr(), bit.offs(), attr->unireg_check, name);
}


#ifdef HAVE_SPATIAL
Field *Type_handler_geometry::
  make_table_field_from_def(TABLE_SHARE *share, MEM_ROOT *mem_root,
                            const LEX_CSTRING *name,
                            const Record_addr &rec, const Bit_addr &bit,
                            const Column_definition_attributes *attr,
                            uint32 flags) const
{
  status_var_increment(current_thd->status_var.feature_gis);
  return new (mem_root)
    Field_geom(rec.ptr(), rec.null_ptr(), rec.null_bit(),
               attr->unireg_check, name, share,
               attr->pack_flag_to_pack_length(), attr->geom_type, attr->srid);
}
#endif


Field *Type_handler_string::
  make_table_field_from_def(TABLE_SHARE *share, MEM_ROOT *mem_root,
                            const LEX_CSTRING *name,
                            const Record_addr &rec, const Bit_addr &bit,
                            const Column_definition_attributes *attr,
                            uint32 flags) const
{
  return new (mem_root)
    Field_string(rec.ptr(), (uint32) attr->length,
                 rec.null_ptr(), rec.null_bit(),
                 attr->unireg_check, name, attr->charset, this);
}


Field *Type_handler_varchar::
  make_table_field_from_def(TABLE_SHARE *share, MEM_ROOT *mem_root,
                            const LEX_CSTRING *name,
                            const Record_addr &rec, const Bit_addr &bit,
                            const Column_definition_attributes *attr,
                            uint32 flags) const
{
  if (attr->unireg_check == Field::TMYSQL_COMPRESSED)
    return new (mem_root)
      Field_varstring_compressed(rec.ptr(), (uint32) attr->length,
                                 HA_VARCHAR_PACKLENGTH((uint32) attr->length),
                                 rec.null_ptr(), rec.null_bit(),
                                 attr->unireg_check, name, share, attr->charset,
                                 zlib_compression_method, this);
  return new (mem_root)
    Field_varstring(rec.ptr(), (uint32) attr->length,
                    HA_VARCHAR_PACKLENGTH((uint32) attr->length),
                    rec.null_ptr(), rec.null_bit(),
                    attr->unireg_check, name, share, attr->charset, this);
}


Field *Type_handler_blob_common::
  make_table_field_from_def(TABLE_SHARE *share, MEM_ROOT *mem_root,
                            const LEX_CSTRING *name,
                            const Record_addr &rec, const Bit_addr &bit,
                            const Column_definition_attributes *attr,
                            uint32 flags) const
{
  if (attr->unireg_check == Field::TMYSQL_COMPRESSED)
    return new (mem_root)
      Field_blob_compressed(rec.ptr(), rec.null_ptr(), rec.null_bit(),
                            attr->unireg_check, name, share,
                            attr->pack_flag_to_pack_length(), attr->charset,
                            zlib_compression_method);
  return new (mem_root)
    Field_blob(rec.ptr(), rec.null_ptr(), rec.null_bit(),
               attr->unireg_check, name, share,
               attr->pack_flag_to_pack_length(), attr->charset);
}


Field *Type_handler_enum::
  make_table_field_from_def(TABLE_SHARE *share, MEM_ROOT *mem_root,
                            const LEX_CSTRING *name,
                            const Record_addr &rec, const Bit_addr &bit,
                            const Column_definition_attributes *attr,
                            uint32 flags) const
{
  return new (mem_root)
    Field_enum(rec.ptr(), (uint32) attr->length, rec.null_ptr(), rec.null_bit(),
               attr->unireg_check, name, attr->pack_flag_to_pack_length(),
               attr->interval, attr->charset);
}


Field *Type_handler_set::
  make_table_field_from_def(TABLE_SHARE *share, MEM_ROOT *mem_root,
                            const LEX_CSTRING *name,
                            const Record_addr &rec, const Bit_addr &bit,
                            const Column_definition_attributes *attr,
                            uint32 flags) const
{
  return new (mem_root)
    Field_set(rec.ptr(), (uint32) attr->length, rec.null_ptr(), rec.null_bit(),
              attr->unireg_check, name, attr->pack_flag_to_pack_length(),
              attr->interval, attr->charset);
}


/***************************************************************************/

void Type_handler::
  Column_definition_attributes_frm_pack(const Column_definition_attributes *def,
                                        uchar *buff) const
{
  def->frm_pack_basic(buff);
  def->frm_pack_charset(buff);
}


#ifdef HAVE_SPATIAL
void Type_handler_geometry::
  Column_definition_attributes_frm_pack(const Column_definition_attributes *def,
                                        uchar *buff) const
{
  def->frm_pack_basic(buff);
  buff[11]= 0;
  buff[14]= (uchar) def->geom_type;
}
#endif


/***************************************************************************/

bool Type_handler::
  Column_definition_attributes_frm_unpack(Column_definition_attributes *attr,
                                          TABLE_SHARE *share,
                                          const uchar *buffer,
                                          LEX_CUSTRING *gis_options)
                                          const
{
  attr->frm_unpack_basic(buffer);
  return attr->frm_unpack_charset(share, buffer);
}


#ifdef HAVE_SPATIAL
bool Type_handler_geometry::
  Column_definition_attributes_frm_unpack(Column_definition_attributes *attr,
                                          TABLE_SHARE *share,
                                          const uchar *buffer,
                                          LEX_CUSTRING *gis_options)
                                          const
{
  uint gis_opt_read, gis_length, gis_decimals;
  Field_geom::storage_type st_type;
  attr->frm_unpack_basic(buffer);
  // charset and geometry_type share the same byte in frm
  attr->geom_type= (Field::geometry_type) buffer[14];
  gis_opt_read= gis_field_options_read(gis_options->str,
                                       gis_options->length,
                                       &st_type, &gis_length,
                                       &gis_decimals, &attr->srid);
  gis_options->str+= gis_opt_read;
  gis_options->length-= gis_opt_read;
  return false;
}
#endif

/***************************************************************************/

bool Type_handler::Vers_history_point_resolve_unit(THD *thd,
                                                   Vers_history_point *point)
                                                   const
{
  /*
    Disallow using non-relevant data types in history points.
    Even expressions with explicit TRANSACTION or TIMESTAMP units.
  */
  point->bad_expression_data_type_error(name().ptr());
  return true;
}


bool Type_handler_typelib::
       Vers_history_point_resolve_unit(THD *thd,
                                       Vers_history_point *point) const
{
  /*
    ENUM/SET have dual type properties (string and numeric).
    Require explicit CAST to avoid ambiguity.
  */
  point->bad_expression_data_type_error(name().ptr());
  return true;
}


bool Type_handler_general_purpose_int::
       Vers_history_point_resolve_unit(THD *thd,
                                       Vers_history_point *point) const
{
  return point->resolve_unit_trx_id(thd);
}


bool Type_handler_bit::
       Vers_history_point_resolve_unit(THD *thd,
                                       Vers_history_point *point) const
{
  return point->resolve_unit_trx_id(thd);
}


bool Type_handler_temporal_result::
       Vers_history_point_resolve_unit(THD *thd,
                                       Vers_history_point *point) const
{
  return point->resolve_unit_timestamp(thd);
}


bool Type_handler_general_purpose_string::
       Vers_history_point_resolve_unit(THD *thd,
                                       Vers_history_point *point) const
{
  return point->resolve_unit_timestamp(thd);
}

/***************************************************************************/

bool Type_handler_null::Item_const_eq(const Item_const *a,
                                      const Item_const *b,
                                      bool binary_cmp) const
{
  return true;
}


bool Type_handler_real_result::Item_const_eq(const Item_const *a,
                                             const Item_const *b,
                                             bool binary_cmp) const
{
  const double *va= a->const_ptr_double();
  const double *vb= b->const_ptr_double();
  return va[0] == vb[0];
}


bool Type_handler_int_result::Item_const_eq(const Item_const *a,
                                            const Item_const *b,
                                            bool binary_cmp) const
{
  const longlong *va= a->const_ptr_longlong();
  const longlong *vb= b->const_ptr_longlong();
  bool res= va[0] == vb[0] &&
            (va[0] >= 0 ||
             (a->get_type_all_attributes_from_const()->unsigned_flag ==
              b->get_type_all_attributes_from_const()->unsigned_flag));
  return res;
}


bool Type_handler_string_result::Item_const_eq(const Item_const *a,
                                               const Item_const *b,
                                               bool binary_cmp) const
{
  const String *sa= a->const_ptr_string();
  const String *sb= b->const_ptr_string();
  return binary_cmp ? sa->bin_eq(sb) :
    a->get_type_all_attributes_from_const()->collation.collation ==
    b->get_type_all_attributes_from_const()->collation.collation &&
    sa->eq(sb, a->get_type_all_attributes_from_const()->collation.collation);
}


bool
Type_handler_decimal_result::Item_const_eq(const Item_const *a,
                                           const Item_const *b,
                                           bool binary_cmp) const
{
  const my_decimal *da= a->const_ptr_my_decimal();
  const my_decimal *db= b->const_ptr_my_decimal();
  return !da->cmp(db) &&
         (!binary_cmp ||
          a->get_type_all_attributes_from_const()->decimals ==
          b->get_type_all_attributes_from_const()->decimals);
}


bool
Type_handler_temporal_result::Item_const_eq(const Item_const *a,
                                            const Item_const *b,
                                            bool binary_cmp) const
{
  const MYSQL_TIME *ta= a->const_ptr_mysql_time();
  const MYSQL_TIME *tb= b->const_ptr_mysql_time();
  return !my_time_compare(ta, tb) &&
         (!binary_cmp ||
          a->get_type_all_attributes_from_const()->decimals ==
          b->get_type_all_attributes_from_const()->decimals);
}

/***************************************************************************/

const Type_handler *
Type_handler_hex_hybrid::cast_to_int_type_handler() const
{
  return &type_handler_longlong;
}


const Type_handler *
Type_handler_hex_hybrid::type_handler_for_system_time() const
{
  return &type_handler_longlong;
}


/***************************************************************************/

bool Type_handler_row::Item_eq_value(THD *thd, const Type_cmp_attributes *attr,
                                     Item *a, Item *b) const
{
  DBUG_ASSERT(0);
  return false;
}


bool Type_handler_int_result::Item_eq_value(THD *thd,
                                            const Type_cmp_attributes *attr,
                                            Item *a, Item *b) const
{
  longlong value0= a->val_int();
  longlong value1= b->val_int();
  return !a->null_value && !b->null_value && value0 == value1 &&
         (value0 >= 0 || a->unsigned_flag == b->unsigned_flag);
}


bool Type_handler_real_result::Item_eq_value(THD *thd,
                                             const Type_cmp_attributes *attr,
                                             Item *a, Item *b) const
{
  double value0= a->val_real();
  double value1= b->val_real();
  return !a->null_value && !b->null_value && value0 == value1;
}


bool Type_handler_time_common::Item_eq_value(THD *thd,
                                             const Type_cmp_attributes *attr,
                                             Item *a, Item *b) const
{
  longlong value0= a->val_time_packed(thd);
  longlong value1= b->val_time_packed(thd);
  return !a->null_value && !b->null_value && value0 == value1;
}


bool Type_handler_temporal_with_date::Item_eq_value(THD *thd,
                                                    const Type_cmp_attributes *attr,
                                                    Item *a, Item *b) const
{
  longlong value0= a->val_datetime_packed(thd);
  longlong value1= b->val_datetime_packed(thd);
  return !a->null_value && !b->null_value && value0 == value1;
}


bool Type_handler_string_result::Item_eq_value(THD *thd,
                                               const Type_cmp_attributes *attr,
                                               Item *a, Item *b) const
{
  String *va, *vb;
  StringBuffer<128> cmp_value1, cmp_value2;
  return (va= a->val_str(&cmp_value1)) &&
         (vb= b->val_str(&cmp_value2)) &&
          va->eq(vb, attr->compare_collation());
}


/***************************************************************************/

void Type_handler_var_string::
  Column_definition_implicit_upgrade(Column_definition *c) const
{
  // Change old VARCHAR to new VARCHAR
  c->set_handler(&type_handler_varchar);
}


void Type_handler_time_common::
  Column_definition_implicit_upgrade(Column_definition *c) const
{
  if (opt_mysql56_temporal_format)
    c->set_handler(&type_handler_time2);
  else
    c->set_handler(&type_handler_time);
}


void Type_handler_datetime_common::
  Column_definition_implicit_upgrade(Column_definition *c) const
{
  if (opt_mysql56_temporal_format)
    c->set_handler(&type_handler_datetime2);
  else
    c->set_handler(&type_handler_datetime);
}


void Type_handler_timestamp_common::
  Column_definition_implicit_upgrade(Column_definition *c) const
{
  if (opt_mysql56_temporal_format)
    c->set_handler(&type_handler_timestamp2);
  else
    c->set_handler(&type_handler_timestamp);
}


/***************************************************************************/


int Type_handler_temporal_with_date::stored_field_cmp_to_item(THD *thd,
                                                              Field *field,
                                                              Item *item) const
{
  MYSQL_TIME field_time, item_time, item_time2, *item_time_cmp= &item_time;
  field->get_date(&field_time, TIME_INVALID_DATES);
  item->get_date(thd, &item_time, TIME_INVALID_DATES);
  if (item_time.time_type == MYSQL_TIMESTAMP_TIME &&
      time_to_datetime(thd, &item_time, item_time_cmp= &item_time2))
    return 1;
  return my_time_compare(&field_time, item_time_cmp);
}


int Type_handler_time_common::stored_field_cmp_to_item(THD *thd,
                                                       Field *field,
                                                       Item *item) const
{
  MYSQL_TIME field_time, item_time;
  field->get_time(&field_time);
  item->get_time(thd, &item_time);
  return my_time_compare(&field_time, &item_time);
}


int Type_handler_string_result::stored_field_cmp_to_item(THD *thd,
                                                         Field *field,
                                                         Item *item) const
{
  StringBuffer<MAX_FIELD_WIDTH> item_tmp;
  StringBuffer<MAX_FIELD_WIDTH> field_tmp;
  String *item_result= item->val_str(&item_tmp);
  /*
    Some implementations of Item::val_str(String*) actually modify
    the field Item::null_value, hence we can't check it earlier.
  */
  if (item->null_value)
    return 0;
  String *field_result= field->val_str(&field_tmp);
  return sortcmp(field_result, item_result, field->charset());
}


int Type_handler_int_result::stored_field_cmp_to_item(THD *thd,
                                                      Field *field,
                                                      Item *item) const
{
  DBUG_ASSERT(0); // Not used yet
  return 0;
}


int Type_handler_real_result::stored_field_cmp_to_item(THD *thd,
                                                       Field *field,
                                                       Item *item) const
{
  /*
    The patch for Bug#13463415 started using this function for comparing
    BIGINTs. That uncovered a bug in Visual Studio 32bit optimized mode.
    Prefixing the auto variables with volatile fixes the problem....
  */
  volatile double result= item->val_real();
  if (item->null_value)
    return 0;
  volatile double field_result= field->val_real();
  if (field_result < result)
    return -1;
  else if (field_result > result)
    return 1;
  return 0;
}


/***************************************************************************/


static bool have_important_literal_warnings(const MYSQL_TIME_STATUS *status)
{
  return (status->warnings & ~MYSQL_TIME_NOTE_TRUNCATED) != 0;
}


static void literal_warn(THD *thd, const Item *item,
                         const char *str, size_t length, CHARSET_INFO *cs,
                         timestamp_type time_type,
                         const MYSQL_TIME_STATUS *st,
                         const char *typestr, bool send_error)
{
  if (likely(item))
  {
    if (st->warnings) // e.g. a note on nanosecond truncation
    {
      ErrConvString err(str, length, cs);
      make_truncated_value_warning(thd,
                                   Sql_condition::time_warn_level(st->warnings),
                                   &err, time_type, 0);
    }
  }
  else if (send_error)
  {
    ErrConvString err(str, length, cs);
    my_error(ER_WRONG_VALUE, MYF(0), typestr, err.ptr());
  }
}


Item_literal *
Type_handler_date_common::create_literal_item(THD *thd,
                                              const char *str,
                                              size_t length,
                                              CHARSET_INFO *cs,
                                              bool send_error) const
{
  MYSQL_TIME_STATUS st;
  Item_literal *item= NULL;
  Temporal_hybrid tmp(&st, str, length, cs, sql_mode_for_dates(thd));
  if (tmp.is_valid_temporal() &&
      tmp.get_mysql_time()->time_type == MYSQL_TIMESTAMP_DATE &&
      !have_important_literal_warnings(&st))
    item= new (thd->mem_root) Item_date_literal(thd, tmp.get_mysql_time());
  literal_warn(thd, item, str, length, cs, MYSQL_TIMESTAMP_DATE,
               &st, "DATE", send_error);
  return item;
}


Item_literal *
Type_handler_temporal_with_date::create_literal_item(THD *thd,
                                                     const char *str,
                                                     size_t length,
                                                     CHARSET_INFO *cs,
                                                     bool send_error) const
{
  MYSQL_TIME_STATUS st;
  Item_literal *item= NULL;
  Temporal_hybrid tmp(&st, str, length, cs, sql_mode_for_dates(thd));
  if (tmp.is_valid_temporal() &&
      tmp.get_mysql_time()->time_type == MYSQL_TIMESTAMP_DATETIME &&
      !have_important_literal_warnings(&st))
    item= new (thd->mem_root) Item_datetime_literal(thd, tmp.get_mysql_time(),
                                                    st.precision);
  literal_warn(thd, item, str, length, cs, MYSQL_TIMESTAMP_DATETIME,
               &st, "DATETIME", send_error);
  return item;
}


Item_literal *
Type_handler_time_common::create_literal_item(THD *thd,
                                              const char *str,
                                              size_t length,
                                              CHARSET_INFO *cs,
                                              bool send_error) const
{
  MYSQL_TIME_STATUS st;
  Item_literal *item= NULL;
  Time::Options opt(TIME_TIME_ONLY, Time::DATETIME_TO_TIME_DISALLOW);
  Time tmp(thd, &st, str, length, cs, opt);
  if (tmp.is_valid_time() &&
      !have_important_literal_warnings(&st))
    item= new (thd->mem_root) Item_time_literal(thd, tmp.get_mysql_time(),
                                                st.precision);
  literal_warn(thd, item, str, length, cs, MYSQL_TIMESTAMP_TIME,
               &st, "TIME", send_error);
  return item;
}
