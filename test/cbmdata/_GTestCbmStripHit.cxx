#include "CbmStripHit.h"

#include "TVector3.h"

#include "gtest/gtest.h"
#include "gtest/gtest-spi.h"

// Since some functions in CbmHit are protected we have
// to create a derived class without any data members
// which simply forwards the function
// calls to the base class

void compareStripHitDataMembers(CbmStripHit& test, HitType type, Double_t z, Double_t dz,
    Int_t refid, Int_t address, CbmMatch* match, Double_t time, Double_t errortime,
    Double_t u, Double_t du, Double_t phi, Double_t dphi)
{
  Int_t retValInt{-111};
  Float_t retValFloat{-111.};

  EXPECT_EQ(type, test.GetType());

  retValFloat = test.GetZ();
  EXPECT_FLOAT_EQ(z, retValFloat);

  retValFloat = test.GetDz();
  EXPECT_FLOAT_EQ(dz, retValFloat);

  retValInt = test.GetRefId();
  EXPECT_EQ(refid, retValInt);

  retValInt = test.GetAddress();
  EXPECT_EQ(address, retValInt);

  EXPECT_EQ(match, test.GetMatch());

  retValFloat =  test.GetTime();
  EXPECT_FLOAT_EQ(time, retValFloat );

  retValFloat = test.GetTimeError();
  EXPECT_FLOAT_EQ(errortime, retValFloat);

  retValFloat = test.GetU();
  EXPECT_FLOAT_EQ(u, retValFloat);

  retValFloat = test.GetDu();
  EXPECT_FLOAT_EQ(du, retValFloat);

  retValFloat = test.GetPhi();
  EXPECT_FLOAT_EQ(phi, retValFloat);

  retValFloat = test.GetDphi();
  EXPECT_FLOAT_EQ(dphi, retValFloat);
}


TEST(_GTestCbmStripHit, CheckDefaultConstructor)
{
  CbmStripHit test;
  {
    SCOPED_TRACE("CheckDefaultConstructor");
    compareStripHitDataMembers(test, kSTRIPHIT, 0., 0., -1, -1, nullptr, -1., -1., 0., 0., 0., 0.);
  }
}

TEST(_GTestCbmStripHit, CheckStandardConstructor)
{
  CbmStripHit test{-1, 0., 0., 0., 0., 0., 0., -1};
  {
    SCOPED_TRACE("CheckStandardConstructor");
    compareStripHitDataMembers(test, kSTRIPHIT, 0., 0., -1, -1, nullptr, -1., -1., 0., 0., 0., 0.);
  }
}

TEST(_GTestCbmStripHit, CheckStandardConstructorWithTime)
{
  CbmStripHit test{-1, 0., 0., 0., 0., 0., 0., -1, -2., -2.};
  {
    SCOPED_TRACE("CheckStandardConstructorWithTime");
    compareStripHitDataMembers(test, kSTRIPHIT, 0., 0., -1, -1, nullptr, -2., -2., 0., 0., 0., 0.);
  }
}

TEST(_GTestCbmStripHit, CheckStandardConstructorTVector3)
{
  TVector3 pos{0., 0., 0.,};
  TVector3 poserror{0., 0., 0.,};
  CbmStripHit test{-1, pos, poserror, -1};
  {
    SCOPED_TRACE("CheckStandardConstructor");
    compareStripHitDataMembers(test, kSTRIPHIT, 0., 0., -1, -1, nullptr, -1., -1., 0., 0., 0., 0.);
  }
}

TEST(_GTestCbmStripHit, CheckStandardConstructorTVector3WithTime)
{
  TVector3 pos{0., 0., 0.,};
  TVector3 poserror{0., 0., 0.,};
  CbmStripHit test{-1, pos, poserror, -1, -2., -2};
  {
    SCOPED_TRACE("CheckStandardConstructorWithTime");
    compareStripHitDataMembers(test, kSTRIPHIT, 0., 0., -1, -1, nullptr, -2., -2., 0., 0., 0., 0.);
  }
}

TEST(_GTestCbmStripHit, CheckSetters)
{
  CbmStripHit test;
  {
    SCOPED_TRACE("CheckSetters: Initial Test");
    compareStripHitDataMembers(test, kSTRIPHIT, 0., 0., -1, -1, nullptr, -1., -1., 0., 0., 0., 0.);
  }

  test.SetU(-11.);
  {
    SCOPED_TRACE("CheckSetters: SetU");
    compareStripHitDataMembers(test, kSTRIPHIT, 0., 0., -1, -1, nullptr, -1., -1., -11., 0., 0., 0.);
  }

  test.SetDu(-12.);
  {
    SCOPED_TRACE("CheckSetters: SetDu");
    compareStripHitDataMembers(test, kSTRIPHIT, 0., 0., -1, -1, nullptr, -1., -1., -11., -12., 0., 0.);
  }

  test.SetPhi(-13.);
  {
    SCOPED_TRACE("CheckSetters: SetPhi");
    compareStripHitDataMembers(test, kSTRIPHIT, 0., 0., -1, -1, nullptr, -1., -1., -11., -12., -13., 0.);
  }

  test.SetDphi(-14.);
  {
    SCOPED_TRACE("CheckSetters: SetDphi");
    compareStripHitDataMembers(test, kSTRIPHIT, 0., 0., -1, -1, nullptr, -1., -1., -11., -12., -13., -14.);
  }

}

TEST(_GTestCbmMatch, CheckToString)
{
  CbmStripHit test;
  EXPECT_STREQ("CbmStripHit: address=-1 pos=(0,0,0) err=(0,0,0) refId=-1\n", test.ToString().c_str());
}
