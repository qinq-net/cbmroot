#!/bin/bash

#cRun=CbmTofPiHd_01Mar1558
#cRun=CbmTofPiHd_21Mar1734
#cRun=CbmTofPiHd_30Mar1435
#cRun=CbmTofPiHd_01Apr1647
#cRun=CbmTofPiHd_08Apr1811
#cRun=CbmTofPiHd_14Apr1255
#cRun=CbmTofPiHd_Plastic_26Jul1744
#cRun=CbmTofPiHd_Plastic_27Jul1725
#cRun=CbmTofPiHd_Plastic_01Aug1700
#cRun=CbmTofPiHd_Plastic_05Aug1913
#cRun=CbmTofPiHd_09Aug1533
#cRun=CbmTofPiHd_10Aug1728
cRun=CbmTofPiHd_11Aug1224

# Removing files: Fresh unpacking
if((0)); then
rm all_*.par core_dump_* *.pdf
rm tofMbsUnp.hst.root
rm tofMbsCal.hst.root
rm tofMbsMap.hst.root
rm tofTrbUnp.hst.root
rm TofTdcCalibHistos_$cRun.root
rm tofFindTracks.hst.root
rm unpack_$cRun.buffer.root
rm unpack_$cRun.calib.root
rm unpack_$cRun.out.root 
rm unpack_$cRun.params.root
fi

# Removing files: Without unpacking
if((0)); then
rm -r $cRun
rm *.pdf
rm $cRun_*tofTestBeamClust.hst.root
rm $cRun_*tofAnaTestBeam.hst.root
rm tofTestBeamClust_$cRun_*.hst.root
rm digi_$cRun_*.out.root
fi

# ------------------------ New Calibration Class ------------------------------

# Step 1: Create Calib
if((1)); then
echo "create_calib.C is called..."
root -l<<EOF
Int_t nEvents  =10000000
char* cFileId  ="$cRun"
FairRunOnline* run = new FairRunOnline();
.x create_calib.C(nEvents, cFileId);
EOF
fi


# Step 2: Apply Calib
if((1)); then
echo "apply_calib.C is called..."
root -l<<EOF
Int_t nEvents  =10000000
char* cFileId  ="$cRun"
FairRunOnline* run = new FairRunOnline();
.x apply_calib.C(nEvents, cFileId);
EOF
fi

