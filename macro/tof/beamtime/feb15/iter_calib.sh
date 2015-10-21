# shell script to iterate clusterizer calibrations 
cRun='CernSps05Mar0041'
#cRun='CernSps05Mar0041_up'
#cRun='CernSps03Mar0112_up'
#cRun='CernSps02Mar2227_hdref_200_hdp2_220_thupad_170_thustrip_23c_bucref_160_buc2013_160_ustc_220_diam_020_nb_up'
#cRun='CernSps02Mar2202_hdref_200_hdp2_220_thupad_170_thustrip_23c_bucref_160_buc2013_160_ustc_220_diam_030_nb_up'
#cRun='CernSps02Mar2151_hdref_200_hdp2_220_thupad_170_thustrip_23c_bucref_160_buc2013_160_ustc_220_diam_030_nb_up'
#cRun='CernSps02Mar2120_hdref_200_hdp2_220_thupad_170_thustrip_23c_bucref_160_buc2013_160_ustc_220_diam_010_nb_up'
#cRun='CernSps02Mar2056_hdref_200_hdp2_220_thupad_170_thustrip_23c_bucref_160_buc2013_160_ustc_220_diam_000_nb_up'
#cRun='CernSps02Mar2044_hdref_200_hdp2_220_thupad_170_thustrip_23c_bucref_160_buc2013_160_ustc_220_diam_000_nb_up'
#cRun='CernSps02Mar2031_hdref_200_hdp2_220_thupad_170_thustrip_23c_bucref_160_buc2013_160_ustc_220_diam_000_nb_up'
#cRun='CernSps02Mar2024_hdref_200_hdp2_220_thupad_170_thustrip_23c_bucref_160_buc2013_160_ustc_220_diam_000_nb_up'
#cRun='CernSps01Mar2203_hdref_200_hdp2_220_thupad_170_thustrip_23c_bucref_160_buc2013_160_ustc_220_diam_0c0_nb_up'
#cRun='CernSps01Mar2203_hdref_200_hdp2_220_thupad_170_thustrip_23c_bucref_160_buc2013_160_ustc_220_diam_0c0_nb'
#cRun='CernSps01Mar1253_hdref_3ff_hdp2_220_thupad_170_thustrip_23c_bucref_160_buc2013_160_ustc_220_diam_0c0_nb'
#cRun='CernSps28Feb0448_hdref_210_hdp2_220_thupad_170_thustrip_23c_bucref_140_buc2013_140_ustc_210_diam_0a0_nb_up'
#cRun='CernSps27Feb0510_hdref_3ff_hdp2_260_thupad_140_thustrip_220_bucref_140_buc2013_140_ustc_000_diam_0c0_nb'
#cRun='CernSps26Feb1603_hdref_3ff_hdp2_220_thupad_180_thustrip_180_bucref_140_buc2013_140_ustc_09e_diam_080_nb'
#cRun='Trb25Feb0847'
#cRun='Trb24Feb2028'

#iDut=3; iRef=9; iSel2=-7
#iDut=9; iRef=4; iSel2=-3
#iDut=9; iRef=7; iSel2=-3
#iDut=6; iRef=1;   iSel2=-8
iDut=6; iRef=1; iSel2=8;  
c0='00000'

((iSet=$iDut*10+$iRef))

if((${iSel2}<0));then
 ((iBRef=-iSel2))
 else
 iBRef=5
 fi

echo iDut=$iDut, iRef=$iRef, iSet=$iSet, iSel2=$iSel2, iBRef=$iBRef

# history for 1805: 1 6 8 2 10 2 10
for mode in 1 6 8 2 10 4 5 4
#for mode in 2 10 2 10 
#for mode in 4
#for mode in 5
#for mode in 1
do

case $mode in 
1)
nIter=2
alist=`echo '1'$c0'0,93,1,'$iRef'' '1'$c0'0,64,1,'$iDut'' '1'$c0'0,24,1,'$iBRef'' '1'$c0'0,93,0,'$iDut'' '1'$c0'0,64,0,'$iRef'' '1'$c0'0,24,0,'$iBRef'' '1'$c0'00,41,1,-'$iRef'' '1'$c0'0,93,1,'$iRef''`
;;
2)
nIter=1
alist=`echo '1'$c0'0,92,1,'$iRef'' '1'$c0'00,64,1,-'$iRef'' '1'$c0'00,64,1,'$iBRef'' '1'$c0'0,92,0,'$iDut'' '1'$c0'00,64,0,-'$iDut'' '1'$c0'00,64,0,'$iBRef'' '1'$c0'0,92,1,'$iRef''`
;;
3)
nIter=3
alist=`echo '1'$c0',93,1,4' `
;;
4)
nIter=5
alist=`echo '5'$c0'0,93,1,'$iRef'' '5'$c0'0,93,-2,2' '1'$c0'000,61,-1,-2' '5'$c0'0,93,-3,2' ` 
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
nIter=2
alist=`echo '1'$c0'0,93,1,'$iRef'' '1'$c0'0,92,0,'$iDut'' '1'$c0'00,61,-1,-2' '1'$c0'00,93,-2,2' '1'$c0'0,93,1,'$iRef''`
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

echo 'Iterate Clusterizer calibration for run '$cRun, Set ${iSet} - execute $nIter iterations of ${alist}
#lastOpt=''

while [[ $nIter > 0 ]]; do
for inOpt in $alist 
do

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
cp -v tofTestBeamClust_${cRun}_set${iSet}.hst.root ${cRun}_set${iSet}_${cMode}_${cSel}tofTestBeamClust.hst.root
fi 

# generate new calibration file
root -b -q 'ana_digi.C('$inOpt',0,"'$cRun'",'$iSet',0,'$iSel2') '

lastOpt=$inOpt

#./screenshot.sh 

done
(( nIter -= 1))
done
cp -v tofTestBeamClust_${cRun}_set${iSet}.hst.root ${cRun}_set${iSet}_${cMode}_${cSel}${mode}tofTestBeamClust.hst.root

done 
#
