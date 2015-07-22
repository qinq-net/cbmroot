#include "CbmTutorial2DetGeo.h"
#include "FairGeoNode.h"

ClassImp(CbmTutorial2DetGeo)

// -----   Default constructor   -------------------------------------------
CbmTutorial2DetGeo::CbmTutorial2DetGeo() {
  // Constructor
  fName="tutdet";
  maxSectors=0;
  maxModules=10;
}

// -------------------------------------------------------------------------

const char* CbmTutorial2DetGeo::getModuleName(Int_t m) {
  /** Returns the module name of Tutorial2Det number m
      Setting MyDet here means that all modules names in the 
      ASCII file should start with Tutorial2Det otherwise they will 
      not be constructed
  */
  sprintf(modName,"tutdet%i",m+1); 
  return modName;
}

const char* CbmTutorial2DetGeo::getEleName(Int_t m) {
  /** Returns the element name of Det number m */
  sprintf(eleName,"tutdet%i",m+1);
  return eleName;
}
