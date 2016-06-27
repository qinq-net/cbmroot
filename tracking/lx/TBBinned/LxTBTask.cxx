/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#define LXTB_QA

#ifdef LXTB_QA
#define LXTB_QA_IDEAL
#endif//LXTB_QA

#include "LxTBTask.h"
#include "FairLogger.h"
#include "CbmMCDataManager.h"
#include "CbmMuchGeoScheme.h"
#include "TRandom3.h"
#include "TMath.h"
#include "TDatabasePDG.h"
#include "TParticlePDG.h"
#include "CbmMCTrack.h"
#include <map>
#include <set>
#include "TClonesArray.h"
#include "TGeoCompositeShape.h"
#include "TGeoArb8.h"
#include "TGeoBoolNode.h"
#include "CbmMuchPixelHit.h"
#include "CbmTrdHit.h"
#include "CbmCluster.h"
#include "CbmMatch.h"

#ifdef LXTB_QA
#include "TH1F.h"
#include "TH2F.h"
#endif//LXTB_QA

ClassImp(LxTBFinder)

LxTbBinnedFinder::SignalParticle LxTbBinnedFinder::particleDescs[] = { { "jpsi", 443, 3.0, true  }, { "omega", 223, 1.5, false }, { "", -1, 0, false } };

LxTBFinder::LxTBFinder() : fMuchMCPoints(0), fMuchPixelHits(0), fMuchClusters(0), fMuchPixelDigiMatches(0),
   fTrdMCPoints(0), fTrdHits(0), fTrdClusters(0), fTrdDigiMatches(0),
   isEvByEv(false), fFinder(0), hasTrd(false), useTrd(true), fSignalParticle("jpsi")
{
}

static bool HasTrd()
{
   gGeoManager->cd("/cave_1");
   TObjArray* children = gGeoManager->GetCurrentNode()->GetNodes();

   if (0 != children)
   {
      TObject* child = children->First();

      while (child)
      {
         TString name(static_cast<TGeoNode*> (child)->GetName());
         
         if (name.Contains("trd", TString::kIgnoreCase))
            return true;

         child = children->After(child);
      }
   }
   
   return false;
}

static void FindGeoChild(TGeoNode* node, const char* name, list<TGeoNode*>& results)
{
   Int_t nofChildren = node->GetNdaughters();
   
   for (Int_t i = 0; i < nofChildren; ++i)
   {
      TGeoNode* child = node->GetDaughter(i);
      TString childName(child->GetName());
      
      if (childName.Contains(name, TString::kIgnoreCase))
         results.push_back(child);
   }
}

