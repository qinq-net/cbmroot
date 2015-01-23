// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                      CbmTSUnpackGet4v1x                           -----
// -----                    Created 15.12.2014 by                          -----
// -----                        P.-A. Loizeau                              -----
// -----                  Based on CbmTSUnpackGet4v1x                       -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmTSUnpackGet4v1x.h"

// Specific headers
//#include "CbmNxyterRawMessage.h"
//#include "CbmNxyterRawSyncMessage.h"
#include "CbmGet4v1xDef.h"

// FAIRROOT headers
#include "FairLogger.h"
#include "FairRootManager.h"

// ROOT headers
#include "TF1.h"
#include "TH2.h"
#include "TClonesArray.h"
#include "TFile.h"

// C++ std headers
#include <iostream>
#include <stdint.h>

struct DTM_header
{
   uint8_t packet_length;
   uint8_t packet_counter;
   uint16_t ROC_ID;

   void Dump() {
      printf ("HEADER ======================= pack_len=0x%02X, pack_cnt=0x%02X, ROC=0x%04X\n",
               packet_length, packet_counter, ROC_ID);
   }
};

CbmTSUnpackGet4v1x::CbmTSUnpackGet4v1x()
  : CbmTSUnpack(),
	fbVerbose(kFALSE),
	fiMode(0),
   fuNbRocs(0),
   fuNbGet4(0),
   fhMessageTypePerRoc(0),
   fhSysMessTypePerRoc(0),
   fhGet4EpochFlags(0),
   fhGet4EpochSyncDist(0),
   fhGet4ChanDataCount(0),
   fhGet4ChanDllStatus(0),
   fhGet4ChanTotMap(0),
   fhGet4ChanErrors(0),
   fhGet4ChanSlowContM(0)
//    fNxyterRaw(new TClonesArray("CbmNxyterRawMessage", 10)),
//    fNxyterRawSync(new TClonesArray("CbmNxyterRawSyncMessage", 10))
{
}

CbmTSUnpackGet4v1x::~CbmTSUnpackGet4v1x()
{
}

Bool_t CbmTSUnpackGet4v1x::Init()
{
  LOG(INFO) << "Initializing" << FairLogger::endl;

  FairRootManager* ioman = FairRootManager::Instance();
  if (ioman == NULL) {
    LOG(FATAL) << "No FairRootManager instance" << FairLogger::endl;
  }
//  ioman->Register("Get4RawMessage",       "GET4 raw data",      fGet4Raw, kTRUE);
//  ioman->Register("NxyterRawSyncMessage", "GET4 raw sync data", fGet4RawSync, kTRUE);

  if( 0 >= fuNbRocs || 0 >= fuNbGet4 )
  {
     LOG(ERROR) << "CbmTSUnpackGet4v1x::Init => Nb of ROCs or GET4s not defined!!! " << FairLogger::endl;
     LOG(ERROR) << "Nb of ROCs:"<< fuNbRocs << " Nb of GET4s : " << fuNbGet4 << FairLogger::endl;
     LOG(FATAL) << "Please use the functions SetRocNb and/or SetGet4Nb before running!!" << FairLogger::endl;
  }

  // Prepare the epoch storing vectors
  fvuCurrEpoch.resize( fuNbRocs);
  for( UInt_t uRoc = 0; uRoc < fuNbRocs; uRoc++)
     fvuCurrEpoch[uRoc] = 0;
  fvuCurrEpoch2.resize(fuNbGet4);
  for( UInt_t uChip = 0; uChip < fuNbGet4; uChip++)
     fvuCurrEpoch2[uChip] = 0;

  InitMonitorHistograms();

  return kTRUE;
}


void CbmTSUnpackGet4v1x::SetPulserChans(
      Int_t inPulserChanA, Int_t inPulserChanB, Int_t inPulserChanC, Int_t inPulserChanD,
      Int_t inPulserChanE, Int_t inPulserChanF, Int_t inPulserChanG, Int_t inPulserChanH,
      Int_t inPulserChanI, Int_t inPulserChanJ, Int_t inPulserChanK, Int_t inPulserChanL,
      Int_t inPulserChanM, Int_t inPulserChanN, Int_t inPulserChanO, Int_t inPulserChanP )
{
   fiPulserChan[ 0] = inPulserChanA;
   fiPulserChan[ 1] = inPulserChanB;
   fiPulserChan[ 2] = inPulserChanC;
   fiPulserChan[ 3] = inPulserChanD;
   fiPulserChan[ 4] = inPulserChanE;
   fiPulserChan[ 5] = inPulserChanF;
   fiPulserChan[ 6] = inPulserChanG;
   fiPulserChan[ 7] = inPulserChanH;
   fiPulserChan[ 8] = inPulserChanI;
   fiPulserChan[ 9] = inPulserChanJ;
   fiPulserChan[10] = inPulserChanK;
   fiPulserChan[11] = inPulserChanL;
   fiPulserChan[12] = inPulserChanM;
   fiPulserChan[13] = inPulserChanN;
   fiPulserChan[14] = inPulserChanO;
   fiPulserChan[15] = inPulserChanP;
}

