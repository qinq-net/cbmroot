/** @file CbmDaq.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 20 July 2012
 **/


#include <cassert>
#include <iomanip>

#include "FairEventHeader.h"
#include "FairRunAna.h"

#include "CbmDaq.h"
#include "CbmDaqBuffer.h"
#include "CbmTimeSlice.h"



// =====   Constructor   =====================================================
CbmDaq::CbmDaq(Double_t timeSliceSize) : FairTask("Daq"),
                   fCurrentStartTime (0.),
                   fDuration (timeSliceSize),
                   fStoreEmptySlices(kTRUE),
                   fTimer(),
                   fNofSteps(0),
                   fNofDigis(0),
                   fNofTimeSlices(0),
                   fNofTimeSlicesEmpty(0),
                   fTimeDigiFirst(-1.),
                   fTimeDigiLast(-1.),
                   fTimeSliceFirst(-1.),
                   fTimeSliceLast(-1.),
                   fTimeSlice(NULL),
                   fBuffer(NULL),
                   fEventList(),
                   fEventsCurrent(NULL),
                   fEventRange(){
}
// ===========================================================================



// =====   Destructor   ======================================================
CbmDaq::~CbmDaq() {
}
// ===========================================================================



// =====   Close the current time slice and fill it to the tree   ============
void CbmDaq::CloseTimeSlice() {

  // --- Time slice status
  if ( fTimeSlice->IsEmpty() ) {
    LOG(DEBUG) << GetName() << ": closing " << fTimeSlice->ToString()
                  << FairLogger::endl;
    fNofTimeSlicesEmpty++;
  } //? empty time slice
  else
  LOG(INFO) << GetName() << ": closing " << fTimeSlice->ToString()
               << FairLogger::endl;

  // --- Fill current time slice into tree (if required)
  if ( fStoreEmptySlices || (!fTimeSlice->IsEmpty()) ) {
    Int_t nMCEvents = CopyEventList();
    LOG(DEBUG) << GetName() << ": " << nMCEvents << " MC events for time slice"
        << FairLogger::endl;
    FairRootManager::Instance()->Fill();
  }
  fTimeSliceLast = fTimeSlice->GetEndTime();
  fNofTimeSlices++;
  if ( gLogger->IsLogNeeded(DEBUG) ) {
    fTimeSlice->SelfTest();
    PrintCurrentEventRange();
  }


  // --- Reset time slice with new time interval
  fCurrentStartTime += fDuration;
  fTimeSlice->Reset(fCurrentStartTime, fDuration);
  fEventRange.clear();
  fEventsCurrent->Clear("");

}
// ===========================================================================



// =====   Copy event list to output branch   ================================
Int_t CbmDaq::CopyEventList() {

  Int_t nMCEvents = 0;

  map<Int_t, pair<Int_t, Int_t>>::iterator fileIt;
  for (fileIt = fEventRange.begin(); fileIt != fEventRange.end(); fileIt++) {
    Int_t file = fileIt->first;
    Int_t firstEvent = fileIt->second.first;
    Int_t lastEvent  = fileIt->second.second;
    for (Int_t event = firstEvent; event <= lastEvent; event++) {
      Double_t time = fEventList.GetEventTime(event, file);
      fEventsCurrent->Insert(event, file, time);
      nMCEvents++;
    }  //# events
  } //# files

  return nMCEvents;
}
// ===========================================================================



