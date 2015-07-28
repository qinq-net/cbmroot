// ------------------------------------------------------------------
// -----                     TGet4v1Rec.cxx                     -----
// -----              Created 22/07/2013 by P.-A. Loizeau       -----
// -----                 Adapted from TGet4v1Rec.cxx            -----
// -----         in RocLib/beamtime/tof-tdctest rev.4862        -----
// ------------------------------------------------------------------

#include "TGet4v1Rec.h"

TimeWindowCond::TimeWindowCond():
   TNamed("", "Time Window Condition"),
   fdLowT (0.0),
   fdHighT(0.0)
{
}
TimeWindowCond::TimeWindowCond( const char* name, Double_t tLow, Double_t tHigh ):
   TNamed( name, "Time Window Condition" ),
   fdLowT (tLow),
   fdHighT(tHigh)
{
}

void TimeWindowCond::SetValues( Double_t tLow, Double_t tHigh )
{
   fdLowT  = tLow;
   fdHighT = tHigh;
}
void TimeWindowCond::Clear(Option_t *t)
{
   TNamed::Clear( t );
   fdLowT  = 0.0;
   fdHighT = 0.0;
}

Bool_t TimeWindowCond::Test( Double_t dValToTest)
{
   if( fdLowT <= dValToTest && dValToTest <= fdHighT)
      return kTRUE;
      else return kFALSE;
}

