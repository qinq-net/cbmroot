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
#include "CbmMuchPixelHit.h"
#include "FairRootManager.h"

using namespace std;

//#define DO_ERROR_STAT

#ifdef DO_ERROR_STAT
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

#endif//DO_ERROR_STAT

CbmBinnedSettings* CbmBinnedHitReader::fSettings = 0;

CbmBinnedHitReader::CbmBinnedHitReader() : fStations(), fHitArray(0)
{
}

CbmBinnedHitReader::~CbmBinnedHitReader()
{
}

class CbmBinnedStsHitReader : public CbmBinnedHitReader
{
public:
   CbmBinnedStsHitReader() : CbmBinnedHitReader()
   {
      FairRootManager* ioman = FairRootManager::Instance();
      fHitArray = static_cast<TClonesArray*> (ioman->GetObject("StsHit"));
   }
   
   void Read()
   {
      Int_t nofHits = fHitArray->GetEntriesFast();
   
      for (Int_t i = 0; i < nofHits; ++i)
      {
         const CbmPixelHit* hit = static_cast<const CbmPixelHit*> (fHitArray->At(i));
         int stationNumber = CbmStsSetup::Instance()->GetStationNumber(hit->GetAddress());
         fStations[stationNumber]->AddHit(hit, i);
#ifdef DO_ERROR_STAT
         UpdateMax("Sts", stationNumber, hit);
#endif//DO_ERROR_STAT
      }
   }
};

#include "CbmMCTrack.h"

class CbmBinnedMCTrackReader : public CbmBinnedHitReader
{
public:
   static CbmBinnedMCTrackReader* Instance()
   {
      static CbmBinnedMCTrackReader* instance = 0;
      
      if (0 == instance)
         AddReader("mctrack");
      
      return instance;
   }
   
public:
   CbmBinnedMCTrackReader() : CbmBinnedHitReader()
   {
      FairRootManager* ioman = FairRootManager::Instance();
      fHitArray = static_cast<TClonesArray*> (ioman->GetObject("MCTrack"));
   }
   
   void Read()
   {
      Int_t nofTracks = fHitArray->GetEntriesFast();
   
      for (Int_t i = 0; i < nofTracks; ++i)
      {
         const CbmMCTrack* track = static_cast<const CbmMCTrack*> (fHitArray->At(i));
         //fStations[stationNumber]->AddHit(hit, i);
      }
   }
};

class CbmBinnedMCPointReader : public CbmBinnedHitReader
{
protected:
   CbmBinnedMCPointReader() : CbmBinnedHitReader(), fTrackReader(CbmBinnedMCTrackReader::Instance()) {}
   
protected:
   CbmBinnedMCTrackReader* fTrackReader;
};

#include "CbmStsPoint.h"

class CbmBinnedStsMCReader : public CbmBinnedMCPointReader
{
public:
   CbmBinnedStsMCReader() : CbmBinnedMCPointReader()
   {
      FairRootManager* ioman = FairRootManager::Instance();
      fHitArray = static_cast<TClonesArray*> (ioman->GetObject("StsPoint"));
   }
   
   void Read()
   {
      Int_t nofPoints = fHitArray->GetEntriesFast();
   
      for (Int_t i = 0; i < nofPoints; ++i)
      {
         const CbmStsPoint* point = static_cast<const CbmStsPoint*> (fHitArray->At(i));
         int stationNumber = CbmStsSetup::Instance()->GetStationNumber(point->GetDetectorID());
         //fStations[stationNumber]->AddHit(hit, i);
      }
   }
};

#include "rich/CbmRichDigi.h"
#include "CbmLink.h"
#include "rich/CbmRichPoint.h"
#include "CbmMatch.h"
//#include "CbmMCDataArray.h"
//#include "CbmMCDataManager.h"

class CbmBinnedRichHitReader : public CbmBinnedHitReader
{
public:
   CbmBinnedRichHitReader() : CbmBinnedHitReader()
   {
      FairRootManager* ioman = FairRootManager::Instance();
      fHitArray = static_cast<TClonesArray*> (ioman->GetObject("RichHit"));
   }
   
   CbmBinnedRichHitReader(const CbmBinnedRichHitReader&) = delete;
   CbmBinnedRichHitReader& operator=(const CbmBinnedRichHitReader&) = delete;
   
