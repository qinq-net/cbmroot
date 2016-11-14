/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "LxTBMLTask.h"
#include "TGeoCompositeShape.h"
#include "TGeoArb8.h"
#include "TGeoBoolNode.h"
#include "TGeoManager.h"
#include "LxTBMatEffs.h"
#include "FairLogger.h"
#include "TClonesArray.h"
#include "CbmMCDataManager.h"
#include "CbmMuchGeoScheme.h"
#include "CbmMuchPixelHit.h"
#include "CbmCluster.h"
#include "CbmMatch.h"
#include "CbmMuchPoint.h"
#include "TRandom3.h"
#include "TMath.h"
#include "TDatabasePDG.h"
#include "CbmMCTrack.h"
#include "TH1F.h"
#include "Simple/LxSettings.h"

using namespace std;

ClassImp(LxTBMLFinder)

Double_t speedOfLight = 0;

struct LxTbMLStation
{
   struct Q
   {
      scaltype Q11, Q12, Q21, Q22;
   };
    
   LxTbLayer* fLayers;
   LxTbAbsorber fAbsorber;
   scaltype fDeltaThetaX;
   scaltype fThetaX;
   scaltype fScatXRL;
   scaltype fScatXLS;
   scaltype fDeltaThetaY;
   scaltype fThetaY;
   scaltype fScatYRL;
   scaltype fScatYLS;
   Q qs[2];
    
   LxTbMLStation(int nofxb, int nofyb, int noftb) : fLayers(reinterpret_cast<LxTbLayer*> (new unsigned char[NOF_LAYERS * sizeof(LxTbLayer)])),
      fDeltaThetaX(0), fThetaX(0), fScatXRL(0), fScatXLS(0), fDeltaThetaY(0), fThetaY(0), fScatYRL(0), fScatYLS(0),
      fHandleMPoint(*this), fHandleRPoint(*this), fHandleLPoint(*this)
   {
      for (int i = 0; i < NOF_LAYERS; ++i)
         new (&fLayers[i]) LxTbLayer(nofxb, nofyb, noftb);
   }
   
   void Init()
   {
      for (int i = 0; i < NOF_LAYERS; ++i)
      {
         //fLayers[i].xBinLength = (fLayers[i].maxX - fLayers[i].minX) / fLayers[i].nofXBins;
         //fLayers[i].yBinLength = (fLayers[i].maxY - fLayers[i].minY) / fLayers[i].nofYXBins;
         fLayers[i].Init();
      }
      
      fHandleMPoint.Init();
      fHandleRPoint.Init();
      fHandleLPoint.Init();
   }
   
   void Clear()
   {
      for (int i = 0; i < NOF_LAYERS; ++i)
         fLayers[i].Clear();
   }
    
   void SetMinT(timetype v)
   {
      for (int i = 0; i < NOF_LAYERS; ++i)
        fLayers[i].SetMinT(v);
   }
   
   struct HandleMPoint
   {
      LxTbMLStation& station;
      timetype c;
      scaltype deltaZr;
      
      explicit HandleMPoint(LxTbMLStation& parent) : station(parent), c(0), deltaZr(0) {}
      
      void Init()
      {
         c = speedOfLight;
         deltaZr = station.fLayers[2].z - station.fLayers[1].z;
      }
      
      void operator()(LxTbBinnedPoint& point)
      {         
         scaltype txR = point.x / station.fLayers[1].z;
         scaltype tyR = point.y / station.fLayers[1].z;
         scaltype pXr = point.x + txR * deltaZr;
         scaltype pYr = point.y + tyR * deltaZr;
         scaltype trajLenR = sqrt(1 + txR * txR + tyR * tyR) * deltaZr;
         timetype pTr = point.t + 1.e9 * trajLenR / c; // 1.e9 to convert to ns.
         station.fHandleRPoint.mPoint = &point;
         station.fHandleLPoint.mPoint = &point;
         IterateNeighbourhood(station.fLayers[2], pXr, point.dx, station.fScatXRL, pYr, point.dy, station.fScatYRL, pTr, point.dt, station.fHandleRPoint);
      }
   };
   
   HandleMPoint fHandleMPoint;
   
   struct HandleRPoint
   {
      LxTbMLStation& station;
      timetype c;
      scaltype deltaZl;
      LxTbBinnedPoint* mPoint;
      
      explicit HandleRPoint(LxTbMLStation& parent) : station(parent), c(0), deltaZl(0), mPoint(0) {}
      
      void Init()
      {
         c = speedOfLight;
         deltaZl = station.fLayers[0].z - station.fLayers[1].z;
      }
      
      void operator()(LxTbBinnedPoint& point)
      {         
         scaltype txL = (point.x - mPoint->x) / (station.fLayers[2].z - station.fLayers[1].z);
         scaltype tyL = (point.y - mPoint->y) / (station.fLayers[2].z - station.fLayers[1].z);
         scaltype pXl = mPoint->x + txL * deltaZl;
         scaltype pYl = mPoint->y + tyL * deltaZl;
         scaltype trajLenL = sqrt(1 + txL * txL + tyL * tyL) * deltaZl;
         timetype pTl = mPoint->t + 1.e9 * trajLenL / c; // 1.e9 to convert to ns and trajLenL is negative.
         station.fHandleLPoint.rPoint = &point;
         IterateNeighbourhood(station.fLayers[0], pXl, sqrt(mPoint->dx * mPoint->dx + point.dx * point.dx), 0,
            pYl, sqrt(mPoint->dy * mPoint->dy + point.dy * point.dy), 0,
            pTl, mPoint->dt/*sqrt(mPoint->dt * mPoint->dt + point.dt * point.dt) / 2*/,
            station.fHandleLPoint);
      }
   };
   
   HandleRPoint fHandleRPoint;
   
   struct HandleLPoint
   {
      LxTbMLStation& station;
      scaltype deltaZ;
      LxTbBinnedPoint* mPoint;
      LxTbBinnedPoint* rPoint;
      
      explicit HandleLPoint(LxTbMLStation& parent) : station(parent), deltaZ(parent.fLayers[2].z - parent.fLayers[0].z), mPoint(0), rPoint(0) {}
      
      void Init()
      {
         deltaZ = station.fLayers[0].z - station.fLayers[2].z;
      }
      
      void operator()(LxTbBinnedPoint& point)
      {         
         LxTbBinnedTriplet* triplet = new LxTbBinnedTriplet(&point, rPoint, deltaZ);
         mPoint->triplets.push_back(triplet);
      }
   };
   
   HandleLPoint fHandleLPoint;
    
   void Reconstruct()
   {
      IterateLayer<HandleMPoint> (fLayers[1], fHandleMPoint);
   }
};

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

struct SignalParticle
{
   const char* fName;
   Int_t fPdgCode;
   scaltype fMinEnergy;
};

static SignalParticle particleDescs[] = { { "omega", 223, 1.5 }, { "", -1, 0 } };

struct LxTbDetector
{    
   LxTbMLStation* fStations;
   list<LxTBMLFinder::Chain*> recoTracks;
   SignalParticle* fSignalParticle;
   
