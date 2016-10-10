/** @file CbmStsModule.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 14.05.2013
 **/

#include "TClonesArray.h"
#include "TRandom.h"
#include "FairLogger.h"
#include "FairRunAna.h"
#include "CbmMatch.h"
#include "CbmStsAddress.h"
#include "CbmStsCluster.h"
#include "CbmStsDigi.h"
#include "digitize/CbmStsDigitize.h"
#include "digitize/CbmStsPhysics.h"
#include "digitize/CbmStsSensorTypeDssd.h"
#include "setup/CbmStsModule.h"
#include "setup/CbmStsSensorType.h"
#include "setup/CbmStsSetup.h"

using std::pair;
using std::multimap;
using std::multiset;
using std::cout;

// -----   Default constructor   -------------------------------------------
CbmStsModule::CbmStsModule() : CbmStsElement(),
                               fNofChannels(2048),
                               fDynRange(0.),
                               fThreshold(0.),
                               fNofAdcChannels(0),
                               fTimeResolution(0),
                               fDeadTime(0.),
                               fNoise(0.),
                               fIsSet(kFALSE),
                               fDeadChannels(),
			       fPhysics(NULL),
                               fAnalogBuffer(),
                               fDigis(),
                               fClusters(),
                               fDigisTb(),
			       fDigisTbtemp(),
                               fIt_DigiTb()
{
	fPhysics = CbmStsPhysics::Instance();
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
                           fTimeResolution(0),
                           fDeadTime(0.),
                           fNoise(0.),
                           fIsSet(0),
                           fDeadChannels(),
			   fPhysics(NULL),
                           fAnalogBuffer(),
                           fDigis(),
                           fClusters(),
                           fDigisTb(),
			   fDigisTbtemp(),
                           fIt_DigiTb()
{
	fPhysics = CbmStsPhysics::Instance();
}
// -------------------------------------------------------------------------



// --- Destructor   --------------------------------------------------------
CbmStsModule::~CbmStsModule() {

	// --- Clean analog buffer
	map<Int_t, multiset<CbmStsSignal*, CbmStsSignal::Before> >::iterator chanIt;
	multiset<CbmStsSignal*>::iterator sigIt;
	for (chanIt = fAnalogBuffer.begin(); chanIt != fAnalogBuffer.end();
			 chanIt++) {
		for ( sigIt = (chanIt->second).begin(); sigIt != (chanIt->second).end();
				  sigIt++) {
			delete (*sigIt);
		}
	}
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
                             Double_t charge, Int_t index, Int_t entry,
                             Int_t file) {

	// --- Debug
	LOG(DEBUG3) << GetName() << ": Receiving signal " << charge
			        << " in channel " << channel << " at time "
			        << time << " s" << FairLogger::endl;
	
	// --- Discard charge if the channel is dead
	if ( fDeadChannels.count(channel) ) {
		LOG(DEBUG) << GetName() << ": discarding signal in dead channel "
				       << channel << FairLogger::endl;
		return;
	}

	// --- If the channel is not yet active: create a new set and insert
	// --- new signal into it.
	if ( fAnalogBuffer.find(channel) == fAnalogBuffer.end() ) {
		CbmStsSignal* signal = new CbmStsSignal(time, charge,
				                                    index, entry, file);
		fAnalogBuffer[channel].insert(signal);
		LOG(DEBUG4) << GetName() << ": Activating channel " << channel
				        << FairLogger::endl;
		return;
	}  //? Channel not yet active

	// --- The channel is active: there are already signals in.
	// --- Loop over all signals in the channels and compare their time.
	//TODO: Loop over all signals is not needed, since they are time-ordered.
	Bool_t isMerged = kFALSE;
	sigset::iterator it;
	for (it = fAnalogBuffer[channel].begin();
			 it != fAnalogBuffer[channel].end(); it++) {

		// Time between new and old signal smaller than dead time: merge signals
		if ( TMath::Abs( (*it)->GetTime() - time ) < fDeadTime ) {

			// Current implementation of merging signals:
			// Add charges, keep first signal time
			// TODO: Check with STS electronics people on more realistic behaviour.
			LOG(DEBUG4) << GetName() << ": channel " << channel
					        << ", new signal at t = " << time
					        << " ns is merged with present signal at t = "
					        << (*it)->GetTime() << " ns" << FairLogger::endl;
			(*it)->SetTime( TMath::Min( (*it)->GetTime(), time) );
			(*it)->AddLink(charge, index, entry, file);
			isMerged = kTRUE;  // mark new signal as merged
			LOG(DEBUG4) << "    New signal: time " << (*it)->GetTime()
					        << ", charge " << (*it)->GetCharge()
					        << ", number of links " << (*it)->GetMatch().GetNofLinks()
					        << FairLogger::endl;
			break;  // Merging should be necessary only for one buffer signal

		} //? Time difference smaller than dead time

	} // Loop over signals in buffer for this channel

	// --- If signal was merged: no further action
	if ( isMerged ) return;

	// --- Arriving here, the signal did not interfere with existing ones.
	// --- So, it is added to the analog buffer.
	CbmStsSignal* signal = new CbmStsSignal(time, charge,
                                          index, entry, file);
	fAnalogBuffer[channel].insert(signal);
	LOG(DEBUG4) << GetName() << ": Adding signal at t = " << time
			        << " ns, charge " << charge << " in channel " << channel
			        << FairLogger::endl;

}
// -------------------------------------------------------------------------



// -----   Status of analog buffer   ---------------------------------------
void CbmStsModule::BufferStatus(Int_t& nofSignals,
		                            Double_t& timeFirst,
		                            Double_t& timeLast) {

	map<Int_t, sigset>::iterator chanIt;
	sigset::iterator sigIt;

	Int_t nSignals   = 0;
	Double_t tFirst  = -1.;
	Double_t tLast   = -1.;
	Double_t tSignal = -1.;

	// --- Loop over active channels
	for ( chanIt  = fAnalogBuffer.begin();
			  chanIt != fAnalogBuffer.end(); chanIt++) {

		// --- Loop over signals in channel
		for ( sigIt  = (chanIt->second).begin();
				  sigIt != (chanIt->second).end(); sigIt++) {

			tSignal = (*sigIt)->GetTime();
			nSignals++;
			tFirst = tFirst < 0. ? tSignal : TMath::Min(tFirst, tSignal);
			tLast  = TMath::Max(tLast, tSignal);

		} // signals in channel

	} // channels in module

	nofSignals = nSignals;
	timeFirst  = tFirst;
	timeLast   = tLast;

}
// -------------------------------------------------------------------------



// -----   Convert analog charge to ADC channel number   -------------------
Int_t CbmStsModule::ChargeToAdc(Double_t charge) {
	if ( charge < fThreshold ) return -1;
	Int_t adc = Int_t ( (charge - fThreshold) * Double_t(fNofAdcChannels)
			        / fDynRange );
	return ( adc < fNofAdcChannels ? adc : fNofChannels - 1 );
}
// -------------------------------------------------------------------------


// -----   Create a cluster in the output TClonesArray   -------------------
void CbmStsModule::CreateCluster(Int_t clusterStart, Int_t clusterEnd,
		                             TClonesArray* clusterArray, Int_t algorithm, Int_t eLossModel) {

        CbmStsCluster* cluster = NULL;

	// --- If output array is specified: Create a new cluster there
	if ( clusterArray ) {
		Int_t nClusters = clusterArray->GetEntriesFast();
		cluster = new ( (*clusterArray)[nClusters] ) CbmStsCluster();
	}

	// --- If no output array is specified: Create a new cluster and add it
	// --- to the module
	else {
	    cluster = new CbmStsCluster();
	    AddCluster(cluster);
	}


	// --- Calculate total charge, cluster position and spread
	Double_t sum1 = 0.;  // sum of charges
	Double_t sum2 = 0.;  // position in channel number
	Double_t sum3 = 0.;  // sum of channel^2 * charge
	Double_t tsum = 0.;  // sum of digi times
	
	Double_t errorMethod = 0.;
	Double_t errorMeas = 0.;
	Double_t errorNoise2 = fNoise*fNoise;
	Double_t errorDiscr2 = fDynRange * fDynRange / 12. / fNofAdcChannels / fNofAdcChannels;  

	Double_t error = 0.;
	Int_t index = -1;

// FU: Don't know why the following line is here. Comment it since the
// change is global. Probably is was commited by accident with r 9816 
//std:cout.precision(10);

	//--- Center-of-gravity algorithm
	if (algorithm == 0) {
	    for (Int_t channel = clusterStart; channel <= clusterEnd; channel++) {
		CbmStsDigi* digi = GetDigi(channel, index);
		if ( ! digi ) continue;
		cluster->AddDigi(index);
		Double_t charge = AdcToCharge(digi->GetCharge());
		sum1 += charge;
		sum2 += charge * Double_t(channel);
		sum3 += charge * Double_t(channel) * Double_t(channel);
		tsum += digi->GetTime();
	    }

	    if ( sum1 > 0. ) {
		sum2 /= sum1;
		for (Int_t channel = clusterStart; channel <= clusterEnd; channel++) {
		    CbmStsDigi* digi = GetDigi(channel, index);
		    if ( ! digi ) continue;
		    errorMeas += (channel - sum2) * (channel - sum2);
			}
		errorMeas /= sum1;
		errorMeas *= sqrt(errorNoise2 + errorDiscr2); 
		errorMethod = 1. / sqrt(24.);
	    }
	}

	// --- advanced algorithm
	 else if (algorithm == 1){
	    //1-strip cluster
	    if (clusterStart == clusterEnd){
	    	CbmStsDigi* digi = GetDigi(clusterStart, index);
	    	if ( digi ){
	    		cluster->AddDigi(index);
	    		Double_t charge = AdcToCharge(digi->GetCharge());
	    		sum1 = charge;
	    		sum2 = Double_t (clusterStart);
	    		sum3 = charge * Double_t(clusterStart) * Double_t(clusterStart);
	    		tsum += digi->GetTime();

	    		if (sum1 > 0.) {
	    			errorMethod = 1. / sqrt(24.);
	    			errorMeas = 0.;
	    		}
	    	}
	    }//end of 1-strip clusters

	    //2- strip cluster 
	    if ((clusterEnd - clusterStart) == 1){
		Double_t chargeFirst = 0.;
		Double_t chargeLast = 0.;
		Double_t dq12, dq22;
		for (Int_t channel = clusterStart; channel <= clusterEnd; channel++) {
		    CbmStsDigi* digi = GetDigi(channel, index);
		    if ( ! digi ) continue;
		    cluster->AddDigi(index);
		    Double_t charge = AdcToCharge(digi->GetCharge());
		    tsum += digi->GetTime();
		    if (channel == clusterStart) chargeFirst = charge;
		    if (channel == clusterEnd) chargeLast = charge;
		}
		dq12 = errorNoise2 + errorDiscr2;
		dq22 = errorNoise2 + errorDiscr2;
		if (eLossModel == 2){
		    dq12 += fPhysics -> LandauWidth(chargeFirst) * fPhysics -> LandauWidth(chargeFirst);
		    dq22 += fPhysics -> LandauWidth(chargeLast) * fPhysics -> LandauWidth(chargeLast);
		}

		sum1 = chargeFirst + chargeLast;
		sum3 = chargeFirst * Double_t(clusterStart) * Double_t(clusterStart) + chargeLast * Double_t(clusterEnd) * Double_t(clusterEnd);
		if ( sum1 > 0. ) {
		    sum2 = clusterEnd - 0.5 + (chargeLast - chargeFirst) / TMath::Max(chargeLast, chargeFirst) / 3.;
		    errorMethod =  1. / sqrt(72.) * TMath::Abs(chargeLast - chargeFirst) / TMath::Max(chargeLast, chargeFirst);
		    errorMeas = 1. / 3. / TMath::Max(chargeFirst, chargeLast) / TMath::Max(chargeFirst, chargeLast) * sqrt(chargeFirst*chargeFirst * dq22 + chargeLast*chargeLast * dq12);
		} 
	    }// end of 2-strip clusters

	    //3,4....-strip cluster
	    if (1 < (clusterEnd - clusterStart)){
		Double_t chargeFirst = 0.;
		Double_t chargeLast = 0.;
		Double_t chargeMiddle = 0.;
		Double_t dq2, dq12, dqN2;
		Double_t dqMiddle2 = 0.;
		for (Int_t channel = clusterStart; channel <= clusterEnd; channel++) {
		    CbmStsDigi* digi = GetDigi(channel, index);
		    if ( ! digi ) continue;
		    cluster->AddDigi(index);
		    Double_t charge = AdcToCharge(digi->GetCharge());
		    dq2 = errorNoise2 + errorDiscr2;
		    if (eLossModel == 2) dq2 += fPhysics -> LandauWidth(charge) * fPhysics -> LandauWidth(charge);
		    sum1 += charge;
		    sum3 += charge * Double_t(channel) * Double_t(channel);
		    if (channel == clusterStart) {
			chargeFirst = charge;
			dq12 = dq2;
		    }
		    if (channel == clusterEnd) {
			chargeLast = charge;
			dqN2 = dq2;
		    }
		    if (channel > clusterStart && channel < clusterEnd) {
			chargeMiddle += charge;
			dqMiddle2 += dq2;
		    }

		    tsum += digi->GetTime();
		}
		dqMiddle2 /= (clusterEnd - clusterStart - 1);
		chargeMiddle /= (clusterEnd - clusterStart - 1);
		if (chargeMiddle > 0.1){
		    sum2 = 0.5 * (clusterStart + clusterEnd + (chargeLast - chargeFirst) / chargeMiddle);
		    errorMethod = 0.;
		    errorMeas = 1. / 2. / chargeMiddle * sqrt(dq12 + dqN2 + (chargeLast - chargeFirst) * (chargeLast - chargeFirst) * dqMiddle2 / chargeMiddle / chargeMiddle);

		} else if (sum1 > 0.1){ //all the middle strips has 0 charge, but first and last strips have some charge
		    sum2 = (clusterEnd - clusterStart) / 2. + (chargeLast - chargeFirst) / TMath::Max(chargeLast, chargeFirst) / 3.;
		    errorMethod =  1. / sqrt(72.) * TMath::Abs(chargeLast - chargeFirst) / TMath::Max(chargeLast, chargeFirst);
		    errorMeas = 1. / 3. / TMath::Max(chargeFirst, chargeLast) / TMath::Max(chargeFirst, chargeLast) * sqrt(chargeFirst*chargeFirst * dqN2 + chargeLast*chargeLast * dq12);
		}

	
	    }// end of 3-strip and bigger clusters
	 }// end of advanced

	 if (sum1 > 0) {
	     sum3 /= sum1;
	     error = sqrt(errorMethod*errorMethod + errorMeas*errorMeas);
	 }
	 else {
	     sum3 = -1.;
	     sum2 = -1.;
	     error = -1.;
	 }

	// --- Cluster time is average of all digi time
	if ( cluster->GetNofDigis() ) tsum /= cluster->GetNofDigis();
	else tsum = 0.;
	cluster->SetProperties(sum1, sum2, sum3, tsum);
	cluster->SetPositionError(error);
	cluster->SetAddress(fAddress);
	cluster->SetSize(clusterEnd - clusterStart + 1);

	LOG(DEBUG4) << GetName() << ": Created new cluster from channel "
	    << clusterStart << " to " << clusterEnd << ", charge "
	    << sum1 << ", time " << tsum << "ns, channel mean "
	    << sum2 << FairLogger::endl;
}
// -------------------------------------------------------------------------

	

// -----   Digitise an analog charge signal   ------------------------------
void CbmStsModule::Digitize(Int_t channel, CbmStsSignal* signal) {

	// --- Add noise to the signal
	Double_t charge = signal->GetCharge();
	if ( fNoise > 0.)
		charge = signal->GetCharge() + gRandom->Gaus(0., fNoise);

	// --- No action if charge is below threshold
	if ( charge < fThreshold ) return;

	// --- Construct channel address from module address and channel number
	UInt_t address = CbmStsAddress::SetElementId(GetAddress(),
			                                         kStsChannel, channel);

	// --- Digitise charge
	// --- Prescription according to the information on the STS-XYTER
	// --- by C. Schmidt.
	UShort_t adc = 0;
	if ( charge > fDynRange ) adc = fNofAdcChannels - 1;
	else adc = UShort_t( (charge - fThreshold) / fDynRange
				     * Double_t(fNofAdcChannels) );

	// --- Digitise time
	// Note that the time is truncated at 0 to avoid negative times. This
	// will show up in event-by-event simulations, since the digi times
	// in this case are mostly below 1 ns.
	Double_t  deltaT = gRandom->Gaus(0., fTimeResolution);
	Long64_t dTime = Long64_t(std::round(signal->GetTime() + deltaT));;

	// --- Send the message to the digitiser task
	LOG(DEBUG4) << GetName() << ": charge " << signal->GetCharge()
			        << ", dyn. range " << fDynRange << ", threshold "
			        << fThreshold << ", # ADC channels "
			        << fNofAdcChannels << FairLogger::endl;
	LOG(DEBUG3) << GetName() << ": Sending message. Address " << address
			        << ", time " << dTime << ", adc " << adc << FairLogger::endl;
	CbmStsDigitize* digitiser = CbmStsSetup::Instance()->GetDigitizer();
	if ( digitiser ) digitiser->CreateDigi(address, dTime, adc,
			                                   signal->GetMatch());
	
	// --- If no digitiser task is present (debug mode): create a digi and
	// --- add it to the digi buffer.
	else {
		CbmStsDigi* digi = new CbmStsDigi(address, dTime, adc);
		AddDigi(digi, 0);
	}
	return;
}
// -------------------------------------------------------------------------



// -----   Find hits   -----------------------------------------------------
Int_t CbmStsModule::FindHits(TClonesArray* hitArray) {

	// --- Call FindHits method in each daughter sensor
	Int_t nHits = 0;
	for (Int_t iSensor = 0; iSensor < GetNofDaughters(); iSensor++) {
		CbmStsSensor* sensor = dynamic_cast<CbmStsSensor*>(GetDaughter(iSensor));
		nHits += sensor->FindHits(fClusters, hitArray, fDeadTime);
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



// -----  Initialise the analog buffer   -----------------------------------
void CbmStsModule::InitAnalogBuffer() {

	 for (Int_t channel = 0; channel < fNofChannels; channel++) {
		 multiset<CbmStsSignal*, CbmStsSignal::Before> mset;
		 fAnalogBuffer[channel] = mset;
	 } // channel loop

}
// -------------------------------------------------------------------------



// -----   Process the analog buffer   -------------------------------------
Int_t CbmStsModule::ProcessAnalogBuffer(Double_t readoutTime) {

	// --- Counter
	Int_t nDigis = 0;

	// --- Time limit up to which signals are digitised and sent to DAQ.
	// --- Up to that limit, it is guaranteed that future signals do not
	// --- interfere with the buffered ones. The readoutTime is the time
	// --- of the last processed StsPoint. All coming points will be later
	// --- in time. So, the time limit is defined by this time minus
	// --- 5 times the time resolution (maximal deviation of signal time
	// --- from StsPoint time) minus the dead time, within which
	// --- interference of signals can happen.
	Double_t timeLimit = readoutTime - 5. * fTimeResolution - fDeadTime;

	// --- Iterate over active channels
	map<Int_t, sigset>::iterator chanIt;
	for (chanIt = fAnalogBuffer.begin();
			 chanIt != fAnalogBuffer.end(); chanIt++) {

		// --- Digitise all signals up to the specified time limit
		sigset::iterator sigIt = (chanIt->second).begin();
		sigset::iterator oldIt = sigIt;
		while ( sigIt != (chanIt->second).end() ) {

			// --- Exit loop if signal time is larger than time limit
			// --- N.b.: Readout time < 0 means digitise everything
			if ( readoutTime >= 0. && (*sigIt)->GetTime() > timeLimit ) break;

			// --- Digitise signal
			Digitize( chanIt->first, (*sigIt) );
			nDigis++;

			// --- Increment iterator before it becomes invalid
			oldIt = sigIt;
			sigIt++;

			// --- Delete digitised signal
			delete (*oldIt);
			(chanIt->second).erase(oldIt);

		} // Iterate over signals in channel
	}  // Iterate over channels

	return nDigis;
}
// -------------------------------------------------------------------------



// -----   Create list of dead channels   ----------------------------------
Int_t CbmStsModule::SetDeadChannels(Double_t percentage) {

	Double_t fraction = percentage;

	// --- Catch illegal percentage values
	if ( percentage < 0. ) {
		LOG(WARNING) << GetName() << ": illegal percentage of dead channels "
				<< percentage << ", is set to 0." << FairLogger::endl;
		fraction = 0.;
	}
	if ( percentage > 100. ) {
		LOG(WARNING) << GetName() << ": illegal percentage of dead channels "
				<< percentage << ", is set to 100." << FairLogger::endl;
		fraction = 100.;
	}

	// --- Re-set dead channel list
	fDeadChannels.clear();

	// --- Number of dead channels
	UInt_t nOfDeadChannels = fraction * fNofChannels / 100;

	// --- Case percentage < 50: randomise inactive channels
	// --- N.b.: catches also zero fraction (nOfDeadChannels = 0)
	// --- N.b.: set::insert has no effect if element is already present
	if ( nOfDeadChannels < (fNofChannels / 2) ) {
		while ( fDeadChannels.size() < nOfDeadChannels )
			fDeadChannels.insert( Int_t( gRandom->Uniform(fNofChannels) ) );
	}

	// --- Case percentage > 50: randomise active channels
	// --- N.b.: catches also unity fraction (nOfDeadChannels = fNofChannels)
	// --- N.b.: set::erase has no effect if element is not present
	else {
		for (Int_t channel = 0; channel < fNofChannels; channel++)
			fDeadChannels.insert(channel);
		while ( fDeadChannels.size() > nOfDeadChannels )
			fDeadChannels.erase( Int_t ( gRandom->Uniform(fNofChannels) ) );
	}

	return fDeadChannels.size();
}
// -------------------------------------------------------------------------



// -----   Status info   ---------------------------------------------------
void CbmStsModule::Status() const {
	LOG(INFO) << GetName() << ": Signals " << fAnalogBuffer.size()
			      << ", digis " << fDigis.size()
			      << ", clusters " << fClusters.size() << FairLogger::endl;
}
// -------------------------------------------------------------------------

// -----   Time-based cluster finding   ----------------------------
// -----   Add a digi to the multimap     ----------------------------------
void CbmStsModule::AddDigiTb(CbmStsDigi* digi, Int_t index) {
	if ( CbmStsAddress::GetMotherAddress(digi->GetAddress(), kStsModule ) != fAddress ) {
		LOG(ERROR) << GetName() << ": Module address is " << fAddress
						<< ", trying to add a digi with module address "
						<< CbmStsAddress::GetMotherAddress(digi->GetAddress(), kStsModule)
						<< FairLogger::end;
		return;
	}
	Int_t channel = CbmStsAddress::GetElementId(digi->GetAddress(), kStsChannel);
	fDigisTb.insert(pair<Int_t, pair<CbmStsDigi*, Int_t> >(channel, pair<CbmStsDigi*, Int_t>(digi, index)));
	return;
}

// -----   Start clustering procedure for the current module   -------------
void CbmStsModule::StartClusteringTb() {
	fDigisTbtemp = fDigisTb;
	fIt_DigiTb = fDigisTbtemp.begin();
}

// -----   Get information about next digi in multimap   -------------------
Bool_t CbmStsModule::GetNextDigiTb(Int_t& channel, Double_t& time, Int_t& index, Int_t& charge) {
	if ( fIt_DigiTb == fDigisTbtemp.end() ) return kFALSE;
	channel = fIt_DigiTb->first;
	index = fIt_DigiTb->second.second;
	charge = fIt_DigiTb->second.first->GetCharge();
	time = fIt_DigiTb->second.first->GetTime();
	fIt_DigiTb++;
	return kTRUE;
}

// -----   Delete used digi from the multimap   --------------------------------
void CbmStsModule::DeactivateDigiTb() {
	fIt_DigiTb = fDigisTbtemp.erase(--fIt_DigiTb);
}

CbmStsDigi* CbmStsModule::GetDigiTb(Int_t channel, Int_t index) {

	if ( ! ( channel >= 0 && channel < fNofChannels) ) {
		LOG(ERROR) << GetName() << ": Illegal channel number " << channel
				       << FairLogger::endl;
		return NULL;
	}

	multimap<Int_t, pair<CbmStsDigi*, Int_t> >::iterator it = fDigisTb.find(channel);
	do {
		if ( (it->second).second == index )
			return (it->second).first;
		else
			++it;
	} while ( it->first == channel );

	return NULL;
}

// -----   Create cluster in the output array   ----------------------------
void CbmStsModule::CreateClusterTb(vector<Int_t> digiIndexes, Double_t s1,
		Double_t s2, Double_t s3, Double_t ts, Bool_t /*side*/, TClonesArray* clusterArray) {
	CbmStsCluster* cluster = NULL;
	// --- If output array is specified: Create a new cluster there
	if ( clusterArray ) {
		Int_t nClusters = clusterArray->GetEntriesFast();
		cluster = new ( (*clusterArray)[nClusters] ) CbmStsCluster();
	}
	// --- If no output array is specified: Create a new cluster and add it
	// --- to the module
	else {
		cluster = new CbmStsCluster();
		AddCluster(cluster);
	}
	cluster->SetDigis(digiIndexes);
	cluster->SetProperties(s1, s2, s3, ts);
	cluster->SetAddress(fAddress);
	cluster->SetSize(digiIndexes.size());
	fIt_DigiTb = fDigisTbtemp.begin();
}

void CbmStsModule::CreateClusterTb(vector<Int_t>* digiIndexes, Int_t firstChannel, TClonesArray* clusterArray, Int_t algorithm) {
	CbmStsCluster* cluster = NULL;
	// --- If output array is specified: Create a new cluster there
	if ( clusterArray ) {
		Int_t nClusters = clusterArray->GetEntriesFast();
		cluster = new ( (*clusterArray)[nClusters] ) CbmStsCluster();
	}
	// --- If no output array is specified: Create a new cluster and add it
	// --- to the module
	else {
		cluster = new CbmStsCluster();
		AddCluster(cluster);
	}

	Double_t sum1 = 0.;  // sum of charges
	Double_t sum2 = 0.;  // position in channel number
	Double_t sum3 = 0.;  // sum of channel^2 * charge
	Double_t tsum = 0.;  // sum of digi times

	Double_t errorMethod = 0.;
	Double_t errorMeas = 0.;
	Double_t errorNoise2 = fNoise*fNoise;
	Double_t errorDiscr2 = fDynRange * fDynRange / 12. / fNofAdcChannels / fNofAdcChannels;

	Double_t error = 0.;

	Int_t clusterSize = digiIndexes->size();
	Int_t lastChannel = firstChannel + clusterSize - 1;
	if ( algorithm == 0 ) {
		for ( Int_t channel = firstChannel; channel <= lastChannel; channel++ ) {
			CbmStsDigi* digi = GetDigiTb(channel, (*digiIndexes)[channel - firstChannel]);
			if ( ! digi ) continue;
			Double_t charge = AdcToCharge(digi->GetCharge());
			sum1 += charge;
			sum2 += charge * Double_t(channel);
			sum3 += charge * Double_t(channel) * Double_t(channel);
			tsum += digi->GetTime();
		}

		if ( sum1 > 0. ) {
			sum2 /= sum1;
			for ( Int_t channel = firstChannel; channel <= lastChannel; channel++ ) {
				CbmStsDigi* digi = GetDigiTb(channel, (*digiIndexes)[channel - firstChannel]);
				if ( ! digi ) continue;
				errorMeas += (channel - sum2) * (channel - sum2);
			}
			errorMeas /= sum1;
			errorMeas *= sqrt(errorNoise2 + errorDiscr2);
			errorMethod = 1. / sqrt(24.);
		}
	}

	// --- advanced algorithm
	else if (algorithm == 1){
		if ( digiIndexes->size() == 1 ){
			CbmStsDigi* digi = GetDigiTb(firstChannel, (*digiIndexes)[0]);
			if ( digi ){
				Double_t charge = AdcToCharge(digi->GetCharge());
				sum1 = charge;
				sum2 = charge * Double_t(firstChannel);	// charge * - added
				sum3 = charge * Double_t(firstChannel) * Double_t(firstChannel);
				tsum += digi->GetTime();

				if (sum1 > 0.) {
					errorMethod = 1. / sqrt(24.);
					errorMeas = 0.;
				}
			}
		}//end of 1-strip clusters

		//2- strip cluster
		if ( digiIndexes->size() == 2 ){
			Double_t chargeFirst = 0.;
			Double_t chargeLast = 0.;
			Double_t dq12, dq22;
			for ( Int_t channel = firstChannel; channel <= lastChannel; channel++ ) {
				CbmStsDigi* digi = GetDigiTb(channel, (*digiIndexes)[channel - firstChannel]);
				if ( ! digi ) continue;
				Double_t charge = AdcToCharge(digi->GetCharge());
				tsum += digi->GetTime();
				if (channel == firstChannel) chargeFirst = charge;
				if (channel == lastChannel) chargeLast = charge;
			}
			dq12 = errorNoise2 + errorDiscr2 + fPhysics -> LandauWidth(chargeFirst) * fPhysics -> LandauWidth(chargeFirst);
			dq22 = errorNoise2 + errorDiscr2 + fPhysics -> LandauWidth(chargeLast) * fPhysics -> LandauWidth(chargeLast);

			sum1 = chargeFirst + chargeLast;
			sum3 = chargeFirst * Double_t(firstChannel) * Double_t(firstChannel) + chargeLast * Double_t(firstChannel + clusterSize) * Double_t(firstChannel + clusterSize);
			if ( sum1 > 0. ) {
				sum2 = lastChannel - 0.5 + (chargeLast - chargeFirst) / TMath::Max(chargeLast, chargeFirst) / 3.;
				errorMethod =  1. / sqrt(72.) * TMath::Abs(chargeLast - chargeFirst) / TMath::Max(chargeLast, chargeFirst);
				errorMeas = 1. / 3. / TMath::Max(chargeFirst, chargeLast) / TMath::Max(chargeFirst, chargeLast) * sqrt(chargeFirst*chargeFirst * dq22 + chargeLast*chargeLast * dq12);
			}
		}// end of 2-strip clusters

		//3,4....-strip cluster
		if ( digiIndexes->size() > 2 ){
			Double_t chargeFirst = 0.;
			Double_t chargeLast = 0.;
			Double_t chargeMiddle = 0.;
			Double_t dq2, dq12, dqN2;
			Double_t dqMiddle2 = 0.;
			for ( Int_t channel = firstChannel; channel <= lastChannel; channel++ ) {
				CbmStsDigi* digi = GetDigiTb(channel, (*digiIndexes)[channel - firstChannel]);
				if ( ! digi ) continue;
				Double_t charge = AdcToCharge(digi->GetCharge());
				dq2 = errorNoise2 + errorDiscr2 + fPhysics -> LandauWidth(charge) * fPhysics -> LandauWidth(charge);
				sum1 += charge;
				sum3 += charge * Double_t(channel) * Double_t(channel);
				if (channel == firstChannel) {
					chargeFirst = charge;
					dq12 = dq2;
				}
				if (channel == lastChannel) {
					chargeLast = charge;
					dqN2 = dq2;
				}
				if (channel > firstChannel && channel < lastChannel) {
					chargeMiddle += charge;
					dqMiddle2 += dq2;
				}

				tsum += digi->GetTime();
			}
			dqMiddle2 /= (clusterSize - 1);
			chargeMiddle /= (clusterSize - 1);
			sum2 = 0.5 * (firstChannel + lastChannel + (chargeLast - chargeFirst) / chargeMiddle);
			errorMethod = 0.;
			errorMeas = 1. / 2. / chargeMiddle * sqrt(dq12 + dqN2 + (chargeLast - chargeFirst) * (chargeLast - chargeFirst) * dqMiddle2 / chargeMiddle / chargeMiddle);
		}// end of 3-strip and bigger clusters
	}

	if ( sum1 > 0 ) {
		sum3 /= sum1;
		error = sqrt(errorMethod*errorMethod + errorMeas*errorMeas);
	}
	else {
		sum3 = -1.;
		sum2 = -1.;
		error = -1.;
	}
	// --- Cluster time is average of all digi time
	cluster->SetDigis(*digiIndexes);
	cluster->SetSize(digiIndexes->size());
	if ( cluster->GetNofDigis() ) tsum /= cluster->GetNofDigis();
	else tsum = 0.;
	cluster->SetProperties(sum1, sum2, sum3, tsum);
	cluster->SetPositionError(error);
	cluster->SetAddress(fAddress);
	fIt_DigiTb = fDigisTbtemp.begin();

	LOG(DEBUG4) << GetName() << ": Created new cluster from channel "
			<< firstChannel << " to " << lastChannel << ", charge "
			<< sum1 << ", time " << tsum << "ns, channel mean "
			<< sum2 << FairLogger::endl;
}
// -------------------------------------------------------------------------



ClassImp(CbmStsModule)
