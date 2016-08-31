#!/bin/bash
# shell script to iterate clusterizer calibrations 

# -----------------------------------------------------------------------------
# gen_digi.sh will repeat the last step in "iter_calib.sh", similar to running 
# the ana_digi.C('$inOpt', 1, "'$cRun'", '$iCalSet', 0, '$iSel2') interactively.
# Where $inOpt= 100000000, 93, 1, $iRef in the iter_calib.sh
# -----------------------------------------------------------------------------

#cRun=$1
#cRun='CbmTofPiHd_09Aug1533'
#cRun='CbmTofPiHd_11Aug1224'
#cRun='CbmTofPiHd_13Aug0855'
#cRun='CbmTofPiHd_15Aug0818'
#cRun='CbmTofPiHd_17Aug1724'
cRun='CbmTofPiHd_22Aug1616'

#iDut=921; iRef=920; iSel2=-300
#((iSet=$iDut*1000+$iRef))
  
c0='00000'

#iCalSet=$2
iCalSet=901900921

((iTmp  = $iCalSet ))
((iBRef = $iTmp % 1000))
((iTmp  = $iTmp - $iBRef))
((iSet  = $iTmp / 1000))
((iRef  = $iTmp % 1000000))
((iRef  = $iRef / 1000))
((iTmp  = $iTmp - $iRef))
((iDut  = $iTmp / 1000000))

#iSel2=$3
iSel2=-921

if((${iSel2}<0));then
 ((iBRef=-$iSel2))
fi

echo gen_digi with iDut=$iDut, iRef=$iRef, iCalSet=$iCalSet, iSet=$iSet, iBRef=$iBRef, iSel2=$iSel2

# void ana_digi(nEvents, calMode, calSel, calSm, RefSel, cFileId, iCalSet, bOut, iSel2) 
root -b -q './ana_digi.C(100000000,93,1,'$iRef',0,"'$cRun'",'$iCalSet',1,'$iSel2') '

