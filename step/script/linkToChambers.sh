#!/bin/zsh
# Creates web pages and symbolic links to analysis results in a chamber-oriented directory structure
# Usage:
#   linkToChambers <results_dir> <dir_for_symlinks>
# where
#   results_dir      : contains the Test_* directories created by the analyzer
#   dir_for_symlinks : is where the chamber-oriented directory structure should be created for the symlinks

# Example for analysis results directory structure:
#   Test_CFEB04/csc_00000001_EmuRUI01_Test_17b_000_130531_092346_UTC.plots/ME+2.1.03
# Symlink to it in a chamber-oriented directory structure:
#   ME+2.1.03/Test_CFEB04/csc_00000001_EmuRUI01_Test_17b_000_130531_092346_UTC/plots
# Web page in a chamber-oriented directory structure:
#   ME+2.1.03/Test_CFEB04/csc_00000001_EmuRUI01_Test_17b_000_130531_092346_UTC/index.html

function generateWebPage(){
    # argument 1: directory
    cd $1
    {
	print "<!DOCTYPE html>"
	print "<html>"
	print "<head><title>${1:t}</title><head>"
	print "<style>"
	print "body { background-color:#ffffff; }"
	print "table { background-color:#ffffff; border-collapse: separate; }"
	print "td, th { color:#000000; font-size:90%; }"
	print "th { background-color:#dddddd; }"
	print "td { background-color:#eeeeee; text-align:left; padding-left: 10px; padding-right: 10px; }"
	print "</style>"
	print "<body>"
	print "<a id=\"top\"/>"
	print "<table style=\"font-size:small;\"><tr><th>${1:t}</th></tr>"
	print "<tr><td>"
	for IMAGEFILE in ./plots/*.png(N); do
	    PLOTCODE=${${IMAGEFILE:r}##*_}
	    print "<a href=\"#csc_$PLOTCODE\">$PLOTCODE</a> |"
	done
	print "</td></tr>"
	for IMAGEFILE in ./plots/*.png(N); do
	    PLOTCODE=${${IMAGEFILE:r}##*_}
	    print "<tr><td><a id=\"csc_$PLOTCODE\" href=\"#top\"><img src=\"$IMAGEFILE\" alt=\"$IMAGEFILE\"></a></td></tr>"
	done
	print "<table>"
	print "</body>"
	print "</html>"
    } > index.html
    cd -
}

if [[ $# -ne 2 || ${1[1]} != "/" || ${2[1]} != "/" ]]; then
    print "Need two full paths as arguments. Exiting.\nUsage:\n  $0 <results_dir> <dir_for_symlinks>"
    exit 1
fi

# Remove old chamber-oriented directory structure...
OLDDIRS=( $2/(crate|DDU_|EMU|ME(+|-)(1|2|3|4)\.(1|2|3)\.)*(/N) )
[[ ${#OLDDIRS} -gt 0 ]] && rm -rf $OLDDIRS

# ...and recreate it for all but tests 27 (Cosmics) and 40 (Beam)
setopt extendedglob # needed for the ~ glob operator to work (to exclude pattern)
OLDIFS=$IFS
IFS='/'
for DIR in $1/Test_*/csc_*.plots/*~*Test_(27|40)*(/); do
    DIRARRAY=( ${=DIR} )
    mkdir -p $2/${DIRARRAY[-1]}/${DIRARRAY[-3]}/${DIRARRAY[-2]%.plots}
    [[ -L $2/${DIRARRAY[-1]}/${DIRARRAY[-3]}/${DIRARRAY[-2]%.plots}/plots ]] || ln -s $DIR $2/${DIRARRAY[-1]}/${DIRARRAY[-3]}/${DIRARRAY[-2]%.plots}/plots
    generateWebPage $2/${DIRARRAY[-1]}/${DIRARRAY[-3]}/${DIRARRAY[-2]%.plots}
done
IFS=$OLDIFS

# Tests 27 (Cosmics) and 40 (Beam) are special in that runEmuCSCAnalyzer.exe creates a web page for their results. We just need to link to it.
for TESTNAME in Test_27_Cosmics Test_40_Beam; do
    for CHAMBERLISTFILE in $1/$TESTNAME/csc_*.plots/chambers.txt(N); do
	for CHAMBERNAME in $( cat $CHAMBERLISTFILE ); do
	    CHAMBER=${CHAMBERNAME//\//.} # replace slashes with dots
	    mkdir -p $2/$CHAMBER/$TESTNAME
	    [[ -L $2/$CHAMBER/$TESTNAME/${CHAMBERLISTFILE:h:t} ]] || ln -s ${CHAMBERLISTFILE:h} $2/$CHAMBER/$TESTNAME/${CHAMBERLISTFILE:h:t}
	done
    done
done
