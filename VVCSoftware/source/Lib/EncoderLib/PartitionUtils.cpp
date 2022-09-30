//
// Created by altissie on 14/10/2019.
//
#include "PartitionUtils.h"
#include "PartitionGlobalManager.h"


using namespace std;

std::vector<float> string_to_vector(string vector_pred) {
  std::string delimiter = " ";
  int i = 0;
  size_t pos = 0;
  std::vector<float> float_pred;
  while ((pos = vector_pred.find(delimiter)) < std::string::npos) {
    float_pred.push_back(stof(vector_pred.substr(0, pos)));
    vector_pred.erase(0, pos + delimiter.length());
    i++;
  }
  float_pred.push_back(stof(vector_pred));
  return float_pred;
}

int getInverseSplit(EncTestModeType mode) {
  switch (mode) {
    case ETM_SPLIT_QT :
      return 0;
    case ETM_SPLIT_BT_H :
      return 1;
    case ETM_SPLIT_BT_V :
      return 2;
    case ETM_SPLIT_TT_H :
      return 3;
    case ETM_SPLIT_TT_V :
      return 4;
    default:
      return -1;
  }
}




string transformVectorString(std::vector<float> currPixelsVector){
  string currPixels = "";
  for(int i =0; i<currPixelsVector.size();i++){
    currPixels += to_string(currPixelsVector.at(i)) + " ";
  }
  return currPixels;
}


std::vector<float> getCurrCuLuma(CodingStructure *&tempCS) {
  std::vector<float> currPixels;
  int max = 0;
  int min = 255;
  for (int i = 0; i < tempCS->area.blocks[0].height + 4; i++) {
    for (int j = 0; j < tempCS->area.blocks[0].width + 4; j++) {
      if ((tempCS->area.blocks[0].y + i) - 4 < 0 || (j + tempCS->area.blocks[0].x) - 4 < 0 ||
          tempCS->area.blocks[0].y + i - 4 >= tempCS->picture->blocks[0].height ||
          j + tempCS->area.blocks[0].x - 4 >= tempCS->picture->blocks[0].width) {
        currPixels.push_back(128);
        if(max<128){
          max = 128;
        }
        if(min>128){
          min = 128;
        }
      } else {
        int pix = tempCS->picture->m_bufs[2].bufs[0].buf[(
                ((tempCS->area.blocks[0].y + i - 4) * tempCS->picture->blocks[0].width) + j - 4 +
                tempCS->area.blocks[0].x)] / 4;
        currPixels.push_back(pix);
        if (max < pix) {
          max = pix;
        }
        if (min > pix) {
          min = pix;
        }
      }
    }
  }
  //To normalize pixels between [-1,1]
  for(int i = 0 ; i<currPixels.size();i++){
    currPixels.at(i) = currPixels.at(i)/255;
    //currPixels.at(i) = 2*(currPixels.at(i)-min)/(max-min)-1;
  }
  return currPixels;
}

std::vector<float> getCtusLumaInter(Picture * pic, Picture * picRef0, Picture * picRef1, int x, int y, Mv* mvUni) {
  std::vector<float> currPixels;
  int width = 128;
  int height = 128;
  Picture *picUsed [3]={pic, picRef0, picRef1};
  //Picture * picUsed = nullptr;
  int x_modified = x;
  int y_modified = y;
  for(int picIdx = 0; picIdx<3; picIdx++){
    if(picIdx != 0){
      x_modified = x + mvUni[picIdx-1].getHor();
      y_modified = y + mvUni[picIdx-1].getVer();
    }
    for (int i = 0; i < height; i++) {
      for (int j = 0; j < width; j++) {
        if (y_modified + i < 0 || j + x_modified < 0 || y_modified + i >= picUsed[picIdx]->blocks[0].height || j + x_modified >= picUsed[picIdx]->blocks[0].width) {
          currPixels.push_back(128);
        } else {
          int pix = picUsed[picIdx]->m_bufs[2].bufs[0].buf[(((y_modified + i) * picUsed[picIdx]->blocks[0].width) + j + x_modified)] / 4;
          currPixels.push_back(pix);
        }
      }
    }
  }

  //To normalize pixels between [-1,1]
  //std::vector<float> currPixels2;
  for(int i = 0 ; i<currPixels.size();i++){
    /*if(i%(height*width)==0){
      std::cout<<std::endl<<std::endl<<std::endl<<std::endl<<std::endl<<std::endl;
    }*/
    currPixels.at(i) = currPixels.at(i)/255;
    /*currPixels2.push_back(currPixels.at(i)/255);
    currPixels2.push_back(currPixels.at(i+currPixels.size()/3)/255);
    currPixels2.push_back(currPixels.at(i+(currPixels.size()/3)*2)/255);*/
    //std::cout<<currPixels.at(i)<<", ";
    //currPixels.at(i) = 2*(currPixels.at(i)-min)/(max-min)-1;
  }
  //std::cout<<std::endl<<std::endl<<std::endl<<std::endl<<std::endl<<std::endl<<std::endl<<std::endl<<std::endl<<std::endl<<std::endl<<std::endl;
  return currPixels;
}

std::vector<float> getCtusLumaInterBenchmark(Picture * pic, int x, int y) {
  std::vector<float> currPixels;
  int width = 128;
  int height = 128;
//  Picture *picUsed [3]={pic, picRef0, picRef1};
  //Picture * picUsed = nullptr;
  for (int i = 0; i < height; i++){
      for (int j = 0; j < width; j++){
        int pix = pic->m_bufs[2].bufs[0].buf[(((y + i) * pic->blocks[0].width) + j + x)] / 4;
        currPixels.push_back(pix);
      }
  }
  return currPixels;
}

