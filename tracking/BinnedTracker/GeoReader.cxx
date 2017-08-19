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

using namespace std;

CbmBinnedGeoReader* CbmBinnedGeoReader::fInstance = 0;

CbmBinnedGeoReader* CbmBinnedGeoReader::Instance()
{
   if (0 != fInstance)
      return fInstance;
   
   FairRootManager* ioman = FairRootManager::Instance();
   
   if (0 == ioman)
      return 0;
   
   fInstance = new CbmBinnedGeoReader;
   fInstance->fIoman = ioman;
   return fInstance;
}

CbmBinnedGeoReader::CbmBinnedGeoReader() : fIoman(0), fNavigator(0), fDetectorReaders(), fTracker(0)
{
   fNavigator = gGeoManager->GetCurrentNavigator();
   fDetectorReaders["sts"] = &CbmBinnedGeoReader::ReadSts;
   fDetectorReaders["rich"] = &CbmBinnedGeoReader::ReadRich;
   fDetectorReaders["much"] = &CbmBinnedGeoReader::ReadMuch;
   fDetectorReaders["trd"] = &CbmBinnedGeoReader::ReadTrd;
   fDetectorReaders["tof"] = &CbmBinnedGeoReader::ReadTof;
   
   CbmBinnedHitReader::AddReader("sts", static_cast<TClonesArray*> (fIoman->GetObject("StsHit")));
   CbmBinnedHitReader::AddReader("rich", static_cast<TClonesArray*> (fIoman->GetObject("RichHit")));
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

void CbmBinnedGeoReader::Read()
{
   ReadDetector("sts");
   ReadDetector("trd");
   ReadDetector("much");
   ReadDetector("tof");
   ReadDetector("rich");
}

void CbmBinnedGeoReader::ReadDetector(const char* name)
{
   (this->*fDetectorReaders[name])();
}

static const int gNofZbins = 5;
static const int gNofYbins = 20;
static const int gNofXbins = 20;
static const int gNofTbins = 5;

void CbmBinnedGeoReader::SearchStation(TGeoNode* node, CbmBinnedHitReader* hitReader, list<const char*>::const_iterator stationPath,
   list<const char*>::const_iterator stationPathEnd, const std::list<const char*>& geoPath, bool is4d)
{   
   if (stationPath == stationPathEnd)
   {
      CbmBinnedStation* station = 0;
      
      if (is4d)
         station = new CbmBinned4DStation(gNofZbins, gNofYbins, gNofXbins, gNofTbins);
      else
         station = new CbmBinned3DStation(gNofYbins, gNofXbins, gNofTbins);
      
      fTracker->AddStation(station);
      hitReader->AddStation(station);
      HandleStation(node, station, geoPath.begin(), geoPath.end());
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
      }
   }
   
   fNavigator->CdUp();
}

void CbmBinnedGeoReader::HandleStation(TGeoNode* node, CbmBinnedStation* station, list<const char*>::const_iterator geoPath, list<const char*>::const_iterator geoPathEnd)
{   
   if (geoPath == geoPathEnd)
   {
      Double_t left = 10000;
      Double_t right = -10000;
      Double_t top = -10000;
      Double_t bottom = 10000;
      HandleActive(node, left, right, top, bottom);
   }
   else
   {
      const char* name = *geoPath++;
      list<TGeoNode*> subNodes;
      FindGeoChild(node, name, subNodes);
   
      for (list<TGeoNode*>::iterator i = subNodes.begin(); i != subNodes.end(); ++i)
      {
         fNavigator->CdDown(*i);
         HandleStation(*i, station, geoPath, geoPathEnd);
      }
   }
   
   fNavigator->CdUp();
}

void CbmBinnedGeoReader::HandleActive(TGeoNode* node, Double_t& left, Double_t& right, Double_t& top, Double_t& bottom)
{
   TGeoBBox* pBox = static_cast<TGeoBBox*> (node->GetVolume()->GetShape());
   pBox->ComputeBBox();
   
   for (int i = -1; i <= 1; i += 2)
   {
      for (int j = -1; j <= 1; j += 2)
      {
         for (int k = -1; k <= 1; k += 2)
         {
            Double_t localCoords[3] = {i * pBox->GetDX(), j * pBox->GetDY(), k * pBox->GetDZ() };
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
   list<const char*> stationPath = { "rich", "rich_smallprototype" };
   list<const char*> geoPath = { "Box", "Gas", "PmtContainer", "Pmt" };
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
