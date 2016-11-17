// -------------------------------------------------------------------------
// -----                CbmTofUnpackPar source file                  -----
// -----                Created 02/11/16  by F. Uhlig                  -----
// -------------------------------------------------------------------------

#include "CbmTofUnpackPar.h"

#include "FairParamList.h"
#include "FairDetParIo.h"
#include "FairParIo.h"
#include "FairLogger.h"

#include "TString.h"

// -----   Standard constructor   ------------------------------------------
CbmTofUnpackPar::CbmTofUnpackPar(const char* name, 
			             const char* title,
			             const char* context)
  : FairParGenericSet(name, title, context), 
    fNrOfGdpb(-1),
    fGdpbIdArray(),
    fNrOfFebsPerGdpb(-1),
    fNrOfGet4PerFeb(-1),
    fNrOfChannelsPerGet4(-1),
    fNrOfChannels(-1),
    fChannelToDetUIdMap(),
    fPlotChannelRate(0)
{
  detName="Tof";
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmTofUnpackPar::~CbmTofUnpackPar() 
{
}
// -------------------------------------------------------------------------


// -----   Public method clear   -------------------------------------------
void CbmTofUnpackPar::clear() 
{
  status = kFALSE;
  resetInputVersions();
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------

void CbmTofUnpackPar::putParams(FairParamList* l) 
{
  if (!l) return;
  l->add("NrOfGdpbs",          fNrOfGdpb);
  l->add("GdpbIdArray",        fGdpbIdArray);
  l->add("NrOfFebsPerGdpb",    fNrOfFebsPerGdpb);
  l->add("NrOfGet4PerFeb",     fNrOfGet4PerFeb);
  l->add("NrOfChannelsPerGet4",fNrOfChannelsPerGet4);
  l->add("NrOfChannels",       fNrOfChannels);
  l->add("ChannelToDetUIdMap", fChannelToDetUIdMap);
  l->add("PlotChannelRate",    fPlotChannelRate);
}

//------------------------------------------------------

Bool_t CbmTofUnpackPar::getParams(FairParamList* l) {

  if (!l) return kFALSE;
  
  if ( ! l->fill("NrOfGdpbs", &fNrOfGdpb) ) return kFALSE;
  
  fGdpbIdArray.Set(fNrOfGdpb);
  if ( ! l->fill("GdpbIdArray", &fGdpbIdArray) ) return kFALSE;

  if ( ! l->fill("NrOfFebsPerGdpb", &fNrOfFebsPerGdpb) ) return kFALSE;
  if ( ! l->fill("NrOfGet4PerFeb", &fNrOfGet4PerFeb) ) return kFALSE;
  if ( ! l->fill("NrOfChannelsPerGet4", &fNrOfChannelsPerGet4) ) return kFALSE;

  if ( ! l->fill("NrOfChannels", &fNrOfChannels) ) return kFALSE;

  fChannelToDetUIdMap.Set(fNrOfChannels);

  if ( ! l->fill("ChannelToDetUIdMap", &fChannelToDetUIdMap) ) return kFALSE;

//  if ( ! l->fill("PlotChannelRate", &fPlotChannelRate) ) return kFALSE;
  l->fill("PlotChannelRate", &fPlotChannelRate); // Optionally read this flag, do not crash if not there
  
  return kTRUE;
}


ClassImp(CbmTofUnpackPar)
