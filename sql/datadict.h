#ifndef DATADICT_INCLUDED
#define DATADICT_INCLUDED
/* Copyright (c) 2010, Oracle and/or its affiliates.
   Copyright (c) 2017 MariaDB corporation.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1335  USA */

#include "handler.h"

struct ddl_log_info;

/*
  Data dictionary API.
*/

enum Table_type
{
  TABLE_TYPE_UNKNOWN,
  TABLE_TYPE_NORMAL,                            /* Normal table */
  TABLE_TYPE_SEQUENCE,
  TABLE_TYPE_VIEW
};


#define INVISIBLE_MAX_BITS              3

/*
  Types of values in the MariaDB extra2 frm segment.
  Each value is written as
    type:       1 byte
    length:     1 byte  (1..255) or \0 and 2 bytes.
    binary value of the 'length' bytes.

  Older MariaDB servers can ignore values of unknown types if
  the type code is less than 128 (EXTRA2_ENGINE_IMPORTANT).
  Otherwise older (but newer than 10.0.1) servers are required
  to report an error.
*/
enum extra2_frm_value_type
{
  EXTRA2_TABLEDEF_VERSION=0,
  EXTRA2_DEFAULT_PART_ENGINE=1,
  EXTRA2_GIS=2,
  EXTRA2_APPLICATION_TIME_PERIOD=3,
  EXTRA2_PERIOD_FOR_SYSTEM_TIME=4,
  EXTRA2_FOREIGN_KEY_INFO=5,

#define EXTRA2_ENGINE_IMPORTANT 128

  EXTRA2_ENGINE_TABLEOPTS=128,
  EXTRA2_FIELD_FLAGS=129,
  EXTRA2_FIELD_DATA_TYPE_INFO=130
};

enum extra2_field_flags
{
  VERS_OPTIMIZED_UPDATE= 1 << INVISIBLE_MAX_BITS
};

uchar * extra2_write_len(uchar *pos, size_t len);
uchar * extra2_write_str(uchar *pos, const LEX_CSTRING &str);
inline
uchar *
extra2_write(uchar *pos, enum extra2_frm_value_type type, const LEX_CSTRING &str)
{
  *pos++ = type;
  return extra2_write_str(pos, str);
}
inline
uchar *
extra2_write(uchar *pos, enum extra2_frm_value_type type, const LEX_CUSTRING &str)
{
  return extra2_write(pos, type, *reinterpret_cast<const LEX_CSTRING*>(&str));
}
uchar *
extra2_write_field_properties(uchar *pos, List<Create_field> &create_fields);


struct Extra2_info
{
  LEX_CUSTRING version;
  LEX_CUSTRING options;
  Lex_ident engine;
  LEX_CUSTRING gis;
  LEX_CUSTRING field_flags;
  LEX_CUSTRING system_period;
  LEX_CUSTRING application_period;
  LEX_CUSTRING field_data_type_info;
  LEX_CUSTRING foreign_key_info;
  uint read_size;
  size_t write_size;

  Extra2_info()
  {
    bzero((void*)this, sizeof(*this));
  }

  template <class S>
  size_t store_size(const S &f) const
  {
    if (f.length == 0)
      return 0;
    DBUG_ASSERT(f.length <= 65535);
    // 1 byte is for type; 1 or 3 bytes for length
    return f.length + (f.length <= 255 ? 2 : 4);
  }
  size_t store_size() const
  {
    return
      store_size(version) +
      store_size(options) +
      store_size(engine) +
      store_size(gis) +
      store_size(field_flags) +
      store_size(system_period) +
      store_size(application_period) +
      store_size(field_data_type_info) +
      store_size(foreign_key_info);
  }
  bool read(const uchar* frm_image, size_t frm_size);
  uchar * write(uchar* frm_image, size_t frm_size);
};

class Table_name;
bool fk_backup_frm(ddl_log_info &log_info, Table_name table);
void fk_drop_shadow_frm(ddl_log_info &log_info, Table_name table);
void fk_drop_backup_frm(ddl_log_info &log_info, Table_name table);

/*
  Take extra care when using dd_frm_type() - it only checks the .frm file,
  and it won't work for any engine that supports discovery.

  Prefer to use ha_table_exists() instead.
  To check whether it's an frm of a view, use dd_frm_is_view().
*/

enum Table_type dd_frm_type(THD *thd, char *path, LEX_CSTRING *engine_name,
                            bool *is_sequence);

static inline bool dd_frm_is_view(THD *thd, char *path)
{
  bool not_used2;
  return dd_frm_type(thd, path, NULL, &not_used2) == TABLE_TYPE_VIEW;
}

bool dd_recreate_table(THD *thd, const char *db, const char *table_name);
size_t dd_extra2_len(const uchar** pos, const uchar* end);
#endif // DATADICT_INCLUDED