void LxTBFinder::HandleGeometry()
{
   Double_t localCoords[3] = {0., 0., 0.};
   Double_t globalCoords[3];
   TGeoNavigator* pNavigator = gGeoManager->GetCurrentNavigator();
   gGeoManager->cd("/cave_1");
   list<TGeoNode*> detectors;
   FindGeoChild(gGeoManager->GetCurrentNode(), "much", detectors);
   
   for (list<TGeoNode*>::iterator i = detectors.begin(); i != detectors.end(); ++i)
   {
      TGeoNode* detector = *i;
      pNavigator->CdDown(detector);
      list<TGeoNode*> stations;
      FindGeoChild(detector, "station", stations);
      int stationNumber = 0;
      
      for (list<TGeoNode*>::iterator j = stations.begin(); j != stations.end(); ++j)
      {
         TGeoNode* station = *j;
         pNavigator->CdDown(station);
         list<TGeoNode*> layers;
         FindGeoChild(station, "layer", layers);
         int layerNumber = 0;
      
         for (list<TGeoNode*>::iterator k = layers.begin(); k != layers.end(); ++k)
         {
            TGeoNode* layer = *k;
            
            if (1 == layerNumber)
            {
               pNavigator->CdDown(layer);
               gGeoManager->LocalToMaster(localCoords, globalCoords);
               fFinder->stations[stationNumber].stationNumber = stationNumber;
               fFinder->stations[stationNumber].z = globalCoords[2];
               fFinder->stations[stationNumber].minX = 0;
               fFinder->stations[stationNumber].maxX = 0;
               fFinder->stations[stationNumber].binSizeX = 0;
               fFinder->stations[stationNumber].minY = 0;
               fFinder->stations[stationNumber].maxY = 0;
               fFinder->stations[stationNumber].binSizeY = 0;
               
               list<TGeoNode*> actives;
               FindGeoChild(layer, "active", actives);
      
               for (list<TGeoNode*>::iterator l = actives.begin(); l != actives.end(); ++l)
               {
                  TGeoNode* active = *l;
                  pNavigator->CdDown(active);
                  TGeoCompositeShape* cs = dynamic_cast<TGeoCompositeShape*> (active->GetVolume()->GetShape());
                  TGeoBoolNode* bn = cs->GetBoolNode();
                  TGeoTrap* trap = dynamic_cast<TGeoTrap*> (bn->GetLeftShape());

                  if (0 != trap)
                  {
                     Double_t* xy = trap->GetVertices();

                     for (int m = 0; m < 4; ++m)
                     {
                        Double_t localActiveCoords[3] = {xy[2 * m], xy[2 * m + 1], 0.};
                        Double_t globalActiveCoords[3];
                        gGeoManager->LocalToMaster(localActiveCoords, globalActiveCoords);

                        if (fFinder->stations[stationNumber].minY > globalActiveCoords[1])
                           fFinder->stations[stationNumber].minY = globalActiveCoords[1];

                        if (fFinder->stations[stationNumber].maxY < globalActiveCoords[1])
                           fFinder->stations[stationNumber].maxY = globalActiveCoords[1];

                        if (fFinder->stations[stationNumber].minX > globalActiveCoords[0])
                           fFinder->stations[stationNumber].minX = globalActiveCoords[0];

                        if (fFinder->stations[stationNumber].maxX < globalActiveCoords[0])
                           fFinder->stations[stationNumber].maxX = globalActiveCoords[0];
                     }
                  }
                  
                  pNavigator->CdUp();
               }
               
               pNavigator->CdUp();
            }
            
            ++layerNumber;
         }
         
         ++stationNumber;
         pNavigator->CdUp();
      }
      
      int nofStations = stationNumber;
      
      list<TGeoNode*> absorbers;
      FindGeoChild(detector, "absorber", absorbers);
      int absorberNumber = 0;
      
      for (list<TGeoNode*>::iterator j = absorbers.begin(); j != absorbers.end(); ++j)
      {
         TGeoNode* absorber = *j;
         pNavigator->CdDown(absorber);
         TGeoVolume* absVol = gGeoManager->GetCurrentVolume();
         const TGeoBBox* absShape = static_cast<const TGeoBBox*> (absVol->GetShape());
         
         if (absorberNumber < nofStations)
         {
            fFinder->stations[absorberNumber].absorber.width = 2 * absShape->GetDZ();
            fFinder->stations[absorberNumber].absorber.radLength = absVol->GetMaterial()->GetRadLen();
            fFinder->stations[absorberNumber].absorber.rho = absVol->GetMaterial()->GetDensity();
            fFinder->stations[absorberNumber].absorber.Z = absVol->GetMaterial()->GetZ();
            fFinder->stations[absorberNumber].absorber.A = absVol->GetMaterial()->GetA();
         }
         else
         {
            fFinder->trdStation.absorber.width = 2 * absShape->GetDZ();
            fFinder->trdStation.absorber.radLength = absVol->GetMaterial()->GetRadLen();
            fFinder->trdStation.absorber.rho = absVol->GetMaterial()->GetDensity();
            fFinder->trdStation.absorber.Z = absVol->GetMaterial()->GetZ();
            fFinder->trdStation.absorber.A = absVol->GetMaterial()->GetA();
         }
         
         ++absorberNumber;
         pNavigator->CdUp();
      }
      
      pNavigator->CdUp();
   }
   
   fFinder->trdStation.minX = 0;
   fFinder->trdStation.maxX = 0;
   fFinder->trdStation.binSizeX = 0;
   fFinder->trdStation.minY = 0;
   fFinder->trdStation.maxY = 0;
   fFinder->trdStation.binSizeY = 0;
   detectors.clear();
   FindGeoChild(gGeoManager->GetCurrentNode(), "trd", detectors);
   
   for (list<TGeoNode*>::iterator i = detectors.begin(); i != detectors.end(); ++i)
   {
      TGeoNode* detector = *i;
      pNavigator->CdDown(detector);
      list<TGeoNode*> layers;
      FindGeoChild(detector, "layer", layers);
      int layerNumber = 0;
      
      for (list<TGeoNode*>::iterator j = layers.begin(); j != layers.end(); ++j)
      {
         TGeoNode* layer = *j;
         pNavigator->CdDown(layer);
         list<TGeoNode*> modules;
         FindGeoChild(layer, "module", modules);
         
         for (list<TGeoNode*>::iterator k = modules.begin(); k != modules.end(); ++k)
         {
            TGeoNode* module = *k;
            pNavigator->CdDown(module);
            list<TGeoNode*> padCoppers;
            FindGeoChild(module, "padcopper", padCoppers);
         
            for (list<TGeoNode*>::iterator l = padCoppers.begin(); l != padCoppers.end(); ++l)
            {
               TGeoNode* padCopper = *l;
               pNavigator->CdDown(padCopper);               
               TGeoBBox* pBox = static_cast<TGeoBBox*> (padCopper->GetVolume()->GetShape());
               pBox->ComputeBBox();
               gGeoManager->LocalToMaster(localCoords, globalCoords);
               fFinder->trdStation.Zs[layerNumber] = globalCoords[2];

               if (fFinder->trdStation.minY > globalCoords[1] - pBox->GetDY())
                  fFinder->trdStation.minY = globalCoords[1] - pBox->GetDY();

               if (fFinder->trdStation.maxY < globalCoords[1] + pBox->GetDY())
                  fFinder->trdStation.maxY = globalCoords[1] + pBox->GetDY();

               if (fFinder->trdStation.minX > globalCoords[0] - pBox->GetDX())
                  fFinder->trdStation.minX = globalCoords[0] - pBox->GetDX();

               if (fFinder->trdStation.maxX < globalCoords[0] + pBox->GetDX())
                  fFinder->trdStation.maxX = globalCoords[0] + pBox->GetDX();
               
               pNavigator->CdUp();
            }
            
            pNavigator->CdUp();
         }
         
         ++layerNumber;
         pNavigator->CdUp();
      }
      
      pNavigator->CdUp();
   }
}

#ifdef LXTB_QA
static list<scaltype> shortSignalMCTimes;
static list<scaltype> longSignalMCTimes;
static list<pair<scaltype, scaltype> > triggerTimes_trd0_sign0_dist0;
static list<pair<scaltype, scaltype> > triggerTimes_trd0_sign0_dist1;
static list<pair<scaltype, scaltype> > triggerTimes_trd0_sign1_dist0;
static list<pair<scaltype, scaltype> > triggerTimes_trd0_sign1_dist1;
static list<pair<scaltype, scaltype> > triggerTimes_trd1_sign0_dist0;
static list<pair<scaltype, scaltype> > triggerTimes_trd1_sign0_dist1;
static list<pair<scaltype, scaltype> > triggerTimes_trd1_sign1_dist0;
static list<pair<scaltype, scaltype> > triggerTimes_trd1_sign1_dist1;
#endif//LXTB_QA

