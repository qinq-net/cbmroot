
/** CbmStsEventData.cxx
 **@author Selim SEDDIKI <s.seddiki at gsi.de>
 **@since 23.10.2012
 **@version 1.0
 **
 **/


#include "CbmStsEventData.h"
#include <iostream>
using std::cout;
using std::endl;


// -----   Default constructor   -------------------------------------------
CbmStsEventData::CbmStsEventData() 
  : TNamed()    
    //fEP_RECO(-9999)
{
}

// -----   constructor   -------------------------------------------
CbmStsEventData::CbmStsEventData(const char *name) 
  : TNamed()    
    //fEP_RECO(-9999)
{
  TString namst = "RP_";
  namst += name;
  SetName(namst.Data());
  SetTitle(namst.Data());
}

/*
CbmStsEventData::CbmStsEventData(Float_t rp, Float_t b) 
  : TNamed(),
    fRP_MC(rp),
    fB_MC(b) 
{ 
 
}
*/

// -----   Destructor   ----------------------------------------------------
CbmStsEventData::~CbmStsEventData(){}


void CbmStsEventData::Print() {  
  cout << "(RECO) EVENT PLANE : using first harmonic =  "<< fEP_RECO_harmo1_full << ", using second harmonic = " << fEP_RECO_harmo2_full << endl;

}

ClassImp(CbmStsEventData)
