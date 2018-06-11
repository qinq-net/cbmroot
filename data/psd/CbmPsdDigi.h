/** CbmPsdDigi.h
 **@author Alla Maevskaya <alla@inr.ru>
 **@31 October 2012
 **@version 1.0
 **
 ** Data class for digital PSD information
 **
 ** energy deposition in 1  channel for 6 scintillators 
 ** Ideal case - Edep in GeV, idel  detector response
 **
 ** The time is relative to the event time. 
 **
 ** Modified to simplify fEdep[10][44] -> fEdep (S. Seddiki)
 **/


#ifndef CBMPSDDIGI_H
#define CBMPSDDIGI_H 1


#include "CbmDigi.h"

#include <vector>
#include "CbmDefs.h"

class CbmPsdDigi : public CbmDigi
{

 public:

  /**   Default constructor   **/
  CbmPsdDigi() ;
  
  CbmPsdDigi(Int_t section, Int_t module, Double_t edep, Double_t time = 0.) ;
  
  
  /**   Destructor   **/
  virtual ~CbmPsdDigi();
     
  /**   Setter - Getter   **/
  
  //Float_t GetEdep(Int_t module, Int_t section) const { return fEdep[module][section]; }        
  //void SetEdep(Float_t edep, Int_t module, Int_t section) {fEdep[module][section]=edep;}

  Double_t GetEdep() const { return fEdep; }         // SELIM: simplification matrix [10][44] -> simple double 
  void SetEdep(Double_t edep) { fEdep = edep; }

  Int_t GetModuleID() const { return fModuleID; }
  void SetModuleID(Int_t mod) {  fModuleID = mod; }

  Int_t GetSectionID() const { return fSectionID; }
  void SetSectionID(Int_t sec) {  fSectionID = sec; }


  /** @Address
   ** @value Unique address
   **
   ** TODO: Since there is no addressing scheme for the PSD, for the time being
   ** kPsd is returned.
   **/
  virtual Int_t GetAddress() const { return kPsd; }


  /** @brief System ID (PSD)
   ** @value kPsd
   **/
  virtual Int_t GetSystemId() const { return kPsd; };


  /** @brief Time
   ** @value Time of measurement [ns]
   **/
  virtual Double_t GetTime() const { return fTime; }

  
  /** @brief Info to string **/
  virtual std::string ToString() const;


 private:


  /**   Data members  **/
 
  Int_t fModuleID;
  Int_t fSectionID;
  Double_t fEdep;//[10][44];       // SELIM: simplification matrix [10][44] -> simple double 
  Double_t fTime;

  
  ClassDef(CbmPsdDigi, 2);

};


#endif
