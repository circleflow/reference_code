
#include "test_helper.h"

#include "type_ext.h"
#include "error.h"
#include "qual.h"
#include "fp/vfp_rx.h"

#include "vfp_rx_entry_mock_stub.hpp"

#include <map>
using std::map;
using std::make_pair;

class VFP_RX_test : public ::testing::Test {
protected:

    bool check_mock;

    UINT8 unit;
    UINT8 unit_2;

    vector<UINT8> ports;
    vector<UINT8> ports_2;

    QUAL::PDF_RULES rules;
    QUAL::PDF_RULES rules_2;

    virtual void SetUp()
    {
        check_mock = false;

        opmock_test_reset();

        unit = 0;
        unit_2 = 1;

        ports.push_back(4);

        ports_2 = ports;
        ports_2.push_back(6);

        QUAL::MATCH match;
        match.value.push_back(0x80);
        match.value.push_back(0x00);
        match.mask.push_back(0xff);
        match.mask.push_back(0xff);
        rules.insert(make_pair(QUAL::ether_type, match));

        rules_2 = rules;
        rules_2[QUAL::ether_type].value[1]=0x08;
    }

    virtual void TearDown()
    {
        if(check_mock) {
            EXPECT_VERIFY_MOCK();
        }
    }
};

struct ENTRY_KEY {
    UINT8 unit;
    UINT8 port;
    VFP_CLASS_ID class_id;

    ENTRY_KEY(UINT8 _unit, UINT8 _port, VFP_CLASS_ID _class_id)
    :unit(_unit),port(_port),class_id(_class_id) {}

    bool operator < (const ENTRY_KEY &ref) const
    { return make_pair(make_pair(unit,port),class_id)
            < make_pair(make_pair(ref.unit,ref.port),ref.class_id); }
};

static map<ENTRY_KEY, VFP_ENTRY *> db_entry;

VFP_ENTRY::VFP_ENTRY(UINT8 unit, UINT8 port,
                     VFP_CLASS_ID class_id,
                     const QUAL::PDF_RULES &rules,
                     UINT8 pri)
{
    //printf("\r\n VFP_ENTRY construct, instance %x", this);

    VFP_ENTRY_construct(unit, port, class_id, rules, pri);

    ENTRY_KEY key(unit,port,class_id);
    EXPECT_EQ(db_entry.end(), db_entry.find(key));
    db_entry.insert(make_pair(key,this));
}

VFP_ENTRY::~VFP_ENTRY()
{
    //printf("\r\n VFP_ENTRY destruct, instance %x", this);
    VFP_ENTRY_destruct (this);
}

void mock_construct_VFP_ENTRY(UINT8 unit, const vector<UINT8> &ports,
                              VFP_CLASS_ID class_id,
                              const QUAL::PDF_RULES &rules)
{
    for(vector<UINT8>::const_iterator it=ports.begin(); it!=ports.end(); it++) {
        VFP_ENTRY_construct_ExpectAndReturn (unit, *it, class_id, rules, 0,
                cmp_type<UINT8>, cmp_type<UINT8>, cmp_type<VFP_CLASS_ID>, cmp_container<QUAL::PDF_RULES>, cmp_type<UINT8>);
    }
}

void mock_destruct_VFP_ENTRY(VFP_ENTRY *instance)
{
    VFP_ENTRY_destruct_ExpectAndReturn (instance, cmp_ptr);
}


TEST_F(VFP_RX_test, basic)
{
    check_mock = true;

    VFP_CLASS_ID class_id = 1;

    db_entry.clear();

    mock_construct_VFP_ENTRY(unit,ports,class_id,rules);

    VFP_RX rx(unit, ports, rules, 0);

    EXPECT_EQ(1, db_entry.size());

    ENTRY_KEY key(unit,ports[0],class_id);
    EXPECT_NE(db_entry.end(), db_entry.find(key));

    mock_destruct_VFP_ENTRY(db_entry[key]);
}


TEST_F(VFP_RX_test, repeated_entry)
{
    check_mock = true;

    VFP_CLASS_ID class_id = 2;

    db_entry.clear();

    mock_construct_VFP_ENTRY(unit,ports,class_id,rules);

    VFP_RX rx(unit, ports, rules, 0);

    //repeated entry
    {
        VFP_RX rx_2(unit, ports, rules, 0);

        EXPECT_EQ(rx.get_class_id(), rx_2.get_class_id());
    }

    mock_destruct_VFP_ENTRY(db_entry[ENTRY_KEY(unit,ports[0],class_id)]);

}

TEST_F(VFP_RX_test, two_ports)
{
    check_mock = true;

    VFP_CLASS_ID class_id = 3;

    db_entry.clear();

    mock_construct_VFP_ENTRY(unit,ports_2,class_id,rules);

    VFP_RX rx(unit, ports_2, rules, 0);

    EXPECT_TRUE(2==db_entry.size());

    mock_destruct_VFP_ENTRY(db_entry[ENTRY_KEY(unit,ports_2[0],class_id)]);

    mock_destruct_VFP_ENTRY(db_entry[ENTRY_KEY(unit,ports_2[1],class_id)]);
}

TEST_F(VFP_RX_test, two_class_id)
{
    check_mock = true;

    VFP_CLASS_ID class_id = 4, class_id_2=5;

    db_entry.clear();

    //install first
    mock_construct_VFP_ENTRY(unit,ports,class_id,rules);

    VFP_RX rx(unit, ports, rules, 0);

    //install second rule/class_id
    {
        mock_construct_VFP_ENTRY(unit,ports,class_id_2,rules_2);
        VFP_RX rx_2(unit, ports, rules_2, 0);

        //uninstall second
        mock_destruct_VFP_ENTRY(db_entry[ENTRY_KEY(unit,ports[0],class_id_2)]);
    }

    //unistall first
    mock_destruct_VFP_ENTRY(db_entry[ENTRY_KEY(unit,ports[0],class_id)]);

}


TEST_F(VFP_RX_test, mix_all)
{
    check_mock = true;

    VFP_CLASS_ID class_id = 6, class_id_2=7;

    db_entry.clear();

    //install first
    mock_construct_VFP_ENTRY(unit,ports,class_id,rules);

    VFP_RX rx(unit, ports, rules, 0);

    //repeate entry on first port
    {
        mock_construct_VFP_ENTRY(unit,vector<UINT8>(1,ports_2[1]),class_id,rules);

        VFP_RX rx(unit, ports_2, rules, 0);

        mock_destruct_VFP_ENTRY(db_entry[ENTRY_KEY(unit,ports_2[1],class_id)]);
    }

    //different rule
    {
        mock_construct_VFP_ENTRY(unit,ports_2,class_id_2,rules_2);

        VFP_RX rx(unit, ports_2, rules_2, 0);

        mock_destruct_VFP_ENTRY(db_entry[ENTRY_KEY(unit,ports_2[0],class_id_2)]);
        mock_destruct_VFP_ENTRY(db_entry[ENTRY_KEY(unit,ports_2[1],class_id_2)]);
    }

    //uninstall
    mock_destruct_VFP_ENTRY(db_entry[ENTRY_KEY(unit,ports[0],class_id)]);
}


