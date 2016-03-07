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

#include <iostream>

using namespace std;

CbmRichDigiMapManager::CbmRichDigiMapManager() 
 : fPathToAddressMap(),
   fAddressToDataMap(),
   fAddresses()
{
	Init();
}

CbmRichDigiMapManager::~CbmRichDigiMapManager() {

}

void CbmRichDigiMapManager::Init() {

	fPathToAddressMap.clear();
	fAddressToDataMap.clear();
	fAddresses.clear();
	Int_t currentAddress = 1;

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
			fPathToAddressMap.insert(pair<string, Int_t>(path, currentAddress));
			CbmRichMapData* data = new CbmRichMapData();
			data->fX = curNodeTr[0];
			data->fY = curNodeTr[1];
			data->fZ = curNodeTr[2];
			fAddressToDataMap.insert(pair<Int_t, CbmRichMapData*>(currentAddress, data));
			fAddresses.push_back(currentAddress);
			currentAddress++;
		}
	}

	LOG(INFO) << "CbmRichDigiMapManager is initialized" << FairLogger::endl;
	LOG(INFO) << "fPathToAddressMap.size() = " << fPathToAddressMap.size() << FairLogger::endl;
	LOG(INFO) << "fAddressToDataMap.size() = " << fAddressToDataMap.size() << FairLogger::endl;
}

Int_t CbmRichDigiMapManager::GetAdressByPath(const string& path)
{
	std::map<string,Int_t>::iterator it;
	it = fPathToAddressMap.find(path);
	if (it == fPathToAddressMap.end()) return -1;
	return it->second;
}


CbmRichMapData* CbmRichDigiMapManager::GetDataByAddress(Int_t address)
{
	std::map<Int_t,CbmRichMapData*>::iterator it;
	it = fAddressToDataMap.find(address);
	if (it == fAddressToDataMap.end()) return NULL;
	return it->second;
}

Int_t CbmRichDigiMapManager::GetRandomAddress()
{
	Int_t nofPixels = fAddresses.size();
	Int_t index = gRandom->Integer(nofPixels);
	return fAddresses[index];
}

vector<Int_t> CbmRichDigiMapManager::GetAddresses()
{
    return fAddresses;
}

vector<Int_t> CbmRichDigiMapManager::GetDirectNeighbourPixels(Int_t address)
{
	std::vector<Int_t> v;

	return v;
}

vector<Int_t> CbmRichDigiMapManager::GetDiagonalNeighbourPixels(Int_t address)
{
	std::vector<Int_t> v;

	return v;
}



