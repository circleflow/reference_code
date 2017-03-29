
#ifndef CIRCLE_FLOW_FIELD_SEED_H_
#define CIRCLE_FLOW_FIELD_SEED_H_

#include "field.h"

namespace CIRCLE_FLOW {

    class FIELD::SEED {
    public:

        unsigned int get_count() const;
        unsigned int get_size_of_bit() const;

        virtual void reset() = 0;
        virtual const BYTES & next() = 0;

        virtual SEED * clone() const = 0;
        virtual ~SEED() = 0;

    protected:
        unsigned int size_of_bit, count;

        SEED(unsigned int size_of_bit, unsigned int count);
    };

    const int MAX_SEED_COUNT = 256;

    class FIELD::STEP : public FIELD::SEED {
    public:
        STEP(bool is_increase,
             const BYTES &step,
             const BYTES &base,
             unsigned int size_of_bit,
             unsigned int count);
        ~STEP();

        void reset();
        const BYTES & next();

        FIELD::SEED * clone() const;

    private:
        bool is_increase;
        BYTES step, base, current;
        unsigned int i;
    };

    class FIELD::RANDOM : public FIELD::SEED {
    public:
        RANDOM(unsigned int size_of_bit, unsigned int count);
        ~RANDOM();

        void reset();
        const BYTES & next();

        FIELD::SEED * clone() const;

    private:
        BYTES bytes;
    };
}

#endif /* _FIELD_SEED_H_ */
