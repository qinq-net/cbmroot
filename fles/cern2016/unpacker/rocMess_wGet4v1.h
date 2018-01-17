#ifndef ROC_MESS_WGET4V1X_DEF_H
#define ROC_MESS_WGET4V1X_DEF_H

#include <stdint.h>
#include <iostream>

#ifndef __CINT__
namespace get4v1x {
   // Size of one clock cycle (=1 coarse bin)
   const double   kdClockCycleSize    = 6250.0; //[ps]
   const double   kdClockCycleSizeNs  = kdClockCycleSize / 1000.0; //[ns]
   // TODO:For now make 100ps default, maybe need later an option for it
   const double   kdTotBinSize      =   50.0; //ps

   const uint32_t kuFineTime    = 0x0000007F; // Fine Counter value
   const uint32_t kuFtShift     =          0; // Fine Counter offset
   const uint32_t kuCoarseTime  = 0x0007FF80; // Coarse Counter value
   const uint32_t kuCtShift     =          7; // Coarse Counter offset
   const uint32_t kuCtSize      =         12; // Coarse Counter size in bits

   const uint32_t kuFineCounterSize    = ( (kuFineTime>>kuFtShift)+1 );
   const uint32_t kuCoarseCounterSize  = ( (kuCoarseTime>>kuCtShift)+1 );
   const uint32_t kuCoarseOverflowTest = kuCoarseCounterSize / 2 ; // Limit for overflow check
   const uint32_t kuTotCounterSize     = 256;

   // Nominal bin size of NL are neglected
   const double   kdBinSize     = kdClockCycleSize / static_cast<double>(kuFineCounterSize);
   // Epoch Size in bins
   const uint32_t kuEpochInBins = kuFineTime + kuCoarseTime + 1;
   // Epoch Size in ps
   // alternatively: (kiCoarseTime>>kiCtShift + 1)*kdClockCycleSize
   const double   kdEpochInPs   = static_cast<double>(kuEpochInBins)*kdBinSize;
   const double   kdEpochInNs   = kdEpochInPs / 1000.0;

   // Epoch counter size in epoch
   const uint32_t kuEpochCounterSz = 0x7FFFFFFF;
   const double   kdEpochCycleInS  = static_cast<double>(kuEpochCounterSz) * (kdEpochInNs/1e9);
}
namespace get4v2x {
   const uint32_t kuChipIdMergedEpoch = 63; // 0x3F
}
#endif

namespace ngdpb {


   enum MessageFormat {
      formatNormal = 4   // new message format with little-endian coding, 16-bit roc number (default)
   };

   enum MessageTypes {
      MSG_NOP      =  0,
      MSG_HIT      =  1,
      MSG_EPOCH    =  2,
      MSG_SYNC     =  3,
      MSG_AUX      =  4,
      MSG_EPOCH2   =  5,
      MSG_GET4     =  6,
      MSG_SYS      =  7,
      MSG_GET4_SLC =  8,
      MSG_GET4_32B =  9,
      MSG_GET4_SYS = 10,
      MSG_STAR_TRI = 0xD
   };

