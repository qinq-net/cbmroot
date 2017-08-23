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

static map<string, map<int, Double_t> > maxDx;
static map<string, map<int, Double_t> > maxDy;
static map<string, map<int, Double_t> > maxDt;
static Double_t maxTofDtx = 0;
static Double_t maxTofDty = 0;

static void UpdateMax(map<string, map<int, Double_t> >& errorMap, string stationName, int stationNumber, Double_t v)
{
   map<string, map<int, Double_t> >::iterator i = errorMap.find(stationName);
   
   if (i == errorMap.end())
   {
      errorMap[stationName][stationNumber] = v;
      return;
   }

   map<int, Double_t>::iterator j = i->second.find(stationNumber);
   
   if (j == i->second.end())
   {
      i->second[stationNumber] = v;
      return;
   }
   
   if (j->second < v)
      j->second = v;
}

static void UpdateMax(string stationName, int stationNumber, const CbmPixelHit* hit)
{
   UpdateMax(maxDx, stationName, stationNumber, hit->GetDx());
   UpdateMax(maxDy, stationName, stationNumber, hit->GetDy());
   UpdateMax(maxDt, stationName, stationNumber, hit->GetTimeError());
   
   if (stationName == "Tof")
   {
      Double_t dtx = hit->GetDx() / hit->GetZ();
      
      if (maxTofDtx < dtx)
         maxTofDtx = dtx;
      
      Double_t dty = hit->GetDy() / hit->GetZ();
      
      if (maxTofDty < dty)
         maxTofDty = dty;
   }
}

#include <iostream>

static void DumpMax(map<string, map<int, Double_t> >& errorMap, const char* errorName)
{
   cout << errorName << ":" << endl;
   
   for (map<string, map<int, Double_t> >::const_iterator i = errorMap.begin(); i != errorMap.end(); ++i)
   {
      for (map<int, Double_t>::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
         cout << i->first << "[" << j->first << "] = " << j->second << endl;
   }
   
   cout << endl;
}

static void DumpMax()
{
   DumpMax(maxDx, "Dx");
   DumpMax(maxDy, "Dy");
   DumpMax(maxDt, "Dt");
   cout << "ToF dtx: " << maxTofDtx << endl;
   cout << "ToF dty: " << maxTofDty << endl;
}

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
         int stationNumber = CbmStsSetup::Instance()->GetStationNumber(hit->GetAddress());
         fStations[stationNumber]->AddHit(hit, i);
         UpdateMax("Sts", stationNumber, hit);
      }
   }
};

#include "FairRootManager.h"
#include "rich/CbmRichDigi.h"
#include "CbmLink.h"
#include "rich/CbmRichPoint.h"
#include "CbmMatch.h"

class CbmBinnedRichHitReader : public CbmBinnedHitReader
{
public:
   explicit CbmBinnedRichHitReader(TClonesArray* hitArray) : CbmBinnedHitReader(hitArray), fDigiArray(0), fPointArray(0)
   {
      FairRootManager* ioman = FairRootManager::Instance();
      fDigiArray = static_cast<TClonesArray*> (ioman->GetObject("RichDigi"));
      fPointArray = static_cast<TClonesArray*> (ioman->GetObject("RichPoint"));
   }
   
   CbmBinnedRichHitReader(const CbmBinnedRichHitReader&) = delete;
   CbmBinnedRichHitReader& operator=(const CbmBinnedRichHitReader&) = delete;
   
