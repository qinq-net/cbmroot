#!/bin/bash

LOCATION=/lustre/nyx

## choose cbm root installation
. $LOCATION/cbm/users/$USER/CBMsoft/cbm-env.sh -n

## go to directory with macros
export OUTDIR="$1/"
mkdir -p $OUTDIR
cd "$OUTDIR/../."

## get cbm setup macro forseen
export SETUP=$(basename $OUTDIR/../setup* .C)
SETUP=${SETUP/setup_/}
echo "cbm setup is: $SETUP"

## number of events to process
NEVT=$2

## check files in output directory
echo "$OUTDIR"
ls -lhSr "$OUTDIR"

## simulation
if [ -f "$1/run_sim_${SETUP}_ok" ] ; then
    echo "mc file was produced already";
else
    echo "start mc generation";
    rm -rfv "$1/${SETUP}_mc.root"
    rm -rfv "$1/${SETUP}_reco.root"
    root -l -b -q "./run_sim_new.C( $NEVT,\"$SETUP\")"
fi


## reconstruction
if [ -f "$1/run_reco_${SETUP}_ok" ] ; then
    echo "reco file was produced already";
elif [ -f "$1/run_sim_${SETUP}_ok" ] ; then
    echo "start reconstruction";
    root -l -b -q "./run_reco_new.C( $NEVT,\"$SETUP\")"
    echo
else
    echo "mc production was not succesfull!";
fi

## validation
if [ -f "$1/run_reco_${SETUP}_ok" ] ; then
    echo
    ls -lhSr "$1"
    echo "everything done";
else
    echo "validation failed!"
    rm -rfv "$1/${SETUP}_reco.root"
    if [ -f "$1/run_sim_${SETUP}_ok" ] ; then
	echo
	ls -lhSr "$1"
	echo "simulation okay";
    else
	rm -rfv "$1/${SETUP}_mc.root"
    fi
fi


