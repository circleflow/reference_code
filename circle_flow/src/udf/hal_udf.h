
#ifndef FP_UDF_H_
#define FP_UDF_H_

#include "qual.h"
#include "hal.h"

namespace CIRCLE_FLOW {

    void hal_udf_qset(int unit, void *qset);
    void hal_udf_qualify(int unit, int eid, const QUAL::MATCH &match);

    typedef struct {
        UINT8 num_of_block; //number of UDF visible in FP qual
        UINT8 len_of_block; //length of UDF, in byte
        UINT8 num_of_chunk; //total number of chunks in all block
        UINT8 len_of_chunk; //length of chunk, in byte

        UINT8 max_offset;   //pkt content window which FP can see

        typedef int (*chunk_offset)(int offset);
        chunk_offset chunk_aligned_offset;  //adjust to offset of pre-defined chunk

    } UDF_HW_SPECS;

    const UDF_HW_SPECS & hal_udf_specs(UINT8 unit=DEFAULT_UNIT);

    void hal_udf_chunk_set(UINT8 unit, UINT8 chunk_idx, UINT8 offset);
}

#endif /* FP_UDF_H_ */
