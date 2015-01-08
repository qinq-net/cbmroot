// -----------------------------------------------------------------------------
// -----                                                                   -----
// -----                      CbmTSUnpackGet4v1x                           -----
// -----                    Created 15.12.2014 by                          -----
// -----                        P.-A. Loizeau                              -----
// -----                  Based on CbmTSUnpackGet4v1x                       -----
// -----                                                                   -----
// -----------------------------------------------------------------------------

#include "CbmTSUnpackGet4v1x.h"

//#include "CbmNxyterRawMessage.h"
//#include "CbmNxyterRawSyncMessage.h"

#include "CbmGet4v1xHackDef.h"

#include "FairLogger.h"
#include "FairRootManager.h"

#include "TClonesArray.h"

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
   fiCurrEpoch(0)
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

  InitMonitorHistograms();

  return kTRUE;
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

         // Debug printout
         cur_DTM_header.Dump();

         uint32_t packageSize = static_cast<uint32_t>(cur_DTM_header.packet_length*2+4);

         // Loop over messages
         local_offset = 4;
         if( 0 == fiMode || kTRUE == fbVerbose )
         {
            while (local_offset < packageSize)
            {
               get4v1x::Message mess;
               mess.setData( msContent_shifted[0] );
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
                  local_offset += 6; // next message
               } // while (local_offset < packageSize)
               break;
            } // case 1
            case 2:
            {
               // Normal mode, unpack data and fill TClonesArray of CbmRawMessage
               while (local_offset < packageSize)
               {
                  /*
                  // Extract the message type to define which procedure to apply to the 6-bytes message
                  uint8_t messageType = (msContent_shifted[local_offset+5] >> 0) & 0x07;   // 3 bits

                  this->Print6bytesMessage(&msContent_shifted[local_offset]);

                  switch(messageType) {
                  case MSG_HIT:
                     // This is NXYTER => ignore
                     break;
                  case MSG_EPOCH:
                     this->ProcessMessage_epoch(&msContent_shifted[local_offset],
                                                msDescriptor.eq_id, cur_DTM_header.ROC_ID);
                     break;
                  case MSG_SYNC:
                     this->ProcessMessage_sync(&msContent_shifted[local_offset],
                                                msDescriptor.eq_id, cur_DTM_header.ROC_ID);
                     break;
                  case MSG_AUX:
                     this->ProcessMessage_aux(&msContent_shifted[local_offset],
                                               msDescriptor.eq_id, cur_DTM_header.ROC_ID);
                     break;
                  case MSG_SYS:
                     this->ProcessMessage_sys(&msContent_shifted[local_offset],
                                               msDescriptor.eq_id, cur_DTM_header.ROC_ID);
                     break;
                  }
                   */
                  local_offset += 6; // next message
               } // while (local_offset < packageSize)
               break;
            } // case 2
            default:
               break;
         }

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

}
void CbmTSUnpackGet4v1x::FillMonitorHistograms()
{

}
void CbmTSUnpackGet4v1x::WriteMonitorHistograms()
{

}
void CbmTSUnpackGet4v1x::DeleteMonitorHistograms()
{

}
//**********************************************/

