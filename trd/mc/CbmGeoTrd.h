#ifndef CBMGEOTRD_H
#define CBMGEOTRD_H

#include "FairGeoSet.h"

class  CbmGeoTrd : public FairGeoSet {
protected:
  char modName[20];  // name of module
  char eleName[20];  // substring for elements in module
public:
  CbmGeoTrd();
  ~CbmGeoTrd() {}
  const char* getModuleName(Int_t);
  const char* getEleName(Int_t);
  inline Int_t getModNumInMod(const TString&);
  ClassDef(CbmGeoTrd,0) // Class for Trd
};

inline Int_t CbmGeoTrd::getModNumInMod(const TString& name) {
  // returns the module index from module name
  return (Int_t)(name[3]-'0')-1;
}

#endif  /* !CBMGEOTRD_H */
