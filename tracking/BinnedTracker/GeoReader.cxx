/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "GeoReader.h"
#include "TGeoBBox.h"
#include "Tracker.h"
#include "Station3D.h"
#include "Station4D.h"
#include "TClonesArray.h"
#include "TMath.h"

using namespace std;

CbmBinnedGeoReader* CbmBinnedGeoReader::Instance()
{
   static CbmBinnedGeoReader* theInstance = 0;
   
   if (0 != theInstance)
      return theInstance;
   
   FairRootManager* ioman = FairRootManager::Instance();
   
   if (0 == ioman)
      return 0;
   
   theInstance = new CbmBinnedGeoReader(ioman, CbmBinnedTracker::Instance());
   return theInstance;
}

CbmBinnedGeoReader::CbmBinnedGeoReader(FairRootManager* ioman, CbmBinnedTracker* tracker) : fIoman(ioman), fNavigator(0), fDetectorReaders(), fTracker(tracker)
{
   fNavigator = gGeoManager->GetCurrentNavigator();
   fDetectorReaders["sts"] = &CbmBinnedGeoReader::ReadSts;
   //fDetectorReaders["rich"] = &CbmBinnedGeoReader::ReadRich;
   fDetectorReaders["much"] = &CbmBinnedGeoReader::ReadMuch;
   fDetectorReaders["trd"] = &CbmBinnedGeoReader::ReadTrd;
   fDetectorReaders["tof"] = &CbmBinnedGeoReader::ReadTof;
   
   CbmBinnedHitReader::AddReader("sts", static_cast<TClonesArray*> (fIoman->GetObject("StsHit")));
   //CbmBinnedHitReader::AddReader("rich", static_cast<TClonesArray*> (fIoman->GetObject("RichHit")));
   CbmBinnedHitReader::AddReader("much", static_cast<TClonesArray*> (fIoman->GetObject("MuchPixelHit")));
   CbmBinnedHitReader::AddReader("trd", static_cast<TClonesArray*> (fIoman->GetObject("TrdHit")));
   CbmBinnedHitReader::AddReader("tof", static_cast<TClonesArray*> (fIoman->GetObject("TofHit")));
}

void CbmBinnedGeoReader::FindGeoChild(TGeoNode* node, const char* name, list<TGeoNode*>& results)
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

Double_t cbmBinnedSOL = 0;

void CbmBinnedGeoReader::Read()
{
   ReadDetector("sts");
   ReadDetector("trd");
   ReadDetector("much");
   ReadDetector("tof");
   //ReadDetector("rich");
   //ReadTarget();
   cbmBinnedSOL = 1.e-7 * TMath::C();// Speed of light in cm/ns
}

void CbmBinnedGeoReader::ReadDetector(const char* name)
{
   (this->*fDetectorReaders[name])();
}

static const int gNofZbins = 5;
static const int gNofYbins = 20;
static const int gNofXbins = 20;
static const int gNofTbins = 5;

static Double_t gStationErrors[][3] =
{
   { 0.014601, 0.156848, 3.535534 },// Sts 0
   { 0.013942, 0.108157, 3.535534 },// Sts 1
   
   { 0.359375, 7.75, 4 },// Trd 0
   { 7.75, 0.359375, 4 },// Trd 1
   { 0.359375, 7.75, 4 },// Trd 2
   { 7.75, 0.359375, 4 },// Trd 3
   /*{ 0.359375, 2.75, 4 },// Trd 0
   { 2.75, 0.359375, 4 },// Trd 1
   { 0.359375, 2.75, 4 },// Trd 2
   { 2.75, 0.359375, 4 },// Trd 3*/
   
   { 0.487468, 0.487468, 4 },// Much 0
   { 0.487468, 0.487468, 4 },// Much 1
   { 0.487468, 0.487468, 4 },// Much 2
   
   { 0.288675, 0.72, 4 },// Tof
   
   { 0.175, 0.175, 4 }// Rich
};

static Double_t gTofStationTxError = 0.001277;
static Double_t gTofStationTyError = 0.003186;

