// ---------------------------------------------------------- --------------
// -----                  CbmMapsFileManager cxx file                  -----
// -----                       Created by M.Deveaux,                   -----
// -----                        Update by P.Sitzmann                   -----
// -------------------------------------------------------------------------
 
// Includes from CBM
#include "CbmMapsFileManager.h"

// Includes from FairRoot
#include "FairLogger.h"

// Includes from ROOT
#include "TROOT.h"

// Includes from C++

using namespace std;

// -------------------------------------------------------------------------
CbmMapsFileManager::CbmMapsFileManager ()
:   TObject(),
    fInputFile(),
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
 Fatal( "CbmMapsFileManager: Do not use the standard constructor","Wrong constructor");
};
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
CbmMapsFileManager::CbmMapsFileManager (TString fileName,TString branchName)
:   TObject(),
    fInputFile(),
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
	  LOG(FATAL)<<"CbmMapsFileManager: Error, could not open " << fileName << FairLogger::endl;
      }
  fTree = (TTree*) fInputFile->Get("cbmsim");
  fFolder = (TFolder*) fInputFile->Get("cbmout");
  fListOfStsPoints = (TClonesArray*)fFolder->FindObjectAny(branchName);
  
  if (fListOfStsPoints == NULL)
      {
	  LOG(FATAL) << "CbmMapsFileManager: Error, could not open branch " << branchName << FairLogger::endl;
      }
  fTree->SetBranchAddress(fListOfStsPoints->GetName(),&fListOfStsPoints);
  fEventsInFile=(UInt_t) fTree->GetEntries();
  if(fEventsInFile<1)
      {
	  LOG(WARNING)<< "Warning: No events in file" << fileName<< FairLogger::endl;
      }
  else
      {
	  LOG(INFO) << "CbmMapsFileManager: " << fileName << " was opened successfully. " << endl <<
	      fEventsInFile << " events available." << FairLogger::endl;
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
// -------------------------------------------------------------------------

// ------------------------------------------------------------------------- 
CbmMapsFileManager::~CbmMapsFileManager () {

delete fFolder;
delete fTree;
fInputFile->Close();
delete fInputFile;

};
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmMapsFileManager::LoadEntryIntoRam (UInt_t entry) {

if ( ( entry >= fEventsInFile ))
    LOG(FATAL) << "CbmMapsFileManager::GetStsPoint: Error, event " << entry
	<< " doesn't exist." << FairLogger::endl;
    
 fTree->GetEntry(entry);      
 fHitsInEvent=(UInt_t) fListOfStsPoints->GetEntries();
 
 };
// -------------------------------------------------------------------------

// ------------------------------------------------------------------------- 

TClonesArray* CbmMapsFileManager::GetEntry (UInt_t eventNumber){
 LoadEntryIntoRam (eventNumber);
 return fListOfStsPoints;

};
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
ClassImp(CbmMapsFileManager)
