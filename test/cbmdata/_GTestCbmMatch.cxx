#include "CbmMatch.h"
#include "CbmLink.h"

#include "gtest/gtest.h"
#include "gtest/gtest-spi.h"

TEST(_GTestCbmMatch, CheckDefaultConstructor)
{

  CbmMatch test;

  std::vector<CbmLink> link = test.GetLinks();
  Int_t linkLength = link.size();
  EXPECT_EQ(0, linkLength);

  Double_t totalWeight = test.GetTotalWeight();
  EXPECT_FLOAT_EQ(0., totalWeight);
}

TEST(_GTestCbmMatch, AddLink1)
{

  CbmMatch test;

  Int_t linkLength{-11};
  Double_t totalWeight{-11.};

  linkLength = test.GetNofLinks();
  EXPECT_EQ(0, linkLength);

  totalWeight = test.GetTotalWeight();
  EXPECT_FLOAT_EQ(0., totalWeight);

  test.AddLink(-2., -2);

  linkLength = test.GetNofLinks();
  EXPECT_EQ(1, linkLength);

  totalWeight = test.GetTotalWeight();
  EXPECT_FLOAT_EQ(-2., totalWeight);

  test.AddLink(-2., -2);

  linkLength = test.GetNofLinks();
  EXPECT_EQ(1, linkLength);

  totalWeight = test.GetTotalWeight();
  EXPECT_FLOAT_EQ(-4., totalWeight);

  test.AddLink(-8., -3);


  linkLength = test.GetNofLinks();
  EXPECT_EQ(2, linkLength);

  totalWeight = test.GetTotalWeight();
  EXPECT_FLOAT_EQ(-12., totalWeight);

}

TEST(_GTestCbmMatch, AddLink2)
{

  CbmMatch test;

  Int_t linkLength{-11};
  Double_t totalWeight{-11.};

  linkLength = test.GetNofLinks();
  EXPECT_EQ(0, linkLength);

  totalWeight = test.GetTotalWeight();
  EXPECT_FLOAT_EQ(0., totalWeight);

  test.AddLink(-2., -2, -2, -2);

  linkLength = test.GetNofLinks();
  EXPECT_EQ(1, linkLength);

  totalWeight = test.GetTotalWeight();
  EXPECT_FLOAT_EQ(-2., totalWeight);

  test.AddLink(-2., -2, -2, -2);

  linkLength = test.GetNofLinks();
  EXPECT_EQ(1, linkLength);

  totalWeight = test.GetTotalWeight();
  EXPECT_FLOAT_EQ(-4., totalWeight);

  test.AddLink(-8., -3, -2, -2);

  linkLength = test.GetNofLinks();
  EXPECT_EQ(2, linkLength);

  totalWeight = test.GetTotalWeight();
  EXPECT_FLOAT_EQ(-12., totalWeight);

  test.AddLink(8., -3, -3, -2);

  linkLength = test.GetNofLinks();
  EXPECT_EQ(3, linkLength);

  totalWeight = test.GetTotalWeight();
  EXPECT_FLOAT_EQ(-4., totalWeight);

  test.AddLink(4., -3, -3, -3);

  linkLength = test.GetNofLinks();
  EXPECT_EQ(4, linkLength);

  totalWeight = test.GetTotalWeight();
  EXPECT_FLOAT_EQ(0., totalWeight);

}

CbmMatch TestAddLinks3()
{
  CbmMatch test;

  CbmLink testLink1{};
  CbmLink testLink2{};
  CbmLink testLink3{-8., -3, -2, -2};
  CbmLink testLink4{8., -3, -3, -2};
  CbmLink testLink5{2., -3, -3, -3};

  Int_t linkLength{-11};
  Double_t totalWeight{-11.};

  linkLength = test.GetNofLinks();
  EXPECT_EQ(0, linkLength);

  totalWeight = test.GetTotalWeight();
  EXPECT_FLOAT_EQ(0., totalWeight);

  test.AddLink(testLink1);

  linkLength = test.GetNofLinks();
  EXPECT_EQ(1, linkLength);

  totalWeight = test.GetTotalWeight();
  EXPECT_FLOAT_EQ(-1., totalWeight);

  test.AddLink(testLink2);

  linkLength = test.GetNofLinks();
  EXPECT_EQ(1, linkLength);

  totalWeight = test.GetTotalWeight();
  EXPECT_FLOAT_EQ(-2., totalWeight);

  test.AddLink(testLink3);

  linkLength = test.GetNofLinks();
  EXPECT_EQ(2, linkLength);

  totalWeight = test.GetTotalWeight();
  EXPECT_FLOAT_EQ(-10., totalWeight);

  test.AddLink(testLink4);

  linkLength = test.GetNofLinks();
  EXPECT_EQ(3, linkLength);

  totalWeight = test.GetTotalWeight();
  EXPECT_FLOAT_EQ(-2., totalWeight);

  test.AddLink(testLink5);

  linkLength = test.GetNofLinks();
  EXPECT_EQ(4, linkLength);

  totalWeight = test.GetTotalWeight();
  EXPECT_FLOAT_EQ(0., totalWeight);

  return test;
}