std::vector<std::vector<float>> getCurrCuChroma(CodingStructure *&tempCS) {
  std::vector<std::vector<float>> currPixels;
  std::vector<float> chromaU;
  std::vector<float> chromaV;
  for(int x=0;x<(tempCS->area.blocks[1].height + 2 +tempCS->area.blocks[1].width + 2)*3;x++){
    chromaU.push_back(128);
    chromaV.push_back(128);
  }
  for (int i = 0; i < tempCS->area.blocks[1].height + 1; i++) {
    for(int x=0;x<3;x++){
      chromaU.push_back(128);
      chromaV.push_back(128);
    }
    for (int j = 0; j < tempCS->area.blocks[1].width + 1; j++) {
      if(j==0){
        if((tempCS->area.blocks[1].y + i) - 1 < 0 || (j + tempCS->area.blocks[1].x) - 1 < 0){
          chromaU.push_back(128);
          chromaV.push_back(128);
        }
        else{
          int pixU = tempCS->picture->m_bufs[2].bufs[1].buf[(
                  ((tempCS->area.blocks[1].y + i - 1) * tempCS->picture->blocks[1].width) + j - 1+
                  tempCS->area.blocks[1].x)] / 4;
          chromaU.push_back(pixU);
          int pixV = tempCS->picture->m_bufs[2].bufs[2].buf[(
                  ((tempCS->area.blocks[2].y + i - 1) * tempCS->picture->blocks[2].width) + j - 1 +
                  tempCS->area.blocks[2].x)] / 4;
          chromaV.push_back(pixV);
        }
      }
      else if ((tempCS->area.blocks[1].y + i) - 1 < 0 || (j + tempCS->area.blocks[1].x) - 1 < 0 ||
               tempCS->area.blocks[1].y + i > tempCS->picture->blocks[1].height ||
               j + tempCS->area.blocks[1].x > tempCS->picture->blocks[1].width) {
        chromaU.push_back(128);
        chromaU.push_back(128);
        chromaV.push_back(128);
        chromaV.push_back(128);
      } else {
        int pixU = tempCS->picture->m_bufs[2].bufs[1].buf[(
                ((tempCS->area.blocks[1].y + i - 1) * tempCS->picture->blocks[1].width) + j - 1 +
                tempCS->area.blocks[1].x)] / 4;
        chromaU.push_back(pixU);
        chromaU.push_back(pixU);
        int pixV = tempCS->picture->m_bufs[2].bufs[2].buf[(
                ((tempCS->area.blocks[2].y + i - 1) * tempCS->picture->blocks[2].width) + j - 1 +
                tempCS->area.blocks[2].x)] / 4;
        chromaV.push_back(pixV);
        chromaV.push_back(pixV);
      }
    }
    if(i!=0){
      for(int x=(tempCS->area.blocks[1].height + 2 + tempCS->area.blocks[1].width + 2) * ((i+1)*2) ; x<(tempCS->area.blocks[1].height + 2 + tempCS->area.blocks[1].width + 2) * (((i+1)*2)+1) ;x++){
        chromaU.push_back(chromaU[x]);
        chromaV.push_back(chromaV[x]);
      }
    }

  }
  //To normalize pixels between [-1,1]
  for(int i = 0 ; i<chromaU.size();i++){
    chromaU.at(i) = chromaU.at(i)/255;
    chromaV.at(i) = chromaV.at(i)/255;
    //currPixels.at(i) = 2*(currPixels.at(i)-min)/(max-min)-1;
  }
  currPixels.push_back(chromaU);
  currPixels.push_back(chromaV);
  return currPixels;
}

//Get the split depending on int value
EncTestModeType getSplit(int i) {
  switch (i) {
    case 0 :
      return ETM_SPLIT_QT;
    case 1 :
      return ETM_SPLIT_BT_H;
    case 2 :
      return ETM_SPLIT_BT_V;
    case 3 :
      return ETM_SPLIT_TT_H;
    case 4 :
      return ETM_SPLIT_TT_V;
  }
  return ETM_INVALID;
}

void extract_from_vector(vector<float> * full_vector, double* sub_vector, int x, int y, int w, int h) {
  x/=2 ;
  y/=2 ;
  w = w/2 -1 ;
  h = h/2 - 1 ;

  int k = 0;
  int start = y + 31 * (x / 2);
  for (int i = 0; i < h / 2; ++i){
    for (int j = 0; j < w; ++j)
      sub_vector[k++] = full_vector->at(start++);
    start += 31 - w;
  }
  if ( x+h==31 ) {
    start = 465 + y / 2;
    for (int i = 0; i < w / 2; ++i)
      sub_vector[k++] = full_vector->at(start++);
  }
  else{
    start++ ;
    for (int i = 0; i < w/2; ++i) {
      sub_vector[k++] = full_vector->at(start);
      start += 2;
    }
  }
}

void extract_from_vector_inter(vector<float> * full_vector, double* sub_vector, int x, int y, int w, int h) {
  x/=2 ;
  y/=2 ;

  w = w/2 -1 ;
  h = h/2 - 1 ;
  int k = 0;
  int start = y + 63 * (x / 2);
  for (int i = 0; i < h / 2; ++i){
    for (int j = 0; j < w; ++j)
      sub_vector[k++] = full_vector->at(start++);
    start += 63 - w;
  }
  if ( x+h==63 ) {
    start = 1953 + y / 2;
    for (int i = 0; i < w / 2; ++i)
      sub_vector[k++] = full_vector->at(start++);
  }
  else{
    start++ ;
    for (int i = 0; i < w/2; ++i) {
      sub_vector[k++] = full_vector->at(start);
      start += 2;
    }
  }
}

