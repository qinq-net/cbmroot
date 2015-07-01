/** CBM MAPS Hitproducer 
 Written by Michael Deveaux (m.deveaux@gsi.de)
 Acknowledgements to M. Al-Turany, D. Bertini, G. Gaycken
 
 Version beta 0.1 (02.02.2005)
*/ 

#ifndef CbmMapsFILEMANAGER_H
#define CbmMapsFILEMANAGER_H

#include <TFile.h>
#include <TDirectory.h>
#include <TTree.h>
#include <TClonesArray.h>
#include <FairMCApplication.h>
#include <CbmStack.h>
#include <TFolder.h>
#include <CbmStsPoint.h>

#include <TParticle.h>

#include <iostream>

using namespace std;


class CbmMapsFileManager { 

protected:

TFile *fInputFile;  // InputFile, Output from Transport
FairMCApplication *fCbm; // Whatever it is good for ?!?
TTree* fTree; 
TFolder *fFolder;
TClonesArray *fListOfStsPoints;

TFile* fFrameworkFile;
TDirectory* fFrameworkDirectory;


Int_t fCurrentParticle;
Int_t fNumberOfCurrentEvent;
UInt_t fEventsInFile;
UInt_t fHitsInEvent;
UInt_t fErrorCode;

public: 

CbmMapsFileManager(TString fileName,TString branchName);
CbmMapsFileManager (); // Do not use that one!
virtual ~CbmMapsFileManager (); //destructor     

UInt_t GetNumberOfEventsInFile ();
UInt_t GetNumberOfCurrentEvent ();
UInt_t GetNumberOfHitsInEvent ();

/** Loads an event into the corresponding TClonesArray */
void LoadEntryIntoRam (UInt_t entry); 

/**Gives entry of the file */
TClonesArray* GetEntry(UInt_t entry);

/** Gives the current TClonesArray without reloading it from file */
TClonesArray* GetFrameworkEntry();

 ClassDef(CbmMapsFileManager,1)

};     



#endif
