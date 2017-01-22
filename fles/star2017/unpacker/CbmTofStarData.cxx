// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                        CbmTofStarData                             -----
// -----               Created 13.01.2017 by P.-A. Loizeau                 -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmTofStarData.h"

/********************** CbmTofStarTrigger *****************************/
CbmTofStarTrigger::CbmTofStarTrigger( ULong64_t ulGdpbTsFullIn, ULong64_t ulStarTsFullIn,
                     UInt_t    uStarTokenIn,   UInt_t    uStarDaqCmdIn, 
                     UInt_t    uStarTrigCmdIn ) :
   fulGdpbTsFull( ulGdpbTsFullIn ),
   fulStarTsFull( ulStarTsFullIn ),
   fuStarToken(   uStarTokenIn ),
   fuStarDaqCmd(  uStarDaqCmdIn) ,
   fuStarTrigCmd( uStarTrigCmdIn )
{
}
//! strict weak ordering operator, assumes same TS cycle for both triggers
bool CbmTofStarTrigger::operator<(const CbmTofStarTrigger& other) const
{
   ULong64_t uThisTs  = this->GetFullGdpbTs();
   ULong64_t uOtherTs = other.GetFullGdpbTs();

   return uThisTs < uOtherTs;
}

//ClassImp(CbmTofStarTrigger)
/**********************************************************************/
