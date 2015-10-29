#include "Riostream.h"
#include "TString.h"
#include <map>
#include "TH1.h"
#include "TH2.h"
void readHV(TString inFile="exampleHV.txt")
{
  Bool_t debug = false;
  ifstream in;
  in.open(inFile);
  Int_t time_high(-1), time_low(-1), chID(-1), lineLength(0);
  Float_t voltage(0.0), current(0.0);
  
  TString line("");
  TString sTime(""), sChID(""), sVoltage(""), sCurrent("");
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
      } else if (line.EndsWith("V")){

	line.ReplaceAll("WIENER-CRATE-MIB::outputMeasurementTerminalVoltage.u","");
	line.ReplaceAll(" V","");
	if (debug)
	  cout << "V: ";
	sVoltage = line(4,lineLength-4);
	sCurrent = "";
      } else {
	if (debug)
	  cout << "T: ";
	sTime = line;
      }
      if (debug)
	cout << lineLength << ":   " << line << endl;

      sChID = line(0,3);
      chID = sChID.Atoi();
      voltage = sVoltage.Atof();
      current = sCurrent.Atof();
      if (debug){
	cout << "T=" << sTime  << endl;
	cout << "C=" << chID << endl;
	cout << "V=" << voltage << endl;
	cout << "A=" << current << endl;
      }
      nLines++;
    }
    cout << "found " << nLines << " lines in file " << inFile << endl;
  }

}
