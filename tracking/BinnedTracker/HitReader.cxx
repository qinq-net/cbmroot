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

class CbmBinnedMCPointReader;

class CbmBinnedMCTrackReader : public CbmBinnedHitReader
{
private:
   struct Point
   {
      Double_t x;
      Double_t y;
      Double_t z;
   };
   
   struct Track
   {
      Int_t motherInd;
      Double_t x;
      Double_t y;
      Double_t z;
      map<Double_t, list<Point> > points;
      
      Track() : motherInd(-1), x(0), y(0), z(0), points() {}
      
      void Init(Int_t mi, Double_t vx, Double_t vy, Double_t vz, const set<Double_t>& stationZs)
      {
         motherInd = mi;
         x = vx;
         y = vy;
         z = vz;
         
         for (set<Double_t>::const_iterator i = stationZs.begin(); i != stationZs.end(); ++i)
            points[*i] = list<Point> ();
      }
   };
   
public:
   static CbmBinnedMCTrackReader* Instance()
   {
      static CbmBinnedMCTrackReader* instance = 0;
      
      if (0 == instance)
      {
         AddReader("mctrack");
         instance = static_cast<CbmBinnedMCTrackReader*> (CbmBinnedHitReader::Instance("mctrack"));
      }
      
      return instance;
   }
   
public:
   CbmBinnedMCTrackReader() : CbmBinnedHitReader(), fStationZs(), fTracks(), fXScats(), fYScats(), fNofEvents(0)
   {
      FairRootManager* ioman = FairRootManager::Instance();
      fHitArray = static_cast<TClonesArray*> (ioman->GetObject("MCTrack"));
   }
   
   void AddStation(CbmBinnedStation* station)
   {
      fStationZs.insert(station->GetMinZ());
      fXScats.push_back(0);
      fYScats.push_back(0);
   }
   
   void Read()
   {
      fTracks.clear();
      Int_t nofTracks = fHitArray->GetEntriesFast();
      fTracks.resize(nofTracks);
   
      for (Int_t i = 0; i < nofTracks; ++i)
      {
         const CbmMCTrack* mcTrack = static_cast<const CbmMCTrack*> (fHitArray->At(i));
         Int_t motherInd = mcTrack->GetMotherId();
         Double_t x = mcTrack->GetStartX();
         Double_t y = mcTrack->GetStartY();
         Double_t z = mcTrack->GetStartZ();
         fTracks[i].Init(motherInd, x, y, z, fStationZs);
      }
   }
   
   void Handle()
   {
      ++fNofEvents;
      
      for (vector<Track>::const_iterator i = fTracks.begin(); i != fTracks.end(); ++i)
      {
         const Track& track = *i;
         
         if (track.motherInd >= 0)
            continue;
         
         bool isRef = true;
         
         for (map<Double_t, list<Point> >::const_iterator j = track.points.begin(); j != track.points.end(); ++j)
         {
            if (j->second.empty())
            {
               isRef = false;
               break;
            }
         }
         
         if (!isRef)
            continue;
         
         Point vertexPoint = { track.x, track.y, track.z };
         list<Point> vertexList;
         vertexList.push_back(vertexPoint);
         map<Double_t, list<Point> >::const_iterator jPrev = track.points.end();
         
         for (map<Double_t, list<Point> >::const_iterator j = track.points.begin(); j != track.points.end(); ++j)
         {
            map<Double_t, list<Point> >::const_iterator jNext = j;
            ++jNext;
            
            if (jNext == track.points.end())
               break;
            
            const list<Point>& points0 = (j == track.points.begin()) ? vertexList : jPrev->second;
            const list<Point>& points1 = j->second;
            const list<Point>& points2 = jNext->second;
            
            for (list<Point>::const_iterator k = points0.begin(); k != points0.end(); ++k)
            {
               const Point& p0 = *k;
               
               for (list<Point>::const_iterator l = points1.begin(); l != points1.end(); ++l)
               {
                  const Point& p1 = *l;
                  
                  for (list<Point>::const_iterator m = points2.begin(); m != points2.end(); ++m)
                  {
                     const Point& p2 = *m;
                  }
               }
            }
            
            jPrev = j;
         }
      }
   }
   
   void Finish()
   {
      
   }
   
   void AddPoint(Int_t trackInd, Double_t stationZ, Double_t x, Double_t y, Double_t z)
   {
      Track& track = fTracks[trackInd];
      Point point = { x, y, z };
      track.points[stationZ].push_back(point);
   }
   
private:
   set<Double_t> fStationZs;
   vector<Track> fTracks;
   vector<Double_t> fXScats;
   vector<Double_t> fYScats;
   int fNofEvents;
};

class CbmBinnedMCPointReader : public CbmBinnedHitReader
{
protected:
   CbmBinnedMCPointReader() : CbmBinnedHitReader(), fTrackReader(CbmBinnedMCTrackReader::Instance()), fStationZs() {}
   CbmBinnedMCPointReader(const CbmBinnedMCPointReader&) = delete;
   CbmBinnedMCPointReader& operator=(const CbmBinnedMCPointReader&) = delete;
   
public:
   void AddStation(CbmBinnedStation* station)
   {
      fTrackReader->AddStation(station);
      fStationZs.push_back(station->GetMinZ());
   }
   
protected:
   CbmBinnedMCTrackReader* fTrackReader;
   vector<Double_t> fStationZs;
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
         Int_t trackInd = point->GetTrackID();
         
         if (trackInd < 0)
            continue;
         
