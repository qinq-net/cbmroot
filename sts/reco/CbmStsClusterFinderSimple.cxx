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
CbmStsClusterFinderSimple::CbmStsClusterFinderSimple(Int_t finderModel, Int_t algorithm, Int_t eLossModel) : TObject(),
    fClusters(NULL)
  , fNofClustersWithGap (0)
  , fNofSplittedClusters (0)
  , fFinderModel(finderModel)			    
  , fAlgorithm(algorithm)			    
  , fELossModel(eLossModel)			    
{
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmStsClusterFinderSimple::~CbmStsClusterFinderSimple() {
}
// -------------------------------------------------------------------------


// -----   Cluster finding   -----------------------------------------------
Int_t CbmStsClusterFinderSimple::FindClustersSimple(CbmStsModule* module) {

    // --- Counter and indizes
    Int_t nClusters = 0;
    Int_t index = -1;       // dummy
    Int_t clusterStart = -1;
    Int_t clusterEnd   = -1;

    // --- First channel for back side
    Int_t channelHalf = module->GetNofChannels() / 2;

    map<Int_t, pair<CbmStsDigi*, Int_t> >* digiMap = module->GetDigiMap();
    map<Int_t, pair<CbmStsDigi*, Int_t> >::iterator digiIt;

    Bool_t gap = 0;
    Int_t mcIndex = -1;

    switch( fFinderModel ){

	// --- Ideal model: collect neighboring strips which correspond to one particle
	// TODO implement gaps in clusters: in case of non-ideal digitization
	case 0:

	    // --- Loop over digis in module
	    for (digiIt = digiMap->begin(); digiIt != digiMap->end(); digiIt++) {
		Int_t channel = digiIt->first;  // current channel
		CbmMatch * digiMatch = get<0>(digiIt->second) -> GetMatch();

		// --- Avoid overlaped particles in a digi
		if (digiMatch -> GetNofLinks() != 1) { 
		    LOG(DEBUG4) << "IdealClusterFinder: two particles produce one digi - ignore the digi" << FairLogger::endl;
		    continue;
		} 
		// --- No cluster yet; start one with the first digi
		if ( clusterStart == -1 ) {
		    mcIndex = digiMatch -> GetLink(0).GetIndex();
		    clusterStart = channel;
		    clusterEnd   = channel;
		}

		// --- Existing cluster: check whether adjacent
		else {          // current cluster is there

		    // --- Neighbouring channel; add to cluster. Avoid clustering channels
		    // --- on different sensor sides.
		    if ( channel == clusterEnd + 1  &&  channel != channelHalf){
			if (digiMatch -> GetLink(0).GetIndex() == mcIndex) clusterEnd = channel;
			// --- Digi from other particle: close old cluster and start new one
			else { 			
			    mcIndex = digiMatch -> GetLink(0).GetIndex();
			    module->CreateCluster(clusterStart, clusterEnd, fClusters, fAlgorithm, fELossModel);
			    nClusters++;
			    fNofSplittedClusters++;
			    clusterStart = channel;
			    clusterEnd   = channel;
			}
		}
		// --- Not neighbouring; close old cluster and start new one
		else {
		    mcIndex = digiMatch -> GetLink(0).GetIndex();
		    module->CreateCluster(clusterStart, clusterEnd, fClusters, fAlgorithm, fELossModel);
		    nClusters++;
		    clusterStart = channel;
		    clusterEnd   = channel;
		}  //? not neighbouring channel

		} //? current cluster exists

	    }  //# digis in module
	    break;

	// --- Simple model: combine only neighboring strips
	case 1:

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
			module->CreateCluster(clusterStart, clusterEnd, fClusters, fAlgorithm, fELossModel);
			nClusters++;
			clusterStart = channel;
			clusterEnd   = channel;
		    }  //? not neighbouring channel

		} //? current cluster exists

	    }  //# digis in module
	    break;

	// --- Simple model with gap: allow 1-strip gap in cluster if gap-strip is dead
	case 2:

	    set <Int_t> deadChannels = module->GetSetOfDeadChannels();
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
		    else if ( channel == (clusterEnd + 2) && deadChannels.find(channel - 1) != deadChannels.end() && !gap ){
			clusterEnd = channel;
			gap = 1;
		    }
		    // --- Not neighbouring; close old cluster and start new one
		    else {
			module->CreateCluster(clusterStart, clusterEnd, fClusters, fAlgorithm, fELossModel);
			if (gap) fNofClustersWithGap ++;
			nClusters++;
			clusterStart = channel;
			clusterEnd   = channel;
			gap = 0;
		    }  //? not neighbouring channel

		} //? current cluster exists

	    }  //# digis in module
	break;
    }


    // --- Create last cluster
    module->CreateCluster(clusterStart, clusterEnd, fClusters, fAlgorithm, fELossModel);
    if (gap) fNofClustersWithGap ++;
    nClusters++;

   return nClusters;
}
// -------------------------------------------------------------------------