// =====   Task execution   ==================================================
void CbmDaq::Exec(Option_t*) {

	// Start timer and digi counter
	fTimer.Start();
	Int_t nDigis = 0;

	// Get system time (start time of current event)
	Double_t systemTime = FairRootManager::Instance()->GetEventTime();
	LOG(DEBUG) << GetName() << ": System time is " << systemTime
			       << " ns " << FairLogger::endl;


  // --- DaqBuffer and time slice info
	LOG(DEBUG) << GetName() << ": " << fBuffer->ToString()
			       << FairLogger::endl;
	LOG(DEBUG) << GetName() << ": " << fTimeSlice->ToString()
			       << FairLogger::endl;

	// --- Fill the data from the buffer into the current time slice.
  while ( kTRUE ) {        /* Loop until time slice cannot be closed */

    nDigis += FillTimeSlice();

    // --> Exit if current time slice cannot yet be closed
    if ( systemTime < fTimeSlice->GetEndTime() + 2. * fDuration ) {
    	LOG(DEBUG) << GetName() << ": System time " << fixed
    			       << setprecision(3) << systemTime
    			       << " ns; waiting for data." << FairLogger::endl;
    	break;
    }

  	LOG(DEBUG) << GetName() << ": System time " << fixed
  			       << setprecision(3) << systemTime
  			       << " ns" << FairLogger::endl;
    CloseTimeSlice();

  }     /* Loop over time slices */

  // --- DaqBuffer and time slice info
	LOG(DEBUG) << GetName() << ": " << fBuffer->ToString()
			       << FairLogger::endl;

  // --- Event log
  fTimer.Stop();
  LOG(INFO) << "+ " << setw(20) << GetName() << ": event  " << setw(6)
  		      << right << fNofSteps << ", real time " << fixed
  		      << setprecision(6) << fTimer.RealTime() << " s, " << nDigis
  		      << " digis transported" << FairLogger::endl;

  // --- Store event start time in event list
  Int_t file  = FairRunAna::Instance()->GetEventHeader()->GetInputFileId();
  Int_t event = FairRunAna::Instance()->GetEventHeader()->GetMCEntryNumber();
  Double_t eventTime = FairRunAna::Instance()->GetEventHeader()->GetEventTime();
  fEventList.Insert(event, file, eventTime);

  // --- Increase exec counter
  fNofSteps++;

}
// ===========================================================================



// =====   Fill current time slice with data from buffers   ==================
Int_t CbmDaq::FillTimeSlice() {

  // --- Digi counter
  Int_t nDigis = 0;

  // --- Loop over all detector systems
  for (Int_t iDet = kREF; iDet < kNOFDETS; iDet++) {

    // --- Loop over digis from DaqBuffer and fill them into current time slice
    CbmDigi* digi = fBuffer->GetNextData(iDet, fTimeSlice->GetEndTime());
    while (digi) {

      LOG(DEBUG2) << fName << ": Inserting digi with detector ID "
		     << digi->GetAddress() << " at time " << digi->GetTime()
		     << " into time slice [" << fixed << setprecision(3)
		     << fTimeSlice->GetStartTime() << ", "
		     << fTimeSlice->GetEndTime() << ") ns" << FairLogger::endl;
      fTimeSlice->InsertData(digi);
      nDigis++;
      if (!fNofDigis) fTimeDigiFirst = digi->GetTime();
      fTimeDigiLast = TMath::Max(fTimeDigiLast, digi->GetTime());
      RegisterEvent(digi);

      // --- Store event and input number
       delete digi;
      digi = fBuffer->GetNextData(iDet, fTimeSlice->GetEndTime());
    }  //? Valid digi from buffer

  }  // Detector loop

  LOG(DEBUG) << GetName() << ": filled " << nDigis << " digis into "
		<< fTimeSlice->ToString() << FairLogger::endl;
  fNofDigis += nDigis;

  return nDigis;
}
// ===========================================================================



// =====   End-of-run action   ===============================================
void CbmDaq::Finish() {

  LOG(INFO) << FairLogger::endl;
  LOG(INFO) << fName << ": End of run" << FairLogger::endl;

  while ( fBuffer->GetSize() ) {  // time slice loop until buffer is emptied

    FillTimeSlice();
    CloseTimeSlice();

    // --- DaqBuffer and time slice info
  	LOG(DEBUG) << GetName() << ": " << fBuffer->ToString()
  			       << FairLogger::endl;
  	LOG(DEBUG) << GetName() << ": " << fTimeSlice->ToString()
  			       << FairLogger::endl << FairLogger::endl;

  }

	LOG(DEBUG) << GetName() << ": " << fBuffer->ToString()
			       << FairLogger::endl;
  LOG(INFO)  << GetName() << ": run finished." << FairLogger::endl;
	std::cout << std::endl;
	LOG(INFO) << "=====================================" << FairLogger::endl;
	LOG(INFO) << GetName() << ": Run summary" << FairLogger::endl;
	LOG(INFO) << "Events:       " << setw(10) << right << fNofSteps
						<< FairLogger::endl;
	LOG(INFO) << "Digis:        " << setw(10) << right << fNofDigis
			      << " from " << setw(10) << right << fixed << setprecision(1)
			      << fTimeDigiFirst << " ns  to " << setw(10) << right << fixed
			      << setprecision(1) << fTimeDigiLast << " ns" << FairLogger::endl;
	LOG(INFO) << "Time slices:  " << setw(10) << right << fNofTimeSlices
			      << " from " << setw(10) << right << fixed << setprecision(1)
			      << fTimeSliceFirst << " ns  to " << setw(10) << right << fixed
			      << setprecision(1) << fTimeSliceLast << " ns" << FairLogger::endl;
	LOG(INFO) << "Empty slices: " << setw(10) << right << fNofTimeSlicesEmpty
			      << FairLogger::endl;
	LOG(INFO) << "=====================================" << FairLogger::endl;

	std::cout << std::endl;
	fEventList.Print();

}
// ===========================================================================



