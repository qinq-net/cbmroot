
/** CbmMCEventData.cxx
 **@author SELIM <S.Seddiki@gsi.de>
 **@since 06.2013
 ** Data container class to store MC info (b, phi_RP) in output of event plane reconstruction (seperately from reconstructed data)
  **/


#include "CbmMCEventData.h"

#include "FairLogger.h"

// -----   Default constructor   -------------------------------------------
CbmMCEventData::CbmMCEventData() 
  : TNamed(),
    fRP(-9999),
    fB(-1)    
{
}

// -----   constructor   -------------------------------------------
CbmMCEventData::CbmMCEventData(const char *name) 
  : TNamed(),
    fRP(-9999),
    fB(-1)    
{
  TString namst = "RP_";
  namst += name;
  SetName(namst.Data());
  SetTitle(namst.Data());
}

/*
CbmMCEventData::CbmMCEventData(Float_t rp, Float_t b) 
  : TNamed(),
    fRP(rp),
    fB(b) 
{ 
 
}
*/

// -----   Destructor   ----------------------------------------------------
CbmMCEventData::~CbmMCEventData(){}


void CbmMCEventData::Print(Option_t*) const {  
  LOG(INFO) << "(MC) impact parameter: " << fB << ", (MC) REACTION PLANE: " << fRP 
            << FairLogger::endl;
}

ClassImp(CbmMCEventData)
