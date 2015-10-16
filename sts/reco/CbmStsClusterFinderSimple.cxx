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


// -----   Cluster finding Simple  -----------------------------------------
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

// -----   Time based Cluster finding  -------------------------------------
Int_t CbmStsClusterFinderSimple::FindClustersTb(CbmStsModule* module) {
	Int_t nDigis = module->GetNofDigisTb();
	module->StartClusteringTb();
	Int_t nClusters = 0;
	Double_t clusterTime = 0.;
	Double_t clusterCharge(0.), sum2(0.), sum3(0.);
	Int_t prevCharge(0), maxCharge(0), prevChannel(0), clusterSide(-1);
	vector<Int_t> digiArray;
	Int_t channel_(-1), index_(0), charge_(0);
	Double_t time_ = 0.;
	// --- Loop over digis in module ---
	for ( ; nDigis >= 0; ) {
		// --- If no open cluster ---
		if ( clusterSide < 0 ) {
			// --- If digi found in module -> open new cluster ---
			if ( module->GetNextDigiTb(channel_, time_, index_, charge_) ) {
				digiArray.clear();
				clusterSide = channel_ < 1024 ? 0 : 1;
				clusterTime = time_;
				prevCharge = maxCharge = clusterCharge = sum2 = sum3 = charge_;
				sum2 *= channel_;
				sum3 *= channel_;
				sum3 *= channel_;
				prevChannel = channel_;
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
				if ( clusterCharge > 0 ) {
					sum2 /= clusterCharge;
					sum3 /= clusterCharge;
				}
				Bool_t side = clusterSide ? 1 : 0;
				clusterSide = -1;
				module->CreateClusterTb(digiArray, clusterCharge, sum2, sum3,
											meanClusterTime, side, fClusters);
				nClusters++;
				continue;
			}
			// --- If incorrect charge -> close cluster ---
//			if ( (charge_ < maxCharge) && (charge_ > prevCharge) ) {
//				//... close cluster
//				if ( clusterCharge > 0 ) {
//					sum2 /= clusterCharge;
//					sum3 /= clusterCharge;
//				}
//				Bool_t side = clusterSide ? 1 : 0;
//				clusterSide = -1;
//				module->CreateClusterTb(digiArray, clusterCharge, sum2, sum3,
//											meanClusterTime, side, fClusters);
//				nClusters++;
//				continue;
//			}
			// --- ELSE add digi to cluster ---
			if ( charge_ > maxCharge ) maxCharge = charge_;
			prevCharge = charge_;
			clusterTime += time_;
			prevChannel = channel_;
			clusterCharge += charge_;
			sum2 += charge_ * channel_;
			sum3 += charge_ * channel_ * channel_;
			digiArray.push_back(index_);
			nDigis--;
			module->DeactivateDigiTb();
		}
		// --- ELSE (digi not found, cluster is open) -> close cluster ---
		else {
			if ( digiArray.size() ) {
				Double_t meanClusterTime = clusterTime / (Double_t)digiArray.size();
				//... close cluster
				if ( clusterCharge > 0 ) {
					sum2 /= clusterCharge;
					sum3 /= clusterCharge;
				}
				Bool_t side = clusterSide ? 1 : 0;
				clusterSide = -1;
				module->CreateClusterTb(digiArray, clusterCharge, sum2, sum3,
											meanClusterTime, side, fClusters);
				nClusters++;
			}
			continue;
		}
	}
	return nClusters;
}



ClassImp(CbmStsClusterFinderSimple)

