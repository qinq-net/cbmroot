// ------------------------------------------------------
// -----          CbmD0HistogramManager class file  -----
// -----          Created 09.11.2015 by P.Sitzmann  -----
// ------------------------------------------------------
#include "CbmD0HistogramManager.h"

// Includes from KF


// Includes from Cbm
#include "CbmMCTrack.h"
#include "CbmMvdPoint.h"

// Includes from Fair
#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairMCPoint.h"

// Includes from Root
#include "TClonesArray.h"

#include <stdio.h>
using namespace std;

// -------------------------------------------------------------------------
CbmD0HistogramManager::CbmD0HistogramManager()
    :CbmHistManager(),
    fChois(ALL),
    fcutPT(),
    fcutSvZ(),
    fcutPZ(),
    fListMCTracks(),
    fListMCPointsMvd(),
    fListMCPointsSts(),
    fListMCPointsRich(),
    fListMCPointsTrd(),
    fListMCPointsTof(),
    fwriteOutFile()
{

}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
CbmD0HistogramManager::CbmD0HistogramManager(const char* group,Float_t PTCut,
					     Float_t SvZCut, Float_t PZCut)
    :CbmHistManager(),
    fChois(),
    fcutPT(PTCut),
    fcutSvZ(SvZCut),
    fcutPZ(PZCut),
    fListMCTracks(),
    fListMCPointsMvd(),
    fListMCPointsSts(),
    fListMCPointsRich(),
    fListMCPointsTrd(),
    fListMCPointsTof(),
    fwriteOutFile()
{
SetHistogramChois(group);
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
CbmD0HistogramManager::~CbmD0HistogramManager()
{
;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmD0HistogramManager::SetHistogramChois(const char* group)
{
    if(group == "MCQA") fChois = MCQA;
    else
        fChois = MCQA;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmD0HistogramManager::SetCuts(Float_t PTCut, Float_t SvZCut, Float_t PZCut)
{
    fcutPT = PTCut;
    fcutSvZ = SvZCut;
    fcutPZ = PZCut;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmD0HistogramManager::Init()
{
    if(fChois == SINGELTRACK) InitSingel();
    if(fChois == PAIR) InitPair();
    if(fChois == MCQA) InitMc();
    if(fChois == ALL) InitAll();
    return;
}
// -------------------------------------------------------------------------
void CbmD0HistogramManager::InitSingel()
{
;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmD0HistogramManager::InitPair()
{
;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmD0HistogramManager::InitMc()
{
    /*  Create1<TH1F>("name", "title", 100, 0, 100)     */

    Create1<TH1F>("MCPointMomentumDist", "Momentumdistribution MC Tracks", 100, 0, 30);        // all mc points

    Create1<TH1F>("MCPointMomentumDistMvd", "Momentumdistribution MC Track in MVD acceptance", 100 , 0, 30);   // MVD-MCPoints > 0
    Create1<TH1F>("MCPointMomentumDistMvdTrackable", "Momentumdistribution MC Track in MVD trackable", 100 , 0, 30);   // MVD-Points >= 3
    Create1<TH1F>("MCPointMomentumDistMvdStsTrackable", "Momentumdistribution MC Track in MVD and STS trackable", 100 , 0, 30);          // MVD + STS MC Points >= 3 but MVD MC Points < 3 & STS MC Points < 3
    Create1<TH1F>("MCPointMomentumDistStsTrackMvdVertex", "Momentumdistribution MC Track in STS trackable with Vertex information from MVD", 100 , 0, 30); // STS MC Points >= 3 MVD MC Points >= 2
   /*
    Create1<TH1F>("MCPointMomentumDistMvdStsRich", "Momentumdistribution MC Points in MVD, STS, RICH acceptance", 100 , 0, 30);  // trackable + Rich MC Point
    Create1<TH1F>("MCPointMomentumDistMvdStsRichTrd", "Momentumdistribution MC Points in MVD, STS, RICH, TRD acceptance", 100 , 0, 30);   // + TRD Point
    Create1<TH1F>("MCPointMomentumDistMvdStsRichTrdTof", "Momentumdistribution MC Points in full acceptance", 100 , 0, 30);      // + Tof Point
  
    Create1<TH1F>("MCSignalMomentumDist", "Momentumdistribution of embedted Signal", 100, 0, 15);
    */
    Create1<TH1F>("MCSignalMomentumDistTrackableKaon", "Momentumdistribution of embedted Signal Trackable Kaon", 100, 0, 15);
    Create1<TH1F>("MCSignalMomentumDistTrackableKaonPID", "Momentumdistribution of embedted Signal Trackable Kaon and PID", 100, 0, 15);

    Create1<TH1F>("MCSignalMomentumDistTrackablePion1", "Momentumdistribution of embedted Signal Trackable Pion 1", 100, 0, 15);
    Create1<TH1F>("MCSignalMomentumDistTrackablePion1PID", "Momentumdistribution of embedted Signal Trackable Pion 1 and PID", 100, 0, 15);
    /*
    Create1<TH1F>("MCSignalMomentumDistTrackablePion2", "Momentumdistribution of embedted Signal Trackable Pion 2", 100, 0, 15);
    Create1<TH1F>("MCSignalMomentumDistTrackablePion2PID", "Momentumdistribution of embedted Signal Trackable Pion 2 and PID", 100, 0, 15);

    Create1<TH1F>("MCSignalMomentumDistTrackablePion3", "Momentumdistribution of embedted Signal Trackable Pion 3", 100, 0, 15);
    Create1<TH1F>("MCSignalMomentumDistTrackablePion3PID", "Momentumdistribution of embedted Signal Trackable Pion 3 and PID", 100, 0, 15);
              */
    Create1<TH1F>("MCSignalMomentumDistTrackable", "Momentumdistribution of embedted Signal trackable ", 100, 0, 15);
    Create1<TH1F>("MCSignalMomentumDistTrackablePID", "Momentumdistribution of embedted Signal trackable and PID", 100, 0, 15);

    TTree* mcTree = (TTree*)fMCFile->Get("cbmsim");
    fnrMcEvents = mcTree->GetEntries();

    fListMCTracks = new TClonesArray("CbmMCTrack", 1000);
    fmcTrackBranch = mcTree->GetBranch("MCTrack");
    fmcTrackBranch->SetAddress( &fListMCTracks);

   // fListMCPointsMvd = new TClonesArray("CbmMvdPoint", 1000);
   // fmvdPointBranch = mcTree->GetBranch("MvdPoint");
   // fmcTrackBranch->SetAddress( &fListMCPointsMvd);

   // fListMCPointsSts = new TClonesArray("CbmMCPoint", 1000);
   // fStsPointBranch = mcTree->GetBranch("StsPoint");
   // fmcTrackBranch->SetAddress( &fListMCPointsSts);


    //fListMCPointsRich   = (TClonesArray*) ioman->GetObject("RichPoint");
    //fListMCPointsTrd    = (TClonesArray*) ioman->GetObject("TrdPoint");
    //fListMCPointsTof    = (TClonesArray*) ioman->GetObject("TofPoint");
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmD0HistogramManager::InitAll()
{
;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmD0HistogramManager::Register()
{
;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmD0HistogramManager::Exec(Option_t* option = "")
{
    if(fChois == SINGELTRACK) ExecSingel();
    if(fChois == PAIR) ExecPair();
    if(fChois == MCQA) ExecMc();
    if(fChois == ALL) ExecAll();

    fwriteOutFile->cd();
    WriteToFile();
    return;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmD0HistogramManager::ExecMc()
{
    LOG(INFO) << "Starting MC Branch of CbmD0HistogramManager" << FairLogger::endl;

    TH1* mcMomentum = H1("MCPointMomentumDist");
    TH1* mcMomentumMvd = H1("MCPointMomentumDistMvd");
    TH1* mcMomentumMvdTrack = H1("MCPointMomentumDistMvdTrackable");
    TH1* mcMomentumMvdStsTrack = H1("MCPointMomentumDistMvdStsTrackable");
    TH1* mcMomentumStsTrackMvdVertex = H1("MCPointMomentumDistStsTrackMvdVertex");
    TH1* mcMomentumKaonTrackable = H1("MCSignalMomentumDistTrackableKaon");
    TH1* mcMomentumKaonTrackPID = H1("MCSignalMomentumDistTrackableKaonPID");
    TH1* mcMomentumPion1Trackable = H1("MCSignalMomentumDistTrackablePion1");
    TH1* mcMomentumPion1TrackPID = H1("MCSignalMomentumDistTrackablePion1PID");
    TH1* mcMomentumSignal = H1("MCSignalMomentumDistTrackable");
    TH1* mcMomentumSignalPID = H1("MCSignalMomentumDistTrackablePID");

    CbmMCTrack* mcTrack;
    CbmMCTrack* kaon;
    CbmMCTrack* pion1;
    TLorentzVector kaonLV, pion1LV, signalLV;

    for(Int_t iMcEvents = 0; iMcEvents < fnrMcEvents; iMcEvents++)
    {
	kaon = NULL;
	pion1 = NULL;

    if(iMcEvents%100 == 0)cout << endl << "Processing event " << iMcEvents << endl;
	fmcTrackBranch->GetEntry(iMcEvents);
     	int nrOfMCTracks = fListMCTracks->GetEntriesFast();

    for(int i = 0; i < nrOfMCTracks; i++)
       {
       mcTrack = (CbmMCTrack*)fListMCTracks->At(i);

       if(mcTrack->GetMotherId() == -1)
       {
       mcMomentum->Fill(mcTrack->GetP());

       if(mcTrack->GetNPoints(kMVD) > 0)
         {
	   mcMomentumMvd->Fill(mcTrack->GetP());
	   if(mcTrack->GetNPoints(kMVD)>=3)
	   {
               mcMomentumMvdTrack->Fill(mcTrack->GetP());
	   }
	   if(mcTrack->GetNPoints(kMVD) + mcTrack->GetNPoints(kSTS) >= 3)
	      {
		  mcMomentumMvdStsTrack->Fill(mcTrack->GetP());

		  if(mcTrack->GetPdgCode() == -321 && mcTrack->GetStartZ() != 0)  // Kaons from signal Trackable
		      {
		          kaon = (CbmMCTrack*)fListMCTracks->At(i);
			  mcMomentumKaonTrackable->Fill(mcTrack->GetP());
			  if(mcTrack->GetNPoints(kTOF)>0)  // Kaons from signal Trackable and TOF Pid
			  {
			    mcMomentumKaonTrackPID->Fill(mcTrack->GetP());
			  }
		      }

                  if(mcTrack->GetPdgCode() == 211 && mcTrack->GetStartZ() != 0 )  // Pion+ from signal Trackable
		      {
		          pion1 =  (CbmMCTrack*)fListMCTracks->At(i);
			  mcMomentumPion1Trackable->Fill(mcTrack->GetP());
			  if(mcTrack->GetNPoints(kTOF)>0)  // Pion+ from signal Trackable and TOF Pid
			  {
			      mcMomentumPion1TrackPID->Fill(mcTrack->GetP());
                          }
                      }

	      }
	   if( mcTrack->GetNPoints(kSTS) >= 3 && mcTrack->GetNPoints(kMVD) > 1 )
	      {
              mcMomentumStsTrackMvdVertex->Fill(mcTrack->GetP());
	      }

         }
       }
       }
    if (pion1 != NULL && kaon != NULL)
    {
        kaon->Get4Momentum(kaonLV);
        pion1->Get4Momentum(pion1LV);
        signalLV = kaonLV + pion1LV;
	mcMomentumSignal->Fill(signalLV.P());
	if(kaon->GetNPoints(kTOF)>0 && pion1->GetNPoints(kTOF)>0)
            mcMomentumSignalPID->Fill(signalLV.P());
    }
    
    }
}
// -------------------------------------------------------------------------