TGet4v1Rec::TGet4v1Rec() :
   fRocId (0),
   fIter(),
   fMsgTypes        (NULL),
   fSysTypes        (NULL),
   fSysUserTypes    (NULL),
   fAUXch           (NULL),
   fTrigger_All     (NULL),
   fTrigger_AUX     (NULL),
   fTrigger_All_100 (NULL),
   fTrigger_All_us  (NULL),
   fTrigger_All_ms  (NULL),
   fTrigger_All_s   (NULL),
   fTriggerWind     (),
   fAUXWind         (),
   fSelfTrigWind    (),
   fEvntMultipl     (NULL),
   fALLt            (NULL),
   fDATAt           (NULL),
   fERRORt          (NULL),
   fSLOWCt          (NULL),
   fSelfTriggT      (NULL),
   fEPOCHt          (NULL),
   fAllSelectedT    (NULL),
   fAllLongT        (NULL),
   fDataLongT       (NULL),
   fErrorLongT      (NULL),
   fSlowCLongT      (NULL),
   fSelfTriggLongT  (NULL),
   fEpochLongT      (NULL),
   fAllEpoch2LongT  (NULL),
   fAllSelLongT     (NULL),
   fLastTm          (0),
   fLastTriggerTm   (0),
   fStartSyncTm     (0),
   fStopSyncTm      (0),
   fUnprocessedMsg  (),
   fDabcSeparator   (-1),
   bIgnoreData      (kFALSE),
   fIgnoreCnt       (0),
   fIgnoreTime      (0),
   fCollectTime     (0),  
   fuCurrEpoch      (0),
   fuEpochCycle     (0),
   fDistribEpochs      (NULL),
   fDistribEpochs2     (NULL),
   fEpochShiftsPerChip (NULL),
   fEpochShiftsDurationPerChip (NULL),
   fbBufferWithLastFullEpoch (kFALSE),
   fbEpochSinceTrigger (kFALSE),
   fbSelectionRocDone   (kTRUE),
   fDataSelfTriggerPerEvent (NULL),
   fDataSelfTrigDistanceNs  (NULL),
   fDataSelfTrigDistanceUs  (NULL),
   fDataSelfTrigDistanceMs  (NULL),
   fDataSelfTrigDistanceS   (NULL),
   fNbEventsPerMbsEvent     (NULL),
   fNbNotEmptyEventsPerMbsEvent (NULL),
   fChannelsMapping       (NULL),
   fChannelInputMessCount (NULL),
   fChannelMultiplicity   (NULL),
   fdRateEvolutionBinSize (0.0),
   fb24bitsReadoutDetected (kFALSE),
   fb32bitsReadoutDetected (kFALSE),
   fGet4V1SlowControlType(NULL),
   fGet4V1SlowControlScaler(NULL),
   fGet4V1SlowControlDeadTime(NULL),
   fGet4V1SlowControlHamming(NULL),
   fGet4V1SlowControlSeuEvo(NULL),
   fGet4ErrorChip(NULL),
   fGet4ErrorChan(NULL),
   fGet4ErrorPatt(NULL),
   fGet4V1DllLockBit(NULL)
{
/*
   fRocId = 0;

   fAUXch = 0;
   fMsgTypes = 0;
   fSysTypes = 0;
   fSysUserTypes = 0;
   fTrigger_All = 0;
   fTrigger_All_100 = 0;
   fTrigger_All_us = 0;
   fTrigger_All_ms = 0;
   fTrigger_All_s = 0;
   fTriggerWind = 0; // time window to assign hits to event
   fAUXWind = 0; // time window to assign AUX to event
   fEvntMultipl = 0; // event multiplicity in time window after trigger signal

   fLastTm = 0;  //
   fLastTriggerTm = 0;
*/
   for (int n=0;n<get4v10::kiMaxAux;n++) fLastAuxTm[n] = 0;
   for (int n=0;n<get4v10::kiMaxSync;n++) {
      fLastSyncTm[n] = 0;
      fLastSyncId[n] = 0;
   }
/*
   fStartSyncTm = 0;
   fStopSyncTm = 0;
*/
   for (int aux=0;aux<get4v10::kiMaxAux;aux++)
      fAUXt[aux] = 0; // time distribution of aux signals
   for (int n=0;n<get4v10::kiMaxSync;n++)
   {
      fSYNCt[n] = 0; // time distribution of sync signals
      fSyncLongT[n] = 0; // time distribution of sync signals
   }
/*
   fALLt  = 0;
   fDATAt = 0;
   fERRORt = 0;
   fSLOWCt = 0;
   fSelfTriggT = 0;
   fEPOCHt = 0;
   fAllSelectedT = 0;

   // Long duration time distribution (1 min. bin, 10 days length)
   fAllLongT       = 0;
   fDataLongT      = 0;
   fErrorLongT     = 0;
   fSlowCLongT     = 0;
   fSelfTriggLongT = 0;
   fEpochLongT     = 0;
   fAllEpoch2LongT = 0;
   fAllSelLongT    = 0;
*/
   fUnprocessedMsg.clear();
/*
   fDabcSeparator=-1;

   bIgnoreData = kFALSE;
   fIgnoreCnt = 0;
   fIgnoreTime = 0;       // time when ignore message was generated
   fCollectTime = 0;      // time when collect message was generated
*/
   /** GET4 unpacking & pre-processing **/
   /*************************************/

   /* Modified part */

      // Epochs
/*
   fuCurrEpoch = 0;             // current epoch on the roc
   fuEpochCycle = 0;            // Number of time the epoch counter cycled
*/
   for( Int_t iGet4 = 0; iGet4 < get4v10::kiMaxGet4Roc; iGet4++)
   {
      fEPOCH2t[iGet4] = 0;
      fuCurrEpoch2[iGet4] = 0;  // current epoch2 for each get4 chip
      fuEpoch2Cycle[iGet4] = 0; // Number of time the epoch2 counter cycled
      fiEpochShift[iGet4] = 0;
      fuNbShiftedEpochs[iGet4] = 0;
      fEpochShiftsDuration[iGet4] = 0;
      fuEpochWithData[iGet4] = 0; // Data flag for suppressed epoch mode
      fEpSuppBuffer[iGet4].clear(); // temp buffer in supp epoch mode to wait for epoch message
   }
/*
   fDistribEpochs      = 0;
   fDistribEpochs2     = 0;
   fEpochShiftsPerChip = 0;
   fEpochShiftsDurationPerChip = 0;

      // Buffers
   fbBufferWithLastFullEpoch = kFALSE;
*/
   fPrevEpochsBuffer[0].clear(); // buffer of all ROC messages in current epoch and previous one
   fPrevEpochsBuffer[1].clear(); // buffer of all ROC messages in current epoch and previous one
//   fbEpochSinceTrigger = kFALSE;
   for( Int_t iGet4 = 0; iGet4 < get4v10::kiMaxGet4Roc; iGet4++)
   {
      fbBufferWithLastFullEpoch2[iGet4] = kFALSE;
      fPrevEpochs2Buffer[iGet4][0].clear(); // buffer of all get4 messages in current epoch and previous one
      fPrevEpochs2Buffer[iGet4][1].clear(); // buffer of all get4 messages in current epoch and previous one
      bLookInPreviousEpoch[iGet4] = kFALSE;
      bLookInNextEpoch[iGet4] = kFALSE;
      fbEpoch2SinceTrigger[iGet4]= kFALSE;

      for( Int_t iGet4Chan = 0; iGet4Chan < get4v10::kuNbChan; iGet4Chan++)
      {
         fSelectedT[iGet4][iGet4Chan] = 0;
         fTrigger_Get4Channel[iGet4][iGet4Chan] = 0;
         fTriggerMs_Get4Channel[iGet4][iGet4Chan] = 0;
         fTriggerS_Get4Channel[iGet4][iGet4Chan] = 0;
         fbDllFlag[iGet4][iGet4Chan] = kFALSE;

         fbDataTimeOut[iGet4][iGet4Chan] = kFALSE;
         fbDataFallingOut[iGet4][iGet4Chan] = kFALSE;

         fLastExtMess[iGet4][iGet4Chan].Clear();
      } // for( Int_t iGet4Chan = 0; iGet4Chan < get4v10::kuNbChan; iGet4Chan++)
      fGet4V1HitsDistanceNs[iGet4] = 0;
      fGet4V1HitsDistanceUs[iGet4] = 0;
      fGet4V1HitsDistanceMs[iGet4] = 0;
      fGet4V1HitsDistanceS[iGet4]  = 0;
   } // for( Int_t iGet4 = 0; iGet4 < get4v10::kiMaxGet4Roc; iGet4++)
/*
   fb24bitsReadoutDetected = kFALSE;
   fb32bitsReadoutDetected = kFALSE;

      // HitsSelection
   fbSelectionRocDone   = kTRUE;
*/
   for( Int_t iGet4 = 0; iGet4 < get4v10::kiMaxGet4Roc; iGet4++)
   {
      fbSelectionDone[iGet4]      = kTRUE;
   }

      // Triggering on data themselves
/*
   fDataSelfTriggerPerEvent = 0;
   fDataSelfTrigDistanceNs  = 0;
   fDataSelfTrigDistanceUs  = 0;
   fDataSelfTrigDistanceMs  = 0;
   fDataSelfTrigDistanceS   = 0;
*/
   for( Int_t iTrigger = 0; iTrigger < get4v10::kiMaxNbDataTriggs; iTrigger++)
   {
      fuDataSelfTriggerCount[iTrigger] = 0;
      fdLastFullTimeSelfTrigger[iTrigger] = 0;
      fextMessLastMainChannel[iTrigger].Clear();
      fdLastFullTimeMainChannelTot[iTrigger]  = 0;
      for( Int_t iSecTriggerChan = 0; iSecTriggerChan < get4v10::kiMaxNbSecTriggCh; iSecTriggerChan++)
      {
         fdLastFullTimeSecChannel[iTrigger][iSecTriggerChan]  = 0;
         fdLastFullTimeSecChannelTot[iTrigger][iSecTriggerChan]  = 0;
      }
   }

   // Event statistics
/*
   fNbEventsPerMbsEvent = 0;
   fNbNotEmptyEventsPerMbsEvent = 0;
   fChannelsMapping = 0;
   fChannelInputMessCount = 0;
   fChannelMultiplicity = 0;
   fdRateEvolutionBinSize = 0;
*/
   for( Int_t iGet4 = 0; iGet4 < get4v10::kiMaxGet4Roc; iGet4++)
   {
      fuNbHitsChipEpoch[iGet4] = 0;
      fChipRateEvolution[iGet4]    = 0;
      for( Int_t iGet4Chan = 0; iGet4Chan < get4v10::kuNbChan; iGet4Chan++)
      {
         fChannelRateEvolution[iGet4][iGet4Chan] = 0;
      } // for( Int_t iGet4Chan = 0; iGet4Chan < get4v10::kuNbChan; iGet4Chan++)
   } //  for( Int_t iGet4 = 0; iGet4 < get4v10::kiMaxGet4Roc; iGet4++)

}
TGet4v1Rec::TGet4v1Rec(const TGet4v1Rec& src):
   fRocId           (src.fRocId),
   fIter            (src.fIter),
   fMsgTypes        (src.fMsgTypes),
   fSysTypes        (src.fSysTypes),
   fSysUserTypes    (src.fSysUserTypes),
   fAUXch           (src.fAUXch),
   fTrigger_All     (src.fTrigger_All),
   fTrigger_AUX     (src.fTrigger_AUX),
   fTrigger_All_100 (src.fTrigger_All_100),
   fTrigger_All_us  (src.fTrigger_All_us),
   fTrigger_All_ms  (src.fTrigger_All_ms),
   fTrigger_All_s   (src.fTrigger_All_s),
   fTriggerWind     (src.fTriggerWind),
   fAUXWind         (src.fAUXWind),
   fSelfTrigWind    (src.fSelfTrigWind),
   fEvntMultipl     (src.fEvntMultipl),
   fALLt            (src.fALLt),
   fDATAt           (src.fDATAt),
   fERRORt          (src.fERRORt),
   fSLOWCt          (src.fSLOWCt),
   fSelfTriggT      (src.fSelfTriggT),
   fEPOCHt          (src.fEPOCHt),
   fAllSelectedT    (src.fAllSelectedT),
   fAllLongT        (src.fAllLongT),
   fDataLongT       (src.fDataLongT),
   fErrorLongT      (src.fErrorLongT),
   fSlowCLongT      (src.fSlowCLongT),
   fSelfTriggLongT  (src.fSelfTriggLongT),
   fEpochLongT      (src.fEpochLongT),
   fAllEpoch2LongT  (src.fAllEpoch2LongT),
   fAllSelLongT     (src.fAllSelLongT),
   fLastTm          (src.fLastTm),
   fLastTriggerTm   (src.fLastTriggerTm),
   fStartSyncTm     (src.fStartSyncTm),
   fStopSyncTm      (src.fStopSyncTm),
   fUnprocessedMsg  (src.fUnprocessedMsg),
   fDabcSeparator   (src.fDabcSeparator),
   bIgnoreData      (src.bIgnoreData),
   fIgnoreCnt       (src.fIgnoreCnt),
   fIgnoreTime      (src.fIgnoreTime),
   fCollectTime     (src.fCollectTime), 
   fuCurrEpoch      (src.fuCurrEpoch),
   fuEpochCycle        (src.fuEpochCycle),
   fDistribEpochs      (src.fDistribEpochs),
   fDistribEpochs2     (src.fDistribEpochs2),
   fEpochShiftsPerChip (src.fEpochShiftsPerChip),
   fEpochShiftsDurationPerChip (src.fEpochShiftsDurationPerChip),
   fbBufferWithLastFullEpoch (src.fbBufferWithLastFullEpoch),
   fbEpochSinceTrigger       (src.fbEpochSinceTrigger),
   fbSelectionRocDone        (src.fbSelectionRocDone),
   fDataSelfTriggerPerEvent  (src.fDataSelfTriggerPerEvent),
   fDataSelfTrigDistanceNs   (src.fDataSelfTrigDistanceNs),
   fDataSelfTrigDistanceUs   (src.fDataSelfTrigDistanceUs),
   fDataSelfTrigDistanceMs   (src.fDataSelfTrigDistanceMs),
   fDataSelfTrigDistanceS    (src.fDataSelfTrigDistanceS),
   fNbEventsPerMbsEvent      (src.fNbEventsPerMbsEvent),
   fNbNotEmptyEventsPerMbsEvent (src.fNbNotEmptyEventsPerMbsEvent),
   fChannelsMapping          (src.fChannelsMapping),
   fChannelInputMessCount    (src.fChannelInputMessCount),
   fChannelMultiplicity      (src.fChannelMultiplicity),
   fdRateEvolutionBinSize    (src.fdRateEvolutionBinSize),
   fb24bitsReadoutDetected   (src.fb24bitsReadoutDetected),
   fb32bitsReadoutDetected   (src.fb32bitsReadoutDetected),
   fGet4V1SlowControlType    (src.fGet4V1SlowControlType),
   fGet4V1SlowControlScaler  (src.fGet4V1SlowControlScaler),
   fGet4V1SlowControlDeadTime(src.fGet4V1SlowControlDeadTime),
   fGet4V1SlowControlHamming (src.fGet4V1SlowControlHamming),
   fGet4V1SlowControlSeuEvo  (src.fGet4V1SlowControlSeuEvo),
   fGet4ErrorChip            (src.fGet4ErrorChip),
   fGet4ErrorChan            (src.fGet4ErrorChan),
   fGet4ErrorPatt            (src.fGet4ErrorPatt),
   fGet4V1DllLockBit         (src.fGet4V1DllLockBit)
{
   
   for (int n=0;n<get4v10::kiMaxAux;n++) 
      fLastAuxTm[n] = src.fLastAuxTm[n] ;
   for (int n=0;n<get4v10::kiMaxSync;n++) {
      fLastSyncTm[n] = src.fLastSyncTm[n];
      fLastSyncId[n] = src.fLastSyncId[n];
   } // for (int n=0;n<get4v10::kiMaxSync;n++)
   
   for (int aux=0;aux<get4v10::kiMaxAux;aux++)
      fAUXt[aux] = src.fAUXt[aux];
   for (int n=0;n<get4v10::kiMaxSync;n++)
   {
      fSYNCt[n]     = src.fSYNCt[n] ;
      fSyncLongT[n] = src.fSyncLongT[n];
   } // for (int n=0;n<get4v10::kiMaxSync;n++)
   
   /** GET4 unpacking & pre-processing **/
   /*************************************/
   
   for( Int_t iGet4 = 0; iGet4 < get4v10::kiMaxGet4Roc; iGet4++)
   {
      fEPOCH2t[iGet4]             = src.fEPOCH2t[iGet4];
      fuCurrEpoch2[iGet4]         = src.fuCurrEpoch2[iGet4];
      fuEpoch2Cycle[iGet4]        = src.fuEpoch2Cycle[iGet4];
      fiEpochShift[iGet4]         = src.fiEpochShift[iGet4];
      fuNbShiftedEpochs[iGet4]    = src.fuNbShiftedEpochs[iGet4];
      fEpochShiftsDuration[iGet4] = src.fEpochShiftsDuration[iGet4];
      fuEpochWithData[iGet4]      = src.fuEpochWithData[iGet4];
      fEpSuppBuffer[iGet4]        = src.fEpSuppBuffer[iGet4];
   } // for( Int_t iGet4 = 0; iGet4 < get4v10::kiMaxGet4Roc; iGet4++)
   
   fPrevEpochsBuffer[0] = src.fPrevEpochsBuffer[0]; 
   fPrevEpochsBuffer[1] = src.fPrevEpochsBuffer[1]; 
   for( Int_t iGet4 = 0; iGet4 < get4v10::kiMaxGet4Roc; iGet4++)
   {
      fbBufferWithLastFullEpoch2[iGet4]  = src.fbBufferWithLastFullEpoch2[iGet4]; 
      fPrevEpochs2Buffer[iGet4][0] = src.fPrevEpochs2Buffer[iGet4][0]; 
      fPrevEpochs2Buffer[iGet4][1] = src.fPrevEpochs2Buffer[iGet4][1]; 
      bLookInPreviousEpoch[iGet4]  = src.bLookInPreviousEpoch[iGet4]; 
      bLookInNextEpoch[iGet4]      = src.bLookInNextEpoch[iGet4]; 
      fbEpoch2SinceTrigger[iGet4]  = src.fbEpoch2SinceTrigger[iGet4]; 

      for( Int_t iGet4Chan = 0; iGet4Chan < get4v10::kuNbChan; iGet4Chan++)
      {
         fSelectedT[iGet4][iGet4Chan]             = src.fSelectedT[iGet4][iGet4Chan]; 
         fTrigger_Get4Channel[iGet4][iGet4Chan]   = src.fTrigger_Get4Channel[iGet4][iGet4Chan]; 
         fTriggerMs_Get4Channel[iGet4][iGet4Chan] = src.fTriggerMs_Get4Channel[iGet4][iGet4Chan]; 
         fTriggerS_Get4Channel[iGet4][iGet4Chan]  = src.fTriggerS_Get4Channel[iGet4][iGet4Chan]; 
         fbDllFlag[iGet4][iGet4Chan]              = src.fbDllFlag[iGet4][iGet4Chan]; 

         fbDataTimeOut[iGet4][iGet4Chan]          = src.fbDataTimeOut[iGet4][iGet4Chan]; 
         fbDataFallingOut[iGet4][iGet4Chan]       = src.fbDataFallingOut[iGet4][iGet4Chan]; 

         fLastExtMess[iGet4][iGet4Chan]           = src.fLastExtMess[iGet4][iGet4Chan]; 
      } // for( Int_t iGet4Chan = 0; iGet4Chan < get4v10::kuNbChan; iGet4Chan++)
      fGet4V1HitsDistanceNs[iGet4] = src.fGet4V1HitsDistanceNs[iGet4]; 
      fGet4V1HitsDistanceUs[iGet4] = src.fGet4V1HitsDistanceUs[iGet4]; 
      fGet4V1HitsDistanceMs[iGet4] = src.fGet4V1HitsDistanceMs[iGet4]; 
      fGet4V1HitsDistanceS[iGet4]  = src.fGet4V1HitsDistanceS[iGet4]; 
   } // for( Int_t iGet4 = 0; iGet4 < get4v10::kiMaxGet4Roc; iGet4++)
   
   for( Int_t iGet4 = 0; iGet4 < get4v10::kiMaxGet4Roc; iGet4++)
   {
      fbSelectionDone[iGet4] = src.fbSelectionDone[iGet4]; 
   } // for( Int_t iGet4 = 0; iGet4 < get4v10::kiMaxGet4Roc; iGet4++)
   
   for( Int_t iTrigger = 0; iTrigger < get4v10::kiMaxNbDataTriggs; iTrigger++)
   {
      fuDataSelfTriggerCount[iTrigger]       = src.fuDataSelfTriggerCount[iTrigger]; 
      fdLastFullTimeSelfTrigger[iTrigger]    = src.fdLastFullTimeSelfTrigger[iTrigger]; 
      fextMessLastMainChannel[iTrigger]      = src.fextMessLastMainChannel[iTrigger]; 
      fdLastFullTimeMainChannelTot[iTrigger] = src.fdLastFullTimeMainChannelTot[iTrigger]; 
      for( Int_t iSecTriggerChan = 0; iSecTriggerChan < get4v10::kiMaxNbSecTriggCh; iSecTriggerChan++)
      {
         fdLastFullTimeSecChannel[iTrigger][iSecTriggerChan]    = src.fdLastFullTimeSecChannel[iTrigger][iSecTriggerChan]; 
         fdLastFullTimeSecChannelTot[iTrigger][iSecTriggerChan] = src.fdLastFullTimeSecChannelTot[iTrigger][iSecTriggerChan]; 
      } // for( Int_t iSecTriggerChan = 0; iSecTriggerChan < get4v10::kiMaxNbSecTriggCh; iSecTriggerChan++)
   } // for( Int_t iTrigger = 0; iTrigger < get4v10::kiMaxNbDataTriggs; iTrigger++)
   
   for( Int_t iGet4 = 0; iGet4 < get4v10::kiMaxGet4Roc; iGet4++)
   {
      fuNbHitsChipEpoch[iGet4]  = src.fuNbHitsChipEpoch[iGet4]; 
      fChipRateEvolution[iGet4] = src.fChipRateEvolution[iGet4]; 
      for( Int_t iGet4Chan = 0; iGet4Chan < get4v10::kuNbChan; iGet4Chan++)
      {
         fChannelRateEvolution[iGet4][iGet4Chan] = src.fChannelRateEvolution[iGet4][iGet4Chan]; 
      } // for( Int_t iGet4Chan = 0; iGet4Chan < get4v10::kuNbChan; iGet4Chan++)
   } //  for( Int_t iGet4 = 0; iGet4 < get4v10::kiMaxGet4Roc; iGet4++)
}

