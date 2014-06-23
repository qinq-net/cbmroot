/** @file CbmStsModule.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 14.05.2013
 **/

#include "TClonesArray.h"
#include "FairLogger.h"
#include "CbmStsAddress.h"
#include "CbmStsCluster.h"
#include "CbmStsDigi.h"
#include "digitize/CbmStsDigitizeIdeal.h"
#include "setup/CbmStsModule.h"
#include "setup/CbmStsSensorType.h"
#include "setup/CbmStsSetup.h"


// -----   Default constructor   -------------------------------------------
CbmStsModule::CbmStsModule() : CbmStsElement(),
 	                             fNofChannels(2048),
	                             fDynRange(0.),
	                             fThreshold(0.),
	                             fNofAdcChannels(0),
	                             fIsSet(kFALSE),
	                             fBuffer()
{
}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
CbmStsModule::CbmStsModule(const char* name, const char* title,
                           TGeoPhysicalNode* node) :
                           CbmStsElement(name, title, kStsModule, node),
                           fNofChannels(2048),
                           fDynRange(0.),
                           fThreshold(0.),
                           fNofAdcChannels(0),
                           fIsSet(0),
                           fBuffer()
{
}
// -------------------------------------------------------------------------



// --- Destructor   --------------------------------------------------------
CbmStsModule::~CbmStsModule() {
}
// -------------------------------------------------------------------------



// -----   Convert ADC channel to analog charge (channel mean)   -----------
Double_t CbmStsModule::AdcToCharge(UShort_t adcChannel) {
	return fThreshold + fDynRange / Double_t(fNofAdcChannels) *
			   ( Double_t(adcChannel) + 0.5 );
}
// -------------------------------------------------------------------------



// -----   Add a digi to the digi map   ------------------------------------
void CbmStsModule::AddDigi(CbmStsDigi* digi, Int_t index) {

	// --- Check whether digi really belongs to the module
	if ( CbmStsAddress::GetMotherAddress(digi->GetAddress(),
			                                 kStsModule ) != fAddress ) {
		LOG(ERROR) << GetName() << ": Module address is " << fAddress
				       << ", trying to add a digi with module address "
				       << CbmStsAddress::GetMotherAddress(digi->GetAddress(),
				       			                             kStsModule )
		           << FairLogger::endl;
		return;
	}

	// --- Get channel number
	Int_t channel = CbmStsAddress::GetElementId(digi->GetAddress(),
			                                        kStsChannel);
	if ( ! ( channel >= 0 && channel < fNofChannels ) ) {
		LOG(ERROR) << GetName() << ": Illegal channel number " << channel
				       << " (module has " << fNofChannels << " channels)."
				       << FairLogger::endl;
		return;
	}

	// --- Check channel for existing entry
	// --- In the event-by-event digitiser, it can happen that there
	// --- are two digis in one channel within the same event - if
	// --- their time difference is larger than the dead time.
	// --- Usually, the later digi corresponds to a spiralling delta
	// --- electron, the time of which may be as large as several
	// --- microseconds. Here, we neglect such digis. A proper treatment
	// --- must be done within a time-based cluster finding procedure.
	if ( fDigis.find(channel) != fDigis.end() ) return;

	// --- Add digi to map
	fDigis[channel] = pair<CbmStsDigi*, Int_t>(digi, index);
	return;
}
// -------------------------------------------------------------------------



