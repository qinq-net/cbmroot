

//   hPulserFeeDnl->Draw("colz");

//   hPulserFeeTotDnl->Draw("colz");

Bool_t PlotFtEdgesRatio( UInt_t uChannel = 0 )
{
   TH1* projRisingEdgeAll  = hPulserFeeDnl->ProjectionX( "projRisingEdgeAll" );

   TH1* projFallingEdgeAll = hPulserFeeTotDnl->ProjectionX( "projFallingEdgeAll" );

   TH1* projRisingEdge  = hPulserFeeDnl->ProjectionY( Form("projRisingEdge_ch%02u", uChannel),  1+uChannel, 1+uChannel);

   TH1* projFallingEdge = hPulserFeeTotDnl->ProjectionY( Form("projFallingEdge_ch%02u", uChannel), 1+uChannel, 1+uChannel);

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

   TCanvas *cFtEdgesRatio= new TCanvas("cFtEdgesRatio", "Ratio of Falling over Rising edges per FT bin");
   cFtEdgesRatio->Divide(2);

   cFtEdgesRatio->cd(1);
   hEdgesRatioAll->Draw("htext");

   cFtEdgesRatio->cd(2);
   hEdgesRatio->Draw();

   return kTRUE;
}
