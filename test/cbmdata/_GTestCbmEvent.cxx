#include "CbmEvent.h"

#include "CbmMatch.h"
#include "CbmVertex.h"
#include "CbmDetectorList.h"

#include "gtest/gtest.h"
#include "gtest/gtest-spi.h"

#include <vector>

 void compareEventDataMembers(CbmEvent& test, Int_t evnumber, Double_t starttime,
 Double_t endtime, Int_t numobjects, CbmMatch* match /*, CbmVertex& vertex*/)
{
  Int_t retValInt{-111};
  Double_t retValDouble{-111.};

  retValInt = test.GetNumber();
  EXPECT_EQ(evnumber, retValInt);

  retValDouble = test.GetStartTime();
  EXPECT_EQ(starttime, retValDouble);

  retValDouble = test.GetEndTime();
  EXPECT_EQ(endtime, retValDouble);

  retValInt = test.GetNofData();
  EXPECT_EQ(numobjects, retValInt);

  EXPECT_EQ(match, test.GetMatch());
}

void compareEventMap(CbmEvent& test, Int_t numobjects, Int_t numobjectstype, Cbm::DataType type, std::vector<UInt_t> indices)
{
  Int_t retValInt{-111};
  UInt_t retValUInt{111};

  retValInt = test.GetNofData();
  EXPECT_EQ(numobjects, retValInt);

  retValInt = test.GetNofData(type);
  EXPECT_EQ(numobjectstype, retValInt);

  if(numobjectstype > 0){
    for(Int_t i=0; i< numobjectstype; ++i){
      retValUInt=test.GetIndex(type, i);
      EXPECT_EQ(indices[i], retValUInt);
    }
  }

}

TEST(_GTestCbmCluster, CheckDefaultConstructor)
{
  CbmEvent test;
  {
    SCOPED_TRACE("CheckDefaultConstructor");
    compareEventDataMembers(test, -1, 0., 0., 0, nullptr);
  }
}

TEST(_GTestCbmCluster, CheckStandardConstructor)
{
  CbmEvent test{-111};
  {
    SCOPED_TRACE("CheckStandardConstructor");
    compareEventDataMembers(test, -111, 0., 0., 0, nullptr);
  }
}