// =====   Task initialisation   =============================================
InitStatus CbmDaq::Init() {

	std::cout << std::endl;
  LOG(INFO) << "=========================================================="
		        << FairLogger::endl;
 	LOG(INFO) << GetName() << ": Initialisation" << FairLogger::endl;
	LOG(INFO) << FairLogger::endl;
	LOG(INFO) << GetName() << ": Time slice interval is " << fDuration
			      << " ns." << FairLogger::endl;

	// Set initial start time
  fCurrentStartTime = 0.;
  fTimeSliceFirst = fCurrentStartTime;

  // Register output branch TimeSlice
  fTimeSlice = new CbmTimeSlice(fCurrentStartTime, fDuration);
  fTimeSliceFirst = fTimeSlice->GetStartTime();
  FairRootManager::Instance()->Register("TimeSlice.",
      "DAQ", fTimeSlice, kTRUE);

  // Register output branch EventList
  fEventsCurrent = new CbmMCEventList();
  FairRootManager::Instance()->Register("MCEventList.", "Event list",
      fEventsCurrent, kTRUE);


  // Get Daq Buffer
  fBuffer = CbmDaqBuffer::Instance();

  LOG(INFO) << GetName() << ": Initialisation successful"
		    << FairLogger::endl;
  LOG(INFO) << "=========================================================="
		        << FairLogger::endl;
	std::cout << std::endl;

  return kSUCCESS;
}
// ===========================================================================



// =====   Print current event range   =======================================
void CbmDaq::PrintCurrentEventRange() const {

  LOG(INFO) << GetName() << ": current MC event range: ";
  if ( fEventRange.empty() ) {
    LOG(INFO) << "empty" << FairLogger::endl;
    return;
  }
  map<Int_t, pair<Int_t, Int_t>>::const_iterator it = fEventRange.begin();
  while ( it != fEventRange.end() ) {
    Int_t file = it->first;
    Int_t firstEvent = it->second.first;
    Int_t lastEvent = it->second.second;
    LOG(INFO) << "\n          Input file " << file << ", first event "
        << firstEvent << ", last event " << lastEvent;
    it++;
  }  //# inputs
  LOG(INFO) << FairLogger::endl;

}
// ===========================================================================



// =====   Register MC event   ===============================================
void CbmDaq::RegisterEvent(CbmDigi* digi) {

  assert(digi);
  CbmMatch* match = digi->GetMatch();
  assert(match);
  for (Int_t iLink = 0; iLink < match->GetNofLinks(); iLink++) {

    Int_t file = match->GetLink(iLink).GetFile();
    Int_t event = match->GetLink(iLink).GetEntry();
    if (fEventRange.find(file) == fEventRange.end()) {
      fEventRange[file] = pair<Int_t, Int_t>(event, event);
    } //? First entry for this input
    else {
      Int_t firstEvent = fEventRange[file].first;
      Int_t lastEvent  = fEventRange[file].second;
      if ( event < firstEvent ) {
        firstEvent = event;
        fEventRange[file] = pair<Int_t, Int_t>(firstEvent, lastEvent);
      }
      if ( event > lastEvent ) {
        lastEvent = event;
        fEventRange[file] = pair<Int_t, Int_t>(firstEvent, lastEvent);
      }
    } //? Compare with existing input

  } //# links

}
// ===========================================================================

ClassImp(CbmDaq)

