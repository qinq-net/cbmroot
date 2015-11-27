/** @file CbmMCEventList.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 24.11.2015
 **/

#include "CbmMCEventList.h"

#include "FairLogger.h"

using std::map;
using std::pair;

// -----   Constructor   ------------------------------------------------------
CbmMCEventList::CbmMCEventList()
    : fEvents(), fNofEvents(0) {
}
// ----------------------------------------------------------------------------

// -----   Destructor   -------------------------------------------------------
CbmMCEventList::~CbmMCEventList() {
}
// ----------------------------------------------------------------------------

// -----   Print to screen   --------------------------------------------------
void CbmMCEventList::Print(Option_t* opt) const {
  LOG(INFO) << "Number of MC events in list: " << fEvents.size()
	       << FairLogger::endl;
  map<Int_t, map<Int_t, Double_t> >::const_iterator it1 = fEvents.begin();
  while (it1 != fEvents.end()) {
    map<Int_t, Double_t>::const_iterator it2 = (it1->second).begin();
    while (it2 != (it1->second).end()) {
      LOG(DEBUG) << "File " << it1->first << ", event " << it2->first
		    << ", event time " << it2->second << " ns"
		    << FairLogger::endl;
      it2++;
    } //# events
    it1++;
  } //# files
}
// ----------------------------------------------------------------------------

ClassImp(CbmMCEventList)

