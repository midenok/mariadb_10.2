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

#
# Remove not-so-important components from the binary archive.
#

OPTION(LEAN_ARCHIVE "Remove extra stuff from archive" ON)

IF (LEAN_ARCHIVE)
  IF (WIN32)
    MESSAGE(STATUS "LEAN_ARCHIVE is currently not supported on Windows.")
  ELSEIF (RPM)
    MESSAGE(STATUS "RPM and LEAN_ARCHIVE are both set, ignoring LEAN_ARCHIVE.")
  ELSE()
    SET(CPACK_COMPONENTS_ALL
        Server IniFiles Server_Scripts SupportFiles
        Development Readme Common Client SharedLibraries
        ClientPlugins)
    MESSAGE(STATUS "Generating archive with following components : ${CPACK_COMPONENTS_ALL}")

    SET(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
    SET(CPACK_COMPONENT_INCLUDE_TOPLEVEL_DIRECTORY ON)
    SET(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)
  ENDIF()
ENDIF()
