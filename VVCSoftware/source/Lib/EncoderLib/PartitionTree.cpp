//
// Created by altissie on 02/09/2019.
//

#include "PartitionTree.h"

PartitionTree::PartitionTree(int width, int height, PartitionTree ** leaves, PartitionTree * root, PartitionParam * cur_param, bool is_bin, bool is_tern, double rd_cost, bool stop_chroma){

    m_width = width;
    m_height = height;
    m_leaves = leaves;
    m_root = root;
    m_cur_param = cur_param;
    m_rd_cost = rd_cost;
    rd_cost_without_split = -1;
    m_is_bin = is_bin;
    m_is_tern = is_tern;

    m_Qt_depth = 0;
    m_Bt_depth = 0;
    m_Tt_depth = 0;

    m_stop_chroma = stop_chroma;

    if(root != nullptr) {
        PartitionTree *current_node = this;

        while((*current_node).m_root != nullptr) {
            if((*current_node).m_is_bin) {
                m_Bt_depth++;
            }
            else if((*current_node).m_is_tern) {
                m_Tt_depth++;
            }
            else {
                m_Qt_depth++;
            }
            current_node = (*current_node).m_root;
        }
    }

    mvUni = (Mv *) malloc(2*sizeof(Mv));
    pocRef = (int *) malloc(2 * sizeof(int));
    if(root== nullptr){
      mvUni[0].setHor(9999);
    }
    m_cur_val = get_cur_val();
}


PartitionTree::~PartitionTree(){
    if(m_leaves != nullptr)
    {
        int nb_leafs;
        if(m_leaves[0]->m_is_bin)
        {
            nb_leafs = 2;
        }
        else if(m_leaves[0]->m_is_tern){
            nb_leafs = 3;
        }
        else
        {
            nb_leafs = 4;
        }

        for(int i=0;i<nb_leafs;i++)
        {
            delete m_leaves[i];
        }

        delete[] m_leaves;
    }

    free(mvUni);
    free(pocRef);

}


u_int32_t PartitionTree::get_cur_val() {
    uint32_t ret_val;

    // Calculating m_cur_val which differs from 0 if we have a root
    if(m_root != nullptr)
    {
        ret_val = m_root->m_cur_val;

        // If we're still in quad-tree
        if(!m_is_bin && !m_is_tern)
        {
            // We add one to the depth
            ret_val += unsigned(0b1) << unsigned(m_cur_param->mttDataSize);
        }
            // If we're in a binary tree
        else if(m_is_bin)
        {
            // If it is horizontal splitting
            if(m_root->m_height > m_height)
            {
                ret_val += unsigned(1) << unsigned(m_cur_param->mttDataSize - 3*(m_Bt_depth+m_Tt_depth));
            }
            else
            {
                ret_val += unsigned(3) << unsigned(m_cur_param->mttDataSize - 3*(m_Bt_depth+m_Tt_depth));
            }
        }
            // If we're in ternary tree
        else{
            // If it is horizontal splitting
            if(m_root->m_height > m_height)
            {
                ret_val += unsigned(5) << unsigned(m_cur_param->mttDataSize - 3*(m_Bt_depth+m_Tt_depth));
            }
            else
            {
                ret_val += unsigned(7) << unsigned(m_cur_param->mttDataSize - 3*(m_Bt_depth+m_Tt_depth));
            }
        }
    }
        // roots have value '0'
    else
    {
        ret_val = 0;
    }

    return ret_val;
}

