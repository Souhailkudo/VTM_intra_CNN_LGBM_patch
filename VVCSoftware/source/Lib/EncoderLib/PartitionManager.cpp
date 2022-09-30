//
// Created by altissie on 02/09/2019.
//

#include "PartitionManager.h"


using namespace std;

PartitionManager::PartitionManager(PartitionParam *param, u_int16_t width, u_int16_t height, std::string filename, bool fileIn){
    m_param = param;
    if(fileIn) m_file_in.open(filename, ios::in | ios::binary);
    else m_file_out.open(filename, ios::out | ios::binary);
    m_width = width;
    m_height = height;
    m_cur_tree = new PartitionTree(param->ctuSize, param->ctuSize, nullptr, nullptr, m_param);
}

PartitionManager::~PartitionManager(){
    if(m_file_out.is_open()){
        m_file_out.close();
    }
    delete m_cur_tree;
}

void PartitionManager::store_params()
{
    write(m_width);
    write(m_height);
    write(m_param->mttDataSize);
    write(m_param->ctuSize);
}


void PartitionManager::load_params()
{
    // reading params from file
    m_file_in.seekg (0, ios::beg);
    read(&m_width);
    read(&m_height);
    read(&m_param->mttDataSize);
    read(&m_param->ctuSize);

    // applying width and height to CTU
    m_cur_tree->setM_width(m_param->ctuSize);
    m_cur_tree->setM_height(m_param->ctuSize);
}

// Generic binary write function
template<typename  Type> void PartitionManager::write(Type data)
{
    //check_file();
    size_t sz = sizeof(data);
    unsigned char bytes[sz];
    // Splitting data into a char array for storage
    for(int i=0; i<sz; i++)
    {
        bytes[sz-i-1] = * ((unsigned char *)(&data)+i);
    }

    m_file_out.write((const char *)bytes, sz);
    m_file_out.flush();
}

// Generic binary read function
template<typename  Type> void PartitionManager::read(Type * data)
{
    //check_file(/*outing=*/false);

    size_t sz = sizeof(* data);
    char *bytes = (char*)calloc(sz,sizeof(char));
    *data = 0;

    m_file_in.read(bytes, sz);

    // Putting read 'bytes' into 'data', 8 bits at a time
    for(int i=0; i<sz; i++)
    {
        * ((char *)(data) + sz-i-1) = bytes[i];
    }
    free(bytes);
}

template<typename Type> void freeArray(std::vector<Type> &vector){
    vector.clear();
    vector.shrink_to_fit();
}


void PartitionManager::store_ctu(PartitionTree * ctu_to_store, bool isInter)
{
    write(isInter);

    uint32_t ** mat = ctu_to_store->array_store();
    for(int i=0; i < ctu_to_store->getM_height(); i++)
    {
        for(int j=0; j < ctu_to_store->getM_width(); j++)
        {
            write(mat[j][i]);
        }
    }

    for(int i=0; i< ctu_to_store->getM_width(); i++)
    {
        delete[] mat[i];
    }
    delete[] mat;

    write(ctu_to_store->get_node_number());

    if(isInter){
        write(ctu_to_store->get_pocRef(0));
        write(ctu_to_store->get_mvUni(0).getHor());
        write(ctu_to_store->get_mvUni(0).getVer());
        write(ctu_to_store->get_pocRef(1));
        write(ctu_to_store->get_mvUni(1).getHor());
        write(ctu_to_store->get_mvUni(1).getVer());
    }
}


// Resets current CTU
void PartitionManager::reset_ctu() {
    delete m_cur_tree;
    m_cur_tree = new PartitionTree(m_param->ctuSize, m_param->ctuSize, nullptr, nullptr, m_param);
}

