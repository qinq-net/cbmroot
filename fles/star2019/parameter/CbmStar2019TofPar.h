// -------------------------------------------------------------------------
// -----                 CbmStar2019TofPar header file                 -----
// -----                 Created 09/09/18  by P.-A. Loizeau            -----
// -------------------------------------------------------------------------

#ifndef CBMSTAR2019TOFPAR_H
#define CBMSTAR2019TOFPAR_H

#include "FairParGenericSet.h"

#include "TArrayI.h"
#include "TArrayD.h"

class FairParIo;
class FairParamList;


class CbmStar2019TofPar : public FairParGenericSet
{

 public:

   /** Standard constructor **/
   CbmStar2019TofPar(const char* name      = "CbmStar2019TofPar",
          const char* title   = "Tof unpacker parameters",
          const char* context = "Default");


   /** Destructor **/
   virtual ~CbmStar2019TofPar();

   /** Reset all parameters **/
   virtual void clear();

   void putParams(FairParamList*);
   Bool_t getParams(FairParamList*);

   static constexpr UInt_t GetNbByteMessage() { return kuBytesPerMessage; }

   inline Int_t GetNrOfGdpbs() { return fiNrOfGdpb; }
   inline Int_t GetGdpbId(Int_t i) { return fiGdpbIdArray[i]; }
/*
   inline Int_t GetNrOfFeesPerGdpb() { return fiNrOfFeesPerGdpb; }
   inline Int_t GetNrOfGet4PerFee() {return fiNrOfGet4PerFee;}
   inline Int_t GetNrOfChannelsPerGet4() {return fiNrOfChannelsPerGet4;}
   */
   static constexpr UInt_t GetNrOfChannelsPerGet4() { return kuNbChannelsPerGet4; }
   static constexpr UInt_t GetNrOfGet4PerFee()      { return kuNbGet4PerFee; }
   static constexpr UInt_t GetNrOfFeePerGbtx()      { return kuNbFeePerGbtx; }
   static constexpr UInt_t GetNrOfGbtxPerGdpb()     { return kuNbGbtxPerGdpb; }
   static constexpr UInt_t GetNrOfChannelsPerFee()  { return kuNbChannelsPerFee; }
   static constexpr UInt_t GetNrOfChannelsPerGbtx() { return kuNbChannelsPerGbtx; }
   static constexpr UInt_t GetNrOfChannelsPerGdpb() { return kuNbChannelsPerGdpb; }
   static constexpr UInt_t GetNrOfGet4PerGbtx()     { return kuNbGet4PerGbtx; }
   static constexpr UInt_t GetNrOfGet4PerGdpb()     { return kuNbGet4PerGdpb; }
   static constexpr UInt_t GetNrOfFeePerGdpb()      { return kuNbFeePerGdpb; }

   inline UInt_t GetNumberOfChannels() { return kuNbChannelsPerGdpb * fiNrOfGdpb; }

   Int_t Get4ChanToPadiChan( UInt_t uChannelInFee );
   Int_t PadiChanToGet4Chan( UInt_t uChannelInFee );

   Int_t ElinkIdxToGet4Idx( UInt_t uElink );
   Int_t Get4IdxToElinkIdx( UInt_t uElink );

   inline Int_t GetNrOfGbtx() {return fiNrOfGbtx;}
   inline Int_t GetNrOfModules() {return fiNrOfModule;}
   Int_t GetNrOfRpc( UInt_t uGbtx );
   Int_t GetRpcType( UInt_t uGbtx );
   Int_t GetRpcSide( UInt_t uGbtx );
   Int_t GetModuleId( UInt_t uGbtx );

   inline Double_t GetSizeMsInNs() { return fdSizeMsInNs;}

   inline Double_t GetStarTriggAllowedSpread() { return fdStarTriggAllowedSpread;}
   Double_t GetStarTriggDeadtime( UInt_t uGdpb );
   Double_t GetStarTriggDelay( UInt_t uGdpb );
   Double_t GetStarTriggWinSize( UInt_t uGdpb );