InitStatus LxTBFinder::Init()
{
    FairRootManager* ioman = FairRootManager::Instance();
    
    if (0 == ioman)
        fLogger->Fatal(MESSAGE_ORIGIN, "No FairRootManager");
    
    speedOfLight = 100 * TMath::C();// Multiply by 100 to express in centimeters.
    gMuonMass = TDatabasePDG::Instance()->GetParticle(13)->Mass();
    gElectronMass = TDatabasePDG::Instance()->GetParticle(11)->Mass();
    TObjArray* absorbers = CbmMuchGeoScheme::Instance()->GetAbsorbers();
    
    hasTrd = useTrd ? HasTrd() : false;
    pair<int, int> stSpatLimits[] = { { 20, 20 }, { 20, 20 }, { 20, 20 }, { 20, 20 }, { 20, 20 }, { 20, 20 } };
    fFinder = new LxTbBinnedFinder(hasTrd ? CUR_NOF_TRD_LAYERS : 0, CUR_NOF_STATIONS, CUR_NOF_TIMEBINS, stSpatLimits, 20, 20, CUR_TIMEBIN_LENGTH);
    fFinder->SetSignalParticle(fSignalParticle);
    hasTrd = fFinder->fSignalParticle->fHasTrd;
    
    HandleGeometry();    
    fFinder->Init();

    CbmMCDataManager* mcManager = static_cast<CbmMCDataManager*> (ioman->GetObject("MCDataManager"));
    fMuchMCPoints = mcManager->InitBranch("MuchPoint");
    fMuchPixelHits = static_cast<TClonesArray*> (ioman->GetObject("MuchPixelHit"));
    fMuchClusters = static_cast<TClonesArray*> (ioman->GetObject("MuchCluster"));
    fMuchPixelDigiMatches = static_cast<TClonesArray*> (ioman->GetObject("MuchDigiMatch"));
    
    fTrdMCPoints = mcManager->InitBranch("TrdPoint");
    fTrdHits = static_cast<TClonesArray*> (ioman->GetObject("TrdHit"));
    fTrdClusters = static_cast<TClonesArray*> (ioman->GetObject("TrdCluster"));
    fTrdDigiMatches = static_cast<TClonesArray*> (ioman->GetObject("TrdDigiMatch"));
    
    CbmMCDataArray* mcTracks = mcManager->InitBranch("MCTrack");
    
#ifdef LXTB_QA
    for (int i = 0; i < 1000; ++i)
    {
        Int_t evSize = mcTracks->Size(0, i);
        fMCTracks.push_back(vector<TrackDataHolder>());
        
        if (0 >= evSize)
            continue;
        
        vector<TrackDataHolder>& evTracks = fMCTracks.back();
        
        for (int j = 0; j < evSize; ++j)
        {
            evTracks.push_back(TrackDataHolder());
            const CbmMCTrack* mcTrack = static_cast<const CbmMCTrack*> (mcTracks->Get(0, i, j));

            if (mcTrack->GetPdgCode() == 13 || mcTrack->GetPdgCode() == -13)
            {
                Int_t motherId = mcTrack->GetMotherId();

                if (motherId >= 0)
                {
                    const CbmMCTrack* motherTrack = static_cast<const CbmMCTrack*> (mcTracks->Get(0, i, motherId));

                    if (fFinder->fSignalParticle->fPdgCode == motherTrack->GetPdgCode())
                    {
                        evTracks.back().isSignalShort = true;
                        evTracks.back().isSignalLong = true;
                        evTracks.back().isPos = mcTrack->GetPdgCode() == -13;
                    }
                }
            }
        }// for (int j = 0; j < evSize; ++j)
    }// for (int i = 0; i < 1000; ++i)
    
    TH2F* trdHisto = new TH2F("TRD", "TRD", 500, -50., 1000., 500, 0., 600.);
    TH2F* trdHistoXY = new TH2F("TRD_XY", "TRD_XY", 400, -400., 400., 300, -300., 300.);
    
    //Int_t numEvents = 0;
    Int_t numPoints[] = { 0, 0, 0, 0, 0, 0 };
    
    static unsigned long long tsStartTime = 50;
    
    for (int i = 0; i < 1000; ++i)
    {
        Int_t evSize = fMuchMCPoints->Size(0, i);
        fMuchPoints.push_back(vector<PointDataHolder>());
        
        if (0 >= evSize)
        {
           tsStartTime += 100;
            continue;
        }
        
        //++numEvents;
        vector<PointDataHolder>& evPoints = fMuchPoints.back();
        
        for (int j = 0; j < evSize; ++j)
        {
            const CbmMuchPoint* pMuchPt = static_cast<const CbmMuchPoint*> (fMuchMCPoints->Get(0, i, j));
            
            Int_t layerNumber = CbmMuchGeoScheme::GetLayerIndex(pMuchPt->GetDetectorId());
            
            //if (1 == layerNumber)
            {                
                PointDataHolder muchPt;
                muchPt.x = (pMuchPt->GetXIn() + pMuchPt->GetXOut()) / 2;
                muchPt.y = (pMuchPt->GetYIn() + pMuchPt->GetYOut()) / 2;
                muchPt.t = tsStartTime + pMuchPt->GetTime();
                muchPt.eventId = i;
                muchPt.trackId = pMuchPt->GetTrackID();
                muchPt.pointId = j;
                muchPt.stationNumber = CbmMuchGeoScheme::GetStationIndex(pMuchPt->GetDetectorId());
                muchPt.layerNumber = layerNumber;
                evPoints.push_back(muchPt);
            }
        }
        
        tsStartTime += 100;
    }
    
    if (hasTrd)
    {
      tsStartTime = 50;
       
      for (int i = 0; i < 1000; ++i)
      {
         Int_t evSize = fTrdMCPoints->Size(0, i);
         fTrdPoints.push_back(vector<PointDataHolder>());

         if (0 >= evSize)
         {
            tsStartTime += 100;
            continue;
         }

         set<Int_t> trdTracks;
         vector<PointDataHolder>& evPoints = fTrdPoints.back();

         for (int j = 0; j < evSize; ++j)
         {
            const CbmTrdPoint* pTrdPt = static_cast<const CbmTrdPoint*> (fTrdMCPoints->Get(0, i, j));
            Int_t trackId = pTrdPt->GetTrackID();
            trdTracks.insert(trackId);

            Int_t layerNumber = CbmTrdAddress::GetLayerId(pTrdPt->GetModuleAddress());

            //if (0 != layerNumber)
            //continue;

            PointDataHolder trdPt;
            trdPt.x = (pTrdPt->GetXIn() + pTrdPt->GetXOut()) / 2;
            trdPt.y = (pTrdPt->GetYIn() + pTrdPt->GetYOut()) / 2;
            trdPt.z = (pTrdPt->GetZIn() + pTrdPt->GetZOut()) / 2;
            trdPt.t = tsStartTime + pTrdPt->GetTime();
            trdPt.eventId = i;
            trdPt.trackId = pTrdPt->GetTrackID();
            trdPt.pointId = j;
            trdPt.stationNumber = layerNumber;
            evPoints.push_back(trdPt);
            
            trdHistoXY->Fill(trdPt.x, trdPt.y);
         }
         
         for (set<Int_t>::const_iterator j = trdTracks.begin(); j != trdTracks.end(); ++j)
         {
            Int_t trackId = *j;
            const CbmMCTrack* mcTrack = static_cast<const CbmMCTrack*> (mcTracks->Get(0, i, trackId));
            Double_t startZ = mcTrack->GetStartZ();
            Double_t startR = sqrt(mcTrack->GetStartX() * mcTrack->GetStartX() + mcTrack->GetStartY() * mcTrack->GetStartY());
            trdHisto->Fill(startZ, startR);
         }
         
         tsStartTime += 100;
      }
    }

    for (vector<vector<PointDataHolder> >::iterator i = fMuchPoints.begin(); i != fMuchPoints.end(); ++i)
    {
        vector<PointDataHolder>& evPoints = *i;
       
         for (vector<PointDataHolder>::iterator j = evPoints.begin(); j != evPoints.end(); ++j)
         {
            PointDataHolder& pt = *j;
            
            if (1 == pt.layerNumber)
               fMCTracks[pt.eventId][pt.trackId].pointInds[pt.stationNumber] = pt.pointId;
         }
    }
    
    if (hasTrd)
    {
      for (vector<vector<PointDataHolder> >::iterator i = fTrdPoints.begin(); i != fTrdPoints.end(); ++i)
      {
         vector<PointDataHolder>& evPoints = *i;
       
         for (vector<PointDataHolder>::iterator j = evPoints.begin(); j != evPoints.end(); ++j)
         {
            PointDataHolder& pt = *j;    
            fMCTracks[pt.eventId][pt.trackId].trdPointInds[pt.stationNumber] = pt.pointId;
         }
      }
    }
    
    TH1F* trd2XHisto = new TH1F("trd2XHisto", "trd2XHisto", 100, -2.5, 2.5);
    TH1F* trd2YHisto = new TH1F("trd2YHisto", "trd2YHisto", 100, -2.5, 2.5);
    TH1F* trd3XHisto = new TH1F("trd3XHisto", "trd3XHisto", 100, -2.5, 2.5);
    TH1F* trd3YHisto = new TH1F("trd3YHisto", "trd3YHisto", 100, -2.5, 2.5);
    int eventId = 0;

    for(vector<vector<TrackDataHolder> >::iterator i = fMCTracks.begin(); i != fMCTracks.end(); ++i)
    {
        vector<TrackDataHolder>& evTracks = *i;
        
        for (vector<TrackDataHolder>::iterator j = evTracks.begin(); j != evTracks.end(); ++j)
        {
            TrackDataHolder& track = *j;
            
            if (!track.isSignalShort)
                continue;
            
            for (int k = 0; k < CUR_NOF_STATIONS; ++k)
            {
                if (track.pointInds[k] < 0)
                {
                    track.isSignalShort = false;
                    track.isSignalLong = false;
                    break;
                }
            }
            
            if (hasTrd)
            {
               for (int k = 0; k < CUR_NOF_TRD_LAYERS; ++k)
               {
                  if (track.trdPointInds[k] < 0)
                  {
                     track.isSignalLong = false;
                     break;
                  }
               }
            
               if (!track.isSignalLong)
                  continue;

               PointDataHolder& trdPt0 = fTrdPoints[eventId][track.trdPointInds[0]];
               PointDataHolder& trdPt1 = fTrdPoints[eventId][track.trdPointInds[1]];
               PointDataHolder& trdPt2 = fTrdPoints[eventId][track.trdPointInds[2]];
               PointDataHolder& trdPt3 = fTrdPoints[eventId][track.trdPointInds[3]];

               Double_t tx = (trdPt1.x - trdPt0.x) / (trdPt1.z - trdPt0.z);
               Double_t ty = (trdPt1.y - trdPt0.y) / (trdPt1.z - trdPt0.z);
               Double_t p2X = trdPt1.x + tx * (trdPt2.z - trdPt1.z);
               trd2XHisto->Fill(p2X - trdPt2.x);
               Double_t p2Y = trdPt1.y + ty * (trdPt2.z - trdPt1.z);
               trd2YHisto->Fill(p2Y - trdPt2.y);
               Double_t p3X = trdPt1.x + tx * (trdPt3.z - trdPt1.z);
               trd3XHisto->Fill(p3X - trdPt3.x);
               Double_t p3Y = trdPt1.y + ty * (trdPt3.z - trdPt1.z);
               trd3YHisto->Fill(p3Y - trdPt3.y);
            }
        }
        
        ++eventId;
    }
    
    if (hasTrd)
    {
      {
         TFile* curFile = TFile::CurrentFile();
         TString histoName = "trd2XHisto.root";

         TFile fh(histoName.Data(), "RECREATE");
         trd2XHisto->Write();
         fh.Close();
         delete trd2XHisto;

         TFile::CurrentFile() = curFile;
      }

      {
         TFile* curFile = TFile::CurrentFile();
         TString histoName = "trd2YHisto.root";

         TFile fh(histoName.Data(), "RECREATE");
         trd2YHisto->Write();
         fh.Close();
         delete trd2YHisto;

         TFile::CurrentFile() = curFile;
      }

      {
         TFile* curFile = TFile::CurrentFile();
         TString histoName = "trd3XHisto.root";

         TFile fh(histoName.Data(), "RECREATE");
         trd3XHisto->Write();
         fh.Close();
         delete trd3XHisto;

         TFile::CurrentFile() = curFile;
      }

      {
         TFile* curFile = TFile::CurrentFile();
         TString histoName = "trd3YHisto.root";

         TFile fh(histoName.Data(), "RECREATE");
         trd3YHisto->Write();
         fh.Close();
         delete trd3YHisto;

         TFile::CurrentFile() = curFile;
      }
    }
    
   int evNum = 0;
   TH1F* signalDistHisto = new TH1F("signalDistHisto", "signalDistHisto", 200, 0., 200.);
   
   for(vector<vector<TrackDataHolder> >::iterator i = fMCTracks.begin(); i != fMCTracks.end(); ++i)
   {
      vector<TrackDataHolder>& evTracks = *i;
      bool hasShortPos = false;
      bool hasShortNeg = false;
      bool hasLongPos = false;
      bool hasLongNeg = false;
      scaltype posTime = 0;
      scaltype negTime = 0;
      Double_t posX = 0;
      Double_t posY = 0;
      Double_t negX = 0;
      Double_t negY = 0;
        
      for (vector<TrackDataHolder>::iterator j = evTracks.begin(); j != evTracks.end(); ++j)
      {
         TrackDataHolder& track = *j;
            
         if (!track.isSignalShort)
             continue;
         
         if (track.isPos)
         {
            hasShortPos = true;
            posTime = fMuchPoints[evNum][track.pointInds[CUR_LAST_STATION]].t;
            posX = fMuchPoints[evNum][track.pointInds[0]].x;
            posY = fMuchPoints[evNum][track.pointInds[0]].y;
            
            if (track.isSignalLong)
               hasLongPos = true;
         }
         else
         {
            hasShortNeg = true;
            negTime = fMuchPoints[evNum][track.pointInds[CUR_LAST_STATION]].t;
            negX = fMuchPoints[evNum][track.pointInds[0]].x;
            negY = fMuchPoints[evNum][track.pointInds[0]].y;
            
            if (track.isSignalLong)
               hasLongNeg = true;
         }
      }
      
      if (hasShortPos && hasShortNeg)
      {
         shortSignalMCTimes.push_back((posTime + negTime) / 2);
         
         if (hasLongPos && hasLongNeg)
         {
            longSignalMCTimes.push_back((posTime + negTime) / 2);
            signalDistHisto->Fill(sqrt((posX - negX) * (posX - negX) + (posY - negY) * (posY - negY)));
         }
      }
      
      ++evNum;
   }
   
   {
      TFile* curFile = TFile::CurrentFile();
      TString histoName = "signalDistHisto.root";

      TFile fh(histoName.Data(), "RECREATE");
      signalDistHisto->Write();
      fh.Close();
      delete signalDistHisto;

      TFile::CurrentFile() = curFile;
   }
   
   if (hasTrd)
   {
      {
         TFile* curFile = TFile::CurrentFile();
         TString histoName = "trdMctByZR_All.root";

         TFile fh(histoName.Data(), "RECREATE");
         trdHisto->Write();
         fh.Close();
         delete trdHisto;

         TFile::CurrentFile() = curFile;
      }
      
      {
         TFile* curFile = TFile::CurrentFile();
         TString histoName = "trdMctByXY_All.root";

         TFile fh(histoName.Data(), "RECREATE");
         trdHistoXY->Write();
         fh.Close();
         delete trdHistoXY;

         TFile::CurrentFile() = curFile;
      }
    }
#endif//LXTB_QA

    return kSUCCESS;
}

