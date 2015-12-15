#!/bin/bash
# shell script to iterate Analyzer histograms
cRun='CbmTofSps_29Nov2103'
iDut=921; iRef=920; iSel2=-300
#iSel=94; iSet=943; iSel2=3

iTraSetup=0;
# what should be done ?
((iSel=$iDut*1000+$iRef))
#which file should be analyzed ?  
cSet="921920_-300";

dDTres=10000000

while [[ $dDTres > 0 ]]; do

for iCal in 1 2 3 4 1
do
root -b -q 'ana_hits.C(10000000,'$iSel','$iCal',"'$cRun'","'$cSet'",'$iSel2','$iTraSetup')'

cp -v tofAnaTestBeam.hst.root ${cRun}_${cSet}_${iSel}_${iSel2}_tofAnaTestBeam.hst.root
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
