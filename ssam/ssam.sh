#!/bin/sh
arg0=`basename "$0"`

usage()
{
    printf "usage: %s [-n] [-e script] [-f sfile] [file ...]\n" "$1"
}

flagn=0
flage=""
flagf=""

while getopts ne:f:h OPT; do
    case "$OPT" in
        n)
            flagn=1
            ;;

        e)
            if [ -z "$OPTARG" ]; then
                usage "$arg0"
                exit 1
            fi
            flage="$OPTARG"
            ;;
        f)
            if [ -z "$OPTARG" ]; then
                usage "$arg0"
                exit 1
            fi
            flagf="$OPTARG"
            ;;
        h)
            usage "$arg0"
            exit 0
            ;;
    esac
done
shift `expr "$OPTIND" - 1`

if [ -z "$TMPDIR" ]; then
    TMPDIR="/tmp"
fi
tmp="$TMPDIR/ssam.tmp.$USER.$$"

trap 'result=$?; rm -f "$tmp"; exit $result' INT EXIT KILL
cat "$@" >"$tmp"

{
    # select entire file
    echo ',{'
    echo k
    echo '}'
    echo 0k

    # run scripts, print
    [ ! -z "$flagf" ] && cat "$flagf"
    [ ! -z "$flage" ] && echo "$flage"
    [ "$flagn" -eq 0 ] && echo ','
} | sam -d "$tmp" 2>/dev/null

exit $?