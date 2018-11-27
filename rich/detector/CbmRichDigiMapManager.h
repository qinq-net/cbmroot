/*
 * CbmRichDigiMap.h
 *
 *  Created on: Dec 17, 2015
 *      Author: slebedev
 */

#ifndef RICH_DETECTOR_CBMRICHDIGIMAPMANAGER_H_
#define RICH_DETECTOR_CBMRICHDIGIMAPMANAGER_H_

#include <map>
#include <iostream>
#include "CbmRichDetectorData.h"

using namespace std;

class CbmRichDigiMapManager {
private:
	CbmRichDigiMapManager();

public:
	/**
	 * Return Instance of CbmRichGeoManager.
	 */
	static CbmRichDigiMapManager& GetInstance() {
		static CbmRichDigiMapManager fInstance;
		return fInstance;
	}

	/*
	 * \brief Return digi address by path to node.
	 */
	Int_t GetPixelAddressByPath(const string& path);

	/*
	 * \brief Return CbmRichDataPixel by digi address.
	 */
	CbmRichPixelData* GetPixelDataByAddress(Int_t address);

	/*
	 * \brief Return the addresses of the direct neighbour pixels.
	 * C++11 efficient way to return vector
	 */
	vector<Int_t> GetDirectNeighbourPixels(Int_t address);

	/*
	 * \brief Return the addresses of the diagonal neighbour pixels.
	 * C++11 efficient way to return vector
	 */
	vector<Int_t> GetDiagonalNeighbourPixels(Int_t address);

	/*
	 * \brief Return random address. Needed for noise digi.
	 */
	Int_t GetRandomPixelAddress();
    
    
    /*
     * \brief Return addresses of all pixels
     */
    vector<Int_t> GetPixelAddresses();

    /*
     * \brief Return ids for all pmts
     */
    vector<Int_t> GetPmtIds();

    /*
     * \brief Return CbmRichDataPmt by id.
     */
    CbmRichPmtData* GetPmtDataById(Int_t id);

public:
	virtual ~CbmRichDigiMapManager();

private:
	map<string, Int_t> fPixelPathToAddressMap;
	map<Int_t, CbmRichPixelData*> fPixelAddressToDataMap;
	vector<Int_t> fPixelAddresses; // vector of all  pixel addresses

    map<string, Int_t> fPmtPathToIdMap;
    map<Int_t, CbmRichPmtData*> fPmtIdToDataMap;
    vector<Int_t> fPmtIds;

	/*
	 * \brief Initialize maps.
	 */
	void Init();

	/**
	* \brief Copy constructor.
	*/
	CbmRichDigiMapManager(const CbmRichDigiMapManager&);

	/**
	* \brief Assignment operator.
	*/
	CbmRichDigiMapManager& operator=(const CbmRichDigiMapManager&);
};

#endif /* RICH_DETECTOR_CBMRICHDIGIMAPMANAGER_H_ */
