const UInt_t   kuNbHistos= 5;
const UInt_t   kuNbDim   = 5;
const TString  ksDimName[kuNbDim] = { "X", "Y", "Z", "R", "T" }; 
const Color_t	kcFileColor[kuNbHistos] = { kBlue, kRed, kGreen, kOrange, kMagenta };
const Int_t    kiLineWidth    = 2;
const Double_t kdLegTextSize  = 0.04;
const Double_t kdPadTitleSize = 0.1;

Bool_t hitProdDigiComp( TString sFilenameA = "data/tofqa.sis300_electron_auau.25gev.centr._HitProd.hst.all.root",
                    TString sFilenameB = "data/tofqa.sis300_electron_auau.25gev.centr._qa.hst.all.root",
                    TString sTagFilesA = "HitProd", 
                    TString sTagFilesB = "DigiClust",
                    TString sOutFileTag = "_25gev" 
                    )
{
   
   // Open the input files
   TFile * filePntA = new TFile( sFilenameA, "READ" );
   TFile * filePntB = new TFile( sFilenameB, "READ" );
   
   if( NULL == filePntA || NULL == filePntB )
   {
      cout<<"One of the input files could not be opened: "
          <<sFilenameA<<" -> "<<filePntA<<" "
          <<sFilenameB<<" -> "<<filePntB<<" "
          <<endl;
      return kFALSE;    
   } // if( NULL == filePntA || NULL == filePntB )
   
   // First recover all input hitograms from files
   gROOT->cd();
   TH1 * hSinglePointHitHpDelta[kuNbDim];
   TH1 * hDiffPointHitLeftDelta[kuNbDim];
   TH1 * hDiffPointHitRightDelta[kuNbDim];
   
   TH1 * hSinglePointHitDelta[kuNbDim];
   TH2 * hMultiPntHitClosestDelta[kuNbDim];
   TH2 * hMultiPntHitFurthestDelta[kuNbDim];
   TH2 * hMultiPntHitMeanDelta[kuNbDim];
   
   TH1 * hMultiPntHitClosestDelta_proj[kuNbDim];
   TH1 * hMultiPntHitFurthestDelta_proj[kuNbDim];
   TH1 * hMultiPntHitMeanDelta_proj[kuNbDim];
   
   TH1 * hAllPntHitHpLeftDelta[kuNbDim];
   TH1 * hAllPntHitHpRightDelta[kuNbDim];
   TH1 * hAllPntHitClosestDelta[kuNbDim];
   TH1 * hAllPntHitFurthestDelta[kuNbDim];
   TH1 * hAllPntHitMeanDelta[kuNbDim];
   
   TH1 * tempOneDimHist = NULL;
   TH2 * tempTwoDimHist = NULL;
   for(UInt_t uDim = 0; uDim < kuNbDim; uDim++)
   {
      tempOneDimHist = (TH1*) (filePntA->FindObjectAny( 
            Form("HitProd_SinglePointHitDelta%s",  ksDimName[uDim].Data()) ) );
      if( NULL != tempOneDimHist )
         hSinglePointHitHpDelta[uDim] = (TH1*)(tempOneDimHist->Clone( 
               Form("%s_SinglePointHitHpDelta%s", sTagFilesA.Data(), ksDimName[uDim].Data()) ) );
         else return kFALSE;
         
      tempOneDimHist = (TH1*) (filePntA->FindObjectAny( 
            Form("HitProd_DiffPointHitLeftDelta%s",  ksDimName[uDim].Data()) ) );
      if( NULL != tempOneDimHist )
         hDiffPointHitLeftDelta[uDim] = (TH1*)(tempOneDimHist->Clone( 
               Form("%s_DiffPointHitLeftDelta%s", sTagFilesA.Data(), ksDimName[uDim].Data()) ) );
         else return kFALSE;
         
      tempOneDimHist = (TH1*) (filePntA->FindObjectAny( 
            Form("HitProd_DiffPointHitRightDelta%s",  ksDimName[uDim].Data()) ) );
      if( NULL != tempOneDimHist )
         hDiffPointHitRightDelta[uDim] = (TH1*)(tempOneDimHist->Clone( 
               Form("%s_DiffPointHitRightDelta%s", sTagFilesA.Data(), ksDimName[uDim].Data()) ) );
         else return kFALSE;
         
      hAllPntHitHpLeftDelta[uDim]  = (TH1*)(hDiffPointHitLeftDelta[uDim]->Clone( 
               Form("%s_AllPntHitHpLeftDelta%s", sTagFilesA.Data(), ksDimName[uDim].Data()) ) );
      hAllPntHitHpRightDelta[uDim]  = (TH1*)(hDiffPointHitRightDelta[uDim]->Clone( 
               Form("%s_AllPntHitHpRightDelta%s", sTagFilesA.Data(), ksDimName[uDim].Data()) ) );
               
      hAllPntHitHpLeftDelta[uDim] ->Add( hSinglePointHitHpDelta[uDim] );
      hAllPntHitHpRightDelta[uDim]->Add( hSinglePointHitHpDelta[uDim] );
                  
      tempOneDimHist = (TH1*) (filePntB->FindObjectAny( 
            Form("TofTests_SinglePointHitDelta%s",  ksDimName[uDim].Data()) ) );
      if( NULL != tempOneDimHist )
         hSinglePointHitDelta[uDim] = (TH1*)(tempOneDimHist->Clone( 
               Form("%s_SinglePointHitDelta%s", sTagFilesB.Data(), ksDimName[uDim].Data()) ) );
         else return kFALSE;

      tempTwoDimHist = (TH2*) (filePntB->FindObjectAny( 
            Form("TofTests_MultiPntHitClosestDelta%s",  ksDimName[uDim].Data()) ) );
      if( NULL != tempTwoDimHist )
         hMultiPntHitClosestDelta[uDim] = (TH2*)(tempTwoDimHist->Clone( 
               Form("%s_MultiPntHitClosestDelta%s", sTagFilesB.Data(), ksDimName[uDim].Data()) ) );
         else return kFALSE;

      tempTwoDimHist = (TH2*) (filePntB->FindObjectAny( 
            Form("TofTests_MultiPntHitFurthestDelta%s",  ksDimName[uDim].Data()) ) );
      if( NULL != tempTwoDimHist )
         hMultiPntHitFurthestDelta[uDim] = (TH2*)(tempTwoDimHist->Clone( 
               Form("%s_MultiPntHitFurthestDelta%s", sTagFilesB.Data(), ksDimName[uDim].Data()) ) );
         else return kFALSE;

      tempTwoDimHist = (TH2*) (filePntB->FindObjectAny( 
            Form("TofTests_MultiPntHitMeanDelta%s",  ksDimName[uDim].Data()) ) );
      if( NULL != tempTwoDimHist )
         hMultiPntHitMeanDelta[uDim] = (TH2*)(tempTwoDimHist->Clone( 
               Form("%s_MultiPntHitMeanDelta%s", sTagFilesB.Data(), ksDimName[uDim].Data()) ) );
         else return kFALSE;
         
      hMultiPntHitClosestDelta_proj[uDim]  = hMultiPntHitClosestDelta[uDim]->ProjectionX(
            Form("%s_MultiPntHitClosestDelta%s_proj", sTagFilesB.Data(), ksDimName[uDim].Data()) );
      hMultiPntHitFurthestDelta_proj[uDim] = hMultiPntHitFurthestDelta[uDim]->ProjectionX(
            Form("%s_MultiPntHitFurthestDelta%s_proj", sTagFilesB.Data(), ksDimName[uDim].Data()) );
      hMultiPntHitMeanDelta_proj[uDim]     = hMultiPntHitMeanDelta[uDim]->ProjectionX(
            Form("%s_MultiPntHitMeanDelta%s_proj", sTagFilesB.Data(), ksDimName[uDim].Data()) );
            
      hAllPntHitClosestDelta[uDim]  = (TH1*)(hMultiPntHitClosestDelta_proj[uDim]->Clone( 
               Form("%s_AllPntHitClosestDelta%s", sTagFilesB.Data(), ksDimName[uDim].Data()) ) );
      hAllPntHitFurthestDelta[uDim] = (TH1*)(hMultiPntHitFurthestDelta_proj[uDim]->Clone( 
               Form("%s_AllPntHitFurthestDelta%s", sTagFilesB.Data(), ksDimName[uDim].Data()) ) );
      hAllPntHitMeanDelta[uDim]     = (TH1*)(hMultiPntHitMeanDelta_proj[uDim]->Clone( 
               Form("%s_AllPntHitMeanDelta%s", sTagFilesB.Data(), ksDimName[uDim].Data()) ) );
               
      hAllPntHitClosestDelta[uDim] ->Add( hSinglePointHitDelta[uDim] );
      hAllPntHitFurthestDelta[uDim]->Add( hSinglePointHitDelta[uDim] );
      hAllPntHitMeanDelta[uDim]    ->Add( hSinglePointHitDelta[uDim] );
   } // Loop on files and dimensions
   
   // Display SinglePnt, Multi Pnts, Single Trk and Multi Trk histos
   THStack * histosStackAll[kuNbDim];
   TLegend * legAll[kuNbDim];
   
   TCanvas *canvAll = new TCanvas( "canvAll", 
         "Distance between Hit and Point, all hits, comparison of HitProducer and Digitizer+Clusterizer",
         1920, 986 );
   canvAll->cd();
   canvAll->Divide(3,2);
   
   for(UInt_t uDim = 0; uDim < kuNbDim; uDim++)
   {   
      if( "T" == ksDimName[uDim] )
      {
         histosStackAll[uDim] = new THStack(Form("histosStackAll%s",  ksDimName[uDim].Data()),
               Form("Distance in %s from Pnt, all Hits; %s Hit - %s Trk [ps]; Counts [Hits]",
                           ksDimName[uDim].Data(), ksDimName[uDim].Data(), ksDimName[uDim].Data() ) );
      } // if( "T" == ksDimName[uDim] )
      else
      {
         histosStackAll[uDim] = new THStack(Form("histosStackAll%s",  ksDimName[uDim].Data()),
               Form("Distance in %s from Pnt, all Hits; %s Hit - %s Trk [cm]; Counts [Hits]",
                           ksDimName[uDim].Data(), ksDimName[uDim].Data(), ksDimName[uDim].Data() ) );
      } // else of if( "T" == ksDimName[uDim] )

      legAll[uDim] = new TLegend(0.55,0.55,0.9,0.9);  
      legAll[uDim]->SetHeader( Form("Dist. in %s",  ksDimName[uDim].Data() ) );
      legAll[uDim]->SetTextSize( kdLegTextSize );

      // HitProd With Left point if multiple MC      
      hAllPntHitHpLeftDelta[uDim]->SetLineColor( kcFileColor[0] );
      hAllPntHitHpLeftDelta[uDim]->SetLineWidth( kiLineWidth );
      histosStackAll[uDim]->Add( hAllPntHitHpLeftDelta[uDim] );
      legAll[uDim]->AddEntry( hAllPntHitHpLeftDelta[uDim] , 
                              Form("%s, Left", sTagFilesA.Data()), "l");
      
      // HitProd With Right point if multiple MC
      hAllPntHitHpRightDelta[uDim]->SetLineColor( kcFileColor[1] );
      hAllPntHitHpRightDelta[uDim]->SetLineWidth( kiLineWidth );
      histosStackAll[uDim]->Add( hAllPntHitHpRightDelta[uDim] );
      legAll[uDim]->AddEntry( hAllPntHitHpRightDelta[uDim] , 
                              Form("%s, Right", sTagFilesA.Data()), "l");
      
      // DigiClust With Closest point if multiple MC
      hAllPntHitClosestDelta[uDim]->SetLineColor( kcFileColor[2] );
      hAllPntHitClosestDelta[uDim]->SetLineWidth( kiLineWidth );
      histosStackAll[uDim]->Add( hAllPntHitClosestDelta[uDim] );
      legAll[uDim]->AddEntry( hAllPntHitClosestDelta[uDim] , 
                              Form("%s, Closest", sTagFilesB.Data()), "l");

      // DigiClust With Furthest point if multiple MC
      hAllPntHitFurthestDelta[uDim]->SetLineColor( kcFileColor[3] );
      hAllPntHitFurthestDelta[uDim]->SetLineWidth( kiLineWidth );
      histosStackAll[uDim]->Add( hAllPntHitFurthestDelta[uDim] );
      legAll[uDim]->AddEntry( hAllPntHitFurthestDelta[uDim] , 
                              Form("%s, Furthest", sTagFilesB.Data()), "l");

      // DigiClust With Mean point if multiple MC
      hAllPntHitMeanDelta[uDim]->SetLineColor( kcFileColor[4] );
      hAllPntHitMeanDelta[uDim]->SetLineWidth( kiLineWidth );
      histosStackAll[uDim]->Add( hAllPntHitMeanDelta[uDim] );
      legAll[uDim]->AddEntry( hAllPntHitMeanDelta[uDim] , 
                              Form("%s, Mean", sTagFilesB.Data()), "l");
      
      canvAll->cd(1 + uDim);
      gPad->SetGridx();
      gPad->SetGridy();
      gPad->SetLogy();
      histosStackAll[uDim]->Draw("nostack");
      if( 2 == uDim )
         legAll[uDim]->Draw();
   } // for(UInt_t uDim = 0; uDim < kuNbDim; uDim++)
   
   canvAll->SaveAs( Form( "HProdDigi_All_%s.png", sOutFileTag.Data() ) );
   canvAll->SaveAs( Form( "HProdDigi_All_%s.pdf", sOutFileTag.Data() ) );
   
   return kTRUE;
}
                       
