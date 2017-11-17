#!/bin/zsh

# user's options
while [[ $1 = -* ]]; do
    case $1 in 
        -d ) RUNINDEBUGGER="true"
            shift ;;
         * ) cat <<EOF
Usage:
    $0 [-d]

Options:
    -d    Run in a debugger (ddd)
EOF
	    exit 1 ;;
    esac
done

# Allow core dumps
ulimit -c unlimited

# BUILD_HOME, XDAQ_ROOT and ROOTSYS should be set. Their default values are:
# export BUILD_HOME=${BUILD_HOME:-$PWD/${0:h}/../../..}
export BUILD_HOME=${BUILD_HOME:-$HOME/TriDAS}
export XDAQ_ROOT=${XDAQ_ROOT:-$HOME/XDAQ}
export ROOTSYS=${ROOTSYS:-$HOME/ROOT/root}

export XDAQ_OS=linux
[[ $(uname -m) == "x86_64" ]] && XDAQ_PLATFORM="x86_64" || XDAQ_PLATFORM="x86"
if [[ ${#DISTRIB_ID} -gt 0 ]]; then
    XDAQ_PLATFORM=${XDAQ_PLATFORM}_$DISTRIB_ID
elif [[ -f /etc/issue ]]; then
    XDAQ_PLATFORM=${XDAQ_PLATFORM}_$(/bin/sed -n -e 's/[^0-9]*\([0-9]\).[0-9]\+[^0-9]*/slc\1/p' /etc/issue)
fi
export XDAQ_PLATFORM
export XDAQ_DOCUMENT_ROOT=${XDAQ_ROOT}/htdocs
export DQMCONFIG=$HOME/config/dqm
export LD_LIBRARY_PATH=$ROOTSYS/lib:$XDAQ_ROOT/lib

print
print "Environment variables:"
print "    BUILD_HOME         = $BUILD_HOME"
print "    XDAQ_ROOT          = $XDAQ_ROOT"
print "    XDAQ_OS            = $XDAQ_OS"
print "    XDAQ_PLATFORM      = $XDAQ_PLATFORM"
print "    XDAQ_DOCUMENT_ROOT = $XDAQ_DOCUMENT_ROOT"
print "    ROOTSYS            = $ROOTSYS"
print "    DQMCONFIG          = $DQMCONFIG"
print "    LD_LIBRARY_PATH    = $LD_LIBRARY_PATH"
print

OPTIONS="\
 -h $HOST \
 -p 10000 \
 -c ${HOME}/config/step/AllInOne_STEP.xml \
 -e ${HOME}/etc/default.profile"

#OPTIONS="\
# -h $HOST \
# -p 10000 \
# -c ${BUILD_HOME}/emu/step/xml/EmuSTEP.xml \
# -e ${BUILD_HOME}/emu/step/xml/EmuSTEP.profile"

#OPTIONS="\
# -h emu42fastprod01.cern.ch \
# -p 10000 \
# -c /home/cscupdate/config/fm/Commissioning/startNewSTEP.xml \
# -e ${BUILD_HOME}/emu/step/xml/EmuSTEP.profile"

if [[ $RUNINDEBUGGER = "true" ]]; then
    print "Execute in ddd the command:"
    print "run $OPTIONS"
    print
    eval "ddd ${XDAQ_ROOT}/bin/xdaq.exe"
else
    print "Executing:"
    print "${XDAQ_ROOT}/bin/xdaq.exe $OPTIONS"
    print
    eval "${XDAQ_ROOT}/bin/xdaq.exe $OPTIONS"
fi
