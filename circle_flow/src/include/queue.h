
#ifndef QUEUE_H_
#define QUEUE_H_

#include "type_ext.h"

class QUEUE{
public:
    static UINT8 num_of_q(UINT8 unit, UINT8 port);

    QUEUE(UINT8 unit, UINT8 port);
    QUEUE(UINT8 unit, UINT8 port, UINT8 cosq);
    ~QUEUE();

    void shaper(const RATE &rate);

    void stop(void);
    void start(void);

    COUNTER counter (bool clear) const;

    bool is_empty();

    struct INFO {
        UINT8 unit;
        UINT8 port;
        UINT8 cosq;
    };

    INFO info() const;

private:

    QUEUE (const QUEUE &ref);
    QUEUE & operator= (const QUEUE &ref);

    class IMPL;
    IMPL *pimpl;

};


#endif /* QUEUE_H_ */
