/** @file CbmClusterFinderSimple.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 16.06.2014
 **/

// --- Include class header
#include <reco/CbmStsClusterFinderSimple.h>

// --- Includes from ROOT
#include "TClonesArray.h"

// --- Includes from FAIRROOT
#include "FairLogger.h"

// --- Includes from CBMROOT
#include "CbmStsAddress.h"
#include "CbmStsDigi.h"
#include "CbmStsCluster.h"

// --- Includes from STS
#include "CbmStsModule.h"

// -----   Constructor   ---------------------------------------------------
CbmStsClusterFinderSimple::CbmStsClusterFinderSimple() : TObject(),
    fClusters(NULL)
  , fGap (0)
  , fNofClustersWithGap (0)
{
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmStsClusterFinderSimple::~CbmStsClusterFinderSimple() {
}
// -------------------------------------------------------------------------


// -----   Cluster finding   -----------------------------------------------
Int_t CbmStsClusterFinderSimple::FindClusters(CbmStsModule* module) {

    // --- Counter and indizes
    Int_t nClusters = 0;
    Int_t index = -1;       // dummy
    Int_t clusterStart = -1;
    Int_t clusterEnd   = -1;

    // --- First channel for back side
    Int_t channelHalf = module->GetNofChannels() / 2;

    map<Int_t, pair<CbmStsDigi*, Int_t> >* digiMap = module->GetDigiMap();
    map<Int_t, pair<CbmStsDigi*, Int_t> >::iterator digiIt;

    // --- Loop over digis in module
    for (digiIt = digiMap->begin(); digiIt != digiMap->end(); digiIt++) {
    	Int_t channel = digiIt->first;  // current channel

    	// --- No cluster yet; start one with the first digi
    	if ( clusterStart == -1 ) {
    		clusterStart = channel;
    		clusterEnd   = channel;
    	}

    	// --- Existing cluster: check whether adjacent
    	else {          // current cluster is there

    		// --- Neighbouring channel; add to cluster. Avoid clustering channels
    		// --- on different sensor sides.
    		if ( channel == clusterEnd + 1  &&  channel != channelHalf )
    			clusterEnd = channel;

    		// --- Not neighbouring; close old cluster and start new one
    		else {
     			module->CreateCluster(clusterStart, clusterEnd, fClusters);
    			nClusters++;
    			clusterStart = channel;
    			clusterEnd   = channel;
    		}  //? not neighbouring channel

    	} //? current cluster exists

    }  //# digis in module

    // --- Create last cluster
    module->CreateCluster(clusterStart, clusterEnd, fClusters);
    nClusters++;

   return nClusters;
}
// -------------------------------------------------------------------------





ClassImp(CbmStsClusterFinderSimple)

