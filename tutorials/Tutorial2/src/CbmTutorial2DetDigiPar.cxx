// -------------------------------------------------------------------------
// -----                CbmTutorial2DetDigiPar source file              -----
// -----                  Created 05/06/08  by F. Uhlig                -----
// -------------------------------------------------------------------------

#include "CbmTutorial2DetDigiPar.h"

#include "FairParamList.h"
#include "FairDetParIo.h"
#include "FairParIo.h"

#include "TString.h"
#include "TMath.h"

#include <iostream>

using std::cout;
using std::endl;

// -----   Standard constructor   ------------------------------------------
CbmTutorial2DetDigiPar::CbmTutorial2DetDigiPar(const char* name, 
					     const char* title,
					     const char* context)
  : FairParGenericSet(name, title, context),
    ftutdetdigipar(new TArrayF(10)),
    ftutdetdigiparsector(),
    ftutdetdigiparstation(-1)
{
  detName="Tutorial2Det";
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmTutorial2DetDigiPar::~CbmTutorial2DetDigiPar() {
  clear();
}
// -------------------------------------------------------------------------


// -----   Public method clear   -------------------------------------------
void CbmTutorial2DetDigiPar::clear() {
  status = kFALSE;
  resetInputVersions();
}
// -------------------------------------------------------------------------

// -----   Public method printParams ---------------------------------------
void CbmTutorial2DetDigiPar::printparams() {
  
  cout <<"Print"<<endl;
  cout << "ftutdetdigiparstation: " << ftutdetdigiparstation << endl;
  Int_t size =  ftutdetdigiparsector.GetSize();
  cout <<"size: " << size << endl;
  for ( Int_t i=0; i< size; i++) {
    cout << i <<" :" << ftutdetdigiparsector.GetAt(i) << endl;
  }
  size =  ftutdetdigipar->GetSize();
  cout <<"size: " << size << endl;
  for ( Int_t i=0; i< size; i++) {
    cout << i <<" :" << ftutdetdigipar->GetAt(i) << endl;
  }
    
}
// -------------------------------------------------------------------------

void CbmTutorial2DetDigiPar::putParams(FairParamList* l) {
  //   print();
  cout << " I am in CbmTutorial2DetDigiPar::putParams " << endl;
   if (!l) return;
   l->add("CbmTutorial2DetDigiStations", ftutdetdigiparstation);
   l->add("CbmTutorial2DetDigiSectorsPerStation", ftutdetdigiparsector);
   Int_t count_sectors = 0;
   for ( Int_t i=0; i< ftutdetdigiparstation; i++) {
     count_sectors+=ftutdetdigiparsector[i];
   }
   Int_t array_size = (count_sectors * 10 + ftutdetdigiparstation * 3);
   cout << "Array Size: " << array_size << endl;
   /*
   Float_t zwischen[array_size];
   for ( Int_t i=0; i< array_size; i++) {
     zwischen[i] = ftutdetdigipar.GetAt(i);
   }
   l->addObject("CbmTutorial2DetDigiPar",zwischen,array_size);
   */
   ftutdetdigipar->Set(array_size);
   l->add("CbmTutorial2DetDigiPar",*ftutdetdigipar);

}

//------------------------------------------------------

Bool_t CbmTutorial2DetDigiPar::getParams(FairParamList* l) {
  //print();
    cout << " I am in CbmTutorial2DetDigiPar::getParams " << endl;

    if (!l) return kFALSE;
    if ( ! l->fill("CbmTutorial2DetDigiStations", &ftutdetdigiparstation) ) return kFALSE;
    ftutdetdigiparsector.Set(ftutdetdigiparstation);

    if ( ! l->fill("CbmTutorial2DetDigiSectorsPerStation", &ftutdetdigiparsector) ) return kFALSE;

    Int_t count_sectors = 0;
    for ( Int_t i=0; i< ftutdetdigiparstation; i++) {
      count_sectors+=ftutdetdigiparsector[i];
    }
    cout << "Total number of sectors: " << count_sectors<<endl;

    Int_t array_size = (count_sectors * 10 + ftutdetdigiparstation * 3);
    cout << "Array Size: " << array_size << endl;

    ftutdetdigipar->Set(array_size);
    if (!(l->fill("CbmTutorial2DetDigiPar",ftutdetdigipar))) {
    cout << "Could not initialize CbmTutorial2DetDigiPar"
                 << endl;
    return kFALSE;
  }
 
  return kTRUE;
}



ClassImp(CbmTutorial2DetDigiPar)
