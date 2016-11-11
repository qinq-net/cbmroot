/** @file CbmStsClusterFinderReal.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 19.10.2016
 **/

#include "CbmStsClusterFinderReal.h"

#include <cassert>
#include "TClonesArray.h"
#include "CbmEvent.h"
#include "CbmMatch.h"
#include "CbmStsModule.h"


// -----   Constructor   ----------------------------------------------------
CbmStsClusterFinderReal::CbmStsClusterFinderReal(TClonesArray* clusterArray)
	: CbmStsClusterFinder(clusterArray) {
	fName  = "StsClusterFinder";
	fTitle = "Real";
}
// ---------------------------------------------------------------------------


// -----   Cluster finding algorithm   ---------------------------------------
Int_t CbmStsClusterFinderReal::FindClusters(CbmStsModule* module,
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

  	// Neighbouring channel ? Avoid clustering channels on different sensor sides.
  	if ( channel == clusterEnd + 1  && channel != channelHalf ) {
  		clusterEnd = channel;
  	} //? neighbouring channel

  	// Not neighbouring channel: close cluster and start a new one
  	else {
  		CreateCluster(clusterStart, clusterEnd, module, event);
  		nClusters++;
  		clusterStart = channel;
  		clusterEnd   = channel;
  	}  //? not neighbouring channel

  	digiIt++;
  } //# digis in module

  // Create last cluster
  CreateCluster(clusterStart, clusterEnd, module, event);
  nClusters++;

  return nClusters;
}
// ---------------------------------------------------------------------------



// -----   Create a cluster   ------------------------------------------------
void CbmStsClusterFinderReal::CreateCluster(Int_t clusterStart,
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


ClassImp(CbmStsClusterFinderReal)