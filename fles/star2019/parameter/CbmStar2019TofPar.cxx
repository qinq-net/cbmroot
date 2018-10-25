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
    fiNrOfFeesPerGdpb(-1),
    fiNrOfGet4PerFee(-1),
    fiNrOfChannelsPerGet4(-1),
    fiNrOfModule( -1 ),
    fiNrOfGbtx(-1),
    fiNrOfRpc(),
    fiRpcType(),
    fiRpcSide(),
    fiModuleId(),
    fiNbMsTot(0),
    fiNbMsOverlap(0),
    fdSizeMsInNs(0.0),
    fdStarTriggerDeadtime(),
    fdStarTriggerDelay(),
    fdStarTriggerWinSize(),
    fdTsDeadtimePeriod(0.0)
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
   l->add("NrOfFeesPerGdpb",     fiNrOfFeesPerGdpb);
   l->add("NrOfGet4PerFee",      fiNrOfGet4PerFee);
   l->add("NrOfChannelsPerGet4", fiNrOfChannelsPerGet4);
   l->add("NrOfGbtx",            fiNrOfGbtx);
   l->add("NrOfModule",          fiNrOfModule);
   l->add("NrOfRpcs",            fiNrOfRpc);
   l->add("RpcType",             fiRpcType);
   l->add("RpcSide",             fiRpcSide);
   l->add("ModuleId",            fiModuleId);
   l->add("NbMsTot",             fiNbMsTot);
   l->add("NbMsOverlap",         fiNbMsOverlap);
   l->add("SizeMsInNs",          fdSizeMsInNs);
   l->add("StarTriggerDeadtime", fdStarTriggerDeadtime);
   l->add("StarTriggerDelay",    fdStarTriggerDelay);
   l->add("StarTriggerWinSize",  fdStarTriggerWinSize);
   l->add("TsDeadtimePeriod",    fdTsDeadtimePeriod);
}

//------------------------------------------------------

Bool_t CbmStar2019TofPar::getParams(FairParamList* l) {

   if (!l) return kFALSE;

   if ( ! l->fill("NrOfGdpbs", &fiNrOfGdpb) ) return kFALSE;

   fiGdpbIdArray.Set(fiNrOfGdpb);
   if ( ! l->fill("GdpbIdArray", &fiGdpbIdArray) ) return kFALSE;

   if ( ! l->fill("NrOfFeesPerGdpb", &fiNrOfFeesPerGdpb) ) return kFALSE;
   if ( ! l->fill("NrOfGet4PerFee", &fiNrOfGet4PerFee) ) return kFALSE;
   if ( ! l->fill("NrOfChannelsPerGet4", &fiNrOfChannelsPerGet4) ) return kFALSE;

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

   if ( ! l->fill("NbMsTot",     &fiNbMsTot) ) return kFALSE;
   if ( ! l->fill("NbMsOverlap", &fiNbMsOverlap) ) return kFALSE;
   if ( ! l->fill("SizeMsInNs",  &fdSizeMsInNs) ) return kFALSE;

   fdStarTriggerDeadtime.Set(fiNrOfGdpb);
   fdStarTriggerDelay.Set(fiNrOfGdpb);
   fdStarTriggerWinSize.Set(fiNrOfGdpb);
   if ( ! l->fill("StarTriggerDeadtime", &fdStarTriggerDeadtime) ) return kFALSE;
   if ( ! l->fill("StarTriggerDelay",    &fdStarTriggerDelay) ) return kFALSE;
   if ( ! l->fill("StarTriggerWinSize",  &fdStarTriggerWinSize) ) return kFALSE;
   if ( ! l->fill("TsDeadtimePeriod",  &fdTsDeadtimePeriod) ) return kFALSE;

   return kTRUE;
}


ClassImp(CbmStar2019TofPar)