static Int_t currentEventN = 0;
static unsigned long long tsStartTime = 0;

#ifdef LXTB_QA
void LxTBFinder::AddHit(const CbmPixelHit* hit, Int_t stationNumber, bool isTrd)
#else
void LxTBFinder::AddHit(const CbmPixelHit* hit, Int_t stationNumber)
#endif//LXTB_QA
{
   scaltype x = hit->GetX();
   scaltype y = hit->GetY();
   scaltype t = tsStartTime + 10;
#ifdef LXTB_QA        
   //point.eventId = pt.eventId;
   //point.trackId = pt.trackId;
   //fMCTracks[point.eventId][point.trackId].hasPoint[stationNumber] = true;
   //point.pHit = hit;
   //point.isTrd = isTrd;
   Int_t clusterId = hit->GetRefId();
#ifdef LXTB_QA_IDEAL
   const FairMCPoint* pMCPt = static_cast<const FairMCPoint*> (isTrd ? fTrdMCPoints->Get(0, currentEventN, clusterId) : fMuchMCPoints->Get(0, currentEventN, clusterId));
   Int_t trackId = pMCPt->GetTrackID();
   LxTbBinnedPoint::PointDesc ptDesc = { currentEventN, clusterId, trackId };
   t = isTrd ? fTrdPoints[currentEventN][clusterId].t : fMuchPoints[currentEventN][clusterId].t;
#endif//LXTB_QA_IDEAL
#endif//LXTB_QA
   scaltype dx = hit->GetDx();
   scaltype dy = hit->GetDy();
   scaltype dt = 4;
   
   scaltype minY = (isTrd ? fFinder->trdStation.minY : fFinder->stations[stationNumber].minY);
   scaltype binSizeY = (isTrd ? fFinder->trdStation.binSizeY : fFinder->stations[stationNumber].binSizeY);
   scaltype lastYBin = (isTrd ? fFinder->trdStation.lastYBin : fFinder->stations[stationNumber].lastYBin);
   scaltype minX = (isTrd ? fFinder->trdStation.minX : fFinder->stations[stationNumber].minX);
   scaltype binSizeX = (isTrd ? fFinder->trdStation.binSizeX : fFinder->stations[stationNumber].binSizeX);
   scaltype lastXBin = (isTrd ? fFinder->trdStation.lastXBin : fFinder->stations[stationNumber].lastXBin);
   
   int tInd = (t - fFinder->minT) / CUR_TIMEBIN_LENGTH;

   if (tInd < 0)
      tInd = 0;
   else if (tInd > CUR_LAST_TIMEBIN)
      tInd = CUR_LAST_TIMEBIN;

   LxTbTYXBin& tyxBin = (isTrd ? fFinder->trdStation.tyxBinsArr[stationNumber][tInd] : fFinder->stations[stationNumber].tyxBins[tInd]);
   int yInd = (y - minY) / binSizeY;

   if (yInd < 0)
      yInd = 0;
   else if (yInd > lastYBin)
      yInd = lastYBin;

   LxTbYXBin& yxBin = tyxBin.yxBins[yInd];
   int xInd = (x - minX) / binSizeX;

   if (xInd < 0)
      xInd = 0;
   else if (xInd > lastXBin)
      xInd = lastXBin;

   LxTbXBin& xBin = yxBin.xBins[xInd];
   //LxTbBinnedPoint point(x, dx, y, dy, t, dt, CUR_LAST_STATION == stationNumber);
   //LxTbBinnedPoint point(x, dx, y, dy, t, dt, false);
   LxTbBinnedPoint point(x, dx, y, dy, t, dt, /*!hasTrd && */CUR_LAST_STATION == stationNumber);

#ifdef LXTB_QA        
   //point.eventId = pt.eventId;
   //point.trackId = pt.trackId;
   //fMCTracks[point.eventId][point.trackId].hasPoint[stationNumber] = true;
   //point.pHit = hit;
   //point.isTrd = isTrd;
#ifdef LXTB_QA_IDEAL
   point.mcRefs.push_back(ptDesc);
#else//LXTB_QA_IDEAL
   const CbmCluster* cluster = static_cast<const CbmCluster*> (isTrd ? fTrdClusters->At(clusterId) : fMuchClusters->At(clusterId));
   Int_t nDigis = cluster->GetNofDigis();

   for (Int_t i = 0; i < nDigis; ++i)
   {
      const CbmMatch* digiMatch = static_cast<const CbmMatch*> (isTrd ? fTrdDigiMatches->At(cluster->GetDigi(i)) : fMuchPixelDigiMatches->At(cluster->GetDigi(i)));
      Int_t nMCs = digiMatch->GetNofLinks();

      for (Int_t j = 0; j < nMCs; ++j)
      {
         const CbmLink& lnk = digiMatch->GetLink(j);
         Int_t eventId = isEvByEv ? currentEventN : lnk.GetEntry();
         Int_t pointId = lnk.GetIndex();
         const FairMCPoint* pMCPt = static_cast<const FairMCPoint*> (isTrd ? fTrdMCPoints->Get(0, eventId, pointId) : fMuchMCPoints->Get(0, eventId, pointId));
         Int_t trackId = pMCPt->GetTrackID();
         LxTbBinnedPoint::PointDesc ptDesc = { eventId, pointId, trackId };
         point.mcRefs.push_back(ptDesc);
      }
   }
#endif//LXTB_QA_IDEAL
#endif//LXTB_QA
   
   xBin.points.push_back(point);

   if (/*!hasTrd && */CUR_LAST_STATION == stationNumber)
   {
      xBin.use = true;
      yxBin.use = true;
      tyxBin.use = true;
   }
}

