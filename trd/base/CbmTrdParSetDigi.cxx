#include "CbmTrdParSetDigi.h"
#include "CbmTrdParModDigi.h"

#include <FairParamList.h>
#include <FairLogger.h>

#include <TArrayI.h>
#include <TArrayD.h>

CbmTrdParSetDigi::CbmTrdParSetDigi(const char* name, 
           const char* title,
           const char* context)
  : CbmTrdParSet(name, title, context)

{

}

//_______________________________________________________________________________
Bool_t CbmTrdParSetDigi::getParams(FairParamList* l) 
{
  if (!l) return kFALSE;
  if ( ! l->fill("NrOfModules", &fNrOfModules) ){ 
    LOG(ERROR)<<GetName()<<"::getParams : Couldn't find \"NrOfModules\""<<FairLogger::endl;
    return kFALSE;
  }
  Int_t maxSectors(0);
  if ( ! l->fill("MaxSectors", &maxSectors) ){ 
    LOG(ERROR)<<GetName()<<"::getParams : Couldn't find \"MaxSectors\""<<FairLogger::endl;
    return kFALSE;
  }
  TArrayI  moduleId(fNrOfModules);
  if ( ! l->fill("ModuleIdArray", &moduleId) ){ 
    LOG(ERROR)<<GetName()<<"::getParams : Couldn't find \"MaxSectors\""<<FairLogger::endl;
    return kFALSE;
  }
  // Instead of a fixed number of values the number of values to
  // store now depends on the maximum number of sectors per module
  Int_t nrValues = 11 + ( maxSectors * 4 );
  TArrayD values(nrValues);
  TArrayD sectorSizeX(maxSectors);
  TArrayD sectorSizeY(maxSectors);
  TArrayD padSizeX(maxSectors);
  TArrayD padSizeY(maxSectors);
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
    if ( ! l->fill(Form("%d", moduleId[i]), &values) ) {
      LOG(ERROR)<<GetName()<<"::getParams : Missing parameter definiton for module "<<moduleId[i]<<FairLogger::endl;
      continue;
    }
    Int_t k(0);
    orientation=values[k++];
    roType=values[k++];
    awPitch=values[k++];
    awPP=values[k++];
    awOff=values[k++];
    x=values[k++];
    y=values[k++];
    z=values[k++];
    sizex= values[k++];
    sizey= values[k++];
    sizez= values[k++];
    for (Int_t j=0; j < maxSectors; j++){       
      sectorSizeX.AddAt(values[k++],j);
      sectorSizeY.AddAt(values[k++],j);
      padSizeX.AddAt(values[k++],j);
      padSizeY.AddAt(values[k++],j);
    }
    fModuleMap[moduleId[i]] = new CbmTrdParModDigi(x, y, z,
                                            sizex, sizey, sizez, maxSectors, orientation, 
                                            sectorSizeX, sectorSizeY,
                                            padSizeX, padSizeY);
    fModuleMap[moduleId[i]]->SetModuleId(moduleId[i]);
    ((CbmTrdParModDigi*)fModuleMap[moduleId[i]])->SetAnodeWireToPadPlaneDistance(awPP>0?awPP:0.35);
    ((CbmTrdParModDigi*)fModuleMap[moduleId[i]])->SetAnodeWireOffset(awOff>0?awOff:0.375);
    ((CbmTrdParModDigi*)fModuleMap[moduleId[i]])->SetAnodeWireSpacing(awPitch>0?awPitch:0.25);
    //fModuleMap[moduleId[i]]->Print();
  }
  return kTRUE;
}

ClassImp(CbmTrdParSetDigi)
