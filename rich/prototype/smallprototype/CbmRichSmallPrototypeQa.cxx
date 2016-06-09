
#include "CbmRichSmallPrototypeQa.h"

#include "TH1D.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TClonesArray.h"
#include "TF1.h"
#include "TStyle.h"

#include "CbmMCTrack.h"
#include "FairTrackParam.h"
#include "CbmRichHit.h"
#include "FairMCPoint.h"
#include "CbmDrawHist.h"
#include "CbmTrackMatchNew.h"
#include "CbmRichRing.h"
#include "CbmRichHit.h"
#include "CbmMatchRecoToMC.h"
#include "CbmRichGeoManager.h"
#include "CbmRichPoint.h"
#include "utils/CbmRichDraw.h"
#include "CbmStsPoint.h"
#include "CbmStsHit.h"
#include "CbmTrdPoint.h"
#include "CbmTrdHit.h"
#include "../../../data/tof/CbmTofPoint.h"
#include "../../../data/tof/CbmTofHit.h"

#include "CbmUtils.h"
#include "CbmHistManager.h"

#include <iostream>
#include <string>
#include <boost/assign/list_of.hpp>
#include <sstream>

using namespace std;
using boost::assign::list_of;

CbmRichSmallPrototypeQa::CbmRichSmallPrototypeQa()
: FairTask("CbmRichSmallPrototypeQa"),
fHM(NULL),
fOutputDir(""),
fRichHits(NULL),
fCanvas(),
fEventNum(0)
//fMinNofHits(7),
//fNofHitsInRingMap()
{
}


InitStatus CbmRichSmallPrototypeQa::Init()
{
	



    cout << "CbmRichSmallPrototypeQa::Init"<<endl;
    FairRootManager* ioman = FairRootManager::Instance();
    if (NULL == ioman) { Fatal("CbmRichSmallPrototypeQa::Init","RootManager not instantised!"); }
    
    
	fMCTracks = (TClonesArray*) ioman->GetObject("MCTrack");
	if (NULL == fMCTracks) { Fatal("CbmRichSmallPrototypeQa::Init", "No MC Tracks!"); }


	fSTSPoints =(TClonesArray*) ioman->GetObject("StsPoint");
	if (NULL == fSTSPoints) { Fatal("CbmRichSmallPrototypeQa::Init", "No STS Points!");}

	fSTSDigis =(TClonesArray*) ioman->GetObject("StsDigi");
	if (NULL == fSTSDigis) { Fatal("CbmRichSmallPrototypeQa::Init", "No STS Digis!");}
    
	fSTSHits =(TClonesArray*) ioman->GetObject("StsHit");
	if (NULL == fSTSHits) { Fatal("CbmRichSmallPrototypeQa::Init", "No STS Hits!");}

	fSTSTrack =(TClonesArray*) ioman->GetObject("StsTrack");
	if (NULL == fSTSTrack) { Fatal("CbmRichSmallPrototypeQa::Init", "No STS Track!");}


	fRichPoints =(TClonesArray*) ioman->GetObject("RichPoint");
	if (NULL == fRichPoints) { Fatal("CbmRichSmallPrototypeQa::Init", "No Rich Points!");}

	fRichDigis =(TClonesArray*) ioman->GetObject("RichDigi");
	if (NULL == fRichDigis) { Fatal("CbmRichSmallPrototypeQa::Init", "No Rich Digis!");}

	fRichHits = (TClonesArray*) ioman->GetObject("RichHit");
    if ( NULL == fRichHits) { Fatal("CbmRichSmallPrototypeQa::Init","No RichHits!"); }

	fRichRings = (TClonesArray*) ioman->GetObject("RichRing");
    if ( NULL == fRichRings) { Fatal("CbmRichSmallPrototypeQa::Init","No RichRings!"); }


	fTRDPoints = (TClonesArray*) ioman->GetObject("TrdPoint");
    if ( NULL == fTRDPoints) { Fatal("CbmRichSmallPrototypeQa::Init","No TRD Points!"); }

	fTRDHits = (TClonesArray*) ioman->GetObject("TrdHit");
    if ( NULL == fTRDHits) { Fatal("CbmRichSmallPrototypeQa::Init","No TRD Hits!"); }

	fTRDTrack = (TClonesArray*) ioman->GetObject("TrdTrack");
    if ( NULL == fTRDTrack) { Fatal("CbmRichSmallPrototypeQa::Init","No TRD Tracks!"); }


	fToFPoints = (TClonesArray*) ioman->GetObject("TofPoint");
	if ( NULL == fToFPoints) { Fatal("CbmRichSmallPrototypeQa::Init","No ToF Points");}

	fToFHits = (TClonesArray*) ioman->GetObject("TofHit");
	if ( NULL == fToFHits) { Fatal("CbmRichSmallPrototypeQa::Init","No ToF Hits");}

	fToFTrack = (TClonesArray*) ioman->GetObject("TofTrack");
	if ( NULL == fToFPoints) { Fatal("CbmRichSmallPrototypeQa::Init","No ToF Track");}

    InitHistograms();

	
    
    return kSUCCESS;
}

