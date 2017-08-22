/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "HitReader.h"
#include "Station.h"
#include "TClonesArray.h"
#include "setup/CbmStsSetup.h"
#include "geo/CbmMuchGeoScheme.h"
#include "CbmTrdHit.h"

using namespace std;

CbmBinnedHitReader::CbmBinnedHitReader(TClonesArray* hitArray) : fStations(), fHitArray(hitArray)
{
}

CbmBinnedHitReader::~CbmBinnedHitReader()
{
}

class CbmBinnedStsHitReader : public CbmBinnedHitReader
{
public:
   explicit CbmBinnedStsHitReader(TClonesArray* hitArray) : CbmBinnedHitReader(hitArray) {}
   
   void Read()
   {
      Int_t nofHits = fHitArray->GetEntriesFast();
   
      for (Int_t i = 0; i < nofHits; ++i)
      {
         const CbmPixelHit* hit = static_cast<const CbmPixelHit*> (fHitArray->At(i));
         fStations[CbmStsSetup::Instance()->GetStationNumber(hit->GetAddress()) - 1]->AddHit(hit, i);
      }
   }
};

class CbmBinnedRichHitReader : public CbmBinnedHitReader
{
public:
   explicit CbmBinnedRichHitReader(TClonesArray* hitArray) : CbmBinnedHitReader(hitArray) {}
   
   void Read()
   {
      Int_t nofHits = fHitArray->GetEntriesFast();
   
      for (Int_t i = 0; i < nofHits; ++i)
      {
         const CbmPixelHit* hit = static_cast<const CbmPixelHit*> (fHitArray->At(i));
         fStations[0]->AddHit(hit, i);
      }
   }
};

class CbmBinnedMuchHitReader : public CbmBinnedHitReader
{
public:
   explicit CbmBinnedMuchHitReader(TClonesArray* hitArray) : CbmBinnedHitReader(hitArray) {}
   
   void Read()
   {
      Int_t nofHits = fHitArray->GetEntriesFast();
   
      for (Int_t i = 0; i < nofHits; ++i)
      {
         const CbmPixelHit* hit = static_cast<const CbmPixelHit*> (fHitArray->At(i));
         fStations[CbmMuchGeoScheme::GetStationIndex(hit->GetAddress()) - 1]->AddHit(hit, i);
      }
   }
};

class CbmBinnedTrdHitReader : public CbmBinnedHitReader
{
public:
   explicit CbmBinnedTrdHitReader(TClonesArray* hitArray) : CbmBinnedHitReader(hitArray) {}
   
   void Read()
   {
      Int_t nofHits = fHitArray->GetEntriesFast();
   
      for (Int_t i = 0; i < nofHits; ++i)
      {
         const CbmTrdHit* hit = static_cast<const CbmTrdHit*> (fHitArray->At(i));
         fStations[hit->GetPlaneId() - 1]->AddHit(hit, i);
      }
   }
};

class CbmBinnedTofHitReader : public CbmBinnedHitReader
{
public:
   explicit CbmBinnedTofHitReader(TClonesArray* hitArray) : CbmBinnedHitReader(hitArray) {}
   
   void Read()
   {
      Int_t nofHits = fHitArray->GetEntriesFast();
   
      for (Int_t i = 0; i < nofHits; ++i)
      {
         const CbmPixelHit* hit = static_cast<const CbmPixelHit*> (fHitArray->At(i));
         fStations[0]->AddHit(hit, i);
      }
   }
};

map<string, CbmBinnedHitReader*> CbmBinnedHitReader::fReaders;

CbmBinnedHitReader* CbmBinnedHitReader::Instance(const char* name)
{
   map<string, CbmBinnedHitReader*>::iterator i = fReaders.find(name);
   
   if (i != fReaders.end())
      return i->second;
   else
      return 0;
}

void CbmBinnedHitReader::AddReader(const char* name, TClonesArray* hitArray)
{
   map<string, CbmBinnedHitReader*>::iterator i = fReaders.find(name);
   
   if (i != fReaders.end())
      return;
   
   string nameStr(name);
   CbmBinnedHitReader* newReader = 0;
   
   if (nameStr == "sts")
      newReader = new CbmBinnedStsHitReader(hitArray);
   else if (nameStr == "rich")
      newReader = new CbmBinnedRichHitReader(hitArray);
   else if (nameStr == "much")
      newReader = new CbmBinnedMuchHitReader(hitArray);
   else if (nameStr == "trd")
      newReader = new CbmBinnedTrdHitReader(hitArray);
   else if (nameStr == "tof")
      newReader = new CbmBinnedTofHitReader(hitArray);
   
   if (0 != newReader)
      fReaders[name] = newReader;
}
