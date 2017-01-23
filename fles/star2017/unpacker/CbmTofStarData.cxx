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
//ClassImp(CbmTofStarTrigger)
/**********************************************************************/
/********************** CbmTofStarSubevent ****************************/
CbmTofStarSubevent::CbmTofStarSubevent( CbmTofStarTrigger triggerIn ) :
   fTrigger( triggerIn ),
   fuEventStatusFlags( 0 ),
   fvMsgBuffer(),
   fpulBuff(NULL)
{
}
CbmTofStarSubevent::~CbmTofStarSubevent()
{
   fvMsgBuffer.clear(); 
   if( NULL != fpulBuff ) 
      delete fpulBuff;
}
void * CbmTofStarSubevent::BuildOutput( Int_t & iOutputSizeBytes )
{
   iOutputSizeBytes = 0;
   
   ULong64_t ulDataA = fTrigger.GetFullGdpbTs();
   ULong64_t ulDataB = fTrigger.GetFullStarTs();
   ULong64_t ulDataC =   (static_cast< ULong64_t >( fTrigger.GetStarToken() )  << 32)
                       + (static_cast< ULong64_t >( fTrigger.GetStarDaqCmd() ) << 16)
                       + (static_cast< ULong64_t >( fTrigger.GetStarTrigCmd() )     );
   
   // Size of output is
   // 3 * Long64 for Header Star Token Info
   // 1 * Long64 for Header Event Status Flags (version, bad/good, ...)
   // n * Long64 for the n messages in Buffer: 1 Epoch + messages + 1 extra epoch in-between if needed
   iOutputSizeBytes = 4 + fvMsgBuffer.size();
   
   // Do loop to find the size 
   
   // Allocate memory
   ULong64_t * pulBuff = new ULong64_t[iOutputSizeBytes]; // Headers only
   
   pulBuff[0] = fTrigger.GetFullGdpbTs();
   pulBuff[1] = fTrigger.GetFullStarTs();
   pulBuff[2] =   (static_cast< ULong64_t >( fTrigger.GetStarToken() )  << 32)
                + (static_cast< ULong64_t >( fTrigger.GetStarDaqCmd() ) << 16)
                + (static_cast< ULong64_t >( fTrigger.GetStarTrigCmd() )     );
   pulBuff[3] = static_cast< ULong64_t >(fuEventStatusFlags);
  
   // does not work due to "error: cannot convert ‘ngdpb::Message’ to ‘long unsigned int’ in assignment"
//  std::copy( fvMsgBuffer.begin(), fvMsgBuffer.end(), pulBuff + 4 );

   // Unoptimized replacement: item by item copy
   for( UInt_t uMsgIdx = 0; uMsgIdx < fvMsgBuffer.size(); uMsgIdx++)
      pulBuff[4 + uMsgIdx] = fvMsgBuffer[uMsgIdx].getData();
   
   iOutputSizeBytes *= sizeof( ULong64_t );
   
   return static_cast< void * >( pulBuff );
}
//ClassImp(CbmTofStarTrigger)
/**********************************************************************/
