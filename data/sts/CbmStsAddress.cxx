/** @file CbmStsAddress.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 17.05.2013
 **/


#include "CbmStsAddress.h"

using std::string;


// -----    Definition of the address field   -------------------------------
const Int_t CbmStsAddress::fgkBits[] = { fgkSystemBits,   // system = kSTS
                                         4,   // unit
                                         4,   // ladder
                                         1,   // half-ladder
                                         3,   // module
                                         2,   // sensor
                                         1,   // side
                                        13 }; // channel
// -------------------------------------------------------------------------



// -----    Initialisation of bit shifts -----------------------------------
const Int_t CbmStsAddress::fgkShift[] =
   { 0,
     CbmStsAddress::fgkShift[0] + CbmStsAddress::fgkBits[0],
     CbmStsAddress::fgkShift[1] + CbmStsAddress::fgkBits[1],
     CbmStsAddress::fgkShift[2] + CbmStsAddress::fgkBits[2],
     CbmStsAddress::fgkShift[3] + CbmStsAddress::fgkBits[3],
     CbmStsAddress::fgkShift[4] + CbmStsAddress::fgkBits[4],
     CbmStsAddress::fgkShift[5] + CbmStsAddress::fgkBits[5],
     CbmStsAddress::fgkShift[6] + CbmStsAddress::fgkBits[6] };
// -------------------------------------------------------------------------



// -----    Initialisation of bit masks  -----------------------------------
const Int_t CbmStsAddress::fgkMask[] = { ( 1 << fgkBits[0] ) -1,
                                         ( 1 << fgkBits[1] ) -1,
                                         ( 1 << fgkBits[2] ) -1,
                                         ( 1 << fgkBits[3] ) -1,
                                         ( 1 << fgkBits[4] ) -1,
                                         ( 1 << fgkBits[5] ) -1,
                                         ( 1 << fgkBits[6] ) -1,
                                         ( 1 << fgkBits[7] ) -1  };
// -------------------------------------------------------------------------



// -----  Unique element address   -----------------------------------------
UInt_t CbmStsAddress::GetAddress(Int_t unit,
                                 Int_t ladder,
                                 Int_t halfladder,
                                 Int_t module,
                                 Int_t sensor,
                                 Int_t side,
                                 Int_t channel) {

  // Catch overrun of allowed ranges
  if ( unit >= ( 1 << fgkBits[kStsUnit] ) ) {
    LOG(ERROR) << "Unit Id "  << unit << " exceeds maximum ("
               << ( 1 << fgkBits[kStsUnit] ) - 1 << ")"
               << FairLogger::endl;
    return 0;
  }
  if ( ladder >= ( 1 << fgkBits[kStsLadder]) ) {
    LOG(ERROR) << "Ladder Id "  << ladder << " exceeds maximum ("
               << ( 1 << fgkBits[kStsLadder] ) - 1 << ")"
               << FairLogger::endl;
    return 0;
  }
  if ( halfladder >= ( 1 << fgkBits[kStsHalfLadder]) ) {
    LOG(ERROR) << "HalfLadder Id "  << halfladder << " exceeds maximum ("
               << ( 1 << fgkBits[kStsHalfLadder] ) - 1 << ")"
               << FairLogger::endl;
    return 0;
  }
  if ( module >= ( 1 << fgkBits[kStsModule]) ) {
    LOG(ERROR) << "Module Id "  << module << " exceeds maximum ("
               << ( 1 << fgkBits[kStsModule] ) - 1 << ")"
               << FairLogger::endl;
    return 0;
  }
  if ( sensor >= ( 1 << fgkBits[kStsSensor]) ) {
    LOG(ERROR) << "Sensor Id "  << sensor << " exceeds maximum ("
               << ( 1 << fgkBits[kStsSensor] ) - 1 << ")"
               << FairLogger::endl;
    return 0;
  }
  if ( side >= ( 1 << fgkBits[kStsSide]) ) {
    LOG(ERROR) << "Side Id "  << side << " exceeds maximum ("
               << ( 1 << fgkBits[kStsSide] ) - 1 << ")"
               << FairLogger::endl;
    return 0;
  }
  if ( channel >= ( 1 << fgkBits[kStsChannel]) ) {
    LOG(ERROR) << "Channel Id "  << channel << " exceeds maximum ("
               << ( 1 << fgkBits[kStsChannel] ) - 1 << ")"
               << FairLogger::endl;
    return 0;
  }

  return kSts       << fgkShift[kStsSystem]     |
         unit       << fgkShift[kStsUnit   ]    |
         ladder     << fgkShift[kStsLadder]     |
         halfladder << fgkShift[kStsHalfLadder] |
         module     << fgkShift[kStsModule]     |
         sensor     << fgkShift[kStsSensor]     |
         side       << fgkShift[kStsSide]       |
         channel    << fgkShift[kStsChannel];

}
// -------------------------------------------------------------------------



// -----  Unique element address   -----------------------------------------
UInt_t CbmStsAddress::GetAddress(Int_t* elementId) {

  UInt_t address = kSts << fgkShift[kStsSystem];
  for (Int_t level = 1; level < kStsNofLevels; level++) {
    if ( elementId[level] >= ( 1 << fgkBits[level] ) ) {
      LOG(ERROR) << "Id " << elementId[level] << " for STS level " << level
                 << " exceeds maximum (" << (1 << fgkBits[level]) - 1
                 << ")" << FairLogger::endl;
      return 0;
    }
    address = address | ( elementId[level] << fgkShift[level] );
  }

  return address;
}
// -------------------------------------------------------------------------



// -----   Info   ----------------------------------------------------------
string CbmStsAddress::ToString() const {
   std::stringstream ss;
   ss << "Number of STS levels is " << kStsNofLevels
       << FairLogger::endl;
   for (Int_t level = 0; level < kStsNofLevels; level++)
     ss << "Level " << std::setw(2) << std::right << level
               << ": bits " << std::setw(2) << fgkBits[level]
               << ", max. range " << std::setw(6) << fgkMask[level]
               << FairLogger::endl;
   return ss.str();
}
// -------------------------------------------------------------------------


ClassImp(CbmStsAddress)

