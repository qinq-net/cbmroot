// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmTofStarData2018                         -----
// -----               Created 13.01.2017 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CbmTofStarData2018_H
#define CbmTofStarData2018_H

//#ifndef __CINT__
  #include "rocMess_wGet4v2.h"
//#endif
#include "CbmTofStarData.h"

#include "Rtypes.h"

#include <vector>

/*
class CbmTofStarTrigger2018
{
   // Methods implementation can be found at the end of CbmTsMonitorTofStar.cxx!
   public:
      // Constructors
      CbmTofStarTrigger2018( ULong64_t ulGdpbTsFullIn, ULong64_t ulStarTsFullIn,
                         UInt_t    uStarTokenIn,   UInt_t    uStarDaqCmdIn,
                         UInt_t    uStarTrigCmdIn );

      // Destructor
      ~CbmTofStarTrigger2018() {};

      // Setters
      inline void SetFullGdpbTs( ULong64_t ulGdpbTsFullIn ){ fulGdpbTsFull = ulGdpbTsFullIn; }
      inline void SetFullStarTs( ULong64_t ulStarTsFullIn ){ fulStarTsFull = ulStarTsFullIn; }
      inline void SetStarToken(  UInt_t    uStarTokenIn ){   fuStarToken   = uStarTokenIn; }
      inline void SetStarDaqCmd( UInt_t    uStarDaqCmdIn ){  fusStarDaqCmd  = uStarDaqCmdIn; }
      inline void SetStarTRigCmd(UInt_t    uStarTrigCmdIn ){ fusStarTrigCmd = uStarTrigCmdIn; }

      // Accessors
      inline ULong64_t GetFullGdpbTs()  const { return fulGdpbTsFull;}
      inline ULong64_t GetFullStarTs()  const { return fulStarTsFull;}
      inline UInt_t    GetStarToken()   const { return fuStarToken;}
      inline UShort_t  GetStarDaqCmd()  const { return fusStarDaqCmd;}
      inline UShort_t  GetStarTrigCmd() const { return fusStarTrigCmd;}
      UInt_t           GetStarTrigerWord() const;
      UInt_t           GetFullGdpbEpoch()  const;

      // Operators
      bool operator<(const CbmTofStarTrigger2018& other) const;

   private:
      ULong64_t fulGdpbTsFull;
      ULong64_t fulStarTsFull;
      UInt_t    fuStarToken;
      UShort_t  fusStarDaqCmd;
      UShort_t  fusStarTrigCmd;

//      CbmTofStarTrigger2018(const CbmTofStarTrigger2018&);
//      CbmTofStarTrigger2018 operator=(const CbmTofStarTrigger2018&);

//   ClassDef(CbmTofStarTrigger2018, 1)
};
*/

class CbmTofStarSubevent2018
{
   // Methods implementation can be found at the end of CbmTsMonitorTofStar.cxx!
   public:
      // Constructors
      CbmTofStarSubevent2018();
      CbmTofStarSubevent2018( CbmTofStarTrigger triggerIn, UShort_t sourceIdIn );

      // Destructor
      ~CbmTofStarSubevent2018();