void PartitionManager::load_ctu(PartitionTree * ctu_loaded)
{
    bool isInter;
    read(&isInter);

    int sz = m_param->ctuSize;
    uint32_t ** read_values;
    read_values = new uint32_t*[sz];
    for(int i=0; i< sz; i++)
    {
        read_values[i] = new uint32_t[sz];
    }


    int *depth = (int*)calloc(sz*sz, sizeof(int));
    int *mtt_split_mode = (int*)(calloc(sz*sz, sizeof(int)));
    unsigned int mtt_val_pu_tree, temp_mtt_val;

    for(int i = 0; i < sz; i++)
    {
        for(int j=0; j < sz; j++)
        {
            // Reading values (in PU_Tree units)
            read(&read_values[j][i]);

            // Converting them to JEM Depth format for QT splitting
            // and BT split mode for BT splitting
            // (could be a separate function mat_to_depth...)
            depth[j+i*sz] = read_values[j][i] >> m_param->mttDataSize;

            mtt_val_pu_tree = unsigned(read_values[j][i] - (depth[j+i*sz]  << m_param->mttDataSize));

            //Inverse the 3 values
            for(int k = 0; k < m_param->mttDataSize/3; k++)
            {
                temp_mtt_val = mtt_val_pu_tree >> unsigned(m_param->mttDataSize -3*k - 3);

                mtt_split_mode[j+i*sz] += temp_mtt_val << (3*k);

                mtt_val_pu_tree -= temp_mtt_val << unsigned(m_param->mttDataSize -3*k - 3);

            }

        }
    }


    split_from_depth(depth, mtt_split_mode, sz, sz, ctu_loaded);

    uint16_t node_number;
    read(&node_number);

    Mv * mvUni = (Mv *) malloc(2*sizeof(Mv));
    int * pocRef = (int *) malloc(2 * sizeof(int));

    if(isInter){
        read(&pocRef[0]);
        read(&mvUni[0].hor);
        read(&mvUni[0].ver);
        read(&pocRef[1]);
        read(&mvUni[1].hor);
        read(&mvUni[1].ver);
        ctu_loaded->set_pocRef(pocRef);
        ctu_loaded->set_mvUniWithoutMovingPrec(mvUni);
        //std::cout<<"OUT : Im ref 0 : "<<pocRef[0]<<" / X : "<<mvUni[0].getHor()<<" / Y : "<<mvUni[0].getVer()<<std::endl;
        //std::cout<<"OUT : Im ref 1 : "<<pocRef[1]<<" / X : "<<mvUni[1].getHor()<<" / Y : "<<mvUni[1].getVer()<<std::endl;
    }

    free(mvUni);
    free(pocRef);

    for(int i=0; i< sz; i++)
    {
        delete[] read_values[i];
    }
    delete[] read_values;
    free(depth);
    free(mtt_split_mode);
}


