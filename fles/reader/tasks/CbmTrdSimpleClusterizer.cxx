#include "CbmTrdSimpleClusterizer.h"
#include "CbmTrdDigi.h"
#include "CbmTrdCluster.h"
#include "CbmTrdAddress.h"

CbmTrdSimpleClusterizer::CbmTrdSimpleClusterizer ()
    : fDigis (nullptr), fClusters (nullptr),
        CbmTrdQABase ("CbmTrdSimpleClusterizer")
{
}
;

InitStatus
CbmTrdSimpleClusterizer::Init ()
{
  LOG(DEBUG) << "Initilization of " << this->GetName () << FairLogger::endl;

  // Get a handle from the IO manager
  FairRootManager* ioman = FairRootManager::Instance ();

  // Get a pointer to the previous already existing data level
  fRaw = static_cast<TClonesArray*> (ioman->GetObject ("SpadicRawMessage"));
  if (!fRaw)
    {
      LOG(FATAL) << "No InputDataLevelName SpadicRawMessage array!\n"
                    << this->GetName () << " will be inactive"
                    << FairLogger::endl;
      return kERROR;
    }

  fDigis = static_cast<TClonesArray*> (ioman->GetObject ("TrdDigi"));
  if (!fDigis)
    {
      LOG(FATAL) << "No InputDataLevelName TrdDigi array!\n" << this->GetName ()
                    << " will be inactive" << FairLogger::endl;
      return kERROR;
    }
  fBT->SetDigisArray(fDigis);
  fClusters = new TClonesArray ("CbmTrdCluster", 100);
  ioman->Register ("TrdCluster", "TRD Clusters", fClusters,
                   IsOutputBranchPersistent ("TrdCluster"));

  // Do whatever else is needed at the initilization stage
  // Create histograms to be filled
  // initialize variables
  CbmTrdSimpleClusterizer::CreateHistograms ();

  return kSUCCESS;

}

void
CbmTrdSimpleClusterizer::CreateHistograms ()
{
  TString GraphName = "Clusters_Created";
  fHm->Add (GraphName.Data (), new TGraph);
  fHm->G1 (GraphName.Data ())->SetNameTitle (GraphName.Data (),
                                             GraphName.Data ());
  fHm->G1 (GraphName.Data ())->GetXaxis ()->SetTitle ("TimeSlice");
  fHm->G1 (GraphName.Data ())->GetYaxis ()->SetTitle ("Nr of created Clusters");
  for (Int_t layer = 0; layer < fBT->GetNrRobs(); layer++)
    {
      TString HistName = "Cluster_size_Distribution_Layer_"
          + std::to_string (layer);
      fHm->Add (
          HistName.Data (),
          new TH2I (HistName.Data (), HistName.Data (), NrOfActiveSpadics, -0.5,
                    NrOfActiveSpadics - 0.5, 16, -0.5, 15.5));
      fHm->H2 (HistName.Data ())->GetXaxis ()->SetTitle ("Spadic");
      fHm->H2 (HistName.Data ())->GetYaxis ()->SetTitle ("Cluster Size");
    }
  for (Int_t layer = 0; layer < fBT->GetNrRobs(); layer++)
    {
      TString HistName = "Cluster_Size_Type_Distribution_Layer_"
          + std::to_string (layer);
      fHm->Add (
          HistName.Data (),
          new TH2I (HistName.Data (), HistName.Data (), 16, -0.5, 15.5, 6, -0.5,
                    5.5));
      fHm->H2 (HistName.Data ())->GetYaxis ()->SetTitle ("Cluster Type");
      fHm->H2 (HistName.Data ())->GetXaxis ()->SetTitle ("Cluster Width");
    }

}
;

