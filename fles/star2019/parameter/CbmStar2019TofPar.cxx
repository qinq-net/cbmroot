// -------------------------------------------------------------------------
// -----                CbmStar2019TofPar source file                  -----
// -----                Created 09/09/18  by P.-A. Loizeau             -----
// -------------------------------------------------------------------------

#include "CbmStar2019TofPar.h"

#include "FairParamList.h"
#include "FairDetParIo.h"
#include "FairParIo.h"
#include "FairLogger.h"

#include "TString.h"

// -----   Standard constructor   ------------------------------------------
CbmStar2019TofPar::CbmStar2019TofPar(const char* name,
			             const char* title,
			             const char* context)
  : FairParGenericSet(name, title, context),
    fiNrOfGdpb(-1),
    fiGdpbIdArray(),
    fiNrOfModule( -1 ),
    fiNrOfGbtx(-1),
    fiNrOfRpc(),
    fiRpcType(),
    fiRpcSide(),
    fiModuleId(),
    fdSizeMsInNs(0.0),
    fdStarTriggAllowedSpread(0.0),
    fdStarTriggerDeadtime(),
    fdStarTriggerDelay(),
    fdStarTriggerWinSize()
{
   detName="Tof";
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmStar2019TofPar::~CbmStar2019TofPar()
{
}
// -------------------------------------------------------------------------


// -----   Public method clear   -------------------------------------------
void CbmStar2019TofPar::clear()
{
   status = kFALSE;
   resetInputVersions();
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------

void CbmStar2019TofPar::putParams(FairParamList* l)
{
   if (!l) return;
   l->add("NrOfGdpbs",           fiNrOfGdpb);
   l->add("GdpbIdArray",         fiGdpbIdArray);
   l->add("NrOfGbtx",            fiNrOfGbtx);
   l->add("NrOfModule",          fiNrOfModule);
   l->add("NrOfRpcs",            fiNrOfRpc);
   l->add("RpcType",             fiRpcType);
   l->add("RpcSide",             fiRpcSide);
   l->add("ModuleId",            fiModuleId);
   l->add("SizeMsInNs",          fdSizeMsInNs);
   l->add("StarTriggerDeadtime", fdStarTriggerDeadtime);
   l->add("StarTriggerDelay",    fdStarTriggerDelay);
   l->add("StarTriggerWinSize",  fdStarTriggerWinSize);
}

//------------------------------------------------------

Bool_t CbmStar2019TofPar::getParams(FairParamList* l) {

   if (!l) return kFALSE;

   if ( ! l->fill("NrOfGdpbs", &fiNrOfGdpb) ) return kFALSE;

   fiGdpbIdArray.Set(fiNrOfGdpb);
   if ( ! l->fill("GdpbIdArray", &fiGdpbIdArray) ) return kFALSE;

   if ( ! l->fill("NrOfGbtx", &fiNrOfGbtx) ) return kFALSE;

   if ( ! l->fill("NrOfModule", &fiNrOfModule) ) return kFALSE;

   fiNrOfRpc.Set(fiNrOfGbtx);
   if ( ! l->fill("NrOfRpc", &fiNrOfRpc) ) return kFALSE;

   fiRpcType.Set(fiNrOfGbtx);
   if ( ! l->fill("RpcType", &fiRpcType) ) return kFALSE;

   fiRpcSide.Set(fiNrOfGbtx);
   if ( ! l->fill("RpcSide", &fiRpcSide) ) return kFALSE;

   fiModuleId.Set(fiNrOfGbtx);
   if ( ! l->fill("ModuleId", &fiModuleId) ) return kFALSE;

   if ( ! l->fill("SizeMsInNs",  &fdSizeMsInNs) ) return kFALSE;

   fdStarTriggerDeadtime.Set(fiNrOfGdpb);
   fdStarTriggerDelay.Set(fiNrOfGdpb);
   fdStarTriggerWinSize.Set(fiNrOfGdpb);
   if ( ! l->fill("StarTriggerDeadtime", &fdStarTriggerDeadtime) ) return kFALSE;
   if ( ! l->fill("StarTriggerDelay",    &fdStarTriggerDelay) ) return kFALSE;
   if ( ! l->fill("StarTriggerWinSize",  &fdStarTriggerWinSize) ) return kFALSE;

   return kTRUE;
}

// -------------------------------------------------------------------------
Int_t CbmStar2019TofPar::Get4ChanToPadiChan( UInt_t uChannelInFee )
{
   if( uChannelInFee < kuNbChannelsPerFee )
      return kuGet4topadi[ uChannelInFee ] - 1;
      else
      {
         LOG(FATAL) << "CbmStar2019TofPar::Get4ChanToPadiChan => Index out of bound, "
                    << uChannelInFee << " vs " << kuNbChannelsPerFee
                    << ", returning crazy value!"
                    << FairLogger::endl;
         return -1;
      } // else of if( uChannelInFee < kuNbChannelsPerFee )
}
Int_t CbmStar2019TofPar::PadiChanToGet4Chan( UInt_t uChannelInFee )
{
   if( uChannelInFee < kuNbChannelsPerFee )
      return kuPaditoget4[ uChannelInFee ] - 1;
      else
      {
         LOG(FATAL) << "CbmStar2019TofPar::PadiChanToGet4Chan => Index out of bound, "
                    << uChannelInFee << " vs " << kuNbChannelsPerFee
                    << ", returning crazy value!"
                    << FairLogger::endl;
         return -1;
      } // else of if( uChannelInFee < kuNbChannelsPerFee )
}
// -------------------------------------------------------------------------
Int_t CbmStar2019TofPar::ElinkIdxToGet4Idx( UInt_t uElink )
{
   if( uElink < kuNbGet4PerGbtx )
      return kuGet4topadi[ uElink ];
      else
      {
         LOG(FATAL) << "CbmStar2019TofPar::ElinkIdxToGet4Idx => Index out of bound, "
                    << uElink << " vs " << kuNbGet4PerGbtx
                    << ", returning crazy value!"
                    << FairLogger::endl;
         return -1;
      } // else of if( uElink < kuNbGet4PerGbtx )
}
Int_t CbmStar2019TofPar::Get4IdxToElinkIdx( UInt_t uElink )
{
   if( uElink < kuNbGet4PerGbtx )
      return kuPaditoget4[ uElink ];
      else
      {
         LOG(FATAL) << "CbmStar2019TofPar::Get4IdxToElinkIdx => Index out of bound, "
                    << uElink << " vs " << kuNbGet4PerGbtx
                    << ", returning crazy value!"
                    << FairLogger::endl;
         return -1;
      } // else of if( uElink < kuNbGet4PerGbtx )
}
// -------------------------------------------------------------------------
Int_t CbmStar2019TofPar::GetNrOfRpc( UInt_t uGbtx )
{
   if( uGbtx < fiNrOfGbtx )
      return fiNrOfRpc[ uGbtx ];
      else
      {
         LOG(FATAL) << "CbmStar2019TofPar::GetNrOfRpc => Index out of bound, "
                    << uGbtx << " vs " << fiNrOfGbtx
                    << ", returning crazy value!"
                    << FairLogger::endl;
         return -1;
      } // else of if( uGbtx < fiNrOfGbtx )
}
Int_t CbmStar2019TofPar::GetRpcType( UInt_t uGbtx )
{
   if( uGbtx < fiNrOfGbtx )
      return fiRpcType[ uGbtx ];
      else
      {
         LOG(FATAL) << "CbmStar2019TofPar::GetRpcType => Index out of bound, "
                    << uGbtx << " vs " << fiNrOfGbtx
                    << ", returning crazy value!"
                    << FairLogger::endl;
         return -1;
      } // else of if( uGbtx < fiNrOfGbtx )
}
Int_t CbmStar2019TofPar::GetRpcSide( UInt_t uGbtx )
{
   if( uGbtx < fiNrOfGbtx )
      return fiRpcSide[ uGbtx ];
      else
      {
         LOG(FATAL) << "CbmStar2019TofPar::GetRpcSide => Index out of bound, "
                    << uGbtx << " vs " << fiNrOfGbtx
                    << ", returning crazy value!"
                    << FairLogger::endl;
         return -1;
      } // else of if( uGbtx < fiNrOfGbtx )
}
Int_t CbmStar2019TofPar::GetModuleId( UInt_t uGbtx )
{
   if( uGbtx < fiNrOfGbtx )
      return fiModuleId[ uGbtx ];
      else
      {
         LOG(FATAL) << "CbmStar2019TofPar::GetModuleId => Index out of bound, "
                    << uGbtx << " vs " << fiNrOfGbtx
                    << ", returning crazy value!"
                    << FairLogger::endl;
         return -1;
      } // else of if( uGbtx < fiNrOfGbtx )
}
// -------------------------------------------------------------------------
Double_t CbmStar2019TofPar::GetStarTriggDeadtime( UInt_t uGdpb )
{
   if( uGdpb < fiNrOfGdpb )
      return fdStarTriggerDeadtime[ uGdpb ];
      else
      {
         LOG(FATAL) << "CbmStar2019TofPar::GetStarTriggDeadtime => Index out of bound, "
                    << uGdpb << " vs " << fiNrOfGdpb
                    << ", returning crazy value!"
                    << FairLogger::endl;
         return -1;
      } // else of if( uGdpb < fiNrOfGdpb )
}
Double_t CbmStar2019TofPar::GetStarTriggDelay( UInt_t uGdpb )
{
   if( uGdpb < fiNrOfGdpb )
      return fdStarTriggerDelay[ uGdpb ];
      else
      {
         LOG(FATAL) << "CbmStar2019TofPar::GetStarTriggDelay => Index out of bound, "
                    << uGdpb << " vs " << fiNrOfGdpb
                    << ", returning crazy value!"
                    << FairLogger::endl;
         return -1;
      } // else of if( uGdpb < fiNrOfGdpb )
}
Double_t CbmStar2019TofPar::GetStarTriggWinSize( UInt_t uGdpb )
{
   if( uGdpb < fiNrOfGdpb )
      return fdStarTriggerWinSize[ uGdpb ];
      else
      {
         LOG(FATAL) << "CbmStar2019TofPar::GetStarTriggWinSize => Index out of bound, "
                    << uGdpb << " vs " << fiNrOfGdpb
                    << ", returning crazy value!"
                    << FairLogger::endl;
         return -1;
      } // else of if( uGdpb < fiNrOfGdpb )
}
// -------------------------------------------------------------------------

ClassImp(CbmStar2019TofPar)