// -----   Add a signal to the buffer   ------------------------------------
void CbmStsModule::AddSignal(Int_t channel, Double_t time,
                             Double_t charge) {

	// --- Debug
	LOG(DEBUG3) << GetName() << ": Receiving signal " << charge
			        << " in channel " << channel << " at time "
			        << time << " s" << FairLogger::endl;

	// --- If there is no signal in the buffer for this channel:
	// --- write the signal into the buffer
	if ( fBuffer.find(channel) == fBuffer.end() ) {
		fBuffer[channel] = pair<Double_t, Double_t>(charge, time);
		LOG(DEBUG4) << GetName() << ": New signal " << charge << " at time "
				        << time << " in channel " << channel << FairLogger::endl;
	}  //? No signal in buffer

	// --- If there is already a signal, compare the time of the two
	else {
		Double_t chargeOld = (fBuffer.find(channel)->second).first;
		Double_t timeOld   = (fBuffer.find(channel)->second).second;
		LOG(DEBUG4) << GetName() << ": old signal in channel " << channel
				        << " at time " << timeOld << FairLogger::endl;

		// --- Time separation large; no interference. Digitise the old signal,
		// --- send it to the DAQ and write the new signal into the buffer.
		if ( time - timeOld > 100. ) {  // 5 Mark in die Kasse für hardcoded numbers
			Digitize(channel, chargeOld, timeOld);
			fBuffer[channel] = pair<Double_t, Double_t>(charge, time);
			LOG(DEBUG4) << GetName() << ": New signal " << charge << " at time "
					        << time << " in channel " << channel << FairLogger::endl;
		}  //? No interference of signals

		// --- Time separation small; interference.
		// --- Current implementation: Add up charge in the buffer.
		// --- Time is the one of the first signal.
		else {
			fBuffer[channel] =
					pair<Double_t, Double_t>(chargeOld + charge, timeOld);
			LOG(DEBUG4) << GetName() << ": New signal " << chargeOld + charge
					        << " at time "<< timeOld << " in channel " << channel
					        << FairLogger::endl;
		}  //? Interference of signals

	}  //? Already signal in buffer

}
// -------------------------------------------------------------------------



// -----   Convert analog charge to ADC channel number   ------------------
Int_t CbmStsModule::ChargeToAdc(Double_t charge) {
	if ( charge < fThreshold ) return -1;
	Int_t adc = Int_t ( (charge - fThreshold) * Double_t(fNofAdcChannels)
			        / fDynRange );
	return ( adc < fNofAdcChannels ? adc : fNofChannels - 1 );
}
// -------------------------------------------------------------------------



// -----   Clean the analog buffer   ---------------------------------------
void CbmStsModule::CleanBuffer(Double_t readoutTime) {

	map<Int_t, pair<Double_t, Double_t> >::iterator it = fBuffer.begin();

	while ( it != fBuffer.end() ) {

		// --- Compare signal time to readout time
		Double_t signalTime = (it->second).second;
		if ( readoutTime < 0 || signalTime < readoutTime ) {

			// --- Digitise the signal and remove it from buffer
			Double_t charge = (it->second).first;
			Int_t channel = it->first;
			LOG(DEBUG4) << GetName() << ": Clean Buffer " << channel << " "
				        << signalTime << " " << charge << FairLogger::endl;
			Digitize(channel, charge, signalTime);

			// Next entry in buffer. Delete digitised buffer entry.
			it++;
			fBuffer.erase(channel);

		} //? Signal before read-out time

	}  // Iterator over buffer

}
// -------------------------------------------------------------------------



// -----   Create a cluster in the output TClonesArray   -------------------
void CbmStsModule::CreateCluster(Int_t clusterStart, Int_t clusterEnd,
		                             TClonesArray* clusterArray) {

	// --- Create new cluster
	Int_t nClusters = clusterArray->GetEntriesFast();
	CbmStsCluster* cluster =
			new ( (*clusterArray)[nClusters] ) CbmStsCluster();

	// --- Calculate total charge, cluster position and spread
	Double_t sum1 = 0.;  // sum of charges
	Double_t sum2 = 0.;  // sum of channel * charge
	Double_t sum3 = 0.;  // sum of channel^2 * charge
	Int_t index = -1;
	for (Int_t channel = clusterStart; channel <= clusterEnd; channel++) {
		CbmStsDigi* digi = GetDigi(channel, index);
		if ( ! digi ) continue;
		cluster->AddDigi(index);
		Double_t charge = AdcToCharge(digi->GetCharge());
		sum1 += charge;
		sum2 += charge * Double_t(channel);
		sum3 += charge * Double_t(channel) * Double_t(channel);
	}
	if ( sum1 > 0. ) {
		sum2 /= sum1;
		sum3 /= sum1;
	}
	else {
		sum2 = -1.;
		sum3 = -1.;
	}
	cluster->SetProperties(sum1, sum2, sum3);
	cluster->SetAddress(fAddress);

	LOG(DEBUG2) << GetName() << ": Created new cluster from channel "
			        << clusterStart << " to " << clusterEnd << ", charge "
			        << sum1 << ", channel mean " << sum2 << FairLogger::endl;
}
// -------------------------------------------------------------------------