void CbmRichSmallPrototypeQa::Exec(
                            Option_t* option)
{

    fEventNum++;
    
    cout << "CbmRichSmallPrototypeQa, event No. " <<  fEventNum << endl;

	Int_t nofMCTracks = fMCTracks->GetEntriesFast();


	Int_t nofSTSPoints = fSTSPoints->GetEntriesFast();
	Int_t nofSTSDigis = fSTSDigis->GetEntriesFast();
	Int_t nofSTSHits = fSTSHits->GetEntriesFast();
	Int_t nofSTSTrack = fSTSTrack->GetEntriesFast();

	Int_t nofRichPoints = fRichPoints->GetEntriesFast();
	Int_t nofRichDigis = fRichDigis->GetEntriesFast();
	Int_t nofRichHits = fRichHits->GetEntriesFast();	
	Int_t nofRichRings = fRichRings->GetEntriesFast();

	Int_t nofTRDPoints = fTRDPoints->GetEntriesFast();
	Int_t nofTRDHits = fTRDHits->GetEntriesFast();
	Int_t nofTRDTrack = fTRDTrack->GetEntriesFast(); 

	Int_t nofToFPoints = fToFPoints->GetEntriesFast();
	Int_t nofToFHits = fToFHits->GetEntriesFast();
	Int_t nofToFTrack = fToFTrack->GetEntriesFast();  


	fHM->H1("fh_nof_mc_tracks")->Fill(nofMCTracks);

	fHM->H1("fh_nof_sts_points")->Fill(nofSTSPoints);
    fHM->H1("fh_nof_sts_digis")->Fill(nofSTSDigis);
	fHM->H1("fh_nof_sts_hits")->Fill(nofSTSHits);
	fHM->H1("fh_nof_sts_track")->Fill(nofSTSTrack);

	fHM->H1("fh_nof_rich_points")->Fill(nofRichPoints);
	fHM->H1("fh_nof_rich_digis")->Fill(nofRichDigis);
    fHM->H1("fh_nof_rich_hits")->Fill(nofRichHits); 
	fHM->H1("fh_nof_rich_rings")->Fill(nofRichRings);

	fHM->H1("fh_nof_trd_points")->Fill(nofTRDPoints);
	fHM->H1("fh_nof_trd_hits")->Fill(nofTRDHits);
	fHM->H1("fh_nof_trd_track")->Fill(nofTRDTrack);

	fHM->H1("fh_nof_tof_points")->Fill(nofToFPoints);
	fHM->H1("fh_nof_tof_hits")->Fill(nofToFHits);
	fHM->H1("fh_nof_tof_track")->Fill(nofToFTrack);


	for (int i=0; i<nofRichHits; i++)
	{ 
	 CbmRichHit* richHit= (CbmRichHit*) (fRichHits->At(i));
	 //CbmRichHit* richHit = static_cast<CbmRichHit*>(fRichHits -> At(i));
        }
	for (int i=0; i<nofMCTracks; i++)
	{
		CbmMCTrack* mctrack= (CbmMCTrack*) (fMCTracks->At(i));
		Double_t momentum = mctrack->GetP();
		fHM->H1("fh_mc_mom_all")->Fill(momentum);
		
		Double_t pdg = mctrack->GetPdgCode();
		fHM->H1("fh_mc_pdg_all")->Fill(pdg);

	
		
		if(pdg == 211)
		{
			fHM->H1("fh_mc_mom_pion")->Fill(momentum);
		}
		else if(pdg == -11)
		{
			fHM->H1("fh_mc_mom_elec")->Fill(momentum);
		}
		
		Double_t startx = mctrack->GetStartX();
		Double_t starty = mctrack->GetStartY();
		fHM->H2("fh_mc_startxy")->Fill(startx, starty);
		Double_t startz = mctrack->GetStartZ();
		fHM->H1("fh_mc_startz")->Fill(startz);


		Double_t motherid;
		CbmMCTrack* gamma= (CbmMCTrack*) (fMCTracks->At(i));
		Double_t particles = gamma->GetPdgCode();
		if(particles == 11)
		{
			motherid = gamma->GetMotherId();

			if(motherid >= 0)
			{
				CbmMCTrack* gamma1= (CbmMCTrack*) (fMCTracks->At(motherid));
			
				if(gamma1!=0)
				{	
					Double_t gammapdg = gamma1->GetPdgCode();
				
					if(gammapdg == 22)
					{
						Double_t gammax = gamma->GetStartX();
						Double_t gammay = gamma->GetStartY();
						fHM->H2("fh_mc_gammaxy")->Fill(gammax, gammay);
						Double_t gammaz = gamma->GetStartZ();
						fHM->H1("fh_mc_gammaz")->Fill(gammaz);
					}
				}
			}
		}
		
		

	}

	for(int i=0; i<nofRichPoints; i++)
	{
		CbmRichPoint* richpoints= (CbmRichPoint*) (fRichPoints->At(i));
		Double_t xrichp = richpoints->GetX();
		Double_t yrichp = richpoints->GetY();
		fHM->H2("fh_dis_rich_points")->Fill(xrichp, yrichp);
		
	}
	
	for(int i=0; i<nofRichHits; i++)
	{
		CbmRichHit* richhits= (CbmRichHit*) (fRichHits->At(i));
		Double_t xrichh = richhits->GetX();
		Double_t yrichh = richhits->GetY();
		fHM->H2("fh_dis_rich_hits")->Fill(xrichh, yrichh);
		
	}
	
	for(int i=0; i<nofSTSPoints; i++)
	{
		CbmStsPoint* stspoints= (CbmStsPoint*) (fSTSPoints->At(i));
		Double_t xstsp = stspoints->GetXIn();
		Double_t ystsp = stspoints->GetYIn();
		fHM->H2("fh_dis_sts_points")->Fill(xstsp, ystsp);
		
	}
	
	for(int i=0; i<nofSTSHits; i++)
	{
		CbmStsHit* stshits= (CbmStsHit*) (fSTSHits->At(i));
		Double_t xstsh = stshits->GetX();
		Double_t ystsh = stshits->GetY();
		fHM->H2("fh_dis_sts_hits")->Fill(xstsh, ystsh);
		
	}
	
	for(int i=0; i<nofTRDPoints; i++)
	{
		CbmTrdPoint* trdpoints= (CbmTrdPoint*) (fTRDPoints->At(i));
		Double_t xtrdp = trdpoints->GetXIn();
		Double_t ytrdp = trdpoints->GetYIn();
		fHM->H2("fh_dis_trd_points")->Fill(xtrdp, ytrdp);
	}

	for(int i=0; i<nofTRDHits; i++)
	{
		CbmTrdHit* trdhits= (CbmTrdHit*) (fTRDHits->At(i));
		Double_t xtrdh = trdhits->GetX();
		Double_t ytrdh = trdhits->GetY();
		fHM->H2("fh_dis_trd_hits")->Fill(xtrdh, ytrdh);
	}
	
	for( int i=0; i<nofToFPoints; i++)
	{
		CbmTofPoint* tofpoints= (CbmTofPoint*) (fToFPoints->At(i));
		Double_t xtofp = tofpoints->GetX();
		Double_t ytofp = tofpoints->GetY();
		fHM->H2("fh_dis_tof_points")->Fill(xtofp, ytofp);
	}
	
	for( int i=0; i<nofToFHits; i++)
	{
		CbmTofHit* tofhits= (CbmTofHit*) (fToFHits->At(i));
		Double_t xtofh = tofhits->GetX();
		Double_t ytofh = tofhits->GetY();
		fHM->H2("fh_dis_tof_hits")->Fill(xtofh, ytofh);
	}
}	