   LxTbDetector(int nofxb, int nofyb, int noftb) : fStations(reinterpret_cast<LxTbMLStation*> (new unsigned char[NOF_STATIONS * sizeof(LxTbMLStation)])), fSignalParticle(&particleDescs[0]),
      fHandleLastPoint(*this)
   {
      for (int i = 0; i < NOF_STATIONS; ++i)
          new (&fStations[i]) LxTbMLStation(nofxb, nofyb, noftb);
   }
   
   void Init()
   {
      speedOfLight = 100 * TMath::C();// Multiply by 100 to express in centimeters.
      gMuonMass = TDatabasePDG::Instance()->GetParticle(13)->Mass();
      gElectronMass = TDatabasePDG::Instance()->GetParticle(11)->Mass();
      HandleGeometry();
      
      for (int i = 0; i < NOF_STATIONS; ++i)
      {
         LxTbMLStation& station = fStations[i];
         station.Init();
      }
      
      scaltype E = fSignalParticle->fMinEnergy; // GeV
      scaltype E0 = E;
      scaltype totalLength = fStations[0].fAbsorber.zCoord;
      scaltype deltaZs[NOF_STATIONS];
      deltaZs[0] = fStations[0].fLayers[1].z - fStations[0].fAbsorber.zCoord;
         
      for (int i = 1; i < NOF_STATIONS; ++i)
         deltaZs[i] = fStations[i].fLayers[1].z - fStations[i - 1].fLayers[1].z;

      for (int i = 0; i < NOF_STATIONS; ++i)
      {
         LxTbMLStation& station = fStations[i];
         scaltype L = station.fAbsorber.width; // / cos(3.14159265 * 15 / 180);
         E -= EnergyLoss(E, L, &station.fAbsorber);
         scaltype Escat = (E0 + E) / 2;
         //scaltype Escat = E;
         scaltype deltaTheta = CalcThetaPrj(Escat, L, &station.fAbsorber);
         station.fDeltaThetaX = deltaTheta;
         station.fDeltaThetaY = deltaTheta;
         scaltype thetaXSq = 0;
         scaltype thetaYSq = 0;
         
         for (int j = 0; j <= i; ++j)
         {
            totalLength = station.fLayers[1].z;
            
            scaltype deltaZsum = 0;
            
            for (int k = j; k <= i; ++k)
               deltaZsum += deltaZs[k];
            
            thetaXSq += (1 - deltaZsum / totalLength) * (1 - deltaZsum / totalLength) * fStations[j].fDeltaThetaX * fStations[j].fDeltaThetaX;
            thetaYSq += (1 - deltaZsum / totalLength) * (1 - deltaZsum / totalLength) * fStations[j].fDeltaThetaY * fStations[j].fDeltaThetaY;
         }
         
         station.fThetaX = sqrt(thetaXSq);
         station.fThetaY = sqrt(thetaYSq);
         scaltype deltaZRL = station.fLayers[2].z - station.fLayers[1].z;
         station.fScatXRL = station.fThetaX * deltaZRL;
         station.fScatYRL = station.fThetaY * deltaZRL;
         
         if (i > 0)
         {
            scaltype deltaZLS = station.fLayers[1].z - fStations[i - 1].fLayers[1].z;
            station.fScatXLS = station.fDeltaThetaX * deltaZLS;
            station.fScatYLS = station.fDeltaThetaY * deltaZLS;
         }
         
         scaltype q0XSq = station.fDeltaThetaX * station.fDeltaThetaX;
         scaltype q0YSq = station.fDeltaThetaY * station.fDeltaThetaY;
         station.qs[0] = {q0XSq * L * L / 3, q0XSq * L / 2, q0XSq * L / 2, q0XSq};
         station.qs[1] = {q0YSq * L * L / 3, q0YSq * L / 2, q0YSq * L / 2, q0YSq};
         E0 = E;
      }
      
      fHandleRPoint.Init();
   }
   
   void Clear()
   {
      recoTracks.clear();
      
      for (int i = 0; i < NOF_STATIONS; ++i)
         fStations[i].Clear();
   }
   
   void SetMinT(timetype v)
   {
      for (int i = 0; i < NOF_STATIONS; ++i)
         fStations[i].SetMinT(v);
   }
   
   void HandleGeometry()
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
               pNavigator->CdDown(layer);
               gGeoManager->LocalToMaster(localCoords, globalCoords);
               
               fStations[stationNumber].fLayers[layerNumber].z = globalCoords[2];
               fStations[stationNumber].fLayers[layerNumber].minX = 0;
               fStations[stationNumber].fLayers[layerNumber].maxX = 0;
               fStations[stationNumber].fLayers[layerNumber].xBinLength = 0;
               fStations[stationNumber].fLayers[layerNumber].minY = 0;
               fStations[stationNumber].fLayers[layerNumber].maxY = 0;
               fStations[stationNumber].fLayers[layerNumber].yBinLength = 0;

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

                        if (fStations[stationNumber].fLayers[layerNumber].minY > globalActiveCoords[1])
                           fStations[stationNumber].fLayers[layerNumber].minY = globalActiveCoords[1];

                        if (fStations[stationNumber].fLayers[layerNumber].maxY < globalActiveCoords[1])
                           fStations[stationNumber].fLayers[layerNumber].maxY = globalActiveCoords[1];

                        if (fStations[stationNumber].fLayers[layerNumber].minX > globalActiveCoords[0])
                           fStations[stationNumber].fLayers[layerNumber].minX = globalActiveCoords[0];

