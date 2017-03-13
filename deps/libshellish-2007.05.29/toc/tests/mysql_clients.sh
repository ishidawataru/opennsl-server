# toc_run_description = looking for mysql clients
# toc_begin_help = 
# Looks for mysql client apps and sets the following config vars:
#	   HAVE_MYSQL_CLIENTS=1 or 0
#	   MYSQL=/path/to/mysql
#	   MYSQLADMIN=/path/to/mysqladmin
#	   MYSQLDUMP=/path/to/mysqldump
# = toc_end_help

toc_add_config MYSQL=
toc_add_config MYSQLDUMP=
toc_add_config MYSQLADMIN=
toc_add_config HAVE_MYSQL_CLIENTS=0

if test "x${configure_with_mysql}" = "x0"; then
	echo "mysql checks have been explicitely disabled."

	return 0
fi

ret=0
path="${configure_with_mysql-${PATH}}"
test "x$path" = "x1" && path="${PATH}"

for x in \
    mysql=MYSQL \
    mysqladmin=MYSQLADMIN \
    mysqldump=MYSQLDUMP \
    ; do
    f=${x%%=*}
    v=${x##*=}
    # echo search path=$path
    toc_find $f "$path"
    err=$?
    if test $err = 0; then
	bin="${TOC_FIND_RESULT}"
    else
	ret=1
	bin=
    fi
    toc_add_config $v="${bin}"
done

test $ret = 0 && {
    toc_add_config HAVE_MYSQL_CLIENTS=1
}
return $ret

