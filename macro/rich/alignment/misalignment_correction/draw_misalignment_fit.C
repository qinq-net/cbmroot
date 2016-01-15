void draw_misalignment_fit()
{

    // --------------------------------------------------------------------------------------------------------------------- //
    // ------------------------- Draw misalignment points in Y axis and fit with a linear function ------------------------- //
    // --------------------------------------------------------------------------------------------------------------------- //

    // ------------------------------ CALCULATION OF THE MISALIGNMENT ON THE MISALIGNED AXIS ------------------------------ //

    TCanvas *c1 = new TCanvas("c1","c1",200,10,600,400);
    c1->Divide(2,1);
    c1->SetFillColor(42);
    c1->SetGrid();

    c1->cd(1);
    const Int_t n = 20;
    Double_t x[n], y[n], z[n];
    x[0]=0.1, x[1]=0.2, x[2]=0.3, x[3]=0.5, x[4]=0.75, x[5]=1, x[6]=1.5, x[7]=2, x[8]=2.5, x[9]=3, x[10]=4, x[11]=5, x[12]=5.236, x[13]=8.727, x[14]=10.472, x[15]=12.217, x[16]=13.963, x[17]=15.708, x[18]=17.453, x[19]=34.907;
    y[0]=0.0577, y[1]=0.1681, y[2]=0.2607, y[3]=0.4647, y[4]=0.7240, y[5]=0.9760, y[6]=1.4923, y[7]=2.0099, y[8]=2.5283, y[9]=3.0472, y[10]=4.0625, y[11]=5.0712, y[12]=5.3034, y[13]=8.6172, y[14]=10.1330, y[15]=11.3784, y[16]=12.3138, y[17]=11.9113, y[18]=12.1449, y[19]=14.1062;
    z[0]=0.0958, z[1]=0.0923, z[2]=0.0952, z[3]=0.0903, z[4]=0.0852, z[5]=0.0960, z[6]=0.0913, z[7]=0.0864, z[8]=0.0841, z[9]=0.0880, z[10]=0.0728, z[11]=0.0674, z[12]=0.0726, z[13]=0.0440, z[14]=0.0172, z[15]=0.0061, z[16]=0.0261, z[17]=0.0369, z[18]=0.0255, z[19]=0.0667;

    gr = new TGraph(n,x,y);
    gr->SetLineColor(2);
    gr->SetLineWidth(4);
    gr->SetMarkerColor(5);
    gr->SetMarkerSize(1.5);
    gr->SetMarkerStyle(21);
    gr->SetTitle("Misalignment in Y axis");
    gr->GetXaxis()->SetTitle("Rot_Y real [mrad]");
    gr->GetXaxis()->SetLabelSize(0.02);
    gr->GetYaxis()->SetTitle("Rot_Y calculated [mrad]");
    gr->GetYaxis()->SetLabelSize(0.02);
    gr->Draw("ACP");

    TF1 *f1 = new TF1("f1", "[0]*x+[1]", 0., 35.);
    f1->SetParameters(0,0);
    f1->SetLineColor(4);
    gr->Fit("f1","","",0,11);
    TF1* fit = gr->GetFunction("f1");

    // TCanvas::Update() draws the frame, after which one can change it
    c1->Update();
    c1->GetFrame()->SetFillColor(21);
    c1->GetFrame()->SetBorderSize(12);
    c1->Modified();

    TLegend* LEG= new TLegend(0.15,0.72,0.45,0.87); // Set legend position
    Char_t leg[128];
    LEG->SetBorderSize(1);
    LEG->SetFillColor(0);
    LEG->SetMargin(0.2);
    LEG->SetTextSize(0.03);
    sprintf(leg, "Fitted linear a*x+b");
    LEG->AddEntry(f1, leg, "l");
    sprintf(leg, "a = %f", fit->GetParameter(0));
    LEG->AddEntry("", leg, "l");
    sprintf(leg, "b = %f", fit->GetParameter(1));
    LEG->AddEntry("", leg, "l");
    LEG->Draw();

    c1->cd(2);
    Double_t diff_x[n];
    for (Int_t i=0; i<n; i++) { diff_x[i] = TMath::Abs(y[i] - x[i])/x[i]; }
    // fit2->GetParameter(0)*i+fit2->GetParameter(1)

    gr_b = new TGraph(n,x,diff_x);
    gr_b->SetLineColor(2);
    gr_b->SetLineWidth(4);
    gr_b->SetMarkerColor(5);
    gr_b->SetMarkerSize(1.5);
    gr_b->SetMarkerStyle(21);
    gr_b->SetTitle("Misalignment in Y axis - Difference");
    gr_b->GetXaxis()->SetTitle("Rot_Y real [mrad]");
    gr_b->GetXaxis()->SetLabelSize(0.02);
    gr_b->GetYaxis()->SetTitle("Difference Calculated-Real [mrad]");
    gr_b->GetYaxis()->SetLabelSize(0.02);
    gr_b->Draw("ACP");

    // ------------------------------ CALCULATION OF MISALIGNMENT ON THE OTHER AXIS ------------------------------ //

    TCanvas* c1_b = new TCanvas("c1_b","c1_b",200,10,600,400);
    c1_b->Divide(2,1);
    c1_b->SetFillColor(42);
    c1_b->SetGrid();
    c1_b->cd(1);

    gr1 = new TGraph(n,x,z);
    gr1->SetLineColor(2);
    gr1->SetLineWidth(4);
    gr1->SetMarkerColor(5);
    gr1->SetMarkerSize(1.5);
    gr1->SetMarkerStyle(21);
    gr1->SetTitle("Calculated misalignment on X axis VS induced displacement on Y axis");
    gr1->GetXaxis()->SetTitle("Rot_Y real [mrad]");
    gr1->GetXaxis()->SetLabelSize(0.02);
    gr1->GetYaxis()->SetTitle("Rot_X calculated [mrad]");
    gr1->GetYaxis()->SetLabelSize(0.02);
    gr1->Draw("ACP");

    c1_b->cd(2);
    Double_t diff_y[n];
    for(i=0; i<n; i++) { diff_y[i] = TMath::Abs(z[i]-x[i])/x[i]; }

    gr1_b = new TGraph(n,x,diff_y);
    gr1_b->SetLineColor(2);
    gr1_b->SetLineWidth(4);
    gr1_b->SetMarkerColor(5);
    gr1_b->SetMarkerSize(1.5);
    gr1_b->SetMarkerStyle(21);
    gr1_b->SetTitle("Difference on X axis VS induced displacement on Y axis");
    gr1_b->GetXaxis()->SetTitle("Rot_Y real [mrad]");
    gr1_b->GetXaxis()->SetLabelSize(0.02);
    gr1_b->GetYaxis()->SetTitle("Difference Calculated-Real [mrad]");
    gr1_b->GetYaxis()->SetLabelSize(0.02);
    gr1_b->Draw("ACP");

    // --------------------------------------------------------------------------------------------------------------------- //
    // ------------------------- Draw misalignment points in X axis and fit with a linear function ------------------------- //
    // --------------------------------------------------------------------------------------------------------------------- //

    // ------------------------------ CALCULATION OF THE MISALIGNMENT ON THE MISALIGNED AXIS ------------------------------ //

    TCanvas *c2 = new TCanvas("c2","c2",200,10,600,400);
    c2->Divide(2,1);
    c2->SetFillColor(42);
    c2->SetGrid();

    c2->cd(1);
    const Int_t n2 = 22;
    Double_t x_2[n2], y_2[n2], z_2[n2];
    x_2[0]=0.1, x_2[1]=0.2, x_2[2]=0.3, x_2[3]=0.5, x_2[4]=0.75, x_2[5]=1, x_2[6]=1.5, x_2[7]=2, x_2[8]=2.5, x_2[9]=3, x_2[10]=4, x_2[11]=5, x_2[12]=5.236, x_2[13]=8.727, x_2[14]=9.599, x_2[15]=10.472, x_2[16]=11.345, x_2[17]=12.217, x_2[18]=13.090, x_2[19]=15.708, x_2[20]=17.453, x_2[21]=34.907;
    y_2[0]=0.0015, y_2[1]=0.0942, y_2[2]=0.1900, y_2[3]=0.3775, y_2[4]=0.6178, y_2[5]=0.8556, y_2[6]=1.3286, y_2[7]=1.8270, y_2[8]=2.2984, y_2[9]=2.7793, y_2[10]=3.7374, y_2[11]=4.6731, y_2[12]=4.9028, y_2[13]=8.0473, y_2[14]=8.7741, y_2[15]=9.4815, y_2[16]=10.1713, y_2[17]=10.8464, y_2[18]=11.4854, y_2[19]=12.3687, y_2[20]=12.0596, y_2[21]=13.8091;
    z_2[0]=0.0312, z_2[1]=0.0196, z_2[2]=0.0172, z_2[3]=0.0029, z_2[4]=0.0216, z_2[5]=0.0408, z_2[6]=0.0824, z_2[7]=0.1290, z_2[8]=0.1595, z_2[9]=0.2061, z_2[10]=0.2863, z_2[11]=0.3714, z_2[12]=0.3846, z_2[13]=0.6517, z_2[14]=0.7347, z_2[15]=0.8052, z_2[16]=0.8435, z_2[17]=0.8784, z_2[18]=0.9521, z_2[19]=1.0373, z_2[20]=1.0311, z_2[21]=1.1232;

    gr2 = new TGraph(n2,x_2,y_2);
    gr2->SetLineColor(2);
    gr2->SetLineWidth(4);
    gr2->SetMarkerColor(5);
    gr2->SetMarkerSize(1.5);
    gr2->SetMarkerStyle(21);
    gr2->SetTitle("Misalignment in X axis");
    gr2->GetXaxis()->SetTitle("Rot_X real [mrad]");
    gr2->GetXaxis()->SetLabelSize(0.02);
    gr2->GetYaxis()->SetTitle("Rot_X calculated [mrad]");
    gr2->GetYaxis()->SetLabelSize(0.02);
    gr2->Draw("ACP");

    TF1 *f2 = new TF1("f2", "[0]*x+[1]", 0., 17.);
    f2->SetParameters(0,0);
    f2->SetLineColor(4);
    gr2->Fit("f2","","",0,12);
    TF1* fit2 = gr2->GetFunction("f2");

    // TCanvas::Update() draws the frame, after which one can change it
    c2->Update();
    c2->GetFrame()->SetFillColor(21);
    c2->GetFrame()->SetBorderSize(12);
    c2->Modified();

    TLegend* LEG2= new TLegend(0.15,0.72,0.45,0.87); // Set legend position
    LEG2->SetBorderSize(1);
    LEG2->SetFillColor(0);
    LEG2->SetMargin(0.2);
    LEG2->SetTextSize(0.03);
    sprintf(leg, "Fitted linear a*x+b");
    LEG2->AddEntry(f2, leg, "l");
    sprintf(leg, "a = %f", fit2->GetParameter(0));
    LEG2->AddEntry("", leg, "l");
    sprintf(leg, "b = %f", fit2->GetParameter(1));
    LEG2->AddEntry("", leg, "l");
    LEG2->Draw();

    c2->cd(2);
    Double_t diff_2[n2];
    for (Int_t i=0; i<n2; i++) { diff_2[i] = TMath::Abs(y_2[i] - x_2[i])/x_2[i]; }
    // fit2->GetParameter(0)*i+fit2->GetParameter(1)

    gr2_b = new TGraph(n2,x_2,diff_2);
    gr2_b->SetLineColor(2);
    gr2_b->SetLineWidth(2);
    gr2_b->SetMarkerColor(5);
    gr2_b->SetMarkerSize(1.5);
    gr2_b->SetMarkerStyle(21);
    gr2_b->SetTitle("Misalignment in X axis - Difference");
    gr2_b->GetXaxis()->SetTitle("Rot_X real [mrad]");
    gr2_b->GetXaxis()->SetLabelSize(0.02);
    gr2_b->GetYaxis()->SetTitle("Difference Calculated-Real [mrad]");
    gr2_b->GetYaxis()->SetLabelSize(0.02);
    gr2_b->Draw("ACP");

    // ------------------------------ CALCULATION OF MISALIGNMENT ON THE OTHER AXIS ------------------------------ //

    TCanvas* c2_b = new TCanvas("c2_b","c2_b",200,10,600,400);
    c2_b->Divide(2,1);
    c2_b->SetFillColor(42);
    c2_b->SetGrid();
    c2_b->cd(1);

    gr3 = new TGraph(n2,x_2,z_2);
    gr3->SetLineColor(2);
    gr3->SetLineWidth(2);
    gr3->SetMarkerColor(5);
    gr3->SetMarkerSize(1.5);
    gr3->SetMarkerStyle(21);
    gr3->SetTitle("Calculated misalignment on Y axis VS induced displacement on X axis");
    gr3->GetXaxis()->SetTitle("Rot_X real [mrad]");
    gr3->GetXaxis()->SetLabelSize(0.02);
    gr3->GetYaxis()->SetTitle("Rot_Y calculated [mrad]");
    gr3->GetYaxis()->SetLabelSize(0.02);
    gr3->Draw("ACP");

    c2_b->cd(2);
    Double_t diff_x[n];
    for(i=0; i<n; i++) { diff_x[i] = TMath::Abs(z_2[i] - x_2[i])/x_2[i]; }

    gr3_b = new TGraph(n2,x_2,diff_x);
    gr3_b->SetLineColor(2);
    gr3_b->SetLineWidth(2);
    gr3_b->SetMarkerColor(5);
    gr3_b->SetMarkerSize(1.5);
    gr3_b->SetMarkerStyle(21);
    gr3_b->SetTitle("Difference on Y axis VS induced displacement on X axis");
    gr3_b->GetXaxis()->SetTitle("Rot_X real [mrad]");
    gr3_b->GetXaxis()->SetLabelSize(0.02);
    gr3_b->GetYaxis()->SetTitle("Difference Calculated-Real [mrad]");
    gr3_b->GetYaxis()->SetLabelSize(0.02);
    gr3_b->Draw("ACP");

}