                        if (fStations[stationNumber].fLayers[layerNumber].maxX < globalActiveCoords[0])
                           fStations[stationNumber].fLayers[layerNumber].maxX = globalActiveCoords[0];
                     }
                  }

                  pNavigator->CdUp();
               }

               pNavigator->CdUp();
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
               gGeoManager->LocalToMaster(localCoords, globalCoords);
               fStations[absorberNumber].fAbsorber.zCoord = globalCoords[2] - absShape->GetDZ();
               fStations[absorberNumber].fAbsorber.width = 2 * absShape->GetDZ();
               fStations[absorberNumber].fAbsorber.radLength = absVol->GetMaterial()->GetRadLen();
               fStations[absorberNumber].fAbsorber.rho = absVol->GetMaterial()->GetDensity();
               fStations[absorberNumber].fAbsorber.Z = absVol->GetMaterial()->GetZ();
               fStations[absorberNumber].fAbsorber.A = absVol->GetMaterial()->GetA();
            }

            ++absorberNumber;
            pNavigator->CdUp();
         }

         pNavigator->CdUp();
      }
   }// void HandleGeometry()
   
   struct HandleRPoint
   {
      LxTbMLStation* rStation;
      LxTbLayer* lLayer;
      scaltype deltaZ;
      timetype c;
      
      HandleRPoint() : rStation(0), lLayer(0), deltaZ(0), c(0) {}
      
      void Init()
      {
         c = speedOfLight;
      }
      
      void operator()(LxTbBinnedPoint& point)
      {         
         for (list<LxTbBinnedTriplet*>::iterator i = point.triplets.begin(); i != point.triplets.end(); ++i)
         {
            LxTbBinnedTriplet* triplet = *i;
            scaltype x = point.x + triplet->tx * deltaZ;
            scaltype y = point.y + triplet->ty * deltaZ;
            scaltype trajLen = sqrt(1 + triplet->tx * triplet->tx + triplet->ty * triplet->ty) * deltaZ;
            timetype t = point.t + 1.e9 * trajLen / c; // 1.e9 to convert to ns and trajLenL is negative.
            handleLPoint.rTriplet = triplet;
            IterateNeighbourhood(*lLayer, x, sqrt(point.dx * point.dx + triplet->dtx * triplet->dtx * deltaZ * deltaZ), rStation->fDeltaThetaX * deltaZ,
               y, sqrt(point.dy * point.dy + triplet->dty * triplet->dty * deltaZ * deltaZ), rStation->fDeltaThetaY * deltaZ, t, point.dt, handleLPoint);
         }
      }
      
      struct HandleLPoint
      {
         LxTbBinnedTriplet* rTriplet;
         HandleLPoint() : rTriplet(0) {}
      
         void operator()(LxTbBinnedPoint& point)
         {            
            rTriplet->neighbours.push_back(&point);
         }
      };
   
      HandleLPoint handleLPoint;
   };
   
   HandleRPoint fHandleRPoint;
   
   struct HandleLastPoint
   {
      struct KFParamsCoord
      {
         scaltype coord, tg, C11, C12, C21, C22;
         
         void Clear()
         {
            coord = 0;
            tg = 0;
            C11 = 1.0;
            C12 = 0;
            C21 = 0;
            C22 = 0;
         }
      };

      struct KFParams
      {
         KFParamsCoord xParams;
         KFParamsCoord yParams;
         scaltype chi2;
         
         void Clear()
         {
            xParams.Clear();
            yParams.Clear();
            chi2 = 0;
         }
      };
      
      LxTbDetector& detector;
      
      explicit HandleLastPoint(LxTbDetector& parent) : detector(parent) {}

      KFParamsCoord KFAddPointCoord(KFParamsCoord prevParam, scaltype m, scaltype V, scaltype& chi2, int stationNumber, int layerNumber, int coordNumber)
      {
         KFParamsCoord param = prevParam;
         const LxTbMLStation& station = detector.fStations[stationNumber];
         const LxTbLayer& layer = station.fLayers[layerNumber];
         const LxTbMLStation::Q& Q = station.qs[coordNumber];
         scaltype deltaZ = LAST_LAYER == layerNumber ? LAST_STATION == stationNumber ? 0 : layer.z - detector.fStations[stationNumber + 1].fLayers[0].z :
            layer.z - station.fLayers[layerNumber + 1].z;
         scaltype deltaZSq = deltaZ * deltaZ;

         // Extrapolate.
         param.coord += prevParam.tg * deltaZ; // params[k].tg is unchanged.

         // Filter.
         if (LAST_LAYER == layerNumber)
         {
            param.C11 += prevParam.C12 * deltaZ + prevParam.C21 * deltaZ + prevParam.C22 * deltaZSq + Q.Q11;
            param.C12 += prevParam.C22 * deltaZ + Q.Q12;
            param.C21 += prevParam.C22 * deltaZ + Q.Q21;
            param.C22 += Q.Q22;
         }
         else
         {
            param.C11 += prevParam.C12 * deltaZ + prevParam.C21 * deltaZ + prevParam.C22 * deltaZSq;
            param.C12 += prevParam.C22 * deltaZ;
            param.C21 += prevParam.C22 * deltaZ;
         }

         scaltype S = 1.0 / (V + param.C11);
         scaltype Kcoord = param.C11 * S;
         scaltype Ktg = param.C21 * S;
         scaltype dzeta = m - param.coord;
         param.coord += Kcoord * dzeta;
         param.tg += Ktg * dzeta;
         param.C21 -= param.C11 * Ktg;
         param.C22 -= param.C12 * Ktg;
         param.C11 *= 1.0 - Kcoord;
         param.C12 *= 1.0 - Kcoord;
         chi2 += dzeta * S * dzeta;
         return param;
      }

      KFParams KFAddPoint(KFParams prevParam, scaltype m[2], scaltype V[2], int stationNumber, int layerNumber)
      {
         KFParams param = { KFAddPointCoord(prevParam.xParams, m[0], V[0], param.chi2, stationNumber, layerNumber, 0),
            KFAddPointCoord(prevParam.yParams, m[1], V[1], param.chi2, stationNumber, layerNumber, 1) };
         return param;
      }
      
      KFParams KFAddTriplet(KFParams param, LxTbBinnedPoint* trackCandidatePoints[NOF_STATIONS][NOF_LAYERS], int level)
      {
         LxTbBinnedPoint** points = trackCandidatePoints[level];
         
         for (int i = LAST_LAYER; i >= 0; --i)
         {
            LxTbBinnedPoint* point = points[i];
            scaltype m[2] = { point->x, point->y };
            scaltype V[2] = { point->dx * point->dx, point->dy * point->dy };
            param = KFAddPoint(param, m, V, level, i);
         }
         
         return param;
      }
      
      void HandleTriplet(LxTbBinnedTriplet* triplet, LxTbBinnedPoint* trackCandidatePoints[NOF_STATIONS][NOF_LAYERS], list<LxTBMLFinder::Chain>& chains, int level, KFParams kfParams)
      {
         trackCandidatePoints[level][0] = triplet->lPoint;
         trackCandidatePoints[level][2] = triplet->rPoint;
         kfParams = KFAddTriplet(kfParams, trackCandidatePoints, level);
         
         if (0 == level)
            chains.push_back(LxTBMLFinder::Chain(trackCandidatePoints, kfParams.chi2));
         else
         {            
            for (list<LxTbBinnedPoint*>::iterator i = triplet->neighbours.begin(); i != triplet->neighbours.end(); ++i)
               HandlePoint(*i, trackCandidatePoints, chains, level - 1, kfParams);
         }
      }
      
      void HandlePoint(LxTbBinnedPoint* point, LxTbBinnedPoint* trackCandidatePoints[NOF_STATIONS][NOF_LAYERS], list<LxTBMLFinder::Chain>& chains, int level, KFParams kfParams)
      {
         trackCandidatePoints[level][1] = point;
         
         for (list<LxTbBinnedTriplet*>::iterator i = point->triplets.begin(); i != point->triplets.end(); ++i)
         {
            LxTbBinnedTriplet* triplet = *i;
            HandleTriplet(triplet, trackCandidatePoints, chains, level, kfParams);
         }
      }
      
      void operator()(LxTbBinnedPoint& point)
      {
         LxTbBinnedPoint* trackCandidatePoints[NOF_STATIONS][NOF_LAYERS];
         list<LxTBMLFinder::Chain> chains;
         KFParams kfParams =
         {
            { 0, 0, 1.0, 0, 0, 1.0 },
            { 0, 0, 1.0, 0, 0, 1.0 },
            0
         };
         HandlePoint(&point, trackCandidatePoints, chains, LAST_STATION, kfParams);
         const LxTBMLFinder::Chain* bestChain = 0;
         scaltype chi2 = 0;
                        
         for (list<LxTBMLFinder::Chain>::const_iterator i = chains.begin(); i != chains.end(); ++i)
         {
            const LxTBMLFinder::Chain& chain = *i;

            if (0 == bestChain || chain.chi2 < chi2)
            {
               bestChain = &chain;
               chi2 = chain.chi2;
            }
         }

         if (0 != bestChain)
            detector.recoTracks.push_back(new LxTBMLFinder::Chain(*bestChain));
      }
   };
   
   HandleLastPoint fHandleLastPoint;
   
   struct Debug
   {
      void operator()(LxTbBinnedPoint& point)
      {
         cout << "Point Point Point!!!" << endl;
         
         for (list<LxTbBinnedTriplet*>::iterator i = point.triplets.begin(); i != point.triplets.end(); ++i)
            cout << "Triplet Triplet Triplet!!!" << endl;
      }
   };
   
   Debug debug;
   
   void Reconstruct()
   {
      for (int i = LAST_STATION; i >= 0; --i)
      {
         LxTbMLStation& rStation = fStations[i];
         rStation.Reconstruct();
         
         if (LAST_STATION == i)
         {
            cout << "Points and triplets dump for the station #:" << i << endl;
            IterateLayer(rStation.fLayers[0], debug);
            IterateLayer(rStation.fLayers[1], debug);
            IterateLayer(rStation.fLayers[2], debug);
         }
         
         if (i > 0)
         {
            LxTbLayer& rLayer = rStation.fLayers[1];
            LxTbMLStation& lStation = fStations[i - 1];
            LxTbLayer& lLayer = lStation.fLayers[1];
            fHandleRPoint.rStation = &rStation;
            fHandleRPoint.lLayer = &lLayer;
            fHandleRPoint.deltaZ = lLayer.z - rLayer.z;
            IterateLayer(rLayer, fHandleRPoint);
         }
      }
      
      LxTbMLStation& lastStation = fStations[LAST_STATION];
      LxTbLayer& lastLayer = lastStation.fLayers[1];
      IterateLayer(lastLayer, fHandleLastPoint);
   }
};

