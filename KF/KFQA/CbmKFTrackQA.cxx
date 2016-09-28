//-----------------------------------------------------------
//-----------------------------------------------------------

// Cbm Headers ----------------------
#include "CbmKFTrackQA.h"
#include "CbmTrack.h"
#include "CbmMCTrack.h"
#include "CbmTrackMatchNew.h"

#include "FairRunAna.h"

//KF Particle headers
#include "KFParticleTopoReconstructor.h"
#include "KFTopoPerformance.h"
#include "KFMCTrack.h"
#include "KFParticleMatch.h"
#include "CbmL1PFFitter.h"
#include "L1Field.h"
#include "CbmKFVertex.h"

//ROOT headers
#include "TClonesArray.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TObject.h"
#include "TMath.h"
#include "TDatabasePDG.h"

//c++ and std headers
#include <iostream>
#include <iomanip>
#include <cmath>

#include "CbmStsTrack.h"
#include "CbmGlobalTrack.h"
#include "CbmMuchTrack.h"
#include "CbmTrack.h"
#include "CbmRichRing.h"

using std::vector;
using std::map;

CbmKFTrackQA::CbmKFTrackQA(const char* name, Int_t iVerbose, TString outFileName):
  FairTask(name, iVerbose), fStsTrackBranchName("StsTrack"), fGlobalTrackBranchName("GlobalTrack"), fRichBranchName("RichRing"),
  fTrdBranchName ("TrdTrack"), fTofBranchName("TofHit"), fMuchTrackBranchName("MuchTrack"), fMCTracksBranchName("MCTrack"),
  fStsTrackMatchBranchName("StsTrackMatch"), fRichRingMatchBranchName("RichRingMatch"), fTrdTrackMatchBranchName("TrdTrackMatch"),
  fTofHitMatchBranchName("TofHitMatch"), fMuchTrackMatchBranchName("MuchTrackMatch"),    
  fStsTrackArray(0), fGlobalTrackArray(0), fRichRingArray(0), fTrdTrackArray(0), fTofHitArray(0), fMuchTrackArray(0),
  fMCTrackArray(0), fStsTrackMatchArray(0), fRichRingMatchArray(0), fTrdTrackMatchArray(0),
  fTofHitMatchArray(0), fMuchTrackMatchArray(0),   
  fOutFileName(outFileName), fOutFile(0), fHistoDir(0), fNEvents(0),
  fPDGtoIndexMap()
{ 
  TFile* curFile = gFile;
  TDirectory* curDirectory = gDirectory;
  
  if(!(fOutFileName == ""))
    fOutFile = new TFile(fOutFileName.Data(),"RECREATE");
  else
    fOutFile = gFile;
  
  fOutFile->cd();
  fHistoDir = fOutFile->mkdir( "KFTrackQA" );
  fHistoDir->cd();
  
  gDirectory->mkdir("STS");
  gDirectory->cd("STS");
  {
    TString histoName[NStsHisto] = {"NHits", "chi2/NDF", "prob"};
    TString axisName[NStsHisto] = {"N hits", "#chi^{2}/NDF", "prob"};
    int nBins[NStsHisto] = {16, 100, 100};
    float xMin[NStsHisto] = {-0.5, 0.f,  0.f};
    float xMax[NStsHisto] = {15.5, 20.f, 1.f};
    
    TString subdirs[8] = {"Tracks", "e", "mu", "pi", "K", "p", "fragments", "ghost"};

    for(int iDir=0; iDir<8; iDir++)
    {
      gDirectory->mkdir(subdirs[iDir].Data());
      gDirectory->cd(subdirs[iDir].Data());
      {
        gDirectory->mkdir("TrackFitQA");
        gDirectory->cd("TrackFitQA");
        {
          TString res = "res";
          TString pull = "pull";
          
          TString parName[5] = {"X","Y","Tx","Ty","QP"};
          int nBinsFit = 100;
//          float xMaxFit[5] = {0.15,0.15,0.01,0.01,3.5};
          float xMaxFit[5] = {0.05,0.045,0.01,0.01,0.1};
  
          for( int iH=0; iH<5; iH++ )
          {
            hStsFitHisto[iDir][iH]   = new TH1F((res+parName[iH]).Data(),
                                                (res+parName[iH]).Data(), 
                                                nBinsFit, -xMaxFit[iH],xMaxFit[iH]);
            hStsFitHisto[iDir][iH+5] = new TH1F((pull+parName[iH]).Data(),
                                                (pull+parName[iH]).Data(), 
                                                nBinsFit, -10,10);
          }
        }
        gDirectory->cd("..");
        
        for(int iH=0; iH<NStsHisto; iH++)
        {
          hStsHisto[iDir][iH] = new TH1F(histoName[iH].Data(), histoName[iH].Data(), nBins[iH], xMin[iH], xMax[iH]);
          hStsHisto[iDir][iH] ->GetXaxis()->SetTitle(axisName[iH].Data());
        }
      }
      gDirectory->cd(".."); //STS
    }
  }
  gDirectory->cd("..");

  gDirectory->mkdir("MuCh");
  gDirectory->cd("MuCh");
  {
    TString histoName[NMuchHisto] = {"NHits", "FirstStation", "LastStation", "chi2/NDF", "prob"};
    TString axisName[NMuchHisto] = {"N hits", "First Station", "Last Station", "#chi^{2}/NDF", "prob"};
    int nBins[NMuchHisto] = {16, 16, 16, 100, 100};
    float xMin[NMuchHisto] = {-0.5f, -0.5f, -0.5f, 0.f,  0.f};
    float xMax[NMuchHisto] = {15.5f, 15.5f, 15.5f, 20.f, 1.f};
    
    TString subdirs[3] = {"mu", "Background", "Ghost"};
    
    for(int iDir=0; iDir<3; iDir++)
    {
      gDirectory->mkdir(subdirs[iDir].Data());
      gDirectory->cd(subdirs[iDir].Data());
      for(int iH=0; iH<NMuchHisto; iH++)
      {
        hMuchHisto[iDir][iH] = new TH1F(histoName[iH].Data(), histoName[iH].Data(), nBins[iH], xMin[iH], xMax[iH]);
        hMuchHisto[iDir][iH] ->GetXaxis()->SetTitle(axisName[iH].Data());
      }
      gDirectory->cd(".."); //MuCh
    }
  }
  gDirectory->cd("..");
  
  gDirectory->mkdir("RICH");
  gDirectory->cd("RICH");
  {
    TString subdirs[10] = {"AllTracks", "e", "mu", "pi", "K", "p", "Fragments", "Mismatch", "GhostTrack", "GhostRing"};
    TString histoName2D[NRichRingHisto2D] = {"r", "axisA", "axisB"};
    
    for(int iDir=0; iDir<10; iDir++)
    {
      gDirectory->mkdir(subdirs[iDir]);
      gDirectory->cd(subdirs[iDir]);
      for(int iH=0; iH<NRichRingHisto2D; iH++)
      {
        hRichRingHisto2D[iDir][iH] = new TH2F(histoName2D[iH], histoName2D[iH], 1000, 0, 15., 1000, 0, 10.);
        hRichRingHisto2D[iDir][iH]->GetYaxis()->SetTitle(histoName2D[iH]+TString(" [cm]"));
        hRichRingHisto2D[iDir][iH]->GetXaxis()->SetTitle("p [GeV/c]");
      }
      gDirectory->cd(".."); //RICH
    }
  }
  gDirectory->cd("..");
  
  gFile = curFile;
  gDirectory = curDirectory;
  
  fPDGtoIndexMap[  11] = 1;
  fPDGtoIndexMap[  13] = 2;
  fPDGtoIndexMap[ 211] = 3;
  fPDGtoIndexMap[ 321] = 4;
  fPDGtoIndexMap[2212] = 5;
}

