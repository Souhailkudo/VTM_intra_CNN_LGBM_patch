//
// Created by altissie on 02/09/2019.
//

#ifndef NEXTSOFTWARE_PARTITIONPARAM_H
#define NEXTSOFTWARE_PARTITIONPARAM_H



#include <iostream>

class PartitionParam {
public:
    PartitionParam(int ctuSize, int mttDataSize, bool writePartition, bool readPartition, bool predictPartition, bool predictPartitionInter);

    bool is_writePartition() const {return writePartition;};
    bool is_readPartition() const {return readPartition;};
    bool is_predictPartition() const {return predictPartition;};
    bool is_predictPartitionInter() const {return predictPartitionInter;};

    u_int16_t mttDataSize; // Size of one maximum value of split decision
    u_int16_t ctuSize; // Size of a CTU

private:
    bool writePartition; // True : write partition into dat file
    bool readPartition; // True : load dat file to reinject best partition
    bool predictPartition; // True : predict the partition with the CNN+ML model we loaded in intra encoding
    bool predictPartitionInter; // True : predict the partition with the CNN+ML model we loaded in inter encoding
};


#endif //NEXTSOFTWARE_PARTITIONPARAM_H
