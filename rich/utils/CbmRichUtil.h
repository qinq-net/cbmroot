
#ifndef RICH_CbmRichUtil
#define RICH_CbmRichUtil

#include "TObject.h"
#include "TH2.h"
#include "TCanvas.h"
#include <string>
#include "FairLogger.h"
#include "TClonesArray.h"
#include "FairRootManager.h"
#include "CbmGlobalTrack.h"
#include "FairTrackParam.h"

using namespace std;

class CbmRichUtil {
    
public:
    
	static Double_t GetRingTrackDistance(Int_t globalTrackId)
	{
		Double_t errorValue = 999.;
		FairRootManager* ioman = FairRootManager::Instance();
		if (ioman == NULL) return errorValue;
		// Do we really need static here, depends on ioman->GetObject() method
		static TClonesArray* globalTracks = NULL;
		static TClonesArray* richRings = NULL;
		static TClonesArray* richProjections = NULL;

		if (globalTracks == NULL || richRings == NULL || richProjections == NULL) {
			//cout << "globalTracks == NULL || richRings == NULL || richProjections == NULL" << endl;
			globalTracks = (TClonesArray*) ioman->GetObject("GlobalTrack");
			richRings = (TClonesArray*) ioman->GetObject("RichRing");
			richProjections = (TClonesArray*) ioman->GetObject("RichProjection");
		} else {
			//cout << "globalTracks, richRings, richProjections NOT NULL" << endl;
		}

		if (globalTracks == NULL || richRings == NULL || richProjections == NULL) {
			LOG(ERROR) << "CbmRichUtil::GetRingTrackDistance globalTracks, richRings, richProjections NOT INITIALIZED" << endl;
			return errorValue;
		}

		const CbmGlobalTrack* globalTrack = static_cast<const CbmGlobalTrack*>(globalTracks->At(globalTrackId));
		if (globalTrack == NULL) return errorValue;

		Int_t stsId = globalTrack->GetStsTrackIndex();
		if (stsId < 0) return errorValue;

		const FairTrackParam* pTrack = static_cast<const FairTrackParam*>(richProjections->At(stsId));
		if (pTrack == NULL) return errorValue;

		if (pTrack->GetX() == 0 && pTrack->GetY() == 0) return errorValue;

		Int_t richId = globalTrack->GetRichRingIndex();
		if (richId < 0) return errorValue;

		const CbmRichRing* richRing = static_cast<const CbmRichRing*>(richRings->At(richId));
		if (richRing == NULL) return errorValue;

		Double_t xRing = richRing->GetCenterX();
		Double_t yRing = richRing->GetCenterY();
		Double_t dist = TMath::Sqrt( (richRing->GetCenterX() - pTrack->GetX()) * (richRing->GetCenterX() - pTrack->GetX()) +
				(richRing->GetCenterY() - pTrack->GetY()) * (richRing->GetCenterY() - pTrack->GetY()) );

		return dist;
	}
    
};

#endif
