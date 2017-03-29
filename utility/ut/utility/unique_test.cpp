
#include "smart_ptr_u.h"

#include "test_helper.h"

#include "error.h"
#include "unique.h"
using namespace UTILITY;

#include <vector>
using std::vector;

class UNIQUE_test : public ::testing::Test {
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

#define MIN_ID_BASIC 0
#define MAX_ID_BASIC 127
#define EXPAND_GRAN 64

enum ENUM_TEST_BASIC {};
typedef UNIQUE<ENUM_TEST_BASIC, UINT8, MIN_ID_BASIC, MAX_ID_BASIC> UNIQUE_BASIC;

TEST_F(UNIQUE_test, basic)
{

    //late free not allocate immediately
    {
        UINT16 id;
        {
            UNIQUE_BASIC u;
            id = u.get_id();
        }
        {
            UNIQUE_BASIC u;
            EXPECT_NE(id, u.get_id());
        }
    }

    //expansion
    {
        vector<shared_ptr<UNIQUE_BASIC> > db;

        //granularity
        for(UINT8 i=0; i<EXPAND_GRAN; i++) {

            shared_ptr<UNIQUE_BASIC> ptr(new UNIQUE_BASIC());

            //printf("\r\n i=%d, id=%d", i, ptr->get_id());
            EXPECT_TRUE(ptr->get_id()<EXPAND_GRAN);

            db.push_back(ptr);
        }

        EXPECT_EQ(EXPAND_GRAN, db.size());

        shared_ptr<UNIQUE_BASIC> ptr(new UNIQUE_BASIC());
        EXPECT_TRUE(ptr->get_id()>=EXPAND_GRAN);

        //max value, boarder check
        for(UINT8 i=2; i<EXPAND_GRAN; i++) {

            shared_ptr<UNIQUE_BASIC> ptr(new UNIQUE_BASIC());

            //printf("\r\n i=%d, id=%d", i, ptr->get_id());
            EXPECT_TRUE(ptr->get_id()<MAX_ID_BASIC);

            db.push_back(ptr);
        }

        shared_ptr<UNIQUE_BASIC> ptr_2(new UNIQUE_BASIC());
        EXPECT_TRUE(ptr_2->get_id()==MAX_ID_BASIC);

        //printf("\r\n id=%d", ptr_2->get_id());

        EXPECT_THROW(UNIQUE_BASIC(), EXP_ERROR);
    }
}

enum ENUM_TEST_ONE {};
typedef UNIQUE<ENUM_TEST_ONE, UINT8, 1, 1> UNIQUE_ONE;

enum ENUM_TEST_MAX {};
typedef UNIQUE<ENUM_TEST_MAX, UINT8, 0, 255> UNIQUE_MAX;

TEST_F(UNIQUE_test, special)
{
    //max_id==min_id
    UNIQUE_ONE one;
    EXPECT_EQ(1, one.get_id());

    EXPECT_THROW(UNIQUE_ONE(), EXP_ERROR);

    //id value reverse
    vector<shared_ptr<UNIQUE_MAX> > db;
    for(UINT8 i=0; i<255; i++) {

        shared_ptr<UNIQUE_MAX> ptr(new UNIQUE_MAX());

        //printf("\r\n i=%d, id=%d", i, ptr->get_id());
        db.push_back(ptr);
    }

    UNIQUE_MAX max;
    EXPECT_THROW(UNIQUE_MAX(), EXP_ERROR);
}

TEST_F(UNIQUE_test, pool)
{
    {
        UNIQUE_ONE one(0);
        EXPECT_THROW(UNIQUE_ONE(0), EXP_ERROR);

        UNIQUE_ONE one_1(1);
        EXPECT_THROW(UNIQUE_ONE(1), EXP_ERROR);

        UNIQUE_ONE one_2(2);
        EXPECT_THROW(UNIQUE_ONE(2), EXP_ERROR);
    }

    {
        UNIQUE_ONE one(0);
        UNIQUE_ONE one_1(1);
        UNIQUE_ONE one_2(2);

        EXPECT_THROW(UNIQUE_ONE(0), EXP_ERROR);
        EXPECT_THROW(UNIQUE_ONE(1), EXP_ERROR);
        EXPECT_THROW(UNIQUE_ONE(2), EXP_ERROR);
    }
}
