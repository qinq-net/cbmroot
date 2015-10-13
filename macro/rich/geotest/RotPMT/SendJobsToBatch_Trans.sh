#!/bin/sh

cbmroot_config_path=/hera/cbm/users/tariq/cbmroot/buildcbm/config.sh
source ${cbmroot_config_path}

nEvs=20000
RX=20; RY=10
TrY=0; TrZ=0
MinTheta=25; MaxTheta=250 
MinPhi=90; MaxPhi=180
GeoCase=2; DimCase=2
#DimCase --> 0: enlarged rich (in x,y and z) and PMT, 1: Default RICH dim, 2: enlarged PMT and RICH in x and y, 3: optimised PMT dims (_PMTW1002_H604.)
EnlargedPMTWidth=2; EnlargedPMTHight=4

PtNotP=1
MomMin=0; MomMax=40

rotmir=-10; extendedmir=1

OldCode=1
DefaultDims=0;DefaultDimsLargePMT=0 #for old code
if [ $OldCode == 1 -a $DimCase=3 ];then 
    echo " Want to work with old code but setting DimCase to 3 !!"
    echo " DimCase to 3 goes only with new code!!"
    echo " Setting DimCase to 2!!"
    DimCase=2
    extendedmir=0
fi
if [ $DimCase -lt 3 -a $OldCode == 0 ];then 
    echo " Want to work with older geometries (DimCase=$DimCase) but with new code !!"
    echo " Setting DimCase to 3 --> prepare geo for new code!!"
    DimCase=3
fi
#########################
UpperTransY=0; UpperTransZ=0;
if [ $DimCase == 0 ];then UpperTransY=50; UpperTransZ=50; DefaultDims=0; DefaultDimsLargePMT=0; fi
if [ $DimCase == 1 ];then UpperTransY=31; UpperTransZ=23; DefaultDims=1; DefaultDimsLargePMT=0; fi
if [ $DimCase == 2 ];then UpperTransY=23; UpperTransZ=13; DefaultDims=1; DefaultDimsLargePMT=1; fi
if [ $DimCase == 3 ];then UpperTransY=23; UpperTransZ=13; DefaultDims=1; DefaultDimsLargePMT=1; fi 
	
	
for ((Rm=1; Rm<=1; Rm++)); do
    if [ $Rm == 0 ];then rotmir=1; fi
    if [ $Rm == 1 ];then rotmir=-10; fi
    
    for ((Th=0; Th<=0; Th++)); do
	if [ $Th == 0 ];then MaxTheta=250; fi
	if [ $Th == 1 ];then MaxTheta=350; fi
	
	for ((trY=0; trY<=0; trY++)); do
	    TrY=$(( 20 * ${trY}))
	    for ((trZ=4; trZ<=4; trZ++)); do
		TrZ=$(( 20 * ${trZ}))
		echo $TrY   $TrZ
		
		if [ $OldCode == 1 ];then 
		    echo " working with old code CreateGeo/CreateGDMLfile_April2015_ModAug2015.C()"
		    root -b -q "CreateGeo/CreateGDMLfile_April2015_ModAug2015.C($TrY, $TrZ, $rotmir, $RX, $RY, $DefaultDims, $DefaultDimsLargePMT)"
		else
		    echo " working with new code CreateGeo/CreateGDMLfile_ExtMirrOct2015.C()"
		    root -b -q "CreateGeo/CreateGDMLfile_ExtMirrOct2015.C($TrY, $TrZ, $rotmir, $RX, $RY, $EnlargedPMTWidth, $EnlargedPMTHight)"
		fi
		
 		
#		qsub -l mem=5G,ct=5:00:00 ./RunSimOnBatch.sh -nev $nEvs -rx $RX -ry $RY -dy $TrY -dz $TrZ -mint $Mintheta -maxt $MaxTheta -minp $MinPhi -maxp $MaxPhi -geo $GeoCase -dim $DimCase -pmtw $EnlargedPMTWidth -pmth $EnlargedPMTHight -ptp $PtNotP -minm $MomMin -maxm $MomMax -mir $rotmir -extm $extendedmir -oldc $OldCode -ddim $DefaultDims -pmtd $DefaultDimsLargePMT

		qsub -l mem=5G,ct=5:00:00 ./RunSimOnBatch.sh -nev $nEvs -rx $RX -ry $RY -dy $TrY -dz $TrZ -mint $MinTheta -maxt $MaxTheta -minp $MinPhi -maxp $MaxPhi -geo $GeoCase -dim $DimCase -pmtw $EnlargedPMTWidth -pmth $EnlargedPMTHight -ptp $PtNotP -minm $MomMin -maxm $MomMax -mir $rotmir -extm $extendedmir -oldc $OldCode -ddim $DefaultDims -pmtd $DefaultDimsLargePMT
		
	    done
	done
    done
done


