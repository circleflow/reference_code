
#include "test_helper.h"

#include "bytes_bits.h"
#include "type_ext.h"
#include "error.h"

class BYTES_BITS_test : public ::testing::Test {
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

TEST_F(BYTES_BITS_test, bytes_operation)
{
    // op: or

    //zero
    {
        BYTES b1;
        BYTES b2(1, 0x55);

        EXPECT_EQ(BYTES(1, 0x55), b1|b2);
        EXPECT_EQ(BYTES(1, 0x55), b2|b1);

        b1 |= b2;
        EXPECT_EQ(BYTES(1, 0x55), b1);
    }

    //none zero
    {
        BYTES b1(1, 0x11);
        b1.push_back(0x22);

        BYTES b2(1, 0x44);
        b2.push_back(0x88);

        BYTES b3(1, 0x55);
        b3.push_back(0xaa);

        EXPECT_EQ(b3, b1|b2);
        EXPECT_EQ(b3, b2|b1);

        b1 |= b2;
        EXPECT_EQ(b3, b1);
    }

    //size not equal
    {
        BYTES b1(1, 0x11);
        b1.push_back(0x22);

        BYTES b2(1, 0x44);

        EXPECT_THROW(b1|b2, EXP_ERROR);
    }


    //op: and

    //zero
    {
        BYTES b1;
        BYTES b2(1, 0x55);

        EXPECT_EQ(BYTES(1, 0x00), b1&b2);
        EXPECT_EQ(BYTES(1, 0x00), b2&b1);

        b1 &= b2;
        EXPECT_EQ(BYTES(1, 0x00), b1);
    }

    //none zero
    {
        BYTES b1(1, 0x11);
        b1.push_back(0x44);

        BYTES b2(1, 0x33);
        b2.push_back(0x55);

        BYTES b3(1, 0x11);
        b3.push_back(0x44);

        EXPECT_EQ(b3, b1&b2);
        EXPECT_EQ(b3, b2&b1);

        b1 &= b2;
        EXPECT_EQ(b3, b1);
    }

    //size not equal
    {
        BYTES b1(1, 0x11);
        b1.push_back(0x22);

        BYTES b2(1, 0x44);

        EXPECT_THROW(b1&b2, EXP_ERROR);
    }
}

TEST_F(BYTES_BITS_test, bytes_bits_transform)
{
    //one byte
    {
        BYTES bytes(1, 0xac);
        bool data [] = {true, false, true, false, true, true, false, false};
        BITS bits(data, data+8);

        EXPECT_EQ(bytes, bits_to_bytes(bits));
        EXPECT_EQ(bits,  bytes_to_bits(bytes));
    }

    //less than one byte
    {
        BYTES bytes(1, 0x15);
        bool data [] = {true, false, true, false, true};
        BITS bits(data, data+5);

        EXPECT_EQ(bytes, bits_to_bytes(bits));

        bool data_2 [] = {false, false, false, true, false, true, false, true};
        BITS bits_2(data_2, data_2+8);
        EXPECT_EQ(bits_2,  bytes_to_bits(bytes));
    }

    //more than one byte
    {
        BYTES bytes(1, 0x02);
        bytes.push_back(0xb2);
        bool data [] = {true, false, true, false, true, true, false, false, true, false};
        BITS bits(data, data+10);

        EXPECT_EQ(bytes, bits_to_bytes(bits));

        bool data_2 [] = {false, false, false, false, false, false};
        BITS bits_2(data, data+10);
        bits_2.insert(bits_2.begin(), data_2, data_2+6);
        EXPECT_EQ(bits_2,  bytes_to_bits(bytes));
    }
}

TEST_F(BYTES_BITS_test, make_bytes)
{
    //uint8
    {
        BYTES bytes(1, 0x33);
        EXPECT_EQ(bytes, make_bytes((UINT8)0x33));
        EXPECT_EQ(0x33, (UINT8)BYTES_CONVERT(bytes));
    }

    //uint16
    {
        BYTES bytes;
        bytes.push_back(0x33);
        bytes.push_back(0x44);
        EXPECT_EQ(bytes, make_bytes((UINT16)0x3344));
        EXPECT_EQ(0x3344, (UINT16)BYTES_CONVERT(bytes));
    }

    //uint32
    {
        BYTES bytes;
        bytes.push_back(0x33);
        bytes.push_back(0x44);
        bytes.push_back(0x55);
        bytes.push_back(0x66);
        EXPECT_EQ(bytes, make_bytes((UINT32)0x33445566));
        EXPECT_EQ(0x33445566, (UINT32)BYTES_CONVERT(bytes));
    }
}

TEST_F(BYTES_BITS_test, size_fit)
{
    EXPECT_EQ(BYTES(3,0x33), pattern_fit(BYTES(1,0x33), 3));

    {
        UINT8 pattern[2] = {0x11,0x22};
        UINT8 data[4]    = {0x11,0x22,0x11,0x22};
        EXPECT_EQ(BYTES(data, data+4), pattern_fit(BYTES(pattern, pattern+2), 4));
    }

    {
        UINT8 pattern[2] = {0x11,0x22};
        UINT8 data[5]    = {0x11,0x22,0x11,0x22,0x11};
        EXPECT_EQ(BYTES(data, data+5), pattern_fit(BYTES(pattern, pattern+2), 5));
    }

}
