#ifndef CBMTRDQAHIT_H
#define CBMTRDQAHIT_H

#include "CbmTrdQABase.h"

class CbmTrdQAHit : public CbmTrdQABase
{
      protected:
    /** Constructor with parameters (Optional) **/
    CbmTrdQAHit(TString ClassName);
    public:

    virtual void CreateHistograms();
    virtual void Exec(Option_t*);
    /** Default constructor **/
    CbmTrdQAHit():CbmTrdQAHit("CbmTrdQAHit"){};

    ClassDef(CbmTrdQAHit,1);
};

#endif
