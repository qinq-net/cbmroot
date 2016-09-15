
#include "CbmRichSmallPrototypeQa.h"

#include "TH1D.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TClonesArray.h"
#include "TF1.h"
#include "TStyle.h"
#include "TEllipse.h"

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
		int nofHits = ring->GetNofHits();

     CbmTrackMatchNew* ringMatch = (CbmTrackMatchNew*) fRichRingMatches->At(iR);

        if (NULL == ringMatch) continue;
        
        Int_t mcTrackId = ringMatch->GetMatchedLink().GetIndex();
        if (mcTrackId < 0) continue;
        CbmMCTrack* mcTrack = (CbmMCTrack*)fMCTracks->At(mcTrackId);
        if (!mcTrack) continue;
        Int_t motherId = mcTrack->GetMotherId();
        Int_t pdg = TMath::Abs(mcTrack->GetPdgCode());
        //select only primary protons
        if (!(motherId == -1 && pdg == 2212)) continue;

		if(nofHits>2)
		{
			fHM->H1("fh_nof_good_rings")->Fill(iR+1);
		}

		Double_t cX = ring->GetCenterX();
    	Double_t cY = ring->GetCenterY();
		fHM->H2("fh_cX_cY")->Fill(cX,cY);
        
		fHM->H1("fh_hits_per_ring")->Fill(nofHits);

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
			
				if(dR>0)
				{
					for(int i = 0; i < nofRichHits; i++)
    				{
        				
        				fHM->H2("fh_dis_rich_hits_dR>")->Fill(hitX, hitY);
    				}
				}
				if(dR<0)
				{
					for(int j = 0; j < nofRichHits; j++)
    				{
        				fHM->H2("fh_dis_rich_hits_dR<")->Fill(hitX, hitY);
    				}
				}
        	}

	}
    
    cout<<" Event Number "<< fEventNum<<endl;
    for(int iH = 0; iH < nofRichHits; iH++)
    {
        CbmRichHit* richHit = (CbmRichHit*) (fRichHits->At(iH));
		Double_t hitX = richHit->GetX();
		Double_t hitY = richHit->GetY();
        fHM->H2("fh_dis_rich_hits")->Fill(hitX, hitY);
		
		if(fEventNum==91)
		{
        	fHM->H2("fh_dis_rich_hits_single_event1")->Fill(hitX, hitY);
		}
		if(fEventNum==92)
		{
        	fHM->H2("fh_dis_rich_hits_single_event2")->Fill(richHit->GetX(), richHit->GetY());
		}
		if(fEventNum==93)
		{
        	fHM->H2("fh_dis_rich_hits_single_event3")->Fill(richHit->GetX(), richHit->GetY());
		}
		if(fEventNum==94)
		{
        	fHM->H2("fh_dis_rich_hits_single_event4")->Fill(richHit->GetX(), richHit->GetY());
		}
		if(fEventNum==95)
		{
        	fHM->H2("fh_dis_rich_hits_single_event5")->Fill(richHit->GetX(), richHit->GetY());
		}
		if(fEventNum==96)
		{
        	fHM->H2("fh_dis_rich_hits_single_event6")->Fill(richHit->GetX(), richHit->GetY());
		}
		if(fEventNum==97)
		{
        	fHM->H2("fh_dis_rich_hits_single_event7")->Fill(richHit->GetX(), richHit->GetY());
		}
		if(fEventNum==98)
		{
        	fHM->H2("fh_dis_rich_hits_single_event8")->Fill(richHit->GetX(), richHit->GetY());
		
		}
		if(fEventNum==99)
		{
        	fHM->H2("fh_dis_rich_hits_single_event9")->Fill(richHit->GetX(), richHit->GetY());
		}

		if(fEventNum==100)
		{
        	fHM->H2("fh_dis_rich_hits_single_event10")->Fill(richHit->GetX(), richHit->GetY());
		}


    }//ende schleife hits

	if(fEventNum==1)
	{
			cout << "Rich Rings: " << nofRichRings << endl;
			for(int iR = 0; iR < nofRichRings; iR++) 
			{
				CbmRichRing* ring = (CbmRichRing*) (fRichRings->At(iR));
       	 		if (NULL == ring) continue;
				CbmTrackMatchNew* ringMatch = (CbmTrackMatchNew*) fRichRingMatches->At(iR);
				if(NULL == ringMatch) continue;
				Double_t cX = ring->GetCenterX();
    			Double_t cY = ring->GetCenterY();
				Double_t radius = ring->GetRadius();
				if(radius>0)
				{
					fHM->H2("fh_dis_rich_hits_single_event1")->Fill(cX, cY);
					fcX.push_back(cX);
					fcY.push_back(cY);
					fradius.push_back(radius);
				}		
			
				
				int nofHits = ring->GetNofHits();

				for (int iH = 0; iH < nofHits; iH++)

				{
            		Int_t hitInd = ring->GetHit(iH);
            		CbmRichHit* hit = (CbmRichHit*) fRichHits->At(hitInd);
            		if (NULL == hit) continue;
            		Double_t hitX = hit->GetX();
            		Double_t hitY = hit->GetY();
            		Double_t dR = radius - TMath::Sqrt( (cX - hitX)*( cX - hitX) + (cY - hitY)*(cY - hitY) );
            		fHM->H1("fh_dR_1")->Fill(dR);
				
        		}
			}
	}	

	if(fEventNum==2)
	{
			cout << "Rich Rings: " << nofRichRings << endl;
			for(int iR = 0; iR < nofRichRings; iR++) 
			{
				CbmRichRing* ring = (CbmRichRing*) (fRichRings->At(iR));
       	 		if (NULL == ring) continue;
				CbmTrackMatchNew* ringMatch = (CbmTrackMatchNew*) fRichRingMatches->At(iR);
				if(NULL == ringMatch) continue;
				Double_t cX = ring->GetCenterX();
    			Double_t cY = ring->GetCenterY();
				Double_t radius = ring->GetRadius();
				if(radius>0)
				{
					fHM->H2("fh_dis_rich_hits_single_event2")->Fill(cX, cY);
					fcX.push_back(cX);
					fcY.push_back(cY);
					fradius.push_back(radius);
				}
	
			int nofHits = ring->GetNofHits();

				for (int iH = 0; iH < nofHits; iH++)

				{
            		Int_t hitInd = ring->GetHit(iH);
            		CbmRichHit* hit = (CbmRichHit*) fRichHits->At(hitInd);
            		if (NULL == hit) continue;
            		Double_t hitX = hit->GetX();
            		Double_t hitY = hit->GetY();
            		Double_t dR = radius - TMath::Sqrt( (cX - hitX)*( cX - hitX) + (cY - hitY)*(cY - hitY) );
            		fHM->H1("fh_dR_2")->Fill(dR);
				
        		}		
			}
	}	
	
	if(fEventNum==3)
	{
			cout << "Rich Rings: " << nofRichRings << endl;
			for(int iR = 0; iR < nofRichRings; iR++) 
			{
				CbmRichRing* ring = (CbmRichRing*) (fRichRings->At(iR));
       	 		if (NULL == ring) continue;
				CbmTrackMatchNew* ringMatch = (CbmTrackMatchNew*) fRichRingMatches->At(iR);
				if(NULL == ringMatch) continue;
				Double_t cX = ring->GetCenterX();
    			Double_t cY = ring->GetCenterY();
				Double_t radius = ring->GetRadius();
				if(radius>0)
				{
					fHM->H2("fh_dis_rich_hits_single_event3")->Fill(cX, cY);
					fcX.push_back(cX);
					fcY.push_back(cY);
					fradius.push_back(radius);
				}
			int nofHits = ring->GetNofHits();

				for (int iH = 0; iH < nofHits; iH++)

				{
            		Int_t hitInd = ring->GetHit(iH);
            		CbmRichHit* hit = (CbmRichHit*) fRichHits->At(hitInd);
            		if (NULL == hit) continue;
            		Double_t hitX = hit->GetX();
            		Double_t hitY = hit->GetY();
            		Double_t dR = radius - TMath::Sqrt( (cX - hitX)*( cX - hitX) + (cY - hitY)*(cY - hitY) );
            		fHM->H1("fh_dR_3")->Fill(dR);
				
        		}
			}
	}	

	if(fEventNum==4)
	{
			cout << "Rich Rings: " << nofRichRings << endl;
			for(int iR = 0; iR < nofRichRings; iR++) 
			{
				CbmRichRing* ring = (CbmRichRing*) (fRichRings->At(iR));
       	 		if (NULL == ring) continue;
				CbmTrackMatchNew* ringMatch = (CbmTrackMatchNew*) fRichRingMatches->At(iR);
				if(NULL == ringMatch) continue;
				Double_t cX = ring->GetCenterX();
    			Double_t cY = ring->GetCenterY();
				Double_t radius = ring->GetRadius();
				if(radius>0)
				{
					fHM->H2("fh_dis_rich_hits_single_event4")->Fill(cX, cY);
					fcX.push_back(cX);
					fcY.push_back(cY);
					fradius.push_back(radius);
				}	
			int nofHits = ring->GetNofHits();

				for (int iH = 0; iH < nofHits; iH++)

				{
            		Int_t hitInd = ring->GetHit(iH);
            		CbmRichHit* hit = (CbmRichHit*) fRichHits->At(hitInd);
            		if (NULL == hit) continue;
            		Double_t hitX = hit->GetX();
            		Double_t hitY = hit->GetY();
            		Double_t dR = radius - TMath::Sqrt( (cX - hitX)*( cX - hitX) + (cY - hitY)*(cY - hitY) );
            		fHM->H1("fh_dR_4")->Fill(dR);
				
        		}
					
			}
	}	

	if(fEventNum==5)
	{
			cout << "Rich Rings: " << nofRichRings << endl;
			for(int iR = 0; iR < nofRichRings; iR++) 
			{
				CbmRichRing* ring = (CbmRichRing*) (fRichRings->At(iR));
       	 		if (NULL == ring) continue;
				CbmTrackMatchNew* ringMatch = (CbmTrackMatchNew*) fRichRingMatches->At(iR);
				if(NULL == ringMatch) continue;
				Double_t cX = ring->GetCenterX();
    			Double_t cY = ring->GetCenterY();
				Double_t radius = ring->GetRadius();
				if(radius>0)
				{
					fHM->H2("fh_dis_rich_hits_single_event5")->Fill(cX, cY);
					fcX.push_back(cX);
					fcY.push_back(cY);
					fradius.push_back(radius);
				}
			int nofHits = ring->GetNofHits();

				for (int iH = 0; iH < nofHits; iH++)

				{
            		Int_t hitInd = ring->GetHit(iH);
            		CbmRichHit* hit = (CbmRichHit*) fRichHits->At(hitInd);
            		if (NULL == hit) continue;
            		Double_t hitX = hit->GetX();
            		Double_t hitY = hit->GetY();
            		Double_t dR = radius - TMath::Sqrt( (cX - hitX)*( cX - hitX) + (cY - hitY)*(cY - hitY) );
            		fHM->H1("fh_dR_5")->Fill(dR);
				
        		}	

						
			}
	}	

	if(fEventNum==6)
	{
			cout << "Rich Rings: " << nofRichRings << endl;
			for(int iR = 0; iR < nofRichRings; iR++) 
			{
				CbmRichRing* ring = (CbmRichRing*) (fRichRings->At(iR));
       	 		if (NULL == ring) continue;
				CbmTrackMatchNew* ringMatch = (CbmTrackMatchNew*) fRichRingMatches->At(iR);
				if(NULL == ringMatch) continue;
				Double_t cX = ring->GetCenterX();
    			Double_t cY = ring->GetCenterY();
				Double_t radius = ring->GetRadius();
				if(radius>0)
				{
					fHM->H2("fh_dis_rich_hits_single_event6")->Fill(cX, cY);
					fcX.push_back(cX);
					fcY.push_back(cY);
					fradius.push_back(radius);
				}
			int nofHits = ring->GetNofHits();

				for (int iH = 0; iH < nofHits; iH++)

				{
            		Int_t hitInd = ring->GetHit(iH);
            		CbmRichHit* hit = (CbmRichHit*) fRichHits->At(hitInd);
            		if (NULL == hit) continue;
            		Double_t hitX = hit->GetX();
            		Double_t hitY = hit->GetY();
            		Double_t dR = radius - TMath::Sqrt( (cX - hitX)*( cX - hitX) + (cY - hitY)*(cY - hitY) );
            		fHM->H1("fh_dR_6")->Fill(dR);
				
        		}	
						
			}
	}	

	if(fEventNum==7)
	{
			cout << "Rich Rings: " << nofRichRings << endl;
			for(int iR = 0; iR < nofRichRings; iR++) 
			{
				CbmRichRing* ring = (CbmRichRing*) (fRichRings->At(iR));
       	 		if (NULL == ring) continue;
				CbmTrackMatchNew* ringMatch = (CbmTrackMatchNew*) fRichRingMatches->At(iR);
				if(NULL == ringMatch) continue;
				Double_t cX = ring->GetCenterX();
    			Double_t cY = ring->GetCenterY();
				Double_t radius = ring->GetRadius();
				if(radius>0)
				{
					fHM->H2("fh_dis_rich_hits_single_event7")->Fill(cX, cY);
					fcX.push_back(cX);
					fcY.push_back(cY);
					fradius.push_back(radius);
				}	
			int nofHits = ring->GetNofHits();

				for (int iH = 0; iH < nofHits; iH++)

				{
            		Int_t hitInd = ring->GetHit(iH);
            		CbmRichHit* hit = (CbmRichHit*) fRichHits->At(hitInd);
            		if (NULL == hit) continue;
            		Double_t hitX = hit->GetX();
            		Double_t hitY = hit->GetY();
            		Double_t dR = radius - TMath::Sqrt( (cX - hitX)*( cX - hitX) + (cY - hitY)*(cY - hitY) );
            		fHM->H1("fh_dR_7")->Fill(dR);
				
        		}
						
			}
	}	

	if(fEventNum==8)
	{
			cout << "Rich Rings: " << nofRichRings << endl;
			for(int iR = 0; iR < nofRichRings; iR++) 
			{
				CbmRichRing* ring = (CbmRichRing*) (fRichRings->At(iR));
       	 		if (NULL == ring) continue;
				CbmTrackMatchNew* ringMatch = (CbmTrackMatchNew*) fRichRingMatches->At(iR);
				if(NULL == ringMatch) continue;
				Double_t cX = ring->GetCenterX();
    			Double_t cY = ring->GetCenterY();				
				Double_t radius = ring->GetRadius();
				if(radius>0)
				{
					fHM->H2("fh_dis_rich_hits_single_event8")->Fill(cX, cY);
					fcX.push_back(cX);
					fcY.push_back(cY);
					fradius.push_back(radius);
				}	
			int nofHits = ring->GetNofHits();

				for (int iH = 0; iH < nofHits; iH++)

				{
            		Int_t hitInd = ring->GetHit(iH);
            		CbmRichHit* hit = (CbmRichHit*) fRichHits->At(hitInd);
            		if (NULL == hit) continue;
            		Double_t hitX = hit->GetX();
            		Double_t hitY = hit->GetY();
            		Double_t dR = radius - TMath::Sqrt( (cX - hitX)*( cX - hitX) + (cY - hitY)*(cY - hitY) );
            		fHM->H1("fh_dR_8")->Fill(dR);
				
        		}	
			}
	}	

	if(fEventNum==9)
	{
			cout << "Rich Rings: " << nofRichRings << endl;
			for(int iR = 0; iR < nofRichRings; iR++) 
			{
				CbmRichRing* ring = (CbmRichRing*) (fRichRings->At(iR));
       	 		if (NULL == ring) continue;
				CbmTrackMatchNew* ringMatch = (CbmTrackMatchNew*) fRichRingMatches->At(iR);
				if(NULL == ringMatch) continue;
				Double_t cX = ring->GetCenterX();
    			Double_t cY = ring->GetCenterY();
				Double_t radius = ring->GetRadius();
				if(radius>0)
				{
					fHM->H2("fh_dis_rich_hits_single_event9")->Fill(cX, cY);
					fcX.push_back(cX);
					fcY.push_back(cY);
					fradius.push_back(radius);
				}
			int nofHits = ring->GetNofHits();

				for (int iH = 0; iH < nofHits; iH++)

				{
            		Int_t hitInd = ring->GetHit(iH);
            		CbmRichHit* hit = (CbmRichHit*) fRichHits->At(hitInd);
            		if (NULL == hit) continue;
            		Double_t hitX = hit->GetX();
            		Double_t hitY = hit->GetY();
            		Double_t dR = radius - TMath::Sqrt( (cX - hitX)*( cX - hitX) + (cY - hitY)*(cY - hitY) );
            		fHM->H1("fh_dR_9")->Fill(dR);
				
        		}			
			}
	}	

	if(fEventNum==10)
		{
			cout << "Rich Rings: " << nofRichRings << endl;
			for(int iR = 0; iR < nofRichRings; iR++) 
			{
				CbmRichRing* ring = (CbmRichRing*) (fRichRings->At(iR));
       	 		if (NULL == ring) continue;
				CbmTrackMatchNew* ringMatch = (CbmTrackMatchNew*) fRichRingMatches->At(iR);
				if(NULL == ringMatch) continue;
				Double_t cX = ring->GetCenterX();
    			Double_t cY = ring->GetCenterY();
				Double_t radius = ring->GetRadius();
				if(radius>0)
				{
					fHM->H2("fh_dis_rich_hits_single_event10")->Fill(cX, cY);
					fcX.push_back(cX);
					fcY.push_back(cY);
					fradius.push_back(radius);
				}	
			int nofHits = ring->GetNofHits();

				for (int iH = 0; iH < nofHits; iH++)

				{
            		Int_t hitInd = ring->GetHit(iH);
            		CbmRichHit* hit = (CbmRichHit*) fRichHits->At(hitInd);
            		if (NULL == hit) continue;
            		Double_t hitX = hit->GetX();
            		Double_t hitY = hit->GetY();
            		Double_t dR = radius - TMath::Sqrt( (cX - hitX)*( cX - hitX) + (cY - hitY)*(cY - hitY) );
            		fHM->H1("fh_dR_10")->Fill(dR);
				
        		}
				
			}
		
		}
