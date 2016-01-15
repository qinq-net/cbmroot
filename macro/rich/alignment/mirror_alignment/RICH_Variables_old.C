#include <iostream>
#include <fstream>

using namespace std;

/* Write variables value (distributed according to a Gaussian function) in a specified output
 file. Copy and paste the output file to the geometry file, to take the new values into
account. */

void RICH_Variables(double sigma = 1) {

  static int const SIZE = 20000;
  char Variables_x[256];
  char Variables_y[256];
  char PATH[256];
  double r[36];

  TRandom3 rand_Gaus;
  rand_Gaus.SetSeed(0);
  TH1F *h1 = new TH1F("h_Gauss","h_Gauss title", 50, -1., 1.);

  for (int i=0; i<SIZE; i++) {
    h1->Fill(rand_Gaus.Gaus(0,sigma));
  }

  sprintf(PATH, "/home/pusan/Documents/CbmRoot_install/CbmRoot/geometry/rich/mirror_alignment/Variables.txt");
  ofstream outfile(PATH, ios::out | ios::trunc);

  if(outfile) {
    for(int i=0; i<9; i++) {
      for(int j=1; j<5; j++) {
      
        sprintf(Variables_y, "<variable name=\"misalignment_%d_%d_y\" value=\"", i, j);

        ofstream outfile(PATH, ios::out | ios:: app);
        outfile << Variables_y << h1->GetRandom() << "\"/>" << endl;
        outfile.close();
      }
    }

    for(int i=0; i<9; i++) {
      for(int j=1; j<5; j++) {
      
        sprintf(Variables_x, "<variable name=\"misalignment_%d_%d_x\" value=\"", i, j);

        ofstream outfile(PATH, ios::out | ios:: app);
        outfile << Variables_x << h1->GetRandom() << "\"/>" << endl;
        outfile.close();
      }
    }

  }
  else {cout << "Error in the opening !" << endl;}

  cout << endl << "Histogram used: " << endl << endl;
  h1->Draw();

//  ~rand_Gaus ();
//  ~h1 ();

  rand_Gaus.GetSeed();
  return;
}
