#include "CbmVertex.h"

#include "TMatrixFSym.h"

#include "gtest/gtest.h"
#include "gtest/gtest-spi.h"

  /** Position coordinates  [cm] **/
  Double32_t fX, fY, fZ;

  /** Chi2 of vertex fit **/
  Double32_t fChi2;

  /** Number of degrees of freedom of vertex fit **/
  Int_t fNDF;

  /** Number of tracks used for the vertex fit **/
  Int_t fNTracks;

  /** Covariance matrix for x, y, and z stored in an array. The
   ** sequence is a[0,0], a[0,1], a[0,2], a[1,1], a[1,2], a[2,2]
   **/
  Double32_t fCovMatrix[6];




 void compareVertexDataMembers(CbmVertex& test, Double_t x, Double_t y, Double_t z,
     Double_t chi2, Int_t ndf, Int_t ntracks, Double_t* cov)
{
  Int_t retValInt{-111};
  Double_t retValDouble{-111.};

  retValDouble = test.GetX();
  EXPECT_FLOAT_EQ(x, retValDouble);

  retValDouble = test.GetY();
  EXPECT_FLOAT_EQ(y, retValDouble);

  retValDouble = test.GetZ();
  EXPECT_FLOAT_EQ(z, retValDouble);

  retValDouble = test.GetChi2();
  EXPECT_FLOAT_EQ(chi2, retValDouble);

  retValInt = test.GetNDF();
  EXPECT_EQ(ndf, retValInt);

  retValInt = test.GetNTracks();
  EXPECT_EQ(ntracks, retValInt);

  if (cov) {
    retValDouble = test.GetCovariance(0,0);
    EXPECT_FLOAT_EQ(cov[0], retValDouble);

    retValDouble = test.GetCovariance(0,1);
    EXPECT_FLOAT_EQ(cov[1], retValDouble);

    retValDouble = test.GetCovariance(0,2);
    EXPECT_FLOAT_EQ(cov[2], retValDouble);

    retValDouble = test.GetCovariance(1,1);
    EXPECT_FLOAT_EQ(cov[3], retValDouble);

    retValDouble = test.GetCovariance(1,2);
    EXPECT_FLOAT_EQ(cov[4], retValDouble);

    retValDouble = test.GetCovariance(2,2);
    EXPECT_FLOAT_EQ(cov[5], retValDouble);
  }
}

 TEST(_GTestCbmVertex, CheckDefaultConstructor)
 {
   CbmVertex test;
   Double_t val[6] = {0., 0., 0., 0., 0., 0.};
   {
     SCOPED_TRACE("CheckDefaultConstructor");
     compareVertexDataMembers(test, 0., 0., 0., 0., 0, 0, val);
   }
 }

 TEST(_GTestCbmVertex, CheckStandardConstructor)
 {
   CbmVertex test{"Vertex","Vertex"};
   Double_t val[6] = {0., 0., 0., 0., 0., 0.};
   {
     SCOPED_TRACE("CheckStandardConstructor");
     compareVertexDataMembers(test, 0., 0., 0., 0., 0, 0, val);
   }
 }

 TEST(_GTestCbmVertex, CheckConstructorAllArguments)
 {
   TMatrixFSym tempCov(3);
   tempCov(0, 0) = 0.;
   tempCov(0, 1) = 1.;
   tempCov(0, 2) = 2.;
   tempCov(1, 0) = 3.;
   tempCov(1, 1) = 4.;
   tempCov(1, 2) = 5.;
   tempCov(2, 0) = 6.;
   tempCov(2, 1) = 7.;
   tempCov(2, 2) = 8.;

   Double_t val[6] = {0., 1., 2., 4., 5., 8.};
   CbmVertex test{"Vertex","Vertex", 1., 2., 3., 4., 5, 6, tempCov};
   {
     SCOPED_TRACE("CheckConstructorAllArguments");
     compareVertexDataMembers(test, 1., 2., 3., 4., 5, 6, val);
   }
 }