static void SpliceTriggerings(list<pair<scaltype, scaltype> >& out, LxTbBinnedFinder::TriggerTimeArray& in)
{
   for (int i = 0; i < in.nofTimebins; ++i)
      out.splice(out.end(), in.triggerTimeBins[i]);
}

void LxTBFinder::Exec(Option_t* opt)
{
   fFinder->Clear();
   fFinder->SetTSBegin(tsStartTime);
   
   // As the SIS100 geometry contains 4 MUCH and 1 TRD stations we need to read both MUCH and TRD hits.
   for (int i = 0; i < fMuchPixelHits->GetEntriesFast(); ++i)
   {
      const CbmMuchPixelHit* mh = static_cast<const CbmMuchPixelHit*> (fMuchPixelHits->At(i));
      Int_t hitLrN = CbmMuchGeoScheme::GetLayerIndex(mh->GetAddress());
      
      if (1 != hitLrN)
         continue;
      
      Int_t hitStN = CbmMuchGeoScheme::GetStationIndex(mh->GetAddress());
#ifdef LXTB_QA
      AddHit(mh, hitStN, false);
#else
      AddHit(mh, hitStN);
#endif//LXTB_QA
   }
   
   if (hasTrd)
   {
      for (int i = 0; i < fTrdHits->GetEntriesFast(); ++i)
      {
         const CbmTrdHit* th = static_cast<const CbmTrdHit*> (fTrdHits->At(i));
         Int_t hitStN = th->GetPlaneId();
      
         //if (0 != hitStN)
            //continue;
      
         //hitStN = CUR_LAST_STATION;
#ifdef LXTB_QA
         AddHit(th, hitStN, true);
#else
         AddHit(th, hitStN);
#endif//LXTB_QA
      }
   }
   
   fFinder->Reconstruct();
   tsStartTime += 100;
   
   for (int i = 0; i < fFinder->nofTrackBins; ++i)
   {
      list<LxTbBinnedFinder::Chain*>& recoTracksBin = fFinder->recoTracks[i];
      
      for (list<LxTbBinnedFinder::Chain*>::const_iterator j = recoTracksBin.begin(); j != recoTracksBin.end(); ++j)
         recoTracks.push_back(*j);
   }
   
   SpliceTriggerings(triggerTimes_trd0_sign0_dist0, fFinder->triggerTimes_trd0_sign0_dist0);
   SpliceTriggerings(triggerTimes_trd0_sign0_dist1, fFinder->triggerTimes_trd0_sign0_dist1);
   SpliceTriggerings(triggerTimes_trd0_sign1_dist0, fFinder->triggerTimes_trd0_sign1_dist0);
   SpliceTriggerings(triggerTimes_trd0_sign1_dist1, fFinder->triggerTimes_trd0_sign1_dist1);
   SpliceTriggerings(triggerTimes_trd1_sign0_dist0, fFinder->triggerTimes_trd1_sign0_dist0);
   SpliceTriggerings(triggerTimes_trd1_sign0_dist1, fFinder->triggerTimes_trd1_sign0_dist1);
   SpliceTriggerings(triggerTimes_trd1_sign1_dist0, fFinder->triggerTimes_trd1_sign1_dist0);
   SpliceTriggerings(triggerTimes_trd1_sign1_dist1, fFinder->triggerTimes_trd1_sign1_dist1);
   
   ++currentEventN;
}

