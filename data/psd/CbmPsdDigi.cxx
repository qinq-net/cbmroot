/** CbmPsdDigi.cxx
 **@author Alla Maevskaya <alla@inr.ru>
 **@since 3.08.20212
 **
 ** Modified to simplify fEdep[10][44] -> fEdep (S. Seddiki)
  **/


#include "CbmPsdDigi.h"

#include <sstream>
#include "FairLogger.h"

using std::string;
using std::stringstream;


// -----   Default constructor   -------------------------------------------

CbmPsdDigi::CbmPsdDigi() 
  : CbmDigi(),
    fModuleID(-1),
    fSectionID(-1),
    fEdep(-1),
    fTime(0.)
// SELIM: simplification matrix [10][44] -> simple double
{
  /*
  for (Int_t i=0; i<10; i++)       // SELIM: simplification matrix [10][44] -> simple double
    for (Int_t j=0; j<44; j++)
      fEdep[i][j]=-1;
  */
}


CbmPsdDigi::CbmPsdDigi(Int_t section, Int_t module, Double_t edep, Double_t time)
  : CbmDigi(),
    fModuleID(module),
    fSectionID(section),
    fEdep(edep),
    fTime(time)
{
  /*
  for (Int_t i=0; i<10; i++)        // SELIM: simplification matrix [10][44] -> simple double
    for (Int_t j=0; j<44; j++)
      fEdep[i][j] = edep;
      */
}



// -----   Destructor   ----------------------------------------------------
CbmPsdDigi::~CbmPsdDigi() { }
// -------------------------------------------------------------------------


// -----   String output   -------------------------------------------------
string CbmPsdDigi::ToString() const {
   stringstream ss;
   ss << "PsdDigi: Module " << fModuleID << " | Section " << fSectionID
      << " | charge " << fEdep << " | time " << fTime;
   return ss.str();
}
// -------------------------------------------------------------------------




ClassImp(CbmPsdDigi)
