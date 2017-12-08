/************************************************************
 *
 ************************************************************/

#ifndef STSXYTERMESSAGE_H
#define STSXYTERMESSAGE_H

// C/C++ headers
#include <stdint.h>
#include <iostream>

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
      TsMsb
   };
      /// Printout control
   enum class MessagePrintMask : uint16_t
   {
      msg_print_Prefix = (0x1 << 0),
      msg_print_Data   = (0x1 << 1),
      msg_print_Hex    = (0x1 << 2),
      msg_print_Human  = (0x1 << 3)
   };
   ENABLE_BITMASK_OPERATORS( stsxyter::MessagePrintMask ) // Preproc macro!

      /// Fields position (Start bit index)
   static const uint16_t kusPosNotHitFlag = 31;
   static const uint16_t kusPosLinkIndex  = 22;
      // Hit message
   static const uint16_t kusPosHitChannel = 15;
   static const uint16_t kusPosHitAdc     = 10;
   static const uint16_t kusPosHitTsOver  =  9;
   static const uint16_t kusPosHitTs      =  1;
   static const uint16_t kusPosHitEmFlag  =  0;
      // Non-hit messages
   static const uint16_t kusPosTsMsbFlag  = 22;
         // TS_MSB message
   static const uint16_t kusPosTsMsbVal   =  0;

      /// Fields length (bits)
   static const uint16_t kusLenNotHitFlag =  1;
   static const uint16_t kusLenLinkIndex  =  9;
      // Hit message
   static const uint16_t kusLenHitChannel =  7;
   static const uint16_t kusLenHitAdc     =  5;
   static const uint16_t kusLenHitTsFull  =  9; // Includes 1 bit overlap with TS message ?
   static const uint16_t kusLenHitTsOver  =  1; // 2 bit overlap with TS message
   static const uint16_t kusLenHitTs      =  8;
   static const uint16_t kusLenHitEmFlag  =  1;
      // Other message
   static const uint16_t kusLenTsMsbFlag  =  1;
         // TS_MSB message
   static const uint16_t kusLenTsMsbVal   = 13;

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
   static const MessField kFieldTsMsbVal  ( kusPosTsMsbVal,   kusLenTsMsbVal );

      /// Status/properties constants
   static const uint32_t  kuHitNbAdcBins   = 1 << kusLenHitAdc;
   static const uint32_t  kuHitNbTsBins    = 1 << kusLenHitTs;
   static const uint32_t  kuHitNbOverBins  = 1 << kusLenHitTsOver;
   static const uint32_t  kuTsMsbNbTsBins  = 1 << kusLenTsMsbVal;
   static const uint32_t  kuTsCycleNbBins  = kuTsMsbNbTsBins * kuHitNbTsBins;
   static const uint16_t  kusInvalidTsMsb  = kuTsMsbNbTsBins;
   static const uint16_t  kusMaskTsMsbOver = (1 << kusLenHitTsOver) - 1;
   static const double    kdClockCycleNs   = 3.125; // ns, equivalent to 2*160 MHz clock

   class Message {
      private:


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
         inline bool     IsTsMsb()       const { return (!IsHit() ); }
         //! Returns the message type, see enum MessType
         inline MessType GetMessType() const { return !GetFlag( kFieldNotHitFlag ) ? ( 0 == GetHitAdc() ? MessType::Dummy :
                                                                                                          MessType::Hit ) :
                                                                                     MessType::TsMsb; }

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
         //! For TS MSB data: Returns the TS MSB 13 bit field)
         inline uint16_t GetTsMsbVal() const { return GetField( kFieldTsMsbVal ); }

         //! For TS MSB data: Sets the TS MSB (13 bit field)
         inline void SetTsMsbVal( uint16_t usVal ) { SetField( kFieldTsMsbVal, usVal ); }

         // ------------------------ General OP ---------------------------------------
         bool PrintMess( std::ostream& os, MessagePrintMask ctrl = MessagePrintMask::msg_print_Human ) const;
   };
}
#endif // STSXYTERMESSAGE_H
