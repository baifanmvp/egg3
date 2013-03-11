#include "eggClusterCommon.h"

PUBLIC index_t eggChunkHand_find_spanpoint(HEGGCHUNKHAND hHandSet, count_t nHandCnt, SPANPOINT curPoint)
{
    index_t n_hand_idx = 0;
    while(n_hand_idx != nHandCnt)
    {
        if(hHandSet[n_hand_idx].start<=curPoint && curPoint <= hHandSet[n_hand_idx].end)
        {
            return n_hand_idx;
        }
        n_hand_idx++;
    }
    return EGG_SPANPOINT_NOT_FIND;
}
