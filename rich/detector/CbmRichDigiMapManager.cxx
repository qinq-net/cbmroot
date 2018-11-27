/*
 * CbmRichDigiMapManager.cxx
 *
 *  Created on: Dec 17, 2015
 *      Author: slebedev
 */

#include "CbmRichDigiMapManager.h"
#include "TGeoManager.h"
#include "FairLogger.h"
#include "TRandom.h"
#include "TGeoBBox.h"

#include <iostream>

using namespace std;

CbmRichDigiMapManager::CbmRichDigiMapManager() 
 : fPixelPathToAddressMap(),
   fPixelAddressToDataMap(),
   fPixelAddresses(),
   fPmtPathToIdMap(),
   fPmtIdToDataMap(),
   fPmtIds()
{
	Init();
}

CbmRichDigiMapManager::~CbmRichDigiMapManager() {

}

void CbmRichDigiMapManager::Init() {

	fPixelPathToAddressMap.clear();
	fPixelAddressToDataMap.clear();
	fPixelAddresses.clear();

    fPmtPathToIdMap.clear();
    fPmtIdToDataMap.clear();
    fPmtIds.clear();

	Int_t currentPixelAddress = 1;
	Int_t currentPmtId = 1;

	// Get MAPMT dimensions
	Double_t pmtHeight = 5.2;
	Double_t pmtWidth = 5.2;
	TGeoVolume* pmtVolume = gGeoManager->FindVolumeFast("pmt");
	if (pmtVolume != nullptr) {
	    const TGeoBBox* shape = (const TGeoBBox*)(pmtVolume->GetShape());
	    if (shape != nullptr) {
	        pmtHeight = 2. * shape->GetDY();
	        pmtWidth = 2. * shape->GetDX();
	    }
	}

	TGeoIterator geoIterator(gGeoManager->GetTopNode()->GetVolume());
	geoIterator.SetTopName("/cave_1");
	TGeoNode* curNode;
	// PMT plane position\rotation
	TString pixelNameStr("pmt_pixel");
	geoIterator.Reset();
	while ((curNode=geoIterator())) {
		TString nodeName(curNode->GetName());
		TString nodePath;
		if (TString(curNode->GetVolume()->GetName()).Contains(pixelNameStr)) {
			geoIterator.GetPath(nodePath);
			const TGeoMatrix* curMatrix = geoIterator.GetCurrentMatrix();
			const Double_t* curNodeTr = curMatrix->GetTranslation();
			string path = string(nodePath.Data());

			size_t pmtInd = path.find_last_of("/");
			if (string::npos == pmtInd) continue;
			string pmtPath = path.substr(0, pmtInd + 1);

			fPixelPathToAddressMap.insert(pair<string, Int_t>(path, currentPixelAddress));
			CbmRichPixelData* pixelData = new CbmRichPixelData();
			pixelData->fX = curNodeTr[0];
			pixelData->fY = curNodeTr[1];
			pixelData->fZ = curNodeTr[2];
			pixelData->fAddress = currentPixelAddress;
			fPixelAddressToDataMap.insert(pair<Int_t, CbmRichPixelData*>(pixelData->fAddress, pixelData));
			fPixelAddresses.push_back(currentPixelAddress);
			currentPixelAddress++;

			if (fPmtPathToIdMap.count(pmtPath) == 0) {
			    fPmtPathToIdMap.insert(pair<string, Int_t>(pmtPath, currentPmtId));

	            CbmRichPmtData* pmtData = new CbmRichPmtData();
	            pmtData->fId = currentPmtId;
	            pmtData->fPixelAddresses.push_back(pixelData->fAddress);
	            pmtData->fHeight = pmtHeight;
	            pmtData->fWidth = pmtWidth;
	            fPmtIdToDataMap.insert(pair<Int_t, CbmRichPmtData*>(pmtData->fId, pmtData));
	            pixelData->fPmtId = pmtData->fId;

	            fPmtIds.push_back(pmtData->fId);

			    currentPmtId++;
			} else {
			    //cout << "pmtPath old:" << pmtPath << endl;
			    Int_t pmtId = fPmtPathToIdMap[pmtPath];
			    CbmRichPmtData* pmtData = fPmtIdToDataMap[pmtId];
			    if (pmtData == nullptr || pmtId != pmtData->fId) {
			        LOG(ERROR) << "(pmtData == nullptr || pmtId != pmtData->fId) " << FairLogger::endl;
			    }
			    pmtData->fPixelAddresses.push_back(pixelData->fAddress);
			    pixelData->fPmtId = pmtData->fId;
			    if (pmtData->fPixelAddresses.size() > 64) {
			        cout << "size:" << pmtData->fPixelAddresses.size() << " pmtData->fId:" << pmtData->fId << " pmtPath:" << pmtPath << endl << " path:" << path << endl;
			    }
			}
		}
	}

	// calculate Pmt center as center of gravity of 64 pixels
	for (auto const& pmt : fPmtIdToDataMap) {
	    Int_t pmtId = pmt.first;
	    CbmRichPmtData* pmtData = pmt.second;
	    pmtData->fX = 0.;
	    pmtData->fY = 0.;
	    pmtData->fZ = 0.;
	    for (int pixelId : pmtData->fPixelAddresses) {
	        CbmRichPixelData* pixelData = fPixelAddressToDataMap[pixelId];
	        if (pixelData == nullptr) continue;
	        pmtData->fX += pixelData->fX;
	        pmtData->fY += pixelData->fY;
	        pmtData->fZ += pixelData->fZ;
	    }
	    pmtData->fX /= pmtData->fPixelAddresses.size();
	    pmtData->fY /= pmtData->fPixelAddresses.size();
	    pmtData->fZ /= pmtData->fPixelAddresses.size();
	}


	LOG(INFO) << "CbmRichDigiMapManager is initialized" << FairLogger::endl;
	LOG(INFO) << "fPixelPathToAddressMap.size() = " << fPixelPathToAddressMap.size() << FairLogger::endl;
	LOG(INFO) << "fPixelAddressToDataMap.size() = " << fPixelAddressToDataMap.size() << FairLogger::endl;

    LOG(INFO) << "fPmtPathToIdMap.size() = " << fPmtPathToIdMap.size() << FairLogger::endl;
    LOG(INFO) << "fPmtIdToDataMap.size() = " << fPmtIdToDataMap.size() << FairLogger::endl;

//    for (auto const& pmt : fPmtIdToDataMap) {
//       // cout << pmt.first << endl;
//        cout << pmt.second->ToString() << endl;
//    }
}

