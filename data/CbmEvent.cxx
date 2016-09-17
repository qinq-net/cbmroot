/** CbmEvent.cxx
 ** @author V.Friese <v.friese@gsi.de>
 ** @since 17.09.2016
 ** @version 1.0
 **/

#include "CbmEvent.h"

#include <iostream>
#include <sstream>
#include "FairLogger.h"

//using std::map;
using std::stringstream;
//using std::vector;


// -----   Add data to event   ---------------------------------------------
void CbmEvent::AddData(Cbm::DataType type, Int_t index) {

	if ( index < 0 || index > 65535 ) {
		LOG(ERROR) << "CbmEvent: index " << index << " out of range (UShort)"
				       << FairLogger::endl;
		return;
	}

	fIndexMap[type].push_back(UShort_t(index));

}
// -------------------------------------------------------------------------



// -----   Get a data index   ----------------------------------------------
Int_t CbmEvent::GetIndex(Cbm::DataType type, UInt_t iData) {

	if ( fIndexMap.find(type) == fIndexMap.end() ) return -1;
	if ( fIndexMap[type].size() > iData ) return -1;
	return fIndexMap.at(type)[iData];

}
// -------------------------------------------------------------------------



// -----   Get number of data of a type in this event   --------------------
Int_t CbmEvent::GetNofData(Cbm::DataType type) {

	if ( fIndexMap.find(type) == fIndexMap.end() ) return -1;
	else return fIndexMap.at(type).size();

}
// -------------------------------------------------------------------------



// -----   String output   -------------------------------------------------
std::string CbmEvent::ToString() const
{
   stringstream ss;
   ss << "Event " << fNumber << " at t = " << fTime << " ns."
  		<< " ns. Registered data types: " << fIndexMap.size() << std::endl;
   for (auto it = fIndexMap.begin(); it != fIndexMap.end(); it++) {
  	 ss << "-- Data type " << it->first << ", number of data "
  			 << it->second.size() << std::endl;
  	 it++;
   }
  return ss.str();
}
// -------------------------------------------------------------------------



ClassImp(CbmEvent)