// Uses a depth map  'depth' of size 'size' to quad-split the 'quad-tree'
void PartitionManager::split_from_depth(int* depth, int* mtt_split_mode, const int width,
                                        const int height, PartitionTree * tree)
{
    // in case we do not need to QT split, try MTT split
    if(depth[0] == 0)
    {
        // in case we do not need to MTT split, just return
        if(mtt_split_mode[0]==0)
        {
            return;
        }
        else
        {
            int cur_mode = mtt_split_mode[0];
            unsigned int next_mode = unsigned(cur_mode) >> unsigned(3);
            unsigned int val_split = (cur_mode - (next_mode << unsigned(3)));
            int s_height = height, s_width = width;

            // BT split
            if(val_split >> unsigned(2) == 0)
            {
                // Horizontal split
                if(val_split >> unsigned(1) == 0){
                    s_height /= 2;
                    tree->bth_split();
                }
                    //Vertical split
                else{
                    s_width /= 2;
                    tree->btv_split();
                }

                // Going into sub split of BT
                auto * sub_split_mode = new int[s_height*s_width]{0};

                // decreasing number of split to perform
                for(int i = 0; i < width*height; i++)
                {
                    mtt_split_mode[i] = unsigned(mtt_split_mode[i]) >> unsigned(3);
                }

                // For the two leaves of the resulting binary tree
                for(int h =0; h < 2; h++)
                {
                    // affecting values to the sub split_map
                    // for each row
                    for(int j = 0; j < s_height; j++)
                    {
                        // for each column
                        for(int k = 0; k < s_width; k++)
                        {
                            if(val_split >> unsigned(1) == 0)
                            {
                                sub_split_mode[k+s_width*j] = mtt_split_mode[k+width*j+width*s_height*h];
                            }
                            else
                            {
                                sub_split_mode[k+s_width*j] = mtt_split_mode[k+width*j+s_width*h];
                            }
                        }

                    }
                    // recursively splitting the sub-CU
                    split_from_depth(depth, sub_split_mode, s_width, s_height, tree->getM_leaves(h));
                }
                delete[] sub_split_mode;
            }
                // TT split
            else{
                // Horizontal split
                if(val_split >> unsigned(1) == 2){
                    tree->tth_split();
                }
                    // Vertical split
                else{
                    tree->ttv_split();
                }

                // Going into sub split in TT
                // decreasing number of split to perform
                for(int i = 0; i < width*height; i++)
                {
                    mtt_split_mode[i] = unsigned(mtt_split_mode[i]) >> unsigned(3);
                }

                // For the three leaves of the resulting ternary tree
                for(int h =0; h < 3; h++)
                {
                    // If we are in middle ternary tree (1/2)
                    if(h==1){
                        // If we are in horizontal split
                        if(val_split >> unsigned(1) == 2){
                            s_height = height / 2;
                        }
                            // Else, we are in vertical split
                        else{
                            s_width = width / 2;
                        }
                    }
                        // Else, we are in the side of ternary tree (1/4)
                    else {
                        // If we are in horizontal split
                        if(val_split >> unsigned(1) == 2){
                            s_height = height / 4;
                        }
                            // Else, we are in vertical split
                        else{
                            s_width = width / 4;
                        }
                    }
                    auto * sub_split_mode = new int[s_height*s_width]{0};
                    // affecting values to the sub split_map
                    // for each row
                    // Because the middle ternary tree is 1/2 of the size of the total tree (1/4 for sides)
                    double shift = 0;
                    if(h==1){
                        shift = 0.25;
                    }
                    else if(h==2){
                        shift = 0.75;
                    }
                    for(int j = 0; j < s_height; j++)
                    {
                        // for each column
                        for(int k = 0; k < s_width; k++)
                        {
                            if(val_split >> unsigned(1) == 2)
                            {
                                sub_split_mode[k+s_width*j] = mtt_split_mode[k+width*j+int(width*height*shift)];
                            }
                            else
                            {
                                sub_split_mode[k+s_width*j] = mtt_split_mode[k+width*j+int(width*shift)];
                            }
                        }

                    }
                    // recursively splitting the sub-CU
                    split_from_depth(depth, sub_split_mode, s_width, s_height, tree->getM_leaves(h));
                    delete[] sub_split_mode;
                }
            }
        }

        return;
    }
        // else performs the quad-split
    else
    {
        // decreasing number of split to perform
        for(int i = 0; i < width*width; i++)
        {
            if(depth[i] !=0)
            {
                depth[i] -=1;
            }
        }

        const int s_size = width / 2;
        tree->qt_split();
        auto * sub_depth = new int[s_size*s_size]{0};
        auto * sub_split_mode = new int[s_size*s_size]{0};

        // for the four leaves of the new quad-tree
        // from upper leaves to lower leaves
        for(int h =0; h < 2; h++)
        {
            // from left leaves to right leaves
            for(int i = 0; i<2; i++)
            {


                // affecting values to the sub depth-map
                // for each row
                for(int j = 0; j < s_size; j++)
                {
                    // for each column
                    for(int k = 0; k < s_size; k++)
                    {
                        sub_depth[k+s_size*j] = depth[k+width*j+s_size*i+s_size*width*h];
                        sub_split_mode[k+s_size*j] = mtt_split_mode[k+width*j+s_size*i+s_size*width*h];
                    }

                }

                // recursively splitting the sub-CU
                split_from_depth(sub_depth, sub_split_mode, s_size, s_size, tree->getM_leaves(i+2*h));
            }
        }
        delete[] sub_depth;
        delete[] sub_split_mode;
    }
}


void PartitionManager::shall_we_split(bool * try_split)
{
    // If there is no leaves, try_split stay false
    if(m_cur_tree->getM_leaves() != nullptr)
    {
        // If subtree is quad, we try quad_split
        if(!m_cur_tree->getM_leaves(0)->isM_is_bin() && !m_cur_tree->getM_leaves(0)->isM_is_tern())
        {
            try_split[0] = true;
        }
            // Else, this means MTT split
        else
        {
            // If we have same width for current tree and subtree
            // that means horizontal splitting
            if(m_cur_tree->getM_width() == m_cur_tree->getM_leaves(0)->getM_width())
            {
                //If we have is_bin true then that means we are in BT
                if(m_cur_tree->getM_leaves(0)->isM_is_bin()) {
                    try_split[1] = true;
                }
                    //Else that means we are in TT
                else try_split[3] = true;
            }
                // If we have same width for current tree and subtree
                // that means vertical splitting
            else if(m_cur_tree->getM_height() == m_cur_tree->getM_leaves(0)->getM_height())
            {
                //If we have is_bin true then that means we are in BT
                if(m_cur_tree->getM_leaves(0)->isM_is_bin()){
                    try_split[2] = true;
                }
                    // Else that means we are in TT
                else try_split[4] = true;
            }
        }
    }
}