Bool_t CbmTSUnpackGet4v1x::DoUnpack(const fles::Timeslice& ts, size_t component)
{
   // Loop over microslices
   for (size_t m = 0; m < ts.num_microslices(component); ++m)
   {
      auto msDescriptor = ts.descriptor(component, m);
      const uint8_t* msContent = reinterpret_cast<const uint8_t*>(ts.content(component, m));

      uint32_t offset = 16;
      uint32_t local_offset = 0;
      const uint8_t* msContent_shifted;
      int padding;

      // Loop over the data of one microslice
      while (offset < msDescriptor.size)
      {
         msContent_shifted = &msContent[offset];

         // Extract DTM header info
         DTM_header cur_DTM_header;

         cur_DTM_header.packet_length = msContent_shifted[0];
         cur_DTM_header.packet_counter = msContent_shifted[1];
         const uint16_t* ROC_ID_pointer = reinterpret_cast<const uint16_t*>(&msContent_shifted[2]);
         cur_DTM_header.ROC_ID = *ROC_ID_pointer;

         uint32_t packageSize = static_cast<uint32_t>(cur_DTM_header.packet_length*2+4);

         // Loop over messages
         local_offset = 4;
         if( 0 == fiMode || kTRUE == fbVerbose )
         {
            // Debug printout
            cur_DTM_header.Dump();
            LOG(INFO)<<" Package Size: "<<packageSize<<FairLogger::endl;
            while (local_offset < packageSize)
            {
               // FIXME: This is here just until the bytes order in the ums is found
               // Then it should be removed (Q? is big vs small endian)
               this->Print6bytesMessage(&msContent_shifted[local_offset]);

               get4v1x::Message mess;
               uint64_t dataContent =
                       ( static_cast<uint64_t>( cur_DTM_header.ROC_ID                & 0xFFFF) << 48)
                     + ( static_cast<uint64_t>( (msContent_shifted[local_offset + 0] ) & 0xFF) << 40)
                     + ( static_cast<uint64_t>( (msContent_shifted[local_offset + 1] ) & 0xFF) << 32)
                     + ( static_cast<uint64_t>( (msContent_shifted[local_offset + 2] ) & 0xFF) << 24)
                     + ( static_cast<uint64_t>( (msContent_shifted[local_offset + 3] ) & 0xFF) << 16)
                     + ( static_cast<uint64_t>( (msContent_shifted[local_offset + 4] ) & 0xFF) <<  8)
                     + ( static_cast<uint64_t>( (msContent_shifted[local_offset + 5] ) & 0xFF));
               mess.setData( dataContent );
               mess.printDataLog();

               local_offset += 6; // next message
            } // while (local_offset < packageSize)
         } // if( 0 == fiMode || kTRUE == fbVerbose )
         switch(fiMode)
         {
            case 0:
            {
               // debug mode, nothing to do here (printout already done)
               break;
            } // case 0
            case 1:
            {
               // Monitor mode, fill histograms
               while (local_offset < packageSize)
               {
                  get4v1x::Message mess;
                  uint64_t dataContent =
                          ( static_cast<uint64_t>( cur_DTM_header.ROC_ID                & 0xFFFF) << 48)
                        + ( static_cast<uint64_t>( (msContent_shifted[local_offset + 0] ) & 0xFF) << 40)
                        + ( static_cast<uint64_t>( (msContent_shifted[local_offset + 1] ) & 0xFF) << 32)
                        + ( static_cast<uint64_t>( (msContent_shifted[local_offset + 2] ) & 0xFF) << 24)
                        + ( static_cast<uint64_t>( (msContent_shifted[local_offset + 3] ) & 0xFF) << 16)
                        + ( static_cast<uint64_t>( (msContent_shifted[local_offset + 4] ) & 0xFF) <<  8)
                        + ( static_cast<uint64_t>( (msContent_shifted[local_offset + 5] ) & 0xFF));
                  mess.setData( dataContent );

                  fhMessageTypePerRoc->Fill( cur_DTM_header.ROC_ID, mess.getMessageType() );

                  switch( mess.getMessageType() )
                  {
                     case get4v1x::MSG_HIT:
                        // This is NXYTER in a GET4 unpacker => ignore
                        break;
                     case get4v1x::MSG_EPOCH:
                        this->MonitorMessage_epoch(mess, msDescriptor.eq_id);
                        break;
                     case get4v1x::MSG_SYNC:
                        this->MonitorMessage_sync(mess, msDescriptor.eq_id);
                        break;
                     case get4v1x::MSG_AUX:
                        this->MonitorMessage_aux(mess, msDescriptor.eq_id);
                        break;
                     case get4v1x::MSG_EPOCH2:
                        this->MonitorMessage_epoch2(mess, msDescriptor.eq_id);
                        break;
                     case get4v1x::MSG_GET4:
                        this->MonitorMessage_get4(mess, msDescriptor.eq_id);
                        break;
                     case get4v1x::MSG_SYS:
                        this->MonitorMessage_sys(mess, msDescriptor.eq_id);
                        break;
                     default:
                        break;
                  } // switch( mess.getMessageType() )
                  local_offset += 6; // next message
               } // while (local_offset < packageSize)
               break;
            } // case 1
            case 2:
            {
               // Normal mode, unpack data and fill TClonesArray of CbmRawMessage
               while (local_offset < packageSize)
               {
                  get4v1x::Message mess;
                  uint64_t dataContent =
                          ( static_cast<uint64_t>( cur_DTM_header.ROC_ID                & 0xFFFF) << 48)
                        + ( static_cast<uint64_t>( (msContent_shifted[local_offset + 0] ) & 0xFF) << 40)
                        + ( static_cast<uint64_t>( (msContent_shifted[local_offset + 1] ) & 0xFF) << 32)
                        + ( static_cast<uint64_t>( (msContent_shifted[local_offset + 2] ) & 0xFF) << 24)
                        + ( static_cast<uint64_t>( (msContent_shifted[local_offset + 3] ) & 0xFF) << 16)
                        + ( static_cast<uint64_t>( (msContent_shifted[local_offset + 4] ) & 0xFF) <<  8)
                        + ( static_cast<uint64_t>( (msContent_shifted[local_offset + 5] ) & 0xFF));
                  mess.setData( dataContent );

                  fhMessageTypePerRoc->Fill( cur_DTM_header.ROC_ID, mess.getMessageType() );
                  switch( mess.getMessageType() )
                  {
                     case get4v1x::MSG_HIT:
                        // This is NXYTER in a GET4 unpacker => ignore
                        break;
                     case get4v1x::MSG_EPOCH:
                        this->ProcessMessage_epoch(mess, msDescriptor.eq_id);
                        break;
                     case get4v1x::MSG_SYNC:
                        this->ProcessMessage_sync(mess, msDescriptor.eq_id);
                        break;
                     case get4v1x::MSG_AUX:
                        this->ProcessMessage_aux(mess, msDescriptor.eq_id);
                        break;
                     case get4v1x::MSG_EPOCH2:
                        this->ProcessMessage_epoch2(mess, msDescriptor.eq_id);
                        break;
                     case get4v1x::MSG_GET4:
                        this->ProcessMessage_get4(mess, msDescriptor.eq_id);
                        break;
                     case get4v1x::MSG_SYS:
                        this->ProcessMessage_sys(mess, msDescriptor.eq_id);
                        break;
                     default:
                        break;
                  } // switch( mess.getMessageType() )

                  local_offset += 6; // next message
               } // while (local_offset < packageSize)
               break;
            } // case 2
            default:
               break;
         } // switch(fiMode)

         offset += packageSize;

         // shift some more bytes to fit the CbmNet package size
         if ((padding = offset % 8) > 0)
         {
            offset += (8-padding);
         } // if ((padding = offset % 8) > 0)

      } // while (offset < msDescriptor.size)

   } // for (size_t m = 0; m < ts.num_microslices(component); ++m)

  return kTRUE;
}

void CbmTSUnpackGet4v1x::Reset()
{
//  fGet4Raw->Clear();
//  fGet4RawSync->Clear();
}

void CbmTSUnpackGet4v1x::Finish()
{
   WriteMonitorHistograms();
   DeleteMonitorHistograms();
}

/*
void CbmTSUnpackGet4v1x::Register()
{
}
*/
/*
void CbmTSUnpackGet4v1x::Print6bytesMessage(const uint8_t* msContent_shifted)
{
   if( kTRUE == fbVerbose )
	   printf("0x%02X%02X%02X%02X%02X%02X :\t",
				msContent_shifted[0], msContent_shifted[1],
				msContent_shifted[2], msContent_shifted[3],
				msContent_shifted[4], msContent_shifted[5]);
}
*/

//************** Monitor functions *************/


