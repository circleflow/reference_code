
#ifndef CIRCLE_FLOW_PORT_TYPE_H_
#define CIRCLE_FLOW_PORT_TYPE_H_

namespace CIRCLE_FLOW {
    namespace PORT {
        enum INTERFACE{
            ITF_RJ45,
            ITF_SFP,
            ITF_END
        };

        typedef set<INTERFACE> ITF_SET;

        enum DUPLEX_SPEED{
            FD_10MB,
            HD_10MB,
            FD_100MB,
            HD_100MB,
            FD_1000MB,
            HD_1000MB,
            FD_2500MB,
            HD_2500MB,
            FD_10GB,
            HD_10GB,
            DS_END
        };

        typedef set<DUPLEX_SPEED> DS_SET;

        struct ABILITY {
            DS_SET ds_set;
            bool auto_nego;
            bool pause;
        };

        struct ADVERT {
            DS_SET ds_set;
            bool pause;
        };

        struct MODE {
            DUPLEX_SPEED ds;
            bool pause;

            bool operator == (const MODE& ref) { return (ds==ref.ds) && (pause==ref.pause);}
            bool operator != (const MODE& ref) { return !(operator ==(ref)); }
        };

        struct STATUS{
            DUPLEX_SPEED ds;
            bool         an_enable;
            bool         pause;
            bool         link;
        };
    }
}



#endif /* EXPORT_PORT_TYPE_H_ */
