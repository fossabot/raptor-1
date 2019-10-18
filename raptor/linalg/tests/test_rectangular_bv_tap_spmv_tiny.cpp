// Copyright (c) 2015-2017, RAPtor Developer Team
// License: Simplified BSD, http://opensource.org/licenses/BSD-2-Clause
#include "gtest/gtest.h"
#include "raptor.hpp"

using namespace raptor;

int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);
    ::testing::InitGoogleTest(&argc, argv);
    int temp = RUN_ALL_TESTS();
    MPI_Finalize();
    return temp;
} // end of main() //

TEST(ParBVectorTinyRectangularTAPSpMVTest, TestsInUtil)
{
    setenv("PPN", "4", 1);

    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    int vecs_in_block = 3;
    int global_rows = 16;
    int global_cols = 6;

    aligned_vector<int> on_proc_idx1, on_proc_idx2;
    aligned_vector<int> off_proc_idx1, off_proc_idx2;
    aligned_vector<double> on_proc_data, off_proc_data;
    aligned_vector<int> off_col_map, on_col_map, row_map;

    if (rank == 0)
    {
        on_proc_idx1.push_back(0);
        on_proc_idx1.push_back(0);
        on_proc_idx1.push_back(0);

        off_proc_idx1.push_back(0);
        off_proc_idx1.push_back(2);
        off_proc_idx1.push_back(3);

        off_proc_idx2.push_back(0);
        off_proc_idx2.push_back(1);
        off_proc_idx2.push_back(1);

        off_proc_data.push_back(1.0);
        off_proc_data.push_back(1.0);
        off_proc_data.push_back(1.0);

        off_col_map.push_back(4);
        off_col_map.push_back(5);

        row_map.push_back(0);
        row_map.push_back(1);
    }
    else if (rank == 1)
    {
        on_proc_idx1.push_back(0);
        on_proc_idx1.push_back(0);
        on_proc_idx1.push_back(1);
        on_proc_idx1.push_back(1);

        on_proc_idx2.push_back(0);

        off_proc_idx1.push_back(0);
        off_proc_idx1.push_back(1);
        off_proc_idx1.push_back(2);
        off_proc_idx1.push_back(3);

        off_proc_idx2.push_back(0);
        off_proc_idx2.push_back(1);
        off_proc_idx2.push_back(2);

        on_proc_data.push_back(1.0);

        off_proc_data.push_back(1.0);
        off_proc_data.push_back(1.0);
        off_proc_data.push_back(1.0);
        
        off_col_map.push_back(2);
        off_col_map.push_back(3);
        off_col_map.push_back(5);

        on_col_map.push_back(0);
        row_map.push_back(2);
        row_map.push_back(3);
        row_map.push_back(4);
    }
    else if (rank == 2)
    {
        on_proc_idx1.push_back(0);
        on_proc_idx1.push_back(1);
        on_proc_idx1.push_back(2);
        on_proc_idx1.push_back(2);

        on_proc_idx2.push_back(0);
        on_proc_idx2.push_back(0);

        off_proc_idx1.push_back(0);
        off_proc_idx1.push_back(1);
        off_proc_idx1.push_back(2);
        off_proc_idx1.push_back(3);

        off_proc_idx2.push_back(2);
        off_proc_idx2.push_back(1);
        off_proc_idx2.push_back(0);
        
        on_proc_data.push_back(1.0);
        on_proc_data.push_back(1.0);

        off_proc_data.push_back(1.0);
        off_proc_data.push_back(1.0);
        off_proc_data.push_back(1.0);
        
        off_col_map.push_back(0);
        off_col_map.push_back(3);
        off_col_map.push_back(5);

        on_col_map.push_back(1);

        row_map.push_back(5);
        row_map.push_back(6);
        row_map.push_back(7);
    }
    else if (rank == 3)
    {
        on_proc_idx1.push_back(0);
        on_proc_idx1.push_back(0);

        off_proc_idx1.push_back(0);
        off_proc_idx1.push_back(1);

        off_proc_idx2.push_back(0);
        
        off_proc_data.push_back(1.0);
        
        off_col_map.push_back(4);

        row_map.push_back(8);
    }
    else if (rank == 4)
    {
        on_proc_idx1.push_back(0);
        on_proc_idx1.push_back(1);
        on_proc_idx1.push_back(1);

        on_proc_idx2.push_back(0);

        off_proc_idx1.push_back(0);
        off_proc_idx1.push_back(1);
        off_proc_idx1.push_back(2);

        off_proc_idx2.push_back(1);
        off_proc_idx2.push_back(0);
        
        on_proc_data.push_back(1.0);

        off_proc_data.push_back(1.0);
        off_proc_data.push_back(1.0);
        
        off_col_map.push_back(0);
        off_col_map.push_back(5);

        on_col_map.push_back(2);

        row_map.push_back(9);
        row_map.push_back(10);
    }
    else if (rank == 5)
    {
        on_proc_idx1.push_back(0);
        on_proc_idx1.push_back(1);

        on_proc_idx2.push_back(0);

        off_proc_idx1.push_back(0);
        off_proc_idx1.push_back(1);

        off_proc_idx2.push_back(0);
        
        on_proc_data.push_back(1.0);
        off_proc_data.push_back(1.0);
        
        off_col_map.push_back(2);

        on_col_map.push_back(3);

        row_map.push_back(11);
    }
    else if (rank == 6)
    {
        on_proc_idx1.push_back(0);
        on_proc_idx1.push_back(1);
        on_proc_idx1.push_back(2);

        on_proc_idx2.push_back(0);
        on_proc_idx2.push_back(0);

        off_proc_idx1.push_back(0);
        off_proc_idx1.push_back(1);
        off_proc_idx1.push_back(2);

        off_proc_idx2.push_back(0);
        off_proc_idx2.push_back(1);
        
        on_proc_data.push_back(1.0);
        on_proc_data.push_back(1.0);
        off_proc_data.push_back(1.0);
        off_proc_data.push_back(1.0);
        
        off_col_map.push_back(1);
        off_col_map.push_back(2);

        on_col_map.push_back(4);

        row_map.push_back(12);
        row_map.push_back(13);
    }
    else if (rank == 7)
    {
        on_proc_idx1.push_back(0);
        on_proc_idx1.push_back(0);
        on_proc_idx1.push_back(1);

        on_proc_idx2.push_back(0);

        off_proc_idx1.push_back(0);
        off_proc_idx1.push_back(2);
        off_proc_idx1.push_back(4);

        off_proc_idx2.push_back(1);
        off_proc_idx2.push_back(2);
        off_proc_idx2.push_back(0);
        off_proc_idx2.push_back(2);
        
        on_proc_data.push_back(1.0);

        off_proc_data.push_back(1.0);
        off_proc_data.push_back(1.0);
        off_proc_data.push_back(1.0);
        off_proc_data.push_back(1.0);
        
        off_col_map.push_back(0);
        off_col_map.push_back(1);
        off_col_map.push_back(3);

        on_col_map.push_back(5);

        row_map.push_back(14);
        row_map.push_back(15);
    }

    int first_local_row, first_local_col;
    if (on_col_map.size())
    {
        first_local_row = row_map[0];
        first_local_col = on_col_map[0];
    }
    else
    {
        first_local_row = 0;
        first_local_col = 0;
    }

    Partition* P_part = new Partition(global_rows, global_cols,
                                on_proc_idx1.size()-1, on_col_map.size(),
                                first_local_row, first_local_col);
    
    ParCSRMatrix* P = new ParCSRMatrix(P_part);

    //ParCSRMatrix* P = new ParCSRMatrix(P_part, on_mat, off_mat);

    P->local_nnz = on_proc_idx2.size() + off_proc_idx2.size();
    P->off_proc_num_cols = off_col_map.size();

    // Update on_proc mat
    P->on_proc->resize(on_proc_idx1.size()-1, on_col_map.size());
    P->on_proc->resize_data(on_proc_data.size());
    P->on_proc->idx2.resize(on_proc_data.size());
    P->on_proc->idx1.resize(on_proc_idx1.size());
    for (int i = 0; i < on_proc_data.size(); i++)
    {
        P->on_proc->idx2[i] = on_proc_idx2[i];
        P->on_proc->vals[i] = on_proc_data[i];
    }
    for (int i = 0; i < on_proc_idx1.size(); i++)
    {
        P->on_proc->idx1[i] = on_proc_idx1[i];
    }

    // Update off_proc mat
    P->off_proc->resize(on_proc_idx1.size()-1, off_col_map.size());
    P->off_proc->resize_data(off_proc_data.size());
    P->off_proc->idx2.resize(off_proc_data.size());
    P->off_proc->idx1.resize(off_proc_idx1.size());
    for (int i = 0; i < off_proc_data.size(); i++)
    {
        P->off_proc->idx2[i] = off_proc_idx2[i];
        P->off_proc->vals[i] = off_proc_data[i];
    }
    for (int i = 0; i < off_proc_idx1.size(); i++)
    {
        P->off_proc->idx1[i] = off_proc_idx1[i];
    }

    // Update row and column maps
    P->off_proc_column_map.resize(off_col_map.size());
    P->on_proc_column_map.resize(on_col_map.size());
    P->local_row_map.resize(row_map.size());
    for (int i = 0; i < off_col_map.size(); i++)
    {
        P->off_proc_column_map[i] = off_col_map[i];
    }
    for (int i = 0; i < on_col_map.size(); i++)
    {
        P->on_proc_column_map[i] = on_col_map[i];
    }
    for (int i = 0; i < row_map.size(); i++)
    {
        P->local_row_map[i] = row_map[i];
    }
    /*for (int p = 0; p < num_procs; p++)
    {
        if (rank == p)
        {
            printf("%d glob_rows %d glob_cols %d on_proc_num_cols %d local_rows %d\n",
                rank, P->global_num_rows, P->global_num_cols, P->on_proc_num_cols, P->local_num_rows);
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }*/

    P->finalize();
   
    P->comm = new ParComm(P->partition, P->off_proc_column_map, P->on_proc_column_map);
    //P->tap_comm = new TAPComm(P->partition, P->off_proc_column_map, P->on_proc_column_map);
    //P->tap_mat_comm = new TAPComm(P->partition, P->off_proc_column_map, P->on_proc_column_map, false);

    setenv("PPN", "16", 1);

    delete P;

} // end of TEST(ParBVectorTinyRectangularTAPSpMVTest, TestsInUtil) //
