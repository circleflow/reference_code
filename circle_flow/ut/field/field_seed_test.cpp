#include "test_helper.h"
#include "field_seed.h"
#include "field_concrete.h"

class FIELD_SEED_test : public ::testing::Test {
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

//manual stub
#include "utility/export/random_u.h"
static UINT8 _rand_u8=0xff;

RAND::operator UINT8(void)
{
    return _rand_u8--;
}

//test case
TEST_F(FIELD_SEED_test, increament_byte)
{
    BYTES step(1,0x11);
    BYTES base(4,0x11);

    FIELD::STEP incr(true, step, base, 8*4, 20);

    //increase
    unsigned char result[]={0x11,0x11,0x11,0x11};
    EXPECT_EQ(BYTES(result, result+4), incr.next());

    for(int i=13; i>0; i--) {
        result[3] += 0x11;
        EXPECT_EQ(BYTES(result, result+4), incr.next());
    }

    result[3] = 0xff;
    EXPECT_EQ(BYTES(result, result+4), incr.next());

    //carry bit
    result[2] = 0x12;
    result[3] = 0x10;
    EXPECT_EQ(BYTES(result, result+4), incr.next());

    for(int i=4; i>0; i--) {
        result[3] += 0x11;
        EXPECT_EQ(BYTES(result, result+4), incr.next());
    }

    //revert to base
    result[2] = 0x11;
    result[3] = 0x11;
    EXPECT_EQ(BYTES(result, result+4), incr.next());

    //reset
    result[3] = 0x22;
    EXPECT_EQ(BYTES(result, result+4), incr.next());

    incr.reset();
    result[3] = 0x11;
    EXPECT_EQ(BYTES(result, result+4), incr.next());

    //get_count
    EXPECT_EQ(20, incr.get_count());

    //get size of bit
    EXPECT_EQ(8*4, incr.get_size_of_bit());

    //clone
    FIELD::SEED * incr_2 = incr.clone();

    EXPECT_EQ(incr_2->get_count(), incr.get_count());
    EXPECT_EQ(incr_2->get_size_of_bit(), incr.get_size_of_bit());

    incr_2->reset();
    incr.reset();

    EXPECT_EQ(incr_2->next(), incr.next());
}

TEST_F(FIELD_SEED_test, increament_bit)
{
    BYTES step(1,0x1);
    BYTES base(1,0x0);

    FIELD::STEP incr(true, step, base, 3, 9);

    //increase
    BYTES result(1, 0x00);
    EXPECT_EQ(result, incr.next());

    for(int i=6; i>0; i--) {
        result[0] += 0x01;
        EXPECT_EQ(result, incr.next());
    }

    result[0] = 0x7;
    EXPECT_EQ(result, incr.next());

    //size fit
    result[0] = 0x00;
    EXPECT_EQ(result, incr.next());

    //revert to base
    result[0] = 0x00;
    EXPECT_EQ(result, incr.next());

    //reset
    result[0] = 0x01;
    EXPECT_EQ(result, incr.next());

    incr.reset();
    result[0] = 0x00;
    EXPECT_EQ(result, incr.next());

    //get_count
    EXPECT_EQ(9, incr.get_count());

    //get size of bit
    EXPECT_EQ(3, incr.get_size_of_bit());

    //clone
    FIELD::SEED * incr_2 = incr.clone();

    EXPECT_EQ(incr_2->get_count(), incr.get_count());
    EXPECT_EQ(incr_2->get_size_of_bit(), incr.get_size_of_bit());

    incr_2->reset();
    incr.reset();

    EXPECT_EQ(incr_2->next(), incr.next());
}

TEST_F(FIELD_SEED_test, decreament_byte)
{
    BYTES step(1,0x11);
    BYTES base(4,0xff);

    FIELD::STEP decr(false, step, base, 8*4, 20);

    //decrease
    unsigned char result[]={0xff,0xff,0xff,0xff};
    EXPECT_EQ(BYTES(result, result+4), decr.next());

    for(int i=14; i>0; i--) {
        result[3] -= 0x11;
        EXPECT_EQ(BYTES(result, result+4), decr.next());
    }

    result[3] = 0x00;
    EXPECT_EQ(BYTES(result, result+4), decr.next());

    //carry bit
    result[2] = 0xfe;
    result[3] = 0xef;
    EXPECT_EQ(BYTES(result, result+4), decr.next());

    //count max
    for(int i=3; i>0; i--) {
        result[3] -= 0x11;
        EXPECT_EQ(BYTES(result, result+4), decr.next());
    }

    //revert to base
    result[2] = 0xff;
    result[3] = 0xff;
    EXPECT_EQ(BYTES(result, result+4), decr.next());

    result[3] = 0xee;
    EXPECT_EQ(BYTES(result, result+4), decr.next());

    //reset
    decr.reset();
    result[3] = 0xff;
    EXPECT_EQ(BYTES(result, result+4), decr.next());

    //get_count
    EXPECT_EQ(20, decr.get_count());

    //get size of bit
    EXPECT_EQ(8*4, decr.get_size_of_bit());

    //clone
    FIELD::SEED * decr_2 = decr.clone();

    EXPECT_EQ(decr_2->get_count(), decr.get_count());
    EXPECT_EQ(decr_2->get_size_of_bit(), decr.get_size_of_bit());

    decr_2->reset();
    decr.reset();

    EXPECT_EQ(decr_2->next(), decr.next());

}


TEST_F(FIELD_SEED_test, decreament_bit)
{
    BYTES step(1,0x1);
    BYTES base(1,0x7);

    FIELD::STEP decr(false, step, base, 3, 9);

    //decrease
    BYTES result(1, 0x07);
    EXPECT_EQ(result, decr.next());

    for(int i=6; i>0; i--) {
        result[0] -= 0x01;
        EXPECT_EQ(result, decr.next());
    }

    result[0] = 0x0;
    EXPECT_EQ(result, decr.next());

    //size fit, carry bit
    result[0] = 0x07;
    EXPECT_EQ(result, decr.next());

    //revert to base
    result[0] = 0x07;
    EXPECT_EQ(result, decr.next());

    //reset
    result[0] = 0x06;
    EXPECT_EQ(result, decr.next());

    decr.reset();
    result[0] = 0x07;
    EXPECT_EQ(result, decr.next());

    //get_count
    EXPECT_EQ(9, decr.get_count());

    //get size of bit
    EXPECT_EQ(3, decr.get_size_of_bit());

    //clone
    FIELD::SEED * decr_2 = decr.clone();

    EXPECT_EQ(decr_2->get_count(), decr.get_count());
    EXPECT_EQ(decr_2->get_size_of_bit(), decr.get_size_of_bit());

    decr_2->reset();
    decr.reset();

    EXPECT_EQ(decr_2->next(), decr.next());
}

TEST_F(FIELD_SEED_test, random_byte)
{
    FIELD::RANDOM rand(8*4, 20);

    //random
    unsigned char result[]={0xff,0xfe,0xfd,0xfc};
    EXPECT_EQ(BYTES(result, result+4), rand.next());

    for(int i=4; i>0; i--) {
        for(int j=0; j<4; j++) {
            result[j] -= 4;
        }
        EXPECT_EQ(BYTES(result, result+4), rand.next());
    }

    //get_count
    EXPECT_EQ(20, rand.get_count());

    //get size of bit
    EXPECT_EQ(8*4, rand.get_size_of_bit());

    //clone
    FIELD::SEED * rand_2 = rand.clone();

    EXPECT_EQ(rand_2->get_count(), rand.get_count());
    EXPECT_EQ(rand_2->get_size_of_bit(), rand.get_size_of_bit());

    for(int j=0; j<4; j++) {
        result[j] -= 4;
    }
    EXPECT_EQ(BYTES(result, result+4), rand_2->next());

}

TEST_F(FIELD_SEED_test, random_bit)
{
    _rand_u8=0xff;

    FIELD::RANDOM rand(3, 20);

    EXPECT_EQ(BYTES(1, 0x7), rand.next());
    EXPECT_EQ(BYTES(1, 0x6), rand.next());
}

