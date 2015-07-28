// ------------------------------------------------------------------
// -----                     TofScalerDef                          -----
// -----              Created 21/06/2013 by P.-A. Loizeau       -----
// ------------------------------------------------------------------

#ifndef TOFSCALERDEF_H_
#define TOFSCALERDEF_H_

#include "TString.h"

namespace tofscaler
{
   enum ScalerTypes {
      undef       = 0,  // unknown TDC type, default value
      triglog     = 1,  // TRIGger LOGic on boards of the VULOM family (J. Fruehauf dev.)
      scalormu    = 2,  // Scaler Or Multiplicty on boards of the VULOM family (J. Fruehauf dev.)
      scalormubig = 3,  // Scaler Or Multiplicty on boards of the VULOM family (J. Fruehauf dev.) with more channels
      scaler2014  = 4,  // Scalers on boards of the VULOM family (J. Fruehauf dev.) with 32 input channels and 16 And scaler
      triglogscal = 5,  // TRIGLOG used as Scalers on boards of the VULOM family (J. Fruehauf dev.) with 16 IN, 16 LMU and 16 OUT scalers
      NbScalerTypes   // Nb of Scaler types, require indices to be consecutive (no gap)
   };
   
   // Names for Histos
   // !! Has to be adapted to match in initializer size the number of entries in TdcTypes!!!
   const TString ksTdcHistName[NbScalerTypes] = { "default", "triglog", "scalormu", "scalormubig", "scaler2014", "trigscal"};
   
   // Names for parameters
   // !! Has to be adapted to match in initializer size the number of entries in TdcTypes!!!
   const TString ksTdcParName[NbScalerTypes] = { "Def", "Triglog", "Scalormu", "ScalormuBig", "Scaler2014", "TrigScal"};
}

#endif // TOFSCALERDEF_H_   
