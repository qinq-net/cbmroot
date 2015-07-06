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

    // --- Counter
    Int_t nClusters = 0;
    Int_t index = -1;  // dummy

   // --- Loop over module channels
    Int_t clusterStart = -1;

    if (!fGap){// if no gap in cluster allowed
	for (Int_t channel = 0; channel < module->GetNofChannels(); channel++) {
	    if ( module->GetDigi(channel, index) ) {   // channel is active
		if ( clusterStart == -1 ) clusterStart = channel; // start new cluster
	    } else {  // channel is inactive
		if (clusterStart != -1)  {   // stop current cluster
		    module->CreateCluster(clusterStart, channel-1, fClusters);
		    nClusters++;
		    clusterStart = -1;  // delete current cluster
		}
	    }
	}// Loop over module channels
    } else {//if 1-strip gap is allowed in cluster
    Bool_t gap = 0;
    //gap only at dead channel
    set <Int_t> deadChannels = module->GetSetOfDeadChannels();
	 for (Int_t channel = 0; channel < module->GetNofChannels(); channel++) {
	    if ( module->GetDigi(channel, index) ) {   // channel is active
		if ( clusterStart == -1 ) {
		    clusterStart = channel; // start new cluster
		}
	    } else if (clusterStart != -1) {   // channel is inactive and cluster started
		if (deadChannels.find(channel) != deadChannels.end() && !gap) { // there was no gap yet and current channel is dead
		    gap = 1;
		} else {   // stop current cluster
		    if (module->GetDigi(channel-1, index)){ // previous channel is active
			module->CreateCluster(clusterStart, channel-1, fClusters);
			if (gap) fNofClustersWithGap ++;
		    }
		    else if (gap) {
			module->CreateCluster(clusterStart, channel-2, fClusters);
		    }
		    nClusters++;
		    clusterStart = -1;  // reload for next possible cluster
		    gap = 0;
		}
	    }
	}// Loop over module channels
/*//gap anywhere
    for (Int_t channel = 0; channel < module->GetNofChannels(); channel++) {
	    if ( module->GetDigi(channel, index) ) {   // channel is active
		if ( clusterStart == -1 ) {
		    clusterStart = channel; // start new cluster
		}

	    } else if (clusterStart != -1) {   // channel is inactive and cluster started
		if (!gap) { // there was no gap yet
		    gap = 1;
		} else {   // stop current cluster
		    if (module->GetDigi (channel-1, index)){ // previous channel is active
			module->CreateCluster(clusterStart, channel-1, fClusters);
			if (gap) fNofClustersWithGap ++;
			}
		    else module->CreateCluster(clusterStart, channel-2, fClusters);
		    nClusters++;
		    clusterStart = -1;  // reload for next possible cluster
		    gap = 0;
		}
	    }
	}// Loop over module channels
	*/
    }   
    return nClusters;
}
// -------------------------------------------------------------------------





ClassImp(CbmStsClusterFinderSimple)

