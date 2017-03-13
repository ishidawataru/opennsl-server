#!/do/not/bash
# toc_run_description = Looking for mysql_config
# toc_begin_help = 
#
# Looks for mysql_config. If it finds it then the following vars
# are exported:
#
#  MYSQL_CONFIG_BIN=/path/to/mysql_config
#  MYSQL_LIBS=link args for client apps
#  MYSQL_INCLUDES=includes args for client apps
#
# Clients can use --with-mysql_config=/path/to/mysql_config to specify
# their config script.
#
# = toc_end_help


cf=${configure_with_mysql_config}

test x = "x${cf}" && {
    toc_find_in_path mysql_config ${prefix}/bin:$PATH || {
        return 127
    }
    toc_export MYSQL_CONFIG_BIN=${TOC_FIND_RESULT}
}

toc_export MYSQL_CONFIG_BIN=${TOC_FIND_RESULT}
toc_export MYSQL_INCLUDES="$(${MYSQL_CONFIG_BIN} --include)"
toc_export MYSQL_LIBS="$(${MYSQL_CONFIG_BIN} --libs)"

return 0
