void calcAngles_drawPoints()
{
    // -------------------- Calculate angles of misalignment -------------------- //

    Float_t Focal_length = 150;
    Float_t Delta_Phi = 0.026939; //Mirr_3_2: first fit: 1.485729; second fit: 1.759041;
    Float_t Delta_Lambda = -0.139404; //Mirr_3_2: first fit: 2.70437; second fit: 3.219738;

    Float_t q = TMath::ATan(Delta_Phi/Delta_Lambda);
    Float_t A = Delta_Lambda/TMath::Cos(q);
    Float_t Alpha = TMath::ATan(A/Focal_length)*0.5*TMath::Power(10,3);
    //Float_t Deg = Alpha*TMath::RadToDeg();
    //Float_t Rad = Alpha*TMath::DegToRad()*TMath::Power(10,3);
    Float_t mis_x = TMath::ATan(Delta_Phi/Focal_length)*0.5*TMath::Power(10,3);
    Float_t mis_y = TMath::ATan(Delta_Lambda/Focal_length)*0.5*TMath::Power(10,3);

    cout << "Total angle of misalignment alpha [mrad] = " << Alpha << endl;
    cout << "Angle of misalignment mis_x [mrad] = " << mis_x << endl;
    cout << "Angle of misalignment mis_y [mrad] = " << mis_y << endl << endl;
    //cout << "Angle of misalignment Deg Conv = " << Deg << endl;
    //cout << "Angle of misalignment mRad Conv = " << Rad << endl;

    // -------------------- Calculate angles of misalignment - 2nd set -------------------- //

    //Focal_length = 150;
    Float_t Delta_Phi_2 = 0.026939; //Mirr_3_2: first fit: 1.485729; second fit: 1.759041;
    Float_t Delta_Lambda_2 = -0.139404; //Mirr_3_2: first fit: 2.70437; second fit: 3.219738;

    Float_t q_2 = TMath::ATan(Delta_Phi_2/Delta_Lambda_2);
    Float_t A_2 = Delta_Lambda_2/TMath::Cos(q_2);
    Float_t Alpha_2 = TMath::ATan(A_2/Focal_length)*0.5*TMath::Power(10,3);
    Float_t mis_x_2 = TMath::ATan(Delta_Phi_2/Focal_length)*0.5*TMath::Power(10,3);
    Float_t mis_y_2 = TMath::ATan(Delta_Lambda_2/Focal_length)*0.5*TMath::Power(10,3);

    cout << "Total angle of misalignment alpha_2 [mrad] = " << Alpha_2 << endl;
    cout << "Angle of misalignment mis_x_2 [mrad] = " << mis_x_2 << endl;
    cout << "Angle of misalignment mis_y_2 [mrad] = " << mis_y_2 << endl;

    // ---------- Draw misalignment points and fit with a polynomial function ---------- //

    /*TCanvas *c1 = new TCanvas("c1","c1",200,10,600,400);
    c1->SetFillColor(42);
    c1->SetGrid();

    const Int_t n = 7;
    Double_t x[n], y[n];
    x[0]=0.05, x[1]=0.1, x[2]=0.2, x[3]=0.3, x[4]=0.5, x[5]=1, x[6]=2;
    y[0]=-0.0773295, y[1]=-0.171386, y[2]=-0.288228, y[3]=-0.387177, y[4]=-0.501646, y[5]=-0.628218, y[6]=-0.6732;
    gr = new TGraph(n,x,y);
    gr->SetLineColor(2);
    gr->SetLineWidth(4);
    gr->SetMarkerColor(4);
    gr->SetMarkerSize(1.5);
    gr->SetMarkerStyle(21);
    gr->SetTitle("PolN=1 fit");
    gr->GetXaxis()->SetTitle("Rot_X real [deg]");
    gr->GetYaxis()->SetTitle("Rot_X calculated [deg]");
    gr->Draw("ACP");

    gr->Fit("pol1","","");
   
    // TCanvas::Update() draws the frame, after which one can change it
    c1->Update();
    c1->GetFrame()->SetFillColor(21);
    c1->GetFrame()->SetBorderSize(12);
    c1->Modified();

    TCanvas *c2 = new TCanvas("c2","c2",200,10,600,400);
    c2->SetFillColor(42);
    c2->SetGrid();
    gr2 = new TGraph(n,x,y);
    gr2->SetLineColor(2);
    gr2->SetLineWidth(4);
    gr2->SetMarkerColor(4);
    gr2->SetMarkerSize(1.5);
    gr2->SetMarkerStyle(21);
    gr2->SetTitle("PolN=2 fit");
    gr2->GetXaxis()->SetTitle("Rot_X real [deg]");
    gr2->GetYaxis()->SetTitle("Rot_X calculated [deg]");
    gr2->Draw("ACP");
    gr2->Fit("pol2","","");

    c2->Update();
    c2->GetFrame()->SetFillColor(21);
    c2->GetFrame()->SetBorderSize(12);
    c2->Modified();

    TCanvas *c3 = new TCanvas("c3","c3",200,10,600,400);
    c3->SetFillColor(42);
    c3->SetGrid();
    gr3 = new TGraph(n,x,y);
    gr3->SetLineColor(2);
    gr3->SetLineWidth(4);
    gr3->SetMarkerColor(4);
    gr3->SetMarkerSize(1.5);
    gr3->SetMarkerStyle(21);
    gr3->SetTitle("PolN=4 fit");
    gr3->GetXaxis()->SetTitle("Rot_X real [deg]");
    gr3->GetYaxis()->SetTitle("Rot_X calculated [deg]");
    gr3->Draw("ACP");
    gr3->Fit("pol7","","");

    c3->Update();
    c3->GetFrame()->SetFillColor(21);
    c3->GetFrame()->SetBorderSize(12);
    c3->Modified();*/

}