CbmKFTrackQA::~CbmKFTrackQA()
{
}

InitStatus CbmKFTrackQA::Init()
{
  //Get ROOT Manager
  FairRootManager* ioman= FairRootManager::Instance();
  
  if(ioman==0)
  {
    Warning("CbmKFTrackQA::Init","RootManager not instantiated!");
    return kERROR;
  }
  
  // Get sts tracks
  fStsTrackArray=(TClonesArray*) ioman->GetObject(fStsTrackBranchName);
  if(fStsTrackArray==0)
  {
    Warning("CbmKFTrackQA::Init","track-array not found!");
    return kERROR;
  }
  
  // Get global tracks
  fGlobalTrackArray=(TClonesArray*) ioman->GetObject(fGlobalTrackBranchName);
  if(fGlobalTrackArray==0)
    Warning("CbmKFTrackQA::Init","global track array not found!");
  
  // Get ToF hits
  fTofHitArray=(TClonesArray*) ioman->GetObject(fTofBranchName);
  if(fTofHitArray==0)
    Warning("CbmKFTrackQA::Init","TOF track-array not found!");
  
  // TRD
  fTrdTrackArray = (TClonesArray*) ioman->GetObject(fTrdBranchName);
  if(fTrdTrackArray==0)
    Warning("CbmKFTrackQA::Init","TRD track-array not found!");
    
  fRichRingArray = (TClonesArray*) ioman->GetObject(fRichBranchName);
  if(fRichRingArray == 0)
    Warning("CbmKFTrackQA::Init","Rich ring array not found!");
  
  fMCTrackArray=(TClonesArray*) ioman->GetObject(fMCTracksBranchName);
  if(fMCTrackArray==0)
  {
    Warning("CbmKFTrackQA::Init","mc track array not found!");
    return kERROR;
  }
  
  //Track match
  fStsTrackMatchArray=(TClonesArray*) ioman->GetObject(fStsTrackMatchBranchName);
  if(fStsTrackMatchArray==0)
  {
    Warning("CbmKFTrackQA::Init","track match array not found!");
    return kERROR;
  }

  //Ring match
  fRichRingMatchArray=(TClonesArray*) ioman->GetObject(fRichRingMatchBranchName);
  if(fRichRingMatchArray==0)
    Warning("CbmKFTrackQA::Init","RichRing match array not found!");
  
  //TRD match
  fTrdTrackMatchArray=(TClonesArray*) ioman->GetObject(fTrdTrackMatchBranchName);
  if(fTrdTrackMatchArray==0)
    Warning("CbmKFTrackQA::Init","TrdTrack match array not found!");
  
  //Much track match
  fMuchTrackMatchArray=(TClonesArray*) ioman->GetObject(fMuchTrackMatchBranchName);
  if(fMuchTrackMatchArray==0)
  {
    Warning("CbmKFTrackQA::Init","Much track match array not found!");
  }
  //Much
  fMuchTrackArray = (TClonesArray*) ioman->GetObject(fMuchTrackBranchName);
  if (fMuchTrackArray == 0)
  {
    Warning("CbmKFTrackQA::Init", "Much track-array not found!");
  }
    
  return kSUCCESS;
}

