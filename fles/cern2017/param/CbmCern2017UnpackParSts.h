// -------------------------------------------------------------------------
// -----            CbmCern2017UnpackParSts header file                -----
// -----            Created 25/07/17  by P.-A. Loizeau                 -----
// -------------------------------------------------------------------------

#ifndef CBMCERN2017UNPACKPARSTS_H
#define CBMCERN2017UNPACKPARSTS_H

#include "FairParGenericSet.h"

#include "TArrayI.h"

class FairParIo;
class FairParamList;


class CbmCern2017UnpackParSts : public FairParGenericSet
{

 public:

   /** Standard constructor **/
   CbmCern2017UnpackParSts(const char* name    = "CbmCern2017UnpackParSts",
                         const char* title   = "Sts unpacker parameters",
                         const char* context = "Default");


   /** Destructor **/
   virtual ~CbmCern2017UnpackParSts();

   /** Reset all parameters **/
   virtual void clear();

   void putParams(FairParamList*);
   Bool_t getParams(FairParamList*);

   UInt_t GetNrOfDpbs()       { return fuNrOfDpbs; }
   UInt_t GetDpbId( UInt_t uDpbIdx );
   UInt_t GetNbElinksPerDpb() { return fuNbElinksPerDpb; }
   UInt_t GetNbStsXyters()    { return fuNbStsXyters; }
   UInt_t GetNbChanPerAsic()  { return fuNbChanPerAsic; }
   UInt_t GetElinkToAsicIdx( UInt_t uElinkIdx );

 private:

   UInt_t  fuNrOfDpbs;       // Total number of STS DPBs in system
   TArrayI fiDbpIdArray;     // Array to hold the unique IDs (equipment ID) for all STS DPBs
   UInt_t  fuNbElinksPerDpb; // Number of elinks connected to each DPB
   UInt_t  fuNbStsXyters;    // Number of StsXyter ASICs
   UInt_t  fuNbChanPerAsic;  // Number of channels per StsXyter ASIC
   TArrayI fiElinkToAsicMap; // Array to hold for each link the corresponding ASIC index [fuNrOfDpbs * fuNbElinksPerDpb]


  ClassDef(CbmCern2017UnpackParSts,1);
};
#endif // CBMCERN2017UNPACKPARSTS_H
