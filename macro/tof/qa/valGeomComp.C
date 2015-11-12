
const UInt_t   kuNbFiles = 4;
const UInt_t   kuNbDim   = 5;
const TString  ksDimName[kuNbDim] = { "X", "Y", "Z", "R", "T" }; 
const Color_t	kcFileColor[kuNbFiles] = { kBlue, kRed, kGreen, kOrange };
const Int_t    kiLineWidth    = 2;
const Double_t kdLegTextSize  = 0.04;
const Double_t kdPadTitleSize = 0.1;

//___________________________________________________________________
// Constants definitions: Particles list
const Int_t   kiNbPart =  9;
const TString ksPartTag[kiNbPart] = 
   { "others",
     "ep", "em",   "pip", "pim", "kp", "km", 
     "p",  "pbar" };
const TString ksPartName[kiNbPart] = 
   { "any other part.", 
     "e-", "e+",   "#pi+", "#pi-", "k+", "k-", 
     "p",  "anti-p"};
const Int_t   kiMinNbStsPntAcc = 3; // Number of STS Pnt for Trk to be reconstructable
//___________________________________________________________________

Bool_t valGeomComp( TString sFilenameA = "data/tofqa.sis300_electron_auau.25gev.centr._qa.hst.all.root",
                    TString sFilenameB = "data/tofqa.cbm100_pbar_auau.25gev.centr._qa.hst.all.root",
                    TString sFilenameC = "data/tofqa.sis300_electron_auau.25gev.centr.noTRD_qa.hst.all.root",
                    TString sFilenameD = "data/tofqa.cbm100_pbar_auau.25gev.centr.noTRD_qa.hst.all.root",
                    TString sTagFilesA = "v13-5d",       TString sTagFilesB = "v15b",
                    TString sTagFilesC = "v13-5d_noTrd", TString sTagFilesD = "v15b_noTrd",
                    TString sOutFileTag = "_25gev" 
                    )
{
   
   // Open the input files
   TFile * filePnt[kuNbFiles];
   filePnt[0] = new TFile( sFilenameA, "READ" );
   filePnt[1] = new TFile( sFilenameB, "READ" );
   filePnt[2] = new TFile( sFilenameC, "READ" );
   filePnt[3] = new TFile( sFilenameD, "READ" );
   
   TString sTagFiles[kuNbFiles];
   sTagFiles[0] = sTagFilesA;
   sTagFiles[1] = sTagFilesB;
   sTagFiles[2] = sTagFilesC;
   sTagFiles[3] = sTagFilesD;
   
   if( NULL == filePnt[0] || NULL == filePnt[1] || NULL == filePnt[2] || NULL == filePnt[3] )
   {
      cout<<"One of the input files could not be opened: "
          <<sFilenameA<<" -> "<<filePnt[0]<<" "
          <<sFilenameB<<" -> "<<filePnt[1]<<" "
          <<sFilenameC<<" -> "<<filePnt[2]<<" "
          <<sFilenameD<<" -> "<<filePnt[3]<<" "
          <<endl;
      return kFALSE;    
   } // if( NULL == filePnt[0] || NULL == filePnt[1] || NULL == filePnt[2] || NULL == filePnt[3] )
   
   // First recover all input hitograms from files
   gROOT->cd();
      
   TH1 * hSinglePointHitDelta[kuNbFiles][kuNbDim];
   TH2 * hMultiPntHitClosestDelta[kuNbFiles][kuNbDim];
   TH2 * hMultiPntHitFurthestDelta[kuNbFiles][kuNbDim];
   TH2 * hMultiPntHitMeanDelta[kuNbFiles][kuNbDim];
   TH2 * hMultiPntHitBestDelta[kuNbFiles][kuNbDim];
   
   TH1 * hMultiPntHitClosestDelta_proj[kuNbFiles][kuNbDim];
   TH1 * hMultiPntHitFurthestDelta_proj[kuNbFiles][kuNbDim];
   TH1 * hMultiPntHitMeanDelta_proj[kuNbFiles][kuNbDim];
   TH1 * hMultiPntHitBestDelta_proj[kuNbFiles][kuNbDim];
   
   TH1 * hAllPntHitClosestDelta[kuNbFiles][kuNbDim];
   TH1 * hAllPntHitFurthestDelta[kuNbFiles][kuNbDim];
   TH1 * hAllPntHitMeanDelta[kuNbFiles][kuNbDim];
   TH1 * hAllPntHitBestDelta[kuNbFiles][kuNbDim];
   
   TH1 * hIntegratedPntHitEff[kuNbFiles];
   TH1 * hIntegratedPntHitEffPrim[kuNbFiles];
   TH1 * hIntegratedPntHitEffSec[kuNbFiles];
   
   TH1 * hPlabGenTrk[kuNbFiles][kiNbPart];
   TH1 * hPlabStsPnt[kuNbFiles][kiNbPart];
   TH1 * hPlabGenTrkTofPnt[kuNbFiles][kiNbPart];
   TH1 * hPlabGenTrkTofHit[kuNbFiles][kiNbPart];
   TH1 * hPlabTofPntEff[kuNbFiles][kiNbPart];
   TH1 * hPlabTofHitEff[kuNbFiles][kiNbPart];
   TH1 * hPlabTofHitGenEff[kuNbFiles][kiNbPart];
   
   TH1 * hPlabSecGenTrk[kuNbFiles][kiNbPart];
   TH1 * hPlabSecStsPnt[kuNbFiles][kiNbPart];
   TH1 * hPlabSecGenTrkTofPnt[kuNbFiles][kiNbPart];
   TH1 * hPlabSecGenTrkTofHit[kuNbFiles][kiNbPart];
   TH1 * hPlabSecTofPntEff[kuNbFiles][kiNbPart];
   TH1 * hPlabSecTofHitEff[kuNbFiles][kiNbPart];
   TH1 * hPlabSecTofHitGenEff[kuNbFiles][kiNbPart];
      // Physics coord mapping, 1 per particle type
         // PLab
   Int_t    iNbBinsPlab = 100;
   Double_t dMinPlab    =   0.0;
   Double_t dMaxPlab    =  10.0;    
   
   TH1 * tempOneDimHist = NULL;
   TH2 * tempTwoDimHist = NULL;
   for( UInt_t uFile = 0; uFile < kuNbFiles; uFile++)
   {
      tempOneDimHist = (TH1*) (filePnt[uFile]->FindObjectAny( 
            Form("TofTests_IntegratedHitPntEff") ) );
      if( NULL != tempOneDimHist )
         hIntegratedPntHitEff[uFile] = (TH1*)(tempOneDimHist->Clone( 
               Form("%s_IntegratedHitPntEff", sTagFiles[uFile].Data()) ) );
         else return kFALSE;
    
      tempOneDimHist = (TH1*) (filePnt[uFile]->FindObjectAny( 
            Form("TofTests_IntegratedHitPntEffPrim") ) );
      if( NULL != tempOneDimHist )
         hIntegratedPntHitEffPrim[uFile] = (TH1*)(tempOneDimHist->Clone( 
               Form("%s_IntegratedHitPntEffPrim", sTagFiles[uFile].Data()) ) );
         else return kFALSE;
         
      tempOneDimHist = (TH1*) (filePnt[uFile]->FindObjectAny( 
            Form("TofTests_IntegratedHitPntEffSec") ) );
      if( NULL != tempOneDimHist )
         hIntegratedPntHitEffSec[uFile] = (TH1*)(tempOneDimHist->Clone( 
               Form("%s_IntegratedHitPntEffSec", sTagFiles[uFile].Data()) ) );
         else return kFALSE;
              
      for(UInt_t uDim = 0; uDim < kuNbDim; uDim++)
      {
         tempOneDimHist = (TH1*) (filePnt[uFile]->FindObjectAny( 
               Form("TofTests_SinglePointHitDelta%s",  ksDimName[uDim].Data()) ) );
         if( NULL != tempOneDimHist )
            hSinglePointHitDelta[uFile][uDim] = (TH1*)(tempOneDimHist->Clone( 
                  Form("%s_SinglePointHitDelta%s", sTagFiles[uFile].Data(), ksDimName[uDim].Data()) ) );
            else return kFALSE;

         tempTwoDimHist = (TH2*) (filePnt[uFile]->FindObjectAny( 
               Form("TofTests_MultiPntHitClosestDelta%s",  ksDimName[uDim].Data()) ) );
         if( NULL != tempTwoDimHist )
            hMultiPntHitClosestDelta[uFile][uDim] = (TH2*)(tempTwoDimHist->Clone( 
                  Form("%s_MultiPntHitClosestDelta%s", sTagFiles[uFile].Data(), ksDimName[uDim].Data()) ) );
            else return kFALSE;

         tempTwoDimHist = (TH2*) (filePnt[uFile]->FindObjectAny( 
               Form("TofTests_MultiPntHitFurthestDelta%s",  ksDimName[uDim].Data()) ) );
         if( NULL != tempTwoDimHist )
            hMultiPntHitFurthestDelta[uFile][uDim] = (TH2*)(tempTwoDimHist->Clone( 
                  Form("%s_MultiPntHitFurthestDelta%s", sTagFiles[uFile].Data(), ksDimName[uDim].Data()) ) );
            else return kFALSE;

         tempTwoDimHist = (TH2*) (filePnt[uFile]->FindObjectAny( 
               Form("TofTests_MultiPntHitMeanDelta%s",  ksDimName[uDim].Data()) ) );
         if( NULL != tempTwoDimHist )
            hMultiPntHitMeanDelta[uFile][uDim] = (TH2*)(tempTwoDimHist->Clone( 
                  Form("%s_MultiPntHitMeanDelta%s", sTagFiles[uFile].Data(), ksDimName[uDim].Data()) ) );
            else return kFALSE;

         tempTwoDimHist = (TH2*) (filePnt[uFile]->FindObjectAny( 
               Form("TofTests_MultiPntHitBestDelta%s",  ksDimName[uDim].Data()) ) );
         if( NULL != tempTwoDimHist )
            hMultiPntHitBestDelta[uFile][uDim] = (TH2*)(tempTwoDimHist->Clone( 
                  Form("%s_MultiPntHitBestDelta%s", sTagFiles[uFile].Data(), ksDimName[uDim].Data()) ) );
            else return kFALSE;
            
         hMultiPntHitClosestDelta_proj[uFile][uDim]  = hMultiPntHitClosestDelta[uFile][uDim]->ProjectionX(
               Form("%s_MultiPntHitClosestDelta%s_proj", sTagFiles[uFile].Data(), ksDimName[uDim].Data()) );
         hMultiPntHitFurthestDelta_proj[uFile][uDim] = hMultiPntHitFurthestDelta[uFile][uDim]->ProjectionX(
               Form("%s_MultiPntHitFurthestDelta%s_proj", sTagFiles[uFile].Data(), ksDimName[uDim].Data()) );
         hMultiPntHitMeanDelta_proj[uFile][uDim]     = hMultiPntHitMeanDelta[uFile][uDim]->ProjectionX(
               Form("%s_MultiPntHitMeanDelta%s_proj", sTagFiles[uFile].Data(), ksDimName[uDim].Data()) );
         hMultiPntHitBestDelta_proj[uFile][uDim]     = hMultiPntHitBestDelta[uFile][uDim]->ProjectionX(
               Form("%s_MultiPntHitBestDelta%s_proj", sTagFiles[uFile].Data(), ksDimName[uDim].Data()) );
               
         hAllPntHitClosestDelta[uFile][uDim]  = (TH1*)(hMultiPntHitClosestDelta_proj[uFile][uDim]->Clone( 
                  Form("%s_AllPntHitClosestDelta%s", sTagFiles[uFile].Data(), ksDimName[uDim].Data()) ) );
         hAllPntHitFurthestDelta[uFile][uDim] = (TH1*)(hMultiPntHitFurthestDelta_proj[uFile][uDim]->Clone( 
                  Form("%s_AllPntHitFurthestDelta%s", sTagFiles[uFile].Data(), ksDimName[uDim].Data()) ) );
         hAllPntHitMeanDelta[uFile][uDim]     = (TH1*)(hMultiPntHitMeanDelta_proj[uFile][uDim]->Clone( 
                  Form("%s_AllPntHitMeanDelta%s", sTagFiles[uFile].Data(), ksDimName[uDim].Data()) ) );
         hAllPntHitBestDelta[uFile][uDim]     = (TH1*)(hMultiPntHitBestDelta_proj[uFile][uDim]->Clone( 
                  Form("%s_AllPntHitBestDelta%s", sTagFiles[uFile].Data(), ksDimName[uDim].Data()) ) );
               
         hAllPntHitClosestDelta[uFile][uDim] ->Add( hSinglePointHitDelta[uFile][uDim] );
         hAllPntHitFurthestDelta[uFile][uDim]->Add( hSinglePointHitDelta[uFile][uDim] );
         hAllPntHitMeanDelta[uFile][uDim]    ->Add( hSinglePointHitDelta[uFile][uDim] );
         hAllPntHitBestDelta[uFile][uDim]    ->Add( hSinglePointHitDelta[uFile][uDim] );
      } // Loop on dimensions
      
      for(UInt_t iPartIdx = 0; iPartIdx < kiNbPart; iPartIdx++)
      {
         tempOneDimHist = (TH1*) (filePnt[uFile]->FindObjectAny( 
               Form("TofTests_PlabGenTrk_%s", ksPartTag[iPartIdx].Data() ) ) );
         if( NULL != tempOneDimHist )
            hPlabGenTrk[uFile][iPartIdx] = (TH1*)(tempOneDimHist->Clone( 
                  Form("%s_PlabGenTrk_%s", sTagFiles[uFile].Data(), ksPartTag[iPartIdx].Data()) ) );
            else return kFALSE;
   
         tempOneDimHist = (TH1*) (filePnt[uFile]->FindObjectAny( 
               Form("TofTests_PlabStsPnt_%s", ksPartTag[iPartIdx].Data() ) ) );
         if( NULL != tempOneDimHist )
            hPlabStsPnt[uFile][iPartIdx] = (TH1*)(tempOneDimHist->Clone( 
                  Form("%s_PlabStsPnt_%s", sTagFiles[uFile].Data(), ksPartTag[iPartIdx].Data()) ) );
            else return kFALSE;
   
         tempOneDimHist = (TH1*) (filePnt[uFile]->FindObjectAny( 
               Form("TofTests_PlabGenTrkTofPnt_%s", ksPartTag[iPartIdx].Data() ) ) );
         if( NULL != tempOneDimHist )
            hPlabGenTrkTofPnt[uFile][iPartIdx] = (TH1*)(tempOneDimHist->Clone( 
                  Form("%s_PlabGenTrkTofPnt_%s", sTagFiles[uFile].Data(), ksPartTag[iPartIdx].Data()) ) );
            else return kFALSE;
   
         tempOneDimHist = (TH1*) (filePnt[uFile]->FindObjectAny( 
               Form("TofTests_PlabGenTrkTofHit_%s", ksPartTag[iPartIdx].Data() ) ) );
         if( NULL != tempOneDimHist )
            hPlabGenTrkTofHit[uFile][iPartIdx] = (TH1*)(tempOneDimHist->Clone( 
                  Form("%s_PlabGenTrkTofHit_%s", sTagFiles[uFile].Data(), ksPartTag[iPartIdx].Data()) ) );
            else return kFALSE;
   
         tempOneDimHist = (TH1*) (filePnt[uFile]->FindObjectAny( 
               Form("TofTests_PlabSecGenTrk_%s", ksPartTag[iPartIdx].Data() ) ) );
         if( NULL != tempOneDimHist )
            hPlabSecGenTrk[uFile][iPartIdx] = (TH1*)(tempOneDimHist->Clone( 
                  Form("%s_PlabSecGenTrk_%s", sTagFiles[uFile].Data(), ksPartTag[iPartIdx].Data()) ) );
            else return kFALSE;
   
         tempOneDimHist = (TH1*) (filePnt[uFile]->FindObjectAny( 
               Form("TofTests_PlabSecStsPnt_%s", ksPartTag[iPartIdx].Data() ) ) );
         if( NULL != tempOneDimHist )
            hPlabSecStsPnt[uFile][iPartIdx] = (TH1*)(tempOneDimHist->Clone( 
                  Form("%s_PlabSecStsPnt_%s", sTagFiles[uFile].Data(), ksPartTag[iPartIdx].Data()) ) );
            else return kFALSE;
   
         tempOneDimHist = (TH1*) (filePnt[uFile]->FindObjectAny( 
               Form("TofTests_PlabSecGenTrkTofPnt_%s", ksPartTag[iPartIdx].Data() ) ) );
         if( NULL != tempOneDimHist )
            hPlabSecGenTrkTofPnt[uFile][iPartIdx] = (TH1*)(tempOneDimHist->Clone( 
                  Form("%s_PlabSecGenTrkTofPnt_%s", sTagFiles[uFile].Data(), ksPartTag[iPartIdx].Data()) ) );
            else return kFALSE;
   
         tempOneDimHist = (TH1*) (filePnt[uFile]->FindObjectAny( 
               Form("TofTests_PlabSecGenTrkTofHit_%s", ksPartTag[iPartIdx].Data() ) ) );
         if( NULL != tempOneDimHist )
            hPlabSecGenTrkTofHit[uFile][iPartIdx] = (TH1*)(tempOneDimHist->Clone( 
                  Form("%s_PlabSecGenTrkTofHit_%s", sTagFiles[uFile].Data(), ksPartTag[iPartIdx].Data()) ) );
            else return kFALSE;
            
         hPlabTofPntEff[uFile][iPartIdx] = new TH1D( Form("%s_PlabTofPntEff_%s", sTagFiles[uFile].Data(), ksPartTag[iPartIdx].Data() ),
                              Form("P_{lab} efficiency VS Generator for MC Primary Track with TOF points, %s; P_{lab} [GeV/c]; Eff. (TOF/GEN) [\%]", 
                                   ksPartName[iPartIdx].Data()  ),
                              iNbBinsPlab, dMinPlab, dMaxPlab);
         hPlabTofHitEff[uFile][iPartIdx] = new TH1D( Form("%s_PlabTofHitEff_%s", sTagFiles[uFile].Data(), ksPartTag[iPartIdx].Data() ),
                              Form("P_{lab} efficiency VS Generator for MC Primary Track with TOF hits, %s; P_{lab} [GeV/c]; Eff. (TOF Hit/GEN) [\%]", 
                                   ksPartName[iPartIdx].Data()  ),
                              iNbBinsPlab, dMinPlab, dMaxPlab);
         hPlabTofHitGenEff[uFile][iPartIdx] = new TH1D( Form("%s_PlabTofHitGenEff_%s", sTagFiles[uFile].Data(), ksPartTag[iPartIdx].Data() ),
                              Form("P_{lab} efficiency for MC Primary Track with TOF hits VS same with TOF points, %s; P_{lab} [GeV/c]; Eff. (TOF Hit/TOF Pnt) [\%]", 
                                   ksPartName[iPartIdx].Data()  ),
                              iNbBinsPlab, dMinPlab, dMaxPlab);
         hPlabSecTofPntEff[uFile][iPartIdx] = new TH1D( Form("%s_PlabSecTofPntEff_%s", sTagFiles[uFile].Data(), ksPartTag[iPartIdx].Data() ),
                              Form("P_{lab} efficiency VS Generator for MC Secondary Track with TOF points, %s; P_{lab} [GeV/c]; Eff. (TOF/GEN) [\%]", 
                                   ksPartName[iPartIdx].Data()  ),
                              iNbBinsPlab, dMinPlab, dMaxPlab);
         hPlabSecTofHitEff[uFile][iPartIdx] = new TH1D( Form("%s_PlabSecTofHitEff_%s", sTagFiles[uFile].Data(), ksPartTag[iPartIdx].Data() ),
                              Form("P_{lab} efficiency VS Generator for MC Secondary Track with TOF hits, %s; P_{lab} [GeV/c]; Eff. (TOF Hit/GEN) [\%]", 
                                   ksPartName[iPartIdx].Data()  ),
                              iNbBinsPlab, dMinPlab, dMaxPlab);
         hPlabSecTofHitGenEff[uFile][iPartIdx] = new TH1D( Form("%s_PlabSecTofHitGenEff_%s", sTagFiles[uFile].Data(), ksPartTag[iPartIdx].Data() ),
                              Form("P_{lab} efficiency for MC Secondary Track with TOF hits VS same with TOF points, %s; P_{lab} [GeV/c]; Eff. (TOF Hit/TOF Pnt) [\%]", 
                                   ksPartName[iPartIdx].Data()  ),
                              iNbBinsPlab, dMinPlab, dMaxPlab);
                              
         hPlabTofPntEff[uFile][iPartIdx]      ->Divide( hPlabGenTrkTofPnt[uFile][iPartIdx],
                                                        hPlabGenTrk[uFile][iPartIdx],
                                                        100.0 );
         hPlabTofHitEff[uFile][iPartIdx]      ->Divide( hPlabGenTrkTofHit[uFile][iPartIdx],
                                                        hPlabGenTrk[uFile][iPartIdx],
                                                        100.0 );
         hPlabTofHitGenEff[uFile][iPartIdx]   ->Divide( hPlabGenTrkTofHit[uFile][iPartIdx],
                                                        hPlabGenTrkTofPnt[uFile][iPartIdx],
                                                        100.0 );
         hPlabSecTofPntEff[uFile][iPartIdx]   ->Divide( hPlabSecGenTrkTofPnt[uFile][iPartIdx],
                                                        hPlabSecGenTrk[uFile][iPartIdx],
                                                        100.0 );
         hPlabSecTofHitEff[uFile][iPartIdx]   ->Divide( hPlabSecGenTrkTofHit[uFile][iPartIdx],
                                                        hPlabSecGenTrk[uFile][iPartIdx],
                                                        100.0 );
         hPlabSecTofHitGenEff[uFile][iPartIdx]->Divide( hPlabSecGenTrkTofHit[uFile][iPartIdx],
                                                        hPlabSecGenTrkTofPnt[uFile][iPartIdx],
                                                        100.0 );
      } // Loop on particle species
       
   } // Loop on files
   
   // Display SinglePnt, Multi Pnts, Single Trk and Multi Trk histos
   THStack * histosStackAll[kuNbDim];
   THStack * histosStackAllEff;
   TLegend * legAll[kuNbDim];
   TLegend * legAllEff;
   
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

      for( UInt_t uFile = 0; uFile < kuNbFiles; uFile++)
      {          
 //        if( kTRUE == sTagFiles[uFile].Contains( "HitProd" ) )
         {
            hAllPntHitMeanDelta[uFile][uDim]->SetLineColor( kcFileColor[uFile] );
            hAllPntHitMeanDelta[uFile][uDim]->SetLineWidth( kiLineWidth );
            histosStackAll[uDim]->Add( hAllPntHitMeanDelta[uFile][uDim] );
            legAll[uDim]->AddEntry( hAllPntHitMeanDelta[uFile][uDim] , 
                                    Form("%s, Mean", sTagFiles[uFile].Data()), "l");
         } // if( kTRUE == sTagFiles[uFile].Contains( "HitProd" ) )
         /*
            else
            {
               hAllPntHitBestDelta[uFile][uDim]->SetLineColor( kcFileColor[uFile] );
               hAllPntHitBestDelta[uFile][uDim]->SetLineWidth( kiLineWidth );
               histosStackAll[uDim]->Add( hAllPntHitBestDelta[uFile][uDim] );
               legAll[uDim]->AddEntry( hAllPntHitBestDelta[uFile][uDim] , 
                                       Form("%s, Best", sTagFiles[uFile].Data()), "l");
            } // else of if( kTRUE == sTagFiles[uFile].Contains( "HitProd" ) )
            */
      } // for( UInt_t uFile = 0; uFile < kuNbFiles; uFile++)
      
      canvAll->cd(1 + uDim);
      gPad->SetGridx();
      gPad->SetGridy();
      gPad->SetLogy();
      histosStackAll[uDim]->Draw("nostack");
      if( 2 == uDim )
         legAll[uDim]->Draw();
   } // for(UInt_t uDim = 0; uDim < kuNbDim; uDim++)
   
   // Efficiency
   
   histosStackAllEff = new THStack( "histosStackAllEff",
         "Efficiency of TOF hit generation, all tracks; Eff = Nb_{Trk w/ Hit}/Nb_{Trk w/ Pnt} [\%]; # [Events]");
   legAllEff = new TLegend(0.55,0.55,0.9,0.9);  
   legAllEff->SetHeader( Form("Pnt->Hit eff., all tracks" ) );
   legAllEff->SetTextSize( kdLegTextSize );

   for( UInt_t uFile = 0; uFile < kuNbFiles; uFile++)
   {          
      hIntegratedPntHitEff[uFile]->SetLineColor( kcFileColor[uFile] );
      hIntegratedPntHitEff[uFile]->SetLineWidth( kiLineWidth );
      histosStackAllEff->Add( hIntegratedPntHitEff[uFile] );
      legAllEff->AddEntry( hIntegratedPntHitEff[uFile] , 
                              Form("%s", sTagFiles[uFile].Data()), "l");
   } // for( UInt_t uFile = 0; uFile < kuNbFiles; uFile++)
   
   canvAll->cd(1 + kuNbDim);
   gPad->SetGridx();
   gPad->SetGridy();
   gPad->SetLogy();
   histosStackAllEff->Draw("nostack");
   
