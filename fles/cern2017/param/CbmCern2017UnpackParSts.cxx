// -------------------------------------------------------------------------
// -----            CbmCern2017UnpackParSts source file                -----
// -----            Created 25/07/17  by P.-A. Loizeau                 -----
// -------------------------------------------------------------------------

#include "CbmCern2017UnpackParSts.h"

#include "FairParamList.h"
#include "FairDetParIo.h"
#include "FairParIo.h"
#include "FairLogger.h"

#include "TString.h"

using namespace std;
// -----   Standard constructor   ------------------------------------------
CbmCern2017UnpackParSts::CbmCern2017UnpackParSts(const char* name,
                                                 const char* title,
                                                 const char* context) :
   FairParGenericSet(name, title, context),
   fuNrOfDpbs( 0 ),
   fiDbpIdArray(),
   fuNbElinksPerDpb( 0 ),
   fuNbStsXyters( 0 ),
   fuNbChanPerAsic( 0 ),
   fiElinkToAsicMap()
{
  detName="Much";
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmCern2017UnpackParSts::~CbmCern2017UnpackParSts()
{
}
// -------------------------------------------------------------------------


// -----   Public method clear   -------------------------------------------
void CbmCern2017UnpackParSts::clear()
{
   status = kFALSE;
   resetInputVersions();
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------

void CbmCern2017UnpackParSts::putParams(FairParamList* l)
{
   if (!l) return;

#ifdef VERSION_GREATER_160601
   l->add("NrOfDpbs",       fuNrOfDpbs );
   l->add("DbpIdArray",     fiDbpIdArray);
   l->add("NbElinksPerDpb", fuNbElinksPerDpb );
   l->add("NbStsXyters",    fuNbStsXyters );
   l->add("NbChanPerAsic",  fuNbChanPerAsic );
   l->add("ElinkToAsicMap", fiElinkToAsicMap);
#else
   Int_t iNrOfDpbs       = fuNrOfDpbs;
   Int_t iNbElinksPerDpb = fuNbElinksPerDpb;
   Int_t iNbStsXyters    = fuNbStsXyters;
   Int_t iNbChanPerAsic  = fuNbChanPerAsic;

   l->add("NrOfDpbs",       iNrOfDpbs );
   l->add("DbpIdArray",     fiDbpIdArray);
   l->add("NbElinksPerDpb", iNbElinksPerDpb );
   l->add("NbStsXyters",    iNbStsXyters );
   l->add("NbChanPerAsic",  iNbChanPerAsic );
   l->add("ElinkToAsicMap", fiElinkToAsicMap);
#endif // VERSION_GREATER_160601
}

// -------------------------------------------------------------------------

Bool_t CbmCern2017UnpackParSts::getParams(FairParamList* l) {

   if (!l) return kFALSE;

   if ( ! l->fill("NrOfDpbs", &fuNrOfDpbs ) ) return kFALSE;

   fiDbpIdArray.Set( fuNrOfDpbs );
   if ( ! l->fill("DbpIdArray", &fiDbpIdArray ) ) return kFALSE;

   if ( ! l->fill("NbElinksPerDpb", &fuNbElinksPerDpb ) ) return kFALSE;

   if ( ! l->fill("NbStsXyters", &fuNbStsXyters ) ) return kFALSE;

   if ( ! l->fill("NbChanPerAsic", &fuNbChanPerAsic ) ) return kFALSE;

   fiElinkToAsicMap.Set( fuNrOfDpbs * fuNbElinksPerDpb );
   if ( ! l->fill("ElinkToAsicMap", &fiElinkToAsicMap ) ) return kFALSE;

  return kTRUE;
}
// -------------------------------------------------------------------------
UInt_t CbmCern2017UnpackParSts::GetDpbId( UInt_t uDpbIdx )
{
   if( uDpbIdx < fuNrOfDpbs )
      return fiDbpIdArray[ uDpbIdx ];
      else
      {
         LOG(WARNING) << "CbmCern2017UnpackParSts::GetDpbId => Index out of bound, "
                      << "returning crazy value!"
                      << FairLogger::endl;
         return 0xFFFFFFFF;
      } // else of if( uDpbIdx < fuNrOfDpbs )
}
UInt_t CbmCern2017UnpackParSts::GetElinkToAsicIdx( UInt_t uElinkIdx )
{
   if( uElinkIdx < fuNbElinksPerDpb )
      return fiElinkToAsicMap[ uElinkIdx ];
      else
      {
         LOG(WARNING) << "CbmCern2017UnpackParSts::GetElinkToAsicIdx => Index out of bound, "
                      << "returning crazy value!"
                      << FairLogger::endl;
         return (fuNbStsXyters + 1);
      } // else of if( uDpbIdx < fuNrOfDpbs )
}

ClassImp(CbmCern2017UnpackParSts)
