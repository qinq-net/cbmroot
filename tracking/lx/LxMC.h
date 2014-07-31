#ifndef LXMC_INCLUDED
#define LXMC_INCLUDED

#include <vector>
#include <list>
#include "Rtypes.h"
#include "LxSettings.h"
#include "CbmStsTrack.h"

struct LxPoint;
struct LxStsMCPoint;

struct LxMCPoint
{
  Double_t p, q, x, y, z, px, py, pz;
  Int_t stationNumber, layerNumber;
  std::list<LxPoint*> lxPoints;

  LxMCPoint() 
  : p(0.), q(0.), x(0.), y(0.), z(0.), px(0.), py(0.), pz(0.),
    stationNumber(0), layerNumber(0), lxPoints() {}
};

struct LxMCTrack
{
  Double_t p, q, x, y, z, px, py, pz;
  Int_t mother_ID;
  Int_t pdg;
  std::vector<LxMCPoint*> Points;
#ifdef MAKE_STS_MC_RESEARCHES
  std::list<LxStsMCPoint*> stsPoints[8];
#endif//MAKE_STS_MC_RESEARCHES
  UInt_t fUniqueID;
  CbmStsTrack* externalTrack;

  LxMCTrack()
  : p(0.), q(0.), x(0.), y(0.), z(0.), px(0.), py(0.), pz(0.),
    mother_ID(0), pdg(0), Points(), fUniqueID(0), externalTrack(NULL) {}

};

#endif//LXMC_INCLUDED