// -----   Time based Cluster finding  -------------------------------------
Int_t CbmStsClusterFinderSimple::FindClustersTb(CbmStsModule* module) {
	Int_t nDigis = module->GetNofDigisTb();
	module->StartClusteringTb();
	Int_t nClusters = 0;
	Double_t clusterTime(0.), clusterCharge(0.);
	Int_t prevCharge(0), maxCharge(0), prevChannel(0);
	Bool_t cluster(kFALSE);
	vector<Int_t> digiArray;
	Int_t channel_(-1), index_(0), charge_(0);
	Double_t time_ = 0.;
	Int_t firstChannel;
	// --- Loop over digis in module ---
	for ( ; nDigis >= 0; ) {
		// --- If no open cluster ---
		if ( !cluster ) {
			// --- If digi found in module -> open new cluster ---
			if ( module->GetNextDigiTb(channel_, time_, index_, charge_) ) {
				digiArray.clear();
				cluster = kTRUE;
				clusterTime = time_;
				prevCharge = maxCharge = clusterCharge = charge_;
				prevChannel = channel_;
				firstChannel = channel_;
				digiArray.push_back(index_);
				nDigis--;
				module->DeactivateDigiTb();
			}
			// --- Digi not found in module, no open cluster -> finish clustering ---
			else return nClusters;
		}
		// --- If next digi found in module, cluster is open -> read next digi ---
		if ( module->GetNextDigiTb(channel_, time_, index_, charge_) ) {
			Double_t meanClusterTime = clusterTime / (Double_t)digiArray.size();
			// --- If incorrect time or non-next channel -> get next digi ---
			if ( (fabs(time_ - meanClusterTime) > module->GetDeadTimeTb()) ||
					(channel_ <= prevChannel) ) continue;
			// --- If channel number is bigger than next channel -> close cluster ---
			if ( channel_ > (prevChannel + 1) ) {
				//... close cluster
				cluster = kFALSE;
				module->CreateClusterTb(&digiArray, firstChannel, fClusters, 0);
				nClusters++;
				continue;
			}
			// --- If incorrect charge -> close cluster ---
//			if ( (charge_ < maxCharge) && (charge_ > prevCharge) ) {
//				//... close cluster
//				Bool_t side = clusterSide ? 1 : 0;
//				clusterSide = -1;
//				module->CreateClusterTb(digiArray, firstChannel, fClusters, 0);
//				nClusters++;
//				continue;
//			}
			// --- ELSE add digi to cluster ---
			if ( charge_ > maxCharge ) maxCharge = charge_;
			prevCharge = charge_;
			clusterTime += time_;
			prevChannel = channel_;
			clusterCharge += charge_;
			digiArray.push_back(index_);
			nDigis--;
			module->DeactivateDigiTb();
		}
		// --- ELSE (digi not found, cluster is open) -> close cluster ---
		else {
			if ( digiArray.size() ) {
				Double_t meanClusterTime = clusterTime / (Double_t)digiArray.size();
				//... close cluster
				cluster = kFALSE;
				module->CreateClusterTb(&digiArray, firstChannel, fClusters, 0);
				nClusters++;
			}
			continue;
		}
	}
	return nClusters;
}



ClassImp(CbmStsClusterFinderSimple)

