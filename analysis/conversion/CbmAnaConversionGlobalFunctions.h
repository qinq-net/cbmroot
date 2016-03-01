/**
 * @author S. Reinecke <reinecke@uni-wuppertal.de>
 * @since 2016
 **/

#ifndef CBM_ANA_CONVERSION_GLOBAL_FUNCTIONS
#define CBM_ANA_CONVERSION_GLOBAL_FUNCTIONS

#include "CbmMCTrack.h"
#include "CbmGlobalTrack.h"
#include "CbmRichRing.h"
#include "TMath.h"
#include "TLorentzVector.h"
#include "TClonesArray.h"
#include "CbmRichElectronIdAnn.h"

#define M2E 2.6112004954086e-7

class CbmAnaConversionGlobalFunctions{
public:

	/*
	 * Calculate ANN response for a given globaltrack index and momentum
	 */
	static Double_t ElectronANNvalue(Int_t globalTrackIndex, Double_t momentum)
	{
		FairRootManager *ioman = FairRootManager::Instance();
		if (NULL == ioman) return -2;
		TClonesArray *fGlobalTracks = (TClonesArray*) ioman->GetObject("GlobalTrack");
		if (NULL == fGlobalTracks) return -2;
		TClonesArray *fRichRings = (TClonesArray*) ioman->GetObject("RichRing");
		if (NULL == fRichRings) return -2;
		
		CbmRichElectronIdAnn *fRichElIdAnn = new CbmRichElectronIdAnn();
		fRichElIdAnn->Init();
		
		
		//if (NULL == fGlobalTracks || NULL == fRichRings) return -2;
		//CbmGlobalTrack* globalTrack = (CbmGlobalTrack*) fGlobalTracks->At(globalTrackIndex);
		const CbmGlobalTrack* globalTrack = static_cast<const CbmGlobalTrack*>(fGlobalTracks->At(globalTrackIndex));
		Int_t richId = globalTrack->GetRichRingIndex();
		if (richId < 0) return -2;
		CbmRichRing* ring = static_cast<CbmRichRing*> (fRichRings->At(richId));
		if (NULL == ring) return -2;

		Double_t ann = fRichElIdAnn->DoSelect(ring, momentum);
		return ann;
	}

	/*
	 * Checks, whether a track is an electron based on ANN output for a given globaltrack index and momentum, and for a given ANNcut value
	 */
	static Double_t IsRICHElectronANN(Int_t globalTrackIndex, Double_t momentum, Double_t ANNcut)
	{
		FairRootManager *ioman = FairRootManager::Instance();
		if (NULL == ioman) return -2;
		TClonesArray *fGlobalTracks = (TClonesArray*) ioman->GetObject("GlobalTrack");
		if (NULL == fGlobalTracks) return -2;
		TClonesArray *fRichRings = (TClonesArray*) ioman->GetObject("RichRing");
		if (NULL == fRichRings) return -2;
		
		CbmRichElectronIdAnn *fRichElIdAnn = new CbmRichElectronIdAnn();
		fRichElIdAnn->Init();
		
		
		
		//if (NULL == fGlobalTracks || NULL == fRichRings) return -2;
		//CbmGlobalTrack* globalTrack = (CbmGlobalTrack*) fGlobalTracks->At(globalTrackIndex);
		const CbmGlobalTrack* globalTrack = static_cast<const CbmGlobalTrack*>(fGlobalTracks->At(globalTrackIndex));
		Int_t richId = globalTrack->GetRichRingIndex();
		if (richId < 0) return -2;
		CbmRichRing* ring = static_cast<CbmRichRing*> (fRichRings->At(richId));
		if (NULL == ring) return -2;

		Double_t ann = fRichElIdAnn->DoSelect(ring, momentum);
		if(ann > ANNcut) return true;
		else return false;
	}

};

#endif
