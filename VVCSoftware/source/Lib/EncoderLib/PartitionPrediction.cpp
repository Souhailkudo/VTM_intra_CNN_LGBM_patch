//
// Created by altissie on 18/10/2019.
//

#include "PartitionPrediction.h"
PartitionPrediction::PartitionPrediction(std::string model_str, int qp, bool intra){
  if(intra){
    this->partsize_list = {
            std::make_pair(4, 8),
            std::make_pair(64, 64),
            std::make_pair(8, 32),
            std::make_pair(16, 8),
            std::make_pair(8, 8),
            std::make_pair(4, 32),
            std::make_pair(32, 32),
            std::make_pair(8, 4),
            std::make_pair(16, 16),
            std::make_pair(32, 8),
            std::make_pair(16, 4),
            std::make_pair(32, 4),
            std::make_pair(4, 16),
            std::make_pair(32, 16),
            std::make_pair(8, 16),
            std::make_pair(16, 32)
    };
  }
  else{
    this->partsize_list = {
      std::make_pair(128, 128),
      std::make_pair(128, 64),
      std::make_pair(64, 128),
      std::make_pair(128, 32),
      std::make_pair(32, 128),
      std::make_pair(128, 16),
      std::make_pair(16, 128),
      std::make_pair(16, 64),
      std::make_pair(32, 64),
      std::make_pair(4, 64),
      std::make_pair(64, 16),
      std::make_pair(64, 32),
      std::make_pair(64, 4),
      std::make_pair(8, 64),
      std::make_pair(4, 8),
      std::make_pair(64, 64),
      std::make_pair(64, 8),
      std::make_pair(8, 32),
      std::make_pair(16, 8),
      std::make_pair(8, 8),
      std::make_pair(4, 32),
      std::make_pair(32, 32),
      std::make_pair(8, 4),
      std::make_pair(16, 16),
      std::make_pair(32, 8),
      std::make_pair(16, 4),
      std::make_pair(32, 4),
      std::make_pair(4, 16),
      std::make_pair(32, 16),
      std::make_pair(8, 16),
      std::make_pair(16, 32)
    };
  }


  this->qp = qp;
  //this->model = model;
  this->model = std::make_unique<fdeep::model>(fdeep::load_model(model_str));
  //this->model = fdeep::load_model(model_str);


}


PartitionPrediction::~PartitionPrediction(){
  //free(this->model);
}

void PartitionPrediction::initializeModels(std::string mode, std::string modelFolder){
  for (int i = 0; i < partsize_list.size(); ++i) {
    //load model file
    std::ifstream model_file ;
//    std::string filename = "MODEL_DIRECTORY_HERE/ML_model/"+mode+"/lgbm_"+std::to_string(partsize_list[i].first)+"x"+std::to_string(partsize_list[i].second)+".txt";
    std::string filename = modelFolder+"/ML_model/"+mode+"/lgbm_"+std::to_string(partsize_list[i].first)+"x"+std::to_string(partsize_list[i].second)+".txt";

    const char * charFilename = filename.c_str();
    model_file.open(filename, std::ifstream::in) ;
    std::string model_content((std::istreambuf_iterator<char>(model_file)), std::istreambuf_iterator<char>());
    unsigned long size_t = model_content.length() ;
    const char *cstr = model_content.c_str();
    models[partsize_list[i]] = LightGBM::Boosting::CreateBoosting("gbdt",charFilename);
    models[partsize_list[i]]->LoadModelFromString(cstr, size_t) ;
    model_file.close() ;
  }
}

void PartitionPrediction::predict_once(double* input, double* output, partsize size){
  LightGBM::PredictionEarlyStopConfig test;
  std::string classif = "multiclass";
  if((size.first==64&&size.second==64) || (size.first==8&&size.second==4) || (size.first==4&&size.second==8)){
    classif = "binary";
  }
  const LightGBM::PredictionEarlyStopInstance early_stop = LightGBM::CreatePredictionEarlyStopInstance(classif,test);
  this->models[size]->Predict(input, output, &early_stop) ;
}

void PartitionPrediction::predict_once_inter(double* input, double* output, partsize size){
  LightGBM::PredictionEarlyStopConfig test;
  std::string classif = "multiclass";
  if(/*(size.first==128&&size.second==128) ||*/ (size.first==8&&size.second==4) || (size.first==4&&size.second==8)){
    classif = "binary";
  }
  const LightGBM::PredictionEarlyStopInstance early_stop = LightGBM::CreatePredictionEarlyStopInstance(classif,test);
  this->models[size]->Predict(input, output, &early_stop) ;
}
