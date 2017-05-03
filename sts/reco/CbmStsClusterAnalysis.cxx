/** @file CbmStsClusterAnalysis.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 18.10.2016
 **/

#include "CbmStsClusterAnalysis.h"

#include <cassert>
#include "TClonesArray.h"
#include "CbmStsAddress.h"
#include "CbmStsCluster.h"
#include "CbmStsModule.h"

// -----   Algorithm   ----------------------------------------------------
void CbmStsClusterAnalysis::Analyze(CbmStsCluster* cluster,
		CbmStsModule* module,
		TClonesArray* digiArray) {

	assert(cluster);
	assert(module);
	assert(digiArray);

	// --- Uncertainties of the charge measurements
	Double_t eNoiseSq = module->GetNoise() * module->GetNoise();
	Double_t chargePerAdc = module->GetDynamicRange() /
			                Double_t(module->GetNofAdcChannels());
	Double_t eDigitSq = chargePerAdc * chargePerAdc / 12.;


	// --- For 1-strip clusters
	if ( cluster->GetNofDigis() == 1 ) {

		Int_t index = cluster->GetDigi(0);
		CbmStsDigi* digi = dynamic_cast<CbmStsDigi*> ( digiArray->At(index) );
		assert(digi);
		UInt_t address = digi->GetAddress();
		Double_t x = Double_t(CbmStsAddress::GetElementId(address, kStsChannel));
		Double_t time = digi->GetTime();
		Double_t timeError = module->GetTimeResolution();
		Double_t charge = module->AdcToCharge(digi->GetCharge());
		Double_t xError = 1. / sqrt(24.);

		cluster->SetAddress(module->GetAddress());
		cluster->SetProperties(charge, x, xError, time, timeError);
		cluster->SetSize(1);

	}  //? 1-strip clusters


	// --- For 2-strip clusters
	else if ( cluster->GetNofDigis() == 2 )  {

		Int_t index1 = cluster->GetDigi(0);
		Int_t index2 = cluster->GetDigi(1);
		CbmStsDigi* digi1 = dynamic_cast<CbmStsDigi*> ( digiArray->At(index1) );
		CbmStsDigi* digi2 = dynamic_cast<CbmStsDigi*> ( digiArray->At(index2) );
		assert(digi1);
		assert(digi2);

		UInt_t address1 = digi1->GetAddress();
		UInt_t address2 = digi2->GetAddress();
		Int_t chan1 = CbmStsAddress::GetElementId(address1, kStsChannel);
        Int_t chan2 = CbmStsAddress::GetElementId(address2, kStsChannel);
        assert( chan2 == chan1 + 1 ||
                chan2 == chan1 - module->GetNofChannels()/2 + 1);

        // Channel positions and charge
        Double_t x1 = Double_t(chan1);
        Double_t q1 = module->AdcToCharge(digi1->GetCharge());
        Double_t x2 = Double_t(chan2);
        Double_t q2 = module->AdcToCharge(digi2->GetCharge());

        // Periodic position for clusters round the edge
        if ( chan1 > chan2 ) x1 -= Double_t(module->GetNofChannels() / 2);

		// Uncertainties of the charge measurements
        Double_t width1 = fPhysics->LandauWidth(q1);
		Double_t eq1sq = width1 * width1 + eNoiseSq + eDigitSq;
		Double_t width2 = fPhysics->LandauWidth(q2);
		Double_t eq2sq = width2 * width2 + eNoiseSq + eDigitSq;

		// Cluster time
		Double_t time = 0.5 * ( digi1->GetTime() + digi2->GetTime());
		Double_t timeError = module->GetTimeResolution() * 0.70710678; // 1/sqrt(2)

		// Cluster position
		// See corresponding software note.
		Double_t x = x1 + 0.5 + ( q2 - q1 ) / 3. /  TMath::Max(q1, q2);

		// Correct negative position for clusters around the edge
		if ( x < -0.5 ) x += Double_t(module->GetNofChannels() / 2);

		// Uncertainty on cluster position. See software note.
		Double_t ex0sq = 0.;    // error for ideal charge measurements
		Double_t ex1sq = 0.;    // error from first charge
		Double_t ex2sq = 0.;    // error from second charge
		if ( q1 < q2 ) {
			ex0sq = (q2 - q1) * (q2 -q1) / q2 / q2 / 72.;
			ex1sq = eq1sq / q2 / q2 / 9.;
			ex2sq = eq2sq * q1 * q1 / q2 / q2 / q2 / q2 / 9.;
		}
		else {
			ex0sq = (q2 - q1) * (q2-q1) / q1 / q1 / 72.;
			ex1sq = eq1sq * q2 * q2 / q1 / q1 / q1 / q1 / 9.;
			ex2sq = eq2sq / q1 / q1 / 9.;
		}
		Double_t xError = TMath::Sqrt( ex0sq + ex1sq + ex2sq);

		// Cluster charge
		Double_t charge = q1 + q2;

		cluster->SetAddress(module->GetAddress());
		cluster->SetProperties(charge, x, xError, time, timeError);
		cluster->SetSize(2);

	} //? 2-strip cluster


	// --- For clusters with more than 2 strips
	// It is assumed that the digis are ordered w.r.t. channel number
	else {

		Double_t tSum = 0.;  // sum of digi times
		Double_t eqMidSq = 0.;  // sum of errors of digi charges (middle)
		Int_t chanF = 9999999;  // first channel in cluster
		Int_t chanL = -1;  // last channel in cluster
		Double_t qF = 0.; // charge in first channel
		Double_t qM = 0.;  // sum of charges in middle channels
		Double_t qL = 0.; // charge in last cluster
		Double_t eqFsq = 0.; // uncertainty of qF
		Double_t eqMsq = 0.;  // uncertainty of qMid
		Double_t eqLsq = 0.; // uncertainty of qL
		Double_t prevChannel = 0;

		for (Int_t iDigi = 0; iDigi < cluster->GetNofDigis(); iDigi++) {

			Int_t index = cluster->GetDigi(iDigi);
			CbmStsDigi* digi = dynamic_cast<CbmStsDigi*> ( digiArray->At(index) );
			assert(digi);

			tSum += digi->GetTime();
			Double_t charge = module->AdcToCharge(digi->GetCharge());
			Double_t lWidth = fPhysics->LandauWidth(charge);
			Double_t eChargeSq = lWidth*lWidth + eNoiseSq + eDigitSq;
			UInt_t address = digi->GetAddress();
			Int_t channel = CbmStsAddress::GetElementId(address, kStsChannel);

			// Check ascending order of channel number
			if ( iDigi > 0 )
			  assert(channel == prevChannel + 1 ||
			         channel == prevChannel - module->GetNofChannels() / 2 + 1);
			prevChannel = channel;

			if ( iDigi == 0 ) {  // first channel
				chanF = channel;
				qF = charge;
				eqFsq = eChargeSq;
			}
			else if ( iDigi == cluster->GetNofDigis()-1) { // last channel
				chanL = channel;
				qL = charge;
				eqLsq = eChargeSq;
			}
			else {   // one of the middle channels
				qM += charge;
				eqMsq += eChargeSq;
			}

		} //# digis in cluster

		// Periodic channel position for clusters round the edge
		if ( chanF > chanL ) chanF -= module->GetNofChannels()/2;

		// Cluster time and total charge
		tSum = tSum / Double_t(cluster->GetNofDigis());
		Double_t tError = module->GetTimeResolution()
				        / TMath::Sqrt(Double_t(cluster->GetNofDigis()));
		Double_t qSum = qF + qM + qL;

		// Average charge in middle strips
		qM /= Double_t(cluster->GetNofDigis() - 2);
		eqMsq /= Double_t(cluster->GetNofDigis() - 2);

		// Cluster position
		Double_t x = 0.5 * ( Double_t( chanF + chanL ) + ( qL - qF ) / qM );

		// Correct negative cluster position for clusters round the edge
		if ( x < - 0.5 ) x += Double_t(module->GetNofChannels() / 2);

		// Cluster position error
		Double_t exFsq = eqFsq / qM / qM / 4.;  // error from first charge
		Double_t exMsq = eqMsq * (qL - qF) * (qL - qF) / qM / qM / qM / qM / 4.;
		Double_t exLsq = eqLsq / qM / qM / 4.;
		Double_t ex1sq = 0.;    // error from first charge
		Double_t ex2sq = 0.;    // error from second charge
		Double_t xError = TMath::Sqrt(exFsq + exMsq + exLsq);

		cluster->SetAddress(module->GetAddress());
		cluster->SetProperties(qSum, x, xError, tSum, tError);
		cluster->SetSize(chanL - chanF + 1);

	} //? n-strip cluster

}
// --------------------------------------------------------------------------



ClassImp(CbmStsClusterAnalysis)
