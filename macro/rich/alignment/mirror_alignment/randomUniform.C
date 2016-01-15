#include <iostream>
#include <ctime>
#include <cstdlib>

int rand_numb_gen()
{
  static const int SIZE = 36;		// Generated numbers
  int r[SIZE];
  double random_num[SIZE];
  srand ( time(NULL) ); 		// Initialize the generator

  TH1F *h1 = new TH1F("h1", "h1 title", 2001, -1.5, 1.5);

  for (int i=0; i<SIZE; i++) {
    
    const int k = i;
    r[k] = rand() %2001 - 1000; 		// Produces numbers between [-1000; +1000]

    random_num[k] = r[k]/1000.0; 	// This will create random floating point numbers between [-1.000; 1.000]

    // To increase the precision (here 4 digits), increase the range of the generated number r
  h1->Fill(random_num[k]);

  }
  
  for (int j=0; j<SIZE; j++) {
    const int l = j;
    cout << "Random number " << l+1 << " : " << random_num[l] << endl;
  }

  TH1F *h2 = new TH1F("h2","h2 title", 400, -1., 1.);
  h2.FillRandom("gaus", 50000);

  TCanvas *c1 = new TCanvas("random histo", "h1", 10, 10, 510, 510);
  h1->Draw();

  TCanvas *c2 = new TCanvas("random histo","h2",10, 10, 510, 510);
  h2->Draw();

  return;
}
