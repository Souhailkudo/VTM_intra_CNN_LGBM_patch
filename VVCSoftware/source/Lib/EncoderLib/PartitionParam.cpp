//
// Created by altissie on 02/09/2019.
//

#include "PartitionParam.h"


PartitionParam::PartitionParam(int ctuSize, int mttDataSize, bool writePartition, bool readPartition, bool predictPartition, bool predictPartitionInter){
    this->ctuSize = u_int16_t(ctuSize);
    this->mttDataSize = u_int16_t(mttDataSize);
    this->writePartition = writePartition;
    this->readPartition = readPartition;
    this->predictPartition = predictPartition;
    this->predictPartitionInter = predictPartitionInter;
}