uint8_t * splitChoiceML(int qp, int width, int height, int x, int y, vector<float> * proba) {

  std::map<partsize, std::map<string,int>> resClass = {
          {std::make_pair(4, 8),{{"BTV",-1}, {"NS",-1}}},
          {std::make_pair(64, 64),{{"NS",-1}, {"QT",-1}}},
          {std::make_pair(8, 32),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTV",-1}}},
          {std::make_pair(16, 8),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}}},
          {std::make_pair(8, 8),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}}},
          {std::make_pair(4, 32),{{"BTV",-1}, {"NS",-1}, {"TTV",-1}}},
          {std::make_pair(32, 32),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"QT",-1}, {"TTH",-1}, {"TTV", -1}}},
          {std::make_pair(8, 4),{{"BTH",-1}, {"NS",-1}}},
          {std::make_pair(16, 16),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"QT",-1}, {"TTH",-1}, {"TTV", -1}}},
          {std::make_pair(32, 8),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}}},
          {std::make_pair(16, 4),{{"BTH",-1} ,{"NS",-1}, {"TTH",-1}}},
          {std::make_pair(32, 4),{{"BTH",-1} ,{"NS",-1}, {"TTH",-1}}},
          {std::make_pair(4, 16),{{"BTV",-1}, {"NS",-1}, {"TTV",-1}}},
          {std::make_pair(32, 16),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}, {"TTV", -1}}},
          {std::make_pair(8, 16),{{"BTH",-1}, {"BTV", -1}, {"NS",-1}, {"TTV", -1}}},
          {std::make_pair(16, 32),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}, {"TTV",-1}}}
  } ;



  std::map<string, int> conv = {{"QT",0},{"BTH",1},{"BTV",2},{"TTH",3},{"TTV",4},{"NS",5}};
  double input[481] ;
  double *output = (double *) malloc(sizeof(double) * 6);

  uint8_t *splitDecision = new uint8_t[6]; // 0 = QT, 1 = BTH, 2 = BTV, 3 = TTH, 4 = TTV, 5 = NS
  for (int i = 0; i < 6; i++) {
    splitDecision[i] = 0;
  }



  //int w = width/2 -1 ;
  //int h = height/2 - 1 ;
  int vector_length = (width/4-1)*(height/4)+(width/4)*(height/4-1);
  double * subvector = (double * ) malloc(sizeof(double) * vector_length);
  extract_from_vector(proba, subvector, y, x, width, height);

  input[0] = qp;
  for(int i=0;i<vector_length;i++){
    input[i+1] = subvector[i];
  }


  predict_partition->predict_once(input, output, make_pair(height, width));

  vector<string> v;
  for(map<string,int>::iterator it = resClass[std::make_pair(height, width)].begin(); it != resClass[std::make_pair(height, width)].end(); ++it) {
    v.push_back(it->first);
  }

  int top1 = -1;
  int top2 = -1;
  int top3 = -1;
  //int top4 = -1;
  double valTop1 = 0;
  double valTop2 = 0;
  double valTop3 = 0;
  //double valTop4 = 0;

  for (int i = 0; i < v.size(); i++){
    //std::cout<<v[i]<<" : "<<output[i]<<" / ";
    if(v.size() == 2 && v[0] != "NS" && i == 1){
      if(valTop1<1-output[i-1]){
        top2 = top1;
        top1 = i;
        valTop2 = valTop1;
        valTop1 = 1-output[i-1];
      }
      else {
        top2 = i;
        valTop2 = 1-output[i-1];
      }
    }
    else if(v[0] == "NS"){
      if(i==0){
        valTop1 = 1-output[i];
        top1 = i;
      }
      else{
        if(valTop1<output[i-1]){
          top2 = top1;
          top1 = i;
          valTop2 = valTop1;
          valTop1 = output[i-1];
        }
        else {
          top2 = i;
          valTop2 = output[i-1];
        }
      }
    }
    else if(valTop1<output[i]){
      /*top4 = top3;*/
      top3 = top2;
      top2 = top1;
      top1 = i;
      /*valTop4 = valTop3;*/
      valTop3 = valTop2;
      valTop2 = valTop1;
      valTop1 = output[i];
    }
    else if(valTop2<output[i]){
      /*top4 = top3;*/
      top3 = top2;
      top2 = i;
      /*valTop4 = valTop3;*/
      valTop3 = valTop2;
      valTop2 = output[i];
    }
    else if(valTop3 < output[i]){
      //top4 = top3;
      top3 = i;
      //valTop4 = valTop3;
      valTop3 = output[i];
    }
    /*else if(valTop4 < output[i]){
      top4 = i;
      valTop4 = output[i];
    }*/
  }
  //std::cout<<std::endl;

  //To make top1 with one split at least
  /*if(v[top1] == "NS"){
    resClass[std::make_pair(height, width)][v[top2]]=1;
  }
  else{
    resClass[std::make_pair(height, width)][v[top1]]=1;
  }*/

  //Classical top 3 choice
  resClass[std::make_pair(height, width)][v[top1]]=1;
  //std::cout<<v.at(top1)<<" "<<resClass[std::make_pair(height, width)][v[top1]]<<" / ";
  resClass[std::make_pair(height, width)][v[top2]]=2;

  // comment this to make it top1 and top2 only
  if (top3!=-1){
//    resClass[std::make_pair(height, width)][v[top3]]=3;
      top3 *= 1 ;
  }
  /*if(top4!=-1){
    resClass[std::make_pair(height, width)][v[top4]]=4;
  }*/

  for(map<string,int>::iterator it = resClass[std::make_pair(height, width)].begin(); it != resClass[std::make_pair(height, width)].end(); ++it) {
    if(it->second != -1 ){
      splitDecision[conv[it->first]] = 1;
    }
  }

  delete [] output;
  delete [] subvector;
  return splitDecision;
}



