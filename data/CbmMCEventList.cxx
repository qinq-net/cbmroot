/** @file CbmMCEventList.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 24.11.2015
 **/

#include "CbmMCEventList.h"

#include <sstream>
#include "FairLogger.h"


using std::map;
using std::pair;
using std::string;
using std::stringstream;


// -----   Constructor   ------------------------------------------------------
CbmMCEventList::CbmMCEventList()
    : TNamed("MCEventList", "List of MC events"), fEvents()
{
}
// ----------------------------------------------------------------------------



// -----   Destructor   -------------------------------------------------------
CbmMCEventList::~CbmMCEventList() {
}
// ----------------------------------------------------------------------------



// -----   Get start time of a MC event   -------------------------------------
Double_t CbmMCEventList::GetEventTime(Int_t event, Int_t file) const {
  auto it1 = fEvents.find(file);
  if ( it1 == fEvents.end() ) return -1.;
  auto it2 = it1->second.find(event);
  if ( it2 == it1->second.end() ) return -1.;
  return it2->second;
}
// ----------------------------------------------------------------------------



// -----  Get number of events in the list   ----------------------------------
Int_t CbmMCEventList::GetNofEvents() const {
  Int_t nEvents = 0;
  for (auto it = fEvents.begin(); it != fEvents.end(); it++)
    nEvents += it->second.size();
  return nEvents;
}
// ----------------------------------------------------------------------------



// -----   Status info   ------------------------------------------------------
string CbmMCEventList::ToString() const {
  stringstream ss;
  ss << fName << ": " << GetNofEvents() << " MC events in list\n";
  if ( FairLogger::GetLogger()->IsLogNeeded(DEBUG) ) {
    for (auto it1 = fEvents.begin(); it1 != fEvents.end(); it1++) {
      for (auto it2 = it1->second.begin(); it2 != it1->second.end(); it2++) {
        ss << "File " << it1->first << ", event " << it2->first
            << ", event time " << it2->second << " ns\n";
      } //# events
    } // #files
  } //? debug
  return ss.str();
}
// ----------------------------------------------------------------------------


ClassImp(CbmMCEventList)

