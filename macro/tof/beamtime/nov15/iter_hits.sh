#!/bin/bash
# shell script to iterate Analyzer histograms
cRun=$1
#cRun='CbmTofSps_29Nov2103'
iDut=$2; iRef=$3; iSel2=$4
#iDut=921; iRef=920; iSel2=-300
#iSel=94; iSet=943; iSel2=3

iTraSetup=0;
# what should be done ?
((iSel=$iDut*1000+$iRef))
#which file should be analyzed ?  
cSet=$5
#cSet="921920_-300";

dDTres=10000000

while [[ $dDTres > 0 ]]; do

cSel2=$iSel2;
if [[ $iSel2 = 0 ]]; then
cSel2="000"
fi

for iCal in 1 2 3 4 1
do
root -b -q 'ana_hits.C(1000000,'$iSel','$iCal',"'$cRun'","'$cSet'",'$iSel2','$iTraSetup')'

cp -v tofAnaTestBeam.hst.root ${cRun}_${cSet}_${iSel}_${cSel2}_tofAnaTestBeam.hst.root
done

Tres=`cat Test.res`
dTdif=`echo "$dDTres - $Tres" | bc`
compare_result=`echo "$Tres < $dDTres" | bc`

echo got Tres = $Tres, compare to $dDTres, dTdif = $dTdif, compare_result = $compare_result

if [[ $compare_result > 0 ]]; then
dDTres=$Tres
else
dDTres=0
fi

done
