// -------------------------------------------------------------------------
// -----            CbmMcbm2018MuchPar source file                      -----
// -----            Created 25/07/17  by P.-A. Loizeau                  ----
// -----            Modified 07/12/18  Ajit Kumar                 -----
// -------------------------------------------------------------------------

#include "CbmMcbm2018MuchPar.h"

#include "FairParamList.h"
#include "FairDetParIo.h"
#include "FairParIo.h"
#include "FairLogger.h"

#include "TString.h"
#include "TMath.h"

using namespace std;

// -----   Standard constructor   ------------------------------------------
CbmMcbm2018MuchPar::CbmMcbm2018MuchPar(const char* name,
                                     const char* title,
                                     const char* context) :
   FairParGenericSet(name, title, context),
   fuNrOfDpbs( 0 ),
   fiDbpIdArray(),
   fiCrobActiveFlag()
{
  detName="Much";
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmMcbm2018MuchPar::~CbmMcbm2018MuchPar()
{
}
// -------------------------------------------------------------------------


// -----   Public method clear   -------------------------------------------
void CbmMcbm2018MuchPar::clear()
{
   status = kFALSE;
   resetInputVersions();
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------

void CbmMcbm2018MuchPar::putParams(FairParamList* l)
{
   if (!l) return;

   l->add("NrOfDpbs",       fuNrOfDpbs );
   l->add("DbpIdArray",     fiDbpIdArray );
   l->add("CrobActiveFlag", fiCrobActiveFlag);

}

// -------------------------------------------------------------------------

Bool_t CbmMcbm2018MuchPar::getParams(FairParamList* l) {

   if (!l) return kFALSE;

   if ( ! l->fill("NrOfDpbs", &fuNrOfDpbs ) ) return kFALSE;

   fiDbpIdArray.Set( fuNrOfDpbs );
   if ( ! l->fill("DbpIdArray", &fiDbpIdArray ) ) return kFALSE;

   fiCrobActiveFlag.Set( fuNrOfDpbs * kuNbCrobsPerDpb );
   if ( ! l->fill("CrobActiveFlag", &fiCrobActiveFlag ) ) return kFALSE;

  return kTRUE;
}
// -------------------------------------------------------------------------
Int_t CbmMcbm2018MuchPar::ElinkIdxToFebIdx( UInt_t uElink )
{
   //LOG (INFO) <<" uElink "<<uElink<<" kuNbElinksPerCrob "<<kuNbElinksPerCrob<<FairLogger::endl;
   if( uElink < kuNbElinksPerCrob )
      return kiCrobMapElinkFebIdx[ uElink ];
      else
      {
         LOG(WARNING) << "CbmMcbm2018MuchPar::ElinkIdxToFebIdx => Index out of bound, "
                      << "returning crazy value!"
                      << FairLogger::endl;
         return -1;
      } // else of if( uElink < kuNbElinksPerCrob )
}
// -------------------------------------------------------------------------
UInt_t CbmMcbm2018MuchPar::ElinkIdxToAsicIdxFebMuch( UInt_t uElink )
{
   if( uElink < kuNbElinksPerCrob )
      return kuCrobMapElinkFebMuch[ uElink ];
      else
      {
         LOG(WARNING) << "CbmMcbm2018MuchPar::ElinkIdxToAsicIdxFebMuch => Index out of bound, "
                      << "returning crazy value!"
                      << FairLogger::endl;
         return 0xFFFF;
      } // else of if( uElink < kuNbElinksPerCrob )
}
// -------------------------------------------------------------------------
UInt_t CbmMcbm2018MuchPar::GetDpbId( UInt_t uDpbIdx )
{
   if( uDpbIdx < fuNrOfDpbs )
      return fiDbpIdArray[ uDpbIdx ];
      else
      {
         LOG(WARNING) << "CbmMcbm2018MuchPar::GetDpbId => Index out of bound, "
                      << "returning crazy value!"
                      << FairLogger::endl;
         return 0xFFFFFFFF;
      } // else of if( uDpbIdx < fuNrOfDpbs )
}
Bool_t CbmMcbm2018MuchPar::IsCrobActive( UInt_t uDpbIdx, UInt_t uCrobIdx )
{
   if( uDpbIdx < fuNrOfDpbs )
   {
      if( uCrobIdx < kuNbCrobsPerDpb )
         return 0 < fiCrobActiveFlag[ uDpbIdx * kuNbCrobsPerDpb + uCrobIdx ] ? kTRUE : kFALSE;
         else
         {
            LOG(WARNING) << "CbmMcbm2018MuchPar::IsCrobActive => Crob Index out of bound, "
                         << "returning default inactive!"
                         << FairLogger::endl;
            return kFALSE;
         } // else of if( uCrobIdx < kuNbCrobsPerDpb )
   } // if( uDpbIdx < fuNrOfDpbs )
      else
      {
         LOG(WARNING) << "CbmMcbm2018MuchPar::IsCrobActive => Dpb Index out of bound, "
                      << "returning default inactive!"
                      << FairLogger::endl;
         return kFALSE;
      } // else of if( uDpbIdx < fuNrOfDpbs )
}
Bool_t CbmMcbm2018MuchPar::IsFebActive( UInt_t uFebInSystIdx )
{

   if( uFebInSystIdx < GetNrOfFebs() )
   {
      /// Always return true for now
      return kTRUE;
   } // if( uFebInSystIdx < GetNrOfFebs() )
      else
      {
         LOG(WARNING) << "CbmMcbm2018MuchPar::IsFebActive => Feb Index out of bound, "
                      << "returning default inactive!"
                      << FairLogger::endl;
         return kFALSE;
      } // else of if( uFebInSystIdx < GetNrOfFebs() )
}
Bool_t CbmMcbm2018MuchPar::IsFebActive( UInt_t uDpbIdx, UInt_t uCrobIdx, UInt_t uFebIdx )
{
   if( uDpbIdx < fuNrOfDpbs )
   {
      if( uCrobIdx < kuNbCrobsPerDpb )
      {
         if( uFebIdx < kuNbFebsPerCrob )
         {
            UInt_t uIdx = ( uDpbIdx * kuNbCrobsPerDpb + uCrobIdx ) * kuNbFebsPerCrob + uFebIdx;
            return IsFebActive( uIdx );
         } // if( uFebIdx < kuNbFebsPerCrob )
            else
            {
               LOG(WARNING) << "CbmMcbm2018MuchPar::IsFebActive => Feb Index out of bound, "
                            << "returning default inactive!"
                            << FairLogger::endl;
               return kFALSE;
            } // else of if( uFebIdx < kuNbCrobsPerDpb )
      } // if( uCrobIdx < kuNbCrobsPerDpb )
         else
         {
            LOG(WARNING) << "CbmMcbm2018MuchPar::IsFebActive => Crob Index out of bound, "
                         << "returning default inactive!"
                         << FairLogger::endl;
            return kFALSE;
         } // else of if( uCrobIdx < kuNbCrobsPerDpb )
   } // if( uDpbIdx < fuNrOfDpbs )
      else
      {
         LOG(WARNING) << "CbmMcbm2018MuchPar::IsFebActive => Dpb Index out of bound, "
                      << "returning default inactive!"
                      << FairLogger::endl;
         return kFALSE;
      } // else of if( uDpbIdx < fuNrOfDpbs )
}

ClassImp(CbmMcbm2018MuchPar)