// -----   Digitise an analog charge signal   ------------------------------
void CbmStsModule::Digitize(Int_t channel, Double_t charge, Double_t time) {

	// --- No action if charge is below threshold
	if ( charge <= fThreshold ) return;

	// --- Construct channel address from module address and channel number
	UInt_t address = CbmStsAddress::SetElementId(GetAddress(),
			                                         kStsChannel, channel);

	// --- Qualify front or back side
	Int_t side = ( channel < 1024 ? 0 : 1);
	address = CbmStsAddress::SetElementId(address, kStsSide, side);


	// --- Digitise charge
	// --- Prescription according to the information on the STS-XYTER
	// --- by C. Schmidt.
	UShort_t adc = 0;
	if ( charge > fDynRange ) adc = fNofAdcChannels - 1;
	else adc = UShort_t( (charge - fThreshold) / fDynRange
				     * Double_t(fNofAdcChannels) );

	// --- Digitise time
	// TODO: Add Gaussian time resolution
	ULong64_t dTime = ULong64_t(time);

	// --- Send the message to the digitiser task
	LOG(DEBUG4) << GetName() << ": charge " << charge << ", dyn. range "
			<< fDynRange << ", threshold " << fThreshold << ", # ADC channels "
			<< fNofAdcChannels << FairLogger::endl;
	LOG(DEBUG3) << GetName() << ": Sending message. Address " << address
			        << ", time " << dTime << ", adc " << adc << FairLogger::endl;
	CbmStsDigitizeIdeal* digitiser = CbmStsSetup::Instance()->GetDigitizer();
	if ( digitiser ) digitiser->CreateDigi(address, dTime, adc);

}
// -------------------------------------------------------------------------



// -----   Find hits   -----------------------------------------------------
Int_t CbmStsModule::FindHits(TClonesArray* hitArray) {

	// --- Call FindHits method in each daughter sensor
	Int_t nHits = 0;
	for (Int_t iSensor = 0; iSensor < GetNofDaughters(); iSensor++) {
		CbmStsSenzor* sensor = dynamic_cast<CbmStsSenzor*>(GetDaughter(iSensor));
		nHits += sensor->FindHits(fClusters, hitArray);
	}

	LOG(DEBUG2) << GetName() << ": Clusters " << fClusters.size()
			        << ", sensors " << GetNofDaughters() << ", hits "
			        << nHits << FairLogger::endl;
	return nHits;
}
// -------------------------------------------------------------------------



// -----   Get a digi in a channel   ---------------------------------------
CbmStsDigi* CbmStsModule::GetDigi(Int_t channel, Int_t& index) {

	if ( ! ( channel >= 0 && channel < fNofChannels) ) {
		LOG(ERROR) << GetName() << ": Illegal channel number " << channel
				       << FairLogger::endl;
		return NULL;
	}

	map<Int_t, pair<CbmStsDigi*, Int_t> >::iterator it = fDigis.find(channel);
	if (it == fDigis.end() ) {
		index = -1;
		return NULL;  // inactive channel
	}

	index = (it->second).second;  // index of digi in this channel
	return (it->second).first;    // digi in this channel
}
// -------------------------------------------------------------------------



// -----   Status info   ---------------------------------------------------
void CbmStsModule::Status() const {
	LOG(INFO) << GetName() << ": Signals " << fBuffer.size()
			      << ", digis " << fDigis.size()
			      << ", clusters " << fClusters.size() << FairLogger::endl;
}
// -------------------------------------------------------------------------


ClassImp(CbmStsModule)