// Returns PU representation matrix
uint32_t ** PartitionTree::array_store()
{
    uint32_t ** returned_table;
    returned_table = new uint32_t*[m_width];
    for(int i=0; i< m_width; i++)
    {
        returned_table[i] = new uint32_t[m_height];
    }

    if(m_leaves == nullptr)
    {
        for(int i = 0; i < m_width; i++)
        {
            for(int j = 0; j < m_height; j++)
            {
                returned_table[i][j] = m_cur_val;
            }
        }

    }
    else
    {
        int leaf_number;

        // If sub tree is binary
        if(m_leaves[0]->m_is_bin)
        {
            leaf_number = 2;
            // If we face vertical splitting
            if(m_leaves[0]->m_width < m_width)
            {
                // From upper block to lower block
                for(int i=0;i<leaf_number;i++)
                {
                    // Getting block matrix from leaf
                    uint32_t ** tmp_tab = m_leaves[i]->array_store();

                    // In one leaf
                    //Copying it into current table
                    for(int j=0;j<m_leaves[0]->m_width;j++)
                    {
                        for(int k=0;k<m_leaves[0]->m_height;k++)
                        {
                            returned_table[j+i*m_leaves[0]->m_width][k] = tmp_tab[j][k];
                        }
                    }

                    for(int k = 0; k< m_leaves[0]->m_width; k++)
                    {
                        delete[] tmp_tab[k];
                    }

                    delete[] tmp_tab;

                }
            }
                // If we face horizontal splitting
            else
            {
                // From left block to right block
                for(int i=0;i<leaf_number;i++)
                {
                    // Getting block matrix from leaf
                    uint32_t ** tmp_tab = m_leaves[i]->array_store();

                    // In one leaf
                    //Copying it into current table
                    for(int j=0;j<m_leaves[0]->m_width;j++)
                    {
                        for(int k=0;k<m_leaves[0]->m_height;k++)
                        {
                            returned_table[j][k+i*m_leaves[0]->m_height] = tmp_tab[j][k];
                        }
                    }

                    for(int k = 0; k< m_leaves[0]->m_width; k++)
                    {
                        delete[] tmp_tab[k];
                    }

                    delete[] tmp_tab;

                }
            }
        } // end binary
            //Else if subtree is ternary
        else if(m_leaves[0]->m_is_tern){
            leaf_number = 3;
            // If we have vertical splitting
            if(m_leaves[0]->m_width < m_width)
            {
                // From upper block to lower block
                for(int i=0;i<leaf_number;i++)
                {
                    int actual_width=0;
                    for(int j=0;j<i;j++){
                        actual_width+=m_leaves[j]->m_width;
                    }
                    // Getting block matrix from leaf
                    uint32_t ** tmp_tab = m_leaves[i]->array_store();

                    // In one leaf
                    //Copying it into current table
                    for(int j=0;j<m_leaves[i]->m_width;j++)
                    {
                        for(int k=0;k<m_leaves[i]->m_height;k++)
                        {
                            returned_table[j+actual_width][k] = tmp_tab[j][k];
                        }
                    }

                    for(int k = 0; k< m_leaves[i]->m_width; k++)
                    {
                        delete[] tmp_tab[k];
                    }

                    delete[] tmp_tab;

                }
            }
                // If we face horizontal splitting
            else
            {
                // From left block to right block
                for(int i=0;i<leaf_number;i++)
                {
                    int actual_height=0;
                    for(int j=0;j<i;j++){
                        actual_height+=m_leaves[j]->m_height;
                    }
                    // Getting block matrix from leaf
                    uint32_t ** tmp_tab = m_leaves[i]->array_store();

                    // In one leaf
                    //Copying it into current table
                    for(int j=0;j<m_leaves[i]->m_width;j++)
                    {
                        for(int k=0;k<m_leaves[i]->m_height;k++)
                        {
                            returned_table[j][k+actual_height] = tmp_tab[j][k];
                        }
                    }
                    for(int k = 0; k< m_leaves[i]->m_width; k++)
                    {
                        delete[] tmp_tab[k];
                    }

                    delete[] tmp_tab;

                }
            }
        }
            // Else if subtree is quad
        else
        {
            // From upper block to lower block
            for(int i=0;i<2;i++)
            {
                // From left block to right block
                for(int j=0;j<2;j++)
                {
                    uint32_t ** tmp_tab = m_leaves[j+2*i]->array_store();

                    // In one leaf
                    // From upper cell to lower cell
                    for(int k=0;k<m_leaves[0]->m_width;k++)
                    {
                        // From left cell to right cell
                        for(int l=0;l<m_leaves[0]->m_height;l++)
                        {
                            returned_table[l+j*m_leaves[0]->m_width][k+i*m_leaves[0]->m_height] = tmp_tab[l][k];
                        }

                    }

                    for(int k = 0; k< m_leaves[0]->m_width; k++)
                    {
                        delete[] tmp_tab[k];
                    }

                    delete[] tmp_tab;

                }

            }
        } // end quad

    }

    return returned_table;
}


// get the number of node in a PU_Tree, used for rd_cost storage as there is
// one rd cost per node to store
u_int16_t PartitionTree::get_node_number()
{
    uint16_t nb_nodes = 1;

    if(m_leaves != nullptr)
    {
        int n_leaves;
        if(m_leaves[0]->m_is_bin){
            n_leaves = 2;
        }
        else if(m_leaves[0]->m_is_tern){
            n_leaves = 3;
        }
        else{
            n_leaves = 4;
        }

        for(int i = 0; i< n_leaves; i++)
        {
            nb_nodes += m_leaves[i]->get_node_number();
        }
    }

    return nb_nodes;
}


// Performs a quad tree split on the tree node
void PartitionTree::qt_split()
{
    this->split(true, true);
}

// Performs a horizontal binary split on the tree node
void PartitionTree::bth_split()
{
    this->split( true, false);
}

// Performs a vertical binary split on the tree node
void PartitionTree::btv_split()
{
    this->split(false, true);
}

// Performs a horizontal ternary split on the tree node
void PartitionTree::tth_split()
{
    this->split(true, false, true);
}

// Performs a vertical ternary split on the tree node
void PartitionTree::ttv_split()
{
    this->split(false, true, true);
}

