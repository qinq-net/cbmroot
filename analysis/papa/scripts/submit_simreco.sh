#!/bin/bash

## test train or batch submission
TEST=0

## number of events: default=1000
NEVENT=1000

## number of runs
NRUNS=2000

## storage element and directory
LOCATION=/lustre/nyx
DIR=sim

## test parameters
if [[ $TEST == "1" ]] ; then
    DIR="TEST${DIR}"
    NEVENT=2
    NRUNS=1
fi


## output directory for simulated and reconstructed data
outdir=$LOCATION/cbm/users/$USER/$DIR/sim_AA_UrQMD_eeCocktail_centr010_JUN16_25mum_4lay_wMVD
mkdir -p "$outdir"

## source cbm root environment
. $LOCATION/cbm/users/$USER/CBMsoft/cbm-env.sh -n

## choose CBM setup for simulation and reconstruction
export SETUP="sis100_electron"      #"sis100_muon_jpsi"

## copy cbm setup macro and simulation and reconstruction macros to output directory
## for backup and documentation
cp -nv "$HOME/.rootrc"                                           "$outdir/."
cp -nv "$VMCWORKDIR/geometry/setup/setup_${SETUP}.C"             "$outdir/."
cp -nv "./run_sim_new.C"                                         "$outdir/."
cp -nv "./run_reco_new.C"                                        "$outdir/."

## loop over all runs
I=0
while [ "$I" -lt "$NRUNS" ]; do

  ## create output directory for job $I
  out=$outdir/`printf "%05d" $I`
  mkdir -p "$out"

  ## test analysis (abort after first job)
  if [[ $TEST == "1" ]] ; then

      ./simreco.sh $out $NEVENT     # LOCAL run
      break;

  else
      #### gsi batch environement for kronos (slurm) ####
      #### NOTE: possible partitions: sinfo -o "%9P  %6g %10l %5w %5D %13C %N"
      ####                      and : scontrol show partition main
      #### long: upto 7d (CPUs=3880)
      #### main upto 8h (CPUs=7840)
      #### debug: upto 20m (CPUs=200)
      #### memory max 4096M, default 2048M

      ## configure batch job
      tmpjobscriptname=$PWD/sr_$(basename $outdir)_$I.sh
      ####
      echo "#! /bin/sh"                                 > $tmpjobscriptname
      echo "#SBATCH --output=$out/simreco.slurm.out"    >> $tmpjobscriptname
      echo "#SBATCH --error=$out/simreco.slurm.err"     >> $tmpjobscriptname
      echo "#SBATCH --job-name=sr_${I}"                 >> $tmpjobscriptname
      echo "#SBATCH --mail-type=END"                    >> $tmpjobscriptname
      echo "#SBATCH --mail-user=j.book"                 >> $tmpjobscriptname
      echo "#SBATCH --mem-per-cpu=4000"                 >> $tmpjobscriptname
      echo "#SBATCH --time=1-05:00:00"                  >> $tmpjobscriptname
      echo "#SBATCH --partition=long"                   >> $tmpjobscriptname
      ####
      echo "srun ./simreco.sh $out $NEVENT"                   >> $tmpjobscriptname

      chmod u+x $tmpjobscriptname

      ## submit job
      sbatch $tmpjobscriptname
  fi

  let I=$I+1

done

