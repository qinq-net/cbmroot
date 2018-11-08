#include "CbmTrdParModGeo.h"

#include <TGeoPhysicalNode.h>
#include <TGeoMatrix.h>
#include <TGeoBBox.h>


//___________________________________________________________________
CbmTrdParModGeo::CbmTrdParModGeo(const char* name, const char* title)
  : CbmTrdParMod(name, title)
  ,fNode(NULL)
{
  fNode = new TGeoPhysicalNode(title);
}

//___________________________________________________________________
CbmTrdParModGeo::~CbmTrdParModGeo()
{
  delete fNode;
}

//___________________________________________________________________
Double_t CbmTrdParModGeo::GetDX() const
{
  return ((TGeoBBox*)fNode->GetShape())->GetDX();
}

//___________________________________________________________________
Double_t CbmTrdParModGeo::GetDY() const
{
  return ((TGeoBBox*)fNode->GetShape())->GetDY();
}

//___________________________________________________________________
Double_t CbmTrdParModGeo::GetDZ() const
{
  return ((TGeoBBox*)fNode->GetShape())->GetDZ();
}

//_______________________________________________________________________________
void CbmTrdParModGeo::LocalToMaster(Double_t in[3], Double_t out[3]) const
{
  if(!fNode) return;
  fNode->GetMatrix()->LocalToMaster(in, out);
}

//___________________________________________________________________
void CbmTrdParModGeo::GetXYZ(Double_t xyz[3]) const
{
  memset(xyz, 0, 3*sizeof(Double_t));
  Double_t gxyz[3];
  fNode->GetMatrix()->LocalToMaster(xyz, gxyz);
  memcpy(xyz, gxyz, 3*sizeof(Double_t));
}

ClassImp(CbmTrdParModGeo)
