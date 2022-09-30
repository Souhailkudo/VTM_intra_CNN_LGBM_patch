//
// Created by altissie on 02/09/2019.
//

#ifndef NEXTSOFTWARE_PARTITIONGLOBALMANAGER_H
#define NEXTSOFTWARE_PARTITIONGLOBALMANAGER_H

#include <fdeep/model.hpp>
#include "PartitionManager.h"
#include "PartitionPrediction.h"
//#include "PartitionOccurences.h"
//#include "PartitionPrediction.h"

extern PartitionManager * store_partition;
extern PartitionManager * load_partition;
extern PartitionParam * param_partition;
//extern PartitionOccurences * occur_partition;
extern PartitionPrediction * predict_partition;
extern PartitionPrediction * predict_partitionInter;
extern float time_cnn;
//extern int valPred;
//extern std::vector<std::vector<float>> * pred_vector_chroma;

//extern float time_effnet;


#endif //NEXTSOFTWARE_PARTITIONGLOBALMANAGER_H