   enum SysMessageTypes {
      SYSMSG_DAQ_START    = 1,      // indicates start daq in data stream
      SYSMSG_DAQ_FINISH   = 2,      // stop daq
      SYSMSG_NX_PARITY    = 3,      // nx_parity error
      SYSMSG_SYNC_PARITY  = 4,      // sync parity error
      SYSMSG_DAQ_RESUME   = 5,      // daq resume due to low/high water marker, only in udp case
      SYSMSG_FIFO_RESET   = 6,      // FPGA fifo reset
      SYSMSG_USER         = 7,      // user define message, generated by writing into ROC_ADDSYSMSG register
      SYSMSG_PCTIME       = 8,      // contains value of time() function, indicates when message was created on PC
      SYSMSG_ADC          = 9,      // contains feb1d (1 bit), channel id (7 bit) and adc value (24 bit), measured on PC
      SYSMSG_PACKETLOST   = 10,     // inserted by udp transport when packet was lost at this place
      SYSMSG_GET4_EVENT   = 11,     // GET4 event
      SYSMSG_CLOSYSYNC_ERROR = 12,  // added to data stream when the closy-sync-strobe does not match the rocs 156MHz timestamp counter
      SYSMSG_TS156_SYNC   = 13,     // added when 156MHz timestamp counter is reset by a DLM
      SYSMSG_GDPB_UNKWN   = 15,     // Raw data from gDPB in case of unknown message type from GET4
      SYSMSG_GET4V1_32BIT_0  = 240,     // Get4 V1.0, 32bit mode, Raw messages
      SYSMSG_GET4V1_32BIT_1  = 241,     // Get4 V1.0, 32bit mode, Raw messages
      SYSMSG_GET4V1_32BIT_2  = 242,     // Get4 V1.0, 32bit mode, Raw messages
      SYSMSG_GET4V1_32BIT_3  = 243,     // Get4 V1.0, 32bit mode, Raw messages
      SYSMSG_GET4V1_32BIT_4  = 244,     // Get4 V1.0, 32bit mode, Raw messages
      SYSMSG_GET4V1_32BIT_5  = 245,     // Get4 V1.0, 32bit mode, Raw messages
      SYSMSG_GET4V1_32BIT_6  = 246,     // Get4 V1.0, 32bit mode, Raw messages
      SYSMSG_GET4V1_32BIT_7  = 247,     // Get4 V1.0, 32bit mode, Raw messages
      SYSMSG_GET4V1_32BIT_8  = 248,     // Get4 V1.0, 32bit mode, Raw messages
      SYSMSG_GET4V1_32BIT_9  = 249,     // Get4 V1.0, 32bit mode, Raw messages
      SYSMSG_GET4V1_32BIT_10 = 250,     // Get4 V1.0, 32bit mode, Raw messages
      SYSMSG_GET4V1_32BIT_11 = 251,     // Get4 V1.0, 32bit mode, Raw messages
      SYSMSG_GET4V1_32BIT_12 = 252,     // Get4 V1.0, 32bit mode, Raw messages
      SYSMSG_GET4V1_32BIT_13 = 253,     // Get4 V1.0, 32bit mode, Raw messages
      SYSMSG_GET4V1_32BIT_14 = 254,     // Get4 V1.0, 32bit mode, Raw messages
      SYSMSG_GET4V1_32BIT_15 = 255,     // Get4 V1.0, 32bit mode, Raw messages
   };

   enum SysMessageUserTypes {
      SYSMSG_USER_CALIBR_ON    = 7,
      SYSMSG_USER_CALIBR_OFF   = 8,
      SYSMSG_USER_RECONFIGURE  = 9,
      SYSMSG_USER_ROCFEET_SYNC = 16
   };

   enum MessagePrintMask {
      msg_print_Prefix = 1,
      msg_print_Data   = 2,
      msg_print_Hex    = 4,
      msg_print_Human  = 8
   };

   enum MessagePrintType {
      msg_print_Cout    = 1,
      msg_print_FairLog = 2,
      msg_print_File    = 3
   };

   enum Get4Message32bTypes {
      GET4_32B_EPOCH = 0,
      GET4_32B_SLCM  = 1,
      GET4_32B_ERROR = 2,
      GET4_32B_DATA  = 3
   };

   enum Get4Message32bSlC {
      GET4_32B_SLC_SCALER    = 0,
      GET4_32B_SLC_DEADT     = 1,
      GET4_32B_SLC_SPIREAD   = 2,
      GET4_32B_SLC_START_SEU = 3
   };

   enum Get4Message32bErrors {
      GET4_V1X_ERR_READ_INIT   = 0x00,
      GET4_V1X_ERR_SYNC        = 0x01,
      GET4_V1X_ERR_EP_CNT_SYNC = 0x02,
      GET4_V1X_ERR_EP          = 0x03,
      GET4_V1X_ERR_FIFO_WRITE  = 0x04,
      GET4_V1X_ERR_LOST_EVT    = 0x05,
      GET4_V1X_ERR_CHAN_STATE  = 0x06,
      GET4_V1X_ERR_TOK_RING_ST = 0x07,
      GET4_V1X_ERR_TOKEN       = 0x08,
      GET4_V1X_ERR_READOUT_ERR = 0x09,
      GET4_V1X_ERR_SPI         = 0x0A,
      GET4_V1X_ERR_DLL_LOCK    = 0x0B,
      GET4_V1X_ERR_DLL_RESET   = 0x0C,
      GET4_V1X_ERR_TOT_OVERWRT = 0x11,
      GET4_V1X_ERR_TOT_RANGE   = 0x12,
      GET4_V1X_ERR_EVT_DISCARD = 0x13,
      GET4_V1X_ERR_ADD_RIS_EDG = 0x14,
      GET4_V1X_ERR_UNPAIR_FALL = 0x15,
      GET4_V1X_ERR_SEQUENCE_ER = 0x16,
      GET4_V1X_ERR_UNKNOWN     = 0x7F
   };

