/** @file CbmStsAddress.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 17.05.2013
 **/

#include "CbmStsAddress.h"

#include <cassert>
#include <sstream>
#include "CbmModuleList.h"


namespace CbmStsAddress {

  // -----    Definition of address bit field   ------------------------------
  const Int_t kBits[kCurrentVersion+1][kStsSide+1] =
  {

   // Version 0 (until 23 August 2017)
   {  4,  // system
      4,  // unit / station
      4,  // ladder
      1,  // half-ladder
      3,  // module
      2,  //sensor
      1,  //side
   },

   // Version 1 (current, since 23 August 2017)
   {  4,  // system
      6,  //unit
      5,  // ladder
      1,  // half-ladder
      5,  // module
      4,  //sensor
      1,  //side
   }

  };
  // -------------------------------------------------------------------------


  // -----    Bit shifts -----------------------------------------------------
  const Int_t kShift[kCurrentVersion+1][kStsSide+1] =
  {
   {   0,
       kShift[0][0] + kBits[0][0],
       kShift[0][1] + kBits[0][1],
       kShift[0][2] + kBits[0][2],
       kShift[0][3] + kBits[0][3],
       kShift[0][4] + kBits[0][4],
       kShift[0][5] + kBits[0][5],
   },

   {   0,
       kShift[1][0] + kBits[1][0],
       kShift[1][1] + kBits[1][1],
       kShift[1][2] + kBits[1][2],
       kShift[1][3] + kBits[1][3],
       kShift[1][4] + kBits[1][4],
       kShift[1][5] + kBits[1][5],
   }
  };
  // -------------------------------------------------------------------------


  // -----    Bit masks  -----------------------------------------------------
  const Int_t kMask[kCurrentVersion+1][kStsSide+1] =
  {
   { ( 1 << kBits[0][0] ) -1,
     ( 1 << kBits[0][1] ) -1,
     ( 1 << kBits[0][2] ) -1,
     ( 1 << kBits[0][3] ) -1,
     ( 1 << kBits[0][4] ) -1,
     ( 1 << kBits[0][5] ) -1,
     ( 1 << kBits[0][6] ) -1,
   },
   { ( 1 << kBits[1][0] ) -1,
     ( 1 << kBits[1][1] ) -1,
     ( 1 << kBits[1][2] ) -1,
     ( 1 << kBits[1][3] ) -1,
     ( 1 << kBits[1][4] ) -1,
     ( 1 << kBits[1][5] ) -1,
     ( 1 << kBits[1][6] ) -1,
   }
 };
  // -------------------------------------------------------------------------


} // Namespace CbmStsAddress


// -----   Construct address from element Ids   ------------------------------
Int_t CbmStsAddress::GetAddress(UInt_t unit,
                                UInt_t ladder,
                                UInt_t halfladder,
                                UInt_t module,
                                UInt_t sensor,
                                UInt_t side,
                                UInt_t version) {

  assert ( version <= kCurrentVersion );

  // Catch overrun of allowed ranges
  if ( unit >= ( 1 << kBits[version][kStsUnit] ) ) {
    LOG(ERROR) << "Unit Id "  << unit << " exceeds maximum ("
        << ( 1 << kBits[version][kStsUnit] ) - 1 << ")"
        << FairLogger::endl;
    return 0;
  }
  if ( ladder >= ( 1 << kBits[version][kStsLadder]) ) {
    LOG(ERROR) << "Ladder Id "  << ladder << " exceeds maximum ("
        << ( 1 << kBits[version][kStsLadder] ) - 1 << ")"
        << FairLogger::endl;
    return 0;
  }
  if ( halfladder >= ( 1 << kBits[version][kStsHalfLadder]) ) {
    LOG(ERROR) << "HalfLadder Id "  << halfladder << " exceeds maximum ("
        << ( 1 << kBits[version][kStsHalfLadder] ) - 1 << ")"
        << FairLogger::endl;
    return 0;
  }
  if ( module >= ( 1 << kBits[version][kStsModule]) ) {
    LOG(ERROR) << "Module Id "  << module << " exceeds maximum ("
        << ( 1 << kBits[version][kStsModule] ) - 1 << ")"
        << FairLogger::endl;
    return 0;
  }
  if ( sensor >= ( 1 << kBits[version][kStsSensor]) ) {
    LOG(ERROR) << "Sensor Id "  << sensor << " exceeds maximum ("
        << ( 1 << kBits[version][kStsSensor] ) - 1 << ")"
        << FairLogger::endl;
    return 0;
  }
  if ( side >= ( 1 << kBits[version][kStsSide]) ) {
    LOG(ERROR) << "Side Id "  << side << " exceeds maximum ("
        << ( 1 << kBits[version][kStsSide] ) - 1 << ")"
        << FairLogger::endl;
    return 0;
  }

  return kSts       << kShift[version][kStsSystem]     |
      unit       << kShift[version][kStsUnit   ]    |
      ladder     << kShift[version][kStsLadder]     |
      halfladder << kShift[version][kStsHalfLadder] |
      module     << kShift[version][kStsModule]     |
      sensor     << kShift[version][kStsSensor]     |
      side       << kShift[version][kStsSide];

}
// ---------------------------------------------------------------------------



