/* Generated by Together */

#include "Scalers_Event.h"

void  Scalers_Event::Clear(Option_t*)
{
   TObject::Clear();
  // all members should be cleared.

      // Oct 12
   fDTimeSinceFirstEventSecondsTriglog = 0.0;
   fDTimeSinceLastEventSecondsTriglog  = 0.0;

   // Starting Nov 12
   fDTimeSinceFirstEventSecondsScalOrMu = 0.0;
   fDTimeSinceLastEventSecondsScalOrMu  = 0.0;

   for( Int_t iScalOrMuIndex = 0; iScalOrMuIndex < SCALORMU_NB_SCAL; iScalOrMuIndex++)
   {
      fDScalOrMuRate[iScalOrMuIndex] = 0.0;
      fDDetectorRate[iScalOrMuIndex] = 0.0;
   }

   for( Int_t iTriglogScaler = 0; iTriglogScaler < NUM_SCALERS; iTriglogScaler++)
      for( Int_t iTriglogChannel = 0; iTriglogChannel < N_SCALERS_CH; iTriglogChannel++)
         fDTriglogRate[iTriglogScaler][iTriglogChannel] = 0.0;
}

