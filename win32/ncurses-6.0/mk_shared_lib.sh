#!/bin/sh
SHARED_LIB=$1
IMPORT_LIB=`echo "$1" | sed -e 's/[0-9]*\.dll$/.dll.a/'`
shift
cat <<-EOF
Linking shared library
** SHARED_LIB $SHARED_LIB
** IMPORT_LIB $IMPORT_LIB
EOF
exec $* -shared -Wl,--enable-auto-import,--out-implib=${IMPORT_LIB} -Wl,--export-all-symbols -o ${SHARED_LIB}