   // Max nb of each external trigger signal type
   const uint32_t kuMaxSync    =  2; // <- MAX_SYNC
   const uint32_t kuMaxAux     =  4; // <- MAX_AUX

   class Message {

      protected:
         uint64_t data;   // main and only storage field for the message

      public:
         Message() : data(0) {}

         Message(const Message& src) : data(src.data) {}

         Message( uint64_t dataIn ) : data(dataIn) {}

         virtual ~Message() {};

         void assign(const Message& src) { data = src.data; }

         Message& operator=(const Message& src) { assign(src); return *this; }

         inline void reset() { data = 0; }

         inline uint64_t getData() const { return data; }
         inline void setData( uint64_t value) { data = value; }

         inline uint64_t getFieldLong(uint32_t shift, uint32_t len) const
            { return (data >> shift) & (((static_cast<uint64_t>(1)) << len) - 1); }

         inline uint32_t getField(uint32_t shift, uint32_t len) const
            { return (data >> shift) & (((static_cast<uint32_t>(1)) << len) - 1); }

         inline void setField(uint32_t shift, uint32_t len, uint32_t value)
            { data = (data & ~((((static_cast<uint64_t>(1)) << len) - 1) << shift)) | ((static_cast<uint64_t>(value)) << shift); }

         inline uint8_t getBit(uint32_t shift) const
            { return (data >> shift) & 1; }

         inline void setBit(uint32_t shift, uint8_t value)
            { data = value ? (data | ((static_cast<uint64_t>(1)) << shift)) : (data & ~((static_cast<uint64_t>(1)) << shift)) ; }


         inline uint32_t getFieldBE(uint32_t shift, uint32_t len) const
            { return (dataBE() >> shift) & (((static_cast<uint32_t>(1)) << len) - 1); }
         inline uint8_t getBitBE(uint32_t shift) const
            { return (dataBE() >> shift) & 1; }
         inline uint64_t dataBE() const
            { return ((data&0x00000000000000FF)<<56)+
                     ((data&0x000000000000FF00)<<40)+
                     ((data&0x0000000000FF0000)<<24)+
                     ((data&0x00000000FF000000)<< 8)+
                     ((data>> 8)&0x00000000FF000000)+
                     ((data>>24)&0x0000000000FF0000)+
                     ((data>>40)&0x000000000000FF00)+
                     ((data>>56)&0x00000000000000FF);
                     }

         // --------------------------- common fields ---------------------------------

         //! Returns the message type. Valid for all message types. 4 bit
         inline uint8_t getMessageType() const { return getField(0, 4); }

         //! Returns the number of the sending ROC. Valid for all message types.
         /*!
          * The field has full 16 bits and allows to aggregate data of up to 64K ROC's
          * in one message stream.
          */
         inline uint16_t getRocNumber() const { return getField(48, 16); }

         //! Sets the message type field in the current message
         inline void setMessageType(uint8_t v) { setField(0, 4, v); }

         //! Sets the ROC number field in the current message
         inline void setRocNumber(uint16_t v) { setField(48, 16, v); }


         // ---------- nXYTER Hit data access methods ----------------

         //! For Hit data: Returns nXYTER number (2 bit field)
         /*!
          * A nDPB can support up to 4 nXYTER chips.
          * This field identifies FEB as well as chip..
          */
//         inline uint8_t getNxNumber() const { return getField(6, 2); }
         inline uint8_t getNxNumber() const { return getField(4, 2); }

         //! For Hit data: Returns 3 most significant bits of ROC LTS (3 bit field)
         /*!
          * The 3 MSBs of the ROC local time stamp counter at the time of data
          * capture are stored in this field. A comparison of this number,
          * which reflects the time of readout, and the 3 MSBs of the timestamp
          * (returned by getNxLtsMsb() const), which reflects the time of the
          * hit, allows to estimate the time the hit stayed in the nXYTER internal
          * FIFOs. This in turn gives an indication of the data load in the
          * nXYTER.
          */
         inline uint8_t getNxLtsMsb() const { return getField(8, 3); }

         //! For Hit data: Returns nXYTER time stamp (14 bit field)
         /*!
          * Raw timestamp as delivered by the nXYTER. The last-epoch flag
          * (returned by getNxLastEpoch() const) determines whether the
          * hit belongs to the current or the last epoch. Use the
          * getMsgFullTime(uint32_t) const method to determine the
          * expanded and adjusted timestamp.
          */
         inline uint16_t getNxTs() const { return getField(11, 14); }

         //! For Hit data: Returns nXYTER channel number (7 bit field)
         inline uint8_t getNxChNum() const { return getField(25, 7); }

         // 1 bit unused

