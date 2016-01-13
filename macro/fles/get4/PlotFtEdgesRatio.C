

//   hPulserFeeDnl->Draw("colz");

//   hPulserFeeTotDnl->Draw("colz");

Bool_t PlotFtEdgesRatio( UInt_t uChannel = 0 )
{
/*
   TH2* hInputRis = hPulserFeeDnl;
   TH2* hInputFal = hPulserFeeTotDnl;
*/
   TH2* hInputRis = hFtDistribFeeRis;
   TH2* hInputFal = hFtDistribFeeFal;

   TH1* projRisingEdgeAll  = hInputRis->ProjectionX( "projRisingEdgeAll" );

   TH1* projFallingEdgeAll = hInputFal->ProjectionX( "projFallingEdgeAll" );

   TH1* projRisingEdge  = hInputRis->ProjectionY( Form("projRisingEdge_ch%02u", uChannel),  1+uChannel, 1+uChannel);

   TH1* projFallingEdge = hInputFal->ProjectionY( Form("projFallingEdge_ch%02u", uChannel), 1+uChannel, 1+uChannel);

   // Ratios of the total counts of rising/falling edges per chanl
   TH1* hEdgesRatioAll = projRisingEdgeAll->Clone( "hEdgesRatio_all" );
   hEdgesRatioAll->SetTitle( "Ratio of Falling over Rising edges totals for each channel" );
   (hEdgesRatioAll->GetYaxis())->SetTitle( "Ratio F/R [\%]");

   hEdgesRatioAll->Reset();
   hEdgesRatioAll->Divide( projFallingEdgeAll, projRisingEdgeAll, 100.0);

   // Ratio of the counts of rising/falling edges for each FT bin of the selected channel
   TH1* hEdgesRatio = projRisingEdge->Clone( Form("hEdgesRatio_ch%02u", uChannel) );
   hEdgesRatio->SetTitle( Form("Ratio of Falling over Rising edges per FT bin for channel %02u", uChannel) );
   (hEdgesRatio->GetYaxis())->SetTitle( "Ratio F/R [\%]");

   hEdgesRatio->Reset();
   hEdgesRatio->Divide( projFallingEdge, projRisingEdge, 100.0);

   // Ratios of the total counts of rising/falling edges per chanl
   TH1* hEdgesDiffAll = projRisingEdgeAll->Clone( "hEdgesRatio_all_diff" );
   hEdgesDiffAll->SetTitle( "Ratio of Falling over Rising edges totals for each channel" );
   (hEdgesDiffAll->GetYaxis())->SetTitle( "Diff F/R []");

   hEdgesDiffAll->Reset();
   hEdgesDiffAll->Add( projFallingEdgeAll, projRisingEdgeAll, -1.0);

   // Ratio of the counts of rising/falling edges for each FT bin of the selected channel
   TH1* hEdgesDiff = projRisingEdge->Clone( Form("hEdgesRatio_ch%02u_diff", uChannel) );
   hEdgesDiff->SetTitle( Form("Ratio of Falling over Rising edges per FT bin for channel %02u", uChannel) );
   (hEdgesDiff->GetYaxis())->SetTitle( "Diff F/R []");

   hEdgesDiff->Reset();
   hEdgesDiff->Add( projFallingEdge, projRisingEdge, -1.0);

   TCanvas *cFtEdgesRatio= new TCanvas("cFtEdgesRatio", "Ratio of Falling over Rising edges per FT bin");
   cFtEdgesRatio->Divide(2, 2);

   cFtEdgesRatio->cd(1);
   hEdgesRatioAll->Draw("htext");

   cFtEdgesRatio->cd(2);
   hEdgesRatio->Draw();

   cFtEdgesRatio->cd(3);
   hEdgesDiffAll->Draw("htext");
//   projFallingEdge->Draw("htext");

   cFtEdgesRatio->cd(4);
   hEdgesDiff->Draw();
//   projRisingEdge->Draw("htext");

/*
   TH2* hInputRisCt = hFtDistribFeeRis;
   TH2* hInputFalCt = hFtDistribFeeFal;

   TH1* projRisingEdgeAllCt  = hInputRisCt->ProjectionX( "projRisingEdgeAllCt" );

   TH1* projFallingEdgeAllCt = hInputFalCt->ProjectionX( "projFallingEdgeAllCt" );

   TH1* projRisingEdgeCt  = hInputRisCt->ProjectionY( Form("projRisingEdgeCt_ch%02u", uChannel),  1+uChannel, 1+uChannel);

   TH1* projFallingEdgeCt = hInputFalCt->ProjectionY( Form("projFallingEdgeCt_ch%02u", uChannel), 1+uChannel, 1+uChannel);

   // Ratios of the total counts of rising/falling edges per chanl
   TH1* hEdgesRatioAllCt = projRisingEdgeAllCt->Clone( "hEdgesRatio_allCt" );
   hEdgesRatioAllCt->SetTitle( "Ratio of Falling over Rising edges totals for each channel" );
   (hEdgesRatioAllCt->GetYaxis())->SetTitle( "Ratio F/R [\%]");

   hEdgesRatioAllCt->Reset();
   hEdgesRatioAllCt->Divide( projFallingEdgeAllCt, projRisingEdgeAllCt, 100.0);

   // Ratio of the counts of rising/falling edges for each FT bin of the selected channel
   TH1* hEdgesRatio = projRisingEdge->Clone( Form("hEdgesRatio_ch%02u", uChannel) );
   hEdgesRatio->SetTitle( Form("Ratio of Falling over Rising edges per FT bin for channel %02u", uChannel) );
   (hEdgesRatio->GetYaxis())->SetTitle( "Ratio F/R [\%]");

   hEdgesRatio->Reset();
   hEdgesRatio->Divide( projFallingEdge, projRisingEdge, 100.0);

   // Ratios of the total counts of rising/falling edges per chanl
   TH1* hEdgesDiffAll = projRisingEdgeAll->Clone( "hEdgesRatio_all_diff" );
   hEdgesDiffAll->SetTitle( "Ratio of Falling over Rising edges totals for each channel" );
   (hEdgesDiffAll->GetYaxis())->SetTitle( "Diff F/R []");

   hEdgesDiffAll->Reset();
   hEdgesDiffAll->Add( projFallingEdgeAll, projRisingEdgeAll, -1.0);

   // Ratio of the counts of rising/falling edges for each FT bin of the selected channel
   TH1* hEdgesDiff = projRisingEdge->Clone( Form("hEdgesRatio_ch%02u_diff", uChannel) );
   hEdgesDiff->SetTitle( Form("Ratio of Falling over Rising edges per FT bin for channel %02u", uChannel) );
   (hEdgesDiff->GetYaxis())->SetTitle( "Diff F/R []");

   hEdgesDiff->Reset();
   hEdgesDiff->Add( projFallingEdge, projRisingEdge, -1.0);

   TCanvas *cFtEdgesRatio= new TCanvas("cFtEdgesRatio", "Ratio of Falling over Rising edges per FT bin");
   cFtEdgesRatio->Divide(2, 2);

   cFtEdgesRatio->cd(1);
   hEdgesRatioAll->Draw("htext");

   cFtEdgesRatio->cd(2);
   hEdgesRatio->Draw();

   cFtEdgesRatio->cd(3);
   hEdgesDiffAll->Draw("htext");
//   projFallingEdge->Draw("htext");

   cFtEdgesRatio->cd(4);
   hEdgesDiff->Draw();
//   projRisingEdge->Draw("htext");
*/

   return kTRUE;
}
