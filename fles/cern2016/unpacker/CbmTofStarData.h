// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmTofStarData                             -----
// -----               Created 13.01.2017 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#ifndef CbmTofStarData_H
#define CbmTofStarData_H

#include "Rtypes.h"

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
      inline void SetStarDaqCmd( UInt_t    uStarDaqCmdIn ){  fuStarDaqCmd  = uStarDaqCmdIn; }
      inline void SetStarTRigCmd(UInt_t    uStarTrigCmdIn ){ fuStarTrigCmd = uStarTrigCmdIn; }
      
      // Accessors
      inline ULong64_t GetFullGdpbTs()  const { return fulGdpbTsFull;}
      inline ULong64_t GetFullStarTs()  const { return fulStarTsFull;}
      inline UInt_t    GetStarToken()   const { return fuStarToken;}
      inline UInt_t    GetStarDaqCmd()  const { return fuStarDaqCmd;}
      inline UInt_t    GetStarTRigCmd() const { return fuStarTrigCmd;}
         
      // Operators
      bool operator<(const CbmTofStarTrigger& other) const;
      
   private:
      ULong64_t fulGdpbTsFull;
      ULong64_t fulStarTsFull;
      UInt_t    fuStarToken;
      UInt_t    fuStarDaqCmd;
      UInt_t    fuStarTrigCmd;
   
//      CbmTofStarTrigger(const CbmTofStarTrigger&);
//      CbmTofStarTrigger operator=(const CbmTofStarTrigger&);
    
//   ClassDef(CbmTofStarTrigger, 1)
};

#endif
