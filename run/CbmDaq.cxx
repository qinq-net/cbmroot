/** @file CbmDaq.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 20 July 2012
 **/

#include "CbmDaq.h"

#include <cassert>
#include <iomanip>
#include <sstream>
#include <utility>

#include "TClonesArray.h"
#include "FairEventHeader.h"
#include "FairRunAna.h"

#include "CbmMvdDigi.h"
#include "CbmStsDigi.h"
#include "CbmRichDigi.h"
#include "CbmMuchDigi.h"
#include "CbmTrdDigi.h"
#include "CbmTofDigi.h"
#include "CbmPsdDigi.h"


using std::fixed;
using std::map;
using std::pair;
using std::right;
using std::setprecision;
using std::setw;
using std::string;
using std::stringstream;


// =====   Constructor   =====================================================
CbmDaq::CbmDaq(Double_t interval) : FairTask("Daq"),
                   fEventMode(kFALSE),
                   fTimeSliceInterval(interval),
                   fBufferTime(500.),
                   fStoreEmptySlices(kFALSE),
                   fTimeEventPrevious(-1.),
                   fNofEvents(0),
                   fNofDigis(0),
                   fNofTimeSlices(0),
                   fNofTimeSlicesEmpty(0),
                   fTimeDigiFirst(-1.),
                   fTimeDigiLast(-1.),
                   fTimeSliceFirst(-1.),
                   fTimeSliceLast(-1.),
                   fTimer(),
                   fDigis(),
                   fTimeSlice(nullptr),
                   fBuffer(nullptr),
                   fEventList(),
                   fEventsCurrent(nullptr),
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
    LOG(DEBUG) << "----- " << fName << ": Closing " << CurrentTimeSliceInfo()
        << FairLogger::endl;
    fNofTimeSlicesEmpty++;
  }
  else LOG(INFO) << "----- " << fName << ": Closing " << CurrentTimeSliceInfo()
      << FairLogger::endl;

  // --- Fill current time slice into tree (if required)
  if ( fStoreEmptySlices || (!fTimeSlice->IsEmpty()) ) {
    Int_t nMCEvents = CopyEventList();
    LOG(DEBUG) << GetName() << ": " << nMCEvents
        << " MC " << (nMCEvents == 1 ? "event" : "events" )
        << " for this time slice" << FairLogger::endl;
    FairRootManager::Instance()->Fill();
  }

  if ( fEventMode || fTimeSliceInterval < 0. )
    fTimeSliceLast = fTimeSlice->GetStartTime();
  else
    fTimeSliceLast = fTimeSlice->GetEndTime();
  fNofTimeSlices++;
  if ( gLogger->IsLogNeeded(DEBUG) ) PrintCurrentEventRange();

  // --- Reset time slice with new time interval
  Double_t startTime = fTimeSlice->GetStartTime() + fTimeSliceInterval;
  fTimeSlice->Reset(startTime, fTimeSliceInterval);
  fEventRange.clear();
  fEventsCurrent->Clear("");

  // --- Clear data output arrays
  for ( Int_t detector = kMvd; detector < kNofSystems; detector++) {
    if ( fDigis[detector] ) fDigis[detector]->Delete();
  }

}
// ===========================================================================



