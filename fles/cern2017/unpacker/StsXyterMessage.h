/************************************************************
 *
 ************************************************************/

#ifndef STSXYTERMESSAGE_H
#define STSXYTERMESSAGE_H

// C/C++ headers
#include <stdint.h>
#include <iostream>

// Switch to VHDL-like bitwise CRC if defined
// CrcCpp implementation otherwise
#define USE_VHDL_CRC

namespace stsxyter {

// tools: Use to this allow bitwise operations on C++11 enums
// This needs to be included in the same namespace to work...
#include "bitmask_operators.hpp"

   /// Constants and other auxiliary stuff
      /// Field full info object
   struct MessField
   {
      uint16_t     fusPos; //! Start bit index
      uint16_t     fusLen; //! Field length

      MessField( uint16_t usPos, uint16_t usLen ) :
         fusPos( usPos ),
         fusLen( usLen )
      { };
   };
      /// Message types
   enum class MessType : uint16_t
   {
      Dummy,
      Hit,
      TsMsb,
      ReadDataAck,
      Ack
   };
      /// Printout control
   enum class MessagePrintMask : uint16_t
   {
      msg_print_Prefix = (0x1 << 0),
      msg_print_Data   = (0x1 << 1),
      msg_print_Hex    = (0x1 << 2),
      msg_print_Human  = (0x1 << 3),
      msg_print_Crc    = (0x1 << 4)
   };
   ENABLE_BITMASK_OPERATORS( stsxyter::MessagePrintMask ) // Preproc macro!

      /// Fields position (Start bit index)
   static const uint16_t kusPosLinkIndex  = 24;
   static const uint16_t kusPosNotHitFlag = 23;
      // Hit message
   static const uint16_t kusPosHitChannel = 16;
   static const uint16_t kusPosHitAdc     = 11;
   static const uint16_t kusPosHitTsOver  =  9;
   static const uint16_t kusPosHitTs      =  1;
   static const uint16_t kusPosHitEmFlag  =  0;
      // Non-hit messages
   static const uint16_t kusPosTsMsbFlag  = 22;
         // TS_MSB message
   static const uint16_t kusPosTsMsbValA  = 16;
   static const uint16_t kusPosTsMsbValB  = 10;
   static const uint16_t kusPosTsMsbValC  =  4;
   static const uint16_t kusPosTsMsbCrc   =  0;
         // Control messages
   static const uint16_t kusPosRdDataFlag = 21;
            // Read Data Ack message
   static const uint16_t kusPosRdDataVal  =  6;
   static const uint16_t kusPosRdDataSeqN =  3;
   static const uint16_t kusPosRdDataCrc  =  0;
            // Ack messages
   static const uint16_t kusPosAckValAck  = 19;
   static const uint16_t kusPosAckSeqNb   = 15;
   static const uint16_t kusPosAckCpFlag  = 14;
   static const uint16_t kusPosAckStatus  = 10;
   static const uint16_t kusPosAckTime    =  4;
   static const uint16_t kusPosAckCrc     =  0;

      /// Fields length (bits)
   static const uint16_t kusLenLinkIndex  =  8;
   static const uint16_t kusLenRawMess    = 24;
   static const uint16_t kusLenNotHitFlag =  1;
      // Hit message
   static const uint16_t kusLenHitChannel =  7;
   static const uint16_t kusLenHitAdc     =  5;
   static const uint16_t kusLenHitTsFull  = 10; // Includes 2 bit overlap with TS message
   static const uint16_t kusLenHitTsOver  =  2; // 2 bit overlap with TS message
   static const uint16_t kusLenHitTs      =  8;
   static const uint16_t kusLenHitEmFlag  =  1;
      // Other message
   static const uint16_t kusLenTsMsbFlag  =  1;
         // TS_MSB message
   static const uint16_t kusLenTsMsbValA  =  6;
   static const uint16_t kusLenTsMsbValB  =  6;
   static const uint16_t kusLenTsMsbValC  =  6;
   static const uint16_t kusLenTsMsbCrc   =  4;
         // Control messages
   static const uint16_t kusLenRdDataFlag =  1;
            // Read Data Ack message
   static const uint16_t kusLenRdDataVal  = 15;
   static const uint16_t kusLenRdDataSeqN =  3;
   static const uint16_t kusLenRdDataCrc  =  3;
            // Ack messages
   static const uint16_t kusLenAckValAck  =  2;
   static const uint16_t kusLenAckSeqNb   =  4;
   static const uint16_t kusLenAckCpFlag  =  1;
   static const uint16_t kusLenAckStatus  =  4;
   static const uint16_t kusLenAckTime    =  6;
   static const uint16_t kusLenAckCrc     =  4;

