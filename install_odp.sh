#!/bin/bash

set -e
trap 'echo Error on line $BASH_SOURCE:$LINENO' ERR
trap 'rm -f $tmp' EXIT

START=`date +%s`;

#JUMBO_REPO="http://jumbo.baidu.com"
JUMBO_REPO="ftp://getprod:getprod@product.scm.baidu.com/data/prod-64/inf/odp/jumbo-repo/jumbo-repo_1-0-0_BL/output"
JUMBO_LOCATION=$JUMBO_REPO/jumbo

DEFAULT_JUMBO_ROOT=$HOME/.jumbo

usage()
{
    cat <<"EOT";
    Usage: install ODP environment script [-options]
    -d      Please enter the full path of the directory in which
            You want to install ODP environment$
    -t      Choose the ODP environment type , (online) or (develop)
    -s      Choose the webserver for ODP environment (lighttpd) or (nginx)
    
    Also you cloud use the other two options
    
    -v            Show install odp version
    --help        Print this message
EOT
}

odp_build_repo()
{
    rm -rf installs
    mkdir installs
    find packages -name "*.jumbo" | xargs -i{} cp {} installs/

    f="$PWD/tmp.tar"
    final="$PWD/list.tar.gz"

    (
        cd installs
        ls | while read i
        do
            pkgname_expected=${i%.jumbo}
            [ "$pkgname_expected" == "$i" ] && continue
            (
                . "$i" && [ "$pkgname" == "$pkgname_expected" ]
            ) || continue
            tar -rf "$f.tmp" "$i"
        done
        if [ -f "$f.tmp" ]
        then
            mv "$f.tmp" "$f"
            gzip "$f"
            mv "$f.gz" "$final"
        fi
    )
}


if test $# -eq 0
then
    usage
    exit
fi

while test $# -gt 0; do
    case "$1" in
        -*=*) optarg=$(echo "$1" | sed 's/[-_a-zA-Z0-9]*=//') ;;
        *) optarg= ;;
    esac

    case $1 in
       -d=*)
            ODP_ROOT=$optarg
            ;;  
       -t=*)
            ODP_TYPE=$optarg
            ;;  
       -s=*)
            ODP_SERVER=$optarg
            ;;
       -v)
            echo "ODP 2.4.0 INSTALL PACKAGE"
            exit 0
            ;;
       --help)
            usage
            exit 0
            ;;
       *)
            usage
            exit 1
            ;;
    esac
    shift
done

if [ -d "$ODP_ROOT" ]
then
    echo -e "\033[31;1m [ERROR OCCURRED] \033[37;1m"
    echo -e "\033[32;1m The PATH to install ODP environment is already exits. \033[37;1m"
    echo -e "\033[32;1m Please use -d option to specify a valid path... \033[37;1m"
    exit 3
fi

if [ -z "$ODP_ROOT" ]
then
    #JUMBO_ROOT="$DEFAULT_JUMBO_ROOT"
    echo -e "\033[31;1m [ERROR OCCURRED] \033[37;1m"
    echo -e "\033[32;1m You did not specified a PATH to install ODP environment. \033[37;1m"
    echo -e "\033[32;1m Please use -d option to specify a valid path... \033[37;1m"
    exit 3
fi

if [ -z "$ODP_TYPE" ]
then
    echo -e "\033[31;1m [ERROR OCCURRED] \033[37;1m"
    echo -e "\033[32;1m You did not specified the ODP environment's type. \033[37;1m"
    echo -e "\033[32;1m Please use -t option to specify a valid type... \033[37;1m"
    echo -e "\033[32;1m There has two types for you to choose: -t=online / -t=develop \033[37;1m"
    exit 4
fi

if [ "$ODP_TYPE" != "online" ] && [ "$ODP_TYPE" != "develop" ]
then
    echo -e "\033[31;1m [ERROR OCCURRED] \033[37;1m"
    echo -e "\033[32;1m You did not specified a valid  ODP environment type. \033[37;1m"
    echo -e "\033[32;1m Please use -t option to specify a valid type... \033[37;1m"
    echo -e "\033[32;1m There has two types for you to choose: -t=online / -t=develop \033[37;1m"
    exit 4
fi

if [ -z "$ODP_SERVER" ]
then
    echo -e "\033[31;1m [ERROR OCCURRED] \033[37;1m"
    echo -e "\033[32;1m You did not specified the ODP environment's webserver. \033[37;1m"
    echo -e "\033[32;1m Please use -s option to specify a valid webserver... \033[37;1m"
    echo -e "\033[32;1m There has two types for you to choose: -s=lighttpd / -s=nginx \033[37;1m"
    exit 4
fi

if [ "$ODP_SERVER" != "lighttpd" ] && [ "$ODP_SERVER" != "nginx" ]
then
    echo -e "\033[31;1m [ERROR OCCURRED] \033[37;1m"
    echo -e "\033[32;1m You did not specified a valid ODP environment's webserver. \033[37;1m"
    echo -e "\033[32;1m Please use -s option to specify a valid webserver... \033[37;1m"
    echo -e "\033[32;1m There has two types for you to choose: -s=lighttpd / -s=nginx \033[37;1m"
    exit 4
fi

if [ "${ODP_ROOT:0:1}" != "/" ]
then
    JUMBO_ROOT="$PWD/$JUMBO_ROOT"
else
    JUMBO_ROOT=$ODP_ROOT
fi

mkdir -p "$JUMBO_ROOT"
cd jumbo
./jumbo-0.6.3 -n -r "$JUMBO_ROOT" -p "$JUMBO_REPO" local-install jumbo.jumbo
cd ../

$JUMBO_ROOT/bin/jumbo update jumbo &>/dev/null

echo 'ftp://getprod:getprod@product.scm.baidu.com/data/prod-64/inf/odp/jumbo-repo/jumbo-repo_1-0-0_BL/output' >>$JUMBO_ROOT/etc/jumbo-repo.conf

odp_build_repo

#cd packages
$JUMBO_ROOT/bin/jumbo -p "file://$PWD" check
$JUMBO_ROOT/bin/jumbo -p "file://$PWD" install odp-${ODP_TYPE}-${ODP_SERVER}

END=`date +%s`;
echo Time costs : `expr $END - $START`s
