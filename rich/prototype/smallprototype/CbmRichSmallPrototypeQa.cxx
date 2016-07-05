
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
fEventNum(0),
fOutputDir(""),
fMCTracks(NULL),
fRichPoints(NULL),
fRichDigis(NULL),
fRichHits(NULL),
fRichRings(NULL),
fRichRingMatches(NULL),
fCanvas()
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

	fRichPoints =(TClonesArray*) ioman->GetObject("RichPoint");
	if (NULL == fRichPoints) { Fatal("CbmRichSmallPrototypeQa::Init", "No Rich Points!");}

	fRichDigis =(TClonesArray*) ioman->GetObject("RichDigi");
	if (NULL == fRichDigis) { Fatal("CbmRichSmallPrototypeQa::Init", "No Rich Digis!");}

	fRichHits = (TClonesArray*) ioman->GetObject("RichHit");
    if ( NULL == fRichHits) { Fatal("CbmRichSmallPrototypeQa::Init","No RichHits!"); }

	fRichRings = (TClonesArray*) ioman->GetObject("RichRing");
    if ( NULL == fRichRings) { Fatal("CbmRichSmallPrototypeQa::Init","No RichRings!"); }
    
    fRichRingMatches = (TClonesArray*) ioman->GetObject("RichRingMatch");
    if ( NULL == fRichRingMatches) { Fatal("CbmRichGeoTest::Init","No RichRingMatch array!"); }
	
	fRefPlanePoints = (TClonesArray*) ioman->GetObject("RefPlanePoint");
    if ( NULL == fRefPlanePoints) { Fatal("CbmRichSmallPrototypeQa::Init","No RefPlanePoints!"); }
	
    InitHistograms();
    
    return kSUCCESS;
}

