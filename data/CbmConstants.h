#ifndef CBM_CONSTANTS
#define CBM_CONSTANTS

/**
 * Header file which is used to put all needed constants at one place. Instead of defining 
 * constants all over the code the constants are defined only in this header. When such a
 * constant is needed in the code the header file should be included and the appropriate
 * function to get the correct value should be used. 
 *
 * All masses are defined in GeV.
 * 
 * 
 */


#include "Rtypes.h"

namespace Cbm {

  // Masses are tacken from the particle data book
  inline Double_t ProtonMass()   { return 0.938272046;} 
  inline Double_t NeutronMass()  { return 0.939565379;}
  inline Double_t ElectronMass() { return 0.000510998928;}

}


#endif