cout << fradius[4] << endl;
		Int_t counterpionplus=0;
		Int_t counterpionminus=0;
		Int_t countermyonminus=0;
		Int_t counterpositron=0;
		Int_t counterelectron=0;
		Int_t countermyonplus=0;

	for (Int_t i=0; i<nofRefPlanePoints; i++)
	{
		CbmRichPoint* pRefPlane = (CbmRichPoint*) (fRefPlanePoints->At(i));
        
		Int_t trackid = pRefPlane->GetTrackID();
		if( trackid < 0) continue;
		CbmMCTrack* mctrack= (CbmMCTrack*) (fMCTracks->At(trackid));
		if(mctrack ==NULL) continue;
		Int_t pdg = mctrack->GetPdgCode();
		fHM->H1("fh_sensplane_pdg")->Fill(pdg);
		if(pdg == 211)
		{
			counterpionplus ++;
			fHM->H1("fh_pions+_per_event")->Fill(counterpionplus); 
		}
		if(pdg == -211)
		{
			counterpionminus ++;
			fHM->H1("fh_pions-_per_event")->Fill(counterpionminus); 
		}
		if(pdg == -11)
		{
			counterelectron ++;
			fHM->H1("fh_electrons_per_event")->Fill(counterelectron); 
		}
		if(pdg == 11)
		{
			counterpositron ++;
			fHM->H1("fh_positrons_per_event")->Fill(counterpositron); 
		}
		if(pdg == -13)
		{
			countermyonminus ++;
			fHM->H1("fh_myons-_per_event")->Fill(countermyonminus); 
		}
		if(pdg == 13)
		{
			countermyonplus ++;
			fHM->H1("fh_myons+_per_event")->Fill(countermyonplus);
		}
		
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
	fHM->Create1<TH1D>("fh_nof_rich_points", "fh_nof_rich_points;Nof Rich Points;Yield (a.u.)", 1000, 0., 1000.);
	fHM->Create1<TH1D>("fh_nof_rich_digis", "fh_nof_rich_digis;Nof Rich Digis;Yield (a.u.)", 250, 0., 1000.);
	fHM->Create1<TH1D>("fh_nof_rich_hits", "fh_nof_rich_hits;Nof Rich hits;Yield (a.u.)", 100, 0., 100.);
	fHM->Create1<TH1D>("fh_nof_rich_rings", "fh_nof_rich_rings;Nof Rich rings;Yield (a.u.)", 5, -0.5, 4.5);
	fHM->Create1<TH1D>("fh_nof_good_rings", "fh_nof_good_rings;Nof good rings;Yield (a.u.)", 5, -0.5, 4.5);

	fHM->Create1<TH1D>("fh_rich_ring_radius","fh_rich_ring_radius; Ring Radius [cm]; Yield (a.u.)", 160, 2., 10.);
	
	fHM->Create2<TH2D>("fh_dis_rich_points", "fh_dis_rich_points; x [cm]; y [cm]", 1000, -10., 10., 700, -10., 10.);

	Float_t XBins[] = {-7.725, -7.11875, -6.5125, -5.90625, -5.3, -4.69375, -4.0875, -3.48125, -2.875, -2.425, -1.81875, -1.2125, -0.60625, 0., 0.60625, 1.2125, 1.81875, 2.425, 2.875, 3.48125, 4.0875, 4.69375, 5.3, 5.90625, 6.5125, 7.11875, 7.725}; 
	Float_t YBins[] = {-12.075, -11.46875, -10.8625, -10.25625, -9.65, -9.04375, -8.4375, -7.83125, -7.225, -6.775, -6.16875, -5.5625, -4.95625, -4.35, -3.74375, -3.1375, -2.53125, -1.925, 1.925, 2.53125, 3.1375, 3.74375, 4.35, 4.95625, 5.5625, 6.16875, 6.775, 7.225, 7.83125, 8.4375, 9.04375, 9.65, 10.25625, 10.8625, 11.46875, 12.075}; 
	Int_t NBinsX = sizeof(XBins)/sizeof(Float_t) - 1; 
	Int_t NBinsY = sizeof(YBins)/sizeof(Float_t) - 1;
	TH2D* histbins = new TH2D("fh_dis_rich_hits", "fh_dis_rich_hits; x[cm]; y[cm]", NBinsX, XBins, NBinsY, YBins);
	fHM->Add("fh_dis_rich_hits", histbins);

	fHM->Create2<TH2D>("fh_dis_rich_hits_single_event1", "fh_dis_rich_hits_single_event1; x[cm]; y[cm]", 50, -10., 10., 50, -10., 10.);
	fHM->Create2<TH2D>("fh_dis_rich_hits_single_event2", "fh_dis_rich_hits_single_event2; x[cm]; y[cm]", 50, -10., 10., 50, -10., 10.);
	fHM->Create2<TH2D>("fh_dis_rich_hits_single_event3", "fh_dis_rich_hits_single_event3; x[cm]; y[cm]", 50, -10., 10., 50, -10., 10.);
	fHM->Create2<TH2D>("fh_dis_rich_hits_single_event4", "fh_dis_rich_hits_single_event4; x[cm]; y[cm]", 50, -10., 10., 50, -10., 10.);
	fHM->Create2<TH2D>("fh_dis_rich_hits_single_event5", "fh_dis_rich_hits_single_event5; x[cm]; y[cm]", 50, -10., 10., 50, -10., 10.);	
	fHM->Create2<TH2D>("fh_dis_rich_hits_single_event6", "fh_dis_rich_hits_single_event6; x[cm]; y[cm]", 50, -10., 10., 50, -10., 10.);	
	fHM->Create2<TH2D>("fh_dis_rich_hits_single_event7", "fh_dis_rich_hits_single_event7; x[cm]; y[cm]", 50, -10., 10., 50, -10., 10.);	
	fHM->Create2<TH2D>("fh_dis_rich_hits_single_event8", "fh_dis_rich_hits_single_event8; x[cm]; y[cm]", 50, -10., 10., 50, -10., 10.);	
	fHM->Create2<TH2D>("fh_dis_rich_hits_single_event9", "fh_dis_rich_hits_single_event9; x[cm]; y[cm]", 50, -10., 10., 50, -10., 10.);	
	fHM->Create2<TH2D>("fh_dis_rich_hits_single_event10", "fh_dis_rich_hits_single_event10; x[cm]; y[cm]", 50, -10., 10., 50, -10., 10.);

	fHM->Create1<TH1D>("fh_dR_1", "fh_dR_1; dR[cm]; Yield(a.u.)", 80, -0.8, 0.8);	
	fHM->Create1<TH1D>("fh_dR_2", "fh_dR_2; dR[cm]; Yield(a.u.)", 80, -0.8, 0.8);	
	fHM->Create1<TH1D>("fh_dR_3", "fh_dR_3; dR[cm]; Yield(a.u.)", 80, -0.8, 0.8);	
	fHM->Create1<TH1D>("fh_dR_4", "fh_dR_4; dR[cm]; Yield(a.u.)", 80, -0.8, 0.8);	
	fHM->Create1<TH1D>("fh_dR_5", "fh_dR_5; dR[cm]; Yield(a.u.)", 80, -0.8, 0.8);	
	fHM->Create1<TH1D>("fh_dR_6", "fh_dR_6; dR[cm]; Yield(a.u.)", 80, -0.8, 0.8);	
	fHM->Create1<TH1D>("fh_dR_7", "fh_dR_7; dR[cm]; Yield(a.u.)", 80, -0.8, 0.8);	
	fHM->Create1<TH1D>("fh_dR_8", "fh_dR_8; dR[cm]; Yield(a.u.)", 80, -0.8, 0.8);	
	fHM->Create1<TH1D>("fh_dR_9", "fh_dR_9; dR[cm]; Yield(a.u.)", 80, -0.8, 0.8);	
	fHM->Create1<TH1D>("fh_dR_10", "fh_dR_10; dR[cm]; Yield(a.u.)", 80, -0.8, 0.8);

	fHM->Create1<TH1D>("fh_pions+_per_event", "fh_pions+_per_event; Number of Pions(+); Yield", 5, -0.5, 4.5);
	fHM->Create1<TH1D>("fh_pions-_per_event", "fh_pions-_per_event; Number of Pions(-); Yield", 5, -0.5, 4.5);	
	fHM->Create1<TH1D>("fh_myons-_per_event", "fh_myons-_per_event; Number of Myons(-); Yield", 5, -0.5, 4.5);
	fHM->Create1<TH1D>("fh_electrons_per_event", "fh_electrons_per_event; Number of Electrons; Yield", 5, -0.5, 4.5);	
	fHM->Create1<TH1D>("fh_positrons_per_event", "fh_positrons_per_event; Number of Positrons; Yield", 5, -0.5, 4.5);
	fHM->Create1<TH1D>("fh_myons+_per_event", "fh_myons+_per_event; Number of Myons (+); Yield", 5, -0.5, 4.5);
	

	fHM->Create2<TH2D>("fh_dis_rich_hits_dR>", "fh_dis_rich_hits_dR>; x[cm]; y[cm]", 50, -10., 10., 50, -10., 10.);
	fHM->Create2<TH2D>("fh_dis_rich_hits_dR<", "fh_dis_rich_hits_dR<; x[cm]; y[cm]", 50, -10., 10., 50, -10., 10.);
	fHM->Create1<TH1D>("fh_hits_per_ring", "fh_hits_per_ring;Hits per Ring;Yield (a.u.)", 58, 2.5, 60.5);
	
	fHM->Create2<TH2D>("fh_cX_cY","fh_cX_cY;cX[cm];cY[cm]", 100, -10, 10, 100, -10, 10);	
	fHM->Create2<TH2D>("fh_hits_per_ring_per_ring","fh_hits_per_ring_per_ring; Ring Number; Nof Hits", 10, 0., 10., 40., 0., 40.);
	fHM->Create2<TH2D>("fh_radius_ring","fh_radius_ring;Ring Number;Radius[cm]", 10, 0., 10., 140 , 3., 10. );
	
	fHM->Create2<TH2D>("fh_proton_startxy","fh_proton_startxy; x [cm]; y[cm];", 100, -5.,5, 100, -5., 5.);

	fHM->Create1<TH1D>("fh_dR","fh_dR; dR [cm]; Yield(a.u.)", 80, -0.8, 0.8);

	fHM->Create2<TH2D>("fh_dis_primsec", "fh_dis_primsec; x [cm]; y [cm];", 100, -100., 100, 100, -100., 100.);
	fHM->Create2<TH2D>("fh_dis_prim", "fh_dis_prim; x [cm]; y [cm];", 100, -5., 5, 100, -5., 5.);
	fHM->Create2<TH2D>("fh_dis_sec", "fh_dis_sec; x [cm]; y [cm];", 100, -5., 5, 100, -5., 5.);
	fHM->Create1<TH1D>("fh_sensplane_pdg", "fh_sensplane_pdg;PDG Codes;Yield (a.u.)", 6000, -3000., 3000.);

	fHM->Create2<TH2D>("fh_dis_pmt_protons","fh_dis_pmt_protons; x [cm]; y [cm]", 50, -10., 10., 50, -10., 10.);

	fHM->Create2<TH2D>("fh_focus","fh_focus; pmt-lense distance d [cm]; RMS", 7, 2.7, 3.3, 40, 0., 0.4);
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
		TCanvas* c=CreateCanvas("rich_dis_of_points", "rich_dis_of_points", 600, 400);
		c->SetLogz();
		DrawH2(fHM->H2("fh_dis_rich_points"));
		fHM->H2("fh_dis_rich_points")->SetTitle("Rich Points");
	}
	{
		TCanvas* c=CreateCanvas("rich_dis_of_hits", "rich_dis_of_hits", 600, 400);
		c->SetLogz();
		DrawH2(fHM->H2("fh_dis_rich_hits"));
		fHM->H2("fh_dis_rich_hits")->SetTitle("Rich Hits");

	}

	{
		TCanvas* c=CreateCanvas("rich_dis_hits_single_events", "rich_dis_hits_single_events", 1800, 1200);
		c->Divide(4,3);
		c->cd(1);
		DrawH2(fHM->H2("fh_dis_rich_hits_single_event1"));
		fHM->H2("fh_dis_rich_hits_single_event1")->SetTitle("Rich Hits Single Event1");
		TEllipse *ringfit= new TEllipse(fcX[0],fcY[0],fradius[0],fradius[0]);
		ringfit->SetFillStyle(0);
		ringfit->Draw("SAME");

		c->cd(2);
		DrawH2(fHM->H2("fh_dis_rich_hits_single_event2"));
		fHM->H2("fh_dis_rich_hits_single_event2")->SetTitle("Rich Hits Single Event2");
		TEllipse *ringfit1= new TEllipse(fcX[1],fcY[1],fradius[1],fradius[1]);
		ringfit1->SetFillStyle(0);
		ringfit1->Draw("SAME");

		c->cd(3);
		DrawH2(fHM->H2("fh_dis_rich_hits_single_event3"));
		fHM->H2("fh_dis_rich_hits_single_event3")->SetTitle("Rich Hits Single Event3");
		TEllipse *ringfit2= new TEllipse(fcX[2],fcY[2],fradius[2],fradius[2]);
		ringfit2->SetFillStyle(0);
		ringfit2->Draw("SAME");

		c->cd(4);
		DrawH2(fHM->H2("fh_dis_rich_hits_single_event4"));
		fHM->H2("fh_dis_rich_hits_single_event4")->SetTitle("Rich Hits Single Event4");
		TEllipse *ringfit3= new TEllipse(fcX[3],fcY[3],fradius[3],fradius[3]);
		ringfit3->SetFillStyle(0);
		ringfit3->Draw("SAME");

		c->cd(5);
		DrawH2(fHM->H2("fh_dis_rich_hits_single_event5"));
		fHM->H2("fh_dis_rich_hits_single_event5")->SetTitle("Rich Hits Single Event5");
		TEllipse *ringfit4= new TEllipse(fcX[4],fcY[4],fradius[4],fradius[04]);
		ringfit4->SetFillStyle(0);
		ringfit4->Draw("SAME");

		c->cd(6);
		DrawH2(fHM->H2("fh_dis_rich_hits_single_event6"));
		fHM->H2("fh_dis_rich_hits_single_event6")->SetTitle("Rich Hits Single Event6");
		TEllipse *ringfit5= new TEllipse(fcX[5],fcY[5],fradius[5],fradius[5]);
		ringfit5->SetFillStyle(0);
		ringfit5->Draw("SAME");

		c->cd(7);
		DrawH2(fHM->H2("fh_dis_rich_hits_single_event7"));
		fHM->H2("fh_dis_rich_hits_single_event7")->SetTitle("Rich Hits Single Event7");
		TEllipse *ringfit6= new TEllipse(fcX[6],fcY[6],fradius[6],fradius[6]);
		ringfit6->SetFillStyle(0);
		ringfit6->Draw("SAME");

		c->cd(8);
		DrawH2(fHM->H2("fh_dis_rich_hits_single_event8"));
		fHM->H2("fh_dis_rich_hits_single_event8")->SetTitle("Rich Hits Single Event8");
		TEllipse *ringfit7= new TEllipse(fcX[7],fcY[7],fradius[7],fradius[7]);
		ringfit7->SetFillStyle(0);
		ringfit7->Draw("SAME");

		c->cd(9);
		DrawH2(fHM->H2("fh_dis_rich_hits_single_event9"));
		fHM->H2("fh_dis_rich_hits_single_event9")->SetTitle("Rich Hits Single Event9");
		TEllipse *ringfit8= new TEllipse(fcX[8],fcY[8],fradius[8],fradius[8]);
		ringfit8->SetFillStyle(0);
		ringfit8->Draw("SAME");

		c->cd(10);
		DrawH2(fHM->H2("fh_dis_rich_hits_single_event10"));
		fHM->H2("fh_dis_rich_hits_single_event10")->SetTitle("Rich Hits Single Event10");
		TEllipse *ringfit9= new TEllipse(fcX[9],fcY[9],fradius[9],fradius[9]);
		ringfit9->SetFillStyle(0);
		ringfit9->Draw("SAME");
		
	}

	{
		TCanvas* c=CreateCanvas("rich_dR_single_events", "rich_dR_single_events", 1800, 1200);
		c->Divide(4,3);
		c->cd(1);
		DrawH1(fHM->H1("fh_dR_1"));
		fHM->H1("fh_dR_1")->SetTitle("dR Single Event1");
	
		c->cd(2);
		DrawH1(fHM->H1("fh_dR_2"));
		fHM->H1("fh_dR_2")->SetTitle("dR Single Event2");

		c->cd(3);
		DrawH1(fHM->H1("fh_dR_3"));
		fHM->H1("fh_dR_3")->SetTitle("dR Single Event3");

		c->cd(4);
		DrawH1(fHM->H1("fh_dR_4"));
		fHM->H1("fh_dR_4")->SetTitle("dR Single Event4");

		c->cd(5);
		DrawH1(fHM->H1("fh_dR_5"));
		fHM->H1("fh_dR_5")->SetTitle("dR Single Event5");
	
		c->cd(6);
		DrawH1(fHM->H1("fh_dR_6"));
		fHM->H1("fh_dR_6")->SetTitle("dR Single Event6");

		c->cd(7);
		DrawH1(fHM->H1("fh_dR_7"));
		fHM->H1("fh_dR_7")->SetTitle("dR Single Event7");

		c->cd(8);
		DrawH1(fHM->H1("fh_dR_8"));
		fHM->H1("fh_dR_8")->SetTitle("dR Single Event8");

		c->cd(9);
		DrawH1(fHM->H1("fh_dR_9"));
		fHM->H1("fh_dR_9")->SetTitle("dR Single Event9");
	
		c->cd(10);
		DrawH1(fHM->H1("fh_dR_10"));
		fHM->H1("fh_dR_10")->SetTitle("dR Single Event10");
	
	}

	{
		TCanvas* c=CreateCanvas("particles_per_event", "particles_per_event", 1600, 800);
		c->Divide(3,2);
		c->cd(1);
		DrawH1(fHM->H1("fh_pions+_per_event"));
		fHM->H1("fh_pions+_per_event")->SetTitle("Pions(+) per Event)");
		c->cd(2);
		DrawH1(fHM->H1("fh_pions-_per_event"));
		fHM->H1("fh_pions-_per_event")->SetTitle("Pions(-) per Event)");
		c->cd(3);
		DrawH1(fHM->H1("fh_myons-_per_event"));
		fHM->H1("fh_myons-_per_event")->SetTitle("Myons(-) per Event)");
		c->cd(4);
		DrawH1(fHM->H1("fh_electrons_per_event"));
		fHM->H1("fh_electrons_per_event")->SetTitle("Electrons per Event)");
		c->cd(5);
		DrawH1(fHM->H1("fh_positrons_per_event"));
		fHM->H1("fh_positrons_per_event")->SetTitle("Protons per Event)");
		c->cd(6);
		DrawH1(fHM->H1("fh_myons+_per_event"));
		fHM->H1("fh_myons+_per_event")->SetTitle("Myons (+) per Event");

	}
	{
		TCanvas* c=CreateCanvas("rich_dis_hits_dR","rich_dis_hits_dR", 800, 400);
		c->Divide(2,1);
		c->cd(1);
		c->SetLogz();
		DrawH2(fHM->H2("fh_dis_rich_hits_dR>"));
		fHM->H2("fh_dis_rich_hits_dR>")->SetTitle("Rich Hits dR>");
		c->cd(2);
		c->SetLogz();
		DrawH2(fHM->H2("fh_dis_rich_hits_dR<"));
		fHM->H2("fh_dis_rich_hits_dR<")->SetTitle("Rich Hits dR<");
	}

 
	{
		TCanvas* c=CreateCanvas("fh_proton_startxy", "fh_proton_startxy", 400, 400);
		c->SetLogz();
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
		//DrawH1(fHM->H1("fh_rich_ring_radius"));
		//fHM->H1("fh_rich_ring_radius")->Fit("gaus","Q","", 5.,5.8);
		//TF1* fit=fHM->H1("fh_rich_ring_radius")->GetFunction("gaus");
		//fit->SetLineColor(kBlack);
		fHM->H1("fh_rich_ring_radius")->SetTitle("Ring Radius");	
	}
	
	{
		TCanvas* c=CreateCanvas("fh_dR", "fh_dR", 400, 400);
		DrawH1andFitGauss(fHM->H1("fh_dR"));
		fHM->H1("fh_dR")->SetTitle("dR");
	}
	
	{
		TCanvas* c=CreateCanvas("fh_dis_sec", "fh_dis_sec", 400, 400);
		c->SetLogz();
		DrawH2(fHM->H2("fh_dis_sec"));
		fHM->H2("fh_dis_sec")->SetTitle("Secondaries");
	}
	
	{
		TCanvas* c=CreateCanvas("fh_dis_prim", "fh_dis_prim", 400, 400);
		c->SetLogz();
		DrawH2(fHM->H2("fh_dis_prim"));
		fHM->H2("fh_dis_prim")->SetTitle("Primaries");
	}
	
	{
		TCanvas* c=CreateCanvas("fh_dis_primsec", "fh_dis_primsec", 400, 400);
		c->SetLogz();
		DrawH2(fHM->H2("fh_dis_primsec"));
		fHM->H2("fh_dis_primsec")->SetTitle("Primaries and Secondaries");
	}
	
	{
		TCanvas* c=CreateCanvas("fh_dis_pmt_protons", "fh_dis_pmt_protons", 400, 400);
		c->SetLogz();
		DrawH2(fHM->H2("fh_dis_pmt_protons"));
		fHM->H2("fh_dis_pmt_protons")->SetTitle("Protons on PMT");
	}

	{
		TCanvas* c=CreateCanvas("fh_sensplane_pdg", "fh_sensplane_pdg", 1600, 400);
		c->SetLogy();
		DrawH1(fHM->H1("fh_sensplane_pdg"));
		fHM->H1("fh_sensplane_pdg")->SetTitle("PDGs on sens_plane");
	}

	{
		TCanvas* c=CreateCanvas("fh_hits_per_ring", "fh_hits_per_ring", 400, 400);
		DrawH1andFitGauss(fHM->H1("fh_hits_per_ring"));
		//DrawH1(fHM->H1("fh_hits_per_ring"));
		//fHM->H1("fh_hits_per_ring")->Fit("gaus","Q","", 12.,55.);
		//TF1* fit=fHM->H1("fh_hits_per_ring")->GetFunction("gaus");
		//fit->SetLineColor(kBlack);
		fHM->H1("fh_hits_per_ring")->SetTitle("Hits per Ring");
	}

	{
		TCanvas* c=CreateCanvas("fh_cX_cY", "fh_cX_cY", 400, 400);
		c->SetLogz();
		DrawH2(fHM->H2("fh_cX_cY"));
		fHM->H1("fh_cX_cY")->SetTitle("Ringcenter");
	}
	
	{
		TCanvas* c=CreateCanvas("fh_hits_per_ring_per_ring", "fh_hits_per_ring_per_ring", 400, 400);
		c->SetLogz();
		DrawH2(fHM->H2("fh_hits_per_ring_per_ring"));
		fHM->H2("fh_hits_per_ring_per_ring")->SetTitle("Hits per single Ring");
	}

	{
		TCanvas* c=CreateCanvas("fh_radius_ring", "fh_radius_ring", 400, 400);
		c->SetLogz();
		DrawH2(fHM->H2("fh_radius_ring"));
		fHM->H1("fh_radius_ring")->SetTitle("Radius per single Ring");
	}
	
	{
		TCanvas* c=CreateCanvas("fh_nof_good_rings", "fh_nof_good_rings", 400, 400);
		DrawH1(fHM->H1("fh_nof_good_rings"));
		fHM->H1("fh_nof_good_rings")->SetTitle("Nof good Rings");
	}
	{
		TCanvas* c=CreateCanvas("fh_focus", "fh_focus", 400, 400);
		DrawH2(fHM->H2("fh_focus"));
		fHM->H2("fh_focus")->SetTitle("Beam Focus");
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