/*
// Missed event (currently skipped), epoch counter value, ROC id, messageType=2
// [MMMMMMMM][EEEEEEEE][EEEEEEEE][EEEEEEEE][EEEEEEEE][--RRR010]
void CbmTSUnpackGet4v1x::ProcessMessage_epoch(const uint8_t* msContent_shifted, uint16_t EqID, uint16_t RocID)
{
   Int_t messageType = (msContent_shifted[5] >> 0) & 0x07;          // 3 bits

   fiCurrEpoch = ((msContent_shifted[4] >> 0) & 0xFF)                // 8 bits
             + (((msContent_shifted[3] >> 0) & 0xFF) << 8)          // 8 bits
             + (((msContent_shifted[2] >> 0) & 0xFF) << 16)         // 8 bits
             + (((msContent_shifted[1] >> 0) & 0xFF) << 24);        // 8 bits

   // Debug printout
   if( kTRUE == fbVerbose )
   	   printf("messType=%d\tepoch=0x%08X\n", messageType, fiCurrEpoch);

}


// Status, Data, Timestamp, syncChannel, ROC id, messageType=3
// [SSDDDDDD][DDDDDDDD][DDDDDDDD][DDTTTTTT][TTTTTTTT][CCRRR011]
void CbmTSUnpackGet4v1x::ProcessMessage_sync(const uint8_t* msContent_shifted, uint16_t EqID, uint16_t RocID)
{
   Int_t messageType, rocID, syncCh, timestamp, data, status;

   messageType = (msContent_shifted[5] >> 0) & 0x07;          // 3 bits
   rocID       = (msContent_shifted[5] >> 3) & 0x07;          // 3 bits
   syncCh      = (msContent_shifted[5] >> 6) & 0x03;          // 2 bits
   timestamp  = ((msContent_shifted[4] >> 0) & 0xFF)          // 8 bits
             + (((msContent_shifted[3] >> 0) & 0x3F) << 8);   // 6 bits
   data       = ((msContent_shifted[3] >> 6) & 0x03)          // 2 bits
             + (((msContent_shifted[2] >> 0) & 0xFF) << 2)    // 8 bits
             + (((msContent_shifted[1] >> 0) & 0xFF) << 10)   // 8 bits
             + (((msContent_shifted[0] >> 0) & 0x3F) << 18);  // 6 bits
   status      = (msContent_shifted[0] >> 6) & 0x03;          // 2 bits

   new( (*fNxyterRawSync)[fNxyterRawSync->GetEntriesFast()] )
   CbmNxyterRawSyncMessage(
         EqID,
         RocID*4 + syncCh,  //TODO check
         syncCh,
         fiCurrEpoch, //              - cur_hit_data.NxLastEpoch,  //TODO subtract here or in GetFullTime() method ?!
         timestamp,
         status,
         data);

   if( kTRUE == fbVerbose )
      printf("messType=%d\tROC=0x%01X\tsyncCh=0x%01X\tTs=0x%04X\tdata=0x%08X\tstat=0x%01X\n",
         messageType, rocID, syncCh, timestamp, data, status);

}

// Pileup flag, edge type, timestamp, channel, ROC id, messageType=4
// [--------][--------][---PETTT][TTTTTTTT][TTTCCCCC][CCRRR100]
void CbmTSUnpackGet4v1x::ProcessMessage_aux(const uint8_t* msContent_shifted, uint16_t EqID, uint16_t RocID)
{
   Int_t messageType, rocID, channel, timestamp;
   Bool_t edgeType, pileUp;

   messageType = (msContent_shifted[5] >> 0) & 0x07;          // 3 bits
   rocID       = (msContent_shifted[5] >> 3) & 0x07;          // 3 bits
   channel    = ((msContent_shifted[5] >> 6) & 0x03)          // 2 bits
             + (((msContent_shifted[4] >> 0) & 0x1F) << 2);   // 5 bits

   timestamp  = ((msContent_shifted[4] >> 5) & 0x07)          // 3 bits
             + (((msContent_shifted[3] >> 0) & 0xFF) << 3)    // 8 bits
             + (((msContent_shifted[2] >> 0) & 0x07) << 11);  // 3 bit

   edgeType    = (msContent_shifted[2] >> 3) & 0x01;          // 1 bit
   pileUp      = (msContent_shifted[2] >> 4) & 0x01;          // 1 bit

   //TODO put the unpacked data into some output array

   if( kTRUE == fbVerbose )
      printf("messType=%d\tROC=0x%01X\tCh=0x%02X\tTs=0x%04X\tedgeType=%d\tpileUp=%d\n",
         messageType, rocID, channel, timestamp, edgeType, pileUp);

}

// First four bytes are yet not unpacked, system message type, NX id, ROC id, messageType=7
// [--------][--------][--------][--------][TTTTTTTT][NNRRR111]
void CbmTSUnpackGet4v1x::ProcessMessage_sys(const uint8_t* msContent_shifted, uint16_t EqID, uint16_t RocID)
{
   Int_t messageType, rocID, nxID, sysMessType;

   messageType = (msContent_shifted[5] >> 0) & 0x07;          // 3 bits
   rocID       = (msContent_shifted[5] >> 3) & 0x07;          // 3 bits
   nxID        = (msContent_shifted[5] >> 6) & 0x03;          // 2 bits
   sysMessType = (msContent_shifted[4] >> 0) & 0xFF;          // 8 bits

   //TODO put the unpacked data into some output array

   if( kTRUE == fbVerbose )
      printf("messType=%d\tROC=0x%01X\tNX=0x%02X\tsysMessType=%02X\n",
         messageType, rocID, nxID, sysMessType);
}
*/
ClassImp(CbmTSUnpackGet4v1x)