         Int_t stationNumber = CbmStsSetup::Instance()->GetStationNumber(point->GetDetectorID());
         Double_t stationZ = fStationZs[stationNumber];
         fTrackReader->AddPoint(trackInd, stationZ, (point->GetXIn() + point->GetXOut()) / 2, (point->GetYIn() + point->GetYOut()) / 2,
            (point->GetZIn() + point->GetZOut()) / 2);
      }
   }
};

#include "rich/CbmRichDigi.h"
#include "CbmLink.h"
#include "rich/CbmRichPoint.h"
#include "CbmMatch.h"

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

class CbmBinnedRichMCReader : public CbmBinnedMCPointReader
{
public:
   CbmBinnedRichMCReader() : CbmBinnedMCPointReader()
   {
      FairRootManager* ioman = FairRootManager::Instance();
      fHitArray = static_cast<TClonesArray*> (ioman->GetObject("RichPoint"));
   }
   
   void Read()
   {
      Int_t nofPoints = fHitArray->GetEntriesFast();
   
      for (Int_t i = 0; i < nofPoints; ++i)
      {
         const CbmRichPoint* point = static_cast<const CbmRichPoint*> (fHitArray->At(i));
         Int_t trackInd = point->GetTrackID();
         
         if (trackInd < 0)
            continue;
         
         Double_t stationZ = fStationZs[0];
         fTrackReader->AddPoint(trackInd, stationZ, point->GetX(), point->GetY(), point->GetZ());
      }
   }
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
};

class CbmBinnedMuchMCReader : public CbmBinnedMCPointReader
{
public:
   CbmBinnedMuchMCReader() : CbmBinnedMCPointReader()
   {
      FairRootManager* ioman = FairRootManager::Instance();
      fHitArray = static_cast<TClonesArray*> (ioman->GetObject("MuchPoint"));
   }
   
   void Read()
   {
      Int_t nofPoints = fHitArray->GetEntriesFast();
   
      for (Int_t i = 0; i < nofPoints; ++i)
      {
         const CbmMuchPoint* point = static_cast<const CbmMuchPoint*> (fHitArray->At(i));
         Int_t trackInd = point->GetTrackID();
         
         if (trackInd < 0)
            continue;
         
         Int_t muchStationNumber = CbmMuchGeoScheme::GetStationIndex(point->GetDetectorID());
         Int_t layerNumber = CbmMuchGeoScheme::GetLayerIndex(point->GetDetectorID());
         Int_t stationNumber = muchStationNumber * 3 + layerNumber;
         Double_t stationZ = fStationZs[stationNumber];
         fTrackReader->AddPoint(trackInd, stationZ, (point->GetXIn() + point->GetXOut()) / 2, (point->GetYIn() + point->GetYOut()) / 2,
            (point->GetZIn() + point->GetZOut()) / 2);
      }
   }
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
};

class CbmBinnedTrdMCReader : public CbmBinnedMCPointReader
{
public:
   CbmBinnedTrdMCReader() : CbmBinnedMCPointReader()
   {
      FairRootManager* ioman = FairRootManager::Instance();
      fHitArray = static_cast<TClonesArray*> (ioman->GetObject("TrdPoint"));
   }
   
   void Read()
   {
      Int_t nofPoints = fHitArray->GetEntriesFast();
   
      for (Int_t i = 0; i < nofPoints; ++i)
      {
         const CbmTrdPoint* point = static_cast<const CbmTrdPoint*> (fHitArray->At(i));
         Int_t trackInd = point->GetTrackID();
         
         if (trackInd < 0)
            continue;
         
         Int_t stationNumber = CbmTrdAddress::GetLayerId(point->GetDetectorID());
         Double_t stationZ = fStationZs[stationNumber];
         fTrackReader->AddPoint(trackInd, stationZ, (point->GetXIn() + point->GetXOut()) / 2, (point->GetYIn() + point->GetYOut()) / 2,
            (point->GetZIn() + point->GetZOut()) / 2);
      }
   }
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

#include "CbmTofPoint.h"

class CbmBinnedTofMCReader : public CbmBinnedMCPointReader
{
public:
   CbmBinnedTofMCReader() : CbmBinnedMCPointReader()
   {
      FairRootManager* ioman = FairRootManager::Instance();
      fHitArray = static_cast<TClonesArray*> (ioman->GetObject("TofPoint"));
   }
   
   void Read()
   {
      Int_t nofPoints = fHitArray->GetEntriesFast();
   
      for (Int_t i = 0; i < nofPoints; ++i)
      {
         const CbmTofPoint* point = static_cast<const CbmTofPoint*> (fHitArray->At(i));
         Int_t trackInd = point->GetTrackID();
         
         if (trackInd < 0)
            continue;

         Double_t stationZ = fStationZs[0];
         fTrackReader->AddPoint(trackInd, stationZ, point->GetX(), point->GetY(), point->GetZ());
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
   
   void Handle()
   {
      for (map<string, CbmBinnedHitReader*>::iterator i = fReaders.begin(); i != fReaders.end(); ++i)
         i->second->Handle();
   }
   
   void Finish()
   {
      for (map<string, CbmBinnedHitReader*>::iterator i = fReaders.begin(); i != fReaders.end(); ++i)
         i->second->Finish();
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
   else if (nameStr == "stsmc")
      newReader = new CbmBinnedStsMCReader;
   else if (nameStr == "richmc")
      newReader = new CbmBinnedRichMCReader;
   else if (nameStr == "muchmc")
      newReader = new CbmBinnedMuchMCReader;
   else if (nameStr == "trdmc")
      newReader = new CbmBinnedTrdMCReader;
   else if (nameStr == "tofmc")
      newReader = new CbmBinnedTofMCReader;
   
   if (0 != newReader)
      fReaders[name] = newReader;
}
