
#ifndef CBM_ANA_JPSI_CANDIDATE_H
#define CBM_ANA_JPSI_CANDIDATE_H

#include "TVector3.h"

class CbmAnaJpsiCandidate{
public:
	CbmAnaJpsiCandidate()
	  : fMomentum(),
		fPosition(),
	    fMass(0.),
	    fEnergy(0.),
	    fRapidity(0.),
	    fCharge(0),
	    fChi2Prim(0.),
	    fChi2sts(0.)
	{
	}


	// track parameters
	TVector3 fPosition;
	TVector3 fMomentum;
	Double_t fMass;
	Double_t fEnergy;
	Double_t fRapidity;
	Int_t fCharge;
	Double_t fChi2Prim;
	Double_t fChi2sts;

};

#endif
