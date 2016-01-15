#include <iostream>
#include <fstream>

using namespace std;

/* Write variables value (distributed according to a Gaussian function) in a specified output
 file. Copy and paste the output file to the geometry file, to take the new values into
account. */

void RICH_Variables_bad() {

  static int const SIZE = 36;
  int r1[SIZE];
  int r2[SIZE];
  double random_num1[SIZE];
  double random_num2[SIZE];
  char Variables_x[256];
  char Variables_y[256];
  char FileName[256];
  char PATH[256];

  srand ( time(NULL) ); 		// Initialize the generator

// Creation of array of size SIZE of random numbers.
  for (int l=0; l<SIZE; l++) {
    const int k = l;
    r1[k] = rand() %2001 - 1000; 	// Produces numbers between [-1000; +1000]
    random_num1[k] = r1[k]/1000.0; 	// This will create random floating point numbers between [-1.000; 1.000]
    // To increase the precision (here 3 digits), increase the range of the generated number r
    r2[k] = rand() %2001 - 1000;
    random_num2[k] = r2[k]/1000.0;
  }

  sprintf(PATH, "/home/pusan/Documents/CbmRoot_install/CbmRoot/geometry/rich/mirror_alignment/Variables.txt");
  ofstream outfile(PATH, ios::out | ios::trunc);

  if(outfile) {
  for(int i=0; i<9; i++) {
    for(int j=1; j<5; j++) {
      
//      sprintf(Variables_x, "<variable name=\"misalignment_%d_%d_x\" value=\"", i, j);
      sprintf(Variables_y, "<variable name=\"misalignment_%d_%d_y\" value=\"", i, j);

      /*cout << "VARIABLES : " << endl;
      cout << "X : " << Variables_x << endl;
      cout << " et Y : " << Variables_y << endl;*/

      ofstream outfile(PATH, ios::out | ios:: app);
//      outfile << Variables_x << random_num1[i*4+j-1] << "\"/>" << endl;
      outfile << Variables_y << random_num2[i*4+j-1] << "\"/>" << endl;
      outfile.close();
    }
  }

  for(int i=0; i<9; i++) {
    for(int j=1; j<5; j++) {
      
      sprintf(Variables_x, "<variable name=\"misalignment_%d_%d_x\" value=\"", i, j);
//      sprintf(Variables_y, "<variable name=\"misalignment_%d_%d_y\" value=\"", i, j);

      /*cout << "VARIABLES : " << endl;
      cout << "X : " << Variables_x << endl;
      cout << " et Y : " << Variables_y << endl;*/

      ofstream outfile(PATH, ios::out | ios:: app);
      outfile << Variables_x << random_num1[i*4+j-1] << "\"/>" << endl;
//      outfile << Variables_y << random_num2[i*4+j-1] << "\"/>" << endl;
      outfile.close();
    }
  }


  }

  else {cout << "Error in the opening !" << endl;}

  /*for (int m=0; m<SIZE; m++) {
    const int l = m;
    cout << "Random number 1 : " << l+1 << " : " << random_num1[l] << endl;
  }

  for (int n=0; n<SIZE; n++) {
    const int l = n;
    cout << "Random number 2 : " << l+1 << " : " << random_num2[l] << endl;
  }*/

  return;
}