uint8_t * splitChoiceML_inter(int qp, int width, int height, int x, int y, vector<float> * proba) {

//  std::map<partsize, std::map<string,int>> resClass = {
//          {std::make_pair(128, 128),{{"NS",-1}, {"QT",-1}}},
//          {std::make_pair(16, 64),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}, {"TTV",-1}}},
//          {std::make_pair(32, 64),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}, {"TTV",-1}}},
//          {std::make_pair(4, 64),{{"BTV",-1}, {"NS",-1}, {"TTV",-1}}},
//          {std::make_pair(64, 16),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}, {"TTV",-1}}},
//          {std::make_pair(64, 32),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}, {"TTV",-1}}},
//          {std::make_pair(64, 4),{{"BTH",-1}, {"NS",-1}, {"TTH",-1}}},
//          {std::make_pair(8, 64),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTV",-1}}},
//          {std::make_pair(4, 8),{{"BTV",-1}, {"NS",-1}}},
//          {std::make_pair(64, 64),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"QT",-1}, {"TTH",-1}, {"TTV", -1}}},
//          {std::make_pair(64, 8),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}}},
//          {std::make_pair(8, 32),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTV",-1}}},
//          {std::make_pair(16, 8),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}}},
//          {std::make_pair(8, 8),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}}},
//          {std::make_pair(4, 32),{{"BTV",-1}, {"NS",-1}, {"TTV",-1}}},
//          {std::make_pair(32, 32),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"QT",-1}, {"TTH",-1}, {"TTV", -1}}},
//          {std::make_pair(8, 4),{{"BTH",-1}, {"NS",-1}}},
//          {std::make_pair(16, 16),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"QT",-1}, {"TTH",-1}, {"TTV", -1}}},
//          {std::make_pair(32, 8),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}}},
//          {std::make_pair(16, 4),{{"BTH",-1} ,{"NS",-1}, {"TTH",-1}}},
//          {std::make_pair(32, 4),{{"BTH",-1} ,{"NS",-1}, {"TTH",-1}}},
//          {std::make_pair(4, 16),{{"BTV",-1}, {"NS",-1}, {"TTV",-1}}},
//          {std::make_pair(32, 16),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}, {"TTV", -1}}},
//          {std::make_pair(8, 16),{{"BTH",-1}, {"BTV", -1}, {"NS",-1}, {"TTV", -1}}},
//          {std::make_pair(16, 32),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}, {"TTV",-1}}}
//  } ;

  std::map<partsize, std::map<string,int>> resClass = {
    {std::make_pair(128, 128),{{"NS",-1}, {"QT",-1}}},
    {std::make_pair(128, 64),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}, {"TTV",-1}}},
    {std::make_pair(64, 128),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}, {"TTV",-1}}},
    {std::make_pair(128, 32),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}, {"TTV",-1}}},
    {std::make_pair(32, 128),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}, {"TTV",-1}}},
    {std::make_pair(128, 16),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}, {"TTV",-1}}},
    {std::make_pair(16, 128),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}, {"TTV",-1}}},
    {std::make_pair(16, 64),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}, {"TTV",-1}}},
    {std::make_pair(32, 64),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}, {"TTV",-1}}},
    {std::make_pair(4, 64),{{"BTV",-1}, {"NS",-1}, {"TTV",-1}}},
    {std::make_pair(64, 16),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}, {"TTV",-1}}},
    {std::make_pair(64, 32),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}, {"TTV",-1}}},
    {std::make_pair(64, 4),{{"BTH",-1}, {"NS",-1}, {"TTH",-1}}},
    {std::make_pair(8, 64),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTV",-1}}},
    {std::make_pair(4, 8),{{"BTV",-1}, {"NS",-1}}},
    {std::make_pair(64, 64),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"QT",-1}, {"TTH",-1}, {"TTV", -1}}},
    {std::make_pair(64, 8),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}}},
    {std::make_pair(8, 32),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTV",-1}}},
    {std::make_pair(16, 8),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}}},
    {std::make_pair(8, 8),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}}},
    {std::make_pair(4, 32),{{"BTV",-1}, {"NS",-1}, {"TTV",-1}}},
    {std::make_pair(32, 32),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"QT",-1}, {"TTH",-1}, {"TTV", -1}}},
    {std::make_pair(8, 4),{{"BTH",-1}, {"NS",-1}}},
    {std::make_pair(16, 16),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"QT",-1}, {"TTH",-1}, {"TTV", -1}}},
    {std::make_pair(32, 8),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}}},
    {std::make_pair(16, 4),{{"BTH",-1} ,{"NS",-1}, {"TTH",-1}}},
    {std::make_pair(32, 4),{{"BTH",-1} ,{"NS",-1}, {"TTH",-1}}},
    {std::make_pair(4, 16),{{"BTV",-1}, {"NS",-1}, {"TTV",-1}}},
    {std::make_pair(32, 16),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}, {"TTV", -1}}},
    {std::make_pair(8, 16),{{"BTH",-1}, {"BTV", -1}, {"NS",-1}, {"TTV", -1}}},
    {std::make_pair(16, 32),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}, {"TTV",-1}}}
  } ;




  std::map<string, int> conv = {{"QT",0},{"BTH",1},{"BTV",2},{"TTH",3},{"TTV",4},{"NS",5}};
  double input[1985];
  double *output = (double *) malloc(sizeof(double) * 6);

  uint8_t *splitDecision = new uint8_t[6]; // 0 = QT, 1 = BTH, 2 = BTV, 3 = TTH, 4 = TTV, 5 = NS
  for (int i = 0; i < 6; i++) {
    splitDecision[i] = 0;
  }



  //int w = width/2 -1 ;
  //int h = height/2 - 1 ;
  int vector_length = (width/4-1)*(height/4)+(width/4)*(height/4-1);
  double * subvector = (double * ) malloc(sizeof(double) * vector_length);
  extract_from_vector_inter(proba, subvector, y, x, width, height);

  input[0] = qp;
  for(int i=0;i<vector_length;i++){
    input[i+1] = subvector[i];
  }


  predict_partitionInter->predict_once_inter(input, output, make_pair(height, width));

  vector<string> v;
  for(map<string,int>::iterator it = resClass[std::make_pair(height, width)].begin(); it != resClass[std::make_pair(height, width)].end(); ++it) {
    v.push_back(it->first);
  }

  int top1 = -1;
  int top2 = -1;
  int top3 = -1;
  //int top4 = -1;
  double valTop1 = 0;
  double valTop2 = 0;
  double valTop3 = 0;
  //double valTop4 = 0;

  for (int i = 0; i < v.size(); i++){
    //std::cout<<v[i]<<" : "<<output[i]<<" / ";
    if(v.size() == 2 && v[0] != "NS" && i == 1){
      if(valTop1<1-output[i-1]){
        top2 = top1;
        top1 = i;
        valTop2 = valTop1;
        valTop1 = 1-output[i-1];
      }
      else {
        top2 = i;
        valTop2 = 1-output[i-1];
      }
    }
    else if(v[0] == "NS"){
      if(i==0){
        valTop1 = 1-output[i];
        top1 = i;
      }
      else{
        if(valTop1<output[i-1]){
          top2 = top1;
          top1 = i;
          valTop2 = valTop1;
          valTop1 = output[i-1];
        }
        else {
          top2 = i;
          valTop2 = output[i-1];
        }
      }
    }
    else if(valTop1<output[i]){
      /*top4 = top3;*/
      top3 = top2;
      top2 = top1;
      top1 = i;
      /*valTop4 = valTop3;*/
      valTop3 = valTop2;
      valTop2 = valTop1;
      valTop1 = output[i];
    }
    else if(valTop2<output[i]){
      /*top4 = top3;*/
      top3 = top2;
      top2 = i;
      /*valTop4 = valTop3;*/
      valTop3 = valTop2;
      valTop2 = output[i];
    }
    else if(valTop3 < output[i]){
      //top4 = top3;
      top3 = i;
      //valTop4 = valTop3;
      valTop3 = output[i];
    }
    /*else if(valTop4 < output[i]){
      top4 = i;
      valTop4 = output[i];
    }*/
  }
  //std::cout<<std::endl;

  //To make top1 with one split at least
  /*if(v[top1] == "NS"){
    resClass[std::make_pair(height, width)][v[top2]]=1;
  }
  else{
    resClass[std::make_pair(height, width)][v[top1]]=1;
  }*/

  //Classical top 3 choice
  resClass[std::make_pair(height, width)][v[top1]]=1;
  //std::cout<<v.at(top1)<<" "<<resClass[std::make_pair(height, width)][v[top1]]<<" / ";
  resClass[std::make_pair(height, width)][v[top2]]=2;
  if (top3!=-1){
    resClass[std::make_pair(height, width)][v[top3]]=3;
  }
  /*if(top4!=-1){
    resClass[std::make_pair(height, width)][v[top4]]=4;
  }*/

  for(map<string,int>::iterator it = resClass[std::make_pair(height, width)].begin(); it != resClass[std::make_pair(height, width)].end(); ++it) {
    if(it->second != -1 ){
      splitDecision[conv[it->first]] = 1;
    }
  }

  delete [] output;
  delete [] subvector;
  return splitDecision;
}