void CbmKFTrackQA::Exec(Option_t* /*opt*/)
{ 
  fNEvents++;
  
  Int_t nMCTracks = fMCTrackArray->GetEntriesFast();
  vector<KFMCTrack> mcTracks(nMCTracks);
  for(Int_t iMC=0; iMC<nMCTracks; iMC++)
  {
    CbmMCTrack *cbmMCTrack = (CbmMCTrack*)fMCTrackArray->At(iMC);

    
    mcTracks[iMC].SetX ( cbmMCTrack->GetStartX() );
    mcTracks[iMC].SetY ( cbmMCTrack->GetStartY() );
    mcTracks[iMC].SetZ ( cbmMCTrack->GetStartZ() );
    mcTracks[iMC].SetPx( cbmMCTrack->GetPx() );
    mcTracks[iMC].SetPy( cbmMCTrack->GetPy() );
    mcTracks[iMC].SetPz( cbmMCTrack->GetPz() );
    
    Int_t pdg = cbmMCTrack->GetPdgCode();
    Double_t q=1;
    if ( pdg < 9999999 && ( (TParticlePDG *)TDatabasePDG::Instance()->GetParticle(pdg) ))
      q = TDatabasePDG::Instance()->GetParticle(pdg)->Charge()/3.0;
    else if(pdg ==  1000010020) q =  1;
    else if(pdg == -1000010020) q = -1;
    else if(pdg ==  1000010030) q =  1;
    else if(pdg == -1000010030) q = -1;
    else if(pdg ==  1000020030) q =  2;
    else if(pdg == -1000020030) q = -2;
    else if(pdg ==  1000020040) q =  2;
    else if(pdg == -1000020040) q = -2;
    else q = 0;
    Double_t p = cbmMCTrack->GetP();
    
    mcTracks[iMC].SetMotherId(cbmMCTrack->GetMotherId());
    mcTracks[iMC].SetQP(q/p);
    mcTracks[iMC].SetPDG(pdg);
    mcTracks[iMC].SetNMCPoints(0);
  }
  
  Int_t ntrackMatches=fStsTrackMatchArray->GetEntriesFast();
  vector<int> trackMatch(ntrackMatches, -1);

  for(int iTr=0; iTr<ntrackMatches; iTr++)
  {
    CbmTrackMatchNew* stsTrackMatch = (CbmTrackMatchNew*) fStsTrackMatchArray->At(iTr);
    if(stsTrackMatch -> GetNofLinks() == 0) continue;
    Float_t bestWeight = 0.f;
    Float_t totalWeight = 0.f;
    Int_t mcTrackId = -1;
    for(int iLink=0; iLink<stsTrackMatch -> GetNofLinks(); iLink++)
    {
      totalWeight += stsTrackMatch->GetLink(iLink).GetWeight();
      if( stsTrackMatch->GetLink(iLink).GetWeight() > bestWeight)
      {
        bestWeight = stsTrackMatch->GetLink(iLink).GetWeight();
        mcTrackId = stsTrackMatch->GetLink(iLink).GetIndex();
      }
    }
    if(bestWeight/totalWeight < 0.7) continue;
    if(mcTrackId >= nMCTracks || mcTrackId < 0)
    {
      std::cout << "Sts Matching is wrong!    StsTackId = " << mcTrackId << " N mc tracks = " << nMCTracks << std::endl;
      continue;
    }
    
    mcTracks[mcTrackId].SetReconstructed();
    trackMatch[iTr] = mcTrackId;
  }
  
  //Check fit quality of the STS tracks
  vector<CbmStsTrack> vRTracks(fStsTrackArray->GetEntriesFast());
  vector<int> pdg(fStsTrackArray->GetEntriesFast(), 211);
  for(int iTr=0; iTr<fStsTrackArray->GetEntriesFast(); iTr++)
  {
    CbmStsTrack* stsTrack = ( (CbmStsTrack*) fStsTrackArray->At(iTr));
    vRTracks[iTr] = *stsTrack;
    
    if(trackMatch[iTr] > -1)
      pdg[iTr] = mcTracks[trackMatch[iTr]].PDG();
  }
  
  CbmKFVertex kfVertex;
  
  CbmL1PFFitter fitter;
  vector<float> vChiToPrimVtx;
  vector<L1FieldRegion> vField;
  fitter.Fit(vRTracks, pdg);
  fitter.GetChiToVertex(vRTracks, vField, vChiToPrimVtx, kfVertex, 3000000);

  for(unsigned int iTr=0; iTr<vRTracks.size(); iTr++)
  {
    if(trackMatch[iTr] < 0) continue;
    
    const KFMCTrack& mcTrack = mcTracks[trackMatch[iTr]];
    if(mcTrack.MotherId() > -1) continue;
//    if ( vRTracks[iTr].GetNofHits() < 11 ) continue;
    
    const FairTrackParam* parameters = vRTracks[iTr].GetParamFirst();
    
    Double_t recoParam[5] = { parameters->GetX(), parameters->GetY(), parameters->GetTx(), parameters->GetTy(), parameters->GetQp() };
    Double_t recoError[5] = { parameters->GetCovariance(0,0), parameters->GetCovariance(1,1), parameters->GetCovariance(2,2),
                           parameters->GetCovariance(3,3), parameters->GetCovariance(4,4)  };
    Double_t mcParam[5] = {mcTrack.X(), mcTrack.Y(), mcTrack.Px()/mcTrack.Pz(), mcTrack.Py()/mcTrack.Pz(), mcTrack.Par()[6] };
    
    int iDir = GetHistoIndex(mcTrack.PDG());
    
    for(int iParam=0; iParam<5; iParam++)
    {
      Double_t residual = recoParam[iParam] - mcParam[iParam];
      if(iParam==4)
      {
        Double_t pReco = fabs(1./recoParam[iParam]);
        Double_t pMC = fabs(1./mcParam[iParam]);
        
        hStsFitHisto[0][iParam]->Fill( (pReco - pMC)/pMC );
        hStsFitHisto[iDir][iParam]->Fill( (pReco - pMC)/pMC );
      }
      else
      {
        hStsFitHisto[0][iParam]->Fill(residual);
        hStsFitHisto[iDir][iParam]->Fill(residual);
      }
      
      if( recoError[iParam] >=0. )
      {
        Double_t pull = residual / sqrt( recoError[iParam] );
        hStsFitHisto[0][iParam+5]->Fill(pull);
        hStsFitHisto[iDir][iParam+5]->Fill(pull);
      } 
    }
  }
  
  //Check quality of global tracks
  
  vector<int> trackMuchMatch;
  if(fMuchTrackMatchArray != NULL)
  {
    Int_t nMuchTrackMatches=fMuchTrackMatchArray->GetEntriesFast();
    trackMuchMatch.resize(nMuchTrackMatches, -1);

    for(int iTr=0; iTr<nMuchTrackMatches; iTr++)
    {
      CbmTrackMatchNew* muchTrackMatch = (CbmTrackMatchNew*) fMuchTrackMatchArray->At(iTr);
      if(muchTrackMatch -> GetNofLinks() == 0) continue;
      Float_t bestWeight = 0.f;
      Float_t totalWeight = 0.f;
      Int_t mcTrackId = -1;
      for(int iLink=0; iLink<muchTrackMatch -> GetNofLinks(); iLink++)
      {
        totalWeight += muchTrackMatch->GetLink(iLink).GetWeight();
        if( muchTrackMatch->GetLink(iLink).GetWeight() > bestWeight)
        {
          bestWeight = muchTrackMatch->GetLink(iLink).GetWeight();
          mcTrackId = muchTrackMatch->GetLink(iLink).GetIndex();
        }
      }
      if(bestWeight/totalWeight < 0.7) continue;
      if(mcTrackId >= nMCTracks || mcTrackId < 0)
      {
        std::cout << "Much Matching is wrong!    MuchTackId = " << mcTrackId << " N mc tracks = " << nMCTracks << std::endl;
        continue;
      }
      
      trackMuchMatch[iTr] = mcTrackId;
    }
  }
  
  if (fGlobalTrackArray == NULL) 
    Warning("KF Track QA", "No GlobalTrack array!"); 
  else
  {
    for (Int_t igt = 0; igt < fGlobalTrackArray->GetEntriesFast(); igt++)
    {
      const CbmGlobalTrack* globalTrack = static_cast<const CbmGlobalTrack*>(fGlobalTrackArray->At(igt));
      
      Int_t stsTrackIndex = globalTrack->GetStsTrackIndex();//for STS histos
      CbmStsTrack* cbmStsTrack = (CbmStsTrack*) fStsTrackArray->At(stsTrackIndex);
      int stsTrackMCIndex = trackMatch[stsTrackIndex];
      Double_t* stsHistoData = new Double_t[NStsHisto];
      stsHistoData[0] = cbmStsTrack->GetNofHits();//NHits
      stsHistoData[1] = cbmStsTrack->GetChiSq()/cbmStsTrack->GetNDF();//Chi2/NDF
      stsHistoData[2] = TMath::Prob(cbmStsTrack->GetChiSq(),cbmStsTrack->GetNDF());//prob
      if(stsTrackMCIndex>-1)
      {
        int iDir = GetHistoIndex(mcTracks[stsTrackMCIndex].PDG());
        for(int iH=0; iH<NStsHisto; iH++)
        {
          hStsHisto[iDir][iH]->Fill(stsHistoData[iH]);
          hStsHisto[0][iH]->Fill(stsHistoData[iH]);
        }
      }
      else
        for(int iH=0; iH<NStsHisto; iH++)//ghost
          hStsHisto[7][iH]->Fill(stsHistoData[iH]);
      delete [] stsHistoData;
      
      
      Int_t muchIndex = globalTrack->GetMuchTrackIndex();//for MuCh histos
      if(muchIndex>-1)
      {
        CbmMuchTrack* muchTrack = (CbmMuchTrack*)fMuchTrackArray->At(muchIndex);
        
        int muchTrackMCIndex = trackMuchMatch[muchIndex];
        
        Double_t* muchHistoData = new Double_t[NMuchHisto];
        muchHistoData[0] = muchTrack->GetNofHits();//NHits
        muchHistoData[1] = GetZtoNStation(muchTrack->GetParamFirst()->GetZ());//FirstStation
        muchHistoData[2] = GetZtoNStation(muchTrack->GetParamLast()->GetZ());//LastStation
        muchHistoData[3] = muchTrack->GetChiSq()/muchTrack->GetNDF();//Chi2/NDF
        muchHistoData[4] = TMath::Prob(muchTrack->GetChiSq(),muchTrack->GetNDF());//prob

        if(stsTrackMCIndex<0 || stsTrackMCIndex!=muchTrackMCIndex) //ghost
          for(int iH=0; iH<NMuchHisto; iH++)
            hMuchHisto[2][iH]->Fill(muchHistoData[iH]);
        else
        {
          if(TMath::Abs(mcTracks[stsTrackMCIndex].PDG()) == 13) //muon 
            for(int iH=0; iH<NMuchHisto; iH++)
              hMuchHisto[0][iH]->Fill(muchHistoData[iH]);
          else //BG
            for(int iH=0; iH<NMuchHisto; iH++)
              hMuchHisto[1][iH]->Fill(muchHistoData[iH]);
        }
        delete [] muchHistoData;
      }

      //Check RICH quality
      const FairTrackParam *stsPar = cbmStsTrack->GetParamFirst();
      TVector3 mom;
      stsPar->Momentum(mom);

      Double_t p = mom.Mag();
    
      if(fRichRingArray && fRichRingMatchArray)
      {
        Int_t richIndex = globalTrack->GetRichRingIndex();
        if (richIndex > -1)
        {
          CbmRichRing* richRing = (CbmRichRing*)fRichRingArray->At(richIndex);
          if (richRing)
          {
            int richTrackMCIndex = -1;
            CbmTrackMatchNew* richRingMatch = (CbmTrackMatchNew*)fRichRingMatchArray->At(richIndex);
            if(richRingMatch)
            {
              if(richRingMatch->GetNofLinks() > 0)
              {
                float bestWeight = 0.f;
                float totalWeight = 0.f;
                int bestMCTrackId = -1;
                for(int iLink=0; iLink<richRingMatch->GetNofLinks(); iLink++)
                {
                  totalWeight += richRingMatch->GetLink(iLink).GetWeight();
                  if( richRingMatch->GetLink(iLink).GetWeight() > bestWeight)
                  {
                    bestWeight = richRingMatch->GetLink(iLink).GetWeight();
                    bestMCTrackId = richRingMatch->GetLink(iLink).GetIndex();
                  }
                }
                if(bestWeight/totalWeight >= 0.7)
                  richTrackMCIndex = bestMCTrackId;
              }
            }
            
            Double_t r = richRing->GetRadius();
            Double_t axisA = richRing->GetAaxis();
            Double_t axisB = richRing->GetBaxis();

            hRichRingHisto2D[0][0]->Fill(p, r);
            hRichRingHisto2D[0][1]->Fill(p, axisA);
            hRichRingHisto2D[0][2]->Fill(p, axisB);
            
            int iTrackCategory = -1;
            if(stsTrackMCIndex<0)
              iTrackCategory = 8; // ghost sts track + any ring
            else if(richTrackMCIndex<0)
              iTrackCategory = 9; // normal sts track + ghost ring
            else if(stsTrackMCIndex != richTrackMCIndex)
              iTrackCategory = 7; // mismatched sts track and ring
            else
              iTrackCategory = GetHistoIndex(pdg[stsTrackIndex]);
            
            hRichRingHisto2D[iTrackCategory][0]->Fill(p, r);
            hRichRingHisto2D[iTrackCategory][1]->Fill(p, axisA);
            hRichRingHisto2D[iTrackCategory][2]->Fill(p, axisB);
          }
        }
      }
    }
  }  
}

