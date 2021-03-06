/**@file CbmTbEvent.cxx
 **@date 26.06.2009
 **@author V. Friese <v.friese@gsi.de>
 **/


#include "CbmTbEvent.h"
#include "CbmDigi.h"
#include "CbmModuleList.h"

#include "FairLogger.h"

#include <iomanip>

using std::fixed;
using std::setprecision;

// -----   Constructor  ------------------------------------------------------ 
CbmTbEvent::CbmTbEvent()
  : FairEventHeader(),
    fTimeStart(0.),          
    fTimeStop(0.),           
    fEpochTime(0.),          
    fNofData(0),            
    fTimeBc(),
    fNofDigis(),
    fNofAux(0),
    fEventType(0),
    fTimeSum(0.)
{
  Clear();
}
// ---------------------------------------------------------------------------


// -----   Add digi to event   ----------------------------------------------
void CbmTbEvent::AddDigi(CbmDigi* digi) {

	// --- Set event start and stop times for empty event.
	if ( IsEmpty() ) {
		fTimeStart = digi->GetTime();
		fTimeStop  = digi->GetTime();
	}

	// --- For non-empty events: check time ordering
	else {
		if ( digi->GetTime() < fTimeStop ) {
			LOG(FATAL) << GetName() << ": time disorder. Event stop is "
			           << fTimeStop << ", digi time is " << digi->GetTime()
			           << FairLogger::endl;
			return;
		}
	}

  // --- Set event stop time
	fTimeStop = digi->GetTime();

	// --- Calculate average event time
	fNofData++;
	fTimeSum += digi->GetTime();
	fEventTime = fTimeSum / Double_t(fNofData);

	// --- Increase digi counter
	if ( digi->GetSystemId() == 999 ) fNofAux++;
	else fNofDigis[digi->GetSystemId()]++;

}
// --------------------------------------------------------------------------








// -----   Reset event   -----------------------------------------------------
void CbmTbEvent::Clear(Option_t*) {
  fEventTime          = 0.;
  fTimeStart     = 0.;
  fTimeStop      = 0.;
  fEpochTime     = 0.;
  fNofData       = 0;
  fNofAux        = 0;
  for (Int_t iSys = 0; iSys < kNofSystems; iSys++) fNofDigis[iSys] = 0;
  fTimeBc.clear();
  fEventType     = 0;
  fTimeSum       = 0.;
 
}
// ---------------------------------------------------------------------------



// -----   Print   -----------------------------------------------------------
void CbmTbEvent::Print(Option_t* /*opt*/) const { 
  LOG(INFO) << "Cbm Testbeam Event: " << setprecision(9) << fixed << "Event time " 
            << fEventTime << " s, duration " << setprecision(0) << GetDuration()
            << " ns, Beam " << fTimeBc.size();
  for (Int_t iSys = 0; iSys < kNofSystems; iSys++) {
  	if ( fNofDigis[iSys] ) {
  		TString systemName = CbmModuleList::GetModuleNameCaps(iSys);
  		LOG(INFO) << ", " << systemName << " " << fNofDigis[iSys];
  	}
  }
  LOG(INFO) << ", AUX " << fNofAux << FairLogger::endl;
}
// ---------------------------------------------------------------------------


ClassImp(CbmTbEvent)