void CbmRichSmallPrototypeQa::Exec(
                            Option_t* option)
{

    fEventNum++;
    
	//cout << "SOMETHING" <<endl;
    cout << "CbmRichSmallPrototypeQa, event No. " <<  fEventNum << endl;

	Int_t nofMCTracks = fMCTracks->GetEntriesFast();

	Int_t nofRichPoints = fRichPoints->GetEntriesFast();
	Int_t nofRichDigis = fRichDigis->GetEntriesFast();
	Int_t nofRichHits = fRichHits->GetEntriesFast();	
	Int_t nofRichRings = fRichRings->GetEntriesFast();
	Int_t nofRefPlanePoints = fRefPlanePoints->GetEntriesFast();


//	fHM->H1("fh_nof_mc_tracks")->Fill(nofMCTracks);


	fHM->H1("fh_nof_rich_points")->Fill(nofRichPoints);
	fHM->H1("fh_nof_rich_digis")->Fill(nofRichDigis);
    fHM->H1("fh_nof_rich_hits")->Fill(nofRichHits); 
	fHM->H1("fh_nof_rich_rings")->Fill(nofRichRings);


	for (int i=0; i<nofRichHits; i++)
	{ 
	 CbmRichHit* richHit= (CbmRichHit*) (fRichHits->At(i));
	 //CbmRichHit* richHit = static_cast<CbmRichHit*>(fRichHits -> At(i));
    }
/*	for (int i=0; i<nofMCTracks; i++)
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
*/

	for(int i=0; i<nofRichPoints; i++)
	{
		CbmRichPoint* richpoints= (CbmRichPoint*) (fRichPoints->At(i));
		Double_t xrichp = richpoints->GetX();
		Double_t yrichp = richpoints->GetY();
		fHM->H2("fh_dis_rich_points")->Fill(xrichp, yrichp);
		
	}
	
		
	for(int i=0; i<nofRichPoints; i++)
	{
		CbmRichPoint* richpoints= (CbmRichPoint*) (fRichPoints->At(i));
		Int_t trackid = richpoints->GetTrackID();
		if(trackid<0) continue;
		CbmMCTrack* mctrack= (CbmMCTrack*) (fMCTracks->At(trackid));
		Int_t motherid = mctrack->GetMotherId();
		if(motherid < 0)
		{	
			Double_t xpmtproton = richpoints->GetX();
			Double_t ypmtproton = richpoints->GetY();
			fHM->H2("fh_dis_pmt_protons")->Fill(xpmtproton, ypmtproton);
		}
	}
	

	
	for( int i=0; i<nofMCTracks; i++)
	{
	

		CbmMCTrack* mctrack= (CbmMCTrack*) (fMCTracks->At(i));
		Double_t pdg = mctrack->GetPdgCode();
		if (pdg == 2212)
		{
			Double_t startx = mctrack->GetStartX();
			Double_t starty = mctrack->GetStartY();
			fHM->H2("fh_proton_startxy")->Fill(startx, starty);	
		}	


	
	}

	
	for(int iR = 0; iR < nofRichRings; iR++) {
	
		CbmRichRing* ring = (CbmRichRing*) (fRichRings->At(iR));
        if (NULL == ring) continue;
        CbmTrackMatchNew* ringMatch = (CbmTrackMatchNew*) fRichRingMatches->At(iR);
cout << iR << endl;
        if (NULL == ringMatch) continue;
        
        Int_t mcTrackId = ringMatch->GetMatchedLink().GetIndex();
        if (mcTrackId < 0) continue;
        CbmMCTrack* mcTrack = (CbmMCTrack*)fMCTracks->At(mcTrackId);
        if (!mcTrack) continue;
        Int_t motherId = mcTrack->GetMotherId();
        Int_t pdg = TMath::Abs(mcTrack->GetPdgCode());
        //select only primary protons
        if (!(motherId == -1 && pdg == 2212)) continue;

		Double_t cX = ring->GetCenterX();
    	Double_t cY = ring->GetCenterY();
		fHM->H2("fh_cX_cY")->Fill(cX,cY);
        int nofHits = ring->GetNofHits();
		fHM->H1("fh_hits_per_ring")->Fill(nofHits);
cout << nofRichRings << endl;

cout<<nofHits<<endl;
		fHM->H2("fh_hits_per_ring_per_ring")->Fill(iR, nofHits);
        


			
			Double_t radius = ring->GetRadius();		
			fHM->H1("fh_rich_ring_radius")->Fill(radius);
			fHM->H2("fh_radius_ring")->Fill(iR,radius);
		
			for (int iH = 0; iH < nofHits; iH++)

			{
            	Int_t hitInd = ring->GetHit(iH);
            	CbmRichHit* hit = (CbmRichHit*) fRichHits->At(hitInd);
            	if (NULL == hit) continue;
            	Double_t hitX = hit->GetX();
            	Double_t hitY = hit->GetY();
            	Double_t dR = radius - TMath::Sqrt( (cX - hitX)*( cX - hitX) + (cY - hitY)*(cY - hitY) );
            	fHM->H1("fh_dR")->Fill(dR);
        	}

	}
    
    
    for(int iH = 0; iH < nofRichHits; iH++)
    {
        CbmRichHit* richHit = (CbmRichHit*) (fRichHits->At(iH));
        fHM->H2("fh_dis_rich_hits")->Fill(richHit->GetX(), richHit->GetY());
    }

	for (Int_t i=0; i<nofRefPlanePoints; i++)
	{
		CbmRichPoint* pRefPlane = (CbmRichPoint*) (fRefPlanePoints->At(i));
        
		Int_t trackid = pRefPlane->GetTrackID();
		if( trackid < 0) continue;
		CbmMCTrack* mctrack= (CbmMCTrack*) (fMCTracks->At(trackid));
		if(mctrack ==NULL) continue;
		Int_t pdg = mctrack->GetPdgCode();
		fHM->H1("fh_sensplane_pdg")->Fill(pdg);
		
	}
    
	
	for (Int_t i=0; i<nofRefPlanePoints; i++)
	{
		CbmRichPoint* pRefPlane = (CbmRichPoint*) (fRefPlanePoints->At(i));
        Double_t xsec= pRefPlane->GetX();
        Double_t ysec= pRefPlane->GetY();
		Int_t trackid = pRefPlane->GetTrackID();
		if( trackid < 0) continue;
		CbmMCTrack* mctrack= (CbmMCTrack*) (fMCTracks->At(trackid));
		if(mctrack ==NULL) continue;
		Int_t pdg = mctrack->GetPdgCode();
		if(pdg != 2212)										//Check if secondary or primary particle
		{
			fHM->H2("fh_dis_sec")->Fill(xsec,ysec);
   		} 
	}
	
	for (Int_t i=0; i<nofRefPlanePoints; i++)
	{
		CbmRichPoint* pRefPlane = (CbmRichPoint*) (fRefPlanePoints->At(i));
        Double_t xprimsec= pRefPlane->GetX();
        Double_t yprimsec= pRefPlane->GetY();
		Int_t trackid = pRefPlane->GetTrackID();
		if( trackid < 0) continue;
		CbmMCTrack* mctrack= (CbmMCTrack*) (fMCTracks->At(trackid));
		if(mctrack ==NULL) continue;
		Int_t motherid = mctrack->GetMotherId();
		
		{
			fHM->H2("fh_dis_primsec")->Fill(xprimsec,yprimsec);
   		} 
	}
	
	for (Int_t i=0; i<nofRefPlanePoints; i++)
	{
		CbmRichPoint* pRefPlane = (CbmRichPoint*) (fRefPlanePoints->At(i));
        Double_t xprim= pRefPlane->GetX();
        Double_t yprim= pRefPlane->GetY();
		Int_t trackid = pRefPlane->GetTrackID();
		if( trackid < 0) continue;
		CbmMCTrack* mctrack= (CbmMCTrack*) (fMCTracks->At(trackid));
		if(mctrack ==NULL) continue;
		Int_t pdg = mctrack->GetPdgCode();

		if(pdg==2212)
		{
			fHM->H2("fh_dis_prim")->Fill(xprim,yprim);
   		} 
		
	}
	

}	

