#!/bin/bash

LOCATION=/lustre/nyx

## choose cbm root installation
source $LOCATION/cbm/users/$USER/CBMsoft/cbm-env.sh -n

## go to directory with macros
export OUTDIR="$1/"
mkdir -p $OUTDIR
cd $OUTDIR
echo $PWD

## copy rootrc
cp "../.rootrc" "."

## get cbm setup macro forseen
export SETUP=$(basename $OUTDIR/../setup* .C)
SETUP=${SETUP/setup_/}
echo "cbm setup is: $SETUP"

## user cuts in geant
export CONFIG_DIR=$DIR

## number of events to process
NEVT=$2

## check files in base directory
echo "$OUTDIR"
ls -lhSr "$OUTDIR"

## number of sub jobs
NJOBS=$3;
JOB=0;

while [ $JOB -lt $NJOBS ] ; do

    ## go to directory with macros
    export OUTDIR="$1$JOB/"
    mkdir -p $OUTDIR
    cd $OUTDIR
    echo $PWD

    ## check files in output directory
    echo "$OUTDIR"
    ls -lhSr "$OUTDIR"

    ## simulation
    if [ -f "$1$JOB/run_sim_${SETUP}_ok" ] ; then
	echo "mc file was produced already";
    else
	echo "start fast mc generation";
	rm -v "$1$JOB/${SETUP}_mc.root"
	root -l -b -q "$PWD/../run_sim_fast.C( $NEVT,\"$SETUP\")"

	# validate the root file
	if [ -f /cvmfs/fairroot.gsi.de/cbm/bin/validate_cbmroot.sh ] ; then
	    export MACRODIR="/cvmfs/fairroot.gsi.de/cbm/macros"
	    /cvmfs/fairroot.gsi.de/cbm/bin/validate_cbmroot.sh "$1$JOB/${SETUP}_mc.root" $NEVT
	    result=$?
	    if [ "$result" = "0" ]; then
		echo "fast mc production was succesfull!";
	    else
		echo "fast mc production was not succesfull!";
		rm -v "$1$JOB/run_sim_${SETUP}_ok"
		rm -v "$1$JOB/${SETUP}_mc.root"
	    fi
	fi
	echo
    fi

    #### next sub job
    let JOB=$JOB+1;

done

echo "all jobs done";