         //! For Hit data: Returns ADC value (12 bit field)
         inline uint16_t getNxAdcValue() const { return getField(33, 12); }

         //! For Hit data: Returns nXYTER pileup flag (1 bit field)
         /*!
          * This flag is set when the nXYTER trigger circuit detected two hits
          * during the peak sense time interval of the slow amplitude channel.
          * This flag indicates that separation between two hits was smaller than
          * what can be handled by the amplitude channel. \sa getNxOverflow() const
          */
         inline uint8_t getNxPileup() const { return getBit(45); }

         //! For Hit data: Returns nXYTER overflow flag (1 bit field)
         /*!
          * This flag is set when a nXYTER channels detects a hit but this hit
          * can't be stored because the 4 stage channel FIFO is full. This flag
          * indicates that the hit data rate was higher than the chip can handle
          * over a longer time. \sa getNxPileup() const
          */
         inline uint8_t getNxOverflow() const { return getBit(46); }

         //! For Hit data: Returns nXYTER last-epoch flag (1 bit field)
         /*!
          * This flag is set when the hit actually belong to the previous epoch.
          * Use the getMsgFullTime(uint32_t) const method to determine the
          * expanded and adjusted timestamp.
          */
         inline uint8_t getNxLastEpoch() const { return getBit(47); }


         //! For Hit data: Sets nXYTER number (2 bit field)
         inline void setNxNumber(uint8_t v) { setField(6, 2, v); }

         //! For Hit data: Sets 3 most significant bits of ROC LTS (3 bit field)
         inline void setNxLtsMsb(uint8_t v) { setField(8, 3, v); }

         //! For Hit data: Sets nXYTER time stamp (14 bit field)
         inline void setNxTs(uint16_t v) { setField(11, 14, v); }

         //! For Hit data: Sets nXYTER channel number (7 bit field)
         inline void setNxChNum(uint8_t v) { setField(25, 7, v); }

         // 1 bit unused

         //! For Hit data: Sets ADC value (12 bit field)
         inline void setNxAdcValue(uint16_t v) { setField(33, 12, v); }

         //! For Hit data: Sets nXYTER pileup flag (1 bit field)
         inline void setNxPileup(uint8_t v) { setBit(45, v); }

         //! For Hit data: Sets nXYTER overflow flag (1 bit field)
         inline void setNxOverflow(uint8_t v) { setBit(46,v ); }

         //! For Hit data: Sets nXYTER last-epoch flag (1 bit field)
         inline void setNxLastEpoch(uint8_t v) { setBit(47, v); }

         // ---------- Epoch marker access methods ------------

         //! For Epoch data: Returns nXYTER number (2 bit field)
         /*!
          * A nDPB can support up to 4 nXYTER chips.
          * This field identifies FEB as well as chip.
          */
         inline uint8_t getEpochNxNum() const { return getField(4, 2); }

         // 2 bit unused

         //! For Epoch data: Returns current epoch number (32 bit field)
         // inline uint32_t getEpochNumber() const { return getField(8, 32); }
         // on some machines 32-bit field is not working
         inline uint32_t getEpochNumber() const { return data >> 8; }


         //! For Epoch data: Returns number of missed hits (8 bit field)
         inline uint8_t getEpochMissed() const { return getField(40, 8); }

         //! For Epoch data: Sets epoch number (32 bit field)
         inline void setEpochNumber(uint32_t v) { setField(8, 32, v); }

         //! For Epoch data: Sets missed hits count (8 bit field)
         inline void setEpochMissed(uint8_t v) { setField(40, 8, v); }


         // ---------- Sync marker access methods -------------

         //! For Sync data: Returns nXYTER number (2 bit field)
         /*!
          * A nDPB can support up to 4 nXYTER chips.
          * This field identifies FEB as well as chip.
          */
         inline uint8_t getSyncNxNum() const { return getField(4, 2); }

         //! For Sync data: Returns sync channel number (2 bit field)
         inline uint8_t getSyncChNum() const { return getField(6, 2); }

         //! For Sync data: Returns sync time stamp (13 bit field, 2 ns bins)
         inline uint16_t getSyncTs() const { return getField(8, 13) << 1; }

         //! For Sync data: Returns LSB of true epoch (1 bit field)
         inline uint8_t getSyncEpochLSB() const { return getBit(21); }

         //! For Sync data: Returns sync message data (24 bit field)
         inline uint32_t getSyncData() const { return getField(22, 24); }

         //! For Sync data: Returns sync status flags (2 bit field)
         inline uint8_t getSyncStFlag() const { return getField(46, 2); }