void CbmRichSmallPrototypeQa::InitHistograms()
{

    fHM = new CbmHistManager();
  	
//MC Tracks	
/*
	fHM->Create1<TH1D>("fh_nof_mc_tracks", "fh_nof_mc_tracks;Nof MC Tracks;Yield", 300, 0., 3000.);

	fHM->Create1<TH1D>("fh_mc_mom_all", "fh_mc_mom_all;Momentum;Yield", 270, 0., 27.);
	fHM->Create1<TH1D>("fh_mc_pdg_all", "fh_mc_pdg_all;PDG Codes;Yield", 1000, -700., 700.);
	fHM->Create1<TH1D>("fh_mc_mom_pion", "fh_mc_mom_pion;Momentum;Yield", 270, 0., 27.);
	fHM->Create1<TH1D>("fh_mc_mom_elec", "fh_mc_mom_elec;Momentum;Yield", 270, 0., 27.);

	fHM->Create2<TH2D>("fh_mc_startxy", "fh_mc_startxy; Start x [cm]; Start y [cm]", 3000, 0., 300., 3000, 0., 300.);
	fHM->Create1<TH1D>("fh_mc_startz", "fh_mc_startz; Start z [cm]", 3000, 0., 300.);

	fHM->Create2<TH2D>("fh_mc_gammaxy", "fh_mc_gammaxy;x [cm];y [cm]", 3000, 0., 300., 3000., 0., 300.);
	fHM->Create1<TH1D>("fh_mc_gammaz", "fh_mc_gammaz; z [cm]", 3000, 0., 300.);
*/

	
//RICH
	fHM->Create1<TH1D>("fh_nof_rich_points", "fh_nof_rich_points;Nof Rich Points;Yield (a.u.)", 250, 0., 500.);
	fHM->Create1<TH1D>("fh_nof_rich_digis", "fh_nof_rich_digis;Nof Rich Digis;Yield (a.u.)", 250, 0., 1000.);
	fHM->Create1<TH1D>("fh_nof_rich_hits", "fh_nof_rich_hits;Nof Rich hits;Yield (a.u.)", 70, 0., 70.);
	fHM->Create1<TH1D>("fh_nof_rich_rings", "fh_nof_rich_rings;Nof Rich rings;Yield (a.u.)", 5, -0.5, 4.5);

	fHM->Create1<TH1D>("fh_rich_ring_radius","fh_rich_ring_radius; Ring Radius [cm]; Yield (a.u.)", 300, 5.2, 6.);
	
	fHM->Create2<TH2D>("fh_dis_rich_points", "fh_dis_rich_points; x [cm]; y [cm]", 300, -7., 7., 300, -7., 7.);
	fHM->Create2<TH2D>("fh_dis_rich_hits", "fh_dis_rich_hits; x[cm]; y[cm]", 50, -10., 10., 50, -10., 10.);
	fHM->Create1<TH1D>("fh_hits_per_ring", "fh_hits_per_ring;Hits per Ring;Yield (a.u.)", 40, -0.5, 39.5);
	fHM->Create2<TH2D>("fh_cX_cY","fh_cX_cY;cX[cm];cY[cm]", 20, -10, 10, 20, -10, 10);	
	fHM->Create2<TH2D>("fh_hits_per_ring_per_ring","fh_hits_per_ring_per_ring; Ring Number; Hits[cm]", 10, 0., 10., 40., 0., 40.);
	fHM->Create2<TH2D>("fh_radius_ring","fh_radius_ring;Ring Number;Radius[cm]", 10, 0., 10., 140 , 0., 7. );
	
	fHM->Create2<TH2D>("fh_proton_startxy","fh_proton_startxy; x [cm]; y[cm];", 100, -5.,5, 100, -5., 5.);

	fHM->Create1<TH1D>("fh_dR","fh_dR; dR [cm]; Yield (a.u.)", 100, -0.8, 0.8);

	fHM->Create2<TH2D>("fh_dis_primsec", "fh_dis_primsec; x [cm]; y [cm];", 800, -200., 200, 800, -200., 200.);
	fHM->Create2<TH2D>("fh_dis_prim", "fh_dis_prim; x [cm]; y [cm];", 160, -40., 40., 160, -40., 40.);
	fHM->Create2<TH2D>("fh_dis_sec", "fh_dis_sec; x [cm]; y [cm];", 160, -40., 40., 160, -40., 40.);
	fHM->Create1<TH1D>("fh_sensplane_pdg", "fh_sensplane_pdg;PDG Codes;Yield (a.u.)", 6001, -3000., 3000.);

	fHM->Create2<TH2D>("fh_dis_pmt_protons","fh_dis_pmt_protons; x [cm]; y [cm];", 50, -10., 10., 50, -10., 10.);
}

