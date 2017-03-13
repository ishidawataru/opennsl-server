#!/do/not/bash
# toc_run_description = "Looking for pg_config (postgres)"
# toc_begin_help = 
#
# Looks for pg_config. If it finds it then the following vars
# are exported:
#
#  POSTGRES_CONFIG_BIN=/path/to/pg_config
#  POSTGRES_LIBS=link args for client apps
#  POSTGRES_INCLUDES=includes args for client apps
#
# Clients can use --with-postgres-config=/path/to/pg_config to specify
# their config script.
#
# = toc_end_help


cf=${configure_with_postgres_config}

test x = "x${cf}" && {
    toc_find_in_path pg_config "${prefix}/bin:$PATH" || {
        return 127
    }
    toc_export POSTGRES_CONFIG_BIN=${TOC_FIND_RESULT}
}

toc_export POSTGRES_INCLUDES="-I$(${POSTGRES_CONFIG_BIN} --includedir) -I$(${POSTGRES_CONFIG_BIN} --includedir-server)"
toc_export POSTGRES_LIBS="-L$(${POSTGRES_CONFIG_BIN} --libdir) -lpgeasy"

return 0
