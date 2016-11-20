#include "CbmHit.h"

#include "gtest/gtest.h"
#include "gtest/gtest-spi.h"

TEST(_GTestCbmHit , CheckDefaultConstructor)
{
  CbmHit test;

  Int_t retValInt{-111};
  Float_t retValFloat{-111.};

  EXPECT_EQ(kHIT, test.GetType());
  retValFloat = test.GetZ();
  EXPECT_FLOAT_EQ(0., retValFloat);
  retValFloat = test.GetZ();
  EXPECT_FLOAT_EQ(0., retValFloat);
  retValInt = test.GetRefId();
  EXPECT_EQ(-1, retValInt);
  retValInt = test.GetAddress();
  EXPECT_EQ(-1, retValInt);
  EXPECT_EQ(nullptr, test.GetMatch());
  retValFloat =  test.GetTime();
  EXPECT_FLOAT_EQ(-1., retValFloat );
  retValFloat = test.GetTimeError();
  EXPECT_FLOAT_EQ(-1., retValFloat);
}

TEST(_GTestCbmHit , CheckStandardConstructor)
{
  CbmHit test{kHIT, 0., 0., -1, -1};

  Int_t retValInt{-111};
  Float_t retValFloat{-111.};

  EXPECT_EQ(kHIT, test.GetType());
  retValFloat = test.GetZ();
  EXPECT_FLOAT_EQ(0., retValFloat);
  retValFloat = test.GetZ();
  EXPECT_FLOAT_EQ(0., retValFloat);
  retValInt = test.GetRefId();
  EXPECT_EQ(-1, retValInt);
  retValInt = test.GetAddress();
  EXPECT_EQ(-1, retValInt);
  EXPECT_EQ(nullptr, test.GetMatch());
  retValFloat =  test.GetTime();
  EXPECT_FLOAT_EQ(-1., retValFloat );
  retValFloat = test.GetTimeError();
  EXPECT_FLOAT_EQ(-1., retValFloat);
}

TEST(_GTestCbmHit , CheckStandardConstructorWithTime)
{
  CbmHit test{kHIT, 0., 0., -1, -1, -2.,-2.};

  Int_t retValInt{-111};
  Float_t retValFloat{-111.};

  EXPECT_EQ(kHIT, test.GetType());
  retValFloat = test.GetZ();
  EXPECT_FLOAT_EQ(0., retValFloat);
  retValFloat = test.GetZ();
  EXPECT_FLOAT_EQ(0., retValFloat);
  retValInt = test.GetRefId();
  EXPECT_EQ(-1, retValInt);
  retValInt = test.GetAddress();
  EXPECT_EQ(-1, retValInt);
  EXPECT_EQ(nullptr, test.GetMatch());
  retValFloat =  test.GetTime();
  EXPECT_FLOAT_EQ(-2., retValFloat );
  retValFloat = test.GetTimeError();
  EXPECT_FLOAT_EQ(-2., retValFloat);
}

