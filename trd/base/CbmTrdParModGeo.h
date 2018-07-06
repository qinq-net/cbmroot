#ifndef CBMTRDPARMODGEO_H
#define CBMTRDPARMODGEO_H

#include "CbmTrdParMod.h"
class FairGeoNode;
/** \brief Definition of geometry for one TRD module **/
class CbmTrdParModGeo : public CbmTrdParMod
{
public:
  CbmTrdParModGeo(const char* name    = "CbmTrdParModGeo",
    const char* title   = "TRD module geometry");
  virtual ~CbmTrdParModGeo() {;}
  const Char_t*       GetPath() const { return GetTitle();}
  virtual Double_t    GetX() const { return 0.; }
  virtual Double_t    GetY() const { return 0.; }
  virtual Double_t    GetZ() const { return 0.; }
private:
  CbmTrdParModGeo(const CbmTrdParModGeo &);
  const CbmTrdParModGeo& operator=(const CbmTrdParModGeo &);
  
  FairGeoNode   *fSense;
  FairGeoNode   *fPassive;
  
  ClassDef(CbmTrdParModGeo, 1)  // Definition of geometry parameters for one TRD module
};

#endif
