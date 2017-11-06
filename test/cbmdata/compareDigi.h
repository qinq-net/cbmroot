#ifndef COMPAREDIGI_H
#define COMPAREDIGI_H 1

#include "CbmMatch.h"

#include "compareMatch.h"

void compareDigiDataMembers(CbmTestDigi& test, Int_t address, Double_t charge, CbmMatch* match,
    Int_t systemid, Double_t time)
{
  Int_t retValInt{-222};
  Double_t retValDouble{-222.};

  retValInt = test.GetAddress();
  EXPECT_EQ(address, retValInt);

  retValDouble = test.GetCharge();
  EXPECT_FLOAT_EQ(charge, retValDouble);

  if (match !=nullptr) {
    compareMatchDataMembers(*test.GetMatch(), match->GetNofLinks(), match->GetTotalWeight());
  } else {
    EXPECT_EQ(match, test.GetMatch());
  }

  retValInt = test.GetSystemId();
  EXPECT_EQ(systemid, retValInt);

  retValDouble = test.GetTime();
  EXPECT_FLOAT_EQ(time, retValDouble);
}

#endif // COMPAREDIGI_H