void CbmRichSmallPrototypeQa::InitHistograms()
{

    fHM = new CbmHistManager();
  	
//MC Tracks	
	fHM->Create1<TH1D>("fh_nof_mc_tracks", "fh_nof_mc_tracks;Nof MC Tracks;Yield", 300, 0., 3000.);

	fHM->Create1<TH1D>("fh_mc_mom_all", "fh_mc_mom_all;Momentum;Yield", 250, 0., 25.);
	fHM->Create1<TH1D>("fh_mc_pdg_all", "fh_mc_pdg_all;PDG Codes;Yield", 1000, -500., 500.);
	fHM->Create1<TH1D>("fh_mc_mom_pion", "fh_mc_mom_pion;Momentum;Yield", 250, 0., 25.);
	fHM->Create1<TH1D>("fh_mc_mom_elec", "fh_mc_mom_elec;Momentum;Yield", 250, 0., 25.);

	fHM->Create2<TH2D>("fh_mc_startxy", "fh_mc_startxy; Start x; Start y", 3000, 0., 300., 3000, 0., 300.);
	fHM->Create1<TH1D>("fh_mc_startz", "fh_mc_startz; Start z", 3000, 0., 300.);

	fHM->Create2<TH2D>("fh_mc_gammaxy", "fh_mc_gammaxy;x;y", 3000, 0., 300., 3000., 0., 300.);
	fHM->Create1<TH1D>("fh_mc_gammaz", "fh_mc_gammaz; z", 3000, 0., 300.);

//STS 
	fHM->Create1<TH1D>("fh_nof_sts_points", "fh_nof_sts_points;Nof STS Points;Yield", 300, 0., 300.);
	fHM->Create1<TH1D>("fh_nof_sts_digis", "fh_nof_sts-digs;Nof STS Digis;Yield", 300, 0., 10000.);
	fHM->Create1<TH1D>("fh_nof_sts_hits", "fh_nof_sts_hits;Nof STS Hits;Yield", 300, 0., 10000.);
	fHM->Create1<TH1D>("fh_nof_sts_track", "fh_nof_sts_track;Nof STS Tracks;Yield", 300, 0., 1000.);
	
	fHM->Create2<TH2D>("fh_dis_sts_points", "fh_dis_stspoints; x; y", 3000, -150., 150., 3000, -150., 150.);
	fHM->Create2<TH2D>("fh_dis_sts_hits", "fh_dis_stshits; x; y;", 3000, -150., 150., 3000, -150., 150.);
	
//RICH
	fHM->Create1<TH1D>("fh_nof_rich_points", "fh_nof_rich_points;Nof Rich Points;Yield", 300, 0., 2000.);
	fHM->Create1<TH1D>("fh_nof_rich_digis", "fh_nof_rich_digis;Nof Rich Digis;Yield", 300, 0., 1000.);
	fHM->Create1<TH1D>("fh_nof_rich_hits", "fh_nof_rich_hits;Nof Rich hits;Yield", 300, 0., 1000.);
	fHM->Create1<TH1D>("fh_nof_rich_rings", "fh_nof_rich_rings;Nof Rich rings;Yield", 40, 0., 40.);
	
	fHM->Create2<TH2D>("fh_dis_rich_points", "fh_dis_rich_points; x; y", 3000, -150., 150., 3000, -150., 150.);
	fHM->Create2<TH2D>("fh_dis_rich_hits", "fh_dis_rich_hits; x; y", 3000, -150., 150., 3000, -150., 150.);

//TRD
	fHM->Create1<TH1D>("fh_nof_trd_points", "fh_nof_trd_points;Nof TRD Points;Yield", 300, 0., 10000.);
	fHM->Create1<TH1D>("fh_nof_trd_hits", "fh_nof_trd_hits;Nof TRD Hits;Yield", 300, 0., 10000.);
	fHM->Create1<TH1D>("fh_nof_trd_track", "fh_nof_trd_track;Nof TRD Track;Yield", 300, 0., 600.);

	fHM->Create2<TH2D>("fh_dis_trd_points", "fh_dis_trd_points;x;y",3000, -150., 150., 3000, -150., 150.);
	fHM->Create2<TH2D>("fh_dis_trd_hits", "fh_dis_trd_hits;x;y", 3000, -150., 150., 3000, -150., 150.);


//ToF
	fHM->Create1<TH1D>("fh_nof_tof_points", "fh_nof_tof_points;Nof ToF Points;Yield", 300, 0., 2000.);
	fHM->Create1<TH1D>("fh_nof_tof_hits", "fh_nof_tof_hits;Nof ToF Hits;Yield", 300, 0., 2000.);
	fHM->Create1<TH1D>("fh_nof_tof_track", "fh_nof_tof_track;Nof ToF Track;Yield", 300, 0., 2000.);

	fHM->Create2<TH2D>("fh_dis_tof_points", "fh_dis_tof_points;x;y", 3000, -150., 150., 3000, -150., 150.);
	fHM->Create2<TH2D>("fh_dis_tof_hits", "fh_dis_tof_hits;x;y", 3000, -150., 150., 3000, -150., 150.);
}

