# Copyright (c) 2006, 2014, Oracle and/or its affiliates.
# Copyright (c) 2012, 2014, SkySQL Ab.
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

IF(RPM)
  find_program (
    CHECKMODULE_PROGRAM
    checkmodule
  )
  find_program (
    SEMODULE_PACKAGE_PROGRAM
    semodule_package
  )
  SET(SELINUX_MAKEFILE /usr/share/selinux/devel/Makefile)
  IF((SEMODULE_PACKAGE_PROGRAM) AND (CHECKMODULE_PROGRAM) AND (EXISTS ${SELINUX_MAKEFILE}))
    CONFIGURE_FILE(mariadb-server.fc ${CMAKE_CURRENT_BINARY_DIR}/mariadb-server.fc @ONLY)
    CONFIGURE_FILE(mariadb-server.te ${CMAKE_CURRENT_BINARY_DIR}/mariadb-server.te @ONLY)
    add_custom_target(SE ALL 
      COMMAND make -f ${SELINUX_MAKEFILE}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/mariadb-server.pp
      DESTINATION "${CMAKE_INSTALL_PREFIX}/share/mysql/SELinux/"
      COMPONENT selinux
    )
  ELSE()
    message("SE-linux development tools aren't found.\nSE-linux policies package will not be generated. Try to install policycoreutils-python & selinux-policy-devel.")
  ENDIF()
ENDIF()
