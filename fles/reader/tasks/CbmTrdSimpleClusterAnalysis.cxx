/*
 * CbmTrdsimpleClusterAnalysis.cpp
 *
 *  Created on: Sep 4, 2017
 *      Author: philipp
 */

#include <CbmTrdSimpleClusterAnalysis.h>
#include <iostream>

ClassImp(CbmTrdSimpleClusterAnalysis)

CbmTrdSimpleClusterAnalysis::CbmTrdSimpleClusterAnalysis ():
CbmTrdQABase("CbmTrdSimpleClusterAnalysis")
{
  // TODO Auto-generated constructor stub

}

CbmTrdSimpleClusterAnalysis::~CbmTrdSimpleClusterAnalysis ()
{
  // TODO Auto-generated destructor stub
}

void
CbmTrdSimpleClusterAnalysis::CreateHistograms ()
{
  for (int layer = 0; layer < fBT->GetNrLayers (); layer++)
    {
      TString HistName = "Cluster_Heatmap_Layer_" + std::to_string (layer);
      fHm->Add (
	  HistName.Data (),
	  new TH2I (
	      HistName.Data (), HistName.Data (),
	      fBT->GetNrColumns (layer) * 4, -0.5 * fBT->GetPadWidth (layer),
	      (fBT->GetNrColumns (layer) - 0.5) * (fBT->GetPadWidth (layer)),
	      fBT->GetNrRows (layer) * 1, -0.5 * fBT->GetPadHeight (layer),
	      (fBT->GetNrRows (layer) - 0.5) * fBT->GetPadHeight (layer)));
      fHm->H2 (HistName.Data ())->GetXaxis ()->SetTitle ("X/cm");
      fHm->H2 (HistName.Data ())->GetYaxis ()->SetTitle ("Y/cm");
    }
  for (Int_t Layer = 0; Layer < fBT->GetNrLayers(); Layer++)
    {
	{
	  TString HistName = "Spectrum_Layer_" + std::to_string (Layer);
	  fHm->Add (
	      HistName.Data (),
	      new TH1F (HistName.Data (), HistName.Data (), 1000, -0.5, 999.5));
	  fSpectra.push_back (fHm->H1 (HistName.Data ()));
	  fSpectra.back ()->GetXaxis ()->SetTitle ("Charge/ADCValues");
	}
    }
}
void
CbmTrdSimpleClusterAnalysis::Exec (Option_t*)
{  std::vector<TH2*>Heatmaps;
for (Int_t layer = 0; layer < fBT->GetNrLayers(); layer++)
  {
    TString HistName = "Cluster_Heatmap_Layer_"
        + std::to_string (layer);
    Heatmaps.push_back (fHm->H2 (HistName.Data ()));
  }

  UInt_t NrClusters=fClusters->GetEntriesFast();
  for (UInt_t iCluster = 0; iCluster < NrClusters; ++iCluster)
    {
      CbmTrdCluster* CurrentCluster =
	  static_cast<CbmTrdCluster*> (fClusters->At (iCluster));
      if (CurrentCluster)
	{
	  if (fBT->ClassifyCluster (CurrentCluster)
	      != CbmTrdTestBeamTools::CbmTrdClusterClassification::kNormal)
	    continue;
	  Int_t FirstIndex = (CurrentCluster->GetDigis ().at (0));
	  CbmTrdDigi*firstDigi = static_cast<CbmTrdDigi*> (fDigis->At (
	      FirstIndex));
	  Int_t Layer = CbmTrdAddress::GetLayerId (firstDigi->GetAddress ());
	  double displacement = fBT->GetColumnDisplacement (CurrentCluster);
	  if (std::abs (displacement) > 1)
	    continue;
	  Double_t xPos = fBT->GetPadWidth (Layer)
	      * (fBT->GetCentralColumnID (CurrentCluster) + displacement);
	  displacement = fBT->GetRowDisplacement (CurrentCluster);
	  /*if(std::abs(displacement)>1)
	   continue;*/
	  Double_t yPos = fBT->GetPadHeight (Layer)
	      * (fBT->GetCentralRowID (CurrentCluster) + displacement);
	  Heatmaps.at (Layer)->Fill (xPos, yPos);

	    {
	      TString HistName = "Spectrum_Layer_"
		  + std::to_string (fBT->GetLayerID (CurrentCluster));
	      fHm->H1 (HistName.Data ())->Fill (
		  fBT->GetCharge (CurrentCluster));
	    }
	}
      /*LOG(INFO) << this->GetName () << ": " << fBT->GetCharge (CurrentCluster)
	<< FairLogger::endl;
      */
  }

}

