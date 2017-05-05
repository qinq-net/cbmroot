#ifndef CBMPIPE_H
#define CBMPIPE_H 1

#include "FairModule.h"
#include <string>

class TGeoCombiTrans;

class CbmPipe : public FairModule {

  public:

    CbmPipe();

    CbmPipe(const char * name, const char *Title="CBM Pipe");

    virtual ~CbmPipe();

    virtual void ConstructGeometry();

    void ConstructRootGeometry();

    Bool_t IsNewGeometryFile(TString /*filename*/);

    virtual void ConstructAsciiGeometry();

    virtual Bool_t CheckIfSensitive(std::string) { return kFALSE; }
   
  ClassDef(CbmPipe,1) //CBMPIPE

  private:

    TGeoCombiTrans*   fCombiTrans;  //! Transformation matrix for geometry positioning

    std::string fVolumeName;

};

#endif //CBMPIPE_H

