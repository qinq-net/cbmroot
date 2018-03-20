/** CbmPsdSimpleDigitize.h
 *@author Sergey Morozov <morozovs@inr.ru>
 **
 ** The very simple digitizer produces digits of type CbmPsdDigi as sum of Edep
 **for all scintilators and simulates SiPM pixel statistics and electronics noise.
 **/


#ifndef CBMPSDSimpleDigitizer_H
#define CBMPSDSimpleDigitizer_H 1


#include "FairTask.h"

#include <TRandom3.h>

#define N_PSD_MODS 44
#define N_PSD_SECT 10

class TClonesArray;

class CbmPsdSimpleDigitizer : public FairTask
{

 public:

  /** Default constructor **/  
  CbmPsdSimpleDigitizer();


  /** Destructor **/
  ~CbmPsdSimpleDigitizer();


  /** Virtual method Init **/
  virtual InitStatus Init();


  /** Virtual method Exec **/
  virtual void Exec(Option_t* opt);


 private:
  Int_t             fNDigis;
  TRandom3*         fRandom3;

  /** Input array of CbmPsdPoints **/
  TClonesArray* fPointArray;

  /** Output array of CbmPsdDigits **/
  TClonesArray* fDigiArray; 

  CbmPsdSimpleDigitizer(const CbmPsdSimpleDigitizer&);
  CbmPsdSimpleDigitizer operator=(const CbmPsdSimpleDigitizer&);
  /** Reset eventwise counters **/
  void Reset();

  ClassDef(CbmPsdSimpleDigitizer,1);

};

#endif