// -----   Construct address from array of element Ids   ----------------------
Int_t CbmStsAddress::GetAddress(UInt_t* elementId, UInt_t version) {

  assert ( version <= kCurrentVersion );

  ULong64_t address = kSts << kShift[version][kStsSystem];
  for (Int_t level = 1; level <= kStsSide; level++) {
    if ( elementId[level] >= ( 1 << kBits[version][level] ) ) {
      LOG(ERROR) << "Id " << elementId[level] << " for STS level " << level
          << " exceeds maximum (" << (1 << kBits[version][level]) - 1
          << ")" << FairLogger::endl;
      return 0;
    }
    address = address | ( elementId[level] << kShift[version][level] );
  }

  return address;
}
// ---------------------------------------------------------------------------



// -----   Construct address from address of descendant element   -----------
Int_t CbmStsAddress::GetMotherAddress(Int_t address, Int_t level) {
  assert ( level >= kStsSystem && level <= kStsSide );
  UInt_t version = GetVersion(address);
  return ( address & ( ( 1 << kShift[version][level+1] ) - 1 ) ) ;
}
// ---------------------------------------------------------------------------



// -----   Get the index of an element   -------------------------------------
UInt_t CbmStsAddress::GetElementId(Int_t address, Int_t level) {
  assert ( level >= kStsSystem && level <= kStsSide );
  UInt_t version = GetVersion(address);
  return ( address & ( kMask[version][level] << kShift[version][level] ) )
      >> kShift[version][level];
}
// ---------------------------------------------------------------------------



// -----   Get System ID   -------------------------------------------------
UInt_t CbmStsAddress::GetSystemId(Int_t address) {
  return GetElementId(address, kStsSystem);
}
// -------------------------------------------------------------------------



// -----   Get the version number from the address   -------------------------
UInt_t CbmStsAddress::GetVersion(Int_t address) {
  return UInt_t( ( address & ( kVersionMask << kVersionShift ) )
                 >> kVersionShift );
}
// ---------------------------------------------------------------------------



// -----  Construct address by changing the index of an element   ------------
Int_t CbmStsAddress::SetElementId(Int_t address, Int_t level,
                                  UInt_t newId) {
  if ( level < 0 || level > kStsSide ) return address;
  UInt_t version = GetVersion(address);
  if ( newId >= ( 1 << kBits[version][level]) ) {
    LOG(ERROR) << "Id " << newId << " for STS level " << level
        << " exceeds maximum (" << (1 << kBits[version][level]) - 1
        << ")" << FairLogger::endl;
    return 0;
  }
  return ( address & (~ (kMask[version][level] << kShift[version][level]) ) )
      | ( newId << kShift[version][level]);
}
// -------------------------------------------------------------------------



// -----   String output   -------------------------------------------------
std::string CbmStsAddress::ToString(Int_t address) {
  std::stringstream ss;

  ss << "StsAddress: address " << address
      << " (version " << GetVersion(address) << ")"
      << ": system " << GetElementId(address, kStsSystem)
      << ", unit " << GetElementId(address, kStsUnit)
      << ", ladder " << GetElementId(address, kStsLadder)
      << ", half-ladder " << GetElementId(address, kStsHalfLadder)
      << ", module " << GetElementId(address, kStsModule)
      << ", sensor " << GetElementId(address, kStsSensor)
      << ", side " << GetElementId(address, kStsSide);
  return ss.str();
}
// -------------------------------------------------------------------------