TEST(_GTestCbmHit , TestSettersAndGetters)
{
  CbmHit test;

  Int_t retValInt{-111};
  Float_t retValFloat{-111.};

  EXPECT_EQ(kHIT, test.GetType());
  retValFloat = test.GetZ();
  EXPECT_FLOAT_EQ(0., retValFloat);
  retValFloat = test.GetDz();
  EXPECT_FLOAT_EQ(0., retValFloat);
  retValInt = test.GetRefId();
  EXPECT_EQ(-1, retValInt);
  retValInt = test.GetAddress();
  EXPECT_EQ(-1, retValInt);
  EXPECT_EQ(nullptr, test.GetMatch());
  retValFloat =  test.GetTime();
  EXPECT_FLOAT_EQ(-1., retValFloat );
  retValFloat = test.GetTimeError();
  EXPECT_FLOAT_EQ(-1., retValFloat);

  test.SetZ(-2.);

  EXPECT_EQ(kHIT, test.GetType());
  retValFloat = test.GetZ();
  EXPECT_FLOAT_EQ(-2., retValFloat);
  retValFloat = test.GetDz();
  EXPECT_FLOAT_EQ(0., retValFloat);
  retValInt = test.GetRefId();
  EXPECT_EQ(-1, retValInt);
  retValInt = test.GetAddress();
  EXPECT_EQ(-1, retValInt);
  EXPECT_EQ(nullptr, test.GetMatch());
  retValFloat =  test.GetTime();
  EXPECT_FLOAT_EQ(-1., retValFloat );
  retValFloat = test.GetTimeError();
  EXPECT_FLOAT_EQ(-1., retValFloat);

  test.SetDz(-2.);

  EXPECT_EQ(kHIT, test.GetType());
  retValFloat = test.GetZ();
  EXPECT_FLOAT_EQ(-2., retValFloat);
  retValFloat = test.GetDz();
  EXPECT_FLOAT_EQ(-2., retValFloat);
  retValInt = test.GetRefId();
  EXPECT_EQ(-1, retValInt);
  retValInt = test.GetAddress();
  EXPECT_EQ(-1, retValInt);
  EXPECT_EQ(nullptr, test.GetMatch());
  retValFloat =  test.GetTime();
  EXPECT_FLOAT_EQ(-1., retValFloat );
  retValFloat = test.GetTimeError();
  EXPECT_FLOAT_EQ(-1., retValFloat);

  test.SetRefId(-2);

  EXPECT_EQ(kHIT, test.GetType());
  retValFloat = test.GetZ();
  EXPECT_FLOAT_EQ(-2., retValFloat);
  retValFloat = test.GetDz();
  EXPECT_FLOAT_EQ(-2., retValFloat);
  retValInt = test.GetRefId();
  EXPECT_EQ(-2, retValInt);
  retValInt = test.GetAddress();
  EXPECT_EQ(-1, retValInt);
  EXPECT_EQ(nullptr, test.GetMatch());
  retValFloat =  test.GetTime();
  EXPECT_FLOAT_EQ(-1., retValFloat );
  retValFloat = test.GetTimeError();
  EXPECT_FLOAT_EQ(-1., retValFloat);

  test.SetAddress(-2);

  EXPECT_EQ(kHIT, test.GetType());
  retValFloat = test.GetZ();
  EXPECT_FLOAT_EQ(-2., retValFloat);
  retValFloat = test.GetDz();
  EXPECT_FLOAT_EQ(-2., retValFloat);
  retValInt = test.GetRefId();
  EXPECT_EQ(-2, retValInt);
  retValInt = test.GetAddress();
  EXPECT_EQ(-2, retValInt);
  EXPECT_EQ(nullptr, test.GetMatch());
  retValFloat =  test.GetTime();
  EXPECT_FLOAT_EQ(-1., retValFloat );
  retValFloat = test.GetTimeError();
  EXPECT_FLOAT_EQ(-1., retValFloat);

  test.SetTime(-2.);

  EXPECT_EQ(kHIT, test.GetType());
  retValFloat = test.GetZ();
  EXPECT_FLOAT_EQ(-2., retValFloat);
  retValFloat = test.GetDz();
  EXPECT_FLOAT_EQ(-2., retValFloat);
  retValInt = test.GetRefId();
  EXPECT_EQ(-2, retValInt);
  retValInt = test.GetAddress();
  EXPECT_EQ(-2, retValInt);
  EXPECT_EQ(nullptr, test.GetMatch());
  retValFloat =  test.GetTime();
  EXPECT_FLOAT_EQ(-2., retValFloat );
  retValFloat = test.GetTimeError();
  EXPECT_FLOAT_EQ(-1., retValFloat);

  test.SetTimeError(-2.);

  EXPECT_EQ(kHIT, test.GetType());
  retValFloat = test.GetZ();
  EXPECT_FLOAT_EQ(-2., retValFloat);
  retValFloat = test.GetDz();
  EXPECT_FLOAT_EQ(-2., retValFloat);
  retValInt = test.GetRefId();
  EXPECT_EQ(-2, retValInt);
  retValInt = test.GetAddress();
  EXPECT_EQ(-2, retValInt);
  EXPECT_EQ(nullptr, test.GetMatch());
  retValFloat =  test.GetTime();
  EXPECT_FLOAT_EQ(-2., retValFloat );
  retValFloat = test.GetTimeError();
  EXPECT_FLOAT_EQ(-2., retValFloat);

  test.SetTime(-3., -3.);

  EXPECT_EQ(kHIT, test.GetType());
  retValFloat = test.GetZ();
  EXPECT_FLOAT_EQ(-2., retValFloat);
  retValFloat = test.GetDz();
  EXPECT_FLOAT_EQ(-2., retValFloat);
  retValInt = test.GetRefId();
  EXPECT_EQ(-2, retValInt);
  retValInt = test.GetAddress();
  EXPECT_EQ(-2, retValInt);
  EXPECT_EQ(nullptr, test.GetMatch());
  retValFloat =  test.GetTime();
  EXPECT_FLOAT_EQ(-3., retValFloat );
  retValFloat = test.GetTimeError();
  EXPECT_FLOAT_EQ(-3., retValFloat);

  retValInt = test.GetPlaneId();
  EXPECT_EQ(-1, retValInt);

  //TODO: Test setter for fMatch
}

TEST(_GTestCbmHit , ToString)
{
  CbmHit test{kHIT, 0., 0., -1, -1};
  EXPECT_STREQ("Has to be implemented in derrived class", test.ToString().c_str());
}
