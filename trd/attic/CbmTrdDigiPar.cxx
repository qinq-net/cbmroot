// -------------------------------------------------------------------------
// -----                CbmTrdDigiPar source file                      -----
// -----                Created 05/06/08  by F. Uhlig                  -----
// -------------------------------------------------------------------------

#include "CbmTrdDigiPar.h"
#include "CbmTrdModule.h"

#include "FairParamList.h"
#include "FairDetParIo.h"
#include "FairParIo.h"
#include "FairLogger.h"

#include "TString.h"

#include <iomanip>

// -----   Standard constructor   ------------------------------------------
CbmTrdDigiPar::CbmTrdDigiPar(const char* name, 
			     const char* title,
			     const char* context)
  : FairParGenericSet(name, title, context), 
    fModuleMap(),
    fModuleIdArray(),
    fNrOfModules(-1),
    fMaxSectors(-1)
{
  detName="Trd";
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmTrdDigiPar::~CbmTrdDigiPar() 
{
  LOG(DEBUG4) << "Enter CbmTrdDigiPar::~CbmTrdDigiPar"  << FairLogger::endl;
  std::map<Int_t, CbmTrdModule*>::iterator fModuleMapIt;
  for (fModuleMapIt = fModuleMap.begin(); fModuleMapIt != fModuleMap.end(); 
       ++fModuleMapIt) {
    delete fModuleMapIt->second;
  }      
  fModuleMap.clear();
  clear();
  LOG(DEBUG4) << "Leave CbmTrdDigiPar::~CbmTrdDigiPar"  << FairLogger::endl;
}
// -------------------------------------------------------------------------


// -----   Public method clear   -------------------------------------------
void CbmTrdDigiPar::clear() 
{
  status = kFALSE;
  resetInputVersions();
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------

void CbmTrdDigiPar::putParams(FairParamList* l) 
{
/**  
   Instead of a fixed number of values the number of values to
   store now depends on the maximum number of sectors per module
   The first eleven parameters are for the complete module.  
   The parametrs are:
   orientation         : module rotation in 90deg
   RO type             : ASIC/PAD type. The read-out type is ancrypted as a 2 bit map : 
                            bit0=1 triangular pads [default rectangular]
                            bit1=1 FASP asic       [default SPADIC] 
   fAnodeWireSpacing   : width of amplification cell in cm
   fAnodeWireToPadPlaneDistance : anode 2 pad plane distance in cm   
   fAnodeWireOffset    : offset of first anode wire wrt pad plane   
   X, Y, Z             : position of the middle of the gaslayer.
   SizeX, SizeY, SizeZ : size of the gaslayer. The values are only
                         the half size which are the values returned
                         by geant.
   
   The rest of the parameters depend on the number of sectors.
   SectorSizeX(Y)      : size of a sector
   PadSizeX(Y)         : size of the pads in this sector
*/

  if (!l) return;

   l->add("NrOfModules",   fNrOfModules);
   l->add("MaxSectors",    fMaxSectors);
   l->add("ModuleIdArray", fModuleIdArray);

   Int_t nrValues = 11 + ( fMaxSectors * 4 );
   TArrayD values(nrValues);
 
   for (Int_t i=0; i < fNrOfModules; i++){
     Int_t k(0);
     values.AddAt(fModuleMap[fModuleIdArray[i]]->GetOrientation(), k++);         
     Int_t roType(0);
     if(fModuleMap[fModuleIdArray[i]]->GetPadGeoTriangular()) roType |= 1;
     if(fModuleMap[fModuleIdArray[i]]->GetAsicFASP()) roType |= 2;
     values.AddAt(roType, k++);         
     values.AddAt(fModuleMap[fModuleIdArray[i]]->GetAnodeWireSpacing(),k++); 
     values.AddAt(fModuleMap[fModuleIdArray[i]]->GetAnodeWireToPadPlaneDistance(),k++); 
     values.AddAt(fModuleMap[fModuleIdArray[i]]->GetAnodeWireOffset(),k++); 
     values.AddAt(fModuleMap[fModuleIdArray[i]]->GetX(),k++);         
     values.AddAt(fModuleMap[fModuleIdArray[i]]->GetY(),k++);          
     values.AddAt(fModuleMap[fModuleIdArray[i]]->GetZ(),k++);          
     values.AddAt(fModuleMap[fModuleIdArray[i]]->GetSizeX(),k++);      
     values.AddAt(fModuleMap[fModuleIdArray[i]]->GetSizeY(),k++); 
     values.AddAt(fModuleMap[fModuleIdArray[i]]->GetSizeZ(),k++); 
     for (Int_t j=0; j < fMaxSectors; j++){       
       values.AddAt(fModuleMap[fModuleIdArray[i]]->GetSectorSizeX(j),k++);   
       values.AddAt(fModuleMap[fModuleIdArray[i]]->GetSectorSizeY(j),k++);   
       values.AddAt(fModuleMap[fModuleIdArray[i]]->GetPadSizeX(j),k++);   
       values.AddAt(fModuleMap[fModuleIdArray[i]]->GetPadSizeY(j),k++);   
     }

     TString text;
     text += fModuleIdArray[i];
     l->add(text.Data(), values);
   }
}

//------------------------------------------------------

Bool_t CbmTrdDigiPar::getParams(FairParamList* l) {

  if (!l) return kFALSE;
  
  if ( ! l->fill("NrOfModules", &fNrOfModules) ) return kFALSE;
  if ( ! l->fill("MaxSectors", &fMaxSectors) ) return kFALSE;
  
  fModuleIdArray.Set(fNrOfModules);
  if ( ! l->fill("ModuleIdArray", &fModuleIdArray) ) return kFALSE;
  
  // Instead of a fixed number of values the number of values to
  // store now depends on the maximum number of sectors per module
  Int_t nrValues = 11 + ( fMaxSectors * 4 );
  TArrayD *values = new TArrayD(nrValues);
  TArrayD sectorSizeX(fMaxSectors);
  TArrayD sectorSizeY(fMaxSectors);
  TArrayD padSizeX(fMaxSectors);
  TArrayD padSizeY(fMaxSectors);
  Int_t orientation;
  Double_t x;
  Double_t y;
  Double_t z;
  Double_t sizex;
  Double_t sizey;
  Double_t sizez;
  Double_t awPitch(-1.);
  Double_t awPP(-1.);
  Double_t awOff(-1.);
  Int_t roType(0);

  TString text;
  for (Int_t i=0; i < fNrOfModules; i++){
    text="";
    text += fModuleIdArray[i];
    if ( ! l->fill(text.Data(), values) ) return kFALSE;
    Int_t VolumeID = text.Atoi();
    Int_t k(0);
    orientation=values->At(k++);
    roType=values->At(k++);
    awPitch=values->At(k++);
    awPP=values->At(k++);
    awOff=values->At(k++);
    x=values->At(k++);
    y=values->At(k++);
    z=values->At(k++);
    sizex= values->At(k++);
    sizey= values->At(k++);
    sizez= values->At(k++);
    for (Int_t j=0; j < fMaxSectors; j++){       
      sectorSizeX.AddAt(values->At(k++),j);
      sectorSizeY.AddAt(values->At(k++),j);
      padSizeX.AddAt(values->At(k++),j);
      padSizeY.AddAt(values->At(k++),j);
    }

    fModuleMap[VolumeID] = new CbmTrdModule(VolumeID, orientation, x, y, z,
                                            sizex, sizey, sizez, fMaxSectors,
                                            sectorSizeX, sectorSizeY,
                                            padSizeX, padSizeY, roType&1, roType&2);
    fModuleMap[VolumeID]->SetAnodeWireToPadPlaneDistance(awPP>0?awPP:0.35);
    fModuleMap[VolumeID]->SetAnodeWireOffset(awOff>0?awOff:0.375);
    fModuleMap[VolumeID]->SetAnodeWireSpacing(awPitch>0?awPitch:0.25);
    //fModuleMap[VolumeID]->Print();
  }

  delete values;
  return kTRUE;
}

// -----   Public method printParams ---------------------------------------
void CbmTrdDigiPar::printparams()
{

  LOG(INFO) <<"FairTutorialDet2DigiPar::printparams()"<<FairLogger::endl;
  LOG(INFO) <<"fMaxSectors: " << fMaxSectors << FairLogger::endl;
  LOG(INFO) <<"fNrOfModules: " << fNrOfModules << FairLogger::endl;

  std::map<Int_t, CbmTrdModule*>::iterator fModuleMapIt;
  for (fModuleMapIt = fModuleMap.begin(); fModuleMapIt != fModuleMap.end(); 
       ++fModuleMapIt) {
    LOG(INFO) << "VolumeID: " << fModuleMapIt->first << FairLogger::endl;
    CbmTrdModule* mod = fModuleMapIt->second;
    LOG(INFO) << "X: " << std::setprecision(5) << mod->GetX() << FairLogger::endl;
    LOG(INFO) << "Y: " << std::setprecision(5) << mod->GetY() << FairLogger::endl;
    LOG(INFO) << "Z: " << std::setprecision(5) << mod->GetZ() << FairLogger::endl;
    LOG(INFO) << "SizeX: " << std::setprecision(5) << mod->GetSizeX() << FairLogger::endl;
    LOG(INFO) << "SizeY: " << std::setprecision(5) << mod->GetSizeY() << FairLogger::endl;
    LOG(INFO) << "SizeZ: " << std::setprecision(5) << mod->GetSizeZ() << FairLogger::endl;

  }      

}
// -------------------------------------------------------------------------


ClassImp(CbmTrdDigiPar)
