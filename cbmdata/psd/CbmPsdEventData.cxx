
/** CbmPsdEventData.cxx
 **@author Alla Maevskaya <alla@inr.ru> & SELIM
 **@since 3.08.20212
 **
 ** Modified to implement event plane method (S. Seddiki)
  **/


#include "CbmPsdEventData.h"
#include <iostream>
using std::cout;
using std::endl;


// -----   Default constructor   -------------------------------------------
CbmPsdEventData::CbmPsdEventData() 
  : TNamed()
    //fRP_MC(-9999),
    //fB_MC(-1)
    //fEP_RECO(-9999)
{
}

// -----   constructor   -------------------------------------------
CbmPsdEventData::CbmPsdEventData(const char *name) 
  : TNamed()
    //fRP_MC(-9999),
    //fB_MC(-1)
    //fEP_RECO(-9999)
{
  TString namst = "RP_";
  namst += name;
  SetName(namst.Data());
  SetTitle(namst.Data());
}

/*
CbmPsdEventData::CbmPsdEventData(Float_t rp, Float_t b) 
  : TNamed(),
    fRP_MC(rp),
    fB_MC(b) 
{ 
 
}
*/

// -----   Destructor   ----------------------------------------------------
CbmPsdEventData::~CbmPsdEventData(){}


void CbmPsdEventData::Print() {  
  cout << "(RECO) EVENT PLANE : " << fEP_RECO << endl;
}

ClassImp(CbmPsdEventData)
