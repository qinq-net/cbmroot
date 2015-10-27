#include "Riostream.h"
#include "TString.h"
#include <map>
void readHV(TString inFile="exampleHV.txt")
{
  ifstream in;
  in.open(inFile);
  Int_t time_high(-1), time_low(-1); 
  Int_t chID(-1);
  Float_t voltage(0.0), current(0.0);
  
  TString line("");
  char cline[500];
  if (!in){
    cout << inFile << " not found!" << endl;
  } else {
    cout << inFile << " found!" << endl;
    while(in){
      in.getline(cline,500);
      line = cline;
      cout << line << endl;
      TString sTime(line(0,13)),sChID(line(48,3)),sVoltage(line(69,12)),sCurrent(line(131,12));
      chID = sChID.Atoi();
      voltage = sVoltage.Atof();
      current = sCurrent.Atof();
      cout << sTime  << endl;
      cout << chID << endl;
      cout << voltage << endl;
      cout << current << endl;

      }
  }

}
