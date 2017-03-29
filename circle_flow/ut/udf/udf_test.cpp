
#include "test_helper.h"

#include "udf.h"
#include "type_ext.h"
#include "error.h"

#include "test_helper.h"

#include "hal_stub.hpp"
#include "hal_udf_stub.hpp"

#include <algorithm>
using std::swap_ranges;

class UDF_test : public ::testing::Test {
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

static int chunk_aligned_offset(int offset)
{
    int aligned;

    if((offset%4)>=2) {
        aligned = (offset/4)*4+2;
    } else {
        aligned = (offset/4)*4-2;
    }

    return aligned;
}

vector<UINT8> hal_unit_all(void)
{
    static vector<UINT8> units;

    if(0 == units.size()) {
        units.push_back(0);
        units.push_back(1);
    }

    return units;
}

static UDF_HW_SPECS udf_specs;

const CIRCLE_FLOW::UDF_HW_SPECS & udf_specs_cb (UINT8 unit, int)
{
    static bool inited = false;

    if(false == inited) {
        udf_specs.num_of_block = 2;
        udf_specs.len_of_block = 16;
        udf_specs.num_of_chunk = 8;
        udf_specs.len_of_chunk = 4;

        udf_specs.max_offset = 127;

        udf_specs.chunk_aligned_offset = chunk_aligned_offset;

        inited = true;
    }
    return udf_specs;
}

static void mock_chunk_set(UINT8 unit, UINT8 idx, UINT8 offset)
{
    hal_udf_chunk_set_ExpectAndReturn (unit, idx, offset, cmp_byte, cmp_byte, cmp_byte);
}

static QUAL::MATCH udf_match(UINT8 num_of_byte)
{
    QUAL::MATCH match;

    for(UINT8 i=0; i<num_of_byte; i++) {
        match.value.push_back(i);
        match.mask.push_back(0xff);
    }

    return match;
}

static QUAL::MATCH fp_match(const QUAL::MATCH &match, UINT8 offset)
{
    QUAL::MATCH fp_full;
    fp_full.value.resize(udf_specs.num_of_chunk*udf_specs.len_of_chunk, 0);
    fp_full.mask = fp_full.value;

    copy(match.value.begin(), match.value.end(), fp_full.value.begin()+offset);
    copy(match.mask.begin(),  match.mask.end(),  fp_full.mask.begin()+offset);

    return fp_full;
}

TEST_F(UDF_test, basic)
{
    check_mock = true;

    hal_udf_specs_MockWithCallback(udf_specs_cb);

    mock_chunk_set(0, 0, 2);
    UDF *udf_1 = new UDF(0, 2, 4);

    QUAL::MATCH match_1 = udf_match(4);
    udf_1->set_match(match_1);
    EXPECT_EQ(fp_match(match_1,0), udf_1->get_match());

    mock_chunk_set(0, 1, 6);
    UDF *udf_2 = new UDF(0, 7, 3);
    QUAL::MATCH match_2 = udf_match(3);
    udf_2->set_match(match_2);
    EXPECT_EQ(fp_match(match_2,5), udf_2->get_match());

    UDF *udf_3 = new UDF(0, 6, 4);
    udf_3->set_match(match_1);
    EXPECT_EQ(fp_match(match_1,4), udf_3->get_match());

    mock_chunk_set(0, 2, 10);
    UDF *udf_4 = new UDF(0, 7, 4);
    udf_4->set_match(match_1);
    EXPECT_EQ(fp_match(match_1,5), udf_4->get_match());

    delete udf_1;

    mock_chunk_set(0, 3, 2);    //unique ID would allocate 3
    UDF *udf_5 = new UDF(0, 3, 3);

    delete udf_2;
    delete udf_3;
    delete udf_4;
    delete udf_5;
}

TEST_F(UDF_test, two_unit)
{
    check_mock = true;
    hal_udf_specs_MockWithCallback(udf_specs_cb);

    mock_chunk_set(0, 4, 2);    //unique would allocate 4
    UDF udf_1(0, 2, 4);
    QUAL::MATCH match_1 = udf_match(4);
    udf_1.set_match(match_1);
    EXPECT_EQ(fp_match(match_1,16), udf_1.get_match());

    mock_chunk_set(1, 0, 6);
    UDF udf_2(1, 7, 3);
    QUAL::MATCH match_2 = udf_match(3);
    udf_2.set_match(match_2);
    EXPECT_EQ(fp_match(match_2,1), udf_2.get_match());

    mock_chunk_set(0, 5, 6);    //unique would allocate 5
    UDF udf_3(0, 6, 4);
    udf_3.set_match(match_1);
    EXPECT_EQ(fp_match(match_1,20), udf_3.get_match());

    mock_chunk_set(1, 1, 10);
    mock_chunk_set(1, 2, 14);
    UDF udf_4(1, 7, 8);
    QUAL::MATCH match_4 = udf_match(8);
    udf_4.set_match(match_4);
    EXPECT_EQ(fp_match(match_4,1), udf_4.get_match());
}

TEST_F(UDF_test, twist_chunk)
{
    check_mock = true;
    hal_udf_specs_MockWithCallback(udf_specs_cb);

    mock_chunk_set(0, 6, 6);    //unique would allocate 6
    UDF udf_1(0, 6, 4);
    QUAL::MATCH match_1 = udf_match(4);
    udf_1.set_match(match_1);
    EXPECT_EQ(fp_match(match_1,24), udf_1.get_match());

    mock_chunk_set(0, 7, 2);    //unique would allocate 7
    UDF udf_2(0, 3, 3);
    QUAL::MATCH match_2 = udf_match(3);
    udf_2.set_match(match_2);
    EXPECT_EQ(fp_match(match_2,29), udf_2.get_match());

    UDF udf_3 (0, 2, 8);
    QUAL::MATCH match_3 = udf_match(8);
    udf_3.set_match(match_3);
    QUAL::MATCH match_3_2 = fp_match(match_3, 24);
    swap_ranges(match_3_2.value.begin()+24,
                match_3_2.value.begin()+28,
                match_3_2.value.begin()+28);
    swap_ranges(match_3_2.mask.begin()+24,
                match_3_2.mask.begin()+28,
                match_3_2.mask.begin()+28);
    EXPECT_EQ(match_3_2, udf_3.get_match());

}

TEST_F(UDF_test, dimension)
{
    hal_udf_specs_MockWithCallback(udf_specs_cb);

    UDF udf_1(0, 2, 4);
    UDF udf_2(0, 6, 4);
    UDF udf_3(0, 10, 4);
    UDF udf_4(0, 14, 4);

    UDF udf_5(0, 18, 4);
    UDF udf_6(0, 22, 4);
    UDF udf_7(0, 26, 4);
    UDF udf_8(0, 30, 4);

    EXPECT_THROW(UDF(0, 34, 4), EXP_ERROR);
}

TEST_F(UDF_test, boarder)
{
    hal_udf_specs_MockWithCallback(udf_specs_cb);

    UDF udf_1(0, 0, 4);
    UDF udf_2(0, 127, 1);

    EXPECT_THROW(UDF(0, 128, 1), EXP_ERROR);
}
