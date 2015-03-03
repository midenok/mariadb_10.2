#
# sets plugindir variable
#
MACRO(SKY_FIX_PLUGINDIR plugin)
  IF (
      ${plugin} STREQUAL "cassandra"     OR
      ${plugin} STREQUAL "connect"       OR
      ${plugin} STREQUAL "federated"     OR 
      ${plugin} STREQUAL "federatedx"    OR
      ${plugin} STREQUAL "handlersocket" OR
      ${plugin} STREQUAL "oqgraph"       OR
     FALSE)
    SET(plugindir "${INSTALL_MYSQLSHAREDIR}/unsupported-plugins")
  ENDIF()
ENDMACRO()

SET(MYSQL_SERVER_SUFFIX "-enterprise")
MACRO(SKY_FIX_VARS)
  SET(CPACK_RPM_PACKAGE_URL "http://mariadb.com")
  SET(CPACK_PACKAGE_CONTACT "MariaDB Corporation <info@mariadb.com>")
  SET(CPACK_PACKAGE_VENDOR "MariaDB Corporation")
  SET(CPACK_RPM_PACKAGE_DESCRIPTION "MariaDB: a very fast and robust SQL database server

  It is GPL v2 licensed, which means you can use the it free of charge under the
  conditions of the GNU General Public License Version 2 (http://www.gnu.org/licenses/).

  MariaDB documentation can be found at https://mariadb.com/kb/en/
  MariaDB bug reports should be submitted through https://support.mariadb.com/

  ")
ENDMACRO()
