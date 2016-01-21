/** CBM MAPS Hitproducer 
 Written by Michael Deveaux (m.deveaux@gsi.de)
 Acknowledgements to M. Al-Turany, D. Bertini, G. Gaycken
 
 Version beta 0.1 (02.02.2005)
*/ 

#include <TFile.h>
#include <TTree.h>
#include <TClonesArray.h>
#include <FairMCApplication.h>
#include <CbmStack.h>
#include <TFolder.h>
#include <CbmStsPoint.h>
#include <TParticle.h>
#include <iostream>
#include "CbmMapsFileManager.h"
#include "TROOT.h"
#include <iomanip>
#include <vector>


using std::cout;
using std::endl;
using std::map;
using std::setw;
using std::left;
using std::right;
using std::fixed;
using std::pair;
using std::setprecision;
using std::ios_base;
using std::vector;
   
CbmMapsFileManager::CbmMapsFileManager (TString fileName,TString branchName)
    :
    fInputFile(),
    fCbm(),
    fTree(),
    fFolder(),
    fListOfStsPoints(),
    fFrameworkFile(),
    fFrameworkDirectory(),
    fCurrentParticle(),
    fNumberOfCurrentEvent(),
    fEventsInFile(),
    fHitsInEvent(),
    fErrorCode()
{
  fFrameworkFile=gFile;
  fFrameworkDirectory=gDirectory;
  fInputFile=(TFile*) gROOT->FindObject(fileName);
  if (fInputFile==NULL)
      {
      fInputFile = new TFile(fileName);
      }
  if (fInputFile == NULL)
      {
      Fatal("CbmMapsFileManager: Error, could not open",fileName);
      }
  fCbm = (FairMCApplication*) fInputFile->Get("Cbm");
  fTree = (TTree*) fInputFile->Get("cbmsim"); 
  fFolder = (TFolder*) fInputFile->Get("cbmout");
  fListOfStsPoints = (TClonesArray*)fFolder->FindObjectAny(branchName);
  
  if (fListOfStsPoints == NULL)
      {
      Fatal("CbmMapsFileManager: Error, could not open branch",branchName);
      }
  fTree->SetBranchAddress(fListOfStsPoints->GetName(),&fListOfStsPoints);
  fEventsInFile=(UInt_t) fTree->GetEntries();
  if(fEventsInFile<1)
      {
      cout<< "Warning: No events in file" << fileName<< endl;
      }
  else
      {
       cout << fileName << " was opened successfully. " << endl << fEventsInFile;
       cout << " events available." << endl;
      }
  if(fFrameworkFile)
      {
	  gFile=fFrameworkFile;
      }
  if(fFrameworkDirectory)
     {
          gDirectory=fFrameworkDirectory;
     }
}

CbmMapsFileManager::CbmMapsFileManager (){};
 
CbmMapsFileManager::~CbmMapsFileManager () {

delete fFolder;
delete fTree;
delete fCbm;
fInputFile->Close();
delete fInputFile;

};

UInt_t CbmMapsFileManager::GetNumberOfEventsInFile() {return fEventsInFile;};
		
UInt_t CbmMapsFileManager::GetNumberOfHitsInEvent() {return fHitsInEvent;};
	
UInt_t CbmMapsFileManager::GetNumberOfCurrentEvent () {return fNumberOfCurrentEvent;};		
	
void CbmMapsFileManager::LoadEntryIntoRam (UInt_t entry) {

if (((entry>=fEventsInFile) || (entry<0))) 
   {cout << "CbmMapsFileManager::GetStsPoint: Error, event ";
    cout << entry << " doesn't exist." << endl;
    
    Fatal("CbmMapsFileManager::GetStsPoint"," Aborting");
   };
    
 fTree->GetEntry(entry);      
 fHitsInEvent=(UInt_t) fListOfStsPoints->GetEntries();
 
 };
 

TClonesArray* CbmMapsFileManager::GetEntry (UInt_t eventNumber){

// Reads Entry from disk (independant from framework)
 LoadEntryIntoRam (eventNumber);
 return fListOfStsPoints;

};

TClonesArray* CbmMapsFileManager::GetFrameworkEntry (){

// Takes Entry from framework (no independen disk access)
// May be used to reset TClonesArray for same event.

return fListOfStsPoints;

};

ClassImp(CbmMapsFileManager)
