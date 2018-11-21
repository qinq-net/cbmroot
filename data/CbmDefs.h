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
  kMvdPoint, kMvdDigi, kMvdCluster, kMvdHit,             // MVD
  kStsPoint, kStsDigi, kStsCluster, kStsHit, kStsTrack,  // STS
  kRichPoint, kRichDigi, kRichHit, kRichRing,            // RICH
  kMuchDigi, kMuchPixelHit, kMuchStrawHit, kMuchTrack,   // MUCH
  kTrdDigi, kTrdHit, kTrdTrack,                          // TRD
  kTofPoint, kTofDigi, kTofCalDigi, kTofHit, kTofTrack,  // TOF
  kPsdPoint, kPsdDigi,                                   // PSD
  kGlobalTrack                                           // Global
};


/** Global functions for particle masses **/
inline Double_t CbmProtonMass()   { return 0.938272046; }
inline Double_t CbmNeutronMass()  { return 0.939565379; }
inline Double_t CbmElectronMass() { return 0.000510998928; }


namespace Cbm {


  /** @enumerator ETreeAccess
   ** @brief Mode to read entries from a ROOT TTree
   **
   ** kRegular: Incremental; start with first entry; stop with last entry
   ** kRepeat:  Incremental; start with first entry; after last entry jump
   **           to first entry
   ** kRandom:  Random choice of entries between first and last one.
   **/
  enum ETreeAccess {
    kRegular, kRepeat, kRandom
  };



}



#endif
