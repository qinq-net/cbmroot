/** @file CbmMCEventInfo.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 16.06.2018
 **/

#include "CbmMCEventInfo.h"

#include <sstream>

using std::string;
using std::stringstream;


// -----   Constructor   ------------------------------------------------------
CbmMCEventInfo::CbmMCEventInfo(Int_t fileId, Int_t eventId, Double_t time) :
  fFileId(fileId),
  fEventId(eventId),
  fTime(time) {
}
// ----------------------------------------------------------------------------



// -----   Destructor   -------------------------------------------------------
CbmMCEventInfo::~CbmMCEventInfo() {
}
// ----------------------------------------------------------------------------



// -----   Status info   ------------------------------------------------------
string CbmMCEventInfo::ToString() const {
  stringstream ss;
  ss << "File " << fFileId << " Event " << fEventId << " Time " << fTime;
  return ss.str();
}
// ----------------------------------------------------------------------------


ClassImp(CbmMCEventInfo)
