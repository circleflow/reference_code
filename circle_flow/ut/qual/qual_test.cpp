
#include "qual.cpp"
#include "test_helper.h"

#include "field.h"
#include "field_block.h"

#include "ethernet.h"
#include "ipv4.h"

#include "utility/export/error.h"


class QUAL_test : public ::testing::Test {
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
RAND::operator UINT8(void)
{
    return 0xff;
}

TEST_F(QUAL_test, make_mask)
{
    bool data [] = {1, 1, 1, 0, 0, 0};

    EXPECT_EQ(BITS(data, data+6), make_mask(6, 3));

    EXPECT_EQ(BITS(9, 1), make_mask(9, FULL_MASK));

    EXPECT_THROW(make_mask(8, 9), EXP_ERROR);
}

TEST_F(QUAL_test, pdf_rule)
{
    // bytes
    {
        UINT8 data [] = {0x88, 0x64};

        QUAL::PDF_RULE rule;
        rule.first = QUAL::ether_type;
        QUAL::MATCH match;
        match.value = BYTES(data, data+2);
        match.mask  = BYTES(2, 0xff);
        rule.second = match;

        EXPECT_EQ(rule, make_pdf_rule(QUAL::ether_type, BYTES(data, data+2)));

        rule.second.value[1] = 0x00;
        rule.second.mask[1]  = 0x00;
        EXPECT_EQ(rule, make_pdf_rule(QUAL::ether_type, BYTES(data, data+2), 8));
    }

    //bits
    {
        UINT8 data [] = {0x01, 0x02};
        UINT8 mask [] = {0x0f, 0xff};

        QUAL::PDF_RULE rule;
        rule.first = QUAL::outer_vlan_id;
        QUAL::MATCH match;
        match.value = BYTES(data, data+2);
        match.mask  = BYTES(mask, mask+2);
        rule.second = match;

        EXPECT_EQ(rule, make_pdf_rule(QUAL::outer_vlan_id, BYTES(data, data+2)));

        rule.second.value[1] = 0x00;
        rule.second.mask[1]  = 0xf0;
        EXPECT_EQ(rule, make_pdf_rule(QUAL::outer_vlan_id, BYTES(data, data+2), 8));
    }

    //less bit
    {
        QUAL::PDF_RULE rule;
        rule.first = QUAL::outer_vlan_cfi;
        QUAL::MATCH match;
        match.value = BYTES(1, 0x1);
        match.mask  = BYTES(1, 0x1);
        rule.second = match;

        EXPECT_EQ(rule, make_pdf_rule(QUAL::outer_vlan_cfi, BYTES(1, 0x1)));
        EXPECT_EQ(rule, make_pdf_rule(QUAL::outer_vlan_cfi, BYTES(1, 0x1), 1));
    }
}

TEST_F(QUAL_test, udf_rule)
{
    check_mock = true;

    using namespace FB_ETH_II;

    //offset and size are byte aligned
    {
        FIELD_BLOCK fb=make_ETH_II();
        FIELD::INDEX index(ETH_TYPE);

        UINT8 data[]={0x33, 0x44};
        BYTES bytes(data, data+2);

        fb.field(ETH_TYPE) = bytes;

        QUAL::UDF_RULE rule;
        rule.first.offset = 12;
        rule.first.size = 2;
        rule.second.value = bytes;
        rule.second.mask  = BYTES(2, 0xff);

        EXPECT_EQ(rule, make_udf_rule(fb,index));
    }

    //size are byte unaligned
    {
        using namespace FB_IPV4;

        FIELD_BLOCK fb=make_IPV4();
        FIELD::INDEX index(FLAGS);

        BYTES bytes(1, 0x5);
        fb.field(index) = bytes;

        QUAL::UDF_RULE rule;
        rule.first.offset = 6;
        rule.first.size = 1;
        rule.second.value = BYTES(1, 0xa0);
        rule.second.mask  = BYTES(1, 0xe0);

        EXPECT_EQ(rule, make_udf_rule(fb,index));
    }

    //offset is byte unaligned
    {
        using namespace FB_IPV4;

        FIELD_BLOCK fb=make_IPV4();
        FIELD::INDEX index(IHL);

        BYTES bytes(1, 0xa);
        fb.field(index) = bytes;

        QUAL::UDF_RULE rule;
        rule.first.offset = 0;
        rule.first.size = 1;
        rule.second.value.push_back(0x0a);
        rule.second.mask.push_back(0x0f);

        EXPECT_EQ(rule, make_udf_rule(fb,index));
    }

    //offset and size are byte unaligned
    {
        using namespace FB_IPV4;

        FIELD_BLOCK fb=make_IPV4();
        FIELD::INDEX index(FRAG_OFF);

        UINT8 data[]={0x12, 0x34};
        BYTES bytes(data, data+2);
        fb.field(index) = bytes;

        QUAL::UDF_RULE rule;
        rule.first.offset = 6;
        rule.first.size = 2;
        rule.second.value.push_back(0x12);
        rule.second.value.push_back(0x34);
        rule.second.mask.push_back(0x1f);
        rule.second.mask.push_back(0xff);

        EXPECT_EQ(rule, make_udf_rule(fb,index));

        rule.second.value[1]= 0x30;
        rule.second.mask[1] = 0xf0;
        EXPECT_EQ(rule, make_udf_rule(fb,index,9));
    }
}
