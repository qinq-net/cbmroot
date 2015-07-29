#!/bin/bash
# shell script to iterate Analyzer histograms

#cRun='CernSps02Mar2227_hdref_200_hdp2_220_thupad_170_thustrip_23c_bucref_160_buc2013_160_ustc_220_diam_020_nb_up'
#cRun='CernSps02Mar2202_hdref_200_hdp2_220_thupad_170_thustrip_23c_bucref_160_buc2013_160_ustc_220_diam_030_nb_up'
#cRun='CernSps02Mar2151_hdref_200_hdp2_220_thupad_170_thustrip_23c_bucref_160_buc2013_160_ustc_220_diam_030_nb_up'
#cRun='CernSps02Mar2120_hdref_200_hdp2_220_thupad_170_thustrip_23c_bucref_160_buc2013_160_ustc_220_diam_010_nb_up'
#cRun='CernSps02Mar2056_hdref_200_hdp2_220_thupad_170_thustrip_23c_bucref_160_buc2013_160_ustc_220_diam_000_nb_up'
#cRun='CernSps02Mar2044_hdref_200_hdp2_220_thupad_170_thustrip_23c_bucref_160_buc2013_160_ustc_220_diam_000_nb_up'
#cRun='CernSps02Mar2031_hdref_200_hdp2_220_thupad_170_thustrip_23c_bucref_160_buc2013_160_ustc_220_diam_000_nb_up'
#cRun='CernSps02Mar2024_hdref_200_hdp2_220_thupad_170_thustrip_23c_bucref_160_buc2013_160_ustc_220_diam_000_nb_up'
cRun='CernSps01Mar1031_hdref_3ff_hdp2_220_thupad_170_thustrip_23c_bucref_0C0_buc2013_140_ustc_220_diam_0c0_nb'

iSel=94
iSet=943
iSel2=3
dDTres=1000000

while [[ $dDTres > 0 ]]; do

for iCal in 1 2 3 4 1
do
root -b -q 'ana_hits.C(10000000,'$iSel','$iCal',"'$cRun'",'$iSet','$iSel2') '
cp -v tofAnaTestBeam.hst.root ${cRun}_${iSet}${iSel2}_${iSel}_tofAnaTestBeam.hst.root
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