uint8_t * splitChoiceML_inter2(int qp, int width, int height, int x, int y, vector<float> * proba) {

  std::map<partsize, std::map<string,int>> resClass = {
    {std::make_pair(128, 128),{{"NS",-1}, {"QT",-1}}},
    {std::make_pair(16, 64),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}, {"TTV",-1}}},
    {std::make_pair(32, 64),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}, {"TTV",-1}}},
    {std::make_pair(4, 64),{{"BTV",-1}, {"NS",-1}, {"TTV",-1}}},
    {std::make_pair(64, 16),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}, {"TTV",-1}}},
    {std::make_pair(64, 32),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}, {"TTV",-1}}},
    {std::make_pair(64, 4),{{"BTH",-1}, {"NS",-1}, {"TTH",-1}}},
    {std::make_pair(8, 64),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTV",-1}}},
    {std::make_pair(4, 8),{{"BTV",-1}, {"NS",-1}}},
    {std::make_pair(64, 64),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"QT",-1}, {"TTH",-1}, {"TTV", -1}}},
    {std::make_pair(64, 8),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}}},
    {std::make_pair(8, 32),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTV",-1}}},
    {std::make_pair(16, 8),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}}},
    {std::make_pair(8, 8),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}}},
    {std::make_pair(4, 32),{{"BTV",-1}, {"NS",-1}, {"TTV",-1}}},
    {std::make_pair(32, 32),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"QT",-1}, {"TTH",-1}, {"TTV", -1}}},
    {std::make_pair(8, 4),{{"BTH",-1}, {"NS",-1}}},
    {std::make_pair(16, 16),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"QT",-1}, {"TTH",-1}, {"TTV", -1}}},
    {std::make_pair(32, 8),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}}},
    {std::make_pair(16, 4),{{"BTH",-1} ,{"NS",-1}, {"TTH",-1}}},
    {std::make_pair(32, 4),{{"BTH",-1} ,{"NS",-1}, {"TTH",-1}}},
    {std::make_pair(4, 16),{{"BTV",-1}, {"NS",-1}, {"TTV",-1}}},
    {std::make_pair(32, 16),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}, {"TTV", -1}}},
    {std::make_pair(8, 16),{{"BTH",-1}, {"BTV", -1}, {"NS",-1}, {"TTV", -1}}},
    {std::make_pair(16, 32),{{"BTH",-1}, {"BTV",-1}, {"NS",-1}, {"TTH",-1}, {"TTV",-1}}}
  } ;



  std::map<string, int> conv = {{"QT",0},{"BTH",1},{"BTV",2},{"TTH",3},{"TTV",4},{"NS",5}};
  double input[1985];
  double *output = (double *) malloc(sizeof(double) * 6);

  uint8_t *splitDecision = new uint8_t[6]; // 0 = QT, 1 = BTH, 2 = BTV, 3 = TTH, 4 = TTV, 5 = NS
  for (int i = 0; i < 6; i++) {
    splitDecision[i] = 0;
  }



  //int w = width/2 -1 ;
  //int h = height/2 - 1 ;
  int vector_length = (width/4-1)*(height/4)+(width/4)*(height/4-1);
  double * subvector = (double * ) malloc(sizeof(double) * vector_length);
  extract_from_vector_inter(proba, subvector, y, x, width, height);

  input[0] = qp;
  for(int i=0;i<vector_length;i++){
    input[i+1] = subvector[i];
  }


  predict_partitionInter->predict_once_inter(input, output, make_pair(height, width));

  vector<string> v;
  for(map<string,int>::iterator it = resClass[std::make_pair(height, width)].begin(); it != resClass[std::make_pair(height, width)].end(); ++it) {
    v.push_back(it->first);
  }

  int top1 = -1;
  int top2 = -1;
  int top3 = -1;
  //int top4 = -1;
  double valTop1 = 0;
  double valTop2 = 0;
  double valTop3 = 0;
  //double valTop4 = 0;

  for (int i = 0; i < v.size(); i++){
    //std::cout<<v[i]<<" : "<<output[i]<<" / ";
    if(v.size() == 2 && v[0] != "NS" && i == 1){
      if(valTop1<1-output[i-1]){
        top2 = top1;
        top1 = i;
        valTop2 = valTop1;
        valTop1 = 1-output[i-1];
      }
      else {
        top2 = i;
        valTop2 = 1-output[i-1];
      }
    }
    else if(v[0] == "NS"){
      if(i==0){
        valTop1 = 1-output[i];
        top1 = i;
      }
      else{
        if(valTop1<output[i-1]){
          top2 = top1;
          top1 = i;
          valTop2 = valTop1;
          valTop1 = output[i-1];
        }
        else {
          top2 = i;
          valTop2 = output[i-1];
        }
      }
    }
    else if(valTop1<output[i]){
      /*top4 = top3;*/
      top3 = top2;
      top2 = top1;
      top1 = i;
      /*valTop4 = valTop3;*/
      valTop3 = valTop2;
      valTop2 = valTop1;
      valTop1 = output[i];
    }
    else if(valTop2<output[i]){
      /*top4 = top3;*/
      top3 = top2;
      top2 = i;
      /*valTop4 = valTop3;*/
      valTop3 = valTop2;
      valTop2 = output[i];
    }
    else if(valTop3 < output[i]){
      //top4 = top3;
      top3 = i;
      //valTop4 = valTop3;
      valTop3 = output[i];
    }
    /*else if(valTop4 < output[i]){
      top4 = i;
      valTop4 = output[i];
    }*/
  }
  //std::cout<<std::endl;

  //To make top1 with one split at least
  /*if(v[top1] == "NS"){
    resClass[std::make_pair(height, width)][v[top2]]=1;
  }
  else{
    resClass[std::make_pair(height, width)][v[top1]]=1;
  }*/

  //Classical top 3 choice
  resClass[std::make_pair(height, width)][v[top1]]=1;
  //std::cout<<v.at(top1)<<" "<<resClass[std::make_pair(height, width)][v[top1]]<<" / ";
  resClass[std::make_pair(height, width)][v[top2]]=2;
  if (top3!=-1){
    resClass[std::make_pair(height, width)][v[top3]]=3;
  }
  /*if(top4!=-1){
    resClass[std::make_pair(height, width)][v[top4]]=4;
  }*/

  for(map<string,int>::iterator it = resClass[std::make_pair(height, width)].begin(); it != resClass[std::make_pair(height, width)].end(); ++it) {
    if(it->second != -1 ){
      splitDecision[conv[it->first]] = 1;
    }
  }

  delete [] output;
  delete [] subvector;
  return splitDecision;
}