struct RecoTrackData
{
    Int_t eventId;
    Int_t trackId;
    
    RecoTrackData(Int_t eId, Int_t tId) : eventId(eId), trackId(tId) {}
};

struct RTDLess
{
    bool operator() (const RecoTrackData& x, const RecoTrackData& y) const
    {
        if (x.eventId < y.eventId)
            return true;

        return x.trackId < y.trackId;
    }
};

static void PrintTrigger(list<pair<scaltype, scaltype> >& signalRecoTimes, list<scaltype>& signalMCTimes, const char* name)
{
   int nofRecoSignals = 0;
   
   for (list<scaltype>::const_iterator i = signalMCTimes.begin(); i != signalMCTimes.end(); ++i)
   {
      scaltype mcTime = *i;
      bool matched = false;
      
      for (list<pair<scaltype, scaltype> >::const_iterator j = signalRecoTimes.begin(); j != signalRecoTimes.end(); ++j)
      {
         scaltype recoTime = j->first;
         scaltype dt = j->second;
         
         if (fabs(recoTime - mcTime) < NOF_SIGMAS * dt)
            matched = true;
      }
      
      if (matched)
         ++nofRecoSignals;
   }
   
   cout << "Have: " << signalRecoTimes.size() << " signaled " << name << " events" << endl;
   double eff = 0 == signalMCTimes.size() ? 100 : 100.0 * nofRecoSignals / signalMCTimes.size();
   cout << "Triggered signals(" << name << "): " << eff << "% [ " << nofRecoSignals << " / " << signalMCTimes.size() << " ]" << endl;
   
   char buf[256];
   sprintf(buf, "triggerings_%s.txt", name);
   ofstream triggeringsFile(buf, ios_base::out | ios_base::trunc);
   triggeringsFile << signalRecoTimes.size();
   
   sprintf(buf, "signal_triggerings_%s.txt", name);
   ofstream signalTriggeringsFile(buf, ios_base::out | ios_base::trunc);
   signalTriggeringsFile << nofRecoSignals;
}

