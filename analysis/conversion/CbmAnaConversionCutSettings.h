/**
 * @author S. Reinecke <reinecke@uni-wuppertal.de>
 * @since 2015
 * @based on CbmLmvmKinematicParams.h by E. Lebedeva
 **/

#ifndef CBM_ANA_CONVERSION_CUT_SETTINGS
#define CBM_ANA_CONVERSION_CUT_SETTINGS

#include "TMath.h"
#include "CbmMCTrack.h"
#include "TLorentzVector.h"

#define M2E 2.6112004954086e-7

class CbmAnaConversionCutSettings{
public:

	/*
	 * Calculate cut for opening angle between electrons
	 */
	static Double_t CalcOpeningAngleCut(Double_t pt)
	{
		Double_t cut = 0;
		cut = 1.5 - 0.5 * pt;
		
		return cut;
	}


	/*
	 * Calculate cut for chi for reconstructed momenta
	 */
	static Double_t CalcChiCut(Double_t pt)
	{
		Double_t cut = 0;
		if(pt < 0.4) {
			cut = 31. - 70.*pt;
		}
		if(pt >= 0.4) {
			cut = 3;
		}
		
		return cut;
	}

};

#endif
