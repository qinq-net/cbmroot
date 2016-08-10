#!/bin/bash

## test train or batch submission
TEST=0

## split level: defines number of files per jobs
split_level=10;

## number of events: 0=all
NEVENT=0

## unique train id
train="train0001"

## storage element and directory
LOCATION=/lustre/nyx
DIR=sim

## test parameters
if [[ $TEST == "1" ]] ; then
    train=${train/train/test};
    split_level=1;
    NEVENT=100
fi


## input directory of simulated and reconstructed data
indir=$LOCATION/cbm/users/$USER/$DIR/sim_AA_UrQMD_eeCocktail_centr010_JUN16_25mum_4lay_wMVD

## output directory
out=$indir/$train
mkdir -p $out

## path for analysis configs
configPath="$LOCATION/cbm/users/$USER/papa-conf"

## get CBM setup used in simulation and reconstruction
SETUP=$(basename $indir/setup* .C)
SETUP=${SETUP/setup_/}

## copy user config and analysis macro to train output directory
## for backup and documentation
cp -uv "$configPath/"*.C     "$out/."
cp -uv "$PWD/run_analysis.C" "$out/."

## get run list and sort it
if grep -Fq "$indir/" runListSort.txt; then
    echo "sorted run list already there, only split the list";
else
    ## only collect file with validated reconstruction output
    if [[ $TEST == "1" ]] ; then
	find $indir -type f -name "run_reco_${SETUP}_ok" -exec dirname {} \; -quit > runList.txt;
    else
	find $indir -type f -name "run_reco_${SETUP}_ok" -exec dirname {} \; > runList.txt;
    fi
    sort runList.txt > runListSort.txt
fi

## split file list by split_level into many files (1 list per job)
split -a 4 -l $split_level runListSort.txt filelist_${train}_
ListOfFilelist=`ls filelist_${train}_*`

# loop over all lists
I=0
for filelist in $ListOfFilelist ; do
    let I=$I+1

    ## create output directory for job $I
    outdir=$out/`printf "%04d/" $I`
    echo "-$outdir-"
    mkdir -p $outdir

    ## test analysis (abort after first job)
    if [[ $TEST == "1" ]] ; then

	# clean up
	rm $(ls filelist_${train}_* | grep -v aaaa)
	rm -v runListSort.txt;
	./analysis.sh $indir $filelist $outdir $NEVENT     # LOCAL run
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
	tmpjobscriptname=$PWD/ana_${train}_$I.sh
	####
	echo "#! /bin/sh"                                   > $tmpjobscriptname
	echo "#SBATCH --output=$outdir/ana.slurm.out"       >> $tmpjobscriptname
	echo "#SBATCH --error=$outdir/ana.slurm.err"        >> $tmpjobscriptname
	echo "#SBATCH --job-name=ana_${train}_${I}"         >> $tmpjobscriptname
	echo "#SBATCH --mail-type=FAIL"                     >> $tmpjobscriptname
	echo "#SBATCH --mail-user=j.book@gsi.de"            >> $tmpjobscriptname
	echo "#SBATCH --mem-per-cpu=2000"                   >> $tmpjobscriptname
	echo "#SBATCH --time=0-03:00:00"                    >> $tmpjobscriptname
	echo "#SBATCH --partition=main"                     >> $tmpjobscriptname
	####
	echo "srun ./analysis.sh $indir $filelist $outdir $NEVENT"  >> $tmpjobscriptname

	chmod u+x $tmpjobscriptname

	## submit job
	sbatch $tmpjobscriptname

    fi


done
