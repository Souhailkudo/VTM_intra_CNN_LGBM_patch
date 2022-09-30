//
// Created by altissie on 02/09/2019.
//

#ifndef NEXTSOFTWARE_PARTITIONTREE_H
#define NEXTSOFTWARE_PARTITIONTREE_H


#include "PartitionParam.h"
#include <Mv.h>

class PartitionTree {

public:
    //PartitionTree();
    PartitionTree(int width, int height, PartitionTree ** leaves, PartitionTree * root, PartitionParam * cur_param, bool is_bin=false, bool is_tern=false, double rd_cost=-1, bool stop_chroma=false);
    ~ PartitionTree();



    u_int32_t get_cur_val();
    uint32_t ** array_store();
    u_int16_t get_node_number();

    PartitionTree **getM_leaves() const {return m_leaves;};
    PartitionTree *getM_leaves(int idx) const {return m_leaves[idx];};
    bool isM_is_bin() const {return m_is_bin;};
    bool isM_is_tern() const {return m_is_tern;};
    int getM_width() const {return m_width;};
    int getM_height() const {return m_height;};
    double get_rd_cost() const {return m_rd_cost;};
    double get_rd_cost_without_split() const {return rd_cost_without_split;};
    PartitionTree *getM_root() const {return m_root;};
    PartitionParam *getM_cur_param() const {return m_cur_param;};
    bool get_chroma_stop() const {return m_stop_chroma;};
    Mv get_mvUni(int index) const {return mvUni[index];};
    int get_pocRef(int index) const {return pocRef[index];};


    void setM_width(int m_width) {PartitionTree::m_width = m_width;};
    void setM_height(int m_height) {PartitionTree::m_height = m_height;};
    void set_rd_cost(double rd_cost) {PartitionTree::m_rd_cost = rd_cost;};
    void set_rd_cost_without_split(double rd_cost_without_split) {PartitionTree::rd_cost_without_split = rd_cost_without_split;};
    void set_chroma_stop(bool chroma_stop){PartitionTree::m_stop_chroma = chroma_stop;};
    void set_mvUni(Mv * mv);
    void set_pocRef(int * poc);
    void set_mvUniWithoutMovingPrec(Mv * mv);

    void qt_split();
    void bth_split();
    void btv_split();
    void tth_split();
    void ttv_split();

    static void transfer_leaves(PartitionTree * treeSource, PartitionTree * treeDest);

private:
    PartitionParam * m_cur_param;
    PartitionTree ** m_leaves;
    PartitionTree * m_root;
    int m_width;
    int m_height;
    int m_Bt_depth;
    int m_Qt_depth;
    int m_Tt_depth;
    double m_rd_cost;
    double rd_cost_without_split;
    bool m_is_bin;
    bool m_is_tern;
    bool m_stop_chroma;
    u_int32_t m_cur_val;
    Mv * mvUni;
    int * pocRef;
    void split(bool h, bool v, bool is_tern = false);
    void rebuild_values();
    void rebuild_depths();

};


#endif //NEXTSOFTWARE_PARTITIONTREE_H
