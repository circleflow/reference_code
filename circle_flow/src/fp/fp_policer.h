
#ifndef FP_POLICER_H_
#define FP_POLICER_H_

class FP_POLICER {
public:
    FP_POLICER(int unit);
    ~FP_POLICER();

    unsigned int max(void);

    void attach(int eid);
    void set(int num_of_pkt);   //0: pass all, <0: block all

private:
    int unit, eid, pol_id;
};


#endif /* FP_POLICER_H_ */