void CbmBinnedGeoReader::SearchStation(TGeoNode* node, CbmBinnedHitReader* hitReader, list<const char*>::const_iterator stationPath,
   list<const char*>::const_iterator stationPathEnd, const std::list<const char*>& geoPath, bool is4d)
{
   if (stationPath == stationPathEnd)
   {
      /*Double_t localCoords[3] = { 0, 0, 0 };
      Double_t globalCoords[3];
      gGeoManager->LocalToMaster(localCoords, globalCoords);
      Double_t z = globalCoords[2];
      const char* name1 = node->GetName();
      const char* name2 = gGeoManager->GetCurrentNode()->GetName();*/
      static int stationNumber = 0;
      Double_t left = 10000;
      Double_t right = -10000;
      Double_t top = -10000;
      Double_t bottom = 10000;
      Double_t front = 10000;
      Double_t back = -10000;
      HandleStation(node, geoPath.begin(), geoPath.end(), left, right, top, bottom, front, back);
      CbmBinnedStation* station = 0;
      
      if (is4d)
      {
         CbmBinned4DStation* station4d = new CbmBinned4DStation(gNofZbins, gNofYbins, gNofXbins, gNofTbins);
         station4d->SetMinZ(front);
         station4d->SetMaxZ(back);
         station4d->SetDtx(gTofStationTxError);
         station4d->SetDty(gTofStationTyError);
         station = station4d;
      }
      else
      {
         CbmBinned3DStation* station3d = new CbmBinned3DStation(gNofYbins, gNofXbins, gNofTbins);
         station3d->SetZ((front + back) / 2);
         station = station3d;
      }
      
      station->SetMinY(bottom);
      station->SetMaxY(top);
      station->SetMinX(left);
      station->SetMaxX(right);
      
      station->SetDx(gStationErrors[stationNumber][0]);
      station->SetDy(gStationErrors[stationNumber][1]);
      station->SetDt(gStationErrors[stationNumber][2]);
      
      station->Init();
      fTracker->AddStation(station);
      hitReader->AddStation(station);
      ++stationNumber;
   }
   else
   {
      const char* name = *stationPath++;
      list<TGeoNode*> subNodes;
      FindGeoChild(node, name, subNodes);
   
      for (list<TGeoNode*>::iterator i = subNodes.begin(); i != subNodes.end(); ++i)
      {
         fNavigator->CdDown(*i);
         SearchStation(*i, hitReader, stationPath, stationPathEnd, geoPath);
         fNavigator->CdUp();
      }
   }
}

void CbmBinnedGeoReader::HandleStation(TGeoNode* node, list<const char*>::const_iterator geoPath, list<const char*>::const_iterator geoPathEnd,
   Double_t& left, Double_t& right, Double_t& top, Double_t& bottom, Double_t& front, Double_t& back)
{   
   if (geoPath == geoPathEnd)
      HandleActive(node, left, right, top, bottom, front, back);
   else
   {
      const char* name = *geoPath++;
      list<TGeoNode*> subNodes;
      FindGeoChild(node, name, subNodes);
   
      for (list<TGeoNode*>::iterator i = subNodes.begin(); i != subNodes.end(); ++i)
      {
         fNavigator->CdDown(*i);
         HandleStation(*i, geoPath, geoPathEnd, left, right, top, bottom, front, back);
         fNavigator->CdUp();
      }
   }
}

void CbmBinnedGeoReader::HandleActive(TGeoNode* node, Double_t& left, Double_t& right, Double_t& top, Double_t& bottom, Double_t& front, Double_t& back)
{
   TGeoBBox* pBox = static_cast<TGeoBBox*> (node->GetVolume()->GetShape());
   pBox->ComputeBBox();
   
   for (int i = -1; i <= 1; i += 2)
   {
      for (int j = -1; j <= 1; j += 2)
      {
         for (int k = -1; k <= 1; k += 2)
         {
            Double_t localCoords[3] = { i * pBox->GetDX(), j * pBox->GetDY(), k * pBox->GetDZ() };
            Double_t globalCoords[3];
            gGeoManager->LocalToMaster(localCoords, globalCoords);
            
            if (left > globalCoords[0])
               left = globalCoords[0];
            
            if (right < globalCoords[0])
               right = globalCoords[0];
            
            if (bottom > globalCoords[1])
               bottom = globalCoords[1];
            
            if (top < globalCoords[1])
               top = globalCoords[1];
            
            if (front > globalCoords[2])
               front = globalCoords[2];
            
            if (back < globalCoords[2])
               back = globalCoords[2];
         }
      }
   }
}

