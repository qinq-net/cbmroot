// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmTofStarData                             -----
// -----               Created 13.01.2017 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CbmTofStarData_H
#define CbmTofStarData_H

#ifndef __CINT__
#include "rocMess_wGet4v1.h"
#endif

#include "Rtypes.h"

#include <vector>

#ifndef __CINT__
namespace get4v1x {
   // Size of one clock cycle (=1 coarse bin)
   const double   kdClockCycleSize    = 6250.0; //[ps]
   const double   kdClockCycleSizeNs  = kdClockCycleSize / 1000.0; //[ns]
   // TODO:For now make 100ps default, maybe need later an option for it
   const double   kdTotBinSize      =   50.0; //ps

   const uint32_t kuFineTime    = 0x0000007F; // Fine Counter value
   const uint32_t kuFtShift     =          0; // Fine Counter offset
   const uint32_t kuCoarseTime  = 0x0007FF80; // Coarse Counter value
   const uint32_t kuCtShift     =          7; // Coarse Counter offset
   const uint32_t kuCtSize      =         12; // Coarse Counter size in bits

   const uint32_t kuFineCounterSize    = ( (kuFineTime>>kuFtShift)+1 );
   const uint32_t kuCoarseCounterSize  = ( (kuCoarseTime>>kuCtShift)+1 );
   const uint32_t kuCoarseOverflowTest = kuCoarseCounterSize / 2 ; // Limit for overflow check
   const uint32_t kuTotCounterSize     = 256;

   // Nominal bin size of NL are neglected
   const double   kdBinSize     = kdClockCycleSize / static_cast<double>(kuFineCounterSize);
   // Epoch Size in bins
   const uint32_t kuEpochInBins = kuFineTime + kuCoarseTime + 1;
   // Epoch Size in ps
   // alternatively: (kiCoarseTime>>kiCtShift + 1)*kdClockCycleSize
   const double   kdEpochInPs   = kuEpochInBins*kdBinSize;
}
#endif

class CbmTofStarTrigger
{
   // Methods implementation can be found at the end of CbmTsMonitorTofStar.cxx!
   public:
      // Constructors
      CbmTofStarTrigger( ULong64_t ulGdpbTsFullIn, ULong64_t ulStarTsFullIn,
                         UInt_t    uStarTokenIn,   UInt_t    uStarDaqCmdIn, 
                         UInt_t    uStarTrigCmdIn );
      
      // Destructor
      ~CbmTofStarTrigger() {};
   
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
      bool operator<(const CbmTofStarTrigger& other) const;
      
   private:
      ULong64_t fulGdpbTsFull;
      ULong64_t fulStarTsFull;
      UInt_t    fuStarToken;
      UShort_t  fusStarDaqCmd;
      UShort_t  fusStarTrigCmd;
   
//      CbmTofStarTrigger(const CbmTofStarTrigger&);
//      CbmTofStarTrigger operator=(const CbmTofStarTrigger&);
    
//   ClassDef(CbmTofStarTrigger, 1)
};

class CbmTofStarSubevent
{
   // Methods implementation can be found at the end of CbmTsMonitorTofStar.cxx!
   public:
      // Constructors
      CbmTofStarSubevent();
      CbmTofStarSubevent( CbmTofStarTrigger triggerIn );
      
      // Destructor
      ~CbmTofStarSubevent();
   
      // Setters
      inline void SetTrigger( CbmTofStarTrigger triggerIn ){ fTrigger = triggerIn; fbTriggerSet = kTRUE; }
      inline void SetBadEventFlag(  Bool_t bFlagState = kTRUE ){ bFlagState ? (fuEventStatusFlags |= 0x1) : (fuEventStatusFlags &= ~(0x1)); }
#ifndef __CINT__
      inline void AddMsg( ngdpb::Message & msgIn){ fvMsgBuffer.push_back( msgIn ); }
#endif
      
      // Accessors
      inline CbmTofStarTrigger GetTrigger()  const { return fTrigger;}
      inline Bool_t            GetBadEventFlag() const { return (fuEventStatusFlags & 0x1); }
#ifndef __CINT__
      inline ngdpb::Message    GetMsg( UInt_t uMsgIdx ) const;
#endif
      inline UInt_t            GetMsgBuffSize() const { return fvMsgBuffer.size();}
      inline static uint32_t   GetMaxOutputSize() { return kuMaxOutputSize;}
      
      // Content clearing
      void   ClearSubEvent();
      
      // Sub-event output
      void * BuildOutput( Int_t & iOutputSizeBytes );
      
   private:
      static const uint32_t         kuMaxOutputSize = 131072; // 2^17
      static const uint32_t         kuMaxNbMsgs     =  16380; // 4 * 64b in header => floor( (2^17 / 8 ) - 4)
      static const uint64_t         kulFlagBadEvt   = 0x1 << 0;
   
      Bool_t                        fbTriggerSet;
      CbmTofStarTrigger             fTrigger;
      UInt_t                        fuEventStatusFlags;
#ifndef __CINT__
      std::vector< ngdpb::Message > fvMsgBuffer;
#endif
      ULong64_t                     fpulBuff[kuMaxOutputSize];
      
   
      CbmTofStarSubevent(const CbmTofStarSubevent&);
      CbmTofStarSubevent operator=(const CbmTofStarSubevent&);
    
//   ClassDef(CbmTofStarSubevent, 1)
};

#endif
