
#include "test_helper.h"

#include "random_u.h"
using namespace UTILITY;

#include <set>
using std::set;

class RANDOM_test : public ::testing::Test {
 protected:

    bool check_mock;

    virtual void SetUp()
    {
        check_mock = false;
        opmock_test_reset();
    }

    virtual void TearDown()
    {
        if(check_mock) {
            EXPECT_VERIFY_MOCK();
        }
    }
};

TEST_F(RANDOM_test, basic)
{
    set<UINT8> db;

    RAND rand;
    for(UINT8 i=0; i<100; i++) {
        db.insert((UINT8)rand);
    }

    EXPECT_TRUE(db.size()>1);
    EXPECT_TRUE(db.size()>10);
    EXPECT_TRUE(db.size()>50);
    //EXPECT_TRUE(db.size()>80);

}