      /// Fields Info
   static const MessField kFieldLinkIndex(  kusPosLinkIndex,  kusLenLinkIndex );
   static const MessField kFieldNotHitFlag( kusPosNotHitFlag, kusLenNotHitFlag );
      // Hit message
   static const MessField kFieldHitChannel( kusPosHitChannel, kusLenHitChannel );
   static const MessField kFieldHitAdc    ( kusPosHitAdc,     kusLenHitAdc );
   static const MessField kFieldHitTsFull ( kusPosHitTs,      kusLenHitTsFull );
   static const MessField kFieldHitTsOver ( kusPosHitTsOver,  kusLenHitTsOver );
   static const MessField kFieldHitTs     ( kusPosHitTs,      kusLenHitTs );
   static const MessField kFieldHitEmFlag ( kusPosHitEmFlag,  kusLenHitEmFlag );
      // Non-hit messages
   static const MessField kFieldTsMsbFlag ( kusPosTsMsbFlag,  kusLenTsMsbFlag );
         // TS_MSB message
   static const MessField kFieldTsMsbValA ( kusPosTsMsbValA,  kusLenTsMsbValA );
   static const MessField kFieldTsMsbValB ( kusPosTsMsbValB,  kusLenTsMsbValB );
   static const MessField kFieldTsMsbValC ( kusPosTsMsbValC,  kusLenTsMsbValC );
   static const MessField kFieldTsMsbCrc  ( kusPosTsMsbCrc,   kusLenTsMsbCrc );
         // Control messages
   static const MessField kFieldRdDataFlag( kusPosRdDataFlag, kusLenRdDataFlag );
            // Read Data Ack message
   static const MessField kFieldRdDataVal ( kusPosRdDataVal,  kusLenRdDataVal );
   static const MessField kFieldRdDataSeqN( kusPosRdDataSeqN, kusLenRdDataSeqN );
   static const MessField kFieldRdDataCrc ( kusPosRdDataCrc,  kusLenRdDataCrc );
            // Ack messages
   static const MessField kFieldAckValAck ( kusPosAckValAck,  kusLenAckValAck );
   static const MessField kFieldAckSeqNb  ( kusPosAckSeqNb,   kusLenAckSeqNb );
   static const MessField kFieldAckCpFlag ( kusPosAckCpFlag,  kusLenAckCpFlag);
   static const MessField kFieldAckStatus ( kusPosAckStatus,  kusLenAckStatus );
   static const MessField kFieldAckTime   ( kusPosAckTime,    kusLenAckTime );
   static const MessField kFieldAckCrc    ( kusPosAckCrc,     kusLenAckCrc );

      /// Status/properties constants
   static const uint32_t  kuHitNbAdcBins   = 1 << kusLenHitAdc;
   static const uint32_t  kuHitNbTsBins    = 1 << kusLenHitTs;
   static const uint32_t  kuHitNbOverBins  = 1 << kusLenHitTsOver;
   static const uint32_t  kuTsMsbNbTsBins  = 1 << kusLenTsMsbValA;
   static const uint32_t  kuTsCycleNbBins  = kuTsMsbNbTsBins * kuHitNbTsBins;
   static const uint16_t  kusInvalidTsMsb  = kuTsMsbNbTsBins;
   static const uint16_t  kusMaskTsMsbOver = (1 << kusLenHitTsOver) - 1;
   static const double    kdClockCycleNs   = 6.25; // ns, equivalent to 160 MHz clock

   class Message {
      private:
#ifndef USE_VHDL_CRC
         uint16_t CalcCrc3Pol5Len21( uint32_t uVal ) const;
         uint16_t CalcCrc4Pol9Len20( uint32_t uVal ) const;
#endif // not USE_VHDL_CRC
         uint16_t VhdlCalcCrc3Pol5Len21( uint32_t uVal ) const;
         uint16_t VhdlCalcCrc4Pol9Len20( uint32_t uVal ) const;


      protected:
         uint32_t fuData;   // main and only storage field for the message

      public:
         Message() : fuData(0) {}

         Message(const Message& src) : fuData(src.fuData) {}

         Message( uint32_t uDataIn ) : fuData( uDataIn ) {}

         virtual ~Message() {};

         void assign(const Message& src) { fuData = src.fuData; }

         Message& operator=(const Message& src) { assign(src); return *this; }

         inline void reset() { fuData = 0; }

