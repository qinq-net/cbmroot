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
  double rand[72];
  double r;
  int k = 0;
  int ite = 0;

  TRandom3 rand_Gaus;
  rand_Gaus.SetSeed();

  while (k<72)
  {
    double r = rand_Gaus.Gaus(0,sigma);
    //cout << "r = " << r << endl;
    ite++;
    if ((r<-1) || (r>1)) {continue;}
    else {
      rand[k] = r;
      //cout << "Random array " << k << " = " << rand[k] << "." << endl << endl;
      k++;
    }
  }

  cout << "nb d'iterations : " << ite << endl;
  int l = 0;

  sprintf(PATH, "/home/jordan/Documents/Cbmroot/cbmroot/geometry/rich/mirror_alignment/Variables.txt");
  ofstream outfile(PATH, ios::out | ios::trunc);

  if(outfile) {
    for(int i=0; i<9; i++) {
      for(int j=1; j<5; j++) {
      
        sprintf(Variables_y, "<variable name=\"misalignment_%d_%d_y\" value=\"", i, j);

        ofstream outfile(PATH, ios::out | ios::app);
        outfile << Variables_y << rand[l] << "\"/>" << endl;
        outfile.close();
        l++;
      }
    }

    for(int i=0; i<9; i++) {
      for(int j=1; j<5; j++) {
      
        sprintf(Variables_x, "<variable name=\"misalignment_%d_%d_x\" value=\"", i, j);

        ofstream outfile(PATH, ios::out | ios:: app);
        outfile << Variables_x << rand[l] << "\"/>" << endl;
        outfile.close();
        l++;
      }
    }

  }
  else {cout << "Error in the opening !" << endl;}

//  ~rand_Gaus ();

  rand_Gaus.GetSeed();
  return;
}