void LxTBFinder::Finish()
{
   cout << "LxTbBinnedFinder::Reconstruct() full duration was: " << fullDuration << endl;
   
   int nofRecoTracks = 0;
   
   for (list<LxTbBinnedFinder::Chain*>::const_iterator i = recoTracks.begin(); i != recoTracks.end(); ++i)
   {
      const LxTbBinnedFinder::Chain* chain = *i;
      
      if (!hasTrd || chain->highMom)
         ++nofRecoTracks;
   }
   
   cout << "LxTbBinnedFinder::Reconstruct() the number of found tracks: " << nofRecoTracks << endl;
   
#ifdef LXTB_QA
   static int nofSignalTracks = 0;
   static int nofRecoSignalTracks = 0;
   int eventN = 0;
   
   for(vector<vector<TrackDataHolder> >::iterator i = fMCTracks.begin(); i != fMCTracks.end(); ++i)
   {
      vector<TrackDataHolder>& evTracks = *i;
        
      for (vector<TrackDataHolder>::iterator j = evTracks.begin(); j != evTracks.end(); ++j)
      {
         TrackDataHolder& track = *j;
            
         if (!track.isSignalShort || (hasTrd && !track.isSignalLong))
             continue;
         
         ++nofSignalTracks;
         
         int nofMatchPoints = 0;
            
         for (list<LxTbBinnedFinder::Chain*>::const_iterator k = recoTracks.begin(); k != recoTracks.end(); ++k)
         {
            const LxTbBinnedFinder::Chain* chain = *k;
            
            if (hasTrd && !chain->highMom)
               continue;
               
            for (int l = 0; l < CUR_NOF_STATIONS; ++l)
            {
               bool pointsMatched = false;
               
               for (list<LxTbBinnedPoint::PointDesc>::const_iterator m = chain->points[l]->mcRefs.begin(); m != chain->points[l]->mcRefs.end(); ++m)
               {
                  if (m->eventId == eventN && m->pointId == track.pointInds[l])
                  {
                     pointsMatched = true;
                     break;
                  }
               }
               
               if (pointsMatched)
                  ++nofMatchPoints;
            }
         }
         
         if (nofMatchPoints >= 2)
         {
            ++nofRecoSignalTracks;
            continue;
         }
      }
      
      ++eventN;
   }
   
   double eff = 0 == nofSignalTracks ? 100 : 100.0 * nofRecoSignalTracks / nofSignalTracks;
   cout << "Reconstruction efficiency is: " << eff << "% [ " << nofRecoSignalTracks << " / " << nofSignalTracks << " ]" << endl;
   
   int nofRightRecoTracks = 0;
   
   for (list<LxTbBinnedFinder::Chain*>::const_iterator i = recoTracks.begin(); i != recoTracks.end(); ++i)
   {
      const LxTbBinnedFinder::Chain* chain = *i;
      
      if (hasTrd && !chain->highMom)
         continue;
      
      map<RecoTrackData, int, RTDLess> nofTracks;
      
      for (int j = 0; j < CUR_NOF_STATIONS; ++j)
      {
         int stMask = 1 << j;
         
         for (list<LxTbBinnedPoint::PointDesc>::const_iterator k = chain->points[j]->mcRefs.begin(); k != chain->points[j]->mcRefs.end(); ++k)
         {
            RecoTrackData st(k->eventId, k->trackId);
            map<RecoTrackData, int, RTDLess>::iterator nofTIter = nofTracks.find(st);

            if (nofTIter != nofTracks.end())
               nofTIter->second |= stMask;
            else
               nofTracks[st] = stMask;
         }
      }
        
      int nofPoints = 0;
        
      for (map<RecoTrackData, int, RTDLess>::const_iterator j = nofTracks.begin(); j != nofTracks.end(); ++j)
      {
         int nofp = 0;
         
         for (int k = 0; k < CUR_NOF_STATIONS; ++k)
         {
            if (j->second & (1 << k))
               ++nofp;
         }
         
         if (nofp > nofPoints)
            nofPoints = nofp;
      }
        
      if (nofPoints >= 2)
         ++nofRightRecoTracks;
   }
   
   eff = 0 == recoTracks.size() ? 100 : 100.0 * nofRightRecoTracks / nofRecoTracks;
   cout << "Non ghosts are: " << eff << "% [ " << nofRightRecoTracks << " / " << nofRecoTracks << " ]" << endl;
   
   cout << "Have: " << shortSignalMCTimes.size() << " short signaling events" << endl;
   cout << "Have: " << longSignalMCTimes.size() << " long signaling events" << endl;
   cout << "Have: " << currentEventN << " events" << endl;
   
   ofstream nofEventsFile("nof_events.txt", ios_base::out | ios_base::trunc);
   nofEventsFile << currentEventN;
   
   ofstream nofShortSignalsFile("nof_short_signals.txt", ios_base::out | ios_base::trunc);
   nofShortSignalsFile << shortSignalMCTimes.size();
   
   ofstream nofLongSignalsFile("nof_long_signals.txt", ios_base::out | ios_base::trunc);
   nofLongSignalsFile << longSignalMCTimes.size();
   
   PrintTrigger(triggerTimes_trd0_sign0_dist0, shortSignalMCTimes, "triggerTimes_trd0_sign0_dist0");
   PrintTrigger(triggerTimes_trd0_sign0_dist1, shortSignalMCTimes, "triggerTimes_trd0_sign0_dist1");
   PrintTrigger(triggerTimes_trd0_sign1_dist0, shortSignalMCTimes, "triggerTimes_trd0_sign1_dist0");
   PrintTrigger(triggerTimes_trd0_sign1_dist1, shortSignalMCTimes, "triggerTimes_trd0_sign1_dist1");
   PrintTrigger(triggerTimes_trd1_sign0_dist0, longSignalMCTimes, "triggerTimes_trd1_sign0_dist0");
   PrintTrigger(triggerTimes_trd1_sign0_dist1, longSignalMCTimes, "triggerTimes_trd1_sign0_dist1");
   PrintTrigger(triggerTimes_trd1_sign1_dist0, longSignalMCTimes, "triggerTimes_trd1_sign1_dist0");
   PrintTrigger(triggerTimes_trd1_sign1_dist1, longSignalMCTimes, "triggerTimes_trd1_sign1_dist1");
#endif//LXTB_QA
   
   for (list<LxTbBinnedFinder::Chain*>::iterator i = recoTracks.begin(); i != recoTracks.end(); ++i)
      delete *i;
}