static LxTbMLStation* gStations;

LxTBMLFinder::LxTBMLFinder() : fReconstructor(0), fIsEvByEv(true), fNofXBins(20), fNofYBins(20), fNofTBins(fIsEvByEv ? 5 : 1000), fNEvents(1000)
{
   
}

TH1F* deltaXRHisto[NOF_STATIONS];
TH1F* deltaYRHisto[NOF_STATIONS];

InitStatus LxTBMLFinder::Init()
{
   FairRootManager* ioman = FairRootManager::Instance();
    
   if (0 == ioman)
      fLogger->Fatal(MESSAGE_ORIGIN, "No FairRootManager");
   
   int nofEventsInFile = ioman->CheckMaxEventNo();
    
   if (nofEventsInFile < fNEvents)
      fNEvents = nofEventsInFile;
   
   LxTbDetector* pReconstructor = new LxTbDetector(fNofXBins, fNofYBins, fNofTBins);
   fReconstructor = pReconstructor;
   pReconstructor->Init();
   
   fMuchPixelHits = static_cast<TClonesArray*> (ioman->GetObject("MuchPixelHit"));
   
#ifdef LXTB_QA
    CbmMCDataManager* mcManager = static_cast<CbmMCDataManager*> (ioman->GetObject("MCDataManager"));
    fMuchMCPoints = mcManager->InitBranch("MuchPoint");
    fMuchClusters = static_cast<TClonesArray*> (ioman->GetObject("MuchCluster"));
    fMuchPixelDigiMatches = static_cast<TClonesArray*> (ioman->GetObject("MuchDigiMatch"));
    CbmMCDataArray* mcTracks = mcManager->InitBranch("MCTrack");

   for (int i = 0; i < fNEvents; ++i)
   {
      Int_t evSize = mcTracks->Size(0, i);
      fMCTracks.push_back(vector<TrackDataHolder>());

      if (0 >= evSize)
         continue;

      vector<TrackDataHolder>& evTracks = fMCTracks.back();
      const CbmMCTrack* posTrack = 0;
      const CbmMCTrack* negTrack = 0;

      for (int j = 0; j < evSize; ++j)
      {
         evTracks.push_back(TrackDataHolder());
         const CbmMCTrack* mcTrack = static_cast<const CbmMCTrack*> (mcTracks->Get(0, i, j));

         if (mcTrack->GetPdgCode() == 13 || mcTrack->GetPdgCode() == -13)
         {
            Double_t m = mcTrack->GetMass();
            Int_t motherId = mcTrack->GetMotherId();

            if (motherId >= 0 && static_cast<const CbmMCTrack*> (mcTracks->Get(0, i, motherId))->GetPdgCode() == pReconstructor->fSignalParticle->fPdgCode)
            {
               //const CbmMCTrack* motherTrack = static_cast<const CbmMCTrack*> (mcTracks->Get(0, i, motherId));

               //if (fFinder->fSignalParticle->fPdgCode == motherTrack->GetPdgCode())
               {
                  evTracks.back().isSignal = true;
                  evTracks.back().isPos = mcTrack->GetPdgCode() == -13;

                  if (-13 == mcTrack->GetPdgCode())
                     posTrack = mcTrack;
                  else
                     negTrack = mcTrack;
               }
            }
         }
      }// for (int j = 0; j < evSize; ++j)
   }// for (int i = 0; i < fNEvents; ++i)
    
   fEventTimes.resize(fNEvents); 
   fEventTimes[0] = 50;
    
   for (int i = 1; i < fNEvents; ++i)
      fEventTimes[i] = fEventTimes[i - 1] + 100;

   for (int i = 0; i < fNEvents; ++i)
   {
      Int_t evSize = fMuchMCPoints->Size(0, i);
      fMuchPoints.push_back(vector<PointDataHolder>());

      if (0 >= evSize)
         continue;

      //++numEvents;
      vector<PointDataHolder>& evPoints = fMuchPoints.back();

      for (int j = 0; j < evSize; ++j)
      {
         const CbmMuchPoint* pMuchPt = static_cast<const CbmMuchPoint*> (fMuchMCPoints->Get(0, i, j));
         PointDataHolder muchPt;
         muchPt.x = (pMuchPt->GetXIn() + pMuchPt->GetXOut()) / 2;
         muchPt.y = (pMuchPt->GetYIn() + pMuchPt->GetYOut()) / 2;
         muchPt.t = fEventTimes[i] + pMuchPt->GetTime();
         muchPt.eventId = i;
         muchPt.trackId = pMuchPt->GetTrackID();
         muchPt.pointId = j;
         muchPt.stationNumber = CbmMuchGeoScheme::GetStationIndex(pMuchPt->GetDetectorId());
         muchPt.layerNumber = CbmMuchGeoScheme::GetLayerIndex(pMuchPt->GetDetectorId());
         evPoints.push_back(muchPt);
         fMCTracks[muchPt.eventId][muchPt.trackId].pointInds[muchPt.stationNumber][muchPt.layerNumber] = muchPt.pointId;
      }
   }
   
   for (vector<vector<TrackDataHolder> >::iterator i = fMCTracks.begin(); i != fMCTracks.end(); ++i)
   {
      vector<TrackDataHolder>& evTracks = *i;

      for (vector<TrackDataHolder>::iterator j = evTracks.begin(); j != evTracks.end(); ++j)
      {
         TrackDataHolder& track = *j;

         if (!track.isSignal)
            continue;

         for (int k = 0; k < NOF_STATIONS; ++k)
         {
            for (int l = 0; l < NOF_LAYERS; ++l)
            {
               if (track.pointInds[k][l] < 0)
               {
                  track.isSignal = false;
                  break;
               }
            }
            
            if (!track.isSignal)
               break;
         }
      }
   }
   
   for (int i = 0; i < NOF_STATIONS; ++i)
   {
      char buf[64];
      sprintf(buf, "deltaXRHisto_%d", i);
      deltaXRHisto[i] = new TH1F(buf, buf, 300, -15., 15.);
      sprintf(buf, "deltaYRHisto_%d", i);
      deltaYRHisto[i] = new TH1F(buf, buf, 300, -15., 15.);
   }
#endif//LXTB_QA
    
   return kSUCCESS;//, kERROR, kFATAL
}

