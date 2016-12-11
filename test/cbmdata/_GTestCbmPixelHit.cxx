#include "CbmPixelHit.h"

#include "TVector3.h"

#include "gtest/gtest.h"
#include "gtest/gtest-spi.h"

// Since some functions in CbmHit are protected we have
// to create a derived class without any data members
// which simply forwards the function
// calls to the base class

void comparePixelHitDataMembers(CbmPixelHit& test, HitType type, Double_t z, Double_t dz,
    Int_t refid, Int_t address, CbmMatch* match, Double_t time, Double_t errortime,
    Double_t x, Double_t dx, Double_t y, Double_t dy, Double_t dxy)
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

  retValFloat = test.GetX();
  EXPECT_FLOAT_EQ(x, retValFloat);

  retValFloat = test.GetDx();
  EXPECT_FLOAT_EQ(dx, retValFloat);

  retValFloat = test.GetY();
  EXPECT_FLOAT_EQ(y, retValFloat);

  retValFloat = test.GetDy();
  EXPECT_FLOAT_EQ(dy, retValFloat);

  retValFloat = test.GetDxy();
  EXPECT_FLOAT_EQ(dxy, retValFloat);
}


TEST(_GTestCbmPixelHit, CheckDefaultConstructor)
{
  CbmPixelHit test;
  {
    SCOPED_TRACE("CheckDefaultConstructor");
    comparePixelHitDataMembers(test, kPIXELHIT, 0., 0., -1, -1, nullptr, -1., -1., 0., 0., 0., 0., 0.);
  }
}

TEST(_GTestCbmPixelHit, CheckStandardConstructor)
{
  CbmPixelHit test{-1, 0., 0., 0., 0., 0., 0., 0., -1};
  {
    SCOPED_TRACE("CheckStandardConstructor");
    comparePixelHitDataMembers(test, kPIXELHIT, 0., 0., -1, -1, nullptr, -1., -1., 0., 0., 0., 0., 0.);
  }
}

TEST(_GTestCbmPixelHit, CheckStandardConstructorWithTime)
{
  CbmPixelHit test{-1, 0., 0., 0., 0., 0., 0., 0., -1, -2., -2.};
  {
    SCOPED_TRACE("CheckStandardConstructorWithTime");
    comparePixelHitDataMembers(test, kPIXELHIT, 0., 0., -1, -1, nullptr, -2., -2., 0., 0., 0., 0., 0.);
  }
}

TEST(_GTestCbmPixelHit, CheckStandardConstructorTVector3)
{
  TVector3 pos{0., 0., 0.,};
  TVector3 poserror{0., 0., 0.,};
  CbmPixelHit test{-1, pos, poserror, 0., -1};
  {
    SCOPED_TRACE("CheckStandardConstructor");
    comparePixelHitDataMembers(test, kPIXELHIT, 0., 0., -1, -1, nullptr, -1., -1., 0., 0., 0., 0., 0.);
  }
}

TEST(_GTestCbmPixelHit, CheckStandardConstructorTVector3WithTime)
{
  TVector3 pos{0., 0., 0.,};
  TVector3 poserror{0., 0., 0.,};
  CbmPixelHit test{-1, pos, poserror, 0., -1, -2., -2};
  {
    SCOPED_TRACE("CheckStandardConstructorWithTime");
    comparePixelHitDataMembers(test, kPIXELHIT, 0., 0., -1, -1, nullptr, -2., -2., 0., 0., 0., 0., 0.);
  }
}

TEST(_GTestCbmPixelHit, CheckSetters)
{
  CbmPixelHit test;
  {
    SCOPED_TRACE("CheckSetters: Initial Test");
    comparePixelHitDataMembers(test, kPIXELHIT, 0., 0., -1, -1, nullptr, -1., -1., 0., 0., 0., 0., 0.);
  }

  test.SetX(-11.);
  {
    SCOPED_TRACE("CheckSetters: SetX");
    comparePixelHitDataMembers(test, kPIXELHIT, 0., 0., -1, -1, nullptr, -1., -1., -11., 0., 0., 0., 0.);
  }

  test.SetDx(-12.);
  {
    SCOPED_TRACE("CheckSetters: SetDx");
    comparePixelHitDataMembers(test, kPIXELHIT, 0., 0., -1, -1, nullptr, -1., -1., -11., -12., 0., 0., 0.);
  }

  test.SetY(-13.);
  {
    SCOPED_TRACE("CheckSetters: SetY");
    comparePixelHitDataMembers(test, kPIXELHIT, 0., 0., -1, -1, nullptr, -1., -1., -11., -12., -13., 0., 0.);
  }

  test.SetDy(-14.);
  {
    SCOPED_TRACE("CheckSetters: SetDy");
    comparePixelHitDataMembers(test, kPIXELHIT, 0., 0., -1, -1, nullptr, -1., -1., -11., -12., -13., -14., 0.);
  }

  test.SetDxy(-15.);
  {
    SCOPED_TRACE("CheckSetters: SetDxy");
    comparePixelHitDataMembers(test, kPIXELHIT, 0., 0., -1, -1, nullptr, -1., -1., -11., -12., -13., -14., -15.);
  }

  TVector3 pos{-21., -22., -23.};
  test.SetPosition(pos);
  {
    SCOPED_TRACE("CheckSetters: SetPosition");
    comparePixelHitDataMembers(test, kPIXELHIT, -23., 0., -1, -1, nullptr, -1., -1., -21., -12., -22., -14., -15.);
  }

  TVector3 poserror{-31., -32., -33.};
  test.SetPositionError(poserror);
  {
    SCOPED_TRACE("CheckSetters: SetPosition");
    comparePixelHitDataMembers(test, kPIXELHIT, -23., -33., -1, -1, nullptr, -1., -1., -21., -31., -22., -32., -15.);
  }

  TVector3 pos1;
  test.Position(pos1);
  EXPECT_FLOAT_EQ(pos1.X(), pos.X());
  EXPECT_FLOAT_EQ(pos1.Y(), pos.Y());
  EXPECT_FLOAT_EQ(pos1.Z(), pos.Z());

  TVector3 poserror1;
  test.PositionError(poserror1);
  EXPECT_FLOAT_EQ(poserror1.X(), poserror.X());
  EXPECT_FLOAT_EQ(poserror1.Y(), poserror.Y());
  EXPECT_FLOAT_EQ(poserror1.Z(), poserror.Z());
}

TEST(_GTestCbmMatch, CheckToString)
{
  CbmPixelHit test;
  EXPECT_STREQ("CbmPixelHit: address=-1 pos=(0,0,0) err=(0,0,0) dxy=0 refId=-1\n", test.ToString().c_str());
}
