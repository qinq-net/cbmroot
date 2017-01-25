// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmTofStarData                             -----
// -----               Created 13.01.2017 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmTofStarData.h"

#include <algorithm>

/********************** CbmTofStarTrigger *****************************/
CbmTofStarTrigger::CbmTofStarTrigger( ULong64_t ulGdpbTsFullIn, ULong64_t ulStarTsFullIn,
                     UInt_t    uStarTokenIn,   UInt_t    uStarDaqCmdIn, 
                     UInt_t    uStarTrigCmdIn ) :
   fulGdpbTsFull( ulGdpbTsFullIn ),
   fulStarTsFull( ulStarTsFullIn ),
   fuStarToken(   uStarTokenIn ),
   fusStarDaqCmd(  uStarDaqCmdIn) ,
   fusStarTrigCmd( uStarTrigCmdIn )
{
}
//! strict weak ordering operator, assumes same TS cycle for both triggers
bool CbmTofStarTrigger::operator<(const CbmTofStarTrigger& other) const
{
   ULong64_t uThisTs  = this->GetFullGdpbTs();
   ULong64_t uOtherTs = other.GetFullGdpbTs();

   return uThisTs < uOtherTs;
}
UInt_t CbmTofStarTrigger::GetStarTrigerWord() const
{
   // trg_cmd|daq_cmd|tkn_hi|tkn_mid|tkn_lo
   UInt_t uTrigWord =  ( (fusStarTrigCmd & 0x00F) << 16 )
                     + ( (fusStarDaqCmd  & 0x00F) << 12 )
                     + ( (fuStarToken    & 0xFFF)       );
   return uTrigWord;
}
UInt_t CbmTofStarTrigger::GetFullGdpbEpoch() const
{
   // trg_cmd|daq_cmd|tkn_hi|tkn_mid|tkn_lo
   UInt_t uEpochIndex =  (fulGdpbTsFull >> get4v1x::kuCtSize) & 0x7FFFFFFF;
   return uEpochIndex;
}
//ClassImp(CbmTofStarTrigger)
/**********************************************************************/
/********************** CbmTofStarSubevent ****************************/
CbmTofStarSubevent::CbmTofStarSubevent() :
   fbTriggerSet( kFALSE ),
   fTrigger( CbmTofStarTrigger( 0, 0, 0, 0, 0) ),
   fuEventStatusFlags( 0 ),
   fvMsgBuffer()
{
}
CbmTofStarSubevent::CbmTofStarSubevent( CbmTofStarTrigger triggerIn ) :
   fbTriggerSet( kTRUE ),
   fTrigger( triggerIn ),
   fuEventStatusFlags( 0 ),
   fvMsgBuffer()
{
}
CbmTofStarSubevent::~CbmTofStarSubevent()
{
   ClearSubEvent();
}
void CbmTofStarSubevent::ClearSubEvent()
{
   fbTriggerSet = kFALSE;
   fuEventStatusFlags = 0;
   fvMsgBuffer.clear();
}
void * CbmTofStarSubevent::BuildOutput( Int_t & iOutputSizeBytes )
{
   // If trigger not set, something is wrong, we just pass
   if( kFALSE == fbTriggerSet )
   {
      iOutputSizeBytes = 0;
      return NULL;
   } // if( kFALSE == fbTriggerSet )
   
   iOutputSizeBytes = 0;
   
   // Size of output is
   // 3 * Long64 for Header Star Token Info
   // 1 * Long64 for Header Event Status Flags (version, bad/good, ...)
   // n * Long64 for the n messages in Buffer: 1 Epoch + messages + 1 extra epoch in-between if needed
   iOutputSizeBytes = 4 + fvMsgBuffer.size();
   iOutputSizeBytes *= sizeof( ULong64_t );
   UInt_t uMsgsToRead = fvMsgBuffer.size();
   
   // Check if max size passed 
   // If it is the case, limit the number of data read + set Bad Event flag
   if( kuMaxOutputSize < iOutputSizeBytes )
   {
      iOutputSizeBytes = kuMaxOutputSize;
      uMsgsToRead      = kuMaxNbMsgs;
      fuEventStatusFlags |= kulFlagBadEvt;
   } // if( kuMaxOutputSize < iOutputSizeBytes )
   
   // Fills header info
   fpulBuff[0] = fTrigger.GetFullGdpbTs();
   fpulBuff[1] = fTrigger.GetFullStarTs();
   fpulBuff[2] =  (static_cast< ULong64_t >( fTrigger.GetStarToken() )  << 32)
                + (static_cast< ULong64_t >( fTrigger.GetStarDaqCmd() ) << 16)
                + (static_cast< ULong64_t >( fTrigger.GetStarTrigCmd() )     );
   fpulBuff[3] = static_cast< ULong64_t >(fuEventStatusFlags);
  
   // does not work due to "error: cannot convert ‘ngdpb::Message’ to ‘long unsigned int’ in assignment"
//  std::copy( fvMsgBuffer.begin(), fvMsgBuffer.begin() + uMsgsToRead, pulBuff + 4 );

   // Unoptimized replacement: item by item copy
   for( UInt_t uMsgIdx = 0; uMsgIdx < uMsgsToRead; uMsgIdx++)
      fpulBuff[4 + uMsgIdx] = fvMsgBuffer[uMsgIdx].getData();
   
   return static_cast< void * >( fpulBuff );
}
//ClassImp(CbmTofStarTrigger)
/**********************************************************************/