// Splits the node in two, three or four leaves depending on the split
void PartitionTree::split(bool h, bool v, bool is_tern /*= false*/)
{
    // Number of blocks created by splitting
    int n_split;

    // If we are to perform a quad split
    if(h && v)
    {
        n_split = 4;
    }
        // If we are to perform a ternary split
    else if(is_tern){
        n_split = 3;
    }
        // If we are to perform a binary split
    else
    {
        n_split = 2;
    }

    // Allocating PartitionTree array
    m_leaves = new PartitionTree *[n_split];


    int new_width = m_width;
    int new_height = m_height;
    if(h){
        new_height = new_height/2;
    }
    if(v){
        new_width = new_width/2;
    }

    // Filling it with appropriate PU_Trees
    for(int i=0; i<n_split; i++)
    {
        // If we are in ternary split
        if(is_tern){
            // If we are in horizontal ternary split
            if(h){
                // If we are creating the middle tree of the ternary split
                if(i==1){
                    m_leaves[i] = new PartitionTree(m_width, m_height/2, nullptr, this, m_cur_param, false, true);
                }
                    // If we are creating the flank tree of the ternary split
                else m_leaves[i] = new PartitionTree(m_width, m_height/4, nullptr, this, m_cur_param, false, true);
            }
                // If we are in vertical ternary split
            else{
                // If we are creating the middle tree of the ternary split
                if(i==1){
                    m_leaves[i] = new PartitionTree(m_width/2, m_height, nullptr, this, m_cur_param, false, true);
                }
                    // If we are creating the flank tree of the ternary split
                else  m_leaves[i] = new PartitionTree(m_width/4, m_height, nullptr, this, m_cur_param, false, true);
            }

        }
            // If we are in quad or binary split
        else {
            m_leaves[i] = new PartitionTree(new_width, new_height, nullptr, this, m_cur_param, (h && v) ? false : true);
        }
    }
}


// ensure transmission of the leaves of one source PU_Tree
// to a destination PU_Tree
void PartitionTree::transfer_leaves(PartitionTree * treeSource, PartitionTree * treeDest)
{
    int n_leaves;

    if(treeSource->m_leaves[0]->m_is_bin)
    {
        n_leaves = 2;
    }
    else if(treeSource->m_leaves[0]->m_is_tern){
        n_leaves = 3;
    }
    else
    {
        n_leaves = 4;
    }

    treeDest->m_leaves = new PartitionTree*[n_leaves];

    for(int i = 0; i < n_leaves; i++)
    {
        treeDest->m_leaves[i] = treeSource->m_leaves[i];
        treeDest->m_leaves[i]->m_root = treeDest;
    }

    delete[] treeSource->m_leaves;
    treeSource->m_leaves = nullptr;

    // Rebuild depths and values for the PU_Tree to ensure data consistency
    // /!\ must be performed in this order as values depends on depths
    treeDest->rebuild_depths();
    treeDest->rebuild_values();
}


// rebuild recursively m_value and ensure transmission of is_bin property
void PartitionTree::rebuild_values()
{
    // get_cur_val depends on QT and BT depth
    m_cur_val = get_cur_val();

    if(m_leaves == nullptr)
    {
        // Nothing
    }
    else
    {
        int n_leaves;

        if(m_leaves[0]->m_is_bin)
        {
            n_leaves = 2;
        }
        else if(m_leaves[0]->m_is_tern){
            n_leaves = 3;
        }
        else
        {
            n_leaves = 4;
        }


        for(int i = 0; i < n_leaves; i++)
        {
            m_leaves[i]->m_cur_val = m_leaves[i]->get_cur_val();
            m_leaves[i]->rebuild_values();
        }
    }
}

// rebuild recursively BT and QT depths
void PartitionTree::rebuild_depths()
{

    if(m_leaves == nullptr)
    {
        // Nothing
    }
    else
    {
        int n_leaves;

        if(m_leaves[0]->m_is_bin)
        {
            n_leaves = 2;
        }
        else if(m_leaves[0]->m_is_tern){
            n_leaves = 3;
        }
        else
        {
            n_leaves = 4;
        }


        for(int i = 0; i < n_leaves; i++)
        {
            if(m_leaves[0]->m_is_bin)
            {
                m_leaves[i]->m_Bt_depth = m_Bt_depth + 1;
                m_leaves[i]->m_Tt_depth = m_Tt_depth;
                m_leaves[i]->m_Qt_depth = m_Qt_depth;
            }
            else if(m_leaves[0]->m_is_tern){
                m_leaves[i]->m_Tt_depth = m_Tt_depth +1;
                m_leaves[i]->m_Bt_depth = m_Bt_depth;
                m_leaves[i]->m_Qt_depth = m_Qt_depth;
            }
            else
            {
                m_leaves[i]->m_Qt_depth = m_Qt_depth + 1;
            }

            m_leaves[i]->rebuild_depths();

        }
    }
}


void PartitionTree::set_mvUni(Mv * mv) {
    //mv[0].changePrecision(MV_PRECISION_INTERNAL, MV_PRECISION_INT);
    //mv[1].changePrecision(MV_PRECISION_INTERNAL, MV_PRECISION_INT);
    mvUni[0].set(mv[0].getHor(),mv[0].getVer());
    mvUni[1].set(mv[1].getHor(),mv[1].getVer());
}

void PartitionTree::set_mvUniWithoutMovingPrec(Mv * mv) {
    mvUni[0].set(mv[0].getHor(),mv[0].getVer());
    mvUni[1].set(mv[1].getHor(),mv[1].getVer());
}


void PartitionTree::set_pocRef(int * poc) {
    pocRef[0]=poc[0];
    pocRef[1]=poc[1];
}