void CbmTSUnpackGet4v1x::InitMonitorHistograms()
{
   fhMessageTypePerRoc = new TH2I("hMessageTypePerRoc",
         "Nb of message for each type per ROC; ROC #; Type",
         fuNbRocs, -0.5, fuNbRocs -0.5,
         16, -0.5, 15.5);
   fhMessageTypePerRoc->GetYaxis()->SetBinLabel(1 + get4v1x::MSG_NOP,      "NOP");
   fhMessageTypePerRoc->GetYaxis()->SetBinLabel(1 + get4v1x::MSG_HIT,      "HIT");
   fhMessageTypePerRoc->GetYaxis()->SetBinLabel(1 + get4v1x::MSG_EPOCH,    "EPOCH");
   fhMessageTypePerRoc->GetYaxis()->SetBinLabel(1 + get4v1x::MSG_SYNC,     "SYNC");
   fhMessageTypePerRoc->GetYaxis()->SetBinLabel(1 + get4v1x::MSG_AUX,      "AUX");
   fhMessageTypePerRoc->GetYaxis()->SetBinLabel(1 + get4v1x::MSG_EPOCH2,   "EPOCH2");
   fhMessageTypePerRoc->GetYaxis()->SetBinLabel(1 + get4v1x::MSG_GET4,     "GET4");
   fhMessageTypePerRoc->GetYaxis()->SetBinLabel(1 + get4v1x::MSG_SYS,      "SYS");
   fhMessageTypePerRoc->GetYaxis()->SetBinLabel(1 + get4v1x::MSG_GET4_SLC, "MSG_GET4_SLC");
   fhMessageTypePerRoc->GetYaxis()->SetBinLabel(1 + get4v1x::MSG_GET4_32B, "MSG_GET4_32B");
   fhMessageTypePerRoc->GetYaxis()->SetBinLabel(1 + 15, "GET4 Hack 32B");

   fhRocSyncTypePerRoc = new TH2I("hRocSyncTypePerRoc",
         "Nb of message for each SYNC type per ROC; ROC #; SYNC Type",
         fuNbRocs, -0.5, fuNbRocs -0.5,
         get4v1x::kuMaxSync, -0.5, get4v1x::kuMaxSync - 0.5);

   fhRocAuxTypePerRoc = new TH2I("hRocAuxTypePerRoc",
         "Nb of message for each AUX type per ROC; ROC #; AUX Type",
         fuNbRocs, -0.5, fuNbRocs -0.5,
         get4v1x::kuMaxAux, -0.5, get4v1x::kuMaxAux - 0.5);

   fhSysMessTypePerRoc = new TH2I("hSysMessTypePerRoc",
         "Nb of system message for each type per ROC; ROC #; System Type",
         fuNbRocs, -0.5, fuNbRocs -0.5,
         16, -0.5, 15.5);
   fhSysMessTypePerRoc->GetYaxis()->SetBinLabel(1 + get4v1x::SYSMSG_DAQ_START,       "DAQ START");
   fhSysMessTypePerRoc->GetYaxis()->SetBinLabel(1 + get4v1x::SYSMSG_DAQ_FINISH,      "DAQ FINISH");
   fhSysMessTypePerRoc->GetYaxis()->SetBinLabel(1 + get4v1x::SYSMSG_NX_PARITY,       "NX PARITY");
   fhSysMessTypePerRoc->GetYaxis()->SetBinLabel(1 + get4v1x::SYSMSG_SYNC_PARITY,     "SYNC PARITY");
   fhSysMessTypePerRoc->GetYaxis()->SetBinLabel(1 + get4v1x::SYSMSG_DAQ_RESUME,      "DAQ RESUME");
   fhSysMessTypePerRoc->GetYaxis()->SetBinLabel(1 + get4v1x::SYSMSG_FIFO_RESET,      "FIFO RESET");
   fhSysMessTypePerRoc->GetYaxis()->SetBinLabel(1 + get4v1x::SYSMSG_USER,            "USER");
   fhSysMessTypePerRoc->GetYaxis()->SetBinLabel(1 + get4v1x::SYSMSG_PCTIME,          "PCTIME");
   fhSysMessTypePerRoc->GetYaxis()->SetBinLabel(1 + get4v1x::SYSMSG_ADC,             "ADC");
   fhSysMessTypePerRoc->GetYaxis()->SetBinLabel(1 + get4v1x::SYSMSG_PACKETLOST,      "PACKET LOST");
   fhSysMessTypePerRoc->GetYaxis()->SetBinLabel(1 + get4v1x::SYSMSG_GET4_EVENT,      "GET4 ERROR");
   fhSysMessTypePerRoc->GetYaxis()->SetBinLabel(1 + get4v1x::SYSMSG_CLOSYSYNC_ERROR, "CLOSYSYNC ERROR");
   fhSysMessTypePerRoc->GetYaxis()->SetBinLabel(1 + get4v1x::SYSMSG_TS156_SYNC,      "TS156 SYNC");
   fhSysMessTypePerRoc->GetYaxis()->SetBinLabel(1 + 15, "GET4 Hack 32B");

   fhGet4EpochFlags    = new TH2I("hGet4EpochFlags",
         "Number of epochs with corresponding flag set per GET4; GET4 #;",
         fuNbGet4, -0.5, fuNbGet4 -0.5,
         2, -0.5, 1.5);

   fhGet4EpochSyncDist = new TH2I("hGet4EpochSyncDist",
         "Distance between epochs with SYNC flag for each GET4; SYNC distance [epochs]; Epochs",
         fuNbGet4, -0.5, fuNbGet4 -0.5,
         2*get4v1x::kuSyncCycleSzGet4, 0.5, 2*get4v1x::kuSyncCycleSzGet4 -0.5);

   fhGet4ChanDataCount = new TH1I("hGet4ChanDataCount",
         "Data Messages per GET4 channel; GET4 channel #; Data Count",
         fuNbGet4*get4v1x::kuChanPerGet4, -0.5, fuNbGet4*get4v1x::kuChanPerGet4 -0.5);

   fhGet4ChanDllStatus = new TH2I("hGet4ChanDllStatus",
         "DLL flag status per GET4 channel; GET4 channel #; DLL Flag value Count",
         fuNbGet4*get4v1x::kuChanPerGet4 , -0.5, fuNbGet4*get4v1x::kuChanPerGet4 -0.5,
         2, -0.5, 1.5);

   fhGet4ChanTotMap    = new TH2I("hGet4ChanTotMap",
         "TOT values per GET4 channel; GET4 channel #; TOT value [ns]",
         fuNbGet4*get4v1x::kuChanPerGet4, -0.5, fuNbGet4*get4v1x::kuChanPerGet4 -0.5,
         50, 0.0, 50.0);

   fhGet4ChanErrors    = new TH2I("hGet4ChanErrors",
         "Error messages per GET4 channel; GET4 channel #; Error",
         fuNbGet4*get4v1x::kuChanPerGet4*2, -0.5, fuNbGet4*get4v1x::kuChanPerGet4 -0.5,
         32, -0.5, 31.5);
   fhGet4ChanErrors->GetYaxis()->SetBinLabel(1,  "0x00: Readout Init    ");
   fhGet4ChanErrors->GetYaxis()->SetBinLabel(2,  "0x01: Sync            ");
   fhGet4ChanErrors->GetYaxis()->SetBinLabel(3,  "0x02: Epoch count sync");
   fhGet4ChanErrors->GetYaxis()->SetBinLabel(4,  "0x03: Epoch           ");
   fhGet4ChanErrors->GetYaxis()->SetBinLabel(5,  "0x04: FIFO Write      ");
   fhGet4ChanErrors->GetYaxis()->SetBinLabel(6,  "0x05: Lost event      ");
   fhGet4ChanErrors->GetYaxis()->SetBinLabel(7,  "0x06: Channel state   ");
   fhGet4ChanErrors->GetYaxis()->SetBinLabel(8,  "0x07: Token Ring state");
   fhGet4ChanErrors->GetYaxis()->SetBinLabel(9,  "0x08: Token           ");
   fhGet4ChanErrors->GetYaxis()->SetBinLabel(10, "0x09: Error Readout   ");
   fhGet4ChanErrors->GetYaxis()->SetBinLabel(11, "0x0a: SPI             ");
   fhGet4ChanErrors->GetYaxis()->SetBinLabel(12, "0x0b: DLL Lock error  "); // <- From GET4 v1.2
   fhGet4ChanErrors->GetYaxis()->SetBinLabel(13, "0x0c: DLL Reset invoc."); // <- From GET4 v1.2
   fhGet4ChanErrors->GetYaxis()->SetBinLabel(14, "0x11: Overwrite       ");
   fhGet4ChanErrors->GetYaxis()->SetBinLabel(15, "0x12: ToT out of range");
   fhGet4ChanErrors->GetYaxis()->SetBinLabel(16, "0x13: Event Discarded ");
   fhGet4ChanErrors->GetYaxis()->SetBinLabel(17, "0x7f: Unknown         ");
   fhGet4ChanErrors->GetYaxis()->SetBinLabel(18, "Corrupt error or unsupported yet");

   fhGet4ChanSlowContM = new TH2I("hGet4ChanSlowContM",
         "Slow control messages per GET4 channel; GET4 channel #; Type",
         fuNbGet4*get4v1x::kuChanPerGet4*2, -0.5, fuNbGet4*get4v1x::kuChanPerGet4 -0.5,
         4, -0.5, 3.5);
   fhGet4ChanSlowContM->GetYaxis()->SetBinLabel(1,  "0: Scaler event            ");
   fhGet4ChanSlowContM->GetYaxis()->SetBinLabel(2,  "1: Dead time counter event ");
   fhGet4ChanSlowContM->GetYaxis()->SetBinLabel(3,  "2: SPI receiver data        ");
   fhGet4ChanSlowContM->GetYaxis()->SetBinLabel(4,  "3: Start message/Hamming Er.");


   if( kTRUE == fbPulserMode )
   {
      // Full FMC test
      UInt_t uHistoFmcIdx = 0;
      for( UInt_t uChanFmcA = 0; uChanFmcA < kuNbChanFmc; uChanFmcA++)
         for( UInt_t uChanFmcB = uChanFmcA + 1; uChanFmcB < kuNbChanFmc; uChanFmcB++)
         {
            fhTimeResFMC[uHistoFmcIdx] = new TH1I(
                  Form("hTimeResFMC_%03u_%03u", uChanFmcA, uChanFmcB),
                  Form("Time difference for channels %03u an %03u in chosen FMC; DeltaT [ps]; Counts",
                        uChanFmcA, uChanFmcB),
                  1000, -12500, 12500);
            uHistoFmcIdx++;
         } // for any unique pair of channel in chosen FMC
      fhTimeResAllFMC = new TH2D( "hTimeResAllFMC",
            "Time resolution for any channels pair in chosen FMC; Ch A; Ch B",
            kuNbChanTest, -0.5, kuNbChanTest - 0.5,
            kuNbChanTest, -0.5, kuNbChanTest - 0.5);

      // Chosen channels test
      UInt_t uHistoCombiIdx = 0;
      for( UInt_t uChanA = 0; uChanA < kuNbChanTest-1; uChanA++)
      {
         fhTimeResPairs[uChanA]  = new TH1I(
               Form("hTimeResPairs_%03u_%03u", fiPulserChan[uChanA], fiPulserChan[uChanA+1]),
               Form("Time difference for selected channels %03u an %03u; DeltaT [ps]; Counts",
                     fiPulserChan[uChanA], fiPulserChan[uChanA+1]),
                     1000, -12500, 12500);
         for( UInt_t uChanB = uChanA+1; uChanB < kuNbChanComb; uChanB++)
         {
            fhTimeResCombi[uHistoCombiIdx]  = new TH1I(
               Form("hTimeResCombi_%03u_%03u", fiPulserChan[uChanA], fiPulserChan[uChanB]),
               Form("Time difference for selected channels %03u an %03u; DeltaT [ps]; Counts",
                     fiPulserChan[uChanA], fiPulserChan[uChanB]),
               1000, -12500, 12500);
            uHistoCombiIdx++;
         } // for( UInt_t uChanB = uChanA+1; uChanB < kuNbChanComb; uChanB++)
      } // for( UInt_t uChanA = 0; uChanA < kuNbChanTest; uChanA++)

      // Prepare the vector storing the hit data for time diff calculation
      fvuLastHitEp.resize( fuNbGet4 * get4v1x::kuChanPerGet4);
      fvmLastHit.resize(   fuNbGet4 * get4v1x::kuChanPerGet4);

      // Now clear the hits
      for( UInt_t uChan = 0; uChan < fuNbGet4 * get4v1x::kuChanPerGet4; uChan++)
      {
         fvuLastHitEp[uChan] = 0;
         fvmLastHit[uChan].reset();
      } // for( UInt_t uChan = 0; uChan < fuNbGet4 * get4v1x::kuChanPerGet4; uChan++)
   } // if( kTRUE == fbPulserMode )
}
void CbmTSUnpackGet4v1x::FillMonitorHistograms()
{
}
void CbmTSUnpackGet4v1x::WriteMonitorHistograms()
{
   TDirectory * oldir = gDirectory;
   TFile *fHist;
   fHist = new TFile("./get4Monitor.hst.root","RECREATE");

   fHist->cd();

   if( kTRUE == fbPulserMode )
   {
      // First make a gauss fit to obtain the time resolution data
      // for all FMC channels pairs
      UInt_t uHistoFmcIdx = 0;
      TF1 *fitFunc[kuNbChanFmc*(kuNbChanFmc-1)/2];
      for( UInt_t uChanFmcA = 0; uChanFmcA < kuNbChanFmc; uChanFmcA++)
         for( UInt_t uChanFmcB = uChanFmcA + 1; uChanFmcB < kuNbChanFmc; uChanFmcB++)
         {
            Double_t dRes = 0.0;

            // No need to fit if not data in histo
            if( 0 == fhTimeResFMC[uHistoFmcIdx]->Integral() )
               continue;

            fitFunc[uHistoFmcIdx] = new TF1( Form("f_%02d_%02d",uChanFmcA,uChanFmcB), "gaus",
                  fhTimeResFMC[uHistoFmcIdx]->GetMean() - 5*fhTimeResFMC[uHistoFmcIdx]->GetRMS() ,
                  fhTimeResFMC[uHistoFmcIdx]->GetMean() + 5*fhTimeResFMC[uHistoFmcIdx]->GetRMS());

            fhTimeResFMC[uHistoFmcIdx]->Fit( Form("f_%02d_%02d",uChanFmcA,uChanFmcB), "QR");

            dRes = fitFunc[uHistoFmcIdx]->GetParameter(2);

            // If needed uncomment for debugging
            //(WARNING: this adds 1024 histos to the file!)
//            fhTimeResFMC[uHistoFmcIdx]->Write();

            delete fitFunc[uHistoFmcIdx];

            fhTimeResAllFMC->Fill(uChanFmcA, uChanFmcB, dRes);

            uHistoFmcIdx++;
         } // for any unique pair of channel in chosen FMC

      // Then write the FMC summary histo
      fhTimeResAllFMC->Write();

      // Write the histos for the test on chosen channel pairs
      UInt_t uHistoCombiIdx = 0;
      for( UInt_t uChanA = 0; uChanA < kuNbChanTest-1; uChanA++)
      {
         fhTimeResPairs[uChanA]->Write();
         for( UInt_t uChanB = uChanA+1; uChanB < kuNbChanComb; uChanB++)
         {
            fhTimeResCombi[uHistoCombiIdx]->Write();
            uHistoCombiIdx++;
         } // for( UInt_t uChanB = uChanA+1; uChanB < kuNbChanComb; uChanB++)
      } // for( UInt_t uChanA = 0; uChanA < kuNbChanTest; uChanA++)
   } // if( kTRUE == fbPulserMode )

   gDirectory->cd( oldir->GetPath() );
   fHist->Close();
}
void CbmTSUnpackGet4v1x::DeleteMonitorHistograms()
{

}
void CbmTSUnpackGet4v1x::MonitorMessage_epoch(  get4v1x::Message mess, uint16_t EqID)
{
   // ROC epoch message: 250 MHz clock for now
   // TODO: check compatibility when ROC clock changed to 200 MHz
   uint8_t  cRocId    = mess.getRocNumber();
}
void CbmTSUnpackGet4v1x::MonitorMessage_sync(   get4v1x::Message mess, uint16_t EqID)
{
   // ROC sync message: TS in 250 MHz clock for now
   // TODO: check compatibility when ROC clock changed to 200 MHz
   uint8_t  cRocId    = mess.getRocNumber();
   fhRocSyncTypePerRoc->Fill( cRocId, mess.getSyncChNum() );
}
void CbmTSUnpackGet4v1x::MonitorMessage_aux(    get4v1x::Message mess, uint16_t EqID)
{
   // ROC aux message: TS in 250 MHz clock for now
   // TODO: check compatibility when ROC clock changed to 200 MHz
   uint8_t  cRocId    = mess.getRocNumber();
   fhRocAuxTypePerRoc->Fill( cRocId, mess.getAuxChNum() );
}
void CbmTSUnpackGet4v1x::MonitorMessage_epoch2( get4v1x::Message mess, uint16_t EqID)
{
   // GET4 v1.x epoch message (24b only for now)
   // TODO: check compatibility when 32b format without hack ready
}
void CbmTSUnpackGet4v1x::MonitorMessage_get4(   get4v1x::Message mess, uint16_t EqID)
{
   // GET4 v1.x data message (24b only for now)
   // TODO: check compatibility when 32b format without hack ready
}
void CbmTSUnpackGet4v1x::MonitorMessage_sys(    get4v1x::Message mess, uint16_t EqID)
{
   uint8_t  cRocId    = mess.getRocNumber();
   fhSysMessTypePerRoc->Fill( cRocId, mess.getSysMesType() );
   switch( mess.getSysMesType() )
   {
      case get4v1x::SYSMSG_DAQ_START:
      {
         break;
      } // case get4v1x::SYSMSG_DAQ_START
      case get4v1x::SYSMSG_DAQ_FINISH:
      {
         break;
      } // case get4v1x::SYSMSG_DAQ_FINISH
      case get4v1x::SYSMSG_NX_PARITY:
      case get4v1x::SYSMSG_ADC:
      case get4v1x::SYSMSG_PCTIME:
      {
         // NXYTER related => don't care in this unpacker
         break;
      } // case get4v1x::SYSMSG_NX_PARITY
      case get4v1x::SYSMSG_SYNC_PARITY:
      {
         break;
      } // case get4v1x::SYSMSG_SYNC_PARITY
      case get4v1x::SYSMSG_DAQ_RESUME:
      {
         break;
      } // case get4v1x::SYSMSG_DAQ_RESUME
      case get4v1x::SYSMSG_FIFO_RESET:
      {
         break;
      } // case get4v1x::SYSMSG_FIFO_RESET
      case get4v1x::SYSMSG_USER:
      {
         break;
      } // case get4v1x::SYSMSG_USER
      case get4v1x::SYSMSG_PACKETLOST:
      {
         break;
      } // case get4v1x::SYSMSG_PACKETLOST
      case get4v1x::SYSMSG_GET4_EVENT:
      {
         // GET4 v1.x error message (24b only for now)
         // TODO: check compatibility when 32b format without hack ready
         break;
      } // case get4v1x::SYSMSG_GET4_EVENT
      case get4v1x::SYSMSG_CLOSYSYNC_ERROR:
      {
         break;
      } // case get4v1x::SYSMSG_CLOSYSYNC_ERROR
      case get4v1x::SYSMSG_TS156_SYNC:
      {
         break;
      } // case get4v1x::SYSMSG_TS156_SYNC
      case get4v1x::SYSMSG_GET4V1_32BIT_0:
      case get4v1x::SYSMSG_GET4V1_32BIT_1:
      case get4v1x::SYSMSG_GET4V1_32BIT_2:
      case get4v1x::SYSMSG_GET4V1_32BIT_3:
      case get4v1x::SYSMSG_GET4V1_32BIT_4:
      case get4v1x::SYSMSG_GET4V1_32BIT_5:
      case get4v1x::SYSMSG_GET4V1_32BIT_6:
      case get4v1x::SYSMSG_GET4V1_32BIT_7:
      case get4v1x::SYSMSG_GET4V1_32BIT_8:
      case get4v1x::SYSMSG_GET4V1_32BIT_9:
      case get4v1x::SYSMSG_GET4V1_32BIT_10:
      case get4v1x::SYSMSG_GET4V1_32BIT_11:
      case get4v1x::SYSMSG_GET4V1_32BIT_12:
      case get4v1x::SYSMSG_GET4V1_32BIT_13:
      case get4v1x::SYSMSG_GET4V1_32BIT_14:
      case get4v1x::SYSMSG_GET4V1_32BIT_15:
      {
         // GET4 v1.x 32b raw message using hack
         fhMessageTypePerRoc->Fill( cRocId, 15 );
         MonitorMessage_Get4v1( mess, EqID);
         break;
      } // case get4v1x::SYSMSG_TS156_SYNC
      default:
      {
         if( kTRUE == fbVerbose )
            LOG(WARNING)<<"CbmTSUnpackGet4v1x::MonitorMessage_sys => Unknown sysMess type ignored: "
                        <<mess.getSysMesType()<<FairLogger::endl;
         break;
      } // default:
   } // switch( mess.getSysMesType() )
}
void CbmTSUnpackGet4v1x::MonitorMessage_Get4v1( get4v1x::Message mess, uint16_t EqID)
{
   // GET4 v1.x 32b raw message using hack
   uint8_t  cRocId    = mess.getRocNumber();
   uint8_t  cChipId   = mess.getGet4V10R32ChipId();
   uint32_t uChipFullId = cChipId + get4v1x::kuMaxGet4PerRoc*cRocId;
   uint8_t  cMessType = mess.getGet4V10R32MessageType();

   if( fuNbGet4 <= uChipFullId )
   {
      if( kTRUE == fbVerbose )
         LOG(INFO)<<"CbmTSUnpackGet4v1x::MonitorMessage_Get4v1 => Ignored message with ChipId above limit!"
                  <<" ChipId: "<<cChipId
                  <<" RocId: " <<cRocId
                  <<" Limit: " <<fuNbGet4<<FairLogger::endl;
      return;
   } // if( fuNbGet4 <= cChipId + kuMaxGet4PerRoc*mess.getRocNumber() )

   switch( cMessType )
   {
      case get4v1x::GET4_32B_EPOCH: // => Epoch message
      {
         fvuCurrEpoch2[uChipFullId] = mess.getGet4V10R32EpochNumber();

         // Fill Pulser test histos if needed
         // Accepted pairs are when both messages are defined and they are at most
         // 1 epoch apart => possibility of double use is the pulse happens on top of
         // an epoch and more than once every 3 epochs. For example:
         // HHHHEHHHH.......E......HHHHEHHHH leads to
         // (HHHHHHHH)             (HHHHHHHH) and
         //     (HHHH              HHHH)
         if( kTRUE == fbPulserMode )
         {
            // Fill the time difference for all channels pairs in
            // the chosen FMC
            UInt_t uHistoFmcIdx = 0;
            for( UInt_t uChanFmcA = 0; uChanFmcA < kuNbChanFmc; uChanFmcA++)
               for( UInt_t uChanFmcB = uChanFmcA + 1; uChanFmcB < kuNbChanFmc; uChanFmcB++)
                  if( 0xF0 <= fvmLastHit[ fiPulserFmc * kuNbChanFmc+ uChanFmcA].getMessageType() &&
                      0xF0 <= fvmLastHit[ fiPulserFmc * kuNbChanFmc+ uChanFmcB].getMessageType() &&
                      (   fvuLastHitEp[ fiPulserFmc * kuNbChanFmc+ uChanFmcA ]
                        < fvuLastHitEp[ fiPulserFmc * kuNbChanFmc+ uChanFmcB ] + 2 ) &&
                      (   fvuLastHitEp[ fiPulserFmc * kuNbChanFmc+ uChanFmcA ] + 2
                        > fvuLastHitEp[ fiPulserFmc * kuNbChanFmc+ uChanFmcB ] ) )
               {
                  fhTimeResFMC[uHistoFmcIdx]->Fill(
                       ( fvuLastHitEp[ fiPulserFmc * kuNbChanFmc+ uChanFmcA]
                        -fvuLastHitEp[ fiPulserFmc * kuNbChanFmc+ uChanFmcB])*get4v1x::kdEpochInPs
                       + ( fvmLastHit[ fiPulserFmc * kuNbChanFmc+ uChanFmcA].getGet4V10R32HitTimeBin()
                          -fvmLastHit[ fiPulserFmc * kuNbChanFmc+ uChanFmcB].getGet4V10R32HitTimeBin()
                          )*get4v1x::kdBinSize );
                  uHistoFmcIdx++;
               } // for any unique pair of channel in chosen FMC with data

            // Fill the time difference for the chosen channel pairs
            UInt_t uHistoCombiIdx = 0;
            for( UInt_t uChanA = 0; uChanA < kuNbChanTest-1; uChanA++)
            {
               if( 0xF0 <= fvmLastHit[ fiPulserChan[uChanA]   ].getMessageType() &&
                   0xF0 <= fvmLastHit[ fiPulserChan[uChanA+1] ].getMessageType() &&
                   fvuLastHitEp[ fiPulserChan[uChanA]   ]     < fvuLastHitEp[ fiPulserChan[uChanA+1] ] + 2 &&
                   fvuLastHitEp[ fiPulserChan[uChanA]   ] + 2 > fvuLastHitEp[ fiPulserChan[uChanA+1] ] )
                  fhTimeResPairs[uChanA]->Fill(
                        ( fvuLastHitEp[ fiPulserChan[uChanA] ]
                         -fvuLastHitEp[ fiPulserChan[uChanA+1] ])*get4v1x::kdEpochInPs
                        + ( fvmLastHit[ fiPulserChan[uChanA] ].getGet4V10R32HitTimeBin()
                           -fvmLastHit[ fiPulserChan[uChanA+1]].getGet4V10R32HitTimeBin()
                           )*get4v1x::kdBinSize );

               for( UInt_t uChanB = uChanA+1; uChanB < kuNbChanComb; uChanB++)
                  if( 0xF0 <= fvmLastHit[ fiPulserChan[uChanA] ].getMessageType() &&
                      0xF0 <= fvmLastHit[ fiPulserChan[uChanB] ].getMessageType() &&
                      fvuLastHitEp[ fiPulserChan[uChanA] ]     < fvuLastHitEp[ fiPulserChan[uChanB] ] + 2 &&
                      fvuLastHitEp[ fiPulserChan[uChanA] ] + 2 > fvuLastHitEp[ fiPulserChan[uChanB] ] )
               {
                  fhTimeResCombi[uHistoCombiIdx]->Fill(
                        ( fvuLastHitEp[ fiPulserChan[uChanA] ]
                         -fvuLastHitEp[ fiPulserChan[uChanB] ])*get4v1x::kdEpochInPs
                        + ( fvmLastHit[ fiPulserChan[uChanA] ].getGet4V10R32HitTimeBin()
                           -fvmLastHit[ fiPulserChan[uChanB]].getGet4V10R32HitTimeBin()
                           )*get4v1x::kdBinSize );
                  uHistoCombiIdx++;
               } // for( UInt_t uChanB = uChanA+1; uChanB < kuNbChanComb; uChanB++)
            } // for( UInt_t uChanA = 0; uChanA < kuNbChanTest; uChanA++)

         } // if( kTRUE == fbPulserMode )
         break;
      } // case get4v1x::GET4_32B_EPOCH
      case get4v1x::GET4_32B_SLCM:  // => Slow control
      {
         Double_t dFullChId =
               get4v1x::kuChanPerGet4*2*( uChipFullId )
               + mess.getGet4V10R32SlChan()
               + 0.5*mess.getGet4V10R32SlEdge();
         fhGet4ChanSlowContM->Fill( dFullChId, mess.getGet4V10R32SlType() );
         break;
      } // case get4v1x::GET4_32B_SLCM
      case get4v1x::GET4_32B_ERROR: // => Error message
      {
         Double_t dFullChId =
               get4v1x::kuChanPerGet4*2*( uChipFullId )
               + mess.getGet4V10R32ErrorChan()
               + 0.5*mess.getGet4V10R32ErrorEdge();
         switch( mess.getGet4V10R32ErrorData() )
         {
            case get4v1x::GET4_V1X_ERR_READ_INIT:
               fhGet4ChanErrors->Fill( dFullChId, 0);
               break;
            case get4v1x::GET4_V1X_ERR_SYNC:
               fhGet4ChanErrors->Fill( dFullChId, 1);
               break;
            case get4v1x::GET4_V1X_ERR_EP_CNT_SYNC:
               fhGet4ChanErrors->Fill( dFullChId, 2);
               break;
            case get4v1x::GET4_V1X_ERR_EP:
               fhGet4ChanErrors->Fill( dFullChId, 3);
               break;
            case get4v1x::GET4_V1X_ERR_FIFO_WRITE:
               fhGet4ChanErrors->Fill( dFullChId, 4);
               break;
            case get4v1x::GET4_V1X_ERR_LOST_EVT:
               fhGet4ChanErrors->Fill( dFullChId, 5);
               break;
            case get4v1x::GET4_V1X_ERR_CHAN_STATE:
               fhGet4ChanErrors->Fill( dFullChId, 6);
               break;
            case get4v1x::GET4_V1X_ERR_TOK_RING_ST:
               fhGet4ChanErrors->Fill( dFullChId, 7);
               break;
            case get4v1x::GET4_V1X_ERR_TOKEN:
               fhGet4ChanErrors->Fill( dFullChId, 8);
               break;
            case get4v1x::GET4_V1X_ERR_READOUT_ERR:
               fhGet4ChanErrors->Fill( dFullChId, 9);
               break;
            case get4v1x::GET4_V1X_ERR_SPI:
               fhGet4ChanErrors->Fill( dFullChId, 10);
               break;
            case get4v1x::GET4_V1X_ERR_DLL_LOCK:
               fhGet4ChanErrors->Fill( dFullChId, 11);
               break;
            case get4v1x::GET4_V1X_ERR_DLL_RESET:
               fhGet4ChanErrors->Fill( dFullChId, 12);
               break;
            case get4v1x::GET4_V1X_ERR_TOT_OVERWRT:
               fhGet4ChanErrors->Fill( dFullChId, 13);
               break;
            case get4v1x::GET4_V1X_ERR_TOT_RANGE:
               fhGet4ChanErrors->Fill( dFullChId, 14);
               break;
            case get4v1x::GET4_V1X_ERR_EVT_DISCARD:
               fhGet4ChanErrors->Fill( dFullChId, 15);
               break;
            case get4v1x::GET4_V1X_ERR_UNKNOWN:
               fhGet4ChanErrors->Fill( dFullChId, 16);
               break;
            default: // Corrupt error or not yet supported error
               fhGet4ChanErrors->Fill( dFullChId, 17);
               break;
         } // switch( mess.getGet4V10R32ErrorData() )
         break;
      } // case get4v1x::GET4_32B_ERROR
      case get4v1x::GET4_32B_DATA:  // => Hit Data
      {
         UInt_t uFullChId =
               get4v1x::kuChanPerGet4*( uChipFullId )
               + mess.getGet4V10R32HitChan();
         fhGet4ChanDataCount->Fill( uFullChId );
         fhGet4ChanDllStatus->Fill( uFullChId, mess.getGet4V10R32HitDllFlag() );
         fhGet4ChanTotMap->Fill(    uFullChId, mess.getGet4V10R32HitTot() );

         // Save the hit info in order to fill later the pulser histos
         if( kTRUE == fbPulserMode )
         {
            // Epoch of Last hit message (one per GET4 chip & channel)
            fvuLastHitEp[ uFullChId ] = fvuCurrEpoch2[uChipFullId];
            // Last hit message (one per GET4 chip & channel)
            fvmLastHit[ uFullChId ] = mess;
         } // if( kTRUE == fbPulserMode )

         break;
      } // case get4v1x::GET4_32B_DATA
   } // switch( cMessType )

}
//**********************************************/

