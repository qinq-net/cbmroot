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
   fiCrobActiveFlag(),
   fuFebsInGemA(0),
   fnFebsIdsArrayGemA(),
   fuFebsInGemB(0),
   fnFebsIdsArrayGemB(),
   fChannelsToPadX(),
   fChannelsToPadY(),
   fRealX(),
   fRealPadSize()
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
   l->add("NrOfFebsInGemA", fuFebsInGemA);
   l->add("nFebsIdsArrayA", fnFebsIdsArrayGemA);
   l->add("NrOfFebsInGemB", fuFebsInGemB);
   l->add("nFebsIdsArrayB", fnFebsIdsArrayGemB);
   l->add("ChannelsToPadX", fChannelsToPadX);
   l->add("ChannelsToPadY", fChannelsToPadY);
   l->add("RealX",          fRealX);
   l->add("PadSize",        fRealPadSize);
}

// -------------------------------------------------------------------------

Bool_t CbmMcbm2018MuchPar::getParams(FairParamList* l) {

   if (!l) return kFALSE;

   if ( ! l->fill("NrOfDpbs", &fuNrOfDpbs ) ) return kFALSE;

   fiDbpIdArray.Set( fuNrOfDpbs );
   if ( ! l->fill("DbpIdArray", &fiDbpIdArray ) ) return kFALSE;

   fiCrobActiveFlag.Set( fuNrOfDpbs * kuNbCrobsPerDpb );
   if ( ! l->fill("CrobActiveFlag", &fiCrobActiveFlag ) ) return kFALSE;

   if ( ! l->fill("NrOfFebsInGemA", &fuFebsInGemA ) ) return kFALSE;

   fnFebsIdsArrayGemA.Set(GetNrOfFebsInGemA());
   if ( ! l->fill("nFebsIdsArrayA", &fnFebsIdsArrayGemA) ) return kFALSE;

   if ( ! l->fill("NrOfFebsInGemB", &fuFebsInGemB ) ) return kFALSE;

   fnFebsIdsArrayGemB.Set(GetNrOfFebsInGemB());
   if ( ! l->fill("nFebsIdsArrayB", &fnFebsIdsArrayGemB) ) return kFALSE;

   fChannelsToPadX.Set(GetNrOfFebs()*kuNbChanPerAsic);
   if ( ! l->fill("ChannelsToPadX", &fChannelsToPadX) ) return kFALSE;

   fChannelsToPadY.Set(GetNrOfFebs()*kuNbChanPerAsic);
   if ( ! l->fill("ChannelsToPadY", &fChannelsToPadY) ) return kFALSE;

   fRealX.Set(2232); // Number of Sectors in one GEM Module
   if ( ! l->fill("RealX", &fRealX) ) return kFALSE;

   fRealPadSize.Set(2232); // Number of Sectors in one GEM Module
   if ( ! l->fill("PadSize", &fRealPadSize) ) return kFALSE;

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
Short_t CbmMcbm2018MuchPar::GetPadX(Short_t febid, Short_t channelid)
{
  if( fChannelsToPadX.GetSize () <= (febid*kuNbChanPerAsic)+channelid )
  {
    LOG(DEBUG) << "CbmMcbm2018MuchPar::GetPadX => Index out of bounds: "
	       << ((febid*kuNbChanPerAsic)+channelid) << " VS " << fChannelsToPadX.GetSize()
	       << " (" << febid << " and " << channelid << ")"
	       << FairLogger::endl;
    return -2;
  } // if( fChannelsToPadX.GetSize () <= (febid*kuNbChanPerAsic)+channelid )


  return fChannelsToPadX[(febid*kuNbChanPerAsic)+channelid];
}
Short_t CbmMcbm2018MuchPar::GetPadY(Short_t febid, Short_t channelid)
{
//LOG(INFO)<<" fChannelsToPadY.GetSize() "<<fChannelsToPadY.GetSize()<<" fChannelsToPadX.GetSize() "<<fChannelsToPadX.GetSize()<< FairLogger::endl;
  if( fChannelsToPadY.GetSize() <= (febid*kuNbChanPerAsic)+channelid )
  {
      LOG(DEBUG) << "CbmMcbm2018MuchPar::GetPadY => Index out of bounds: "
                 << ((febid*kuNbChanPerAsic)+channelid) << " VS " << fChannelsToPadY.GetSize()
                 << " (" << febid << " and " << channelid << ")"
                 << FairLogger::endl;
    return -2;
  } // if( fChannelsToPadY.GetSize () <= (febid*kuNbChanPerAsic)+channelid )

   return fChannelsToPadY[(febid*kuNbChanPerAsic)+channelid];
}

UInt_t CbmMcbm2018MuchPar::GetFebId(Int_t uAsicIdx)
{
//LOG(INFO)<<" fnFebsIdsArrayGemA.GetSize() "<<fnFebsIdsArrayGemA.GetSize()<<" fnFebsIdsArrayGemB.GetSize()"<<fnFebsIdsArrayGemB.GetSize()<<FairLogger::endl;
   if(uAsicIdx >= GetNrOfFebsInGemA())
   {return fnFebsIdsArrayGemB[uAsicIdx%GetNrOfFebsInGemA()];}
   else return fnFebsIdsArrayGemA[uAsicIdx];
}

UInt_t CbmMcbm2018MuchPar::GetModule(Int_t uAsicIdx)
{
   if(uAsicIdx >= GetNrOfFebsInGemA())
   {return 1;}
   else return 0;
}

Double_t CbmMcbm2018MuchPar::GetRealX(Int_t SectorIndex)
{

  //LOG(INFO)<<" fChannelsToPadX.GetSize() "<<fChannelsToPadX.GetSize()<< FairLogger::endl;
  if( SectorIndex < 0 || SectorIndex <= 97 )
  {
      LOG(DEBUG) << "CbmMcbm2018MuchPar::GetRealX => Index out of bounds: "
                 << FairLogger::endl;
    return -2;
  } // if( fChannelsToPadY.GetSize () <= (febid*kuNbChanPerAsic)+channelid )

   return fRealX[SectorIndex];
}

Double_t CbmMcbm2018MuchPar::GetRealPadSize(Int_t SectorIndex)
{

  //LOG(INFO)<<" fChannelsToPadX.GetSize() "<<fChannelsToPadX.GetSize()<< FairLogger::endl;
  if( SectorIndex < 0 || SectorIndex <= 97 )
  {
      LOG(DEBUG) << "CbmMcbm2018MuchPar::GetRealX => Index out of bounds: "
                 << FairLogger::endl;
    return -2;
  } // if( fChannelsToPadY.GetSize () <= (febid*kuNbChanPerAsic)+channelid )

   return fRealPadSize[SectorIndex];
}

Double_t CbmMcbm2018MuchPar::GetRealX( Int_t Channel, Int_t Sector )
{
  Int_t PadIndex = Channel + 97 * Sector;
  if( Channel < 0 || Sector < 0 )
    return -2;
  if( fRealX.GetSize() <= PadIndex )
  {
      LOG(INFO) << "CbmMcbm2018MuchPar::GetRealX => Index out of bounds: "
                <<  Channel << " " << Sector << " " << PadIndex
                 << FairLogger::endl;
    return -1;
  } // if( fRealX.Size() <= PadIndex )

   return fRealX[PadIndex];
}
Double_t CbmMcbm2018MuchPar::GetRealPadSize( Int_t Channel, Int_t Sector )
{
  Int_t PadIndex = Channel + 97 * Sector;
  if( Channel < 0 || Sector < 0 )
    return -2;
  if( fRealPadSize.GetSize() <= PadIndex )
  {
      LOG(INFO) << "CbmMcbm2018MuchPar::GetRealPadSize => Index out of bounds: "
                <<  Channel << " " << Sector << " " << PadIndex
                 << FairLogger::endl;
    return -1;
  } // if( fRealPadSize.Size() <= PadIndex )

   return fRealPadSize[PadIndex];
}

ClassImp(CbmMcbm2018MuchPar)
