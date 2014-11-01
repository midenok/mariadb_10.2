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
