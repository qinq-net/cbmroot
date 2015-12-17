

//   hPulserFeeDnl->Draw("colz");

//   hPulserFeeTotDnl->Draw("colz");

Bool_t PlotFtEdgesRatio( UInt_t uChannel = 0 )
{
   TH1* projRisingEdge  = hPulserFeeDnl->ProjectionY( Form("projRisingEdge_ch%02u", uChannel),  1+uChannel, 1+uChannel);

   TH1* projFallingEdge = hPulserFeeTotDnl->ProjectionY( Form("projFallingEdge_ch%02u", uChannel), 1+uChannel, 1+uChannel);

   TH1* hEdgesRatio = hPulserFeeDnl->Clone( Form("hEdgesRatio_ch%02u", uChannel) );
   hEdgesRatio->SetTitle( Form("Ratio of Falling over Rising edges per FT bin for channel %02u", uChannel) );
   hEdgesRatio->GetYaxis->SetTitle( "Ratio F/R [\%]");

   hEdgesRatio->Reset();
   hEdgesRatio->Divide( projFallingEdge, projRisingEdge, 100);

   TCanvas *cFtEdgesRatio= new TCanvas("cFtEdgesRatio", "Ratio of Falling over Rising edges per FT bin");
   cFtEdgesRatio->cd();
   hEdgesRatio->Draw();

   return kTRUE;
}