TEST(_GTestCbmMatch, AddLink3)
{

  Int_t linkLength{-11};
  Double_t totalWeight{-11.};

  CbmMatch test = TestAddLinks3();
  CbmMatch test2;

  test2.AddLinks(test);

  linkLength = test2.GetNofLinks();
  EXPECT_EQ(4, linkLength);

  totalWeight = test2.GetTotalWeight();
  EXPECT_FLOAT_EQ(0., totalWeight);
}

TEST(_GTestCbmMatch, ClearLink)
{

  Int_t linkLength{-11};
  Double_t totalWeight{-11.};

  CbmMatch test = TestAddLinks3();

  linkLength = test.GetNofLinks();
  EXPECT_EQ(4, linkLength);

  totalWeight = test.GetTotalWeight();
  EXPECT_FLOAT_EQ(0., totalWeight);

  test.ClearLinks();

  linkLength = test.GetNofLinks();
  EXPECT_EQ(0, linkLength);

  totalWeight = test.GetTotalWeight();
  EXPECT_FLOAT_EQ(0., totalWeight);
}

TEST(_GTestCbmMatch, GetLink)
{

  Int_t linkLength{-11};
  Double_t totalWeight{-11.};
  Int_t intRetVal{-111};
  Float_t floatRetVal{-111.};

  CbmMatch test = TestAddLinks3();

  linkLength = test.GetNofLinks();
  EXPECT_EQ(4, linkLength);

  totalWeight = test.GetTotalWeight();
  EXPECT_FLOAT_EQ(0., totalWeight);

  CbmLink testLink{};
  testLink = test.GetLink(0);

  intRetVal = testLink.GetEntry();
  EXPECT_EQ(-1, intRetVal);
  intRetVal = testLink.GetFile();
  EXPECT_EQ(-1, intRetVal);
  intRetVal = testLink.GetIndex();
  EXPECT_EQ(-1, intRetVal);

  floatRetVal = testLink.GetWeight();
  EXPECT_FLOAT_EQ(-2., floatRetVal);

  testLink = test.GetLink(1);

  intRetVal = testLink.GetEntry();
  EXPECT_EQ(-2, intRetVal);
  intRetVal = testLink.GetFile();
  EXPECT_EQ(-2, intRetVal);
  intRetVal = testLink.GetIndex();
  EXPECT_EQ(-3, intRetVal);

  floatRetVal = testLink.GetWeight();
  EXPECT_FLOAT_EQ(-8., floatRetVal);

  testLink = test.GetLink(2);

  intRetVal = testLink.GetEntry();
  EXPECT_EQ(-3, intRetVal);
  intRetVal = testLink.GetFile();
  EXPECT_EQ(-2, intRetVal);
  intRetVal = testLink.GetIndex();
  EXPECT_EQ(-3, intRetVal);

  floatRetVal = testLink.GetWeight();
  EXPECT_FLOAT_EQ(8., floatRetVal);

  testLink = test.GetLink(3);

  intRetVal = testLink.GetEntry();
  EXPECT_EQ(-3, intRetVal);
  intRetVal = testLink.GetFile();
  EXPECT_EQ(-3, intRetVal);
  intRetVal = testLink.GetIndex();
  EXPECT_EQ(-3, intRetVal);

  floatRetVal = testLink.GetWeight();
  EXPECT_FLOAT_EQ(2., floatRetVal);
}

//TODO: Create correct test. Have to understand what the function does.
/*
TEST(_GTestCbmMatch, GetMatchedLink)
{

  Int_t linkLength{-11};
  Double_t totalWeight{-11.};
  Int_t intRetVal{-111};
  Float_t floatRetVal{-111.};

  CbmMatch test = TestAddLinks3();

  CbmLink testLink = test.GetMatchedLink();

  intRetVal = testLink.GetEntry();
  EXPECT_EQ(-1, intRetVal);
  intRetVal = testLink.GetFile();
  EXPECT_EQ(-1, intRetVal);
  intRetVal = testLink.GetIndex();
  EXPECT_EQ(-1, intRetVal);

  floatRetVal = testLink.GetWeight();
  EXPECT_FLOAT_EQ(-2., floatRetVal);
}
*/

TEST(_GTestCbmMatch, CheckToString)
{

  CbmMatch test;

  EXPECT_STREQ("CbmMatch: nofLinks=0\n totalWeight=0, matchedIndex=-1\n", test.ToString().c_str());

  CbmLink testLink{-8., -3, -2, -2};
  test.AddLink(testLink);

  EXPECT_STREQ("CbmMatch: nofLinks=1\nCbmLink: weight=-8 index=-3 entry=-2 file=-2\n totalWeight=-8, matchedIndex=0\n", test.ToString().c_str());
}
