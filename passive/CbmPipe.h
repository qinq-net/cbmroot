#ifndef CBMPIPE_H
#define CBMPIPE_H 1

#include "FairModule.h"

class CbmPipe : public FairModule {

  public:

    CbmPipe();

    CbmPipe(const char * name, const char *Title="CBM Pipe");

    virtual ~CbmPipe();

    virtual void ConstructGeometry();

    virtual void ConstructAsciiGeometry();

    virtual Bool_t CheckIfSensitive(std::string name) { return kFALSE; }
   
  ClassDef(CbmPipe,1) //CBMPIPE

};

#endif //CBMPIPE_H

