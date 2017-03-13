// -------------------------------------------------------------------------
// -----                CbmStsUnpackPar source file                    -----
// -----                Created 08.03.2017 by P.-A. Loizeau            -----
// -------------------------------------------------------------------------

#include "CbmStsUnpackPar.h"

#include "FairParamList.h"
#include "FairDetParIo.h"
#include "FairParIo.h"
#include "FairLogger.h"

#include "TString.h"

using namespace std;
// -----   Standard constructor   ------------------------------------------
CbmStsUnpackPar::CbmStsUnpackPar(const char* name, 
				   const char* title,
				   const char* context)
  : FairParGenericSet(name, title, context), 
    fNrOfnDpbsModA(-1),
    fnDbpsIdsArrayA(),
    fNrOfnDpbsModB(-1),
    fnDbpsIdsArrayB(),
    fuNbFebPerNdpb(-1),
    fNrOfFebs(-1),
    fnFebsIdsArrayA(),
    fnFebsIdsArrayB(),
    fNrOfChannels(-1),
    fChannelsToPadX(),
    fChannelsToPadY()
{
  detName="Much";
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmStsUnpackPar::~CbmStsUnpackPar() 
{
}
// -------------------------------------------------------------------------


// -----   Public method clear   -------------------------------------------
void CbmStsUnpackPar::clear() 
{
  status = kFALSE;
  resetInputVersions();
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------

void CbmStsUnpackPar::putParams(FairParamList* l) 
{
  if (!l) return;
  l->add("NrOfnDpbsModA",          fNrOfnDpbsModA);
  l->add("nDbpsIdsArrayA",         fnDbpsIdsArrayA);
  l->add("NrOfnDpbsModB",          fNrOfnDpbsModB);
  l->add("nDbpsIdsArrayB",         fnDbpsIdsArrayB);
  l->add("NbFebPerNdpb",           fuNbFebPerNdpb);
  l->add("NrOfFebs",               fNrOfFebs);
  l->add("nFebsIdsArrayA",         fnFebsIdsArrayA);
  l->add("nFebsIdsArrayB",         fnFebsIdsArrayB);
  l->add("NrOfChannels",           fNrOfChannels);
  l->add("ChannelsToPadX",         fChannelsToPadX);
  l->add("ChannelsToPadY",         fChannelsToPadY);
}

//------------------------------------------------------

Bool_t CbmStsUnpackPar::getParams(FairParamList* l) {

  if (!l) return kFALSE;
  
  LOG(INFO)<< " Before CbmStsUnpackPar GetParams at NrOfnDpbs " << FairLogger::endl;
  if ( ! l->fill("NrOfnDpbsModA", &fNrOfnDpbsModA) ) return kFALSE;
  LOG(INFO)<< " After CbmStsUnpackPar GetParams at NrOfnDpbs " << FairLogger::endl;
  
  fnDbpsIdsArrayA.Set(fNrOfnDpbsModA);
  if ( ! l->fill("nDbpsIdsArrayA", &fnDbpsIdsArrayA) ) return kFALSE;
  
  
  
  LOG(INFO)<< " Before CbmStsUnpackPar GetParams at NrOfnDpbs for GEM2 " << FairLogger::endl;
  if ( ! l->fill("NrOfnDpbsModB", &fNrOfnDpbsModB) ) return kFALSE;
  LOG(INFO)<< " After CbmStsUnpackPar GetParams at NrOfnDpbs " << FairLogger::endl;
  
  fnDbpsIdsArrayB.Set(fNrOfnDpbsModB);
  if ( ! l->fill("nDbpsIdsArrayB", &fnDbpsIdsArrayB) ) return kFALSE;
  
  if ( ! l->fill("NbFebPerNdpb", &fuNbFebPerNdpb) ) return kFALSE;
  
  if ( ! l->fill("NrOfFebs", &fNrOfFebs) ) return kFALSE;
  
  fnFebsIdsArrayA.Set(fNrOfFebs);
  if ( ! l->fill("nFebsIdsArrayA", &fnFebsIdsArrayA) ) return kFALSE;
  
  fnFebsIdsArrayB.Set(fNrOfFebs);
  if ( ! l->fill("nFebsIdsArrayB", &fnFebsIdsArrayB) ) return kFALSE;
  
  if ( ! l->fill("NrOfChannels", &fNrOfChannels) ) return kFALSE;
  
  fChannelsToPadX.Set(fNrOfFebs*fNrOfChannels);
  if ( ! l->fill("ChannelsToPadX", &fChannelsToPadX) ) return kFALSE;
  
  fChannelsToPadY.Set(fNrOfFebs*fNrOfChannels);
  if ( ! l->fill("ChannelsToPadY", &fChannelsToPadY) ) return kFALSE;
  
  return kTRUE;
}


Int_t CbmStsUnpackPar::GetPadX(Int_t febid, Int_t channelid)
{
  if( fChannelsToPadX.GetSize () <= (febid*fNrOfChannels)+channelid )
  {
    LOG(DEBUG) << "CbmStsUnpackPar::GetPadX => Index out of bounds: " 
	       << ((febid*fNrOfChannels)+channelid) << " VS " << fChannelsToPadX.GetSize()
	       << " (" << febid << " and " << channelid << ")"
	       << FairLogger::endl;
    return -2;
  } // if( fChannelsToPadX.GetSize () <= (febid*fNrOfChannels)+channelid )
  
  
  return fChannelsToPadX[(febid*fNrOfChannels)+channelid];
}
Int_t CbmStsUnpackPar::GetPadY(Int_t febid, Int_t channelid)
{
  if( fChannelsToPadY.GetSize() <= (febid*fNrOfChannels)+channelid )
  {
      LOG(DEBUG) << "CbmStsUnpackPar::GetPadY => Index out of bounds: " 
                 << ((febid*fNrOfChannels)+channelid) << " VS " << fChannelsToPadY.GetSize()
                 << " (" << febid << " and " << channelid << ")"
                 << FairLogger::endl;
    return -2;
  } // if( fChannelsToPadY.GetSize () <= (febid*fNrOfChannels)+channelid )
   
   return fChannelsToPadY[(febid*fNrOfChannels)+channelid];
}

ClassImp(CbmStsUnpackPar)
