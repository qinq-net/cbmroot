#ifndef CBMTRDPARSETGEO_H
#define CBMTRDPARSETGEO_H


#include "CbmTrdParSet.h"

class FairParamList;
class CbmTrdParSetGeo : public CbmTrdParSet 
{
public:
  CbmTrdParSetGeo(const char* name="CbmTrdParSetGeo",
             const char* title="Trd Geometry Parameters",
             const char* context="TestDefaultContext");
  virtual ~CbmTrdParSetGeo(void);

  void    putParams(FairParamList*);
  Bool_t  getParams(FairParamList*);
private:

  ClassDef(CbmTrdParSetGeo,1)
};

#endif /* !CBMTRDPARSETGEO_H */
