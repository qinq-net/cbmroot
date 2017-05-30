/** @file CbmDefs.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 30.05.2017
 **
 ** Header for definition of CBM enumerators and constants
 **/

#ifndef CBMDEFS_H
#define CBMDEFS_H 1

#include "Rtypes.h"


/** Enumerator for module Identifiers. Modules can be active (detector systems)
 ** or passive (magnet, beam pipe, target etc.)
 ** In order to loop over all detectors, loop until kNofSystems.
 **/
enum ECbmModuleId {
  kRef,         ///< Reference plane
  kMvd,         ///< Micro-Vertex Detector
  kSts,         ///< Silicon Tracking System
  kRich,        ///< Ring-Imaging Cherenkov Detector
  kMuch,        ///< Muon detection system
  kTrd,         ///< Transition Radiation Detector
  kTof,         ///< Time-of-flight Detector
  kEcal,        ///< EM-Calorimeter
  kPsd,         ///< Projectile spectator detector
  kHodo,        ///< Hodoscope (for test beam times)
  kDummyDet,    ///< Dummy for tutorials or tests
  kNofSystems,  ///< For loops over active systems
  kMagnet,      ///< Magnet
  kTarget,      ///< Target
  kPipe,        ///< Beam pipe
  kShield,      ///< Beam pipe shielding in MUCH section
  kPlatform,    ///< RICH rail platform
  kLastModule   ///< For loops over all modules
};


/** Enumerator for CBM data types **/
enum ECbmDataType {
  kUnknown,
  kMCTrack,
  kStsPoint, kStsDigi, kStsCluster, kStsHit, kStsTrack,  // STS
  kMuchDigi, kMuchPixelHit, kMuchStrawHit, kMuchTrack,   // MUCH
  kTrdDigi, kTrdHit, kTrdTrack,                          // TRD
  kTofDigi, kTofHit, kTofTrack,                          // TOF
  kGlobalTrack                                           // Global
};


/** Global functions for particle masses **/
inline Double_t CbmProtonMass()   { return 0.938272046; }
inline Double_t CbmNeutronMass()  { return 0.939565379; }
inline Double_t CbmElectronMass() { return 0.000510998928; }






#endif
