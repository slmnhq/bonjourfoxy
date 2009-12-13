#!/bin/bash
function exiterror()  {
    echo ${2}
    exit ${1}
}
if [ $# -ne 2 ]; then
    exiterror 1 "Usage: $0 [firefox_rel] [xulrunnersdk_rel]"
fi
ff_rel=${1}
sdk_rel=${2}
basedir=`dirname $0`
archivedir=${basedir}/sdk/archive
outbase=${basedir}/sdk/${ff_rel}
pre=xulrunner-${sdk_rel}.en-US
platforms="win32 linux macppc maci386"
win32=${pre}.win32.sdk.zip
linux=${pre}.linux-i686.sdk.tar.bz2
macppc=${pre}.mac-powerpc.sdk.tar.bz2
maci386=${pre}.mac-i386.sdk.tar.bz2
for platform in ${platforms}; do
    if [ ! -f ${archivedir}/${!platform} ]; then
        exiterror 1 "Archive ${archivedir}/${!platform} missing"
    fi
done
for platform in ${platforms}; do
    platformdir=${outbase}/${platform}
    echo ${archivedir}/${!platform} -\> ${platformdir}
    mkdir -p ${platformdir}
    if [ "$platform" == "win32" ]; then
        cmd="unzip -q -o ${archivedir}/${!platform} -d ${platformdir}"
    else
        cmd="tar -xjf ${archivedir}/${!platform} -C ${platformdir}"
    fi
    ${cmd}
    exitcode=$?
    if [ ${exitcode} -ne 0 ]; then
        exiterror ${exitcode} "Failed to extract ${archivedir}/${!platform} (${exitcode})"
    fi
done
