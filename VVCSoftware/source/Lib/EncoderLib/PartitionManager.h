//
// Created by altissie on 02/09/2019.
//

#ifndef NEXTSOFTWARE_PARTITIONMANAGER_H
#define NEXTSOFTWARE_PARTITIONMANAGER_H


#include <vector>
#include <fstream>
#include "PartitionParam.h"
#include "PartitionTree.h"

class PartitionManager {
public:
    PartitionManager(PartitionParam *param, u_int16_t width, u_int16_t height, std::string filename, bool fileIn);
    ~PartitionManager();

    void store_params();
    void store_ctu(PartitionTree * ctu_to_store, bool isInter);
    void load_params();
    void load_ctu(PartitionTree * ctu_loaded);
    void reset_ctu();
    void shall_we_split(bool * try_split);

    PartitionTree *getM_cur_tree() const {return m_cur_tree;};
    void setM_cur_tree(PartitionTree *m_cur_tree) {PartitionManager::m_cur_tree = m_cur_tree;};


private:
    PartitionParam * m_param;
    std::ofstream m_file_out;
    std::ifstream m_file_in;
    u_int16_t m_width;
    u_int16_t m_height;
    PartitionTree * m_cur_tree;


    template<typename  Type> void write(Type data);
    template<typename  Type> void read(Type * data);
    void split_from_depth(int* depth, int* mtt_split_mode, int width, const int height, PartitionTree * tree);
};



#endif //NEXTSOFTWARE_PARTITIONMANAGER_H