         //! For Sync data: Set sync channel number (2 bit field)
         inline void setSyncChNum(uint8_t v) { setField(6, 2, v); }

         //! For Sync data: Set sync time stamp (13 bit field, 2 ns bins )
         inline void setSyncTs(uint16_t v) { setField(8, 13, v >> 1); }

         //! For Sync data: Set LSB of true epoch (1 bit field)
         inline void setSyncEpochLSB(uint8_t v) { setBit(21, v); }

         //! For Sync data: Set sync message data (24 bit field)
         inline void setSyncData(uint32_t v) { setField(22, 24, v); }

         //! For Sync data: Set sync status flags (2 bit field)
         inline void setSyncStFlag(uint8_t v)  { setField(46, 2, v); }


         // ---------- AUX marker access methods --------------

         //! For AUX data: Returns nXYTER number (2 bit field)
         /*!
          * A nDPB can support up to 4 nXYTER chips.
          * This field identifies FEB as well as chip.
          */
         inline uint8_t getAuxNxNum() const { return getField(4, 2); }

         //! For Aux data: Returns aux channel number (7 bit field)
         inline uint8_t getAuxChNum() const { return getField(6, 7); }

         //! For Aux data: Returns aux time stamp (13 bit field, 2 ns bins)
         inline uint16_t getAuxTs() const { return getField(13, 13) << 1; }

         //! For Aux data: Returns LSB of true epoch (1 bit field)
         inline uint8_t getAuxEpochLSB() const { return getBit(26); }

         //! For Aux data: Returns falling edge flag (1 bit flag)
         inline uint8_t getAuxFalling() const { return getBit(27); }

         //! For Aux data: Returns overflow flag (1 bit field)
         inline uint8_t getAuxOverflow() const { return getBit(28); }


         //! For Aux data: Set aux channel number (7 bit field)
         inline void setAuxChNum(uint8_t v) { setField(6, 7, v); }

         //! For Aux data: Set aux time stamp (13 bit field)
         inline void setAuxTs(uint16_t v) { setField(13, 13, v >> 1); }

         //! For Aux data: Set LSB of true epoch (1 bit field)
         inline void setAuxEpochLSB(uint8_t v) { setBit(26, v); }

         //! For Aux data: Set falling edge flag (1 bit flag)
         inline void setAuxFalling(uint8_t v) { setBit(27, v); }

         //! For Aux data: Set overflow flag (1 bit field)
         inline void setAuxOverflow(uint8_t v) { setBit(28, v); }

         // ---------- Epoch2 marker access methods ------------

         //! For Epoch2 data: Returns epoch missmatch flag (set in ROC when
         //! ROC timestamp and timestamp send by GET4 did not match) (1 bit field)
         inline uint32_t getEpoch2EpochMissmatch() const { return getBit(4); }

         //! For Epoch2 data: Returns epoch-lost flag (1 bit field)
         inline uint32_t getEpoch2EpochLost() const { return getBit(5); }

         //! For Epoch2 data: Returns data-lost flag (1 bit field)
         inline uint32_t getEpoch2DataLost() const { return getBit(6); }

         //! For Epoch2 data: Returns sync flag (1 bit field)
         inline uint32_t getEpoch2Sync() const { return getBit(7); }

         //! For Epoch2 data: Returns the LTS156 bits 11 to 8. This
         //! gives information at what time in the Epoche the epoche number
         //! was set (2 bit field)
         inline uint32_t getEpoch2StampTime() const { return getField(8, 2); }

         //! For Epoch2 data: Returns the epoch number (32 bit field)
         // on some machines 32-bit field is not working
         inline uint32_t getEpoch2Number() const { return (data >> 10) & 0xFFFFFFFF; }

         //! For Epoch2 data: Returns the number of the GET4 chip that send
         //! the epoche message (6 bit field)
         inline uint32_t getEpoch2ChipNumber() const { return getField(42, 6); }

         //! For Epoch2 data: Set epoch missmatch flag (1 bit field)
         inline void setEpoch2EpochMissmatch(uint32_t v) { setBit(4, v); }

         //! For Epoch2 data: Set epoch-lost flag (1 bit field)
         inline void setEpoch2EpochLost(uint32_t v) { setBit(5, v); }

         //! For Epoch2 data: Set data-lost flag (1 bit field)
         inline void setEpoch2DataLost(uint32_t v) { setBit(6, v); }

         //! For Epoch2 data: Set sync flag (1 bit field)
         inline void setEpoch2Sync(uint32_t v) { setBit(7, v); }

