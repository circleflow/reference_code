
#ifndef UTILITY_UNIQUE_H_
#define UTILITY_UNIQUE_H_

#include "error.h"

#include <deque>
using std::deque;
#include <map>
using std::map;
#include <set>
using std::set;

namespace UTILITY {

    /*allocation for ID in scope of MIN_ID~MAX_ID,
      the ID range also could be dynamic, provided by function pointer, only called once at first instance
      with pool option
      example:
         static ID range:
             typedef UNIQUE<my_type, int, 0, 127> MY_UNIQUE;
         dynamic ID range:
             int my_min_id();
             int my_max_id();
             typedef UNIQUE<my_type, int, 0, 0, my_min_id, my_max_id> MY_UNIQUE;
     */
    template <typename ID_TYPE, ID_TYPE VAL>
    ID_TYPE unique_id_val() { return VAL; }

    template < typename CLASS_TYPE, typename ID_TYPE,
               ID_TYPE MIN_ID, ID_TYPE MAX_ID,
               ID_TYPE(MIN_F)()=unique_id_val<ID_TYPE,MIN_ID>, ID_TYPE(MAX_F)()=unique_id_val<ID_TYPE,MAX_ID> >
    class UNIQUE
    {
    public:
        UNIQUE(int _pool=0)
        :pool(_pool)
        {
            if(false == is_inited) {
                min_id = MIN_F();
                max_id = MAX_F();
                is_inited = true;
            }

            if(get_db_free(pool).empty()) {
                 expand_free();
            }

            id = get_db_free(pool).front();
            get_db_free(pool).pop_front();
        }

        ~UNIQUE()
        {
            get_db_free(pool).push_back(id);
        }

        ID_TYPE get_id(void) const
        {
            return id;
        }

        operator const ID_TYPE() const
        {
            return id;
        }

        static
        bool is_allocated(ID_TYPE id, int pool=0)
        {
            DB_ALLOC &db=get_db_alloc(pool);
            return (db.end()==db.find(id)) ? false : true;
        }

    private:
        UNIQUE(const UNIQUE &);
        UNIQUE & operator = (const UNIQUE &);

        int pool;
        ID_TYPE id;

        static bool is_inited;
        static ID_TYPE min_id,max_id;

        typedef deque<ID_TYPE> DB_FREE;

        static
        DB_FREE & get_db_free(int pool)
        {
            static map< int, DB_FREE > db;
            return db[pool];
        }

        typedef set<ID_TYPE> DB_ALLOC;

        static
        DB_ALLOC & get_db_alloc(int pool)
        {
            static map< int, DB_ALLOC > db;
            return db[pool];
        }

        void new_id_segment(ID_TYPE &min, ID_TYPE &max)
        {
            ENSURE(max_id>=min_id);

            static map<int, ID_TYPE> db_max;
            const ID_TYPE expand_num = 64;

            if(db_max.end() == db_max.find(pool)) {

                if((max_id-min_id+1)<expand_num) {
                    max = max_id;
                } else {
                    max = min_id + expand_num - 1;
                }

                min = min_id;

            }else {

                ID_TYPE &max_now = db_max[pool];

                ENSURE(max_now<max_id);

                if((max_id-max_now)<expand_num) {
                    max = max_id;
                } else {
                    max = max_now + expand_num;
                }

                min = max_now + 1;
            }

            ENSURE(max<=max_id);

            db_max[pool] = max;
        }

        void expand_free(void)
        {
            ID_TYPE min, max;

            new_id_segment(min, max);

            //keep i never exceed than max_id, avoid value revert
            for(ID_TYPE i=min; i<max; i++) {
                get_db_free(pool).push_back(i);
            }

            get_db_free(pool).push_back(max);
        }
    };

    template < typename CLASS_TYPE, typename ID_TYPE,
               ID_TYPE MIN_ID, ID_TYPE MAX_ID,
               ID_TYPE(MIN_F)(), ID_TYPE(MAX_F)() >
    bool UNIQUE<CLASS_TYPE, ID_TYPE, MIN_ID, MAX_ID, MIN_F, MAX_F>::is_inited=false;

    template < typename CLASS_TYPE, typename ID_TYPE,
               ID_TYPE MIN_ID, ID_TYPE MAX_ID,
               ID_TYPE(MIN_F)(), ID_TYPE(MAX_F)() >
    ID_TYPE UNIQUE<CLASS_TYPE, ID_TYPE, MIN_ID, MAX_ID, MIN_F, MAX_F>::min_id=MIN_ID;

    template < typename CLASS_TYPE, typename ID_TYPE,
               ID_TYPE MIN_ID, ID_TYPE MAX_ID,
               ID_TYPE(MIN_F)(), ID_TYPE(MAX_F)() >
    ID_TYPE UNIQUE<CLASS_TYPE, ID_TYPE, MIN_ID, MAX_ID, MIN_F, MAX_F>::max_id=MAX_ID;
}

#endif /* UTILITY_UNIQUE_H_ */
