/************************************************************
 *
 ************************************************************/

#ifndef STSXYTERBETAHIT_H
#define STSXYTERBETAHIT_H

// C/C++ headers
#include <stdint.h>
#include <iostream>

namespace stsxyter {

   class BetaHit {
      private:


      protected:
         uint64_t fulLongTs;   // (Software-)Extended TS, in clock cycles
         uint16_t fusRawAdc;   // Raw ADC value

      public:

         BetaHit(const BetaHit& src) : fulLongTs(src.fulLongTs), fusRawAdc(src.fusRawAdc) {}

         BetaHit( uint64_t ulLongTsIn, uint16_t usRawAdcIn ) : fulLongTs( ulLongTsIn ), fusRawAdc( usRawAdcIn ) {}
         BetaHit() : BetaHit( 0, 0) {}

         virtual ~BetaHit() {};

         void assign(const BetaHit& src) { fulLongTs = src.fulLongTs; fusRawAdc = src.fusRawAdc; }

         BetaHit& operator=(const BetaHit& src) { assign(src); return *this; }
         bool     operator<(const BetaHit& other) const { return fulLongTs < other.fulLongTs; }

         inline void reset() { fulLongTs = 0; fusRawAdc = 0; }

         inline uint64_t GetTs()  const { return fulLongTs; }
         inline uint16_t GetAdc() const { return fusRawAdc; }
   };
}
#endif // STSXYTERBETAHIT_H
