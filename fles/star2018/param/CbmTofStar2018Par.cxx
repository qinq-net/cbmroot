// -------------------------------------------------------------------------
// -----                CbmTofStar2017Par source file                  -----
// -----                Created 22/01/17  by P.-A. Loizeau             -----
// -------------------------------------------------------------------------

#include "CbmTofStar2017Par.h"

#include "FairParamList.h"
#include "FairDetParIo.h"
#include "FairParIo.h"
#include "FairLogger.h"

#include "TString.h"

// -----   Standard constructor   ------------------------------------------
CbmTofStar2017Par::CbmTofStar2017Par(const char* name, 
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
    fPlotChannelRate(0),
    fiStarSortAndCut(0),
    fiStarActiveMasksArray(),
    fdStarTriggerDelay(),
    fdStarTriggerWinSize()
{
  detName="Tof";
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmTofStar2017Par::~CbmTofStar2017Par() 
{
}
// -------------------------------------------------------------------------


// -----   Public method clear   -------------------------------------------
void CbmTofStar2017Par::clear() 
{
  status = kFALSE;
  resetInputVersions();
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------

void CbmTofStar2017Par::putParams(FairParamList* l) 
{
  if (!l) return;
  l->add("NrOfRocs",          fNrOfGdpb);
  l->add("RocIdArray",        fGdpbIdArray);
  l->add("NrOfFebsPerGdpb",    fNrOfFebsPerGdpb);
  l->add("NrOfGet4PerFeb",     fNrOfGet4PerFeb);
  l->add("NrOfChannelsPerGet4",fNrOfChannelsPerGet4);
  l->add("NrOfChannels",       fNrOfChannels);
  l->add("ChannelToDetUIdMap", fChannelToDetUIdMap);
  l->add("PlotChannelRate",    fPlotChannelRate);
  l->add("StarSortAndCut",       fiStarSortAndCut);
  l->add("StarActiveMasksArray", fiStarActiveMasksArray);
  l->add("StarTriggerDelay",     fdStarTriggerDelay);
  l->add("StarTriggerWinSize",   fdStarTriggerWinSize);
}

//------------------------------------------------------

Bool_t CbmTofStar2017Par::getParams(FairParamList* l) {

  if (!l) return kFALSE;
  
  if ( ! l->fill("NrOfRocs", &fNrOfGdpb) ) return kFALSE;
  
  fGdpbIdArray.Set(fNrOfGdpb);
  if ( ! l->fill("RocIdArray", &fGdpbIdArray) ) return kFALSE;

  if ( ! l->fill("NrOfFebsPerGdpb", &fNrOfFebsPerGdpb) ) return kFALSE;
  if ( ! l->fill("NrOfGet4PerFeb", &fNrOfGet4PerFeb) ) return kFALSE;
  if ( ! l->fill("NrOfChannelsPerGet4", &fNrOfChannelsPerGet4) ) return kFALSE;

  if ( ! l->fill("NrOfChannels", &fNrOfChannels) ) return kFALSE;

  fChannelToDetUIdMap.Set(fNrOfChannels);

  if ( ! l->fill("ChannelToDetUIdMap", &fChannelToDetUIdMap) ) return kFALSE;

//  if ( ! l->fill("PlotChannelRate", &fPlotChannelRate) ) return kFALSE;
  l->fill("PlotChannelRate", &fPlotChannelRate); // Optionally read this flag, do not crash if not there
  
  l->fill("StarSortAndCut", &fiStarSortAndCut); // Optionally read this value, do not crash if not there
  if( IsStarSortAndCutEnabled() )
  {
     fiStarActiveMasksArray.Set(fNrOfGdpb);
     fdStarTriggerDelay.Set(fNrOfGdpb);
     fdStarTriggerWinSize.Set(fNrOfGdpb);
     l->fill("StarActiveMasksArray", &fiStarActiveMasksArray); // Optionally read this value, do not crash if not there
     l->fill("StarTriggerDelay",     &fdStarTriggerDelay); // Optionally read this value, do not crash if not there
     l->fill("StarTriggerWinSize",   &fdStarTriggerWinSize); // Optionally read this value, do not crash if not there
  } // if( IsStarSortAndCutEnabled() )
  
  return kTRUE;
}


ClassImp(CbmTofStar2017Par)