void CbmBinnedGeoReader::ReadSts()
{
   list<const char*> stationPath = { "sts", "Station" };
   list<const char*> geoPath = { "Ladder", "HalfLadder", "Module", "Sensor" };
   gGeoManager->cd("/cave_1");
   SearchStation(gGeoManager->GetCurrentNode(), CbmBinnedHitReader::Instance("sts"), stationPath.begin(), stationPath.end(), geoPath);
}

void CbmBinnedGeoReader::ReadRich()
{
   list<const char*> stationPath = { "rich" };
   list<const char*> geoPath = { "rich_smallprototype", "Box", "Gas", "PmtContainer", "Pmt", "pmt_pixel" };
   gGeoManager->cd("/cave_1");
   SearchStation(gGeoManager->GetCurrentNode(), CbmBinnedHitReader::Instance("rich"), stationPath.begin(), stationPath.end(), geoPath);
}

void CbmBinnedGeoReader::ReadMuch()
{
   list<const char*> stationPath = { "much", "muchstation", "layer" };
   list<const char*> geoPath = { "active" };
   gGeoManager->cd("/cave_1");
   SearchStation(gGeoManager->GetCurrentNode(), CbmBinnedHitReader::Instance("much"), stationPath.begin(), stationPath.end(), geoPath);
}

void CbmBinnedGeoReader::ReadTrd()
{
   list<const char*> stationPath = { "trd", "layer" };
   list<const char*> geoPath = { "module", "padcopper" };
   gGeoManager->cd("/cave_1");
   SearchStation(gGeoManager->GetCurrentNode(), CbmBinnedHitReader::Instance("trd"), stationPath.begin(), stationPath.end(), geoPath);
}

void CbmBinnedGeoReader::ReadTof()
{
   list<const char*> stationPath = { "tof" };
   list<const char*> geoPath = { "module", "gas_box", "counter"};
   gGeoManager->cd("/cave_1");
   SearchStation(gGeoManager->GetCurrentNode(), CbmBinnedHitReader::Instance("tof"), stationPath.begin(), stationPath.end(), geoPath, true);
}

void CbmBinnedGeoReader::ReadTarget()
{
   gGeoManager->cd("/cave_1");
   list<TGeoNode*> pipeNodes;
   FindGeoChild(gGeoManager->GetCurrentNode(), "pipe", pipeNodes);
   
   for (list<TGeoNode*>::iterator i = pipeNodes.begin(); i != pipeNodes.end(); ++i)
   {
      TGeoNode* pipeNode = *i;
      fNavigator->CdDown(pipeNode);
      list<TGeoNode*> pipeVacNodes;
      FindGeoChild(pipeNode, "pipevac", pipeVacNodes);
      
      for (list<TGeoNode*>::iterator j = pipeVacNodes.begin(); j != pipeVacNodes.end(); ++j)
      {
         TGeoNode* pipeVacNode = *j;
         fNavigator->CdDown(pipeVacNode);
         list<TGeoNode*> targetNodes;
         FindGeoChild(pipeVacNode, "target", targetNodes);
         
         for (list<TGeoNode*>::iterator k = targetNodes.begin(); k != targetNodes.end(); ++k)
         {
            TGeoNode* targetNode = *k;
            fNavigator->CdDown(targetNode);
            Double_t localCoords[3] = { 0, 0, 0 };
            Double_t globalCoords[3];
            gGeoManager->LocalToMaster(localCoords, globalCoords);
            Double_t targetX = globalCoords[0];
            Double_t targetY = globalCoords[1];
            Double_t targetZ = globalCoords[2];
            fNavigator->CdUp();
         }
         
         fNavigator->CdUp();
      }
      
      fNavigator->CdUp();
   }
}
