void draw_pmt_mapping_old()
{

    // --------------------------------------------------------------------------------------------------------------------- //
    // ------------------------- Draw misalignment points in Y axis and fit with a linear function ------------------------- //
    // --------------------------------------------------------------------------------------------------------------------- //

    // ------------------------------ CALCULATION OF THE MISALIGNMENT ON THE MISALIGNED AXIS ------------------------------ //

    TCanvas *c1 = new TCanvas("c1","c1",200,10,600,400);
    c1->SetFillColor(42);
    c1->SetGrid();

    const Int_t n = 4;
    Double_t x[n], y[n], z[n];
    x[0]=-54.598072, x[1]=-24.130849, x[2]=-54.598072, x[3]=-24.130849; //x[2]=0.3, x[3]=0.5, x[4]=0.75, x[5]=1, x[6]=1.5, x[7]=2, x[8]=2.5, x[9]=3, x[10]=4, x[11]=5, x[12]=5.236, x[13]=8.727, x[14]=10.472, x[15]=12.217, x[16]=13.963, x[17]=15.708, x[18]=17.453, x[19]=34.907;
    y[0]=132.050955, y[1]=132.050955, y[2]=149.792705, y[3]=149.792705; //y[2]=0.2607, y[3]=0.4647, y[4]=0.7240, y[5]=0.9760, y[6]=1.4923, y[7]=2.0099, y[8]=2.5283, y[9]=3.0472, y[10]=4.0625, y[11]=5.0712, y[12]=5.3034, y[13]=8.6172, y[14]=10.1330, y[15]=11.3784, y[16]=12.3138, y[17]=11.9113, y[18]=12.1449, y[19]=14.1062;

    gr = new TGraph(n,x,y);
    gr->SetLineColor(2);
    gr->SetLineWidth(4);
    gr->SetMarkerColor(5);
    gr->SetMarkerSize(1.5);
    gr->SetMarkerStyle(21);
    gr->SetTitle("PMT Mapping");
    gr->GetXaxis()->SetTitle("PMT_X");
    gr->GetXaxis()->SetLabelSize(0.02);
    gr->GetYaxis()->SetTitle("PMT_Y");
    gr->GetYaxis()->SetLabelSize(0.02);
    gr->Draw("ACP");

    /*TF1 *f1 = new TF1("f1", "[0]*x+[1]", 0., 35.);
    f1->SetParameters(0,0);
    f1->SetLineColor(4);
    gr->Fit("f1","","",0,11);
    TF1* fit = gr->GetFunction("f1");*/

    // TCanvas::Update() draws the frame, after which one can change it
    c1->Update();
    c1->GetFrame()->SetFillColor(21);
    c1->GetFrame()->SetBorderSize(12);
    c1->Modified();

    /*TLegend* LEG= new TLegend(0.15,0.72,0.45,0.87); // Set legend position
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
    LEG->Draw();*/

}
