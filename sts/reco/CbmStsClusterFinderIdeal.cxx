/** @file CbmStsClusterFinderIdeal.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 17.10.2016
 **/

#include "CbmStsClusterFinderIdeal.h"

#include "TClonesArray.h"
#include "CbmEvent.h"
#include "CbmMatch.h"
#include "CbmStsModule.h"


// -----   Constructor   ----------------------------------------------------
CbmStsClusterFinderIdeal::CbmStsClusterFinderIdeal(TClonesArray* clusterArray)
	: CbmStsClusterFinder(clusterArray) {
	fName  = "StsClusterFinder";
	fTitle = "Ideal";
}
// ---------------------------------------------------------------------------


// -----   Cluster finding algorithm   ---------------------------------------
Int_t CbmStsClusterFinderIdeal::FindClusters(CbmStsModule* module,
		                                         CbmEvent* event) {

	// --- Counter
	Int_t nClusters = 0;

	// --- Map from CbmLink to associated cluster. This holds temporarily
	// --- the created clusters, which will be written to the array at the
	// --- end of the method
	std::map<CbmLink, CbmStsCluster*> linkMap;

  // --- First channel for back side. The first half of the module channels
	// --- belong to the front side of the sensors, the second half to the
	// --- back side. This logic applies for double-sided sensors only.
  Int_t channelHalf = module->GetNofChannels() / 2;

  // --- Map of digis from the module and its iterator
  // --- The map index is the channel number, the map value is a pair
  // --- of CbmStsDigi* and its index in the TClonesArray.
  auto digiMap = module->GetDigiMap();
  if ( digiMap->size() == 0 ) return 0;  // no digis to process
  auto digiIt  = digiMap->begin();

  // --- Loop over front-side digis in map (first half of module channels)
  while ( digiIt != digiMap->end() && digiIt->first < channelHalf ) {
  	CbmStsDigi* digi = digiIt->second.first;
  	if ( ! digi->GetMatch() ) {
  		LOG(FATAL) << GetName() << ": no match for digi object!"
  				       << FairLogger::endl;
  		return -1;
  	} //? match object present
  	const CbmLink& link = digi->GetMatch()->GetMatchedLink();

  	// --- If it is the first occurrence of the StsPoint, create a new cluster
  	if ( linkMap.find(link) == linkMap.end() ) {
  		Int_t index = fClusters->GetEntriesFast();
  		CbmStsCluster* cluster = new ((*fClusters)[index]) CbmStsCluster();
  		cluster->SetAddress(module->GetAddress());
   		if ( event ) event->AddData(kStsCluster, index);
    	linkMap[link] = cluster;
  		nClusters++;
  	}

  	// --- Add the digi to the cluster
		linkMap[link]->AddDigi(digiIt->second.second); // argument is the index

  	// --- Increment iterator
  	digiIt++;
  }  //# front-side digis in map

  // --- Now do the same for the back side. Note that the digis in the map
  // --- are sorted w.r.t. channel number.
  linkMap.clear();  // first clear link map not to cluster front with back side
  while ( digiIt != digiMap->end() ) {
  	CbmStsDigi* digi = digiIt->second.first;
  	if ( ! digi->GetMatch() ) {
  		LOG(FATAL) << GetName() << ": no match for digi object!"
  				       << FairLogger::endl;
  		return -1;
  	} //? match object present
  	const CbmLink& link = digi->GetMatch()->GetMatchedLink();

  	// --- If it is the first occurrence of the StsPoint, create a new cluster
  	if ( linkMap.find(link) == linkMap.end() ) {
  		Int_t index = fClusters->GetEntriesFast();
  		CbmStsCluster* cluster = new ((*fClusters)[index]) CbmStsCluster();
   		cluster->SetAddress(module->GetAddress());
     	if ( event ) event->AddData(kStsCluster, index);
  		linkMap[link] = cluster;
  		nClusters++;
  	}

  	// --- Add the digi to the cluster
		linkMap[link]->AddDigi(digiIt->second.second); // argument is the index

  	// --- Increment iterator
  	digiIt++;
  }  //# back-side digis in map

  return nClusters;
}
// ---------------------------------------------------------------------------


ClassImp(CbmStsClusterFinderIdeal)
