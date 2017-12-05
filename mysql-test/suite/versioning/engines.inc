if ($HA_INNODB_SO)
{
    --source suite/versioning/innodb_plugin.inc
}

#
# suite.pm will make sure that all tests including this file
# will be skipped unless innodb is enabled
#
# The test below is redundant

if (`SELECT COUNT(*) = 0 FROM INFORMATION_SCHEMA.ENGINES WHERE engine = 'innodb' AND support IN ('YES', 'DEFAULT', 'ENABLED')`)
{
if ($MTR_COMBINATION_INNODB)
{
  --skip Test requires InnoDB.
}
if ($MTR_COMBINATION_INNODB_PLUGIN)
{
  --skip Test requires InnoDB.
}
}