//************ Processing functions ************/
void CbmTSUnpackGet4v1x::ProcessMessage_epoch(  get4v1x::Message mess, uint16_t EqID)
{
   // ROC epoch message: 250 MHz clock for now
   // TODO: check compatibility when ROC clock changed to 200 MHz
}
void CbmTSUnpackGet4v1x::ProcessMessage_sync(   get4v1x::Message mess, uint16_t EqID)
{
   // ROC sync message: TS in 250 MHz clock for now
   // TODO: check compatibility when ROC clock changed to 200 MHz
   uint8_t  cRocId    = mess.getRocNumber();
   fhRocSyncTypePerRoc->Fill( cRocId, mess.getSyncChNum() );
}
void CbmTSUnpackGet4v1x::ProcessMessage_aux(    get4v1x::Message mess, uint16_t EqID)
{
   // ROC aux message: TS in 250 MHz clock for now
   // TODO: check compatibility when ROC clock changed to 200 MHz
   uint8_t  cRocId    = mess.getRocNumber();
   fhRocSyncTypePerRoc->Fill( cRocId, mess.getSyncChNum() );
}
void CbmTSUnpackGet4v1x::ProcessMessage_epoch2( get4v1x::Message mess, uint16_t EqID)
{
   // GET4 v1.x epoch message (24b only for now)
   // TODO: check compatibility when 32b format without hack ready
   uint8_t  cRocId    = mess.getRocNumber();
   if( kTRUE == fbVerbose )
      LOG(INFO)<<"CbmTSUnpackGet4v1x::ProcessMessage_epoch2 => Found a GET4 24b message: ignore it"
               <<FairLogger::endl;
}
void CbmTSUnpackGet4v1x::ProcessMessage_get4(   get4v1x::Message mess, uint16_t EqID)
{
   // GET4 v1.x data message (24b only for now)
   // TODO: check compatibility when 32b format without hack ready
   uint8_t  cRocId    = mess.getRocNumber();
   if( kTRUE == fbVerbose )
      LOG(INFO)<<"CbmTSUnpackGet4v1x::ProcessMessage_get4   => Found a GET4 24b message: ignore it"
               <<FairLogger::endl;
}
void CbmTSUnpackGet4v1x::ProcessMessage_sys(    get4v1x::Message mess, uint16_t EqID)
{
   uint8_t  cRocId    = mess.getRocNumber();
   fhSysMessTypePerRoc->Fill( cRocId, mess.getSysMesType() );
   switch( mess.getSysMesType() )
   {
      case get4v1x::SYSMSG_DAQ_START:
      {
         break;
      } // case get4v1x::SYSMSG_DAQ_START
      case get4v1x::SYSMSG_DAQ_FINISH:
      {
         break;
      } // case get4v1x::SYSMSG_DAQ_FINISH
      case get4v1x::SYSMSG_NX_PARITY:
      case get4v1x::SYSMSG_ADC:
      case get4v1x::SYSMSG_PCTIME:
      {
         // NXYTER related => don't care in this unpacker
         break;
      } // case get4v1x::SYSMSG_NX_PARITY
      case get4v1x::SYSMSG_SYNC_PARITY:
      {
         break;
      } // case get4v1x::SYSMSG_SYNC_PARITY
      case get4v1x::SYSMSG_DAQ_RESUME:
      {
         break;
      } // case get4v1x::SYSMSG_DAQ_RESUME
      case get4v1x::SYSMSG_FIFO_RESET:
      {
         break;
      } // case get4v1x::SYSMSG_FIFO_RESET
      case get4v1x::SYSMSG_USER:
      {
         break;
      } // case get4v1x::SYSMSG_USER
      case get4v1x::SYSMSG_PACKETLOST:
      {
         break;
      } // case get4v1x::SYSMSG_PACKETLOST
      case get4v1x::SYSMSG_GET4_EVENT:
      {
         // GET4 v1.x error message (24b only for now)
         // TODO: check compatibility when 32b format without hack ready
         break;
      } // case get4v1x::SYSMSG_GET4_EVENT
      case get4v1x::SYSMSG_CLOSYSYNC_ERROR:
      {
         // GET4 v1.x error message (24b only for now)
         // TODO: check compatibility when 32b format without hack ready
         break;
      } // case get4v1x::SYSMSG_CLOSYSYNC_ERROR
      case get4v1x::SYSMSG_TS156_SYNC:
      {
         break;
      } // case get4v1x::SYSMSG_TS156_SYNC
      case get4v1x::SYSMSG_GET4V1_32BIT_0:
      case get4v1x::SYSMSG_GET4V1_32BIT_1:
      case get4v1x::SYSMSG_GET4V1_32BIT_2:
      case get4v1x::SYSMSG_GET4V1_32BIT_3:
      case get4v1x::SYSMSG_GET4V1_32BIT_4:
      case get4v1x::SYSMSG_GET4V1_32BIT_5:
      case get4v1x::SYSMSG_GET4V1_32BIT_6:
      case get4v1x::SYSMSG_GET4V1_32BIT_7:
      case get4v1x::SYSMSG_GET4V1_32BIT_8:
      case get4v1x::SYSMSG_GET4V1_32BIT_9:
      case get4v1x::SYSMSG_GET4V1_32BIT_10:
      case get4v1x::SYSMSG_GET4V1_32BIT_11:
      case get4v1x::SYSMSG_GET4V1_32BIT_12:
      case get4v1x::SYSMSG_GET4V1_32BIT_13:
      case get4v1x::SYSMSG_GET4V1_32BIT_14:
      case get4v1x::SYSMSG_GET4V1_32BIT_15:
      {
         // GET4 v1.x 32b raw message using hack
         fhMessageTypePerRoc->Fill( cRocId, 15 );
         ProcessMessage_Get4v1( mess, EqID);
         break;
      } // case get4v1x::SYSMSG_TS156_SYNC
      default:
      {
         if( kTRUE == fbVerbose )
            LOG(WARNING)<<"CbmTSUnpackGet4v1x::ProcessMessage_sys => Unknown sysMess type ignored: "
                        <<mess.getSysMesType()<<FairLogger::endl;
         break;
      } // default:
   } // switch( mess.getSysMesType() )
}
void CbmTSUnpackGet4v1x::ProcessMessage_Get4v1( get4v1x::Message mess, uint16_t EqID)
{
   // GET4 v1.x 32b raw message using hack
   uint8_t  cRocId    = mess.getRocNumber();
   uint8_t  cChipId   = mess.getGet4V10R32ChipId();
   uint32_t uChipFullId = cChipId + get4v1x::kuMaxGet4PerRoc*cRocId;
   uint8_t  cMessType = mess.getGet4V10R32MessageType();

   if( fuNbGet4 <= uChipFullId )
   {
      if( kTRUE == fbVerbose )
         LOG(INFO)<<"CbmTSUnpackGet4v1x::ProcessMessage_Get4v1 => Ignored message with ChipId above limit!"
                  <<" ChipId: "<<cChipId
                  <<" RocId: " <<cRocId
                  <<" Limit: " <<fuNbGet4<<FairLogger::endl;
      return;
   } // if( fuNbGet4 <= cChipId + kuMaxGet4PerRoc*mess.getRocNumber() )

   switch( cMessType )
   {
      case get4v1x::GET4_32B_EPOCH: // => Epoch message
      {
         break;
      } // case get4v1x::GET4_32B_EPOCH
      case get4v1x::GET4_32B_SLCM:  // => Slow control
      {
         Double_t dFullChId =
               get4v1x::kuChanPerGet4*2*( uChipFullId )
               + mess.getGet4V10R32SlChan()
               + 0.5*mess.getGet4V10R32SlEdge();
         fhGet4ChanSlowContM->Fill( dFullChId, mess.getGet4V10R32SlType() );
         break;
      } // case get4v1x::GET4_32B_SLCM
      case get4v1x::GET4_32B_ERROR: // => Error message
      {
         Double_t dFullChId =
               get4v1x::kuChanPerGet4*2*( uChipFullId )
               + mess.getGet4V10R32ErrorChan()
               + 0.5*mess.getGet4V10R32ErrorEdge();
         switch( mess.getGet4V10R32ErrorData() )
         {
            case get4v1x::GET4_V1X_ERR_READ_INIT:
               fhGet4ChanErrors->Fill( dFullChId, 0);
               break;
            case get4v1x::GET4_V1X_ERR_SYNC:
               fhGet4ChanErrors->Fill( dFullChId, 1);
               break;
            case get4v1x::GET4_V1X_ERR_EP_CNT_SYNC:
               fhGet4ChanErrors->Fill( dFullChId, 2);
               break;
            case get4v1x::GET4_V1X_ERR_EP:
               fhGet4ChanErrors->Fill( dFullChId, 3);
               break;
            case get4v1x::GET4_V1X_ERR_FIFO_WRITE:
               fhGet4ChanErrors->Fill( dFullChId, 4);
               break;
            case get4v1x::GET4_V1X_ERR_LOST_EVT:
               fhGet4ChanErrors->Fill( dFullChId, 5);
               break;
            case get4v1x::GET4_V1X_ERR_CHAN_STATE:
               fhGet4ChanErrors->Fill( dFullChId, 6);
               break;
            case get4v1x::GET4_V1X_ERR_TOK_RING_ST:
               fhGet4ChanErrors->Fill( dFullChId, 7);
               break;
            case get4v1x::GET4_V1X_ERR_TOKEN:
               fhGet4ChanErrors->Fill( dFullChId, 8);
               break;
            case get4v1x::GET4_V1X_ERR_READOUT_ERR:
               fhGet4ChanErrors->Fill( dFullChId, 9);
               break;
            case get4v1x::GET4_V1X_ERR_SPI:
               fhGet4ChanErrors->Fill( dFullChId, 10);
               break;
            case get4v1x::GET4_V1X_ERR_DLL_LOCK:
               fhGet4ChanErrors->Fill( dFullChId, 11);
               break;
            case get4v1x::GET4_V1X_ERR_DLL_RESET:
               fhGet4ChanErrors->Fill( dFullChId, 12);
               break;
            case get4v1x::GET4_V1X_ERR_TOT_OVERWRT:
               fhGet4ChanErrors->Fill( dFullChId, 13);
               break;
            case get4v1x::GET4_V1X_ERR_TOT_RANGE:
               fhGet4ChanErrors->Fill( dFullChId, 14);
               break;
            case get4v1x::GET4_V1X_ERR_EVT_DISCARD:
               fhGet4ChanErrors->Fill( dFullChId, 15);
               break;
            case get4v1x::GET4_V1X_ERR_UNKNOWN:
               fhGet4ChanErrors->Fill( dFullChId, 16);
               break;
            default: // Corrupt error or not yet supported error
               fhGet4ChanErrors->Fill( dFullChId, 17);
               break;
         } // switch( mess.getGet4V10R32ErrorData() )
         break;
      } // case get4v1x::GET4_32B_ERROR
      case get4v1x::GET4_32B_DATA:  // => Hit Data
      {
         UInt_t uFullChId =
               get4v1x::kuChanPerGet4*( uChipFullId )
               + mess.getGet4V10R32HitChan();
         fhGet4ChanDataCount->Fill( uFullChId );
         fhGet4ChanDllStatus->Fill( uFullChId, mess.getGet4V10R32HitDllFlag() );
         break;
      } // case get4v1x::GET4_32B_DATA
   } // switch( cMessType )
}
//**********************************************/

ClassImp(CbmTSUnpackGet4v1x)