         //! For Epoch2 data: Set the LTS156 bits 11 to 8. This
         //! gives information at what time in the Epoche the epoche number
         //! was set (2 bit field)
         inline void setEpoch2StampTime(uint32_t v) { setField(8, 2, v); }

         //! For Epoch2 data: Set the epoch number (32 bit field)
         inline void setEpoch2Number(uint32_t v) { setField(10, 32, v); }

         //! For Epoch2 data: Set the number of the GET4 chip that send
         //! the epoche message (6 bit field)
         inline void setEpoch2ChipNumber(uint32_t v) { setField(42, 6, v); }

         // ---------- Get4 24b Hit data access methods ----------------

         //! For Get4 data: Returns Get4 chip number (6 bit field)
         inline uint8_t getGet4Number() const { return getField(38, 6); }

         //! For Get4 data: Returns Get4 channel number (2 bit field)
         inline uint8_t getGet4ChNum() const { return getField(36, 2); }

         //! For Get4 data: Returns Get4 time stamp, 50 ps binning (19 bit field)
         inline uint32_t getGet4Ts() const { return getField(17, 19); }

         //! For Get4 data: Returns Get4 fine time stamp, 50 ps binning (7 bit field)
         inline uint32_t getGet4FineTs() const { return getField(17, 7); }

         //! For Get4 data: Returns Get4 coarse time stamp, 6.4 ns binning (12 bit field)
         inline uint32_t getGet4CoarseTs() const { return getField(24, 12); }

         //! For Get4 data: Returns Get4 rising or falling edge (1 bit field)
         inline uint32_t getGet4Edge() const { return getBit(16); }

         //! For Get4 data: Returns the CRC-8 of the rest of the message.
         //! For details check the ROC documentation. (8 bit field)
         inline uint32_t getGet4CRC() const { return getField(0, 8); }


         //! For Get4 data: Sets Get4 chip number (6 bit field)
         inline void setGet4Number(uint8_t v) { setField(38, 6, v); }

         //! For Get4 data: Sets Get4 channel number (2 bit field)
         inline void setGet4ChNum(uint8_t v) { setField(36, 2, v); }

         //! For Get4 data: Sets Get4 time stamp, 50 ps binning (19 bit field)
         inline void setGet4Ts(uint32_t v) { setField(17, 19, v); }

         //! For Get4 data: Sets Get4 rising or falling edge (1 bit field)
         inline void setGet4Edge(uint32_t v) { setBit(16, v); }

         //! For Get4 data: Set the CRC-8 of the rest of the message
         //! For details check the ROC documentation. (8 bit field)
         inline void setGet4CRC(uint32_t v) { setField(0, 8, v); }

         // ---------- System message access methods ----------

         // 2 bit unused

         //! For SysMes data: Returns system message subtype (8 bit field)
         inline uint8_t getSysMesType() const { return getField(8, 8); }

         //! For SysMes data: Returns system message data (32 bit field)
         // inline uint32_t getSysMesData() const { return getField(16, 32); }
         // on some machine 32-bit field not working
         inline uint32_t getSysMesData() const { return data >> 16; }

         //! For SysMes data: Set system message type (8 bit field)
         inline void setSysMesType(uint8_t v) { setField(8, 8, v); }

         //! For SysMes data: Set system message data (32 bit field)
         inline void setSysMesData(uint32_t v) { setField(16, 32, v); }

         // ---------- Get4 gDPB 24b/32b ALL access methods ------------------------
         inline uint16_t getGdpbGenChipId()      const { return getField( 42,  6); }
         inline void     setGdpbGenChipId(uint32_t v)   { setField(42, 6, v); }

         // ---------- Get4 gDPB 24b/32b Hit access methods ------------------------
//         inline uint16_t getGdpbHitChipId()      const { return getField( 42,  6); }
         inline uint16_t getGdpbHitChanId()      const { return getField( 40,  2); }
         inline uint32_t getGdpbHitFullTs()      const { return getField( 21, 19); }
         inline uint16_t getGdpbHitCrc()         const { return getField(  4,  8); }
         inline uint16_t getGdpbHitCoarse()      const { return getField( 28, 12); }
         inline uint16_t getGdpbHitFineTs()      const { return getField( 21,  7); }
         // ---------- Get4 gDPB 24b Hit access methods ----------------------------
         inline bool     getGdpbHit24Edge()      const { return getBit(   20    ); }
         // ---------- Get4 gDPB 32b Hit access methods ----------------------------
         inline bool     getGdpbHit32DllLck()    const { return getBit(   20    ); }
         inline uint16_t getGdpbHit32Tot()       const { return getField( 12,  8); }

