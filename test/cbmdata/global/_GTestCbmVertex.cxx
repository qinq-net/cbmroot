#include "CbmVertex.h"

#include "TMatrixFSym.h"
#include "TVector3.h"

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
   TMatrixFSym Cov(3);
   Cov(0, 0) = 0.;      // 0 1 2
   Cov(0, 1) = 1.;      // 1 3 4
   Cov(0, 2) = 2.;      // 2 4 5
   Cov(1, 0) = 1.;
   Cov(1, 1) = 3.;
   Cov(1, 2) = 4.;
   Cov(2, 0) = 2.;
   Cov(2, 1) = 4.;
   Cov(2, 2) = 5.;

   Double_t val[6] = {0., 1., 2., 3., 4., 5.};
   CbmVertex test{"Vertex","Vertex", 1., 2., 3., 4., 5, 6, Cov};
   {
     SCOPED_TRACE("CheckConstructorAllArguments");
     compareVertexDataMembers(test, 1., 2., 3., 4., 5, 6, val);
   }
 }

 TEST(_GTestCbmVertex, CheckReset)
 {
   TMatrixFSym Cov(3);
   Cov(0, 0) = 0.;
   Cov(0, 1) = 1.;
   Cov(0, 2) = 2.;
   Cov(1, 0) = 1.;
   Cov(1, 1) = 3.;
   Cov(1, 2) = 4.;
   Cov(2, 0) = 2.;
   Cov(2, 1) = 4.;
   Cov(2, 2) = 5.;

   Double_t val[6] = {0., 1., 2., 3., 4., 5.};
   CbmVertex test{"Vertex","Vertex", 1., 2., 3., 4., 5, 6, Cov};
   {
     SCOPED_TRACE("CheckReset: Initial Test");
     compareVertexDataMembers(test, 1., 2., 3., 4., 5, 6, val);
   }

   Double_t val1[6] = {0., 0., 0., 0., 0., 0.};
   test.Reset();
   {
     SCOPED_TRACE("CheckReset: Check after reset");
     compareVertexDataMembers(test, 0., 0., 0., 0., 0, 0, val1);
   }
 }

 TEST(_GTestCbmVertex, CheckGetPosition)
 {
   TMatrixFSym Cov(3);
   Cov(0, 0) = 0.;
   Cov(0, 1) = 1.;
   Cov(0, 2) = 2.;
   Cov(1, 0) = 1.;
   Cov(1, 1) = 3.;
   Cov(1, 2) = 4.;
   Cov(2, 0) = 2.;
   Cov(2, 1) = 4.;
   Cov(2, 2) = 5.;

   Double_t val[6] = {0., 1., 2., 3., 4., 5.};
   CbmVertex test{"Vertex","Vertex", 1., 2., 3., 4., 5, 6, Cov};
   {
     SCOPED_TRACE("CheckGetPosition: Initial Test");
     compareVertexDataMembers(test, 1., 2., 3., 4., 5, 6, val);
   }

   TVector3 testVect;

   test.Position(testVect);
   {
     SCOPED_TRACE("CheckGetPosition: Check after Position");
     compareVertexDataMembers(test, 1., 2., 3., 4., 5, 6, val);
   }
   EXPECT_FLOAT_EQ(1., testVect.X());
   EXPECT_FLOAT_EQ(2., testVect.Y());
   EXPECT_FLOAT_EQ(3., testVect.Z());
 }


 TEST(_GTestCbmVertex, CheckGetCovMatrix)
  {
    TMatrixFSym Cov(3);
    Cov(0, 0) = 0.;
    Cov(0, 1) = 1.;
    Cov(0, 2) = 2.;
    Cov(1, 0) = 1.;
    Cov(1, 1) = 3.;
    Cov(1, 2) = 4.;
    Cov(2, 0) = 2.;
    Cov(2, 1) = 4.;
    Cov(2, 2) = 5.;

    Double_t val[6] = {0., 1., 2., 3., 4., 5.};
    CbmVertex test{"Vertex","Vertex", 1., 2., 3., 4., 5, 6, Cov};
    {
      SCOPED_TRACE("CheckGetCovMatrix: Initial Test");
      compareVertexDataMembers(test, 1., 2., 3., 4., 5, 6, val);
    }

    TMatrixFSym testCov(3);

    test.CovMatrix(testCov);
    {
      SCOPED_TRACE("CheckGetCovMatrix: Check after Position");
      compareVertexDataMembers(test, 1., 2., 3., 4., 5, 6, val);
    }
    for(Int_t i=0; i<3; ++i) {
      for(Int_t j=0; j<3; ++j) {
        Double_t origVal = Cov[i][j];
        Double_t testVal = testCov[i][j];
        EXPECT_FLOAT_EQ(testVal, origVal);
      }
    }
  }

 TEST(_GTestCbmVertex, CheckSetVertex)
  {
    TMatrixFSym Cov(3);
    Cov(0, 0) = 0.;
    Cov(0, 1) = 1.;
    Cov(0, 2) = 2.;
    Cov(1, 0) = 1.;
    Cov(1, 1) = 3.;
    Cov(1, 2) = 4.;
    Cov(2, 0) = 2.;
    Cov(2, 1) = 4.;
    Cov(2, 2) = 5.;

    Double_t val[6] = {0., 1., 2., 3., 4., 5.};
    CbmVertex test{"Vertex","Vertex", 1., 2., 3., 4., 5, 6, Cov};
    {
      SCOPED_TRACE("CheckSetVertex: Initial Test");
      compareVertexDataMembers(test, 1., 2., 3., 4., 5, 6, val);
    }

    TMatrixFSym testCov(3);
    testCov(0, 0) = -10.;
    testCov(0, 1) = -1.;
    testCov(0, 2) = -2.;
    testCov(1, 0) = -1.;
    testCov(1, 1) = -3.;
    testCov(1, 2) = -4.;
    testCov(2, 0) = -2.;
    testCov(2, 1) = -4.;
    testCov(2, 2) = -5.;

    Double_t testval[6] = {-10., -1., -2., -3., -4., -5.};

    test.SetVertex(-1., -2., -3., -4., -5, -6, testCov);
    {
      SCOPED_TRACE("CheckSetVertex: Check after SetVertex");
      compareVertexDataMembers(test, -1., -2., -3., -4., -5, -6, testval);
    }
  }

 TEST(_GTestCbmVertex, CheckPrint)
   {
     TMatrixFSym Cov(3);
     Cov(0, 0) = 0.;
     Cov(0, 1) = 1.;
     Cov(0, 2) = 2.;
     Cov(1, 0) = 1.;
     Cov(1, 1) = 3.;
     Cov(1, 2) = 4.;
     Cov(2, 0) = 2.;
     Cov(2, 1) = 4.;
     Cov(2, 2) = 5.;

     Double_t val[6] = {0., 1., 2., 3., 4., 5.};
     CbmVertex test{"Vertex","Vertex", 1., 2., 3., 4., 5, 6, Cov};
     {
       SCOPED_TRACE("CheckSetVertex: Initial Test");
       compareVertexDataMembers(test, 1., 2., 3., 4., 5, 6, val);
     }

     testing::internal::CaptureStdout();
     test.Print();
     std::string output = testing::internal::GetCapturedStdout();
     EXPECT_STREQ("[INFO   ] Vertex coord. (1,2,3) cm, chi2/ndf = 0.8, 6 tracks used\n", output.c_str());

     EXPECT_STREQ("Vertex: position (1.0000, 2.0000, 3.0000) cm, chi2/ndf = 0.8000, tracks used: 6", test.ToString().c_str());


     CbmVertex test1{"Vertex","Vertex", 1., 2., 3., 4., 0, 6, Cov};
     {
        SCOPED_TRACE("CheckSetVertex: Initial Test");
        compareVertexDataMembers(test1, 1., 2., 3., 4., 0, 6, val);
      }

      testing::internal::CaptureStdout();
      test1.Print();
      output = testing::internal::GetCapturedStdout();
      EXPECT_STREQ("[INFO   ] Vertex coord. (1,2,3) cm, chi2/ndf = 0, 6 tracks used\n", output.c_str());

      EXPECT_STREQ("Vertex: position (1.0000, 2.0000, 3.0000) cm, chi2/ndf = 0.0000, tracks used: 6", test1.ToString().c_str());
}
