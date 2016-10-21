/** @file CbmStsClusterFinderGap.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 19.10.2016
 **/

#include "CbmStsClusterFinderGap.h"

#include <cassert>
#include "TClonesArray.h"
#include "CbmEvent.h"
#include "CbmMatch.h"
#include "CbmStsModule.h"


// -----   Constructor   ----------------------------------------------------
CbmStsClusterFinderGap::CbmStsClusterFinderGap(TClonesArray* clusterArray)
	: CbmStsClusterFinder(clusterArray) {
	fName  = "StsClusterFinder";
	fTitle = "Gap";
}
// ---------------------------------------------------------------------------


// -----   Cluster finding algorithm   ---------------------------------------
Int_t CbmStsClusterFinderGap::FindClusters(CbmStsModule* module,
		                                       CbmEvent* event) {

	// --- Counter
	Int_t nClusters = 0;

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

  // --- The first digi starts a cluster
  Int_t channel = digiIt->first;
  Int_t clusterStart = channel;
  Int_t clusterEnd   = channel;

  // --- Loop over the rest of the digis
  digiIt++;
  while ( digiIt != digiMap->end() ) {
  	channel = digiIt->first;

  	Bool_t newCluster = kFALSE;
  	// Case: different sides of sensors
  	if ( clusterEnd < channelHalf && channel >= channelHalf )
  		newCluster = kTRUE;
  	// Case: separated by one active channel
  	else if ( channel == clusterEnd + 2
  			&& module->IsChannelActive(clusterEnd + 1) ) newCluster = kTRUE;
  	// Case: separated by two or more channels
  	else if ( channel > clusterEnd + 2) newCluster = kTRUE;

  	if ( newCluster ) {  // Close the old cluster and start a new one
  		CreateCluster(clusterStart, clusterEnd, module, event);
  		nClusters++;
  		clusterStart = channel;
  		clusterEnd   = channel;
  	} //? new cluster
  	else clusterEnd = channel;  // expand old cluster

  	digiIt++;
  } //# digis in module

  // Create last cluster
  CreateCluster(clusterStart, clusterEnd, module, event);

  return nClusters;
}
// ---------------------------------------------------------------------------



// -----   Create a cluster   ------------------------------------------------
void CbmStsClusterFinderGap::CreateCluster(Int_t clusterStart,
		                                       Int_t clusterEnd,
		                                       CbmStsModule* module,
		                                       CbmEvent* event) {
	Int_t clusterIndex = fClusters->GetEntriesFast();
	CbmStsCluster* cluster = new ( (*fClusters)[clusterIndex] ) CbmStsCluster();
	cluster->SetAddress(module->GetAddress());
	if ( event ) event->AddData(Cbm::kStsCluster, clusterIndex);
	Int_t digiIndex = -1;  // digi index in TClonesArray
	for (Int_t channel = clusterStart; channel <= clusterEnd; channel++) {
		CbmStsDigi* digi = module->GetDigi(channel, digiIndex);
		assert(digi);
		cluster->AddDigi(digiIndex);
	} //# channels in cluster
}
// ---------------------------------------------------------------------------


ClassImp(CbmStsClusterFinderGap)
