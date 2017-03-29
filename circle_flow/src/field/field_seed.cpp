
#include "field_seed.h"
#include "bytes_bits.h"

#include "utility/export/error.h"
#include "utility/export/random_u.h"

FIELD::SEED::SEED(unsigned int size_of_bit, unsigned int count)
:size_of_bit(size_of_bit), count(count)
{ }

FIELD::SEED::~SEED()
{ }

unsigned int FIELD::SEED::get_count() const
{
    return count;
}

unsigned int FIELD::SEED::get_size_of_bit() const
{
    return size_of_bit;
}

typedef BYTES::reverse_iterator BYTES_ITER;
typedef BYTES::const_reverse_iterator BYTES_CITER;

static
void byte_add(BYTES_ITER a_first, BYTES_ITER a_last, UINT8 b)
{
    UINT16 u16 = *a_first + b;
    *a_first = static_cast<UINT8>(u16);

    if(u16>>8) {
        a_first++;

        if(a_first != a_last) {
            byte_add(a_first, a_last, 1);
        }
    }
}

static
BYTES bytes_add(const BYTES &a, const BYTES &b)
{
    ENSURE(a.size() >= b.size(),
            "byte size of operand b need <= a");

    BYTES sum = a;

    BYTES_ITER  i_sum = sum.rbegin();
    BYTES_CITER i_b   = b.rbegin();

    for(; i_b!=b.rend(); ) {

        byte_add(i_sum, sum.rend(), *i_b);

        i_sum++;
        i_b++;
    }

    return sum;
}

static
void byte_sub(BYTES_ITER a_first, BYTES_ITER a_last, UINT8 b)
{
    if( *a_first >= b){

        *a_first = *a_first - b;

    } else {

        UINT16 u16 = 0x0100 + *a_first;
        *a_first = (UINT8) (u16 - b);

        a_first++;
        if(a_first != a_last) {
            byte_sub(a_first, a_last, 1);
        }
    }
}

BYTES bytes_sub(const BYTES &a, const BYTES &b)
{
    ENSURE(a.size() >= b.size(),
            "byte size of operand b need <= a");

    BYTES diff=a;

    BYTES_ITER  i_diff = diff.rbegin();
    BYTES_CITER i_b    = b.rbegin();

    for(; i_b!=b.rend(); ) {
        byte_sub(i_diff, diff.rend(), *i_b);

        i_diff++;
        i_b++;
    }

    return diff;
}


FIELD::STEP::STEP(bool _is_increase,
                  const BYTES &_step,
                  const BYTES &_base,
                  unsigned int size_of_bit,
                  unsigned int count)
: SEED(size_of_bit, count),
  is_increase(_is_increase),
  step(_step), base(_base), current(base),
  i(0)
{
    ENSURE(base.size() >= step.size(),
            "byte size of step need <= base");
}

FIELD::STEP::~STEP()
{ }

FIELD::SEED * FIELD::STEP::clone() const
{
    return new FIELD::STEP(*this);
}

void FIELD::STEP::reset()
{
    current = base;
    i=0;
}

const BYTES & FIELD::STEP::next()
{
    if(0 == i) {
        i++;
        current = base;
    } else {
        i = (i+1)%count;

        if(is_increase) {
            current = bytes_add(current, step);
        } else {
            current = bytes_sub(current, step);
        }
    }

    size_fit(current, size_of_bit);

    return current;
}


FIELD::RANDOM::RANDOM(unsigned int size_of_bit, unsigned int count)
:SEED(size_of_bit,count), bytes((size_of_bit+7)/8, 0)
{ }

FIELD::RANDOM::~RANDOM()
{ }

FIELD::SEED * FIELD::RANDOM::clone() const
{
    return new FIELD::RANDOM(*this);
}

void FIELD::RANDOM::reset()
{ }

const BYTES & FIELD::RANDOM::next()
{
    int size = size_of_bit;

    while(size>0) {
        UINT8 u8 = RAND();
        bytes.push_back(u8);
        size -= 8;
    }

    size_fit(bytes, size_of_bit);

    return bytes;
}