// =====   Copy event list to output branch   ================================
Int_t CbmDaq::CopyEventList() {

  Int_t nMCEvents = 0;
  for (auto fileIt = fEventRange.begin(); fileIt != fEventRange.end();
      fileIt++) {
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



// =====   Info on current time slice   ======================================
string CbmDaq::CurrentTimeSliceInfo() {
  Bool_t empty = kTRUE;
  stringstream ss;
  if ( fTimeSlice->GetDuration() < 0. )
    ss << "time slice: interval flexible, data: ";
  else
    ss << "timeSlice: interval [" << fixed << setprecision(3)
       << fTimeSlice->GetStartTime() << ", " << fTimeSlice->GetEndTime()
       << "] ns, data: ";
  for (Int_t detector = kMvd; detector < kNofSystems; detector++) {
    if ( fDigis[detector] == nullptr ) continue;
    if ( fDigis[detector]->GetEntriesFast() ) {
      ss << " " << CbmModuleList::GetModuleNameCaps(detector) << " "
          << fDigis[detector]->GetEntriesFast();
      empty = kFALSE;
    } //? digis for detector
  } //# detectors
  if ( empty ) ss << " empty";
  return ss.str();
}
// ===========================================================================



// =====   Task execution   ==================================================
void CbmDaq::Exec(Option_t*) {

  // Start timer and digi counter
  fTimer.Start();
  Int_t nDigis = 0;

  // Event info
  Int_t file  = FairRunAna::Instance()->GetEventHeader()->GetInputFileId();
  Int_t event = FairRootManager::Instance()->GetEntryNr();
  Double_t eventTime = FairRunAna::Instance()->GetEventHeader()->GetEventTime();
  fEventList.Insert(event, file, eventTime);

  // Status
  LOG(DEBUG) << fName << ": Event " << event << " at " << eventTime
      << " ns, previous event time " << fTimeEventPrevious
      << " ns" << FairLogger::endl;
  LOG(DEBUG) << fBuffer->ToString() << FairLogger::endl;

  // Time-based mode
  if ( ! fEventMode ) {

    // Time-slices of equal duration
    if ( fTimeSliceInterval > 0. ) {

      // The time slice can only be filled up to the previous event time
      // because digitizers may generate noise between the previous event
      // time and the current event time.
      Double_t fillTime = fTimeEventPrevious - fBufferTime;

      // Copy data from the buffer to the time slice up to the fill time.
      // If this includes data after the time slice end, close the time
      // slice and open a new one.
      while ( kTRUE ) {

        // Fill time is within time slice: fill data into time slice and exit
        // the loop.
        if ( fillTime < fTimeSlice->GetEndTime() ) {
          nDigis += FillTimeSlice(fillTime);
          break;
        } //? fill time within time slice

        // Fill time is beyond time slice: fill data up to the end of the
        // time slice, close the time slice and start a new one.
        else {
          nDigis += FillTimeSlice(fTimeSlice->GetEndTime());
          CloseTimeSlice();
        }

      } //# time slices

    } //? time-slices of equal duration

    else {   // all data into one time-slice

      // Fill data up to the previous event time minus the buffering interval
      Double_t fillTime = fTimeEventPrevious - fBufferTime;
      nDigis = FillTimeSlice(fillTime);

    } //? all data into one time-slice

  } //? time-based mode


  else {   // event-by-event mode

    // Fill all data from the buffer into the time slice and close the
    // time slice.
    nDigis = FillTimeSlice();
    fTimeSlice->SetStartTime(eventTime);
    if ( fNofEvents == 0 ) fTimeSliceFirst = eventTime;
    CloseTimeSlice();

  } //? event-by-event mode


  // Buffer Status
  LOG(DEBUG) << GetName() << ": " << fBuffer->ToString()
                           << FairLogger::endl;

  // --- Save event time for next execution
  fTimeEventPrevious = eventTime;

  // --- Event log
  fTimer.Stop();
  LOG(INFO) << "+ " << setw(15) << GetName() << ": Event  " << setw(6)
                    << right << event << " at " << fixed
                    << setprecision(3) << eventTime
                    << " ns, " << nDigis << " digis transported. Exec time "
                    << setprecision(6)
                    << fTimer.RealTime() << " s." << FairLogger::endl;

  // --- Increase exec counter
  fNofEvents++;

}
// ===========================================================================



// =====   Fill a data (digi) object into the its output array   =============
void CbmDaq::FillData(CbmDigi* data) {

  Int_t iDet = data->GetSystemId();
  if ( fDigis[iDet] == nullptr ) {
    LOG(FATAL) << GetName() << ": received digi from "
        << CbmModuleList::GetModuleNameCaps(iDet)
        << " but have no corresponding output array!" << FairLogger::endl;
    return;
  }
  Int_t nDigis = fDigis[iDet]->GetEntriesFast();

  switch ( iDet ) {

    case kMvd: {
      CbmMvdDigi* digi = static_cast<CbmMvdDigi*>(data);
      new ( (*(fDigis[kMvd]))[nDigis] ) CbmMvdDigi(*digi);
      fTimeSlice->SetEmpty(kFALSE);
      break;
    }

    case kSts: {
      CbmStsDigi* digi = static_cast<CbmStsDigi*>(data);
      new ( (*(fDigis[kSts]))[nDigis] ) CbmStsDigi(*digi);
      fTimeSlice->SetEmpty(kFALSE);
      break;
    }

    case kRich: {
      CbmRichDigi* digi = static_cast<CbmRichDigi*>(data);
      new ( (*(fDigis[kRich]))[nDigis] ) CbmRichDigi(*digi);
      fTimeSlice->SetEmpty(kFALSE);
      break;
    }

    case kMuch: {
      CbmMuchDigi* digi = static_cast<CbmMuchDigi*>(data);
      new ( (*(fDigis[kMuch]))[nDigis] ) CbmMuchDigi(*digi);
      fTimeSlice->SetEmpty(kFALSE);
      break;
    }

    case kTrd: {
      CbmTrdDigi* digi = static_cast<CbmTrdDigi*>(data);
      new ( (*(fDigis[kTrd]))[nDigis] ) CbmTrdDigi(*digi);
      fTimeSlice->SetEmpty(kFALSE);
      break;
    }

    case kTof: {
      CbmTofDigiExp* digi = static_cast<CbmTofDigiExp*>(data);
      new ( (*(fDigis[kTof]))[nDigis] ) CbmTofDigiExp(*digi);
      fTimeSlice->SetEmpty(kFALSE);
      break;
    }

    default:
      TString sysName = CbmModuleList::GetModuleName(iDet);
      LOG(WARNING) << "CbmDaq: System " << sysName
                   << " is not implemented yet!" << FairLogger::endl;
      break;

  }  // detector switch
}
// ===========================================================================



// =====   Copy all buffer data to the time slice   ==========================
Int_t CbmDaq::FillTimeSlice() {

  Int_t nDigis = 0.;
  LOG(DEBUG) << fName << ": Fill all data into current time slice"
      << FairLogger::endl;

  // --- Loop over all detector systems
  for (Int_t iDet = kRef; iDet < kNofSystems; iDet++) {

    // --- Loop over digis from DaqBuffer and fill them into current time slice
    CbmDigi* digi = fBuffer->GetNextData(iDet);
    while (digi) {

      LOG(DEBUG2) << fName << ": Inserting digi with detector ID "
             << digi->GetAddress() << " at time " << digi->GetTime()
             << " into current time slice" << FairLogger::endl;
      FillData(digi);
      if ( !fNofDigis && !nDigis ) fTimeDigiFirst = digi->GetTime();
      fTimeDigiLast = TMath::Max(fTimeDigiLast, digi->GetTime());
      RegisterEvent(digi);
      nDigis++;

      // --- Delete data and get next one from buffer
      delete digi;
      digi = fBuffer->GetNextData(iDet);
    }  //? Valid digi from buffer

  }  // Detector loop

  LOG(DEBUG) << GetName() << ": Filled " << nDigis << " digis into "
        << CurrentTimeSliceInfo() << FairLogger::endl;
  fNofDigis += nDigis;

  return nDigis;
}
// ===========================================================================



// =====   Fill current time slice with data from buffers   ==================
Int_t CbmDaq::FillTimeSlice(Double_t fillTime) {

  Int_t nDigis = 0;
  LOG(DEBUG) << fName << ": Fill data up to t = " << fillTime
          << " into current time slice" << FairLogger::endl;

  // --- Loop over all detector systems
  for (Int_t iDet = kRef; iDet < kNofSystems; iDet++) {

    // --- Loop over digis from DaqBuffer and fill them into current time slice
    CbmDigi* digi = fBuffer->GetNextData(iDet, fillTime);
    while (digi) {

      LOG(DEBUG2) << fName << ": Inserting digi with detector ID "
		     << digi->GetAddress() << " at time " << digi->GetTime()
		     << " into current time slice" << FairLogger::endl;
      FillData(digi);
      if ( !fNofDigis && !nDigis ) fTimeDigiFirst = digi->GetTime();
      fTimeDigiLast = TMath::Max(fTimeDigiLast, digi->GetTime());
      RegisterEvent(digi);
      nDigis++;

      // --- Delete data and get next one from buffer
      delete digi;
      digi = fBuffer->GetNextData(iDet, fillTime);
    }  //? Valid digi from buffer

  }  // Detector loop

  // Adjust time slice limits in case of event-by-event or single time slice
  if ( fEventMode || fTimeSliceInterval < 0. ) {

  }

  LOG(DEBUG) << GetName() << ": Filled " << nDigis << " digis into "
		<< CurrentTimeSliceInfo() << FairLogger::endl;
  fNofDigis += nDigis;

  return nDigis;
}
// ===========================================================================



// =====   End-of-run action   ===============================================
void CbmDaq::Finish() {

  std::cout << std::endl;
  LOG(DEBUG) << fName << ": End of run" << FairLogger::endl;
  LOG(DEBUG) << fBuffer->ToString() << FairLogger::endl;

  if ( ! fEventMode ) { // time-based mode

    if ( fTimeSliceInterval > 0. ) {  // regular time-slices
      while ( fBuffer->GetSize() ) {  // time slice loop until buffer is empty
        FillTimeSlice(fTimeSlice->GetEndTime());
        CloseTimeSlice();
      } //# time slices
    } //? regular time slices

    else {  // all data into one time-slice
      FillTimeSlice();
      CloseTimeSlice();
    } //? all data into one time-slice

  } //? time-based mode


  else { // event-by-event mode
    if ( fBuffer->GetSize() ) LOG(FATAL) << fName
        << ": non-empty buffer at finish in event-by-event mode!"
        << FairLogger::endl;
  } //? event-by-event mode


  LOG(DEBUG) << fBuffer->ToString()
			           << FairLogger::endl;
  LOG(DEBUG) << fName << ": run finished." << FairLogger::endl;

  std::cout << std::endl;
  LOG(INFO) << "=====================================" << FairLogger::endl;
  LOG(INFO) << GetName() << ": Run summary" << FairLogger::endl;
  LOG(INFO) << "Events:       " << setw(10) << right << fNofEvents
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
  LOG(INFO) << fEventList.ToString() << FairLogger::endl;
//  fEventList.Print();

}
// ===========================================================================



// =====   Task initialisation   =============================================
InitStatus CbmDaq::Init() {

  std::cout << std::endl;
  LOG(INFO) << "=========================================================="
      << FairLogger::endl;
  LOG(INFO) << GetName() << ": Initialisation" << FairLogger::endl;
  LOG(INFO) << FairLogger::endl;
  if ( fEventMode ) LOG(INFO) << fName << ": running in event mode."
      << FairLogger::endl;
  else {
    LOG(INFO) << fName << ": time-slice interval is ";
    if ( fTimeSliceInterval < 0. ) LOG(INFO) << "infinity.";
    else LOG(INFO) << fTimeSliceInterval << " ns.";
    LOG(INFO) << FairLogger::endl;
  } //? time-based mode


  // Register output branch TimeSlice
  fTimeSlice = new CbmTimeSlice();
  FairRootManager::Instance()->Register("TimeSlice.",
                                        "DAQ", fTimeSlice, kTRUE);

  // Register output branch EventList
  fEventsCurrent = new CbmMCEventList();
  FairRootManager::Instance()->Register("MCEventList.", "Event list",
                                        fEventsCurrent, kTRUE);

  // Register output branches (CbmDigi)
  for (Int_t detector = kMvd; detector < kNofSystems; detector++) {
    TString className = "Cbm";
    TString branchName = "";

    switch (detector) {
      case kMvd:  branchName = "MvdDigi";  break;
      case kSts:  branchName = "StsDigi";  break;
      case kRich: branchName = "RichDigi"; break;
      case kMuch: branchName = "MuchDigi"; break;
      case kTrd:  branchName = "TrdDigi";  break;
      case kTof:  branchName = "TofDigiExp";  break;
      case kPsd:  branchName = "PsdDigi";  break;
      default: break;
    } //? detector
    if ( branchName.IsNull() ) continue;
    className += branchName;

    fDigis[detector] = new TClonesArray(className.Data(), 1000);
    FairRootManager::Instance()->Register(branchName.Data(), "",
                                          fDigis[detector],IsOutputBranchPersistent(branchName));
    LOG(INFO) << GetName() << ": Registered branch " << branchName
        << FairLogger::endl;

  } //? systems


  // Get Daq Buffer
  fBuffer = CbmDaqBuffer::Instance();

  // Set initial times
  fTimeEventPrevious = -1.;
  if ( fEventMode ) fTimeSliceInterval = -1.;
  fTimeSlice->Reset(0., fTimeSliceInterval);
  fTimeSliceFirst = -1.;
  fTimeSliceLast  = -1.;


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

  LOG(INFO) << GetName() << ": Current MC event range: ";
  if ( fEventRange.empty() ) {
    LOG(INFO) << "empty" << FairLogger::endl;
    return;
  }
  auto it = fEventRange.begin();
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

