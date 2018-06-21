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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "handler.h"

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

struct extra2_fields
{
  const uchar *version;
  size_t version_len;
  const uchar *options;
  size_t options_len;
  const uchar *part_engine;
  size_t part_engine_len;
  const uchar *gis;
  size_t gis_len;
  const uchar *system_period;
  const uchar *field_flags;
  size_t field_flags_len;
};

/*
  Take extra care when using dd_frm_type() - it only checks the .frm file,
  and it won't work for any engine that supports discovery.

  Prefer to use ha_table_exists() instead.
  To check whether it's an frm of a view, use dd_frm_is_view().
*/

enum Table_type dd_frm_type(THD *thd, char *path, LEX_CSTRING *engine_name,
                            bool *is_sequence, bool *is_versioned);

static inline bool dd_frm_is_view(THD *thd, char *path)
{
  bool not_used2;
  return dd_frm_type(thd, path, NULL, &not_used2, NULL) == TABLE_TYPE_VIEW;
}

bool dd_table_versioned(THD *thd, const char *db, const char *table_name,
                        bool &versioned);

bool dd_recreate_table(THD *thd, const char *db, const char *table_name,
                       const char *path = NULL);

bool dd_read_extra2(const uchar *frm_image, size_t len, extra2_fields *fields);
#endif // DATADICT_INCLUDED
