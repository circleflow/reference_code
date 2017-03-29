
#include "test_helper.h"

#include "tx/pkt_helper.h"

#include <algorithm>
using std::fill;

class HELPER_test : public ::testing::Test {
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


TEST_F(HELPER_test, pkt_helper)
{
    //insert tag
    {
        UINT8 data[] = {0x00,0x01,0x02,0x03,0x04,0x05,
                        0x00,0x11,0x22,0x33,0x44,0x55,
                        0x08,0x00};

        PACKET src(data, data+14);
        src.insert(src.end(), 60, 0x33);

        PACKET result(data, data+12);
        result.push_back(0x81);
        result.push_back(0x00);
        result.push_back(0x01);
        result.push_back(0x11);
        result.insert(result.end(), data+12, data+14);
        result.insert(result.end(), 60, 0x33);

        PACKET dst;
        insert_vtag(src, dst, 0x111);

        EXPECT_EQ(result, dst);

        // src as dst
        dst=src;
        insert_vtag(dst, dst, 0x111);
        EXPECT_EQ(result, dst);

        //pkts
        PACKET src_2(src);
        src_2.resize(12+2+1000);
        fill(src_2.begin()+14, src_2.end(), 0x66);
        PACKET result_2(result);
        result_2.resize(12+4+2+1000);
        fill(result_2.begin()+18, result_2.end(), 0x66);

        PACKETS pkts_src;
        pkts_src.push_back(src);
        pkts_src.push_back(src_2);

        PACKETS pkts_result;
        pkts_result.push_back(result);
        pkts_result.push_back(result_2);

        PACKETS pkts_dst;
        insert_vtag(pkts_src, pkts_dst, 0x111);
        EXPECT_EQ(pkts_result, pkts_dst);

        //src as dst
        pkts_dst=pkts_src;
        insert_vtag(pkts_dst, pkts_dst, 0x111);
        EXPECT_EQ(pkts_result, pkts_dst);

        //pkts len
        vector<UINT16> lens;
        lens.push_back(12+2+60);
        lens.push_back(12+2+1000);
        EXPECT_EQ(lens, pkts_len(pkts_src));
    }

    //extract ovid
    {
        UINT8 data[] = {0x00,0x01,0x02,0x03,0x04,0x05,
                        0x00,0x11,0x22,0x33,0x44,0x55,
                        0x81,0x00,0x02,0x34,
                        0x08,0x00};

        PACKET pkt(data, data+18);
        pkt.insert(pkt.end(), 60, 0x33);

        EXPECT_EQ(0x234, extract_ovid(pkt));
    }

    //remove otag
    {
        UINT8 data[] = {0x00,0x01,0x02,0x03,0x04,0x05,
                        0x00,0x11,0x22,0x33,0x44,0x55,
                        0x81,0x00,0x02,0x34,
                        0x08,0x00};

        PACKET pkt(data, data+18);
        pkt.insert(pkt.end(), 60, 0x33);

        PACKET result(data, data+12);
        result.insert(result.end(), data+16, data+18);
        result.insert(result.end(), 60, 0x33);

        PACKET dst;
        remove_otag(pkt, dst);
        EXPECT_EQ(result, dst);
    }

}
