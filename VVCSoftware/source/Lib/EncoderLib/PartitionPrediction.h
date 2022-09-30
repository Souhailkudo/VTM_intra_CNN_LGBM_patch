//
// Created by altissie on 18/10/2019.
//

#ifndef NEXTSOFTWARE_PARTITIONPREDICTION_H
#define NEXTSOFTWARE_PARTITIONPREDICTION_H


#include <fdeep/fdeep.hpp>
#include <iostream>
#include <fstream>
#include <LightGBM/prediction_early_stop.h>
#include <LightGBM/boosting.h>


typedef std::pair<int, int> partsize ;

class PartitionPrediction {
public:
    PartitionPrediction(std::string model_str, int qp, bool intra);
    ~PartitionPrediction();

    void initializeModels(std::string mode, std::string modelFolder);
    void predict_once(double* input, double* output, partsize size);
    void predict_once_inter(double* input, double* output, partsize size);

    //std::unique_ptr<fdeep::model> get_model() {return std::move(model);};
    int getQp() {return qp;};
    std::unique_ptr<fdeep::model> model;

private:
    int qp;

    std::map<partsize, LightGBM::Boosting*> models ;
    std::vector<partsize> partsize_list;
};


#endif //NEXTSOFTWARE_PARTITIONPREDICTION_H
