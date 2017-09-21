# toc_run_description = looking for a running mysql server
# toc_begin_help = 
# Looks for a running mysql server:
#   $@ = optional sql code, used on connecting to the db.
#
# This script requires that the mysql_clients test has been
# run, and will run it HAVE_MYSQL_CLIENTS is not 1.
#
# It accepts these optional config arguments:
#	--mysql-host=server
#	--mysql-user=user
#	--mysql-password=password
#	--mysql-port=server_port
#	--mysql-db=database_name
#
# = toc_end_help
#
#

toc_add_config HAVE_MYSQL_SERVER=0
toc_get_make HAVE_MYSQL_CLIENTS
test "x${TOC_GET_MAKE}" = "x1" || {
    toc_test mysql_clients || return $?
}

sql="${@-show databases}"

toc_get_make MYSQL
client=${TOC_GET_MAKE}

args=
for x in \
    user=mysql_user \
    host=mysql_host \
    password=mysql_password \
    port=mysql_port \
    database=mysql_db \
    ; do
# map --mysql-FOO to --FOO arg for mysql client:
    k="${x%%=*}"
    v=$"${x##*=}"
    uc=$(echo $v | tr [a-z] [A-Z])
    eval v='$'${v}
    test -z "$v" || args="$args --$k=${v}"
    toc_add_make $uc="$v"
done

echo -n "Attempting to connect to mysql server:" " "
# echo $client $args -e "'$sql'"
$client $args -e "$sql" > /dev/null
err=$?
if test $err != 0 ; then
    echo -n ${TOC_EMOTICON_ERROR} " "
    toc_boldecho "Could not connect to server!"
    return $err
fi

echo ${TOC_EMOTICON_OKAY}
toc_add_config HAVE_MYSQL_SERVER=1

return 0