//   canvAll->SaveAs( Form( "HProdDigi_All_%s.png", sOutFileTag.Data() ) );
//   canvAll->SaveAs( Form( "HProdDigi_All_%s.pdf", sOutFileTag.Data() ) );

   TCanvas *canvEffPntPrim = new TCanvas( "canvEffPntPrim", 
         "Efficiency up to TOF Points, for different species, geometries comparison",
         1920, 986 );
   canvEffPntPrim->cd();
   canvEffPntPrim->Divide(3,3);
   
   TCanvas *canvEffHitPrim = new TCanvas( "canvEffHitPrim", 
         "Efficiency up to TOF Hits, for different species, geometries comparison",
         1920, 986 );
   canvEffHitPrim->cd();
   canvEffHitPrim->Divide(3,3);
   
   TCanvas *canvEffHitGenPrim = new TCanvas( "canvEffHitGenPrim", 
         "Efficiency up to TOF Hits generation, for different species, geometries comparison",
         1920, 986 );
   canvEffHitGenPrim->cd();
   canvEffHitGenPrim->Divide(3,3);         

   TCanvas *canvEffPntSec = new TCanvas( "canvEffPntSec", 
         "Efficiency up to TOF Points, for different species, geometries comparison",
         1920, 986 );
   canvEffPntSec->cd();
   canvEffPntSec->Divide(3,3);
   
   TCanvas *canvEffHitSec = new TCanvas( "canvEffHitSec", 
         "Efficiency up to TOF Hits, for different species, geometries comparison",
         1920, 986 );
   canvEffHitSec->cd();
   canvEffHitSec->Divide(3,3);
   
   TCanvas *canvEffHitGenSec = new TCanvas( "canvEffHitGenSec", 
         "Efficiency up to TOF Hits generation, for different species, geometries comparison",
         1920, 986 );
   canvEffHitGenSec->cd();
   canvEffHitGenSec->Divide(3,3);
   
   THStack * histosStackEffPntPrim[kiNbPart];
   THStack * histosStackEffHitPrim[kiNbPart];
   THStack * histosStackEffHitGenPrim[kiNbPart];
   THStack * histosStackEffPntSec[kiNbPart];
   THStack * histosStackEffHitSec[kiNbPart];
   THStack * histosStackEffHitGenSec[kiNbPart];
   
   for(UInt_t iPartIdx = 0; iPartIdx < kiNbPart; iPartIdx++)
   {
      histosStackEffPntPrim[iPartIdx] = new THStack( Form("histosStackEffPntPrim_%s", ksPartTag[iPartIdx].Data() ),
          Form("P_{lab} efficiency VS Generator for MC Primary Track with TOF points, %s; P_{lab} [GeV/c]; Eff. (TOF/GEN) [\%]", 
                                   ksPartName[iPartIdx].Data()  ) );
      histosStackEffHitPrim[iPartIdx] = new THStack( Form("histosStackEffHitPrim_%s", ksPartTag[iPartIdx].Data() ),
          Form("P_{lab} efficiency VS Generator for MC Primary Track with TOF hits, %s; P_{lab} [GeV/c]; Eff. (TOF Hit/GEN) [\%]",
                                   ksPartName[iPartIdx].Data()  ) );
      histosStackEffHitGenPrim[iPartIdx] = new THStack( Form("histosStackEffHitGenPrim_%s", ksPartTag[iPartIdx].Data() ),
          Form("P_{lab} efficiency for MC Primary Track with TOF hits VS same with TOF points, %s; P_{lab} [GeV/c]; Eff. (TOF Hit/TOF Pnt) [\%]", 
                                   ksPartName[iPartIdx].Data()  ) );
      histosStackEffPntSec[iPartIdx] = new THStack( Form("histosStackEffPntSec_%s", ksPartTag[iPartIdx].Data() ),
          Form("P_{lab} efficiency VS Generator for MC Secondary Track with TOF points, %s; P_{lab} [GeV/c]; Eff. (TOF/GEN) [\%]", 
                                   ksPartName[iPartIdx].Data()  ) );
      histosStackEffHitSec[iPartIdx] = new THStack( Form("histosStackEffHitSec_%s", ksPartTag[iPartIdx].Data() ),
          Form("P_{lab} efficiency VS Generator for MC Secondary Track with TOF hits, %s; P_{lab} [GeV/c]; Eff. (TOF Hit/GEN) [\%]",
                                   ksPartName[iPartIdx].Data()  ) );
      histosStackEffHitGenSec[iPartIdx] = new THStack( Form("histosStackEffHitGenSec_%s", ksPartTag[iPartIdx].Data() ),
          Form("P_{lab} efficiency for MC Secondary Track with TOF hits VS same with TOF points, %s; P_{lab} [GeV/c]; Eff. (TOF Hit/TOF Pnt) [\%]", 
                                   ksPartName[iPartIdx].Data()  ) );
                                   
      for( UInt_t uFile = 0; uFile < kuNbFiles; uFile++)
      {
         hPlabTofPntEff[uFile][iPartIdx]->SetLineColor( kcFileColor[uFile] );
         hPlabTofPntEff[uFile][iPartIdx]->SetLineWidth( kiLineWidth );
         histosStackEffPntPrim[iPartIdx]->Add( hPlabTofPntEff[uFile][iPartIdx] );
         
         hPlabTofHitEff[uFile][iPartIdx]->SetLineColor( kcFileColor[uFile] );
         hPlabTofHitEff[uFile][iPartIdx]->SetLineWidth( kiLineWidth );
         histosStackEffHitPrim[iPartIdx]->Add( hPlabTofHitEff[uFile][iPartIdx] );
         
         hPlabTofHitGenEff[uFile][iPartIdx]->SetLineColor( kcFileColor[uFile] );
         hPlabTofHitGenEff[uFile][iPartIdx]->SetLineWidth( kiLineWidth );
         histosStackEffHitGenPrim[iPartIdx]->Add( hPlabTofHitGenEff[uFile][iPartIdx] );
         
         hPlabSecTofPntEff[uFile][iPartIdx]->SetLineColor( kcFileColor[uFile] );
         hPlabSecTofPntEff[uFile][iPartIdx]->SetLineWidth( kiLineWidth );
         histosStackEffPntSec[iPartIdx]->Add( hPlabSecTofPntEff[uFile][iPartIdx] );
         
         hPlabSecTofHitEff[uFile][iPartIdx]->SetLineColor( kcFileColor[uFile] );
         hPlabSecTofHitEff[uFile][iPartIdx]->SetLineWidth( kiLineWidth );
         histosStackEffHitSec[iPartIdx]->Add( hPlabSecTofHitEff[uFile][iPartIdx] );
         
         hPlabSecTofHitGenEff[uFile][iPartIdx]->SetLineColor( kcFileColor[uFile] );
         hPlabSecTofHitGenEff[uFile][iPartIdx]->SetLineWidth( kiLineWidth );
         histosStackEffHitGenSec[iPartIdx]->Add( hPlabSecTofHitGenEff[uFile][iPartIdx] );
      } // for( UInt_t uFile = 0; uFile < kuNbFiles; uFile++)
      
      canvEffPntPrim->cd(1 + iPartIdx);
      gPad->SetGridx();
      gPad->SetGridy();
      histosStackEffPntPrim[iPartIdx]->Draw("nostack");
      
      canvEffHitPrim->cd(1 + iPartIdx);
      gPad->SetGridx();
      gPad->SetGridy();
      histosStackEffHitPrim[iPartIdx]->Draw("nostack");
      
      canvEffHitGenPrim->cd(1 + iPartIdx);
      gPad->SetGridx();
      gPad->SetGridy();
      histosStackEffHitGenPrim[iPartIdx]->Draw("nostack");
      
      canvEffPntSec->cd(1 + iPartIdx);
      gPad->SetGridx();
      gPad->SetGridy();
      histosStackEffPntSec[iPartIdx]->Draw("nostack");
      
      canvEffHitSec->cd(1 + iPartIdx);
      gPad->SetGridx();
      gPad->SetGridy();
      histosStackEffHitSec[iPartIdx]->Draw("nostack");
      
      canvEffHitGenSec->cd(1 + iPartIdx);
      gPad->SetGridx();
      gPad->SetGridy();
      histosStackEffHitGenSec[iPartIdx]->Draw("nostack");
   } // for(UInt_t iPartIdx = 0; iPartIdx < kiNbPart; iPartIdx++)
   
   return kTRUE;
}
                       
