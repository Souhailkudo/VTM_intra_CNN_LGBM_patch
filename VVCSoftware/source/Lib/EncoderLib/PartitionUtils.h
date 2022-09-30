//
// Created by altissie on 14/10/2019.
//

#ifndef NEXTSOFTWARE_PARTITIONUTILS_H
#define NEXTSOFTWARE_PARTITIONUTILS_H

#include <vector>
#include <string>
#include "EncModeCtrl.h"



std::vector<float> string_to_vector(std::string vector_pred);
int getInverseSplit(EncTestModeType mode);
std::string transformVectorString(std::vector<float> currPixelsVector);
std::vector<float> getCurrCuLuma(CodingStructure *&tempCS);
std::vector<float> getCtusLumaInter(Picture * pic, Picture * picRef0, Picture * picRef1, int x, int y, Mv* mvUni);
std::vector<float> getCtusLumaInterBenchmark(Picture * pic, int x, int y) ;
std::vector<std::vector<float>> getCurrCuChroma(CodingStructure *&tempCS);
EncTestModeType getSplit(int i);
uint8_t * splitChoiceML(int qp, int width, int height, int x, int y, std::vector<float> * proba);
uint8_t * splitChoiceML_inter(int qp, int width, int height, int x, int y, std::vector<float> * proba);
int * predictSplit(std::vector<float> * pred_vector, int x, int y, int widthCU, int heightCU);
uint8_t * splitChoice(int * mean_split);
int * predictSplitIntra(std::vector<float> * pred_vector, int x, int y, int widthCU, int heightCU);


#endif //NEXTSOFTWARE_PARTITIONUTILS_H
