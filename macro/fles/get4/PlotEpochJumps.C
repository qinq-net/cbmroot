{
   TCanvas *cEpJumpChips = new TCanvas("cEpJumpChips", "Epoch Jumps CT chips");
   cEpJumpChips->Divide(2, 4);

   cEpJumpChips->cd( 1);
   hFullCtEpJumpFeeA_000->Draw("colz");

   cEpJumpChips->cd( 2);
   hFullCtEpJumpFeeA_001->Draw("colz");

   cEpJumpChips->cd( 3);
   hFullCtEpJumpFeeA_002->Draw("colz");

   cEpJumpChips->cd( 4);
   hFullCtEpJumpFeeA_003->Draw("colz");

   cEpJumpChips->cd( 5);
   hFullCtEpJumpFeeA_004->Draw("colz");

   cEpJumpChips->cd( 6);
   hFullCtEpJumpFeeA_005->Draw("colz");

   cEpJumpChips->cd( 7);
   hFullCtEpJumpFeeA_006->Draw("colz");

   cEpJumpChips->cd( 8);
   hFullCtEpJumpFeeA_007->Draw("colz");

   TCanvas *cEpJumpChans = new TCanvas("cEpJumpChans", "Epoch Jumps CT chans");
   cEpJumpChans->Divide(2, 2);

   cEpJumpChans->cd( 1);
   hFullCtEpJumpFeeACh_000->Draw("colz");

   cEpJumpChans->cd( 2);
   hFullCtEpJumpFeeACh_001->Draw("colz");

   cEpJumpChans->cd( 3);
   hFullCtEpJumpFeeACh_002->Draw("colz");

   cEpJumpChans->cd( 4);
   hFullCtEpJumpFeeACh_003->Draw("colz");

   TCanvas *cEpJumpChansSortCt = new TCanvas("cEpJumpChansSortCt", "Epoch Jumps CT chans sorted");
   cEpJumpChansSortCt->Divide(2, 2);

   cEpJumpChansSortCt->cd( 1);
   hFullCtEpJumpFeeAChSort_000->Draw("colz");

   cEpJumpChansSortCt->cd( 2);
   hFullCtEpJumpFeeAChSort_001->Draw("colz");

   cEpJumpChansSortCt->cd( 3);
   hFullCtEpJumpFeeAChSort_002->Draw("colz");

   cEpJumpChansSortCt->cd( 4);
   hFullCtEpJumpFeeAChSort_003->Draw("colz");

   TCanvas *cEpJumpChansSort = new TCanvas("cEpJumpChansSort", "Epoch Jumps chan sort");
//   cEpJumpChansSort->Divide(2, 2);

   cEpJumpChansSort->cd();
   hFullCtEpJumpFeeAChOrder->Draw("colz");
}
