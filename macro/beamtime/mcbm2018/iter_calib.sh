#!/bin/bash
# shell script to iterate clusterizer calibrations 
cRun=$1

c0='0000000'
#c0='00000'

iCalSet=$2
((iTmp  = $iCalSet ))
((iBRef = $iTmp % 1000))
((iTmp  = $iTmp - $iBRef))
((iSet  = $iTmp / 1000))
((iRef  = $iTmp % 1000000))
((iRef  = $iRef / 1000))
((iTmp  = $iTmp - $iRef))
((iDut  = $iTmp / 1000000))

iSel2=$3

if((${iSel2}<0));then
 ((iBRef=-$iSel2))
fi

echo iter_calib with iDut=$iDut, iRef=$iRef, iSet=$iSet, iSel2=$iSel2, iBRef=$iBRef

cCalSet="000010020"
if (( iCalSet<100000000 )); then 
cCalSet="0"$iCalSet
fi
if (( iCalSet<10000000 )); then 
cCalSet="00"$iCalSet
fi
if (( iCalSet<1000000 )); then 
cCalSet="000"$iCalSet
fi
if (( iCalSet<100000 )); then 
cCalSet="0000"$iCalSet
fi
echo cCalSet = $cCalSet

#mkdir ${cRun}
#cp rootlogon.C ${cRun}
#cp .rootrc ${cRun}
cd ${cRun}

iStep=0
iStepLast=0

#for mode in 4
#for mode in 5
#for mode in 1
for mode in 0 3 1 8 0
#for mode in 0 3 1 8 4 2 8 0

do

case $mode in 
0) 
nIter=1
alist=`echo '1'$c0',93,1,'$iRef''`
;;
1)
nIter=2
alist=`echo  '1'$c0'00,41,1,-'$iRef'' '1'$c0'0,93,0,'$iDut'' '1'$c0'00,41,0,-'$iDut'' '1'$c0'0,93,1,'$iRef''`
;;
2)
nIter=2
alist=`echo  '1'$c0'00,61,1,-'$iRef'' '1'$c0'0,93,0,'$iDut'' '1'$c0'00,61,0,-'$iDut'' '1'$c0'0,93,1,'$iRef''`
;;
3)
nIter=2
alist=`echo '1'$c0',93,0,'$iDut'' '1'$c0',44,0,-'$iDut''  '1'$c0',93,1,'$iRef'' '1'$c0',44,1,-'$iRef'' '1'$c0',93,1,'$iRef''`
;;
4)
nIter=2
alist=`echo '1'$c0',93,0,'$iDut'' '1'$c0',64,0,-'$iDut''  '1'$c0',93,1,'$iRef'' '1'$c0',64,1,-'$iRef'' '1'$c0',93,1,'$iRef''`
;;
5)
nIter=2
alist=`echo '5'$c0'0,93,1,'$iRef'' '5'$c0'0,93,-2,2' '1'$c0'00,64,1,-'$iRef'' '1'$c0'00,64,0,-'$iDut'' '5'$c0'0,93,-3,2' ` 
;;
6)
nIter=2
alist=`echo '1'$c0'0,93,1,'$iRef'' '1'$c0'00,93,-2,-2' '1'$c0'0,93,0,'$iDut'' '1'$c0'00,93,-2,-2' '1'$c0'0,93,1,'$iRef''` 
;;
7)
nIter=3
alist=`echo '1'$c0',92,1,4' '1'$c0'00,93,-2,-2' '1'$c0',92,0,3' '1'$c0'00,93,-2,2' '1'$c0',92,1,4'` 
;;
8)
nIter=4
alist=`echo '1'$c0'0,93,0,'$iDut'' '1'$c0'00,61,-1,-2' '1'$c0'0,93,-2,2' '1'$c0'0,93,1,'$iRef''`
;;
9)
nIter=2
alist=`echo '1'$c0',92,1,4' '1'$c0',92,0,3' '1'$c0'00,61,-1,-2' '1'$c0'0,93,-2,2' '1'$c0',92,1,4'`
;;
10)
nIter=1
alist=`echo '1'$c0'0,92,1,'$iRef'' '1'$c0'00,93,-3,2' '1'$c0'00,93,-2,2' '1'$c0'00,61,-1,-2' '1'$c0'0,92,1,'$iRef''`
;;
11)
nIter=1
alist=`echo '1'$c0',92,1,'$iRef'' '1'$c0'00,93,-3,2' '1'$c0'00,93,-2,2' '1'$c0'00,61,-1,-2' '1'$c0'0,92,1,'$iRef''`
;;
esac

echo 'Iterate Clusterizer calibration for run '$cRun, Set ${iCalSet} - execute $nIter iterations of ${alist}
#lastOpt=''

while [[ $nIter > 0 ]]; do
for inOpt in $alist 
do

((iStepLast = ${iStep}))
((iStep += 1))
mkdir Iter${iStep}
cp rootlogon.C Iter${iStep}
cp .rootrc Iter${iStep}
cd Iter${iStep}

if [[ ${iStep} = 1 ]]; then 
    lastOpt=$inOpt
fi 

if [[ ${lastOpt:+1} ]] ; then
# echo last round was done with $lastOpt, extract 2. and 3. word
i1=`expr index $inOpt , `
i2=($i1+3)
#echo `expr index $inOpt , ` = $i1
cMode=${inOpt:$i1:2}
cSel=${inOpt:$i2:1}
echo Next iteration: cMode=$cMode, cSel=$cSel 
if [[ ${cSel} = "-" ]];then 
    cSel=${inOpt:$i2:2}
    echo cSel=$cSel 
    cSel="0"
fi
#copy calibration file 

if [[ ${iStep} = 1 ]]; then 
    cp -v ../${cRun}_set${cCalSet}_${cMode}_${cSel}tofClust.hst.root .
else 
    cp -v ../Iter${iStepLast}/tofClust_${cRun}_set${cCalSet}.hst.root ${cRun}_set${cCalSet}_${cMode}_${cSel}tofClust.hst.root
fi
fi 


cSel2=$iSel2;
if [[ $iSel2 = 0 ]]; then
cSel2="000"
fi
cp -v  ../${cRun}_${cCalSet}${cSel2}_tofAnaCosmic.hst.root .

# generate new calibration file
root -b -q '../../ana_digi_cal.C('$inOpt',1,"'$cRun'",'$iCalSet',0,'$iSel2') '
#root -b -q '../../ana_digi_cos.C('$inOpt',1,"'$cRun'",'$iCalSet',0,'$iSel2') '
#root -b -q '../../ana_digi_star.C('$inOpt',1,"'$cRun'",'$iCalSet',0,'$iSel2') '

lastOpt=$inOpt

#./screenshot.sh 
cp -v tofClust_${cRun}_set${cCalSet}.hst.root ../${cRun}_set${cCalSet}_${cMode}_${cSel}tofClust.hst.root

cp -v tofAnaCosmic.hst.root ../${cRun}_${cCalSet}${cSel2}_tofAnaCosmic.hst.root
cp *pdf ../
cd .. 
rm ../${cRun}_set${cCalSet}_${cMode}_${cSel}tofClust.hst.root
ln -s ./${cRun}/${cRun}_set${cCalSet}_${cMode}_${cSel}tofClust.hst.root ../${cRun}_set${cCalSet}_${cMode}_${cSel}tofClust.hst.root

done
(( nIter -= 1))
done
#cp -v tofClust_${cRun}_set${cCalSet}.hst.root ${cRun}_set${cCalSet}_${cMode}_${cSel}${mode}tofClust.hst.root

done 
#
