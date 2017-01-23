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
      CbmTofStarSubevent( CbmTofStarTrigger triggerIn );
      
      // Destructor
      ~CbmTofStarSubevent();
   
      // Setters
      inline void SetTrigger( CbmTofStarTrigger triggerIn ){ fTrigger = triggerIn; }
      inline void SetBadEventFlag(  Bool_t bFlagState = kTRUE ){ bFlagState ? (fuEventStatusFlags |= 0x1) : (fuEventStatusFlags &= ~(0x1)); }
      inline void AddMsg( ngdpb::Message & msgIn){ fvMsgBuffer.push_back( msgIn ); }
      
      // Accessors
      inline CbmTofStarTrigger GetTrigger()  const { return fTrigger;}
      inline Bool_t            GetBadEventFlag() const { return (fuEventStatusFlags & 0x1); }
      inline ngdpb::Message    GetMsg( UInt_t uMsgIdx ) const;
      inline UInt_t            GetMsgBuffSize() const { return fvMsgBuffer.size();}
      
      // Sub-event output
      void * BuildOutput( Int_t & iOutputSizeBytes );
      
   private:
      CbmTofStarTrigger             fTrigger;
      UInt_t                        fuEventStatusFlags;
#ifndef __CINT__
      std::vector< ngdpb::Message > fvMsgBuffer;
#endif
      ULong64_t *                   fpulBuff;
      
   
      CbmTofStarSubevent(const CbmTofStarSubevent&);
      CbmTofStarSubevent operator=(const CbmTofStarSubevent&);
    
//   ClassDef(CbmTofStarSubevent, 1)
};

#endif