void CbmRichSmallPrototypeQa::DrawHist()
{
	
	
    cout.precision(4);
    
    SetDefaultDrawStyle();
    fHM->ScaleByPattern("", 1./fEventNum);
	

  {  
        TCanvas* c = CreateCanvas("rich_sp_nof_rich_hits_points", "rich_sp_nof_rich_hits_points", 800, 800);
		c->Divide(1,2);
		c->cd(1);
        DrawH1andFitGauss(fHM->H1("fh_nof_rich_hits"));
        //gPad->SetLogy(true);
		c->cd(2);
		DrawH1andFitGauss(fHM->H1("fh_nof_rich_points"));
		
		
		/*
		fHM->H1("fh_nof_rich_hits")->Fit("gaus");
		TF1 * func = fHM->H1("fh_nof_rich_hits")->GetFunction("gaus");
		double m = func->GetParameter(1) ;
		double sigma = func->GetParameter(2);
		cout << "mean: "<< m << " sigma: " << sigma << endl;
		*/
		
	}


/*
	{
		TCanvas* c = CreateCanvas("rich_sp_nof_mc_tracks", "rich_sp_nof_mc_tracks", 800, 800);
		gStyle->SetOptTitle(1);
		DrawH1andFitGauss(fHM->H1("fh_nof_mc_tracks"));
		fHM->H1("fh_nof_mc_tracks")->SetTitle("MC Track");
	}
*/


/*	{
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
*/

/*
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
*/
	{ 
		TCanvas* c=CreateCanvas("rich_dis_of_points_and_hits", "rich_dis_of_points_hits", 1200, 400);
		c->Divide(2,1);
		c->cd(1);
		DrawH2(fHM->H2("fh_dis_rich_points"));
		fHM->H2("fh_dis_rich_points")->SetTitle("Rich Points");
		c->cd(2);
		DrawH2(fHM->H2("fh_dis_rich_hits"));
		fHM->H2("fh_dis_rich_hits")->SetTitle("Rich Hits");

	}

 
	{
		TCanvas* c=CreateCanvas("fh_proton_startxy", "fh_proton_startxy", 400, 400);
		DrawH2(fHM->H2("fh_proton_startxy"));
		fHM->H2("fh_proton_startxy")->SetTitle("Startproton");
	}

	{
		TCanvas* c=CreateCanvas("rich_rings", "rich_rings", 800, 400);
		c->Divide(2,1);
		c->cd(1);
		DrawH1(fHM->H1("fh_nof_rich_rings"));
		fHM->H1("fh_nof_rich_rings")->SetTitle("Nof Rich Rings");
		c->cd(2);
		DrawH1andFitGauss(fHM->H1("fh_rich_ring_radius"));
		fHM->H1("fh_rich_ring_radius")->SetTitle("Ring Radius");	
	}
	
	{
		TCanvas* c=CreateCanvas("fh_dR", "fh_dR", 400, 400);
		DrawH1andFitGauss(fHM->H1("fh_dR"));
		fHM->H1("fh_dR")->SetTitle("dR");
	}
	
	{
		TCanvas* c=CreateCanvas("fh_dis_sec", "fh_dis_sec", 400, 400);
		DrawH2(fHM->H2("fh_dis_sec"));
		fHM->H2("fh_dis_sec")->SetTitle("Secondaries");
	}
	
	{
		TCanvas* c=CreateCanvas("fh_dis_prim", "fh_dis_prim", 400, 400);
		DrawH2(fHM->H2("fh_dis_prim"));
		fHM->H2("fh_dis_prim")->SetTitle("Primaries");
	}
	
	{
		TCanvas* c=CreateCanvas("fh_dis_primsec", "fh_dis_primsec", 400, 400);
		DrawH2(fHM->H2("fh_dis_primsec"));
		fHM->H2("fh_dis_primsec")->SetTitle("Primaries and Secondaries");
	}
	
	{
		TCanvas* c=CreateCanvas("fh_dis_pmt_protons", "fh_dis_pmt_protons", 400, 400);
		DrawH2(fHM->H2("fh_dis_pmt_protons"));
		fHM->H2("fh_dis_pmt_protons")->SetTitle("Protons on PMT");
	}

	{
		TCanvas* c=CreateCanvas("fh_sensplane_pdg", "fh_sensplane_pdg", 1600, 400);
		DrawH1(fHM->H1("fh_sensplane_pdg"));
		fHM->H1("fh_sensplane_pdg")->SetTitle("PDGs on sens_plane");
	}

	{
		TCanvas* c=CreateCanvas("fh_hits_per_ring", "fh_hits_per_ring", 400, 400);
		DrawH1(fHM->H1("fh_hits_per_ring"));
		fHM->H1("fh_hits_per_ring")->SetTitle("Hits per Ring");
	}

	{
		TCanvas* c=CreateCanvas("fh_cX_cY", "fh_cX_cY", 400, 400);
		DrawH2(fHM->H2("fh_cX_cY"));
		fHM->H1("fh_cX_cY")->SetTitle("Ringcenter");
	}
	
	{
		TCanvas* c=CreateCanvas("fh_hits_per_ring_per_ring", "fh_hits_per_ring_per_ring", 400, 400);
		DrawH2(fHM->H2("fh_hits_per_ring_per_ring"));
		fHM->H1("fh_hits_per_ring_per_ring")->SetTitle("Hits per single Ring");
	}

	{
		TCanvas* c=CreateCanvas("fh_radius_ring", "fh_radius_ring", 400, 400);
		DrawH2(fHM->H2("fh_radius_ring"));
		fHM->H1("fh_radius_ring")->SetTitle("Radius per single Ring");
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