TEST(_GTestCbmCluster, CheckSettersAndGetters)
{
  CbmEvent test{-111, 1., 2.};
  {
    SCOPED_TRACE("CheckSettersAndGetters: Initial Test");
    compareEventDataMembers(test, -111, 1., 2., 0, nullptr);
  }

  test.SetStartTime(-23.);
  {
    SCOPED_TRACE("CheckSettersAndGetters: SetStartTime");
    compareEventDataMembers(test, -111, -23., 2., 0, nullptr);
  }

  test.SetEndTime(-45.);
  {
    SCOPED_TRACE("CheckSettersAndGetters: SetEndTime");
    compareEventDataMembers(test, -111, -23., -45., 0, nullptr);
  }

  CbmMatch* testMatch = new CbmMatch();
  test.SetMatch(testMatch);
  {
    SCOPED_TRACE("CheckSettersAndGetters: SetMatch");
    compareEventDataMembers(test, -111, -23., -45., 0, testMatch);
  }
}

 TEST(_GTestCbmCluster, CheckAddData)
 {
   CbmEvent test{-111, 1., 2.};
   {
     SCOPED_TRACE("CheckAddData: Initial Test");
     compareEventDataMembers(test, -111, 1., 2., 0, nullptr);
   }

   std::vector<UInt_t> mctrack;
   std::vector<UInt_t> stspoint;
   std::vector<UInt_t> stsdigi;
   std::vector<UInt_t> stscluster;
   std::vector<UInt_t> stshit;
   std::vector<UInt_t> ststrack;


   mctrack.push_back(11);
   test.AddData(Cbm::kMCTrack, 11);
   {
     SCOPED_TRACE("CheckAddData: Add first MCTrack");
     compareEventDataMembers(test, -111, 1., 2., 1, nullptr);
     compareEventMap(test, 1,  1, Cbm::kMCTrack,    mctrack);
     compareEventMap(test, 1, -1, Cbm::kStsPoint,   stspoint);
     compareEventMap(test, 1, -1, Cbm::kStsDigi,    stsdigi);
     compareEventMap(test, 1, -1, Cbm::kStsCluster, stscluster);
     compareEventMap(test, 1, -1, Cbm::kStsHit,     stshit);
     compareEventMap(test, 1, -1, Cbm::kStsTrack,   ststrack);
   }

   mctrack.push_back(23);
   test.AddData(Cbm::kMCTrack, 23);
   {
     SCOPED_TRACE("CheckAddData: Add second MCTrack");
     compareEventDataMembers(test, -111, 1., 2., 2, nullptr);
     compareEventMap(test, 2,  2, Cbm::kMCTrack,    mctrack);
     compareEventMap(test, 2, -1, Cbm::kStsPoint,   stspoint);
     compareEventMap(test, 2, -1, Cbm::kStsDigi,    stsdigi);
     compareEventMap(test, 2, -1, Cbm::kStsCluster, stscluster);
     compareEventMap(test, 2, -1, Cbm::kStsHit,     stshit);
     compareEventMap(test, 2, -1, Cbm::kStsTrack,   ststrack);
   }

   UInt_t retValUInt=test.GetIndex(Cbm::kMCTrack, 25);
   EXPECT_EQ(-2, retValUInt);

   stspoint.push_back(1);
   test.AddData(Cbm::kStsPoint, 1);
   {
     SCOPED_TRACE("CheckAddData: Add StsPoint");
     compareEventDataMembers(test, -111, 1., 2., 3, nullptr);
     compareEventMap(test, 3,  2, Cbm::kMCTrack,    mctrack);
     compareEventMap(test, 3,  1, Cbm::kStsPoint,   stspoint);
     compareEventMap(test, 3, -1, Cbm::kStsDigi,    stsdigi);
     compareEventMap(test, 3, -1, Cbm::kStsCluster, stscluster);
     compareEventMap(test, 3, -1, Cbm::kStsHit,     stshit);
     compareEventMap(test, 3, -1, Cbm::kStsTrack,   ststrack);
   }

   stsdigi.push_back(2);
   test.AddData(Cbm::kStsDigi, 2);
   {
     SCOPED_TRACE("CheckAddData: Add StsDigi");
     compareEventDataMembers(test, -111, 1., 2., 4, nullptr);
     compareEventMap(test, 4,  2, Cbm::kMCTrack,    mctrack);
     compareEventMap(test, 4,  1, Cbm::kStsPoint,   stspoint);
     compareEventMap(test, 4,  1, Cbm::kStsDigi,    stsdigi);
     compareEventMap(test, 4, -1, Cbm::kStsCluster, stscluster);
     compareEventMap(test, 4, -1, Cbm::kStsHit,     stshit);
     compareEventMap(test, 4, -1, Cbm::kStsTrack,   ststrack);
   }

   stscluster.push_back(3);
   test.AddData(Cbm::kStsCluster, 3);
   {
     SCOPED_TRACE("CheckAddData: Add StsCluster");
     compareEventDataMembers(test, -111, 1., 2., 5, nullptr);
     compareEventMap(test, 5,  2, Cbm::kMCTrack,    mctrack);
     compareEventMap(test, 5,  1, Cbm::kStsPoint,   stspoint);
     compareEventMap(test, 5,  1, Cbm::kStsDigi,    stsdigi);
     compareEventMap(test, 5,  1, Cbm::kStsCluster, stscluster);
     compareEventMap(test, 5, -1, Cbm::kStsHit,     stshit);
     compareEventMap(test, 5, -1, Cbm::kStsTrack,   ststrack);
   }

   stshit.push_back(4);
   test.AddData(Cbm::kStsHit, 4);
   {
     SCOPED_TRACE("CheckAddData: Add StsHit");
     compareEventDataMembers(test, -111, 1., 2., 6, nullptr);
     compareEventMap(test, 6,  2, Cbm::kMCTrack,    mctrack);
     compareEventMap(test, 6,  1, Cbm::kStsPoint,   stspoint);
     compareEventMap(test, 6,  1, Cbm::kStsDigi,    stsdigi);
     compareEventMap(test, 6,  1, Cbm::kStsCluster, stscluster);
     compareEventMap(test, 6,  1, Cbm::kStsHit,     stshit);
     compareEventMap(test, 6, -1, Cbm::kStsTrack,   ststrack);
   }

   ststrack.push_back(5);
   test.AddData(Cbm::kStsTrack, 5);
   {
     SCOPED_TRACE("CheckAddData: Add StsTrack");
     compareEventDataMembers(test, -111, 1., 2., 7, nullptr);
     compareEventMap(test, 7,  2, Cbm::kMCTrack,    mctrack);
     compareEventMap(test, 7,  1, Cbm::kStsPoint,   stspoint);
     compareEventMap(test, 7,  1, Cbm::kStsDigi,    stsdigi);
     compareEventMap(test, 7,  1, Cbm::kStsCluster, stscluster);
     compareEventMap(test, 7,  1, Cbm::kStsHit,     stshit);
     compareEventMap(test, 7,  1, Cbm::kStsTrack,   ststrack);
   }

   ststrack.push_back(6);
   test.AddStsTrack(6);
   {
     SCOPED_TRACE("CheckAddData: Add StsTrack via AddStsTrack");
     compareEventDataMembers(test, -111, 1., 2., 8, nullptr);
     compareEventMap(test, 8,  2, Cbm::kMCTrack,    mctrack);
     compareEventMap(test, 8,  1, Cbm::kStsPoint,   stspoint);
     compareEventMap(test, 8,  1, Cbm::kStsDigi,    stsdigi);
     compareEventMap(test, 8,  1, Cbm::kStsCluster, stscluster);
     compareEventMap(test, 8,  1, Cbm::kStsHit,     stshit);
     compareEventMap(test, 8,  2, Cbm::kStsTrack,   ststrack);
   }

   retValUInt=test.GetStsTrackIndex(0);
   EXPECT_EQ(5, retValUInt);

   retValUInt=test.GetStsTrackIndex(1);
   EXPECT_EQ(6, retValUInt);

   retValUInt=test.GetStsTrackIndex(2);
   EXPECT_EQ(-2, retValUInt);

   retValUInt=test.GetNofStsTracks();
   EXPECT_EQ(2, retValUInt);

   ststrack.clear();
   ststrack.push_back(11);
   ststrack.push_back(12);
   ststrack.push_back(13);

   test.SetStsTracks(ststrack);
   retValUInt=test.GetNofStsTracks();
   EXPECT_EQ(3, retValUInt);
   {
     SCOPED_TRACE("CheckAddData: Add StsTracks via StsStsTrack");
     compareEventDataMembers(test, -111, 1., 2., 9, nullptr);
     compareEventMap(test, 9,  2, Cbm::kMCTrack,    mctrack);
     compareEventMap(test, 9,  1, Cbm::kStsPoint,   stspoint);
     compareEventMap(test, 9,  1, Cbm::kStsDigi,    stsdigi);
     compareEventMap(test, 9,  1, Cbm::kStsCluster, stscluster);
     compareEventMap(test, 9,  1, Cbm::kStsHit,     stshit);
     compareEventMap(test, 9,  3, Cbm::kStsTrack,   ststrack);
   }

   EXPECT_STREQ("Event -111 at t = 1 ns. Registered data types: 6, data objects: 9\n          -- Data type 1, number of data 2\n          -- Data type 2, number of data 1\n          -- Data type 3, number of data 1\n          -- Data type 4, number of data 1\n          -- Data type 5, number of data 1\n          -- Data type 6, number of data 3\n", test.ToString().c_str());

 }

TEST(_GTestCbmCluster, CheckToString)
{

}