static Int_t currentEventN = 0;
static unsigned long long tsStartTime = 0;

#ifdef LXTB_QA
static long fullDuration = 0;
#endif//LXTB_QA

#ifdef LXTB_EMU_TS
static Double_t min_ts_time = 100000;
static Double_t max_ts_time = -100000;
static list<LxTbBinnedPoint> ts_points;
#endif//LXTB_EMU_TS

#ifdef LXTB_DEBUG
/*struct LxTbDebug
   {
      struct Triplet
      {
         Int_t left;
         Int_t middle;
         Int_t right;
         
         Triplet(Int_t l, Int_t m, Int_t r) : left(l), middle(m), right(r) {}
      };
      
      struct TrLess
      {
         bool operator()(const Triplet& a, const Triplet& b) const
         {
            if (a.left < b.left)
               return true;
            else if (a.middle < b.middle)
               return true;
            else if (a.right < b.right)
               return true;
            
            return false;
         }
      };
      
      map<Triplet, bool, TrLess> triplets[NOF_STATIONS];
      int stationNumber;
      
      explicit LxTbDebug(vector<LxTBMLFinder::TrackDataHolder>& mcTracks) : stationNumber(-1)
      {
         for (vector<LxTBMLFinder::TrackDataHolder>::const_iterator i = mcTracks.begin(); i != mcTracks.end(); ++i)
         {
            const LxTBMLFinder::TrackDataHolder& track = *i;
            
            if (!track.isSignal)
               continue;
            
            for (int j = 0; j < NOF_STATIONS; ++j)
            {
               Triplet trip(track.pointInds[j][0], track.pointInds[j][1], track.pointInds[j][2]);
               triplets[j][trip] = false;
            }
         }
      }
      
      void operator()(const LxTbBinnedPoint& point)
      {
         if (!point.use)
            return;
         
         for (list<LxTbBinnedTriplet*>::const_iterator i = point.triplets.begin(); i != point.triplets.end(); ++i)
         {
            LxTbBinnedTriplet* trip = *i;
            
            for (list<LxTbBinnedPoint::PointDesc>::const_iterator j = point.mcRefs.begin(); j != point.mcRefs.end(); ++j)
            {
               Int_t mId = j->pointId;
               
               for (list<LxTbBinnedPoint::PointDesc>::const_iterator k = trip->rPoint->mcRefs.begin(); k != trip->rPoint->mcRefs.end(); ++k)
               {
                  Int_t rId = k->pointId;
                  
                  for (list<LxTbBinnedPoint::PointDesc>::const_iterator l = trip->lPoint->mcRefs.begin(); l != trip->lPoint->mcRefs.end(); ++l)
                  {
                     Int_t lId = l->pointId;
                     Triplet mcTrip(lId, mId, rId);
                     map<Triplet, bool, TrLess>::iterator iter = triplets[stationNumber].find(mcTrip);
                     
                     if (iter != triplets[stationNumber].end())
                     { 
                        iter->second = true;
                        cout << stationNumber << " " << iter->first.left << " " << iter->first.middle << " " << iter->first.right << " " << iter->second << endl;
                     }
                  }
               }
            }
         }
      }
   };*/
#endif//LXTB_DEBUG