Int_t CbmRichDigiMapManager::GetPixelAddressByPath(const string& path)
{
	std::map<string,Int_t>::iterator it;
	it = fPixelPathToAddressMap.find(path);
	if (it == fPixelPathToAddressMap.end()) return -1;
	return it->second;
}


CbmRichPixelData* CbmRichDigiMapManager::GetPixelDataByAddress(Int_t address)
{
	std::map<Int_t,CbmRichPixelData*>::iterator it;
	it = fPixelAddressToDataMap.find(address);
	if (it == fPixelAddressToDataMap.end()) return nullptr;
	return it->second;
}

Int_t CbmRichDigiMapManager::GetRandomPixelAddress()
{
	Int_t nofPixels = fPixelAddresses.size();
	Int_t index = gRandom->Integer(nofPixels);
	return fPixelAddresses[index];
}

vector<Int_t> CbmRichDigiMapManager::GetPixelAddresses()
{
    return fPixelAddresses;
}


vector<Int_t> CbmRichDigiMapManager::GetPmtIds()
{
    return fPmtIds;
}


CbmRichPmtData* CbmRichDigiMapManager::GetPmtDataById(Int_t id)
{
    std::map<Int_t,CbmRichPmtData*>::iterator it;
    it = fPmtIdToDataMap.find(id);
    if (it == fPmtIdToDataMap.end()) return nullptr;
    return it->second;
}

vector<Int_t> CbmRichDigiMapManager::GetDirectNeighbourPixels(Int_t /*address*/)
{
	std::vector<Int_t> v;

	return v;
}

vector<Int_t> CbmRichDigiMapManager::GetDiagonalNeighbourPixels(Int_t /*address*/)
{
	std::vector<Int_t> v;

	return v;
}



