#include "Riostream.h"
#include "TString.h"
#include <map>
#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TDatime.h"
#include "TStyle.h"
void readHV(TString inFile="exampleHV.txt")
{
  TDatime da(2015,10,30,12,00,00);
  gStyle->SetTimeOffset(da.Convert());
  Bool_t debug = false;
  ifstream in;
  in.open(inFile);
  inFile.ReplaceAll(".txt",".root");
  TFile *out = new TFile(inFile,"RECREATE");
  Int_t year(-1), month(-1), day(-1), hour(-1), min(-1), sec(-1), msec(-1), chID(-1), deltaT(0), lineLength(0);
  Int_t lastDay(-1), lastHour(-1), lastMin(-1), lastSec(-1), lastMsec(-1);
  Float_t voltage(0.0), current(0.0);
  TCanvas *c = new TCanvas("view","view",2*600,2*800);
  c->Divide(2,2);
  TH1F* hTime = new TH1F("hTime","hTime",60000,0,60000);
  TH1I* hChID = new TH1I("hChID","hChID",811,-0.5,810.5);
  hChID->SetXTitle("channel ID");
  TH1I* hDeltaT = new TH1I("hDeltaT","hDeltaT",1001,-0.5,1000.5);
  hDeltaT->SetXTitle("#Delta t (ms)");
  TH1F* hCurrent = new TH1F("hCurrent","hCurrent",100001,-1,1);
  hCurrent->SetXTitle("I (A)");
  TH1F* hVoltage = new TH1F("hVoltage","hVoltage",200001,-1,2000);
  hVoltage->SetXTitle("U (V)");
  std::map<Int_t, TH1F*> mVoltage;
  std::map<Int_t, TH1F*> mCurrent;
  TString line("");
  TString sTime(""), sChID(""), sVoltage(""), sCurrent(""), name("");
  char cline[500];
  if (!in){
    cout << inFile << " not found!" << endl;
  } else {
    cout << inFile << " found!" << endl;
    Int_t nLines = 0;
    while(in){
      //sTime = "";
      in.getline(cline,500);
      line = cline;
      lineLength = (Int_t)line.Length();
      line.ReplaceAll("= Opaque: Float: ",""); 
      if (line.EndsWith("A")){
	line.ReplaceAll("WIENER-CRATE-MIB::outputMeasurementCurrent.u","");
	line.ReplaceAll(" A","");
	if (debug)
	  cout << "A: ";
	sCurrent = line(4,lineLength-4);
	current = sCurrent.Atof();
	hCurrent->Fill(current);
      } else if (line.EndsWith("V")){
	line.ReplaceAll("WIENER-CRATE-MIB::outputMeasurementTerminalVoltage.u","");
	line.ReplaceAll(" V","");
	if (debug)
	  cout << "V: ";
	sVoltage = line(4,lineLength-4);
	sCurrent = "";
	voltage = sVoltage.Atof();
	hVoltage->Fill(voltage);
      } else if (line.BeginsWith("Loop")) {
	cout << line << endl;
      } else if (line.BeginsWith("2015")){
	sTime = line;
	year  = TString(line( 0,4)).Atoi(); 
	month = TString(line( 5,2)).Atoi();
	day   = TString(line( 8,2)).Atoi();
	hour  = TString(line(11,2)).Atoi();
	min   = TString(line(14,2)).Atoi();
	sec   = TString(line(17,2)).Atoi();
	msec  = TString(line(20,3)).Atoi();
	if (debug)
	  printf("T: %s\n   %i-%02i-%02i:%02i:%02i:%02i:%03i\n\n",sTime.Data(),year,month,day,hour,min,sec,msec);
	if (lastDay > 0){
	  deltaT = (msec - lastMsec) + ((sec - lastSec) + ((min - lastMin) + ((hour - lastHour) + (day - lastDay)*24)*60)*60)*1000;
	  hDeltaT->Fill(deltaT);
	  if (debug)
	    cout << deltaT << endl;
	}
	lastDay = day;
	lastHour = hour;
	lastMin = min;
	lastSec = sec;
	lastMsec = msec;
      } else {
	cout << "/" << line << "/ : Unknown content! To be ignored" << endl;
      }
      if (debug)
	cout << lineLength << ":   " << line << endl;

      sChID = line(0,3);
      chID = sChID.Atoi();
      hChID->Fill(chID);

      if (mVoltage.find(chID) == mVoltage.end()){
	name.Form("hVoltage%03i",chID);
	mVoltage[chID] = new TH1F(name,name,200001,-1,2000);
      }
      mVoltage[chID]->Fill(voltage);
      if (mCurrent.find(chID) == mCurrent.end()){
	name.Form("hCurrent%03i",chID);
	mCurrent[chID] = new TH1F(name,name,100001,-1,1);
      }
      mCurrent[chID]->Fill(current);
    
      if (debug){
	cout << "T=" << sTime  << endl;
	cout << "C=" << chID << endl;
	cout << "V=" << voltage << endl;
	cout << "A=" << current << endl;
      }
      nLines++;
    }
    cout << "found " << nLines << " lines in file " << inFile << endl;
    c->cd(1)->SetLogy(1);
    hCurrent->DrawCopy();
    c->cd(2)->SetLogy(1);
    hVoltage->DrawCopy();
    c->cd(3);
    hDeltaT->DrawCopy();
    //hTime->GetXaxis()->SetLabelSize(0.01);
    //hTime->GetXaxis()->SetTimeDisplay(1);
    //hTime->GetXaxis()->SetTimeFormat("%H/%m/%s");
    //hTime->DrawCopy();
    c->cd(4);
    hChID->DrawCopy();
  }
  out->cd();
  for (std::map<Int_t, TH1F*>::iterator it=mCurrent.begin(); it!=mCurrent.end();it++){
    it->second->Write("",TObject::kOverwrite);
  }
  for (std::map<Int_t, TH1F*>::iterator it=mVoltage.begin(); it!=mVoltage.end();it++){
    it->second->Write("",TObject::kOverwrite);
  }
}
