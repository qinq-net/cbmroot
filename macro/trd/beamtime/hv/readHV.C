#include "Riostream.h"
#include "TString.h"
#include <map>
#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TFile.h"
void readHV(TString inFile="exampleHV.txt")
{
  Bool_t debug = false;
  ifstream in;
  in.open(inFile);
  inFile.ReplaceAll(".txt",".root");
  TFile *out = new TFile(inFile,"RECREATE");
  Int_t time_high(-1), time_low(-1), chID(-1), lineLength(0);
  Float_t voltage(0.0), current(0.0);
  TCanvas *c = new TCanvas("view","view",2*600,2*800);
  c->Divide(2,2);
  TH1F* hCurrent = new TH1F("hCurrent","hCurrent",100001,-1,1);
  TH1F* hVoltage = new TH1F("hVoltage","hVoltage",200001,-1,2000);
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
      } else {
	if (debug)
	  cout << "T: ";
	sTime = line;
      }
      if (debug)
	cout << lineLength << ":   " << line << endl;

      sChID = line(0,3);
      chID = sChID.Atoi();

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
    c->cd(1);
    hCurrent->DrawCopy();
    c->cd(2);
    hVoltage->DrawCopy();
  }
  out->cd();
  for (std::map<Int_t, TH1F*>::iterator it=mCurrent.begin(); it!=mCurrent.end();it++){
    it->second->Write("",TObject::kOverwrite);
  }
  for (std::map<Int_t, TH1F*>::iterator it=mVoltage.begin(); it!=mVoltage.end();it++){
    it->second->Write("",TObject::kOverwrite);
  }
}
