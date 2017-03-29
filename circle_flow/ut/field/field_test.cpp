
#include "field/field_concrete.h"
#include "test_helper.h"

#include "type_ext.h"
#include "field/field_impl.h"

#include "utility/export/error.h"

#include "mock_cac_stub.hpp"


class FIELD_test : public ::testing::Test {
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

#include "mock_parser.h"

TEST_F(FIELD_test, fixed_size)
{
    //test on a single byte field
    BYTES bytes;
    bytes.push_back(0x55);

    FIELD_FIXED_SIZE f("test_fixed", 8, PARSER_MOCK());
    f=bytes;

    EXPECT_EQ(bytes, (BYTES)f);
    EXPECT_EQ(8, f.size_of_bit());

    //parser
    mock_parser_bytes.clear();
    mock_parser_bytes.push_back(0x66);
    mock_parser_text="some";
    f=TEXT("any");
    EXPECT_EQ(mock_parser_bytes, (BYTES)f);
    EXPECT_EQ(mock_parser_text, (TEXT)f);

    //more bytes than size defined
    bytes.push_back(0x77);
    f=bytes;

    bytes.erase(bytes.begin());
    EXPECT_EQ(bytes, (BYTES)f);

    //pimpl
    FIELD::IMPL *pimpl = FIELD::IMPL::get_pimpl(&f);
    EXPECT_TRUE(pimpl);
    EXPECT_EQ(string("test_fixed"),pimpl->get_name());
    pimpl->set_name("test_renamed");
    EXPECT_EQ(string("test_renamed"),pimpl->get_name());

    //clone
    FIELD *pf = f.clone();
    EXPECT_TRUE(pf);
    EXPECT_EQ((BYTES)f, (BYTES)(*pf));
    EXPECT_EQ(f.size_of_bit(), pf->size_of_bit());

    EXPECT_TRUE(dynamic_cast<FIELD_FIXED_SIZE*> (pf));
    EXPECT_TRUE(0==dynamic_cast<FIELD_RESIZABLE*> (pf));

    delete pf;

    //test on a multiple byte field
    bytes.clear();
    bytes.push_back(0x00);
    bytes.push_back(0x01);
    bytes.push_back(0x02);
    bytes.push_back(0x03);
    bytes.push_back(0x04);
    bytes.push_back(0x05);

    FIELD_FIXED_SIZE f2("test_fixed_2", 48);
    f2 = bytes;

    EXPECT_EQ(bytes, (BYTES)f2);
    EXPECT_EQ(48, f2.size_of_bit());

    //test on a multiple bit field
    bytes.clear();
    bytes.push_back(0x0a);

    FIELD_FIXED_SIZE f3("test_fixed_3", 4);
    f3 = bytes;

    EXPECT_EQ(bytes, (BYTES)f3);
    f3.size_of_bit();

    pimpl = FIELD::IMPL::get_pimpl(&f3);
    EXPECT_TRUE(pimpl);

    BITS bits;
    bits.push_back(1);
    bits.push_back(0);
    bits.push_back(1);
    bits.push_back(0);
    EXPECT_EQ(bits, pimpl->get_bits());

    //pattern
    {
        FIELD_FIXED_SIZE f4("test_fixed_4", 7*8);

        {
            UINT8 pattern [2] = {0x11,0x33};
            UINT8 data[7]     = {0x11,0x33,0x11,0x33,0x11,0x33,0x11};
            f4.pattern(BYTES(pattern,pattern+2));
            EXPECT_EQ(BYTES(data,data+7), (BYTES)f4);
        }

        {
            TEXT pattern("11:22:33");
            TEXT data("11:22:33:11:22:33:11");
            f4.pattern(pattern);
            EXPECT_EQ(data, (TEXT)f4);
        }
    }
}

TEST_F(FIELD_test, resizable)
{
    FIELD_RESIZABLE f("test_resizable");
    EXPECT_EQ(0, f.size_of_bit());

    //initially assigne bytes
    BYTES bytes(5, 0x11);
    f=bytes;
    EXPECT_EQ(5*8, f.size_of_bit());
    EXPECT_EQ(bytes, (BYTES)f);

    //change bytes
    bytes.erase(bytes.begin());
    f=bytes;
    EXPECT_EQ(8*4, f.size_of_bit());
    EXPECT_EQ(bytes, (BYTES)f);

    //resize
    f.resize(5);
    bytes.push_back(0);
    EXPECT_EQ(8*5, f.size_of_bit());
    EXPECT_EQ(bytes, (BYTES)f);

    f.resize(8);
    bytes.insert(bytes.end(), 3, 0);
    EXPECT_EQ(8*8, f.size_of_bit());
    EXPECT_EQ(bytes, (BYTES)f);

    f.resize(6);
    bytes.erase(bytes.end()-2, bytes.end());
    EXPECT_EQ(8*6, f.size_of_bit());
    EXPECT_EQ(bytes, (BYTES)f);

    //clone
    FIELD *pf = f.clone();
    EXPECT_TRUE(pf);
    EXPECT_EQ((BYTES)f, (BYTES)(*pf));
    EXPECT_EQ(f.size_of_bit(), pf->size_of_bit());

    EXPECT_TRUE(dynamic_cast<FIELD_RESIZABLE*> (pf));
    EXPECT_TRUE(0==dynamic_cast<FIELD_FIXED_SIZE*> (pf));

    delete pf;
}

TEST_F(FIELD_test, option)
{
    FIELD_OPTION f("test_option", 8);

    //add option and assigne option value
    BYTES bytes(1,1);
    f.option_add(bytes);
    f=bytes;
    EXPECT_TRUE(f.is_known_option());

    //assigne value before option added
    bytes[0]=2;
    f=bytes;
    EXPECT_FALSE(f.is_known_option());

    f.option_add(bytes);
    EXPECT_TRUE(f.is_known_option());

    //test option which is not latestly added
    bytes[0]=1;
    f=bytes;
    EXPECT_TRUE(f.is_known_option());

    //clone
    FIELD *pf = f.clone();
    EXPECT_TRUE(pf);
    EXPECT_EQ((BYTES)f, (BYTES)(*pf));
    EXPECT_EQ(f.size_of_bit(), pf->size_of_bit());

    EXPECT_TRUE(dynamic_cast<FIELD_OPTION*> (pf));
    EXPECT_TRUE(0==dynamic_cast<FIELD_RESIZABLE*> (pf));

}

TEST_F(FIELD_test, block)
{
    FIELD_BLOCK fb("test_block");

    //before install any member field
    EXPECT_THROW(fb.block("any"), EXP_ERROR);

    //create individual member field
    FIELD_FIXED_SIZE f_fixed("test_fixed", 8);
    FIELD_RESIZABLE f_resizable("test_resizable");
    FIELD_OPTION f_option("test_option", 3);    //2 option combines one byte
    FIELD_OPTION f_option_2("test_option_2", 5);

    //append and assigne, also test (FIELD::NAME)
    fb.append(f_fixed);
    EXPECT_EQ(8, fb.size_of_bit());
    EXPECT_EQ(0, fb.offset_of_bit("test_fixed"));
    fb.field("test_fixed")=BYTES(1,0x11);
    EXPECT_EQ(BYTES(1,0x11), (BYTES)fb.field("test_fixed"));
    EXPECT_EQ(BYTES(1,0x11), (BYTES)fb.field("test_fixed"));

    //insert and assigne, test (FIELD::INDEX)
    fb.insert("test_fixed", f_resizable);
    EXPECT_EQ(8, fb.size_of_bit());
    EXPECT_EQ(0, fb.offset_of_bit("test_fixed"));
    fb.field("test_resizable") = BYTES(2, 0x1);
    EXPECT_EQ(BYTES(2, 0x1), (BYTES)fb.field("test_resizable"));
    EXPECT_EQ(8*3, fb.size_of_bit());
    EXPECT_EQ(8*2, fb.offset_of_bit("test_fixed"));

    //insert with incorrect position
    EXPECT_THROW(fb.insert("test_not_exist", f_resizable), EXP_ERROR);

    //append all member
    fb.append(f_option);
    EXPECT_EQ(8*3+3, fb.size_of_bit());
    EXPECT_EQ(8*3, fb.offset_of_bit("test_option"));
    fb.field("test_option") = BYTES(1,0x4);
    EXPECT_EQ(BYTES(1, 0X4), (BYTES)fb.field("test_option"));

    fb.append(f_option_2);
    EXPECT_EQ(8*4, fb.size_of_bit());
    EXPECT_EQ(8*3+3, fb.offset_of_bit("test_option_2"));
    fb.field("test_option_2") = BYTES(1,0x10);
    EXPECT_EQ(BYTES(1, 0x10), (BYTES)fb.field("test_option_2"));

    //assigne block value, resizable field is placed in first
    {
        UINT8 data[]={0x01, 0x02, 0x03, 0xff};
        fb= BYTES(data, data+4);
        EXPECT_EQ(BYTES(data, data+4), (BYTES)fb);
        EXPECT_EQ(BYTES(data, data+2), (BYTES)fb.field("test_resizable"));
        EXPECT_EQ(BYTES(data+2, data+3), (BYTES)fb.field("test_fixed"));
        EXPECT_EQ(BYTES(1, 0x7), (BYTES)fb.field("test_option"));
        EXPECT_EQ(BYTES(1, 0x1f), (BYTES)fb.field("test_option_2"));

        //remove and insert back
        fb.remove("test_fixed");
        EXPECT_THROW(fb.field("test_fixed"), EXP_ERROR);
        EXPECT_EQ(8*3, fb.size_of_bit());

        fb.insert("test_resizable", f_fixed);
        EXPECT_EQ(8*4, fb.size_of_bit());

        fb.field("test_fixed")=BYTES(1,0x22);
        EXPECT_EQ(BYTES(1,0x22), (BYTES)fb.field("test_fixed"));

        EXPECT_EQ(BYTES(data, data+2), (BYTES)fb.field("test_resizable"));
        EXPECT_EQ(BYTES(1, 0x7), (BYTES)fb.field("test_option"));
        EXPECT_EQ(BYTES(1, 0x1f), (BYTES)fb.field("test_option_2"));
    }

    //assigne block value, now resizble field is placed in the middle
    {
        UINT8 data[]={0xaa, 0xbb, 0xcc, 0xdd, 0xee};
        fb=BYTES(data, data+5);
        EXPECT_EQ(BYTES(data, data+5), (BYTES)fb);
        EXPECT_EQ(BYTES(data, data+1), (BYTES)fb.field("test_fixed"));
        EXPECT_EQ(BYTES(data+1, data+4), (BYTES)fb.field("test_resizable"));
        EXPECT_EQ(BYTES(1, 0x7), (BYTES)fb.field("test_option"));
        EXPECT_EQ(BYTES(1, 0x0e), (BYTES)fb.field("test_option_2"));

        //remove and insert back
        fb.remove("test_resizable");
        EXPECT_THROW(fb.field("test_resizable"), EXP_ERROR);
        EXPECT_EQ(8*2, fb.size_of_bit());

        fb.append(f_resizable);
        EXPECT_EQ(8*2, fb.size_of_bit());

        EXPECT_EQ(BYTES(data, data+1), (BYTES)fb.field("test_fixed"));
        EXPECT_EQ(BYTES(1, 0x7), (BYTES)fb.field("test_option"));
        EXPECT_EQ(BYTES(1, 0x0e), (BYTES)fb.field("test_option_2"));
    }

    //assigne block value, now resizable field is placed at the end
    {
        UINT8 data[]={0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x56, 0x78};
        fb=BYTES(data, data+8);
        EXPECT_EQ(BYTES(data, data+8), (BYTES)fb);
        EXPECT_EQ(BYTES(data, data+1), (BYTES)fb.field("test_fixed"));
        EXPECT_EQ(BYTES(1, 0x6), (BYTES)fb.field("test_option"));
        EXPECT_EQ(BYTES(1, 0x0d), (BYTES)fb.field("test_option_2"));
        EXPECT_EQ(BYTES(data+2, data+8), (BYTES)fb.field("test_resizable"));
    }

    //in case of block in block (call it as "BB", in brief)
    FIELD_BLOCK fb_inside("block_tmp");
    fb_inside.append(FIELD_FIXED_SIZE("fixed_inside", 8*2));
    fb_inside.append(FIELD_RESIZABLE("resizable_inside"));
    fb_inside.rename("block_inside");

    //create inner block by extend
    fb.extend("test_resizable", fb_inside);
    EXPECT_EQ(8*2, fb.field("test_resizable").size_of_bit());
    EXPECT_EQ(8*4, fb.size_of_bit());
    EXPECT_EQ(8*2, fb.offset_of_bit("test_resizable"));
    EXPECT_EQ(8*2, fb.offset_of_bit("test_resizable.fixed_inside"));
    EXPECT_EQ(8*4, fb.offset_of_bit("test_resizable.resizable_inside"));
    EXPECT_EQ(0,   fb.block("test_resizable").offset_of_bit("fixed_inside"));
    EXPECT_EQ(8*2, fb.block("test_resizable").offset_of_bit("resizable_inside"));

    //create inner block by remove and append
    fb.remove("test_resizable");
    fb.append(fb_inside);
    EXPECT_EQ(8*2, fb.field("block_inside").size_of_bit());
    EXPECT_EQ(8*4, fb.size_of_bit());
    EXPECT_EQ(8*2, fb.offset_of_bit("block_inside"));
    EXPECT_EQ(8*2, fb.offset_of_bit("block_inside.fixed_inside"));
    EXPECT_EQ(8*4, fb.offset_of_bit("block_inside.resizable_inside"));
    EXPECT_EQ(0,   fb.block("block_inside").offset_of_bit("fixed_inside"));
    EXPECT_EQ(8*2, fb.block("block_inside").offset_of_bit("resizable_inside"));

    //BB: assigne value to field of inner block
    {
        UINT8 data[]={0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
        fb.block("block_inside").field("fixed_inside")=BYTES(data, data+2);
        EXPECT_EQ(BYTES(data, data+2), (BYTES)fb.block("block_inside").field("fixed_inside"));

        fb.field("block_inside.resizable_inside")=BYTES(data+2, data+6);
        EXPECT_EQ(BYTES(data+2, data+6), (BYTES)fb.field("block_inside.resizable_inside"));

        EXPECT_EQ(8*6, fb.block("block_inside").size_of_bit());
    }

    //BB: assigne block value
    {
        //to inner block
        UINT8 data[]={0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0};
        fb.block("block_inside")=BYTES(data, data+6);
        EXPECT_EQ(BYTES(data, data+6), (BYTES) (fb.block("block_inside")));
        EXPECT_EQ(BYTES(data, data+2), (BYTES) (fb.block("block_inside").field("fixed_inside")));
        EXPECT_EQ(BYTES(data+2, data+6), (BYTES) (fb.block("block_inside").field("resizable_inside")));
        EXPECT_EQ(8*6, fb.block("block_inside").size_of_bit());

        //to outer whole block
        fb=BYTES(data, data+6);
        EXPECT_EQ(BYTES(data, data+6), (BYTES) fb);
        EXPECT_EQ(BYTES(data, data+1), (BYTES) (fb.field("test_fixed")));
        EXPECT_EQ(BYTES(1, 0x5), (BYTES) (fb.field("test_option")));
        EXPECT_EQ(BYTES(1, 0x10), (BYTES) (fb.field("test_option_2")));
        EXPECT_EQ(BYTES(data+2, data+6), (BYTES) (fb.block("block_inside")));
        EXPECT_EQ(BYTES(data+2, data+4), (BYTES)fb.block("block_inside").field("fixed_inside"));
        EXPECT_EQ(BYTES(data+4, data+6), (BYTES)fb.block("block_inside").field("resizable_inside"));
    }

    //block in block in block (BBB, in brief)
    fb.block("block_inside").append(FIELD_BLOCK("block_tmp"));
    fb.block("block_inside").block("block_tmp").rename("block_most_inner");
    fb.block("block_inside").block("block_most_inner").append(FIELD_FIXED_SIZE("fixed_most_inner", 8));
    EXPECT_EQ(8, fb.block("block_inside.block_most_inner").size_of_bit());
    EXPECT_EQ(8, fb.field("block_inside.block_most_inner.fixed_most_inner").size_of_bit());
    EXPECT_EQ(0, fb.block("block_inside.block_most_inner").offset_of_bit("fixed_most_inner"));
    EXPECT_EQ(8*4, fb.block("block_inside").offset_of_bit("block_most_inner.fixed_most_inner"));
    EXPECT_EQ(8*6, fb.offset_of_bit("block_inside.block_most_inner.fixed_most_inner"));

    //BBB: value assigne
    {
        //to field of most inner block
        UINT8 data[]={0xf0, 0xe0, 0xd0, 0xc0, 0xb0, 0xa0};
        fb.field("block_inside.block_most_inner.fixed_most_inner")=BYTES(data, data+1);
        EXPECT_EQ(BYTES(data, data+1), (BYTES)fb.block("block_inside").block("block_most_inner").field("fixed_most_inner"));

        //to most inner block
        fb.field("block_inside.block_most_inner")=BYTES(data+1, data+2);
        EXPECT_EQ(BYTES(data+1, data+2), (BYTES)fb.block("block_inside").block("block_most_inner"));
        EXPECT_EQ(BYTES(data+1, data+2), (BYTES)fb.block("block_inside").block("block_most_inner").field("fixed_most_inner"));

        //to inner block
        fb.field("block_inside")=BYTES(data+2, data+5);
        EXPECT_EQ(BYTES(data+2, data+5), (BYTES)fb.block("block_inside"));
        EXPECT_EQ(BYTES(data+2, data+4), (BYTES)fb.block("block_inside").field("fixed_inside"));
        EXPECT_EQ(BYTES(), (BYTES)fb.block("block_inside").field("resizable_inside"));
        EXPECT_EQ(BYTES(data+4,data+5), (BYTES)fb.field("block_inside.block_most_inner"));
        EXPECT_EQ(BYTES(data+4,data+5), (BYTES)fb.field("block_inside.block_most_inner.fixed_most_inner"));

        //to out whole block
        fb=BYTES(data, data+6);
        EXPECT_EQ(BYTES(data, data+6), (BYTES)fb);
        EXPECT_EQ(8*6, fb.size_of_bit());
        EXPECT_EQ(BYTES(data, data+1), (BYTES)fb.field("test_fixed"));
        EXPECT_EQ(BYTES(1, 0x7), (BYTES)fb.field("test_option"));
        EXPECT_EQ(BYTES(1, 0x0), (BYTES)fb.field("test_option_2"));

        EXPECT_EQ(8*4, fb.field("block_inside").size_of_bit());
        EXPECT_EQ(8*1, fb.field("block_inside.resizable_inside").size_of_bit());
        EXPECT_EQ(BYTES(data+2, data+6), (BYTES)fb.field("block_inside"));
        EXPECT_EQ(BYTES(data+2, data+4), (BYTES)fb.block("block_inside").field("fixed_inside"));
        EXPECT_EQ(BYTES(data+4, data+5), (BYTES)fb.block("block_inside").field("resizable_inside"));

        EXPECT_EQ(8*1, fb.block("block_inside").block("block_most_inner").size_of_bit());
        EXPECT_EQ(BYTES(data+5, data+6), (BYTES)fb.field("block_inside.block_most_inner"));
        EXPECT_EQ(BYTES(data+5, data+6), (BYTES)fb.field("block_inside.block_most_inner.fixed_most_inner"));
    }

    printf(fb.dump().c_str());
}

TEST_F(FIELD_test, caculator)
{
    check_mock = true;

    //creat block
    FIELD_BLOCK fb("test_cac");
    fb.append(FIELD_FIXED_SIZE("fixed", 8));
    fb.append(FIELD_OPTION("option", 1));
    fb.append(FIELD_OPTION("option_2", 3));
    fb.append(FIELD_OPTION("option_3", 4));
    fb.append(FIELD_RESIZABLE("resizable"));

    //white list, data cac
    {
        fb.append(FIELD_CACULATOR("cac_data", 8,
                                   "resizable",
                                   FIELD_CACULATOR::WHITE_LIST,
                                   data_cac));

        UINT8 data [] = {0x01, 0xa3, 0x03, 0x04, 0x05, 0x06};
        fb.field("fixed")=BYTES(data, data+1);
        fb.field("option")  =BYTES(1, 0x01);
        fb.field("option_2")=BYTES(1, 0x02);
        fb.field("option_3")=BYTES(1, 0x03);
        fb.field("resizable")=BYTES(data+2, data+6);

        BYTES bytes(data+2, data+6);
        data_cac_ExpectAndReturn (bytes, BYTES(1,0xff), cmp_container<BYTES>);
        EXPECT_EQ(BYTES(1, 0xff), (BYTES)fb.field("cac_data"));
    }

    //black list, data cac
    {
        fb.remove("cac_data");
        fb.append(FIELD_CACULATOR("cac_data_2", 8,
                                   "fixed",
                                   FIELD_CACULATOR::BLACK_LIST,
                                   data_cac));

        UINT8 data [] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
        fb=BYTES(data, data+6);//block assigne
        EXPECT_EQ(BYTES(1, 0x66), (BYTES)fb.field("cac_data_2"));//cac field disabled

        BYTES bytes(data+1, data+5);
        data_cac_ExpectAndReturn (bytes, BYTES(1,0xee), cmp_container<BYTES>);
        fb.field("cac_data_2")=BYTES();//cac field enabled
        EXPECT_EQ(BYTES(1, 0xee), (BYTES)fb.field("cac_data_2"));
    }

    //white list, size cac
     {
         fb.remove("cac_data_2");

         FIELD::INDEX fields;
         fields.push_back("fixed");
         fields.push_back("resizable");
         fb.append(FIELD_CACULATOR("cac_size", 8,
                                    fields,
                                    FIELD_CACULATOR::WHITE_LIST,
                                    size_cac));

         UINT8 data [] = {0xaa, 0xbb, 0xcc, 0xdd};
         fb.field("fixed")=BYTES(data, data+1);
         fb.field("option")  =BYTES(1, 0x00);
         fb.field("option_2")=BYTES(1, 0x00);
         fb.field("option_3")=BYTES(1, 0x0b);
         fb.field("resizable")=BYTES(data+2, data+4);

         size_cac_ExpectAndReturn (8*3, BYTES(1,0xcc), cmp_int);
         EXPECT_EQ(BYTES(1, 0xcc), (BYTES)fb.field("cac_size"));
     }

    //black list, size cac
    {
        fb.remove("cac_size");
        fb.append(FIELD_CACULATOR("cac_size_2", 8,
                                   FIELD::INDEX(),
                                   FIELD_CACULATOR::BLACK_LIST,
                                   size_cac));

        UINT8 data [] = {0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00};
        fb=BYTES(data, data+7);
        fb.field("cac_size_2")=BYTES();//cac field enabled
        size_cac_ExpectAndReturn (8*7, BYTES(1,0xdd), cmp_int);
        EXPECT_EQ(BYTES(1, 0xdd), (BYTES)fb.field("cac_size_2"));
    }

    //block in block, (outer size cac, inner data cac)
    fb.remove("resizable");
    fb.append(FIELD_BLOCK("inner_block"));
    fb.block("inner_block").append(FIELD_FIXED_SIZE("inner_fixed", 8*2));
    fb.block("inner_block").append(FIELD_CACULATOR("inner_cac", 8,
                                                  FIELD::INDEX(),
                                                  FIELD_CACULATOR::BLACK_LIST,
                                                  data_cac));
    fb.block("inner_block").append(FIELD_RESIZABLE("inner_resizable"));

    {
        UINT8 data [] = {0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0};
        fb=BYTES(data, data+16);
        fb.field("cac_size_2")=BYTES();
        fb.block("inner_block").field("inner_cac")=BYTES();

        size_cac_ExpectAndReturn (8*16, BYTES(1,0x20), cmp_int);

        BYTES bytes(data+3, data+16);
        bytes.erase(bytes.begin()+2);
        data_cac_ExpectAndReturn (bytes, BYTES(1,0x50), cmp_container<BYTES>);

        EXPECT_EQ(BYTES(data, data+16), (BYTES)fb);
    }

    //block in block, (outer and inner both data cac)
    fb.remove("cac_size_2");
    fb.insert("inner_block",
               FIELD_CACULATOR("outer_cac", 8,
                               FIELD::INDEX(),
                               FIELD_CACULATOR::BLACK_LIST,
                               data_cac));
    {
        UINT8 data [] = {0xf0, 0xe0, 0xd0, 0xc0, 0xb0, 0xa0, 0x90, 0x80, 0x70, 0x60};
        fb=BYTES(data, data+10);
        fb.field("outer_cac")=BYTES();
        fb.block("inner_block").field("inner_cac")=BYTES();

        BYTES bytes_inner(data+3, data+10);
        bytes_inner.erase(bytes_inner.begin()+2);
        data_cac_ExpectAndReturn (bytes_inner, BYTES(1,0xa0), cmp_container<BYTES>);

        BYTES bytes_outer(data, data+10);
        bytes_outer.erase(bytes_outer.begin()+2);
        data_cac_ExpectAndReturn (bytes_outer, BYTES(1,0xd0), cmp_container<BYTES>);


        EXPECT_EQ(BYTES(data, data+10), (BYTES)fb);
    }
}

TEST_F(FIELD_test, op_packets)
{
    //packets
    FIELD_BLOCK fb("block");
    fb.append(FIELD_FIXED_SIZE("fixed_1", 8*2));
    fb.append(FIELD_FIXED_SIZE("fixed_2", 8*4));

    fb.field("fixed_1") = FIELD::STEP(true, BYTES(1,0x11), BYTES(2, 0x11), 8*2, 2);
    fb.field("fixed_2") = BYTES(4,0xaa);

    UINT8 data [] = {0x11,0x11,0xaa,0xaa,0xaa,0xaa};
    PACKETS pkts;
    pkts.push_back(PACKET(data, data+6));
    data[1] = 0x22;
    pkts.push_back(PACKET(data, data+6));

    EXPECT_EQ(pkts, (PACKETS)fb);

    //second time
    EXPECT_EQ(pkts, (PACKETS)fb);

    //with caculator, first update field, then caculate field
    fb.insert("fixed_1", FIELD_CACULATOR("cac_data", 8,
                                           "fixed_1",
                                           FIELD_CACULATOR::WHITE_LIST,
                                           data_cac));

    data[1] = 0x11;
    data_cac_ExpectAndReturn (BYTES(data, data+2), BYTES(1,0xcc), cmp_container<BYTES>);
    data[1] = 0x22;
    data_cac_ExpectAndReturn (BYTES(data, data+2), BYTES(1,0xdd), cmp_container<BYTES>);

    pkts[0].insert(pkts[0].begin(), 0xcc);
    pkts[1].insert(pkts[1].begin(), 0xdd);
    EXPECT_EQ(pkts, (PACKETS)fb);

}

#include <pkt/ethernet.h>
#include <pkt/ipv4.h>
#include <pkt/ipv6.h>

TEST_F(FIELD_test, lib)
{
    {
        using namespace FB_ETH_II;

        FIELD_BLOCK fb(make_ETH_II());
        EXPECT_NE(0, fb.size_of_bit());

        UINT8 data [] = {0x00,0x01,0x02,0x03,0x04,0x05,
                         0x00,0x11,0x22,0x33,0x44,0x55,
                         0x08,0x00,
                         0x00,0x00,0x00,0x00};

        fb=BYTES(data, data+18);
        EXPECT_EQ(BYTES(data, data+6), (BYTES)fb.field(DST_MAC));
        EXPECT_EQ(BYTES(data+6, data+12), (BYTES)fb.field(SRC_MAC));
        EXPECT_EQ(BYTES(data+12, data+14), (BYTES)fb.field(ETH_TYPE));

        //case of insert another block
        using namespace FB_FLOW_TRACK_TAG;
        fb.insert(L3_PAYLOAD, make_FLOW_TRACK_TAG());
    }

    {
        using namespace FB_VLAN_TAG;

        FIELD_BLOCK fb(make_VLAN_TAG());
        EXPECT_EQ(32, fb.size_of_bit());

        UINT8 data [] = {0x81,0x00,0x28,0x00};

        fb=BYTES(data, data+4);
        EXPECT_EQ(BYTES(data, data+2), (BYTES)fb.field(TPID));
        EXPECT_EQ(BYTES(1, 0x1), (BYTES)fb.field(PBIT));
        EXPECT_EQ(BYTES(1, 0x0), (BYTES)fb.field(DEI));

        BYTES vlan_id;
        vlan_id.push_back(0x08);
        vlan_id.push_back(0x00);
        EXPECT_EQ(vlan_id, (BYTES)fb.field(VLAN_ID));
    }

    {
        using namespace FB_IPV4;
        FIELD_BLOCK fb(make_IPV4());
        EXPECT_NE(0, fb.size_of_bit());

        UINT8 data [] = {0x45,0x00,0x00,0x2E,
                         0x00,0x00,0x00,0x00,
                         0x40,0x72,0x31,0xf3,
                         0x01,0x02,0x03,0x04,
                         0x11,0x22,0x33,0x44};

        fb=BYTES(data, data+20);
        EXPECT_EQ(BYTES(1, 0x04), (BYTES)fb.field(VERSION));
        EXPECT_EQ(BYTES(1, 0x05), (BYTES)fb.field(IHL));
        EXPECT_EQ(BYTES(1, 0x00), (BYTES)fb.field(TOS));
        EXPECT_EQ(BYTES(data+4, data+6), (BYTES)fb.field(IP_ID));
        EXPECT_EQ(BYTES(1, 0x00), (BYTES)fb.field(FLAGS));
        EXPECT_EQ(BYTES(2, 0x00), (BYTES)fb.field(FRAG_OFF));
        EXPECT_EQ(BYTES(1, 0x40), (BYTES)fb.field(TTL));
        EXPECT_EQ(BYTES(1, 0x72), (BYTES)fb.field(PROTOCOL));
        EXPECT_EQ(BYTES(data+12, data+16), (BYTES)fb.field(SRC_IP));
        EXPECT_EQ(BYTES(data+16, data+20), (BYTES)fb.field(DST_IP));

        //padding
        fb.field(L4_PAYLOAD)=BYTES(26, 0x00);

        //caculation
        fb.field(TOT_LEN)=BYTES();
        EXPECT_EQ(BYTES(data+2, data+4), (BYTES)fb.field(TOT_LEN));

        fb.field(CHK_SUM)=BYTES();
        EXPECT_EQ(BYTES(data+10, data+12), (BYTES)fb.field(CHK_SUM));
    }

    {
        using namespace FB_IPV6;
        FIELD_BLOCK fb(make_IPV6());
        EXPECT_NE(0, fb.size_of_bit());

        UINT8 data [] = {0x60,0x10,0x00,0x00,
                         0x00,0x00,0x33,0x80,
                         0x00,0x01,0x02,0x03,
                         0x04,0x05,0x06,0x07,
                         0x08,0x09,0x0a,0x0b,
                         0x0c,0x0d,0x0e,0x0f,
                         0x00,0x10,0x20,0x30,
                         0x40,0x50,0x60,0x70,
                         0x80,0x90,0xa0,0xb0,
                         0xc0,0xd0,0xe0,0xf0,
                         0x00,0x11,0x22,0x33};

        fb=BYTES(data, data+44);
        EXPECT_EQ(BYTES(1, 0x06), (BYTES)fb.field(VERSION));
        EXPECT_EQ(BYTES(1, 0x01), (BYTES)fb.field(TC));
        EXPECT_EQ(BYTES(3, 0x00), (BYTES)fb.field(FLB));
        EXPECT_EQ(BYTES(1, 0x33), (BYTES)fb.field(NH));
        EXPECT_EQ(BYTES(1, 0x80), (BYTES)fb.field(HLMT));
        EXPECT_EQ(BYTES(data+8, data+24), (BYTES)fb.field(SRC_IP));
        EXPECT_EQ(BYTES(data+24, data+40), (BYTES)fb.field(DST_IP));
        EXPECT_EQ(BYTES(data+40, data+44), (BYTES)fb.field(L4_PAYLOAD));

        //caculate field
        {
            fb.field(PLEN)=BYTES();

            UINT8 data [] = {0x00, 0x04};
            EXPECT_EQ(BYTES(data, data+2), (BYTES)fb.field(PLEN));
        }
    }

    {
        using namespace FB_FLOW_TRACK_TAG;
        FIELD_BLOCK fb(make_FLOW_TRACK_TAG());
        EXPECT_EQ(32, fb.size_of_bit());

        UINT8 data [] = {0x12,0x34,0x56,0x7f};
        fb=BYTES(data, data+4);

        {
            UINT8 data [] = {0x01,0x23,0x45,0x67};
            EXPECT_EQ(BYTES(data, data+4), (BYTES)fb.field(FLOW_ID));
        }

        EXPECT_EQ(BYTES(1, 0x01), (BYTES)fb.field(LATENCY_FLAG));
        EXPECT_EQ(BYTES(1, 0x07), (BYTES)fb.field(LATENCY_SN));
    }

    {
        using namespace FB_ETH_II;
        using namespace FB_IPV4;
        using namespace FB_VLAN_TAG;

        FIELD_BLOCK fb(make_ETH_II(make_IPV4()));

        fb.insert(ETH_TYPE, make_VLAN_TAG());
    }
}