         // --------------------------- Accessors ---------------------------------
         inline uint32_t GetData() const { return fuData; }
         inline void     SetData( uint32_t uValue) { fuData = uValue; }

         inline uint32_t GetField(uint32_t uShift, uint32_t uLen) const
            { return ( fuData >> uShift) & ( ( (static_cast<uint32_t>( 1 ) ) << uLen ) - 1); }

         inline uint8_t GetBit(uint32_t uShift) const
            { return ( fuData >> uShift) & 1; }

         inline uint32_t GetFieldBE(uint32_t uShift, uint32_t uLen) const
            { return ( DataBE() >> uShift ) & ( ( ( static_cast<uint32_t>( 1 ) ) << uLen ) - 1 ); }
         inline uint8_t GetBitBE(uint32_t uShift) const
            { return (DataBE() >> uShift) & 1; }
         inline uint32_t DataBE() const
            { return ( ( fuData        & 0x000000FF ) << 24) +
                     ( ( fuData        & 0x0000FF00 ) <<  8) +
                     ( ( fuData >>  8) & 0x0000FF00 ) +
                     ( ( fuData >> 24) & 0x000000FF );
                     }

         // --------------------------- Setters ---------------------------------------
         inline void     SetField(uint32_t uShift, uint32_t uLen, uint32_t uValue)
            { fuData =  ( fuData & ~( ( ( ( static_cast< uint32_t >( 1 ) ) << uLen) - 1) << uShift ) )
                      | ( ( static_cast< uint64_t >( uValue ) ) << uShift ); }

         inline void    SetBit(uint32_t uShift, uint8_t uValue)
            { fuData = uValue ? ( fuData |  ( ( static_cast< uint32_t >( 1 ) ) << uShift ) ) :
                                ( fuData & ~( ( static_cast< uint32_t >( 1 ) ) << uShift ) ) ; }

         // --------------------------- Simplified Acc_or/Setters ---------------------
         inline uint32_t GetField(   MessField field ) const { return GetField(   field.fusPos, field.fusLen ); }
         inline uint8_t  GetBit(     MessField field ) const { return GetBit(     field.fusPos ); }
         inline bool     GetFlag(    MessField field ) const { return ( 1 == GetBit( field.fusPos ) ); }
         inline uint32_t GetFieldBE( MessField field ) const { return GetFieldBE( field.fusPos, field.fusLen ); }
         inline uint8_t  GetBitBE(   MessField field ) const { return GetBitBE(   field.fusPos ); }

         inline void     SetField( MessField field, uint32_t uValue  ) { SetField( field.fusPos, field.fusLen, uValue ); }
         inline void     SetBit(   MessField field, uint8_t  ucValue ) { SetBit(   field.fusPos, ucValue ); }

         // --------------------------- common fields ---------------------------------
         //! For all data: Returns the (global) index of the eLink on which the message was received (n bit field)
         inline uint16_t GetLinkIndex()   const { return GetField( kFieldLinkIndex ); }
         //! Check if the message if a Hit Message
         inline bool     IsHit()         const { return !GetFlag( kFieldNotHitFlag ); }
         //! Check if the message if a Dummy Hit Message
         inline bool     IsDummy()       const { return IsHit() && ( 0 == GetHitAdc() ); }
         //! Check if the message if a Ts_Msb
         inline bool     IsTsMsb()       const { return (!IsHit() ) && GetFlag( kFieldTsMsbFlag ); }
         //! Check if the message if a Hit Message
         inline bool     IsReadDataAck() const { return (!IsHit() ) && (!IsTsMsb() )
                                                                    && GetFlag( kFieldRdDataFlag ); }
         //! Returns the message type, see enum MessType
         inline MessType GetMessType() const { return !GetFlag( kFieldNotHitFlag ) ? ( 0 == GetHitAdc() ? MessType::Dummy :
                                                                                                          MessType::Hit ) :
                                                       GetFlag( kFieldTsMsbFlag )  ? MessType::TsMsb       :
                                                       GetFlag( kFieldRdDataFlag ) ? MessType::ReadDataAck :
                                                                                     MessType::Ack; }

         // ------------------------ Hit message fields -------------------------------
         //! For Hit data: Returns StsXYTER channel number (7 bit field)
         inline uint16_t GetHitChannel()   const { return GetField( kFieldHitChannel ); }

         //! For Hit data: Returns ADC value (5 bit field)
         inline uint16_t GetHitAdc()       const { return GetField( kFieldHitAdc ); }

         //! For Hit data: Returns Full timestamp (10 bit field including 2 bits overlap)
         inline uint16_t GetHitTimeFull()  const { return GetField( kFieldHitTsFull ); }