 private:
   /// Constants
      /// Data format
   static const uint32_t kuBytesPerMessage = 8;
      /// Readout chain
   static const uint32_t kuNbChannelsPerGet4 = 4;
   static const uint32_t kuNbGet4PerFee      = 8;
   static const uint32_t kuNbFeePerGbtx      = 5;
   static const uint32_t kuNbGbtxPerGdpb     = 6;
   static const uint32_t kuNbChannelsPerFee  = kuNbChannelsPerGet4 * kuNbGet4PerFee;
   static const uint32_t kuNbChannelsPerGbtx = kuNbChannelsPerFee  * kuNbFeePerGbtx;
   static const uint32_t kuNbChannelsPerGdpb = kuNbChannelsPerGbtx * kuNbGbtxPerGdpb;
   static const uint32_t kuNbGet4PerGbtx     = kuNbGet4PerFee      * kuNbFeePerGbtx;
   static const uint32_t kuNbGet4PerGdpb     = kuNbGet4PerGbtx     * kuNbGbtxPerGdpb;
   static const uint32_t kuNbFeePerGdpb      = kuNbFeePerGbtx      * kuNbGbtxPerGdpb;
      /// Mapping in Readout chain PCBs
   const UInt_t kuGet4topadi[ kuNbChannelsPerFee ] = {  // provided by Jochen
           4,  3,  2,  1,
         24, 23, 22, 21,
          8,  7,  6,  5,
         28, 27, 26, 25,
         12, 11, 10,  9,
         32, 31, 30, 29,
         16, 15, 14, 13,
         20, 19, 18, 17
      }; //! Map from GET4 channel to PADI channel

   const UInt_t kuPaditoget4[ kuNbChannelsPerFee ] = {  // provided by Jochen
          4,  3,  2,  1,
         12, 11, 10,  9,
         20, 19, 18, 17,
         28, 27, 26, 25,
         32, 31, 30, 29,
          8,  7,  6,  5,
         16, 15, 14, 13,
         24, 23, 22, 21
      }; //! Map from PADI channel to GET4 channel
   const UInt_t kuElinkToGet4[ kuNbGet4PerGbtx ] = {
         27,  2,  7,  3, 31, 26, 30,  1,
         33, 37, 32, 13,  9, 14, 10, 15,
         17, 21, 16, 35, 34, 38, 25, 24,
          0,  6, 20, 23, 18, 22, 28,  4,
         29,  5, 19, 36, 39,  8, 12, 11
      };
   const UInt_t kuGet4ToElink[ kuNbGet4PerGbtx ] = {
         24,  7,  1,  3, 31, 33, 25,  2,
         37, 12, 14, 39, 38, 11, 13, 15,
         18, 16, 28, 34, 26, 17, 29, 27,
         23, 22,  5,  0, 30, 32,  6,  4,
         10,  8, 20, 19, 35,  9, 21, 36
      };

   /// Variables
   Int_t    fiNrOfGdpb; // Total number of GDPBs
   TArrayI  fiGdpbIdArray; // Array to hold the unique IDs for all Tof GDPBs
   Int_t    fiNrOfGbtx;   // Total number of Gbtx links
   Int_t    fiNrOfModule; // Total number of Modules
   TArrayI  fiNrOfRpc;    // number of Rpcs connected to Gbtx link, i.e. 3 or 5
   TArrayI  fiRpcType;    // type of Rpcs connected to Gbtx link
   TArrayI  fiRpcSide;    // side of Rpcs connected to Gbtx link, i.e. 0 or 1
   TArrayI  fiModuleId;   // Module Identifier connected to Gbtx link, has to match geometry

   Double_t fdSizeMsInNs; // Size of the MS in ns, needed for MS border detection

   Double_t fdStarTriggAllowedSpread; // Allowed "Jitter" of the triggers in different gDPB relative to each other, used to give a bit of flexibility on TS edges
   TArrayD  fdStarTriggerDeadtime; // STAR: Array to hold for each gDPB the deadtime between triggers in ns
   TArrayD  fdStarTriggerDelay; // STAR: Array to hold for each gDPB the Delay in ns to subtract when looking for beginning of coincidence of data with trigger window
   TArrayD  fdStarTriggerWinSize; // STAR: Array to hold for each gDPB the Size of the trigger window in ns

   ClassDef(CbmStar2019TofPar,1);
};
#endif // CBMSTAR2019TOFPAR_H
