#include <iostream>
#include <ctime>
#include <cstdlib>
#include <iomanip>

int randomGauss(double sigma = 1)
{
  double sig = sigma;
  TRandom3 r1;
  TH1F *h1 = new TH1F("h1","h1",9999,-5,5);
  //TH1F *h2 = new TH1F("h2","h2",100,-5,5);

  for (Int_t i=0;i<100000;i++) {
    //TRandom3 r2;
    Double_t x1 = r1.Gaus(0,sig);
    h1->Fill(x1);
    //h1->Fill(grandom->Gaus(0,1));
    //h2->Fill(r2);
  }

  h1->Draw();
  //h2->Draw("same");
}

/* -----------------------------------------------------

The following basic Random distributions are provided:

   -Exp(tau)
   -Integer(imax)
   -Gaus(mean,sigma)
   -Rndm()
   -Uniform(x1)
   -Landau(mpv,sigma)
   -Poisson(mean)
   -Binomial(ntot,prob)*/