int * predictSplit(std::vector<float> * pred_vector, int x, int y, int widthCU, int heightCU){
  int nbBoundaryPerLine = 63;
  int width_height_max = 128;
  /*if(!isLuma){
    nbBoundaryPerLine = 15;
    width_height_max = 32;
  }*/
  int block_min = 4;
  int * all_mean = new int [8];
  float mean = 0;

  //Split Hor
  if(heightCU<=4){
    all_mean[0]=0;
    all_mean[1]=0;
  }
  else{
    //Middle left part
    for(int i=(((heightCU/block_min)-2)*nbBoundaryPerLine)/2+y/block_min*nbBoundaryPerLine+(x/2); i < (((heightCU/block_min)-2)*nbBoundaryPerLine)/2+nbBoundaryPerLine/(width_height_max/widthCU)/2+y/block_min*nbBoundaryPerLine+x/2 ;i+=2){
      mean += pred_vector->at(i);
    }
    if((float)((float)(widthCU/block_min)/2)<1){
      all_mean[0] = pred_vector->at((((heightCU/block_min)-2)*nbBoundaryPerLine)/2+y/block_min*nbBoundaryPerLine+(x/2))*100;
    }
    else{
      all_mean[0] = (mean / ((float)(widthCU/block_min)/2))*100;
    }
    mean = 0;
    //Middle right part
    for(int i=(((heightCU/block_min)-2)*nbBoundaryPerLine)/2+nbBoundaryPerLine/(width_height_max/widthCU)/2+y/block_min*nbBoundaryPerLine+x/2+1; i < (((heightCU/block_min)-2)*nbBoundaryPerLine)/2+nbBoundaryPerLine/(width_height_max/widthCU)+y/block_min*nbBoundaryPerLine+x/2 ;i+=2){
      mean += pred_vector->at(i);
    }
    if((float)((float)(widthCU/block_min)/2)<1){
      all_mean[1] = all_mean[0];
    }
    else{
      all_mean[1] = (mean / ((float)(widthCU/block_min)/2))*100;
    }
  }

  mean = 0;
  //Split Ver
  if(widthCU<=4){
    all_mean[2]=0;
    all_mean[3]=0;
  }
  else{
    //Middle top part
    for(int i=(widthCU/block_min)-1+y/block_min*nbBoundaryPerLine+x/2; i<((heightCU/block_min)-1)*nbBoundaryPerLine/2+y/block_min*nbBoundaryPerLine+x/4+1;i+=nbBoundaryPerLine){
      mean += pred_vector->at(i);
    }
    if(((float)(heightCU/block_min)/2)<1){
      all_mean[2] = mean * 100;
    }
    else{
      all_mean[2] = (mean / ((float)(heightCU/block_min)/2))*100;
    }

    mean = 0;
    for(int i=(heightCU/(2*block_min))*nbBoundaryPerLine+(y/block_min)*nbBoundaryPerLine+(widthCU/block_min)-1+x/2/*((heightCU/block_min)-1)*nbBoundaryPerLine/2+y/block_min*nbBoundaryPerLine+x/4+nbBoundaryPerLine*/; i<((heightCU/block_min)-1)*nbBoundaryPerLine+y/block_min*nbBoundaryPerLine;i+=nbBoundaryPerLine){
      mean += pred_vector->at(i);
    }
    //The last line because we only have vertical possible split and no horizontal
    if(heightCU+y==width_height_max){
      mean += pred_vector->at(((heightCU/block_min)-1)*nbBoundaryPerLine+(widthCU/block_min/2-1)+y/block_min*nbBoundaryPerLine+x/4);
    }
    else mean += pred_vector->at(((heightCU/block_min)-1)*nbBoundaryPerLine+(widthCU/block_min)-1+y/block_min*nbBoundaryPerLine+x/2);
    if(((float)(heightCU/block_min)/2)<1){
      all_mean[2]= mean *100;
      all_mean[3]= mean *100;
    }
    else{
      all_mean[3]= (mean / ((float)(heightCU/block_min)/2))*100;
    }
  }

  mean = 0;
  //Split TTH
  if(heightCU<=8){
    all_mean[4]=0;
    all_mean[5]=0;
  }
  else{
    //Top part
    for(int i=(((heightCU/block_min)-4)*nbBoundaryPerLine)/4+y/block_min*nbBoundaryPerLine+(x/2); i < (((heightCU/block_min)-4)*nbBoundaryPerLine)/4+nbBoundaryPerLine/(width_height_max/widthCU)+y/block_min*nbBoundaryPerLine+x/2 ;i+=2){
      mean += pred_vector->at(i);
    }
    all_mean[4] = (mean / (float)(widthCU/block_min))*100;
    mean = 0;
    //Bottom part
    for(int i=((heightCU/block_min)*nbBoundaryPerLine)*3/4-nbBoundaryPerLine+y/block_min*nbBoundaryPerLine+x/2; i < ((heightCU/block_min)*nbBoundaryPerLine)*3/4-nbBoundaryPerLine+y/block_min*nbBoundaryPerLine+x/2+nbBoundaryPerLine/(width_height_max/widthCU) ;i+=2){
      mean += pred_vector->at(i);
    }
    all_mean[5] = (mean / (float)(widthCU/block_min))*100;
  }

  mean = 0;
  //Split TTV
  if(widthCU<=8){
    all_mean[6]=0;
    all_mean[7]=0;
  }
  else{
    //Left part
    for(int i=(widthCU/block_min)/2-1+y/block_min*nbBoundaryPerLine+x/2; i<((heightCU/block_min)-1)*nbBoundaryPerLine+y/block_min*nbBoundaryPerLine;i+=nbBoundaryPerLine){
      mean += pred_vector->at(i);
    }
    //The last line because we only have vertical possible split and no horizontal
    if(heightCU+y==width_height_max){
      mean += pred_vector->at(((heightCU/block_min)-1)*nbBoundaryPerLine+(widthCU/block_min/4-1)+y/block_min*nbBoundaryPerLine+x/4);
    }
    else mean += pred_vector->at(((heightCU/block_min)-1)*nbBoundaryPerLine+(widthCU/block_min)/2-1+y/block_min*nbBoundaryPerLine+x/2);
    all_mean[6]= (mean / (float)(heightCU/block_min))*100;

    mean = 0;
    //Right part
    for(int i=(widthCU/block_min)*3/2-1+y/block_min*nbBoundaryPerLine+x/2; i<((heightCU/block_min)-1)*nbBoundaryPerLine+y/block_min*nbBoundaryPerLine;i+=nbBoundaryPerLine){
      mean += pred_vector->at(i);
    }
    //The last line because we only have vertical possible split and no horizontal
    if(heightCU+y==width_height_max){
      mean += pred_vector->at(((heightCU/block_min)-1)*nbBoundaryPerLine+(widthCU/block_min*3/4-1)+y/block_min*nbBoundaryPerLine+x/4);
    }
    else mean += pred_vector->at(((heightCU/block_min)-1)*nbBoundaryPerLine+(widthCU/block_min)*3/2-1+y/block_min*nbBoundaryPerLine+x/2);
    all_mean[7]= (mean / (float)(heightCU/block_min))*100;
  }
  return all_mean;
}

