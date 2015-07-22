#ifndef CBMTUTORIAL2DETGEO_H
#define CBMTUTORIAL2DETGEO_H

#include "FairGeoSet.h"

class  CbmTutorial2DetGeo : public FairGeoSet {

 protected:
  char modName[20];  // name of module
  char eleName[20];  // substring for elements in module
 public:
  CbmTutorial2DetGeo();
  ~CbmTutorial2DetGeo() {}
  const char* getModuleName(Int_t);
  const char* getEleName(Int_t);
  inline Int_t getModNumInMod(const TString&);
  ClassDef(CbmTutorial2DetGeo,1)
};

inline Int_t CbmTutorial2DetGeo::getModNumInMod(const TString& name) {
  /** returns the module index from module name 
   ?? in name[??] has to be the length of the detector name in the
   .geo file. For example if all nodes in this file starts with
   tutdet ?? has to be 6.
  */
  return (Int_t)(name[6]-'0')-1; // 
}

#endif  /*  CBMTUTORIALDETGEO_H */