void CbmKFTrackQA::Finish()
{
  TDirectory *curr = gDirectory;
  TFile *currentFile = gFile;
  // Open output file and write histograms

  fOutFile->cd();
  WriteHistosCurFile(fHistoDir);
  if(!(fOutFileName == ""))
  {   
    fOutFile->Close();
    fOutFile->Delete();
  }
  gFile = currentFile;
  gDirectory = curr;
}

void CbmKFTrackQA::WriteHistosCurFile( TObject *obj ){
  
  
  if( !obj->IsFolder() ) obj->Write();
  else{
    TDirectory *cur = gDirectory;
    TFile *currentFile = gFile;

    TDirectory *sub = cur->GetDirectory(obj->GetName());
    sub->cd();
    TList *listSub = (static_cast<TDirectory*>(obj))->GetList();
    TIter it(listSub);
    while( TObject *obj1=it() ) WriteHistosCurFile(obj1);
    cur->cd();
    gFile = currentFile;
    gDirectory = cur;
  }
}

int CbmKFTrackQA::GetHistoIndex(int pdg)
{
  map<int, int>::iterator it;
  it=fPDGtoIndexMap.find(TMath::Abs(pdg));
  if(it != fPDGtoIndexMap.end()) return it->second;
  else return 6;
}

Int_t CbmKFTrackQA::GetZtoNStation(Double_t getZ)
{
  if(TMath::Abs(getZ-145)<=2.0) return 1;
  if(TMath::Abs(getZ-155)<=2.0) return 2;
  if(TMath::Abs(getZ-165)<=2.0) return 3;
  if(TMath::Abs(getZ-195)<=2.0) return 4;
  if(TMath::Abs(getZ-205)<=2.0) return 5;
  if(TMath::Abs(getZ-215)<=2.0) return 6;
  if(TMath::Abs(getZ-245)<=2.0) return 7;
  if(TMath::Abs(getZ-255)<=2.0) return 8;
  if(TMath::Abs(getZ-265)<=2.0) return 9;
  if(TMath::Abs(getZ-305)<=2.0) return 10;
  if(TMath::Abs(getZ-315)<=2.0) return 11;
  if(TMath::Abs(getZ-325)<=2.0) return 12;
  if(TMath::Abs(getZ-370)<=2.0) return 13;
  if(TMath::Abs(getZ-380)<=2.0) return 14;
  if(TMath::Abs(getZ-390)<=2.0) return 15;
  if(TMath::Abs(getZ-500)<=2.0) return 16;
  if(TMath::Abs(getZ-510)<=2.0) return 17;
  if(TMath::Abs(getZ-520)<=2.0) return 18;

  return -1;
}

ClassImp(CbmKFTrackQA);
