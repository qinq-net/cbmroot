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


	// --- For 1-strip clusters
	if ( cluster->GetNofDigis() == 1 ) {

		Int_t index = cluster->GetDigi(0);
		CbmStsDigi* digi = dynamic_cast<CbmStsDigi*> ( digiArray->At(index) );
		assert(digi);
		UInt_t address = digi->GetAddress();
		Double_t x = Double_t(CbmStsAddress::GetElementId(address, kStsChannel));
		Double_t time = digi->GetTime();
		Double_t charge = module->AdcToCharge(digi->GetCharge());
		Double_t xError = 1. / sqrt(24.);

		cluster->SetAddress(module->GetAddress());
		cluster->SetProperties(charge, x, 0., time);
		cluster->SetPositionError(xError);
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
		Double_t x1 = Double_t(CbmStsAddress::GetElementId(address1, kStsChannel));
		Double_t x2 = Double_t(CbmStsAddress::GetElementId(address2, kStsChannel));
		Double_t q1 = module->AdcToCharge(digi1->GetCharge());
		Double_t q2 = module->AdcToCharge(digi2->GetCharge());

		// Cluster time
		Double_t time = 0.5 * ( digi1->GetTime() + digi2->GetTime());

		// Cluster position
		Double_t x = x1 + 0.5 * ( 1. + (q2 - q1) / (q2 + q1) );
		Double_t dX = 7.6336e-3;

		// Cluster charge
		Double_t charge = q1 + q2;

		cluster->SetAddress(module->GetAddress());
		cluster->SetProperties(charge, x, 0., time);
		cluster->SetPositionError(dX);
		cluster->SetSize(2);

	} //? 2-strip cluster


	// --- For clusters with more than 2 strips
	else {

		Double_t tSum = 0.;
		Double_t qSum = 0.;
		Int_t chanF = 9999999;  // first channel in cluster
		Int_t chanL = -1;  // last channel in cluster
		Double_t qF = 0.; // charge in first channel
		Double_t qL = 0.; // charge in last cluster

		for (Int_t iDigi = 0; iDigi < cluster->GetNofDigis(); iDigi++) {

			Int_t index = cluster->GetDigi(iDigi);
			CbmStsDigi* digi = dynamic_cast<CbmStsDigi*> ( digiArray->At(index) );
			assert(digi);

			tSum += digi->GetTime();
			Double_t charge = module->AdcToCharge(digi->GetCharge());
			qSum += charge;
			UInt_t address = digi->GetAddress();
			Int_t channel = CbmStsAddress::GetElementId(address, kStsChannel);

			if ( channel < chanF ) {
				chanF = channel;
				qF = charge;
			}
			if ( channel > chanL ) {
				chanL = channel;
				qL = charge;
			}

		} //# digis in cluster

		// Cluster time
		tSum = tSum / Double_t(cluster->GetNofDigis());

		// Average charge in middle strips
		Double_t qMid = ( qSum - qF - qL ) / Double_t(cluster->GetNofDigis() - 2);

		// Cluster position
		Double_t x = 0.5 * ( Double_t( chanF + chanL ) + ( qL - qF ) / qMid );

		// Cluster position error
		Double_t dX = 0.;

		cluster->SetAddress(module->GetAddress());
		cluster->SetProperties(qSum, x, 0., tSum);
		cluster->SetPositionError(dX);
		cluster->SetSize(chanL - chanF);

	} //? n-strip cluster

}
// --------------------------------------------------------------------------



ClassImp(CbmStsClusterAnalysis)