      // Setters
      inline void SetTrigger( CbmTofStarTrigger triggerIn ){ fTrigger = triggerIn; fbTriggerSet = kTRUE; }
      inline void SetSource( UShort_t sourceIdIn ){ fusSourceId = sourceIdIn;
		                                              fulEventStatusFlags = (fulEventStatusFlags & ( ( sourceIdIn & kulSourceIdMask ) << kulSourceIdOffset ) )
		                                                                                         | ( ( sourceIdIn & kulSourceIdMask ) << kulSourceIdOffset );
		                                            }
      inline void SetBadEventFlag(  Bool_t bFlagState = kTRUE ){
                        bFlagState ? (fulEventStatusFlags |= kulFlagBadEvt) :
                                     (fulEventStatusFlags &= ~(kulFlagBadEvt) ); }
      inline void SetOverlapEventFlag(  Bool_t bFlagState = kTRUE ){
                        bFlagState ? (fulEventStatusFlags |= kulFlagOverlapEvt) :
                                     (fulEventStatusFlags &= ~(kulFlagOverlapEvt) ); }
      inline void SetEmptyEventFlag(  Bool_t bFlagState = kTRUE ){
                        bFlagState ? (fulEventStatusFlags |= kulFlagEmptyEvt) :
                                     (fulEventStatusFlags &= ~(kulFlagEmptyEvt) ); }
      inline void SetStartBorderEventFlag(  Bool_t bFlagState = kTRUE ){
                        bFlagState ? (fulEventStatusFlags |= kulFlagStartBorderEvt) :
                                     (fulEventStatusFlags &= ~(kulFlagStartBorderEvt) ); }
      inline void SetEndBorderEventFlag(  Bool_t bFlagState = kTRUE ){
                        bFlagState ? (fulEventStatusFlags |= kulFlagEndBorderEvt) :
                                     (fulEventStatusFlags &= ~(kulFlagEndBorderEvt) ); }
//#ifndef __CINT__
      inline void AddMsg( gdpb::FullMessage & msgIn){ fvMsgBuffer.push_back( msgIn ); }
//#endif

      // Accessors
      inline CbmTofStarTrigger GetTrigger() const { return fTrigger;}
      inline Bool_t            GetBadEventFlag()         const { return 0 < (fulEventStatusFlags & kulFlagBadEvt); }
      inline Bool_t            GetOverlapEventFlag()     const { return 0 < (fulEventStatusFlags & kulFlagOverlapEvt); }
      inline Bool_t            GetEmptyEventFlag()       const { return 0 < (fulEventStatusFlags & kulFlagEmptyEvt); }
      inline Bool_t            SetStartBorderEventFlag() const { return 0 < (fulEventStatusFlags & kulFlagStartBorderEvt); }
      inline Bool_t            SetEndBorderEventFlag()   const { return 0 < (fulEventStatusFlags & kulFlagEndBorderEvt); }
//#ifndef __CINT__
      inline gdpb::Message    GetMsg( UInt_t uMsgIdx ) const;
//#endif
      inline UInt_t            GetMsgBuffSize() const { return fvMsgBuffer.size();}
//#ifndef __CINT__
      inline static uint32_t   GetMaxOutputSize() { return kuMaxOutputSize;}
//#endif

      // Content clearing
      void   ClearSubEvent();

      // Sort content of output buffer
      void   SortMessages();

      // Sub-event output
      void * BuildOutput( Int_t & iOutputSizeBytes );

      // Sub-event input
      Bool_t LoadInput( void * pBuff, Int_t iInputSizeBytes );

   private:
//#ifndef __CINT__
      static const uint32_t         kuMaxOutputSize       = 131072; // 2^17
      static const uint32_t         kuMaxNbMsgs           =   8190; // 4 * 64b in header => floor( (2^17 / 8 ) - 4) / 2
      static const uint64_t         kulFlagBadEvt         = 0x1 << 0;
      static const uint64_t         kulFlagOverlapEvt     = 0x1 << 1;
      static const uint64_t         kulFlagEmptyEvt       = 0x1 << 2;
      static const uint64_t         kulFlagStartBorderEvt = 0x1 << 3;
      static const uint64_t         kulFlagEndBorderEvt   = 0x1 << 4;
      static const uint64_t         kulSourceIdOffset     =       16;
      static const uint64_t         kulSourceIdMask       =   0xFFFF;
//#endif

      Bool_t                        fbTriggerSet;
      CbmTofStarTrigger             fTrigger;
      UShort_t                      fusSourceId;
      ULong64_t                     fulEventStatusFlags;
//#ifndef __CINT__
      std::vector< gdpb::FullMessage > fvMsgBuffer;
      ULong64_t                     fpulBuff[kuMaxOutputSize];
//#endif


      CbmTofStarSubevent2018(const CbmTofStarSubevent2018&);
      CbmTofStarSubevent2018 operator=(const CbmTofStarSubevent2018&);

//   ClassDef(CbmTofStarSubevent2018, 1)
};

#endif
