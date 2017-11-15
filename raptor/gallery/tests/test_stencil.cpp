// Copyright (c) 2015-2017, RAPtor Developer Team, University of Illinois at Urbana-Champaign
// License: Simplified BSD, http://opensource.org/licenses/BSD-2-Clause

#include "gtest/gtest.h"
#include "core/types.hpp"
#include "core/matrix.hpp"
#include "gallery/diffusion.hpp"
#include "gallery/stencil.hpp"

using namespace raptor;

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} // end of main() //

TEST(StencilTest, TestsInGallery)
{
    // Create A from diffusion stencil
    int dim = 2;
    std::vector<int> grid(2, 4);
    double eps = 0.001;
    double theta = M_PI / 8.0;
    double* stencil = diffusion_stencil_2d(eps, theta);
    CSRMatrix* A = stencil_grid(stencil, grid.data(), dim);
    delete[] stencil;

    std::vector<double> A_python(16 * 16, 0);
    std::vector<double> A_dense(16 * 16, 0);

    // Add values ot A_python
    int rows[100] = {0, 0, 0, 0, 
        1, 1, 1, 1, 1, 1, 
        2, 2, 2, 2, 2, 2,
        3, 3, 3, 3,
        4, 4, 4, 4, 4, 4,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 
        6, 6, 6, 6, 6, 6, 6, 6, 6,
        7, 7, 7, 7, 7, 7,
        8, 8, 8, 8, 8, 8,
        9, 9, 9, 9, 9, 9, 9, 9, 9,
        10, 10, 10, 10, 10, 10, 10, 10, 10,
        11, 11, 11, 11, 11, 11,
        12, 12, 12, 12,
        13, 13, 13, 13, 13, 13,
        14, 14, 14, 14, 14, 14,
        15, 15, 15, 15};

    int cols[100] = {0, 1, 4, 5,
        0, 1, 2, 4, 5, 6,
        1, 2, 3, 5, 6, 7,
        2, 3, 6, 7, 
        0, 1, 4, 5, 8, 9,
        0, 1, 2, 4, 5, 6, 8, 9, 10,
        1, 2, 3, 5, 6, 7, 9, 10, 11,
        2, 3, 6, 7, 10, 11, 
        4, 5, 8, 9, 12, 13, 
        4, 5, 6, 8, 9, 10, 12, 13, 14, 
        5, 6, 7, 9, 10, 11, 13, 14, 15,
        6, 7, 10, 11, 14, 15,
        8, 9, 12, 13,
        8, 9, 10, 12, 13, 14,
        9, 10, 11, 13, 14, 15,
        10, 11, 14, 15};

    double data[100] = {1.33466666667, 0.186366503869, -0.520033170536, -0.343433251935, 0.186366503869, 1.33466666667, 0.186366503869, 0.00976658526801, -0.520033170536, -0.343433251935, 0.186366503869, 1.33466666667, 0.186366503869, 0.00976658526801, -0.520033170536, -0.343433251935, 0.186366503869, 1.33466666667, 0.00976658526801, -0.520033170536, -0.520033170536, 0.00976658526801, 1.33466666667, 0.186366503869, -0.520033170536, -0.343433251935, -0.343433251935, -0.520033170536, 0.00976658526801, 0.186366503869, 1.33466666667, 0.186366503869, 0.00976658526801, -0.520033170536, -0.343433251935, -0.343433251935, -0.520033170536, 0.00976658526801, 0.186366503869, 1.33466666667, 0.186366503869, 0.00976658526801, -0.520033170536, -0.343433251935, -0.343433251935, -0.520033170536, 0.186366503869, 1.33466666667, 0.00976658526801, -0.520033170536, -0.520033170536, 0.00976658526801, 1.33466666667, 0.186366503869, -0.520033170536, -0.343433251935, -0.343433251935, -0.520033170536, 0.00976658526801, 0.186366503869, 1.33466666667, 0.186366503869, 0.00976658526801, -0.520033170536, -0.343433251935, -0.343433251935, -0.520033170536, 0.00976658526801, 0.186366503869, 1.33466666667, 0.186366503869, 0.00976658526801, -0.520033170536, -0.343433251935, -0.343433251935, -0.520033170536, 0.186366503869, 1.33466666667, 0.00976658526801, -0.520033170536, -0.520033170536, 0.00976658526801, 1.33466666667, 0.186366503869, -0.343433251935, -0.520033170536, 0.00976658526801, 0.186366503869, 1.33466666667, 0.186366503869, -0.343433251935, -0.520033170536, 0.00976658526801, 0.186366503869, 1.33466666667, 0.186366503869, -0.343433251935, -0.520033170536, 0.186366503869, 1.33466666667};

    for (int i = 0; i < 100; i++)
    {
        int row = rows[i];
        int col = cols[i];
        A_python[row*16 + col] = data[i];
    }


    for (int i = 0; i < A->n_rows; i++)
    {
        int row_start = A->idx1[i];
        int row_end = A->idx1[i+1];
        for (int j = row_start; j < row_end; j++)
        {
            int col = A->idx2[j];
            A_dense[i*16 + col] = A->vals[j];
        }
    }

    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            ASSERT_NEAR(A_python[i*16+j], A_dense[i*16+j], 1e-06);
        }
    }

} // end of TEST(StencilTest, TestsInGallery)//

