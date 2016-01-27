// -------------------------------------------------------------------------
// -----                  CbmMapsFileManager header file               -----
// -----                       Created by M.Deveaux,                   -----
// -----                        Update by P.Sitzmann                   -----
// -------------------------------------------------------------------------
 

#ifndef CbmMapsFILEMANAGER_H
#define CbmMapsFILEMANAGER_H

#include "TClonesArray.h"
#include "TFile.h"
#include "TTree.h"
#include "TFolder.h"
#include "TDirectory.h"
#include "TString.h"


class CbmMapsFileManager : public TObject
{
public:
CbmMapsFileManager(TString fileName,TString branchName);
CbmMapsFileManager (); // Do not use that one!

virtual ~CbmMapsFileManager (); //destructor     

UInt_t GetNumberOfEventsInFile () {return fEventsInFile;};;
UInt_t GetNumberOfCurrentEvent (){return fNumberOfCurrentEvent;};
UInt_t GetNumberOfHitsInEvent () {return fHitsInEvent;};

/** Loads an event into the corresponding TClonesArray */
void LoadEntryIntoRam (UInt_t entry); 

/**Gives entry of the file */
TClonesArray* GetEntry(UInt_t entry);

/** Gives the current TClonesArray without reloading it from file */
TClonesArray* GetFrameworkEntry(){return fListOfStsPoints;};

private:

TFile *fInputFile;  // InputFile, Output from Transport
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


CbmMapsFileManager(const CbmMapsFileManager&);
CbmMapsFileManager& operator=(const CbmMapsFileManager&);

 ClassDef(CbmMapsFileManager,1)

};     



#endif
