// -------------------------------------------------------------------------
// -----                    CbmMvdHitMatch source file                 -----
// -----                  Created 07/11/06  by V. Friese               -----
// -----            Based on CbmStsMapsHitInfo by M. Deveaux           -----
// -----           Update to new CbmMatch Class by P. Sitzmann         -----
// -------------------------------------------------------------------------



#include "CbmMvdHitMatch.h"



// -----   Default constructor   -------------------------------------------
CbmMvdHitMatch::CbmMvdHitMatch() {
  AddLink(0. , 0, -1 ,-1);
  
  fFileNumber = -1;
  fIndex = 0; 
  fWeight = 0.;
  fEntry = -1;
}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
CbmMvdHitMatch::CbmMvdHitMatch(Double_t weight, Int_t index, Int_t entry, Int_t file) {
  AddLink(weight, index, entry, file);
 
  fFileNumber = file;
  fIndex = index; 
  fWeight = weight;
  fEntry = entry;
}
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
CbmMvdHitMatch::~CbmMvdHitMatch() {}
// -------------------------------------------------------------------------

ClassImp(CbmMvdHitMatch);
 
