#!/bin/bash
# shell script to iterate Analyzer histograms

# -----------------------------------------------------------------------------
#cRun=$1
#cRun='CbmTofPiHd_09Aug1533'
#cRun='CbmTofPiHd_11Aug1224'
#cRun='CbmTofPiHd_13Aug0855'
#cRun='CbmTofPiHd_15Aug0818'
cRun='CbmTofPiHd_17Aug1724'


#iDut=$2; iRef=$3; iSel2=$4
iDut=901; iRef=900; iSel2=-921
#iDut=910; iRef=900; iSel2=-921

iTraSetup=0;

# what should be done ?
((iSel=$iDut*1000+$iRef))

#which file should be analyzed ?  
#cSet=$5
cSet=901900921_-921
#cSet=910900921_-921

#dScalFac=$6
dScalFac=1.0

# -----------------------------------------------------------------------------
dDTres=10000000
cSel2=$iSel2;
if [[ $iSel2 = 0 ]]; then
cSel2="000"
fi

cd ${cRun}
mkdir          Ana${cRun}_${cSet}_${iSel}_${cSel2}
cp rootlogon.C Ana${cRun}_${cSet}_${iSel}_${cSel2}/
cp .rootrc     Ana${cRun}_${cSet}_${iSel}_${cSel2}/
#cp ../${cRun}_${cSet}_${iSel}_${cSel2}_tofAnaTestBeam.hst.root Ana${cRun}_${cSet}_${iSel}_${cSel2}/
cd Ana${cRun}_${cSet}_${iSel}_${cSel2}

mkdir hst
rm ${cRun}_${cSet}_${iSel}_${cSel2}_tofAnaTestBeam.hst.root

while [[ $dDTres > 0 ]]; do

for iCal in 1 2 3 5 6 1
do
rm Test.res
root -b -q '../../ana_hits.C(10000000,'$iSel','$iCal',"'$cRun'","'$cSet'",'$iSel2','$iTraSetup','$dScalFac')'

mv -v tofAnaTestBeam.hst.root ${cRun}_${cSet}_${iSel}_${cSel2}_tofAnaTestBeam.hst.root
cp -v ${cRun}_${cSet}_${iSel}_${cSel2}_tofAnaTestBeam.hst.root ../../
rm all_*
if (! (test -f Test.res)); then
echo no resolution file available: return
return
fi
done

Tres=`cat Test.res`
dTdif=`echo "$dDTres - $Tres" | bc`
dDTres=`echo "$dDTres - 0.5" | bc`
compare_result=`echo "$Tres < $dDTres" | bc`

echo got Tres = $Tres, compare to $dDTres, dTdif = $dTdif, compare_result = $compare_result

if [[ $compare_result > 0 ]]; then
dDTres=$Tres
else
dDTres=0
fi

done	# end-while [[ $dDTres > 0 ]]

# final action -> scan full statistics 
root -b -q '../../ana_hits.C(1000000000,'$iSel',1,"'$cRun'","'$cSet'",'$iSel2','$iTraSetup','$dScalFac')'
#echo copy results from `pwd` ?
#cp -v ./hst/* ../../hst/
rm all_*
cd ../..