void LxTBMLFinder::Exec(Option_t* opt)
{
   LxTbDetector* pReconstructor = static_cast<LxTbDetector*> (fReconstructor);
   pReconstructor->Clear();
   pReconstructor->SetMinT(tsStartTime);
   
   for (int i = 0; i < fMuchPixelHits->GetEntriesFast(); ++i)
   {
      const CbmMuchPixelHit* hit = static_cast<const CbmMuchPixelHit*> (fMuchPixelHits->At(i));      
      Int_t stationNumber = CbmMuchGeoScheme::GetStationIndex(hit->GetAddress());
      Int_t layerNumber = CbmMuchGeoScheme::GetLayerIndex(hit->GetAddress());
      scaltype x = hit->GetX();
      scaltype y = hit->GetY();
      timetype t = hit->GetTime();
      scaltype dx = hit->GetDx();
      scaltype dy = hit->GetDy();
      timetype dt = 4; //hit->GetTimeError();
      LxTbBinnedPoint point(x, dx, y, dy, t, dt, i, LAST_STATION == stationNumber);
#ifdef LXTB_QA
      point.isTrd = false;
      point.stationNumber = stationNumber;
      point.layerNumber = layerNumber;
      Int_t clusterId = hit->GetRefId();
      const CbmCluster* cluster = static_cast<const CbmCluster*> (fMuchClusters->At(clusterId));
      Int_t nDigis = cluster->GetNofDigis();
      double avT = 0;
#ifdef LXTB_EMU_TS
      double avTErr = 0;
#endif//LXTB_EMU_TS
      int nofT = 0;

      for (Int_t j = 0; j < nDigis; ++j)
      {
         const CbmMatch* digiMatch = static_cast<const CbmMatch*> (fMuchPixelDigiMatches->At(cluster->GetDigi(j)));
         Int_t nMCs = digiMatch->GetNofLinks();

         for (Int_t k = 0; k < nMCs; ++k)
         {
            const CbmLink& lnk = digiMatch->GetLink(k);
            Int_t eventId = fIsEvByEv ? currentEventN : lnk.GetEntry();
            Int_t pointId = lnk.GetIndex();            
            const FairMCPoint* pMCPt = static_cast<const FairMCPoint*> (fMuchMCPoints->Get(0, eventId, pointId));
            Int_t trackId = pMCPt->GetTrackID();
            LxTbBinnedPoint::PointDesc ptDesc = {eventId, pointId, trackId};
            point.mcRefs.push_back(ptDesc);
            Double_t deltaT = fMuchPoints[eventId][pointId].t;
#ifdef LXTB_EMU_TS
            deltaT += gRandom->Gaus(0, 4);
            avTErr += 4 * 4;
#endif//LXTB_EMU_TS
            avT += deltaT;
            ++nofT;
         }
      }

      if (nofT > 0)
      {
         avT /= nofT;
#ifdef LXTB_EMU_TS
         avTErr = TMath::Sqrt(avTErr);
         avTErr /= nofT;
         dt = avT;
#endif//LXTB_EMU_TS
      }

      t = avT;
#endif//LXTB_QA
      point.t = t;
      //point.t = tsStartTime + hit->GetTime();
      //point.dt = hit->GetTimeError();
   
#ifdef LXTB_EMU_TS
      ts_points.push_back(point);
   
      if (min_ts_time > t)
         min_ts_time = t;
   
      if (max_ts_time < t)
         max_ts_time = t;
#else//LXTB_EMU_TS
      scaltype minY = pReconstructor->fStations[stationNumber].fLayers[layerNumber].minY;
      scaltype binSizeY = pReconstructor->fStations[stationNumber].fLayers[layerNumber].yBinLength;
      int lastYBin = (pReconstructor->fStations[stationNumber].fLayers[layerNumber].lastYBinNumber);
      scaltype minX = pReconstructor->fStations[stationNumber].fLayers[layerNumber].minX;
      scaltype binSizeX = pReconstructor->fStations[stationNumber].fLayers[layerNumber].xBinLength;
      int lastXBin = pReconstructor->fStations[stationNumber].fLayers[layerNumber].lastXBinNumber;
      int last_timebin = pReconstructor->fStations[stationNumber].fLayers[layerNumber].lastTimeBinNumber;

      int tInd = (t - pReconstructor->fStations[stationNumber].fLayers[layerNumber].minT) / TIMEBIN_LENGTH;

      if (tInd < 0)
         tInd = 0;
      else if (tInd > last_timebin)
         tInd = last_timebin;

      LxTbTYXBin& tyxBin = pReconstructor->fStations[stationNumber].fLayers[layerNumber].tyxBins[tInd];
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
      xBin.points.push_back(point);

      if (LAST_STATION == stationNumber)
      {
         xBin.use = true;
         yxBin.use = true;
         tyxBin.use = true;
         
         if (0 == layerNumber)
            xBin.use = true;
         else if (1 == layerNumber)
            xBin.use = true;
         else if (2 == layerNumber)
            xBin.use = true;
      }
#endif//LXTB_EMU_TS
   }// for (int i = 0; i < fMuchPixelHits->GetEntriesFast(); ++i)
   
   pReconstructor->Reconstruct();
   
#ifdef LXTB_DEBUG
   static int nofTriplesAll[NOF_STATIONS] = { 0, 0, 0, 0 };
   static int nofTriplesFound[NOF_STATIONS] = { 0, 0, 0, 0 };
   
   struct Debug
   {
      struct Triplet
      {
         Int_t left;
         Int_t middle;
         Int_t right;
         
         Triplet(Int_t l, Int_t m, Int_t r) : left(l), middle(m), right(r) {}
      };
      
      struct TrLess
      {
         bool operator()(const Triplet& a, const Triplet& b) const
         {
            if (a.left < b.left)
               return true;
            else if (a.middle < b.middle)
               return true;
            else if (a.right < b.right)
               return true;
            
            return false;
         }
      };
      
      map<Triplet, bool, TrLess> triplets[NOF_STATIONS];
      int stationNumber;
      
      explicit Debug(vector<TrackDataHolder>& mcTracks) : stationNumber(-1)
      {
         for (vector<TrackDataHolder>::const_iterator i = mcTracks.begin(); i != mcTracks.end(); ++i)
         {
            const TrackDataHolder& track = *i;
            
            if (!track.isSignal)
               continue;
            
            for (int j = 0; j < NOF_STATIONS; ++j)
            {
               Triplet trip(track.pointInds[j][0], track.pointInds[j][1], track.pointInds[j][2]);
               triplets[j][trip] = false;
            }
         }
      }
      
      void operator()(const LxTbBinnedPoint& point)
      {
         if (!point.use)
            return;
         
         for (list<LxTbBinnedTriplet*>::const_iterator i = point.triplets.begin(); i != point.triplets.end(); ++i)
         {
            LxTbBinnedTriplet* trip = *i;
            
            for (list<LxTbBinnedPoint::PointDesc>::const_iterator j = point.mcRefs.begin(); j != point.mcRefs.end(); ++j)
            {
               Int_t mId = j->pointId;
               
               for (list<LxTbBinnedPoint::PointDesc>::const_iterator k = trip->rPoint->mcRefs.begin(); k != trip->rPoint->mcRefs.end(); ++k)
               {
                  Int_t rId = k->pointId;
                  
                  for (list<LxTbBinnedPoint::PointDesc>::const_iterator l = trip->lPoint->mcRefs.begin(); l != trip->lPoint->mcRefs.end(); ++l)
                  {
                     Int_t lId = l->pointId;
                     Triplet mcTrip(lId, mId, rId);
                     map<Triplet, bool, TrLess>::iterator iter = triplets[stationNumber].find(mcTrip);
                     
                     if (iter != triplets[stationNumber].end())
                        iter->second = true;
                  }
               }
            }
         }
      }
   };
   
   Debug debug(fMCTracks[currentEventN]);

   struct DebugTrack
   {
      list<Int_t> pointIds[NOF_STATIONS][NOF_LAYERS];
   };
   
   map<Int_t, DebugTrack> debugTracks;
   
   struct DebugTrack2
   {
      list<LxTbBinnedPoint*> points[NOF_STATIONS][NOF_LAYERS];
   };
   
   map<Int_t, DebugTrack2> debugTracks2;
   
   /*for (int i = 0; i < fMuchPixelHits->GetEntriesFast(); ++i)
   {
      const CbmMuchPixelHit* hit = static_cast<const CbmMuchPixelHit*> (fMuchPixelHits->At(i));      
      Int_t stationNumber = CbmMuchGeoScheme::GetStationIndex(hit->GetAddress());
      Int_t layerNumber = CbmMuchGeoScheme::GetLayerIndex(hit->GetAddress());
      Int_t clusterId = hit->GetRefId();
      const CbmCluster* cluster = static_cast<const CbmCluster*> (fMuchClusters->At(clusterId));
      Int_t nDigis = cluster->GetNofDigis();
      
      for (Int_t j = 0; j < nDigis; ++j)
      {
         const CbmMatch* digiMatch = static_cast<const CbmMatch*> (fMuchPixelDigiMatches->At(cluster->GetDigi(j)));
         Int_t nMCs = digiMatch->GetNofLinks();

         for (Int_t k = 0; k < nMCs; ++k)
         {
            const CbmLink& lnk = digiMatch->GetLink(k);
            Int_t pointId = lnk.GetIndex();
            const FairMCPoint* pMCPt = static_cast<const FairMCPoint*> (fMuchMCPoints->Get(0, currentEventN, pointId));
            Int_t trackId = pMCPt->GetTrackID();
            debugTracks[trackId].pointIds[stationNumber][layerNumber].push_back(pointId);
         }
      }
   }*/
   
   for (int i = 0; i < NOF_STATIONS; ++i)
   {
      for (int j = 0; j < NOF_LAYERS; ++j)
      {
         const LxTbLayer& layer = pReconstructor->fStations[i].fLayers[j];
         
         for (int k = 0; k < layer.nofTYXBins; ++k)
         {
            LxTbTYXBin& tyxBin = layer.tyxBins[k];

            for (int l = 0; l < layer.nofYXBins; ++l)
            {
               LxTbYXBin& yxBin = tyxBin.yxBins[l];

               for (int m = 0; m < layer.nofXBins; ++m)
               {
                  LxTbXBin& xBin = yxBin.xBins[m];

                  for (std::list<LxTbBinnedPoint>::iterator n = xBin.points.begin(); n != xBin.points.end(); ++n)
                  {
                     LxTbBinnedPoint& point = *n;
                     
                     for (list<LxTbBinnedPoint::PointDesc>::const_iterator o = point.mcRefs.begin(); o != point.mcRefs.end(); ++o)
                     {
                        const LxTbBinnedPoint::PointDesc& pd = *o;
                        debugTracks[pd.trackId].pointIds[point.stationNumber][point.layerNumber].push_back(pd.pointId);
                        debugTracks2[pd.trackId].points[point.stationNumber][point.layerNumber].push_back(&point);
                     }
                  }
               }
            }
         }
      }
   }
   
   for (map<Int_t, DebugTrack>::const_iterator i = debugTracks.begin(); i != debugTracks.end(); ++i)
   {
      const DebugTrack& dt = i->second;
      
      for (int j = 0; j < NOF_STATIONS; ++j)
      {
         for (list<Int_t>::const_iterator k = dt.pointIds[j][1].begin(); k != dt.pointIds[j][1].end(); ++k)
         {
            Int_t mId = *k;
            
            for (list<Int_t>::const_iterator l = dt.pointIds[j][2].begin(); l != dt.pointIds[j][2].end(); ++l)
            {
               Int_t rId = *l;
               
               for (list<Int_t>::const_iterator m = dt.pointIds[j][0].begin(); m != dt.pointIds[j][0].end(); ++m)
               {
                  Int_t lId = *m;
                  Debug::Triplet mcTrip(lId, mId, rId);
                  map<Debug::Triplet, bool, Debug::TrLess>::iterator iter = debug.triplets[j].find(mcTrip);
                     
                  if (iter != debug.triplets[j].end())
                     iter->second = true;
               }
            }
         }
      }
   }
   
   for (map<Int_t, DebugTrack2>::const_iterator i = debugTracks2.begin(); i != debugTracks2.end(); ++i)
   {
      const DebugTrack2& dt = i->second;
      
      for (int j = 0; j < NOF_STATIONS; ++j)
      {
         for (list<LxTbBinnedPoint*>::const_iterator k = dt.points[j][1].begin(); k != dt.points[j][1].end(); ++k)
         {
            LxTbBinnedPoint* mPt = *k;
            scaltype rTx = mPt->x / pReconstructor->fStations[j].fLayers[1].z;
            scaltype rTy = mPt->y / pReconstructor->fStations[j].fLayers[1].z;
            scaltype deltaZr = pReconstructor->fStations[j].fLayers[2].z - pReconstructor->fStations[j].fLayers[1].z;
            scaltype rX = mPt->x + rTx * deltaZr;
            scaltype rY = mPt->y + rTy * deltaZr;
            
            for (list<LxTbBinnedPoint*>::const_iterator l = dt.points[j][2].begin(); l != dt.points[j][2].end(); ++l)
            {
               LxTbBinnedPoint* rPt = *l;
               deltaXRHisto[j]->Fill(rPt->x - rX);
               deltaYRHisto[j]->Fill(rPt->y - rY);
               
               for (list<LxTbBinnedPoint*>::const_iterator m = dt.points[j][0].begin(); m != dt.points[j][0].end(); ++m)
               {
                  LxTbBinnedPoint* lPt = *m;
               }
            }
         }
      }
   }
   
   for (int i = LAST_STATION; i >= 0; --i)
   {
      debug.stationNumber = i;
      IterateLayer(pReconstructor->fStations[i].fLayers[1], debug);
      
      nofTriplesAll[i] += debug.triplets[i].size();
      
      for (map<Debug::Triplet, bool, Debug::TrLess>::const_iterator j = debug.triplets[i].begin(); j != debug.triplets[i].end(); ++j)
      {
         if (j->second)
            ++nofTriplesFound[i];
      }
      
      double foundPerc = 100 * nofTriplesFound[i];
      foundPerc /= nofTriplesAll[i];
      cout << "For station #:" << i << " found triplets: " << foundPerc << " ( " << nofTriplesFound[i] << " / " << nofTriplesAll[i] << " )" << endl;
   }
#endif//LXTB_DEBUG
   
   cout << "In event #" << currentEventN << " reconstructed: " << pReconstructor->recoTracks.size() << " tracks" << endl;
   recoTracks.splice(recoTracks.end(), pReconstructor->recoTracks);
   ++currentEventN;
   tsStartTime += 100;
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

void LxTBMLFinder::Finish()
{
#ifdef LXTB_EMU_TS
   Double_t tCoeff = TIMEBIN_LENGTH * nof_timebins / (max_ts_time - min_ts_time);
   
   for (list<LxTbBinnedPoint>::iterator i = ts_points.begin(); i != ts_points.end(); ++i)
   {
      LxTbBinnedPoint& point = *i;
      point.t = (point.t - min_ts_time) * tCoeff;
      point.dt *= tCoeff;
      
      bool isTrd = point.isTrd;
      Int_t stationNumber = point.stationNumber;
      scaltype minY = (isTrd ? fFinder->trdStation.minY : fFinder->stations[stationNumber].minY);
      scaltype binSizeY = (isTrd ? fFinder->trdStation.binSizeY : fFinder->stations[stationNumber].binSizeY);
      int lastYBin = (isTrd ? fFinder->trdStation.lastYBin : fFinder->stations[stationNumber].lastYBin);
      scaltype minX = (isTrd ? fFinder->trdStation.minX : fFinder->stations[stationNumber].minX);
      scaltype binSizeX = (isTrd ? fFinder->trdStation.binSizeX : fFinder->stations[stationNumber].binSizeX);
      int lastXBin = (isTrd ? fFinder->trdStation.lastXBin : fFinder->stations[stationNumber].lastXBin);

      int tInd = (point.t - fFinder->minT) / CUR_TIMEBIN_LENGTH;

      if (tInd < 0)
         tInd = 0;
      else if (tInd > last_timebin)
         tInd = last_timebin;

      LxTbTYXBin& tyxBin = (isTrd ? fFinder->trdStation.tyxBinsArr[stationNumber][tInd] : fFinder->stations[stationNumber].tyxBins[tInd]);
      int yInd = (point.y - minY) / binSizeY;

      if (yInd < 0)
         yInd = 0;
      else if (yInd > lastYBin)
         yInd = lastYBin;

      LxTbYXBin& yxBin = tyxBin.yxBins[yInd];
      int xInd = (point.x - minX) / binSizeX;

      if (xInd < 0)
         xInd = 0;
      else if (xInd > lastXBin)
         xInd = lastXBin;

      LxTbXBin& xBin = yxBin.xBins[xInd];
      xBin.points.push_back(point);

      if (/*!hasTrd && */CUR_LAST_STATION == stationNumber)
      {
         xBin.use = true;
         yxBin.use = true;
         tyxBin.use = true;
      }
   }
   
#ifdef LXTB_TIE
   fDetector->SetTSBegin(0);
#endif//LXTB_TIE
   
   fFinder->Reconstruct();

   for (list<timetype>::iterator i = shortSignalMCTimes.begin(); i != shortSignalMCTimes.end(); ++i)
   {
      timetype& v = *i;
      v = (v - min_ts_time) * tCoeff;
   }
   
   for (list<timetype>::iterator i = longSignalMCTimes.begin(); i != longSignalMCTimes.end(); ++i)
   {
      timetype& v = *i;
      v = (v - min_ts_time) * tCoeff;
   }
   
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
#endif//LXTB_EMU_TS
   cout << "LxTbBinnedFinder::Reconstruct() full duration was: " << fullDuration << endl;
   
   int nofRecoTracks = recoTracks.size();   
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
            
         if (!track.isSignal)
             continue;
         
         ++nofSignalTracks;
         
         int nofMatchPoints = 0;
            
         for (list<Chain*>::const_iterator k = recoTracks.begin(); k != recoTracks.end(); ++k)
         {
            const Chain* chain = *k;
               
            for (int l = 0; l < NOF_STATIONS; ++l)
            {
               for (int m = 0; m < NOF_LAYERS; ++m)
               {
                  bool pointsMatched = false;
               
                  for (list<LxTbBinnedPoint::PointDesc>::const_iterator n = chain->points[l][m]->mcRefs.begin(); n != chain->points[l][m]->mcRefs.end(); ++n)
                  {
                     if (n->eventId == eventN && n->pointId == track.pointInds[l][m])
                     {
                        pointsMatched = true;
                        break;
                     }
                  }
               
                  if (pointsMatched)
                     ++nofMatchPoints;
               }
            }
         }
         
         if (nofMatchPoints >= NOF_STATIONS * NOF_LAYERS * 0.7)
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
   
   for (list<Chain*>::const_iterator i = recoTracks.begin(); i != recoTracks.end(); ++i)
   {
      const Chain* chain = *i;
      map<RecoTrackData, int, RTDLess> nofTracks;
      
      for (int j = 0; j < NOF_STATIONS; ++j)
      {
         for (int k = 0; k < NOF_LAYERS; ++k)
         {
            int stMask = 1 << j * NOF_LAYERS + k;
         
            for (list<LxTbBinnedPoint::PointDesc>::const_iterator l = chain->points[j][k]->mcRefs.begin(); l != chain->points[j][k]->mcRefs.end(); ++l)
            {
               RecoTrackData st(l->eventId, l->trackId);
               map<RecoTrackData, int, RTDLess>::iterator nofTIter = nofTracks.find(st);

               if (nofTIter != nofTracks.end())
                  nofTIter->second |= stMask;
               else
                  nofTracks[st] = stMask;
            }
         }
      }
        
      int nofPoints = 0;
        
      for (map<RecoTrackData, int, RTDLess>::const_iterator j = nofTracks.begin(); j != nofTracks.end(); ++j)
      {
         int nofp = 0;
         
         for (int k = 0; k < NOF_STATIONS; ++k)
         {
            for (int l = 0; l < NOF_LAYERS; ++l)
            {
               if (j->second & (1 << k * NOF_LAYERS + l))
                  ++nofp;
            }
         }
         
         if (nofp > nofPoints)
            nofPoints = nofp;
      }
        
      if (nofPoints >= NOF_STATIONS * NOF_LAYERS * 0.7)
         ++nofRightRecoTracks;
   }
   
   eff = 0 == recoTracks.size() ? 100 : 100.0 * nofRightRecoTracks / nofRecoTracks;
   cout << "Non ghosts are: " << eff << "% [ " << nofRightRecoTracks << " / " << nofRecoTracks << " ]" << endl;
   
   //cout << "Have: " << shortSignalMCTimes.size() << " short signaling events" << endl;
   //cout << "Have: " << longSignalMCTimes.size() << " long signaling events" << endl;
   cout << "Have: " << currentEventN << " events" << endl;
   
   ofstream nofEventsFile("nof_events.txt", ios_base::out | ios_base::trunc);
   nofEventsFile << currentEventN;
   
   /*ofstream nofShortSignalsFile("nof_short_signals.txt", ios_base::out | ios_base::trunc);
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
   PrintTrigger(triggerTimes_trd1_sign1_dist1, longSignalMCTimes, "triggerTimes_trd1_sign1_dist1", true);
   
   Int_t nofTriggerDigis = 0;
   
   for (set<Int_t>::const_iterator i = fFinder->triggerEventNumber.begin(); i != fFinder->triggerEventNumber.end(); ++i)
      nofTriggerDigis += nof_ev_digis[*i];
   
   ofstream nofTriggerDigisFile("nof_trigger_digis.txt", ios_base::out | ios_base::trunc);
   nofTriggerDigisFile << nofTriggerDigis;
   ofstream nofDigisFile("nof_digis.txt", ios_base::out | ios_base::trunc);
   nofDigisFile << nof_digis;*/
   
   for (int i = 0; i < NOF_STATIONS; ++i)
   {
      TFile* curFile = TFile::CurrentFile();
      TString histoNameXR = deltaXRHisto[i]->GetName();
      histoNameXR += ".root";
      TFile fhXR(histoNameXR.Data(), "RECREATE");
      deltaXRHisto[i]->Write();
      fhXR.Close();
      delete deltaXRHisto[i];
      TFile::CurrentFile() = curFile;
      
      curFile = TFile::CurrentFile();
      TString histoNameYR = deltaYRHisto[i]->GetName();
      histoNameYR += ".root";
      TFile fhYR(histoNameYR.Data(), "RECREATE");
      deltaYRHisto[i]->Write();
      fhYR.Close();
      delete deltaYRHisto[i];
      TFile::CurrentFile() = curFile;
   }
#endif//LXTB_QA
   
   for (list<Chain*>::iterator i = recoTracks.begin(); i != recoTracks.end(); ++i)
      delete *i;
}
