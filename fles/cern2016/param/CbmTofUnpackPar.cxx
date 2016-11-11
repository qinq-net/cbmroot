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
    fNrOfRocs(-1),
    fRocIdArray(),
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
  l->add("NrOfRocs",          fNrOfRocs);
  l->add("RocIdArray",        fRocIdArray);
  l->add("NrOfChannels",      fNrOfRocs);
  l->add("ChannelToDetUIdMap",fChannelToDetUIdMap);
  l->add("PlotChannelRate",   fPlotChannelRate);
}

//------------------------------------------------------

Bool_t CbmTofUnpackPar::getParams(FairParamList* l) {

  if (!l) return kFALSE;
  
  if ( ! l->fill("NrOfRocs", &fNrOfRocs) ) return kFALSE;
  
  fRocIdArray.Set(fNrOfRocs);
  if ( ! l->fill("RocIdArray", &fRocIdArray) ) return kFALSE;

  if ( ! l->fill("NrOfChannels", &fNrOfChannels) ) return kFALSE;

  fChannelToDetUIdMap.Set(fNrOfChannels);

  if ( ! l->fill("ChannelToDetUIdMap", &fChannelToDetUIdMap) ) return kFALSE;

//  if ( ! l->fill("PlotChannelRate", &fPlotChannelRate) ) return kFALSE;
  l->fill("PlotChannelRate", &fPlotChannelRate); // Optionally read this flag, do not crash if not there
  
  return kTRUE;
}


ClassImp(CbmTofUnpackPar)