         //! For Hit data: Returns timestamp overlap bits (2 bits field, overlap with 2 LSBs of TS_MSB message)
         inline uint16_t GetHitTimeOver()  const { return GetField( kFieldHitTsOver ); }

         //! For Hit data: Returns timestamp (8 bit field, 2 MSB bits overlap removed)
         inline uint16_t GetHitTime()      const { return GetField( kFieldHitTs ); }

         //! For Hit data: Returns Missed event flag (1 bit field)
         inline bool     IsHitMissedEvts() const { return GetFlag(  kFieldHitEmFlag ); }

         //! For Hit data: Sets StsXYTER channel number (7 bit field)
         inline void SetHitChannel( uint16_t usVal )  { SetField( kFieldHitChannel, usVal ); }

         //! For Hit data: Sets ADC value (5 bit field)
         inline void SetHitAdc(  uint16_t usVal )     { SetField( kFieldHitAdc,     usVal ); }

         //! For Hit data: Sets Full timestamp (10 bit field including 2 bits overlap)
         inline void SetHitTimeFull( uint16_t usVal ) { SetField( kFieldHitTsFull,  usVal ); }

         //! For Hit data: Sets timestamp overlap bits (2 bits field, overlap with 2 LSBs of TS_MSB message)
         inline void SetHitTimeOver( uint16_t usVal ) { SetField( kFieldHitTsOver,  usVal ); }

         //! For Hit data: Sets Full timestamp (8 bit field, 2 MSB bits overlap removed)
         inline void SetHitTime( uint16_t usVal )     { SetField( kFieldHitTs,      usVal ); }

         //! For Hit data: Sets Missed event flag (1 bit field)
         inline void  SetHitMissEvtsFlag( bool bVal)  { SetBit(   kFieldHitEmFlag, bVal ); }

         // ------------------------ TS_MSB message fields ----------------------------
         //! For TS MSB data: Returns 1st copy of the TS MSB (6 bit field)
         inline uint16_t GetTsMsbValA() const { return GetField( kFieldTsMsbValA ); }
         //! For TS MSB data: Returns 2nd copy of the TS MSB (6 bit field)
         inline uint16_t GetTsMsbValB() const { return GetField( kFieldTsMsbValB ); }
         //! For TS MSB data: Returns 3rd copy of the TS MSB (6 bit field)
         inline uint16_t GetTsMsbValC() const { return GetField( kFieldTsMsbValC ); }
         //! For TS MSB data: Returns message CRC (4 bit field)
         inline uint16_t GetTsMsbCrc()  const { return GetField( kFieldTsMsbCrc ); }
         //! For TS MSB data: Returns data protected by CRC (20 bit field)
         inline uint32_t GetTsMsbCrcInput() const { return GetField( kFieldTsMsbValC.fusPos,
                                                                     kusLenRawMess - kFieldTsMsbCrc.fusLen ); }

         //! For TS MSB data: Sets 1st copy of the TS MSB (6 bit field)
         inline void SetTsMsbValA( uint16_t usVal ) { SetField( kFieldTsMsbValA, usVal ); }
         //! For TS MSB data: Sets 2nd copy of the TS MSB (6 bit field)
         inline void SetTsMsbValB( uint16_t usVal ) { SetField( kFieldTsMsbValB, usVal ); }
         //! For TS MSB data: Sets 3rd copy of the TS MSB (6 bit field)
         inline void SetTsMsbValC( uint16_t usVal ) { SetField( kFieldTsMsbValC, usVal ); }
         //! For TS MSB data: Sets message CRC (4 bit field)
         inline void SetTsMsbCRC(  uint16_t usVal ) { SetField( kFieldTsMsbCrc, usVal ); }

         // ------------------------ Read Data Ack message fields ---------------------
         //! For Read Data Ack data: Returns read value (15 bit field)
         inline uint16_t GetRdDataVal()  const { return GetField( kFieldRdDataVal ); }
         //! For Read Data Ack data: Returns Sequence Number (3 bit field)
         inline uint16_t GetRdDataSeqN() const { return GetField( kFieldRdDataSeqN ); }
         //! For Read Data Ack data: Returns message CRC (3 bit field)
         inline uint16_t GetRdDataCrc()  const { return GetField( kFieldRdDataCrc ); }
         //! For Read Data Ack data: Returns data protected by CRC (21 bit field)
         inline uint32_t GetRdDataCrcInput() const { return GetField( kFieldRdDataSeqN.fusPos,
                                                                      kusLenRawMess - kFieldRdDataCrc.fusLen ); }

