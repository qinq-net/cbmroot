// -------------------------------------------------------------------------
// -----                CbmMuchUnpackPar source file                  -----
// -----                Created 11/11/16  by V. Singhal and A. Kumar                  -----
// -------------------------------------------------------------------------

#include "CbmMuchUnpackPar.h"

#include "FairParamList.h"
#include "FairDetParIo.h"
#include "FairParIo.h"
#include "FairLogger.h"

#include "TString.h"

using namespace std;
// -----   Standard constructor   ------------------------------------------
CbmMuchUnpackPar::CbmMuchUnpackPar(const char* name, 
			             const char* title,
			             const char* context)
  : FairParGenericSet(name, title, context), 
    fNrOfnDpbsModA(-1),
    fnDbpsIdsArray(),
    fNrOfFebs(-1),
    fnFebsIdsArray(),
    fNrOfChannels(-1),
    fChannelsToPadX(),
    fChannelsToPadY()
{
  detName="Much";
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmMuchUnpackPar::~CbmMuchUnpackPar() 
{
}
// -------------------------------------------------------------------------


// -----   Public method clear   -------------------------------------------
void CbmMuchUnpackPar::clear() 
{
  status = kFALSE;
  resetInputVersions();
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------

void CbmMuchUnpackPar::putParams(FairParamList* l) 
{
  if (!l) return;
  l->add("NrOfnDpbsModA",          fNrOfnDpbsModA);
  l->add("nDbpsIdsArray",        fnDbpsIdsArray);
  l->add("NrOfFebs",      fNrOfFebs);
  l->add("nFebsIdsArray", fnFebsIdsArray);
  l->add("NrOfChannels", fNrOfChannels);
  l->add("ChannelsToPadX", fChannelsToPadX);
  l->add("ChannelsToPadY", fChannelsToPadY);
}

//------------------------------------------------------

Bool_t CbmMuchUnpackPar::getParams(FairParamList* l) {

  if (!l) return kFALSE;
  
  LOG(INFO)<< " Before CbmMuchUnpackPar GetParams at NrOfnDpbs " << FairLogger::endl;
  if ( ! l->fill("NrOfnDpbsModA", &fNrOfnDpbsModA) ) return kFALSE;
 LOG(INFO)<< " After CbmMuchUnpackPar GetParams at NrOfnDpbs " << FairLogger::endl;

  fnDbpsIdsArray.Set(fNrOfnDpbsModA);
  if ( ! l->fill("nDbpsIdsArray", &fnDbpsIdsArray) ) return kFALSE;

  if ( ! l->fill("NrOfFebs", &fNrOfFebs) ) return kFALSE;

  fnFebsIdsArray.Set(fNrOfFebs);

  if ( ! l->fill("NrOfChannels", &fNrOfChannels) ) return kFALSE;
  
  fChannelsToPadX.Set(fNrOfChannels);
  if ( ! l->fill("ChannelsToPadX", &fChannelsToPadX) ) return kFALSE;

  fChannelsToPadY.Set(fNrOfChannels);
  if ( ! l->fill("ChannelsToPadY", &fChannelsToPadY) ) return kFALSE;

  return kTRUE;
}


ClassImp(CbmMuchUnpackPar)
