#!/bin/sh

cbmroot_config_path=/hera/cbm/users/tariq/cbmroot/buildcbm/config.sh
source ${cbmroot_config_path}

nEvs=20000
RotMir=-10

UpperTransY=50;
UpperTransZ=50;
RX=20; RY=10
DefaultDims=1 
DefaultDimsLargePMT=0
for ((DeDi=1; DeDi<=1; DeDi++)); do			
    DefaultDims=$DeDi					
    for ((DeDiPMT=1; DeDiPMT<=1; DeDiPMT++)); do	
	DefaultDimsLargePMT=$DeDiPMT
	if [ $DeDi == 0 -a $DeDiPMT == 0 ];then UpperTransY=50; UpperTransZ=50;fi
	if [ $DeDi == 0 -a $DeDiPMT == 1 ];then continue; fi
	if [ $DeDi == 1 -a $DeDiPMT == 0 ];then UpperTransY=31; UpperTransZ=23;fi
	if [ $DeDi == 1 -a $DeDiPMT == 1 ];then UpperTransY=23; UpperTransZ=13;fi
	
	
	for ((Rm=1; Rm<=1; Rm++)); do
	    if [ $Rm == 0 ];then RotMir=1; fi
	    if [ $Rm == 1 ];then RotMir=-10; fi
	    
	    for ((Th=0; Th<=0; Th++)); do
		if [ $Th == 0 ];then Theta=25; fi
		if [ $Th == 1 ];then Theta=35; fi
		
#	for ((RX=0; RX<=$UpperTransY; RX++)); do
	 #   for ((RY=0; RY<=$UpperTransZ; RY++)); do
	#for ((RX=0; RX<=3; RX++)); do
	  #  for ((RY=0; RY<=3; RY++)); do
	#	echo $RX $RY

		for ((trY=0; trY<=9; trY++)); do
		    TrY=$(( 20 * ${trY}))
		    for ((trZ=4; trZ<=4; trZ++)); do
			TrZ=$(( 20 * ${trZ}))
			echo $TrY   $TrZ
			
			
			root -b -q "CreateGeo/CreateGDMLfile_April2015_ModAug2015.C($TrY, $TrZ, $RotMir, $RX, $RY, $DefaultDims, $DefaultDimsLargePMT)"
 			qsub -l mem=5G,ct=5:00:00 ./RunSimOnBatch.sh $RX $RY $RotMir $nEvs $TrY $TrZ $Theta $DefaultDims $DefaultDimsLargePMT
			
		    done
		done
	    done
	done
    done
done

