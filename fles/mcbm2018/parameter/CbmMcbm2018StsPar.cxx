// -------------------------------------------------------------------------
// -----            CbmMcbm2018StsPar source file                      -----
// -----            Created 25/07/17  by P.-A. Loizeau                 -----
// -------------------------------------------------------------------------

#include "CbmMcbm2018StsPar.h"

#include "FairParamList.h"
#include "FairDetParIo.h"
#include "FairParIo.h"
#include "FairLogger.h"

#include "TString.h"

using namespace std;
// -----   Standard constructor   ------------------------------------------
CbmMcbm2018StsPar::CbmMcbm2018StsPar(const char* name,
                                     const char* title,
                                     const char* context) :
   FairParGenericSet(name, title, context),
   fuNbModules( 0 ),
   fiModuleType(),
   fiModAddress(),
   fuNrOfDpbs( 0 ),
   fiDbpIdArray(),
   fiCrobActiveFlag(),
   fiFebModuleIdx(),
   fiFebModuleSide()
{
  detName="Sts";
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmMcbm2018StsPar::~CbmMcbm2018StsPar()
{
}
// -------------------------------------------------------------------------


// -----   Public method clear   -------------------------------------------
void CbmMcbm2018StsPar::clear()
{
   status = kFALSE;
   resetInputVersions();
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------

void CbmMcbm2018StsPar::putParams(FairParamList* l)
{
   if (!l) return;

   l->add("NbModules",      fuNbModules );
   l->add("ModuleType",     fiModuleType);
   l->add("ModAddress",     fiModAddress );
   l->add("NrOfDpbs",       fuNrOfDpbs );
   l->add("DbpIdArray",     fiDbpIdArray );
   l->add("CrobActiveFlag", fiCrobActiveFlag);

   l->add("FebModuleIdx",   fiFebModuleIdx);
   l->add("FebModuleSide",  fiFebModuleSide);
}

// -------------------------------------------------------------------------

Bool_t CbmMcbm2018StsPar::getParams(FairParamList* l) {

   if (!l) return kFALSE;

   if ( ! l->fill("NbModules", &fuNbModules ) ) return kFALSE;

   fiModuleType.Set( fuNbModules );
   fiModAddress.Set( fuNbModules );
   if ( ! l->fill("ModuleType", &fiModuleType ) ) return kFALSE;
   if ( ! l->fill("ModAddress", &fiModAddress ) ) return kFALSE;

   if ( ! l->fill("NrOfDpbs", &fuNrOfDpbs ) ) return kFALSE;

   fiDbpIdArray.Set( fuNrOfDpbs );
   if ( ! l->fill("DbpIdArray", &fiDbpIdArray ) ) return kFALSE;

   fiCrobActiveFlag.Set( fuNrOfDpbs * kuNbCrobsPerDpb );
   if ( ! l->fill("CrobActiveFlag", &fiCrobActiveFlag ) ) return kFALSE;

   fiFebModuleIdx.Set( fuNrOfDpbs * kuNbCrobsPerDpb * kuNbFebsPerCrob );
   fiFebModuleSide.Set( fuNrOfDpbs * kuNbCrobsPerDpb * kuNbFebsPerCrob );
   if ( ! l->fill("fiFebModuleIdx", &fiFebModuleIdx ) ) return kFALSE;
   if ( ! l->fill("fiFebModuleSide", &fiFebModuleSide ) ) return kFALSE;

  return kTRUE;
}
// -------------------------------------------------------------------------
UInt_t CbmMcbm2018StsPar::ElinkIdxToAsicIdxFebA( UInt_t uElink )
{
   if( uElink < kuNbElinksPerCrob )
      return kuCrobMapElinkFebA[ uElink ];
      else
      {
         LOG(WARNING) << "CbmMcbm2018StsPar::ElinkIdxToAsicIdxFebA => Index out of bound, "
                      << "returning crazy value!"
                      << FairLogger::endl;
         return 0xFFFF;
      } // else of if( uElink < kuNbElinksPerCrob )
}
UInt_t CbmMcbm2018StsPar::ElinkIdxToAsicIdxFebB( UInt_t uElink )
{
   if( uElink < kuNbElinksPerCrob )
      return kuCrobMapElinkFebB[ uElink ];
      else
      {
         LOG(WARNING) << "CbmMcbm2018StsPar::ElinkIdxToAsicIdxFebB => Index out of bound, "
                      << "returning crazy value!"
                      << FairLogger::endl;
         return 0xFFFF;
      } // else of if( uElink < kuNbElinksPerCrob )
}
// -------------------------------------------------------------------------
UInt_t CbmMcbm2018StsPar::GetModuleType( UInt_t uModuleIdx )
{
   if( uModuleIdx < fuNbModules )
      return fiModuleType[ uModuleIdx ];
      else
      {
         LOG(WARNING) << "CbmMcbm2018StsPar::GetModuleType => Index out of bound, "
                      << "returning crazy value!"
                      << FairLogger::endl;
         return 0xFFFFFFFF;
      } // else of if( uModuleIdx < fuNbModules )
}
UInt_t CbmMcbm2018StsPar::GetModuleAddress( UInt_t uModuleIdx )
{
   if( uModuleIdx < fuNbModules )
      return fiModAddress[ uModuleIdx ];
      else
      {
         LOG(WARNING) << "CbmMcbm2018StsPar::GetModuleAddress => Index out of bound, "
                      << "returning crazy value!"
                      << FairLogger::endl;
         return 0xFFFFFFFF;
      } // else of if( uModuleIdx < fuNbModules )
}
// -------------------------------------------------------------------------
UInt_t CbmMcbm2018StsPar::GetDpbId( UInt_t uDpbIdx )
{
   if( uDpbIdx < fuNrOfDpbs )
      return fiDbpIdArray[ uDpbIdx ];
      else
      {
         LOG(WARNING) << "CbmMcbm2018StsPar::GetDpbId => Index out of bound, "
                      << "returning crazy value!"
                      << FairLogger::endl;
         return 0xFFFFFFFF;
      } // else of if( uDpbIdx < fuNrOfDpbs )
}
Bool_t CbmMcbm2018StsPar::IsCrobActive( UInt_t uDpbIdx, UInt_t uCrobIdx )
{
   if( uDpbIdx < fuNrOfDpbs )
   {
      if( uCrobIdx < kuNbCrobsPerDpb )
         return 0 < fiCrobActiveFlag[ uDpbIdx * kuNbCrobsPerDpb + uCrobIdx ] ? kTRUE : kFALSE;
         else
         {
            LOG(WARNING) << "CbmMcbm2018StsPar::IsCrobActive => Crob Index out of bound, "
                         << "returning default inactive!"
                         << FairLogger::endl;
            return kFALSE;
         } // else of if( uCrobIdx < kuNbCrobsPerDpb )
   } // if( uDpbIdx < fuNrOfDpbs )
      else
      {
         LOG(WARNING) << "CbmMcbm2018StsPar::IsCrobActive => Dpb Index out of bound, "
                      << "returning default inactive!"
                      << FairLogger::endl;
         return kFALSE;
      } // else of if( uDpbIdx < fuNrOfDpbs )
}
Int_t CbmMcbm2018StsPar::GetFebModuleIdx( UInt_t uDpbIdx, UInt_t uCrobIdx, UInt_t uFebIdx )
{
   if( uDpbIdx < fuNrOfDpbs )
   {
      if( uCrobIdx < kuNbCrobsPerDpb )
      {
         if( uFebIdx < kuNbFebsPerCrob )
         {
            UInt_t uIdx = ( uDpbIdx * kuNbCrobsPerDpb + uCrobIdx ) * kuNbFebsPerCrob + uFebIdx;
            return fiFebModuleIdx[ uIdx ];
         } // if( uFebIdx < kuNbFebsPerCrob )
            else
            {
               LOG(WARNING) << "CbmMcbm2018StsPar::GetFebModuleIdx => Feb Index out of bound, "
                            << "returning default inactive!"
                            << FairLogger::endl;
               return -1;
            } // else of if( uCrobIdx < kuNbCrobsPerDpb )

         return 0 < fiCrobActiveFlag[ uDpbIdx * kuNbCrobsPerDpb + uCrobIdx ] ? kTRUE : kFALSE;
      } // if( uCrobIdx < kuNbCrobsPerDpb )
         else
         {
            LOG(WARNING) << "CbmMcbm2018StsPar::GetFebModuleIdx => Crob Index out of bound, "
                         << "returning default inactive!"
                         << FairLogger::endl;
            return -1;
         } // else of if( uCrobIdx < kuNbCrobsPerDpb )
   } // if( uDpbIdx < fuNrOfDpbs )
      else
      {
         LOG(WARNING) << "CbmMcbm2018StsPar::GetFebModuleIdx => Dpb Index out of bound, "
                      << "returning default inactive!"
                      << FairLogger::endl;
         return -1;
      } // else of if( uDpbIdx < fuNrOfDpbs )
}
Int_t CbmMcbm2018StsPar::GetFebModuleSide( UInt_t uDpbIdx, UInt_t uCrobIdx, UInt_t uFebIdx )
{
   if( uDpbIdx < fuNrOfDpbs )
   {
      if( uCrobIdx < kuNbCrobsPerDpb )
      {
         if( uFebIdx < kuNbFebsPerCrob )
         {
            UInt_t uIdx = ( uDpbIdx * kuNbCrobsPerDpb + uCrobIdx ) * kuNbFebsPerCrob + uFebIdx;
            return fiFebModuleSide[ uIdx ];
         } // if( uFebIdx < kuNbFebsPerCrob )
            else
            {
               LOG(WARNING) << "CbmMcbm2018StsPar::GetFebModuleSide => Feb Index out of bound, "
                            << "returning default inactive!"
                            << FairLogger::endl;
               return -1;
            } // else of if( uCrobIdx < kuNbCrobsPerDpb )

         return 0 < fiCrobActiveFlag[ uDpbIdx * kuNbCrobsPerDpb + uCrobIdx ] ? kTRUE : kFALSE;
      } // if( uCrobIdx < kuNbCrobsPerDpb )
         else
         {
            LOG(WARNING) << "CbmMcbm2018StsPar::GetFebModuleSide => Crob Index out of bound, "
                         << "returning default inactive!"
                         << FairLogger::endl;
            return -1;
         } // else of if( uCrobIdx < kuNbCrobsPerDpb )
   } // if( uDpbIdx < fuNrOfDpbs )
      else
      {
         LOG(WARNING) << "CbmMcbm2018StsPar::GetFebModuleSide => Dpb Index out of bound, "
                      << "returning default inactive!"
                      << FairLogger::endl;
         return -1;
      } // else of if( uDpbIdx < fuNrOfDpbs )
}

ClassImp(CbmMcbm2018StsPar)