   void Read()
   {
      Int_t nofHits = fHitArray->GetEntriesFast();
   
      for (Int_t i = 0; i < nofHits; ++i)
      {
         /*const */CbmPixelHit* hit = static_cast</*const */CbmPixelHit*> (fHitArray->At(i));
         
         /*int digiIndex = hit->GetRefId();
         const CbmRichDigi* digi = static_cast<const CbmRichDigi*>(fDigiArray->At(digiIndex));
         CbmMatch* digiMatch = digi->GetMatch();

         vector<CbmLink> links = digiMatch->GetLinks();
         Double_t t = 0;
         int cnt = 0;
         
         for (int j = 0; j < links.size(); ++j)
         {            
            const CbmRichPoint* point = static_cast<const CbmRichPoint*> (fPointArray->Get(links[j]));
            t += point->GetTime();
            ++cnt;
         }
         
         if (0 < cnt)
            t /= cnt;*/
         
         hit->SetTime(0);
         hit->SetTimeError(4);
         hit->SetDx(0.175);
         hit->SetDy(0.175);
         fStations[0]->AddHit(hit, i);
#ifdef DO_ERROR_STAT
         UpdateMax("Rich", 0, hit);
#endif//DO_ERROR_STAT
      }
   }
   
private:
   //TClonesArray* fDigiArray;
   //CbmMCDataArray* fPointArray;
};

#include "CbmMuchCluster.h"
#include "CbmMuchDigi.h"
#include "CbmMuchPoint.h"
#include "CbmTrdPoint.h"

class CbmBinnedMuchHitReader : public CbmBinnedHitReader
{
public:
   CbmBinnedMuchHitReader() : CbmBinnedHitReader()
   {
      FairRootManager* ioman = FairRootManager::Instance();
      fHitArray = static_cast<TClonesArray*> (ioman->GetObject("MuchPixelHit"));
   }
   
   CbmBinnedMuchHitReader(const CbmBinnedMuchHitReader&) = delete;
   CbmBinnedMuchHitReader& operator=(const CbmBinnedMuchHitReader&) = delete;
   
   void Read()
   {
      Int_t nofHits = fHitArray->GetEntriesFast();
   
      for (Int_t i = 0; i < nofHits; ++i)
      {
         /*const */CbmMuchPixelHit* hit = static_cast</*const */CbmMuchPixelHit*> (fHitArray->At(i));
         int muchStationNumber = CbmMuchGeoScheme::GetStationIndex(hit->GetAddress());
         int layerNumber = CbmMuchGeoScheme::GetLayerIndex(hit->GetAddress());
         int stationNumber = muchStationNumber * 3 + layerNumber;
         
         /*Double_t t = 0;
         int cnt = 0;
         Int_t clusterId = hit->GetRefId();
         const CbmMuchCluster* cluster = static_cast<const CbmMuchCluster*> (fMuchClusters->At(clusterId));
         Int_t nofDigis = cluster->GetNofDigis();

         for (Int_t j = 0; j < nofDigis; ++j)
         {
            Int_t digiId = cluster->GetDigi(j);
            const CbmMuchDigi* digi = static_cast<const CbmMuchDigi*> (fMuchDigis->At(digiId));
            const CbmMatch* match = digi->GetMatch();
            Int_t nofLinks = match->GetNofLinks();

            for (Int_t k = 0; k < nofLinks; ++k)
            {
               const CbmLink& link = match->GetLink(k);
               const CbmMuchPoint* muchPoint = static_cast<const CbmMuchPoint*> (fMuchPoints->Get(link));
               t += muchPoint->GetTime();
               ++cnt;
            }
         }
      
         if (0 < cnt)
            t /= cnt;*/
         
         hit->SetTime(0);
         
         fStations[stationNumber]->AddHit(hit, i);
#ifdef DO_ERROR_STAT
         UpdateMax("Much", stationNumber, hit);
#endif//DO_ERROR_STAT
      }
   }
   
//private:
   //TClonesArray* fMuchClusters;
   //TClonesArray* fMuchDigis;
   //CbmMCDataArray* fMuchPoints;
};

