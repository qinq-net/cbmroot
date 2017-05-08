#ifndef PLATFORM_H
#define PLATFORM_H

#include "FairModule.h"
#include <string>

class TGeoCombiTrans;

class CbmPlatform : public FairModule {
  public:
    CbmPlatform();
    CbmPlatform(const char * name, const char *Title="CBM Platform");
    virtual ~CbmPlatform();
    virtual void ConstructGeometry();

    void ConstructRootGeometry();
    Bool_t IsNewGeometryFile(TString /*filename*/);

    virtual void ConstructAsciiGeometry();

  private:
    TGeoCombiTrans*   fCombiTrans;  //! Transformation matrix for geometry positioning
    std::string fVolumeName;

    ClassDef(CbmPlatform,1) //CBMPLATFORM
};

#endif //PLATFORM_H

