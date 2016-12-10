/** @file CbmDigi.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 30.04.2013
 **/

#include "CbmDigi.h"
#include "CbmMatch.h"

#include "FairMultiLinkedData.h"

#include <memory>
#include <utility> // std::forward

// -----   Default constructor   -------------------------------------------
CbmDigi::CbmDigi() 
 : TObject(), 
   fMatch(NULL) 
{
}
// -------------------------------------------------------------------------



// -----   Copy constructor (deep copy)   ----------------------------------
CbmDigi::CbmDigi(const CbmDigi& rhs) 
  : TObject(rhs),
    fMatch(NULL)
{
  if ( rhs.fMatch ) {
		fMatch = new CbmMatch();
		fMatch->AddLinks( *(rhs.fMatch) );
	}
} 
// -------------------------------------------------------------------------

// -----   Move constructor  ----------------------------------------
CbmDigi::CbmDigi(CbmDigi&& other)
  : TObject(std::forward<CbmDigi>(other)), // should be std::forward
    fMatch(nullptr)
{
	fMatch = other.fMatch;
	other.fMatch = nullptr;
}
// -------------------------------------------------------------------------

// -----   Assignment operator (deep copy)  --------------------------------
CbmDigi& CbmDigi::operator=(const CbmDigi& rhs) 
{
  if (this != &rhs) {
    TObject::operator=(rhs);
    if ( rhs.fMatch ) {
  		fMatch = new CbmMatch();
  		fMatch->AddLinks( *(rhs.fMatch) );
    }
    else fMatch = NULL;
  }
  return *this;
}
// -------------------------------------------------------------------------

// -----   Assignment operator (deep copy)  --------------------------------
CbmDigi& CbmDigi::operator=(CbmDigi&& other)
{
  if (this != &other) {
	TObject::operator=(std::forward<CbmDigi>(other));
	// Free the existing resource
	delete fMatch;
	fMatch = other.fMatch;
	other.fMatch = nullptr;
  }
  return *this;
}
// -------------------------------------------------------------------------

// -----   Destructor   ----------------------------------------------------
CbmDigi::~CbmDigi() {
  if ( fMatch) delete fMatch;
}
// -------------------------------------------------------------------------

// The following functions are only implemented for the unit tests.
// They can only be called from a derived class via CbmDigi::GetAddress()
Int_t CbmDigi::GetAddress() const
{
  return -111;
}

Int_t CbmDigi::GetSystemId() const
{
  return -111;
}

Double_t CbmDigi::GetTime() const
{
  return -111.;
}

ClassImp(CbmDigi)