void
CbmTrdSimpleClusterizer::Exec (Option_t*)
{
  static Int_t NrTimeslice = 0;
  LOG(INFO) << this->GetName () << ": Clearing  Clusters" << FairLogger::endl;
  fClusters->Clear ("C");
  UInt_t nSpadicDigis = fDigis->GetEntriesFast (); //SPADIC messages per TimeSlic
  UInt_t NrClusters = 0;
  TString GraphName = "Clusters_Created";
  LOG(INFO) << this->GetName () << ": Reconstructing Clusters from "
               << nSpadicDigis << " Digis" << FairLogger::endl;
  CbmTrdCluster*CurrentCluster = nullptr;
  Bool_t newCluster = true;
  CbmTrdDigi *CurrentDigi = nullptr;
  for (UInt_t iSpadicDigi = 0; iSpadicDigi < nSpadicDigis; ++iSpadicDigi)
    {
      CbmTrdDigi *previousDigi = CurrentDigi;
      CurrentDigi = static_cast<CbmTrdDigi*> (fDigis->At (iSpadicDigi));
      if (!CurrentDigi)
        continue;
      //Fist check for Abort Conditions, done explicitly due to the high amount of state in this loop.
      //If newCluster is true, skip checks.
      //TODO: Check if Implementation as a explicit state machine is feasible.
      if (!newCluster)
        {
          Long64_t previousTime =
              static_cast<Long64_t> ((previousDigi->GetTime () + 1) / (1E3 / 16));
          Long64_t CurrentTime = static_cast<Long64_t> ((CurrentDigi->GetTime ()
              + 1) / (1E3 / 16));
          if (previousTime != CurrentTime)
            {
              //Time window is finished, create new Cluster.
              newCluster++;
            }
          Int_t PreviousLayer = CbmTrdAddress::GetLayerId (
              previousDigi->GetAddress ());
          Int_t CurrentLayer = CbmTrdAddress::GetLayerId (
              CurrentDigi->GetAddress ());
          Int_t PreviousModule = CbmTrdAddress::GetSectorId (
              previousDigi->GetAddress ());
          Int_t CurrentModule = CbmTrdAddress::GetSectorId (
              CurrentDigi->GetAddress ());
          Int_t PreviousRow = CbmTrdAddress::GetRowId (
              previousDigi->GetAddress ());
          Int_t CurrentRow = CbmTrdAddress::GetRowId (
              CurrentDigi->GetAddress ());
          if ((PreviousRow != CurrentRow) || (PreviousModule != CurrentModule)
          //||(PreviousSector!=CurrentSector)
              || (PreviousLayer != CurrentLayer))
            {
              //Layer, sector,module or Row is finished, create new Cluster.
              newCluster++;
            }
          Int_t PreviousCol = CbmTrdAddress::GetColumnId (
              previousDigi->GetAddress ());
          Int_t CurrentCol = CbmTrdAddress::GetColumnId (
              CurrentDigi->GetAddress ());
          if (CurrentCol - PreviousCol > 1)
            {
              //Large Gap between Clusters found, create new Cluster.
              newCluster++;
            }
        }
      //Done Checking for abort conditions, create new Cluster if necessary.
      if (newCluster)
        {
          if (CurrentCluster)
            {
              //	LOG(INFO)<<CurrentCluster->GetDigis().size() << " "/*<<CurrentCluster->GetDigi(0)*/<< FairLogger::endl;
              //TODO: Accurately set number of Columns.
              CurrentCluster->SetNCols (fBT->GetColumnWidth(CurrentCluster));
              CurrentCluster->SetNRows (fBT->GetRowWidth(CurrentCluster));
            }
          CurrentCluster = static_cast<CbmTrdCluster*>(fClusters->ConstructedAt(NrClusters++));
          CurrentCluster->SetDigis(std::vector<int>());
          newCluster = false;
        }
      //We can now add the Current Digi to the current Cluster.
      //TODO: Filter for Hit Type Pattern. Currently planned for V2, the advanced Clusterizer.
      CurrentCluster->AddDigi (iSpadicDigi);
      //Done with loop.
      //TODO: IMPORTANT: cluster tools for charge, displacement and so on.
    }
  fHm->G1 (GraphName.Data ())->SetPoint (fHm->G1 (GraphName.Data ())->GetN (),
                                         NrTimeslice++, NrClusters);
  LOG(INFO) << this->GetName () << ": Finished creating Clusters"
               << FairLogger::endl;
  LOG(INFO) << this->GetName () << ": Check size distribution of Clusters"
               << FairLogger::endl;
  std::vector<TH2*> Sizemaps;
  for (Int_t layer = 0; layer < fBT->GetNrRobs(); layer++)
    {
      TString HistName = "Cluster_size_Distribution_Layer_"
          + std::to_string (layer);
      Sizemaps.push_back (fHm->H2 (HistName.Data ()));
    }

  for (UInt_t iCluster = 0; iCluster < NrClusters; ++iCluster)
    {
      CurrentCluster = static_cast<CbmTrdCluster*> (fClusters->At (iCluster));
      if (CurrentCluster)
        {
          CbmTrdDigi*firstDigi = static_cast<CbmTrdDigi*> (fDigis->At (
              *(CurrentCluster->GetDigis ().begin ())));
          Int_t Layer = CbmTrdAddress::GetLayerId (firstDigi->GetAddress ());
          Int_t Col = CbmTrdAddress::GetColumnId (firstDigi->GetAddress ());
          Sizemaps.at (Layer)->Fill (Col / 16, CurrentCluster->GetNofDigis ());
        }
    }
  Sizemaps.clear();
  for (Int_t layer = 0; layer < fBT->GetNrRobs(); layer++)
    {
      TString HistName = "Cluster_Size_Type_Distribution_Layer_"
          + std::to_string (layer);
      Sizemaps.push_back (fHm->H2 (HistName.Data ()));
    }
  for (UInt_t iCluster = 0; iCluster < NrClusters; ++iCluster)
    {
      CurrentCluster = static_cast<CbmTrdCluster*> (fClusters->At (iCluster));
      if (CurrentCluster)
        {
          Int_t FirstIndex=
              (CurrentCluster->GetDigis().at(0));
          CbmTrdDigi*firstDigi = static_cast<CbmTrdDigi*> (fDigis->At (FirstIndex));
          Int_t Layer = CbmTrdAddress::GetLayerId (firstDigi->GetAddress ());
          Sizemaps.at(Layer)->Fill (fBT->GetColumnWidth(CurrentCluster),fBT->ClassifyCluster(CurrentCluster));
        }
    }

  LOG(INFO) << this->GetName ()
               << ": Done checking size distribution of Clusters"
               << FairLogger::endl;
}

ClassImp(CbmTrdSimpleClusterizer)
