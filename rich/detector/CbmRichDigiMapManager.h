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
#include "CbmRichMapData.h"

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
	Int_t GetAdressByPath(const string& path);

	/*
	 * \brief Return RichMapData by digi address.
	 */
	CbmRichMapData* GetDataByAddress(Int_t address);

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
	Int_t GetRandomAddress();
    
    
    /*
     * \brief Return addresses of all pmts
     */
    vector<Int_t> GetAddresses();


public:
	virtual ~CbmRichDigiMapManager();

private:
	map<string, Int_t> fPathToAddressMap;
	map<Int_t, CbmRichMapData*> fAddressToDataMap;
	vector<Int_t> fAddresses; // vector of all  pixel addresses

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