uint8_t * splitChoice(int * mean_split) {
  // 0 when no split, 1 when undefined, 2 when split
  // 0 = don't go further into split decision
  // 1 = test the split
  // 2 = don't go into the intra mode function
  int thresholdQT = 30;
  int thresholdBT = 30;
  int thresholdTT = 30;
  uint8_t *splitDecision = new uint8_t[5]; // 0 = QT, 1 = BTH, 2 = BTV, 3 = TTH, 4 = TTV
  for (int i = 0; i < 5; i++) {
    splitDecision[i] = 1;
  }

  //int qtProba = min(min(mean_split[2],mean_split[3]),min(mean_split[0],mean_split[1]));
  //int btvProba = int((1/(1+exp(-((float(min(mean_split[2],mean_split[3]))/100-float(min(mean_split[0],mean_split[1]))/100)-1))))*100);
  //int bthProba = int((1/(1+exp(-((float(min(mean_split[0],mean_split[1]))/100-float(min(mean_split[2],mean_split[3]))/100)-1))))*100);
  //int btvProba = min(min(mean_split[2],mean_split[3]),int((1/(1+exp(-((float(min(mean_split[2],mean_split[3]))/100-float(min(mean_split[0],mean_split[1]))/100)-0.4))))*100));
  //int bthProba = min(min(mean_split[0],mean_split[1]),int((1/(1+exp(-((float(min(mean_split[0],mean_split[1]))/100-float(min(mean_split[2],mean_split[3]))/100)-1))))*100));
  //int bthProba = min(mean_split[0],mean_split[1]);
  int bthProba = (mean_split[0]+mean_split[1])/2;
  int btvProba = (mean_split[2]+mean_split[3])/2;
  //int qtProba = ((float(btvProba)/100) * (float(bthProba)/100) + 0.01 * pow(((float(bthProba)/100) + (float(btvProba)/100)),2))*100;
  int qtProba = (bthProba+btvProba)/2;
  int tthProba = (mean_split[4]+mean_split[5])/2;
  int ttvProba = (mean_split[6]+mean_split[7])/2;


  //std::cout<<qtProba<<" "<<bthProba<<" "<<btvProba<<" "<<tthProba<<" "<<ttvProba<<std::endl;

  //Split TTH
  if(tthProba < thresholdTT/*predict_partition->getThreshold("TTH", size)*/){
    splitDecision[3] = 0;
  } /*else if(min(mean_split[4],mean_split[5]) min(mean_split[0],mean_split[1]) > 90 && authorized_split[3]){
    splitDecision[3] = 2;
  }*/

  //Split TTV
  if(ttvProba < thresholdTT/*predict_partition->getThreshold("TTV", size)*/){
    splitDecision[4] = 0;
  } /*else if(min(mean_split[6],mean_split[7])min(mean_split[2],mean_split[3]) > 90 && authorized_split[4]){
    splitDecision[4] = 2;
  }*/

  //Split BTH
  if (bthProba < thresholdBT/*predict_partition->getThreshold("BTH", size)*/) {
    splitDecision[1] = 0;
  } /*else if (min(mean_split[0],mean_split[1]) > 90 && authorized_split[1]) {
    splitDecision[3] = 0;
    splitDecision[4] = 0;
    splitDecision[1] = 2;
  }*/

  //Split BTV
  if (btvProba < thresholdBT/*predict_partition->getThreshold("BTV", size)*/) {
    splitDecision[2] = 0;
  } /*else if (min(mean_split[2],mean_split[3]) > 90 && authorized_split[2]) {
    splitDecision[3] = 0;
    splitDecision[4] = 0;
    splitDecision[2] = 2;
  }*/

  //Split QT
  /*if(authorized_split[0] && splitDecision[1] == 2 && splitDecision[2] == 2){
    splitDecision[0] = 2;
  }*/
  if(qtProba < thresholdQT/*predict_partition->getThreshold("QT", size)*/){
    splitDecision[0] = 0;
  }

  return splitDecision;
}


