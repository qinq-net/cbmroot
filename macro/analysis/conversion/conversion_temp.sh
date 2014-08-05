#!/bin/bash

export NOFEVENTS=500
export mode=2

DATE=$(date +"%H%M%S")

SIM="run_sim_"$DATE".C"
REC="run_reco_"$DATE".C"
ANA="run_analysis_"$DATE".C"

SIM2="run_sim_"$DATE
REC2="run_reco_"$DATE
ANA2="run_analysis_"$DATE



cp run_sim.C $SIM
cp run_reco.C $REC
cp run_analysis.C $ANA

sed -i "s/run_sim/$SIM2/" $SIM
sed -i "s/run_reco/$REC2/" $REC
sed -i "s/run_analysis/$ANA2/" $ANA

echo "Start of full conversion simulation"
echo "-----------------------------------------------------------------------"
root -l -b -q "$SIM($NOFEVENTS, $mode)" || exit 11
root -l -b -q "$REC($NOFEVENTS, $mode)" || exit 12
root -l -b -q "$ANA($NOFEVENTS, $mode)" || exit 13
echo "-----------------------------------------------------------------------"
echo "Chain finished successfully"


rm -f $SIM
rm -f $REC
rm -f $ANA