void CbmRichSmallPrototypeQa::DrawHist()
{
	
	
    cout.precision(4);
    
    SetDefaultDrawStyle();
    fHM->ScaleByPattern("", 1./fEventNum);
	

    {  
       /* TCanvas* c = CreateCanvas("rich_sp_nof_rich_hits", "rich_sp_nof_rich_hits", 800, 800);
        DrawH1andFitGauss(fHM->H1("fh_nof_rich_hits"));*/
        //gPad->SetLogy(true);
		
		/*
		fHM->H1("fh_nof_rich_hits")->Fit("gaus");
		TF1 * func = fHM->H1("fh_nof_rich_hits")->GetFunction("gaus");
		double m = func->GetParameter(1) ;
		double sigma = func->GetParameter(2);
		cout << "mean: "<< m << " sigma: " << sigma << endl;
		*/
		
	}
	{
		TCanvas* c = CreateCanvas("rich_sp_nof_mc_tracks", "rich_sp_nof_mc_tracks", 800, 800);
		gStyle->SetOptTitle(1);
		DrawH1andFitGauss(fHM->H1("fh_nof_mc_tracks"));
		fHM->H1("fh_nof_mc_tracks")->SetTitle("MC Track");
	}
	{
		TCanvas* c=CreateCanvas("rich_sp_sts_nof", "rich_sp_sts_nof", 1600, 800);
		c->Divide(2,2);
		c->cd(1);
		gStyle->SetOptTitle(1);
		DrawH1andFitGauss(fHM->H1("fh_nof_sts_points"));
		c->cd(2);
		gStyle->SetOptTitle(1);
		DrawH1andFitGauss(fHM->H1("fh_nof_sts_digis"));
		c->cd(3);
		gStyle->SetOptTitle(1);
		DrawH1andFitGauss(fHM->H1("fh_nof_sts_hits"));
		c->cd(4);
		gStyle->SetOptTitle(1);
		DrawH1andFitGauss(fHM->H1("fh_nof_sts_track"));
		

	}
	{
		TCanvas* c=CreateCanvas("rich_sp_rich_nof", "rich_sp_rich_nof", 1600, 800);
		c->Divide(2,2);
		c->cd(1);
		gStyle->SetOptTitle(1);
		DrawH1andFitGauss(fHM->H1("fh_nof_rich_points"));
		c->cd(2);
		gStyle->SetOptTitle(1);
		DrawH1andFitGauss(fHM->H1("fh_nof_rich_digis"));
		c->cd(3);
		gStyle->SetOptTitle(1);
		DrawH1andFitGauss(fHM->H1("fh_nof_rich_hits"));
		c->cd(4);
		gStyle->SetOptTitle(1);
		DrawH1andFitGauss(fHM->H1("fh_nof_rich_rings"));
		
	}
	{
		TCanvas* c=CreateCanvas("rich_sp_trd_nof", "rich_sp_trd_nof", 1600, 800);
		c->Divide(2,2);
		c->cd(1);
		gStyle->SetOptTitle(1);
		DrawH1andFitGauss(fHM->H1("fh_nof_trd_points"));
		c->cd(2);
		gStyle->SetOptTitle(1);
		DrawH1andFitGauss(fHM->H1("fh_nof_trd_hits"));
		c->cd(3);
		gStyle->SetOptTitle(1);
		DrawH1andFitGauss(fHM->H1("fh_nof_trd_track"));

	}
	{
		TCanvas* c=CreateCanvas("rich_sp_tof_nof", "rich_sp_tof_nof", 1600, 800);
		c->Divide(2,2);
		c->cd(1);
		gStyle->SetOptTitle(1);
		DrawH1andFitGauss(fHM->H1("fh_nof_tof_points"));
		c->cd(2);
		gStyle->SetOptTitle(1);
		DrawH1andFitGauss(fHM->H1("fh_nof_tof_hits"));
		c->cd(3);
		gStyle->SetOptTitle(1);
		DrawH1andFitGauss(fHM->H1("fh_nof_tof_track"));

	}
	{
		TCanvas* c=CreateCanvas("mc_track_mom","mc_track_mom", 1600, 800);
		c->Divide(2,2);
		c->cd(1);
		gStyle->SetOptTitle(1);
		DrawH1(list_of(fHM->H1("fh_mc_mom_all"))(fHM->H1("fh_mc_mom_pion"))(fHM->H1("fh_mc_mom_elec")), list_of("fh_mc_mom_all")("fh_mc_mom_pion")("fh_mc_mom_elec"), kLinear, kLog);
		c->cd(2);
		gStyle->SetOptTitle(1);
		DrawH1(fHM->H1("fh_mc_pdg_all"));
		c->cd(3);
		gStyle->SetOptTitle(1);
		DrawH2(fHM->H2("fh_mc_startxy"));
		c->cd(4);
		gStyle->SetOptTitle(1);
		DrawH1(fHM->H1("fh_mc_startz"), kLinear, kLog);
	}
	{ 
		TCanvas* c=CreateCanvas("rich_dis_of_points_and_hits", "rich_dis_of_points_hits", 800, 800);
		c->Divide(1,2);
		c->cd(1);
		DrawH2(fHM->H2("fh_dis_rich_points"));
		fHM->H2("fh_dis_rich_points")->SetTitle("Rich Points");
		c->cd(2);
		DrawH2(fHM->H2("fh_dis_rich_hits"));
		fHM->H2("fh_dis_rich_hits")->SetTitle("Rich Hits");

	}
	{
		TCanvas* c=CreateCanvas("sts_dis_of_points_and_hits", "sts_dis_of_points_hits", 800, 800);
		c->Divide(1,2);
		c->cd(1);
		gStyle->SetOptTitle(1);
		DrawH2(fHM->H2("fh_dis_sts_points"));
		fHM->H2("fh_dis_sts_points")->SetTitle("STS Points");
		c->cd(2);
		gStyle->SetOptTitle(1);
		DrawH2(fHM->H2("fh_dis_sts_hits"));
		fHM->H2("fh_dis_sts_hits")->SetTitle("STS Hits");
	}
	{
		TCanvas* c=CreateCanvas("trd_dis_of_points_and_hits", "trd_dis_of_points_and_hits", 800, 800);
		c->Divide(1,2);
		c->cd(1);
		gStyle->SetOptTitle(1);
		DrawH2(fHM->H2("fh_dis_trd_points"));
		fHM->H2("fh_dis_trd_points")->SetTitle("TRD Points");
		c->cd(2);
		gStyle->SetOptTitle(1);
		DrawH2(fHM->H2("fh_dis_trd_hits"));
		fHM->H2("fh_dis_trd_hits")->SetTitle("TRD Hits");
	}
	{
		TCanvas* c=CreateCanvas("tof_dis_of_points_and_hits", "tof_dis_of_points_and_hits", 800, 800);
		c->Divide(1,2);
		c->cd(1);
		gStyle->SetOptTitle(1);
		DrawH2(fHM->H2("fh_dis_tof_points"));
		fHM->H2("fh_dis_tof_points")->SetTitle("ToF Points");
		c->cd(2);
		DrawH2(fHM->H2("fh_dis_tof_hits"));
		fHM->H2("fh_dis_tof_hits")->SetTitle("ToF Hits");
		
	}
	{
		TCanvas* c=CreateCanvas("gamma_conversion", "gamma_conversion", 1600, 400);
		c->Divide(2,1);
		c->cd(1);
		gStyle->SetOptTitle(0);		
		DrawH2(fHM->H2("fh_mc_gammaxy"));
		fHM->H2("fh_mc_gammaxy")->SetTitle("GammaXY");
		c->cd(2);
		gStyle->SetOptTitle(0);
		DrawH1(fHM->H1("fh_mc_gammaz"), kLinear, kLog);
		fHM->H1("fh_mc_gammaz")->SetTitle("GammaZ");
	}
  }

void CbmRichSmallPrototypeQa::Finish()
{
    DrawHist();
    SaveCanvasToImage();
    fHM->WriteToFile();
}


TCanvas* CbmRichSmallPrototypeQa::CreateCanvas(
                                        const string& name,
                                        const string& title,
                                        int width,
                                        int height)
{
    TCanvas* c = new TCanvas(name.c_str(), title.c_str(), width, height);
    fCanvas.push_back(c);
    return c;
}

void CbmRichSmallPrototypeQa::SaveCanvasToImage()
{
    for (int i = 0; i < fCanvas.size(); i++)
    {
        Cbm::SaveCanvasAsImage(fCanvas[i], fOutputDir);
    }
}

ClassImp(CbmRichSmallPrototypeQa)

