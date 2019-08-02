#!/bin/bash
export JAVA_HOME=/etc/alternatives/jre
export RCMS_HOME=/home/cscdev/RunControl
export CATALINA_HOME=/opt/apache-tomcat-5.5.28/
export ANT_HOME=/opt/ant

# For DQM:
export ROOTSYS=/opt/cern/root_v5.34.36
export DQMCONFIG=$HOME/config/dqm
export SQLITE=$HOME/sqlite

# For XDAQ
export XDAQ_OS=linux
slc=$(/bin/sed -n -e 's/[^0-9]*\([0-9]\).[0-9]\+[^0-9]*/slc\1/p' /etc/issue)
[[ $(uname -m) == "x86_64" ]] && export XDAQ_PLATFORM="x86_64_$slc" || export XDAQ_PLATFORM="x86_$slc"
export $XDAQ_PLATFORM
export XDAQ_ROOT=/opt/xdaq
export XDAQ_SETUP_ROOT=$XDAQ_ROOT/share
export XDAQ_DOCUMENT_ROOT=$XDAQ_ROOT/htdocs
#export XDAQ_DOCUMENT_ROOT=/home/cscdev/pakhotin/TriDAS/xdaq-htdocs
#export XDAQ_DOCUMENT_ROOT=/home/cscdev/TriDAS/x86_64_slc6/htdocs
export xdaqPWD=$PWD/../

#export BUILD_HOME=/home/cscdev/pakhotin/TriDAS
export BUILD_HOME=/home/cscdev/TriDAS
export LD_LIBRARY_PATH=${BUILD_HOME}/${XDAQ_PLATFORM}/lib:$ROOTSYS/lib:$XDAQ_ROOT/lib:$LD_LIBRARY_PATH:$SQLITE/lib
export PATH=/usr/kerberos/bin:$JAVA_HOME/bin:$ROOTSYS/bin:$ANT_HOME/bin:$XDAQ_ROOT/bin:$PATH

