/* Copyright (c) 2006, 2010, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1335  USA */

#ifndef SQL_RENAME_INCLUDED
#define SQL_RENAME_INCLUDED

#include "sql_class.h"
#include "table_cache.h"

class THD;
struct TABLE_LIST;

// NB: FK_ddl_backup responds to share release unlike FK_alter_backup
class FK_ddl_backup
{
public:
  Share_acquire sa;
  FK_list foreign_keys;
  FK_list referenced_keys;

  FK_ddl_backup(Share_acquire&& _sa);
  FK_ddl_backup(const FK_ddl_backup&)= delete;
  FK_ddl_backup(FK_ddl_backup&& src) :
    sa(std::move(src.sa)),
    foreign_keys(src.foreign_keys),
    referenced_keys(src.referenced_keys)
  {}

  void rollback();
};

class FK_backup_vector: public vector<FK_ddl_backup> {};

bool mysql_rename_tables(THD *thd, TABLE_LIST *table_list, bool silent);

#endif /* SQL_RENAME_INCLUDED */
