/** CbmStsDigi.cxx
 ** @author V.Friese <v.friese@gsi.de>
 ** @since 28.08.2006
 ** @version 4.0
 **/

#include "CbmStsDigi.h"
#include <sstream>
#include "CbmStsAddress.h"

using namespace std;

// --- String output
string CbmStsDigi::ToString() const {
   stringstream ss;
   ss << "StsDigi: address " << fAddress << " | channel " << fChannel
      << " | charge " << fCharge << " | time " << fTime;
   return ss.str();
}

ClassImp(CbmStsDigi)