class CbmBinnedTrdHitReader : public CbmBinnedHitReader
{
public:
   CbmBinnedTrdHitReader() : CbmBinnedHitReader()
   {
      FairRootManager* ioman = FairRootManager::Instance();
      fHitArray = static_cast<TClonesArray*> (ioman->GetObject("TrdHit"));
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
         /*Int_t clusterId = hit->GetRefId();
         const CbmCluster* cluster = static_cast<const CbmCluster*> (fClusterArray->At(clusterId));
         Int_t nDigis = cluster->GetNofDigis();
         Double_t t = 0;
         Double_t x = 0;
         Double_t y = 0;
         int cnt = 0;
         int nofNegs = 0;
         
         for (Int_t j = 0; j < nDigis; ++j)
         {
            const CbmMatch* digiMatch = static_cast<const CbmMatch*> (fDigiMatchArray->At(cluster->GetDigi(j)));
            Int_t nMCs = digiMatch->GetNofLinks();

            for (Int_t k = 0; k < nMCs; ++k)
            {
               const CbmLink& lnk = digiMatch->GetLink(k);
               const CbmTrdPoint* point = static_cast<const CbmTrdPoint*> (fPointArray->Get(lnk));
               t += point->GetTime();
               x += (point->GetXIn() + point->GetXOut()) / 2;
               y += (point->GetYIn() + point->GetYOut()) / 2;
               ++cnt;
            }
         }
         
         if (0 < cnt)
         {
            t /= cnt;
            x /= cnt;
            //hit->SetX(x);
            y /= cnt;
            //hit->SetY(y);
         }
         else
            x = 1;*/
         
         //hit->SetTime(t);
         hit->SetTime(0);
         hit->SetTimeError(4);
         //hit->SetDx(hit->GetDx() * 4);
         //hit->SetDy(hit->GetDy() * 4);
         fStations[stationNumber]->AddHit(hit, i);
#ifdef DO_ERROR_STAT
         UpdateMax("Trd", stationNumber, hit);
#endif//DO_ERROR_STAT
      }
   }
   
//private:
   //TClonesArray* fClusterArray;
   //TClonesArray* fDigiMatchArray;
   //CbmMCDataArray* fPointArray;
};

class CbmBinnedTofHitReader : public CbmBinnedHitReader
{
public:
   CbmBinnedTofHitReader() : CbmBinnedHitReader()
   {
      FairRootManager* ioman = FairRootManager::Instance();
      fHitArray = static_cast<TClonesArray*> (ioman->GetObject("TofHit"));
   }
   
   void Read()
   {
      Int_t nofHits = fHitArray->GetEntriesFast();
   
      for (Int_t i = 0; i < nofHits; ++i)
      {
         /*const */CbmPixelHit* hit = static_cast</*const */CbmPixelHit*> (fHitArray->At(i));
         fStations[0]->AddHit(hit, i);
         hit->SetTimeError(4);
#ifdef DO_ERROR_STAT
         UpdateMax("Tof", 0, hit);
#endif//DO_ERROR_STAT
      }
   }
};

class CbmBinnedAllHitReader : public CbmBinnedHitReader
{
public:
   CbmBinnedAllHitReader() : CbmBinnedHitReader() {}
   
   void Read()
   {
      for (map<string, CbmBinnedHitReader*>::iterator i = fReaders.begin(); i != fReaders.end(); ++i)
         i->second->Read();
#ifdef DO_ERROR_STAT
      DumpMax();
#endif//DO_ERROR_STAT
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

void CbmBinnedHitReader::AddReader(const char* name)
{
   map<string, CbmBinnedHitReader*>::iterator i = fReaders.find(name);
   
   if (i != fReaders.end())
      return;
   
   string nameStr(name);
   CbmBinnedHitReader* newReader = 0;
   
   if (nameStr == "sts")
      newReader = new CbmBinnedStsHitReader;
   else if (nameStr == "rich")
      newReader = new CbmBinnedRichHitReader;
   else if (nameStr == "much")
      newReader = new CbmBinnedMuchHitReader;
   else if (nameStr == "trd")
      newReader = new CbmBinnedTrdHitReader;
   else if (nameStr == "tof")
      newReader = new CbmBinnedTofHitReader;
   else if (nameStr == "mctrack")
      newReader = new CbmBinnedMCTrackReader;
   
   if (0 != newReader)
      fReaders[name] = newReader;
}