         //! For Read Data Ack data: Sets read value (15 bit field)
         inline void SetRdDataVal(  uint16_t usVal ) { SetField( kFieldRdDataVal,  usVal ); }
         //! For Read Data Ack data: Sets Sequence Number (3 bit field)
         inline void SetRdDataSeqN( uint16_t usVal ) { SetField( kFieldRdDataSeqN, usVal ); }
         //! For Read Data Ack data: Sets message CRC (3 bit field)
         inline void SetRdDataCrc(  uint16_t usVal ) { SetField( kFieldRdDataCrc,  usVal ); }

         // ------------------------ Ack message fields -------------------------------
         //! For Ack data: Returns ACK value (2 bit field)
         inline uint16_t GetAckVal()    const { return GetField( kFieldAckValAck ); }
         //! For Ack data: Returns Sequence Number (4 bit field)
         inline uint16_t GetAckSeqNb()  const { return GetField( kFieldAckSeqNb ); }
         //! For Ack data: Returns config parity flag (1 bit field)
         inline uint16_t GetAckCpFlag() const { return GetFlag(  kFieldAckCpFlag ); }
         //! For Ack data: Returns ACK status (4 bit field)
         inline uint16_t GetAckStatus() const { return GetField( kFieldAckStatus ); }
         //! For Ack data: Returns ACK time (6 bit field)
         inline uint16_t GetAckTime()   const { return GetField( kFieldAckTime ); }
         //! For Ack data: Returns message CRC (4 bit field)
         inline uint16_t GetAckCrc()    const { return GetField( kFieldAckCrc ); }
         //! For Ack data: Returns data protected by CRC (20 bit field)
         inline uint32_t GetAckCrcInput() const { return GetField( kFieldAckTime.fusPos,
                                                                   kusLenRawMess - kFieldAckCrc.fusLen ); }

         //! For Ack data: Sets ACK value (2 bit field)
         inline void SetAckVal(    uint16_t usVal ) { SetField( kFieldAckValAck, usVal ); }
         //! For Ack data: Sets Sequence Number (4 bit field)
         inline void SetAckSeqNb(  uint16_t usVal ) { SetField( kFieldAckSeqNb,  usVal ); }
         //! For Ack data: Sets config parity flag (1 bit field)
         inline void SetAckCpFlag( uint16_t usVal ) { SetBit(   kFieldAckCpFlag, usVal ); }
         //! For Ack data: Sets ACK status (4 bit field)
         inline void SetAckStatus( uint16_t usVal ) { SetField( kFieldAckStatus, usVal ); }
         //! For Ack data: Sets ACK time (6 bit field)
         inline void SetAckTime(   uint16_t usVal ) { SetField( kFieldAckTime,   usVal ); }
         //! For Ack data: Sets message CRC (4 bit field)
         inline void SetAckCrc(    uint16_t usVal ) { SetField( kFieldAckCrc,    usVal ); }

         // ------------------------ Hit message OP -----------------------------------
         // ------------------------ TS_MSB message OP --------------------------------
         //! For TS MSB data: computes message 4b 0x9 CRC
         uint16_t TsMsbCrcCalc()   const;
         //! For TS MSB data: check if 4b 0x9 CRC is ok
         bool     TsMsbCrcCheck()  const;
         //! For TS MSB data: check if CRC is ok and all 3 TS copies agree
         bool     TsMsbTsCheck()   const;
         //! For TS MSB data: count how many values are agreeing, possible are 1 (none), 2 or 3
         uint16_t GetTsMsbMajCnt() const;
         //! For TS MSB data: check TS integrity and try to correct by majority if corrupted
         bool     GetTsMsbValCorr( uint16_t & uVal ) const;
         // ------------------------ Read Data Ack message OP -------------------------
         //! For Read Data Sck data: computes message 3b 0x5 CRC
         uint16_t RdDataCrcCalc()  const;
         //! For Read Data Sck data: check if 3b 0x5 CRC is ok
         bool     RdDataCrcCheck() const;
         // ------------------------ Ack message OP -----------------------------------
         //! For Ack data: computes message 4b 0x9 CRC
         uint16_t AckCrcCalc()     const;
         //! For Ack data: check if 4b 0x9 CRC is ok
         bool     AckCrcCheck()    const;

         // ------------------------ General OP ---------------------------------------
         bool PrintMess( std::ostream& os, MessagePrintMask ctrl = MessagePrintMask::msg_print_Human ) const;
   };
}
#endif // STSXYTERMESSAGE_H
