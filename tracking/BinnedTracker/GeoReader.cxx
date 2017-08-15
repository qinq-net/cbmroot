/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "GeoReader.h"

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

CbmBinnedGeoReader::CbmBinnedGeoReader() : fIoman(0), fNavigator(0), fDetectorReaders()
{
   fNavigator = gGeoManager->GetCurrentNavigator();
   fDetectorReaders["sts"] = &CbmBinnedGeoReader::ReadSts;
   fDetectorReaders["rich"] = &CbmBinnedGeoReader::ReadRich;
   fDetectorReaders["much"] = &CbmBinnedGeoReader::ReadMuch;
   fDetectorReaders["trd"] = &CbmBinnedGeoReader::ReadTrd;
   fDetectorReaders["tof"] = &CbmBinnedGeoReader::ReadTof;
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
   TGeoNavigator* pNavigator = gGeoManager->GetCurrentNavigator();
   gGeoManager->cd("/cave_1");
   list<TGeoNode*> detectors;
   FindGeoChild(gGeoManager->GetCurrentNode(), name, detectors);
   ReadDetector(detectors, fDetectorReaders[name]);
}

void CbmBinnedGeoReader::ReadDetector(list<TGeoNode*>& detectorList, void (CbmBinnedGeoReader::*readDet)(TGeoNode* stsNode))
{
   for (list<TGeoNode*>::iterator i = detectorList.begin(); i != detectorList.end(); ++i)
      (this->*readDet)(*i);
}

void CbmBinnedGeoReader::SearchStation(TGeoNode* node, list<const char*>::const_iterator stationPath, list<const char*>::const_iterator stationPathEnd,
   const std::list<const char*>& geoPath)
{
   fNavigator->CdDown(node);
   const char* name = *stationPath++;
   
   if (stationPath == stationPathEnd)
      HandleStation(node, geoPath.begin(), geoPath.end());
   else
   {
      list<TGeoNode*> subNodes;
      FindGeoChild(node, name, subNodes);
   
      for (list<TGeoNode*>::iterator i = subNodes.begin(); i != subNodes.end(); ++i)
         SearchStation(*i, stationPath, stationPathEnd, geoPath);
   }
   
   fNavigator->CdUp();
}

void CbmBinnedGeoReader::HandleStation(TGeoNode* node, list<const char*>::const_iterator geoPath, list<const char*>::const_iterator geoPathEnd)
{
   fNavigator->CdDown(node);
   
   if (geoPath == geoPathEnd)
      ;
   else
   {
      const char* name = *geoPath++;
      list<TGeoNode*> subNodes;
      FindGeoChild(node, name, subNodes);
   
      for (list<TGeoNode*>::iterator i = subNodes.begin(); i != subNodes.end(); ++i)
         HandleStation(*i, geoPath, geoPathEnd);
   }
   
   fNavigator->CdUp();
}

void CbmBinnedGeoReader::ReadSts(TGeoNode* stsNode)
{
   TGeoNavigator* pNavigator = gGeoManager->GetCurrentNavigator();
   gGeoManager->cd("/cave_1");
   list<TGeoNode*> stations;
   FindGeoChild(gGeoManager->GetCurrentNode(), "Station", stations);
   
   for (list<TGeoNode*>::iterator i = stations.begin(); i != stations.end(); ++i)
   {
      TGeoNode* aStation = *i;
      list<TGeoNode*> ladders;
      FindGeoChild(gGeoManager->GetCurrentNode(), "Ladder", ladders);
   }
}

void CbmBinnedGeoReader::ReadRich(TGeoNode* richNode)
{
   
}

void CbmBinnedGeoReader::ReadMuch(TGeoNode* muchNode)
{
   
}

void CbmBinnedGeoReader::ReadTrd(TGeoNode* trdNode)
{
   
}

void CbmBinnedGeoReader::ReadTof(TGeoNode* tofNode)
{
   
}