   void Read()
   {
      Int_t nofHits = fHitArray->GetEntriesFast();
   
      for (Int_t i = 0; i < nofHits; ++i)
      {
         /*const */CbmPixelHit* hit = static_cast</*const */CbmPixelHit*> (fHitArray->At(i));
         fStations[0]->AddHit(hit, i);
         
         int digiIndex = hit->GetRefId();
         const CbmRichDigi* digi = static_cast<const CbmRichDigi*>(fDigiArray->At(digiIndex));
         CbmMatch* digiMatch = digi->GetMatch();

         vector<CbmLink> links = digiMatch->GetLinks();
         Double_t t = 0;
         int cnt = 0;
         
         for (int j = 0; j < links.size(); ++j)
         {
            int pointId = links[j].GetIndex();
            
            if (pointId < 0)
               continue;
            
            const CbmRichPoint* point = static_cast<const CbmRichPoint*>(fPointArray->At(pointId));
            t += point->GetTime();
            ++cnt;
         }
         
         if (0 != cnt)
            t /= cnt;
         
         hit->SetTime(t);
         hit->SetTimeError(4);
         hit->SetDx(0.175);
         hit->SetDy(0.175);
         UpdateMax("Rich", 0, hit);
      }
   }
   
private:
   TClonesArray* fDigiArray;
   TClonesArray* fPointArray;
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
         int muchStationNumber = CbmMuchGeoScheme::GetStationIndex(hit->GetAddress());
         int layerNumber = CbmMuchGeoScheme::GetLayerIndex(hit->GetAddress());
         int stationNumber = muchStationNumber * 3 + layerNumber;
         fStations[stationNumber]->AddHit(hit, i);
         UpdateMax("Much", stationNumber, hit);
      }
   }
};

class CbmBinnedTrdHitReader : public CbmBinnedHitReader
{
public:
   explicit CbmBinnedTrdHitReader(TClonesArray* hitArray) : CbmBinnedHitReader(hitArray), fClusterArray(0), fDigiMatchArray(0), fPointArray(0)
   {
      FairRootManager* ioman = FairRootManager::Instance();
      fClusterArray = static_cast<TClonesArray*> (ioman->GetObject("TrdCluster"));
      fDigiMatchArray = static_cast<TClonesArray*> (ioman->GetObject("TrdDigiMatch"));
      fPointArray = static_cast<TClonesArray*> (ioman->GetObject("TrdPoint"));
   }
   
   CbmBinnedTrdHitReader(const CbmBinnedTrdHitReader&) = delete;
   CbmBinnedTrdHitReader& operator=(const CbmBinnedTrdHitReader&) = delete;
   
   void Read()
   {
      Int_t nofHits = fHitArray->GetEntriesFast();
   
      for (Int_t i = 0; i < nofHits; ++i)
      {
         /*const */CbmTrdHit* hit = static_cast</*const */CbmTrdHit*> (fHitArray->At(i));
         int stationNumber = hit->GetPlaneId();
         fStations[stationNumber]->AddHit(hit, i);
         
         Int_t clusterId = hit->GetRefId();
         const CbmCluster* cluster = static_cast<const CbmCluster*> (fClusterArray->At(clusterId));
         Int_t nDigis = cluster->GetNofDigis();
         Double_t t = 0;
         int cnt = 0;
         
         for (Int_t j = 0; j < nDigis; ++j)
         {
            const CbmMatch* digiMatch = static_cast<const CbmMatch*> (fDigiMatchArray->At(cluster->GetDigi(j)));
            Int_t nMCs = digiMatch->GetNofLinks();

            for (Int_t k = 0; k < nMCs; ++k)
            {
               const CbmLink& lnk = digiMatch->GetLink(k);
               Int_t pointId = lnk.GetIndex();
               const FairMCPoint* point = static_cast<const FairMCPoint*> (fPointArray->At(pointId));
               t += point->GetTime();
               ++cnt;
            }
         }
         
         if (0 != cnt)
            t /= cnt;
         
         hit->SetTime(t);
         hit->SetTimeError(4);
         UpdateMax("Trd", stationNumber, hit);
      }
   }
   
private:
   TClonesArray* fClusterArray;
   TClonesArray* fDigiMatchArray;
   TClonesArray* fPointArray;
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
         /*const */CbmPixelHit* hit = static_cast</*const */CbmPixelHit*> (fHitArray->At(i));
         fStations[0]->AddHit(hit, i);
         hit->SetTimeError(4);
         UpdateMax("Tof", 0, hit);
      }
   }
};

class CbmBinnedAllHitReader : public CbmBinnedHitReader
{
public:
   CbmBinnedAllHitReader() : CbmBinnedHitReader(0) {}
   
   void Read()
   {
      for (map<string, CbmBinnedHitReader*>::iterator i = fReaders.begin(); i != fReaders.end(); ++i)
         i->second->Read();
      
      DumpMax();
   }
};

CbmBinnedHitReader* CbmBinnedHitReader::Instance()
{
   static CbmBinnedAllHitReader* allReader = 0;
   
   if (0 == allReader)
      allReader = new CbmBinnedAllHitReader;
   
   return allReader;
}

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