int * predictSplitIntra(vector<float> * pred_vector, int x, int y, int widthCU, int heightCU){
  int nbBoundaryPerLine = 31;
  int width_height_max = 64;
  int block_min = 4;
  int * all_mean = new int [8];
  float mean = 0;

  //Split Hor
  if(heightCU<=4){
    all_mean[0]=0;
    all_mean[1]=0;
  }
  else{
    //Middle left part
    for(int i=(((heightCU/block_min)-2)*nbBoundaryPerLine)/2+y/block_min*nbBoundaryPerLine+(x/2); i < (((heightCU/block_min)-2)*nbBoundaryPerLine)/2+nbBoundaryPerLine/(width_height_max/widthCU)/2+y/block_min*nbBoundaryPerLine+x/2 ;i+=2){
      mean += pred_vector->at(i);
    }
    if((float)((float)(widthCU/block_min)/2)<1){
      all_mean[0] = pred_vector->at((((heightCU/block_min)-2)*nbBoundaryPerLine)/2+y/block_min*nbBoundaryPerLine+(x/2))*100;
    }
    else{
      all_mean[0] = (mean / ((float)(widthCU/block_min)/2))*100;
    }
    mean = 0;
    //Middle right part
    for(int i=(((heightCU/block_min)-2)*nbBoundaryPerLine)/2+nbBoundaryPerLine/(width_height_max/widthCU)/2+y/block_min*nbBoundaryPerLine+x/2+1; i < (((heightCU/block_min)-2)*nbBoundaryPerLine)/2+nbBoundaryPerLine/(width_height_max/widthCU)+y/block_min*nbBoundaryPerLine+x/2 ;i+=2){
      mean += pred_vector->at(i);
    }
    if((float)((float)(widthCU/block_min)/2)<1){
      all_mean[1] = all_mean[0];
    }
    else{
      all_mean[1] = (mean / ((float)(widthCU/block_min)/2))*100;
    }
  }

  mean = 0;
  //Split Ver
  if(widthCU<=4){
    all_mean[2]=0;
    all_mean[3]=0;
  }
  else{
    //Middle top part
    for(int i=(widthCU/block_min)-1+y/block_min*nbBoundaryPerLine+x/2; i<((heightCU/block_min)-1)*nbBoundaryPerLine/2+y/block_min*nbBoundaryPerLine+x/4+1;i+=nbBoundaryPerLine){
      mean += pred_vector->at(i);
    }
    if(((float)(heightCU/block_min)/2)<1){
      all_mean[2] = mean * 100;
    }
    else{
      all_mean[2] = (mean / ((float)(heightCU/block_min)/2))*100;
    }

    mean = 0;
    for(int i=(heightCU/(2*block_min))*nbBoundaryPerLine+(y/block_min)*nbBoundaryPerLine+(widthCU/block_min)-1+x/2/*((heightCU/block_min)-1)*nbBoundaryPerLine/2+y/block_min*nbBoundaryPerLine+x/4+nbBoundaryPerLine*/; i<((heightCU/block_min)-1)*nbBoundaryPerLine+y/block_min*nbBoundaryPerLine;i+=nbBoundaryPerLine){
      mean += pred_vector->at(i);
    }
    //The last line because we only have vertical possible split and no horizontal
    if(heightCU+y==width_height_max){
      mean += pred_vector->at(((heightCU/block_min)-1)*nbBoundaryPerLine+(widthCU/block_min/2-1)+y/block_min*nbBoundaryPerLine+x/4);
    }
    else mean += pred_vector->at(((heightCU/block_min)-1)*nbBoundaryPerLine+(widthCU/block_min)-1+y/block_min*nbBoundaryPerLine+x/2);
    if(((float)(heightCU/block_min)/2)<1){
      all_mean[2]= mean *100;
      all_mean[3]= mean *100;
    }
    else{
      all_mean[3]= (mean / ((float)(heightCU/block_min)/2))*100;
    }
  }

  mean = 0;
  //Split TTH
  if(heightCU<=8){
    all_mean[4]=0;
    all_mean[5]=0;
  }
  else{
    //Top part
    for(int i=(((heightCU/block_min)-4)*nbBoundaryPerLine)/4+y/block_min*nbBoundaryPerLine+(x/2); i < (((heightCU/block_min)-4)*nbBoundaryPerLine)/4+nbBoundaryPerLine/(width_height_max/widthCU)+y/block_min*nbBoundaryPerLine+x/2 ;i+=2){
      mean += pred_vector->at(i);
    }
    all_mean[4] = (mean / (float)(widthCU/block_min))*100;
    mean = 0;
    //Bottom part
    for(int i=((heightCU/block_min)*nbBoundaryPerLine)*3/4-nbBoundaryPerLine+y/block_min*nbBoundaryPerLine+x/2; i < ((heightCU/block_min)*nbBoundaryPerLine)*3/4-nbBoundaryPerLine+y/block_min*nbBoundaryPerLine+x/2+nbBoundaryPerLine/(width_height_max/widthCU) ;i+=2){
      mean += pred_vector->at(i);
    }
    all_mean[5] = (mean / (float)(widthCU/block_min))*100;
  }

  mean = 0;
  //Split TTV
  if(widthCU<=8){
    all_mean[6]=0;
    all_mean[7]=0;
  }
  else{
    //Left part
    for(int i=(widthCU/block_min)/2-1+y/block_min*nbBoundaryPerLine+x/2; i<((heightCU/block_min)-1)*nbBoundaryPerLine+y/block_min*nbBoundaryPerLine;i+=nbBoundaryPerLine){
      mean += pred_vector->at(i);
    }
    //The last line because we only have vertical possible split and no horizontal
    if(heightCU+y==width_height_max){
      mean += pred_vector->at(((heightCU/block_min)-1)*nbBoundaryPerLine+(widthCU/block_min/4-1)+y/block_min*nbBoundaryPerLine+x/4);
    }
    else mean += pred_vector->at(((heightCU/block_min)-1)*nbBoundaryPerLine+(widthCU/block_min)/2-1+y/block_min*nbBoundaryPerLine+x/2);
    all_mean[6]= (mean / (float)(heightCU/block_min))*100;

    mean = 0;
    //Right part
    for(int i=(widthCU/block_min)*3/2-1+y/block_min*nbBoundaryPerLine+x/2; i<((heightCU/block_min)-1)*nbBoundaryPerLine+y/block_min*nbBoundaryPerLine;i+=nbBoundaryPerLine){
      mean += pred_vector->at(i);
    }
    //The last line because we only have vertical possible split and no horizontal
    if(heightCU+y==width_height_max){
      mean += pred_vector->at(((heightCU/block_min)-1)*nbBoundaryPerLine+(widthCU/block_min*3/4-1)+y/block_min*nbBoundaryPerLine+x/4);
    }
    else mean += pred_vector->at(((heightCU/block_min)-1)*nbBoundaryPerLine+(widthCU/block_min)*3/2-1+y/block_min*nbBoundaryPerLine+x/2);
    all_mean[7]= (mean / (float)(heightCU/block_min))*100;
  }
  return all_mean;
}