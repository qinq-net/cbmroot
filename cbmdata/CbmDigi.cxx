/** @file CbmDigi.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 30.04.2013
 **/

#include "CbmDigi.h"

#include "FairMultiLinkedData.h"

#include <memory>

// -----   Default constructor   -------------------------------------------
CbmDigi::CbmDigi() 
 : TObject(), 
   fMatch(NULL) 
{
}
// -------------------------------------------------------------------------



// -----   Copy constructor   ----------------------------------------------
CbmDigi::CbmDigi(const CbmDigi& rhs) 
  : TObject(rhs),
    fMatch(NULL) 
{
} 
// -------------------------------------------------------------------------



// -----   Assignment operator   -------------------------------------------
CbmDigi& CbmDigi::operator=(const CbmDigi& rhs) 
{
  if (this != &rhs) {
    TObject::operator=(rhs);
    fMatch = NULL;
  }
  return *this;
}
// -------------------------------------------------------------------------




ClassImp(CbmDigi)
