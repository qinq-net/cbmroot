/** CbmEvent.cxx
 ** @author V.Friese <v.friese@gsi.de>
 ** @since 17.09.2016
 ** @version 1.0
 **/

#include "CbmEvent.h"

#include <iostream>
#include <sstream>
#include "FairLogger.h"



// -----   Add data to event   ---------------------------------------------
void CbmEvent::AddData(Cbm::DataType type, Int_t index) {

	if ( index < 0 || index > 65535 ) {
		LOG(ERROR) << "CbmEvent: index " << index << " out of range (UShort)"
				       << FairLogger::endl;
		return;
	}

	fIndexMap[type].push_back(UShort_t(index));
	fNofData++;

}
// -------------------------------------------------------------------------



// -----   Get a data index   ----------------------------------------------
Int_t CbmEvent::GetIndex(Cbm::DataType type, UInt_t iData) {

	if ( fIndexMap.find(type) == fIndexMap.end() ) return -1;
	if ( fIndexMap[type].size() <= iData ) return -2;
	return fIndexMap.at(type)[iData];

}
// -------------------------------------------------------------------------



// -----   Get number of data of a type in this event   --------------------
Int_t CbmEvent::GetNofData(Cbm::DataType type) const {

	if ( fIndexMap.find(type) == fIndexMap.end() ) return -1;
	else return fIndexMap.at(type).size();

}
// -------------------------------------------------------------------------



// -----   String output   -------------------------------------------------
std::string CbmEvent::ToString() const
{
   std::stringstream ss;
   ss << "Event " << fNumber << " at t = " << fTime
  		<< " ns. Registered data types: " << fIndexMap.size()
  		<< ", data objects: " << fNofData << "\n";
   for (auto it = fIndexMap.begin(); it != fIndexMap.end(); it++) {
  	 ss << "          -- Data type " << it->first
  			<< ", number of data " << it->second.size() << "\n";
   }
  return ss.str();
}
// -------------------------------------------------------------------------



ClassImp(CbmEvent)
