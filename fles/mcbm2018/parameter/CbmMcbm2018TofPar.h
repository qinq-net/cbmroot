// -------------------------------------------------------------------------
// -----                 CbmMcbm2018TofPar header file                 -----
// -----                 Created 09/09/18  by P.-A. Loizeau            -----
// -------------------------------------------------------------------------

#ifndef CBMMCBM2018TOFPAR_H
#define CBMMCBM2018TOFPAR_H

#include "FairParGenericSet.h"

#include "TArrayI.h"
#include "TArrayD.h"

class FairParIo;
class FairParamList;


class CbmMcbm2018TofPar : public FairParGenericSet
{

 public:

   /** Standard constructor **/
   CbmMcbm2018TofPar(const char* name      = "CbmMcbm2018TofPar",
          const char* title   = "Tof unpacker parameters",
          const char* context = "Default");


   /** Destructor **/
   virtual ~CbmMcbm2018TofPar();

   /** Reset all parameters **/
   virtual void clear();

   void putParams(FairParamList*);
   Bool_t getParams(FairParamList*);

   inline Int_t GetNrOfGdpbs() { return fiNrOfGdpb; }
   inline Int_t GetGdpbId(Int_t i) { return fiGdpbIdArray[i]; }
   inline Int_t GetNrOfFeesPerGdpb() { return fiNrOfFeesPerGdpb; }
   inline Int_t GetNrOfGet4PerFee() {return fiNrOfGet4PerFee;}
   inline Int_t GetNrOfChannelsPerGet4() {return fiNrOfChannelsPerGet4;}

   inline Int_t GetNrOfGbtx() {return fiNrOfGbtx;}
   inline Int_t GetNrOfModules() {return fiNrOfModule;}
   inline Int_t GetNrOfRpc(Int_t i) {return fiNrOfRpc[i];}
   inline Int_t GetRpcType(Int_t i) {return fiRpcType[i];}
   inline Int_t GetRpcSide(Int_t i) {return fiRpcSide[i];}
   inline Int_t GetModuleId(Int_t i) {return fiModuleId[i];}

   inline Int_t    GetNbMsTot() { return fiNbMsTot;}
   inline Int_t    GetNbMsOverlap() { return fiNbMsOverlap;}
   inline Double_t GetSizeMsInNs() { return fdSizeMsInNs;}

   inline Double_t GetStarTriggDeadtime(Int_t gdpb) { return fdStarTriggerDeadtime[gdpb];}
   inline Double_t GetStarTriggDelay(Int_t gdpb) { return fdStarTriggerDelay[gdpb];}
   inline Double_t GetStarTriggWinSize(Int_t gdpb) { return fdStarTriggerWinSize[gdpb];}
   inline Double_t GetTsDeadtimePeriod() { return fdTsDeadtimePeriod;}

 private:

   Int_t    fiNrOfGdpb; // Total number of GDPBs
   TArrayI  fiGdpbIdArray; // Array to hold the unique IDs for all Tof GDPBs

   Int_t    fiNrOfFeesPerGdpb; // Number of FEEs which are connected to one GDPB
   Int_t    fiNrOfGet4PerFee;  // Number of GET4 chips which are connected to one FEB
   Int_t    fiNrOfChannelsPerGet4;  // Number of channels per GET4

   Int_t    fiNrOfGbtx;   // Total number of Gbtx links 
   Int_t    fiNrOfModule; // Total number of Modules 
   TArrayI  fiNrOfRpc;    // number of Rpcs connected to Gbtx link, i.e. 3 or 5
   TArrayI  fiRpcType;    // type of Rpcs connected to Gbtx link
   TArrayI  fiRpcSide;    // side of Rpcs connected to Gbtx link, i.e. 0 or 1
   TArrayI  fiModuleId;   // Module Identifier connected to Gbtx link, has to match geometry
 
   Int_t    fiNbMsTot; // Total number of MS per link in TS
   Int_t    fiNbMsOverlap; // Number of overlap MS per TS
   Double_t fdSizeMsInNs; // Size of the MS in ns, needed for MS border detection

   TArrayD  fdStarTriggerDeadtime; // STAR: Array to hold for each gDPB the deadtime between triggers in ns
   TArrayD  fdStarTriggerDelay; // STAR: Array to hold for each gDPB the Delay in ns to subtract when looking for beginning of coincidence of data with trigger window
   TArrayD  fdStarTriggerWinSize; // STAR: Array to hold for each gDPB the Size of the trigger window in ns
   Double_t fdTsDeadtimePeriod; // Period (ns) in the first MS of each TS where events with missing triggers should be built using the overlap MS of previous TS (overlap events)

   ClassDef(CbmMcbm2018TofPar,1);
};
#endif // CBMMCBM2018TOFPAR_H