         // ---------- Get4 gDPB 24b/32b Epoch access methods ----------------------
//         inline uint16_t getGdpbEpChipId()       const { return getField( 42,  6); }
         inline bool     getGdpbEpLinkId()       const { return getBit(   41    ); }
         inline uint32_t getGdpbEpEpochNb()      const { return getField( 10, 31); }
         inline uint16_t getGdpbEpStampTs()      const { return getField(  8,  2); }
         inline bool     getGdpbEpSync()         const { return getBit(    7    ); }
         inline bool     getGdpbEpDataLoss()     const { return getBit(    6    ); }
         inline bool     getGdpbEpEpochLoss()    const { return getBit(    5    ); }
         inline bool     getGdpbEpMissmatch()    const { return getBit(    4    ); }

         // ---------- Get4 gDPB 24b/32b Slow cont. access methods -----------------
//         inline uint16_t getGdpbSlcChipId()      const { return getField( 42,  6); }
         inline uint32_t getGdpbSlcMess()        const { return getField( 12, 29); }
         inline uint32_t getGdpbSlcData()        const { return getField( 12, 24); }
         inline uint32_t getGdpbSlcType()        const { return getField( 36,  2); }
         inline uint32_t getGdpbSlcEdge()        const { return getBit(   38    ); }
         inline uint32_t getGdpbSlcChan()        const { return getField( 39,  2); }
         inline uint16_t getGdpbSlcCrc()         const { return getField(  4,  8); }

         // ---------- Get4 gDPB System Msg access methods -------------------------
//         inline uint16_t getGdpbSysChipId()      const { return getField( 42,  6); }
         inline bool     getGdpbSysLinkId()      const { return getBit(   41    ); }
         inline uint16_t getGdpbSysSubType()     const { return getField( 37,  4); }
         // ---------- Get4 gDPB 24b/32b Errors access methods ---------------------
         inline bool     getGdpbSysErrRoType()   const { return getBit(   36    ); }
         inline uint16_t getGdpbSysErrUnused()   const { return getField( 14,  22); }
         inline uint16_t getGdpbSysErrChanId()   const { return getField( 12,  2); }
         inline bool     getGdpbSysErrEdge()     const { return getBit(   11    ); }
         inline uint16_t getGdpbSysErrData()     const { return getField(  4,  7); }
         // ---------- Get4 gDPB unknown msg type access methods -------------------
         inline uint32_t getGdpbSysUnkwData()    const { return getField(  4, 32); }

         // ---------- STAR Trigger messages access methods ------------------------
         inline uint16_t getStarTrigMsgIndex() const { return getField(      4,  4 ); }
         //++++//
         inline uint64_t getGdpbTsMsbStarA()   const { return getFieldLong(  8, 40 ); }
         //++++//
         inline uint64_t getGdpbTsLsbStarB()   const { return getFieldLong( 24, 24 ); }
         inline uint64_t getStarTsMsbStarB()   const { return getFieldLong(  8, 16 ); }
         //++++//
         inline uint64_t getStarTsMidStarC()   const { return getFieldLong(  8, 40 ); }
         //++++//
         inline uint64_t getStarTsLsbStarD()   const { return getFieldLong( 40,  8 ); }
         /// 12 bits in between are set to 0
         inline uint32_t getStarFillerD()      const { return getField(     28, 12 ); } // Should be always 0
         inline uint32_t getStarTokenStarD()   const { return getField(      8, 12 ); }
         inline uint32_t getStarDaqCmdStarD()  const { return getField(     20,  4 ); }
         inline uint32_t getStarTrigCmdStarD() const { return getField(     24,  4 ); }

         // ---------- Common functions -----------------------