ULong64_t TGet4v1Rec::GetFullEpochNumber()
{
   return (ULong64_t)(fuCurrEpoch) +
           ( (ULong64_t)(get4v10::kulMainEpochCycleSz) +1)*(ULong64_t)(fuEpochCycle);
}
ULong64_t TGet4v1Rec::GetFullEpoch2Number( UInt_t uGet4Id)
{
   return (ULong64_t)(fuCurrEpoch2[uGet4Id]) +
           ( (ULong64_t)(get4v10::kulGet4EpochCycleSz) +1)*(ULong64_t)(fuEpoch2Cycle[uGet4Id]);
}

// Functions to overcome the 32 bit limit in epoch number as input to
// GetMessageFullTime function

ULong64_t TGet4v1Rec::GetSuperCycleEpOffset( )
{
   // Nb of epoch lost by MessageTime function due to 32b limit
//   ULong64_t ulNb32bCycles = GetFullEpochNumber() / ( 0x100000000ULL );
   ULong64_t ulNb32bCycles = GetFullEpochNumber() >> 32;
   if( 0 < ulNb32bCycles)
      return (ULong64_t)( ulNb32bCycles * ( 0x100000000ULL ) )<<14; // epoch nb lost due to 32 bit limit, shifted in ns
   else return 0;
}
ULong64_t TGet4v1Rec::GetSuperCycleEp2Offset( UInt_t uGet4Id)
{
   // Nb of epoch lost by MessageTime function due to 32b limit
//   ULong64_t ulNb32bCycles = GetFullEpoch2Number( uGet4Id) / ( 0x100000000ULL );
   ULong64_t ulNb32bCycles = GetFullEpoch2Number( uGet4Id) >> 32;
   if( 0 < ulNb32bCycles)
      return (ULong64_t)( ulNb32bCycles * ( 0x100000000ULL ) )* // epoch nb lost due to 32 bit limit
              (ULong64_t)( get4tdc::kdEpochInPs / 1000 ); // Get4 epoch in ns
   return 0;
}
Double_t TGet4v1Rec::GetSuperCycleEp2OffsetD( UInt_t uGet4Id)
{
   // Nb of epoch lost by MessageTime function due to 32b limit
//   ULong64_t ulNb32bCycles = GetFullEpoch2Number( uGet4Id) / ( 0x100000000ULL );
   ULong64_t ulNb32bCycles = GetFullEpoch2Number( uGet4Id) >> 32;
   if( 0 < ulNb32bCycles)
      return (Double_t)( ulNb32bCycles * ( 0x100000000ULL ) )* // epoch nb lost due to 32 bit limit
              (Double_t)( get4tdc::kdEpochInPs) / 1000.0 ; // Get4 epoch in ns
   return 0.0;
}