         //! Returns \a true is message type is #MSG_NOP (filler message)
         inline bool isNopMsg() const { return getMessageType() == MSG_NOP; }
         //! Returns \a true is message type is #MSG_HIT (nXYTER hit data)
         inline bool isHitMsg() const { return getMessageType() == MSG_HIT; }
         //! Returns \a true is message type is #MSG_EPOCH (epoch marker)
         inline bool isEpochMsg() const { return getMessageType() == MSG_EPOCH;}
         //! Returns \a true is message type is #MSG_SYNC
         inline bool isSyncMsg() const { return getMessageType() == MSG_SYNC; }
         //! Returns \a true is message type is #MSG_AUX
         inline bool isAuxMsg() const { return getMessageType() == MSG_AUX; }
         //! Returns \a true is message type is #MSG_EPOCH2 (epoch2 marker)
         inline bool isEpoch2Msg() const { return getMessageType() == MSG_EPOCH2;}
         //! Returns \a true is message type is #MSG_GET4 (Get4 hit data)
         inline bool isGet4Msg() const { return getMessageType() == MSG_GET4; }
         //! Returns \a true is message type is #MSG_SYS (system message)
         inline bool isSysMsg() const { return getMessageType() == MSG_SYS; }
         //! Returns \a true is message type is #MSG_GET4_SLC (GET4 Slow Control)
         inline bool isGet4SlCtrMsg() const { return getMessageType() == MSG_GET4_SLC; }
         //! Returns \a true is message type is #MSG_GET4_32B (GET4 Hit Data in 32b mode)
         inline bool isGet4Hit32Msg() const { return getMessageType() == MSG_GET4_32B; }
         //! Returns \a true is message type is #MSG_GET4_SYS (GET4 system message)
         inline bool isGet4SysMsg() const { return getMessageType() == MSG_GET4_SYS; }
         //! Returns \a true is message type is #MSG_STAR_TRI (STAR Trigger message)
         inline bool isStarTrigger() const { return getMessageType() == MSG_STAR_TRI;}
         //! Returns \a true is message type is #MSG_SYS (system message) and subtype is 32bHack
         inline bool isGet4Hack32Msg() const { return
               ( (getMessageType() == MSG_SYS) &&
                 (SYSMSG_GET4V1_32BIT_0 <= getSysMesType()) ); }

         //! Returns \a true if system message and subtype #ROC_SYSMSG_DAQ_START
         inline bool isStartDaqMsg() const
           { return isSysMsg() && (getSysMesType() == SYSMSG_DAQ_START); }
         //! Returns \a true if system message and subtype #ROC_SYSMSG_DAQ_FINISH
         inline bool isStopDaqMsg() const
           { return isSysMsg() && (getSysMesType() == SYSMSG_DAQ_FINISH); }


         void printDataCout(unsigned kind = msg_print_Prefix | msg_print_Data, uint32_t epoch = 0) const;
         void printDataLog(unsigned kind = msg_print_Prefix | msg_print_Data, uint32_t epoch = 0) const;

//         void printData(std::ostream& os, unsigned kind = msg_print_Human, uint32_t epoch = 0) const;
         void printData(unsigned outType = msg_print_Cout, unsigned kind = msg_print_Human,
                      uint32_t epoch = 0, std::ostream& os = std::cout ) const;

         uint64_t getMsgFullTime(uint32_t epoch) const;

         double getMsgFullTimeD(uint32_t epoch) const;

         uint64_t getMsgG4v2FullTime(uint32_t epoch) const;

         double getMsgG4v2FullTimeD(uint32_t epoch) const;

         //! Expanded timestamp for 160 MHz * 14 bit (12 + 2) epochs
         inline static uint64_t FullTimeStamp(uint32_t epoch, uint16_t stamp)
            { return (static_cast<uint64_t>(epoch) << 14) | (stamp & 0x3fff); }

         //! Expanded timestamp for 160 MHz * 19 bit (12 + 7) epochs
         inline static uint64_t FullTimeStamp2(uint32_t epoch, uint32_t stamp)
            { return (static_cast<uint64_t>(epoch) << 19) | (stamp & 0x7ffff); }


         static uint64_t CalcDistance(uint64_t start, uint64_t stop);

         static double CalcDistanceD(double start, double stop);

         bool operator<(const ngdpb::Message& other) const;

         // -------------------- methods for working with different formats

         static uint32_t RawSize(int fmt);

         bool assign(void* src, int fmt = formatNormal);

         bool copyto(void* tgt, int fmt = formatNormal);
   };

   class FullMessage : public Message {
      protected:
         uint64_t fuExtendedEpoch;   // Epoch of the message, extended with 32b epoch cycle counter

      public:
         FullMessage() : Message(), fuExtendedEpoch(0) {}

         FullMessage(const Message& src, uint64_t uEpIn = 0) : Message(src), fuExtendedEpoch(uEpIn) {}

         FullMessage(const FullMessage& src) : Message(src), fuExtendedEpoch(src.fuExtendedEpoch) {}

         void assign(const FullMessage& src) { Message::assign(src); fuExtendedEpoch = src.fuExtendedEpoch; }

         FullMessage& operator=(const FullMessage& src) { assign(src); return *this; }

         bool operator<(const FullMessage& other) const;

         inline void reset() {  Message::reset(); fuExtendedEpoch = 0; }

         inline uint64_t getExtendedEpoch() const {return fuExtendedEpoch; }
   };

}


#endif // ROC_MESS_WGET4V1X_H
