#include "matmult.hpp"

/**************************************************************
 *****   Map To Global
 **************************************************************
 ***** Calculates the dot product of two sparse vectors
 ***** alpha = u^T*v
 *****
 ***** Parameters
 ***** -------------
 ***** i : index_t
 *****    Index to be mapped from local to global
 ***** map : std::vector<index_t>
 *****    Vector that maps from local to global
 *****
 ***** Returns
 ***** -------------
 ***** index_t :
 *****    Global index of i
 **************************************************************/
index_t map_to_global(index_t i, std::vector<index_t> map)
{
    return map[i];
}

/**************************************************************
 *****   Map To Global
 **************************************************************
 ***** Calculates the dot product of two sparse vectors
 ***** alpha = u^T*v
 *****
 ***** Parameters
 ***** -------------
 ***** i : index_t
 *****    Index to be mapped from local to global
 ***** map : index_t
 *****    Additional integer to map local to global
 *****
 ***** Returns
 ***** -------------
 ***** index_t :
 *****    Global index of i
 **************************************************************/
index_t map_to_global(index_t i, index_t addition)
{
    return i + addition;
}

/**************************************************************
 *****   Dot Product
 **************************************************************
 ***** Calculates the dot product of two sparse vectors
 ***** alpha = u^T*v
 *****
 ***** Parameters
 ***** -------------
 ***** size_u : index_t
 *****    Number of elements in vector u
 ***** size_v : index_t 
 *****    Number of elements in vector v
 ***** local_u : index_t* 
 *****    Indices of nonzeros in vector u
 ***** local_v : index_t*
 *****    Indices of nonzeros in vector v
 ***** data_u : data_t*
 *****    Values of nonzeros in vector u
 ***** data_v : data_t*
 *****    Values of nonzeros in vector v
 ***** map_u : UType
 *****    Maps indices of u from local to global
 ***** map_v : VType
 *****    Maps indices of v from local to global
 **************************************************************/
template <typename UType, typename VType>
data_t dot(index_t size_u, index_t size_v, index_t* local_u, 
    index_t* local_v, data_t* data_u, data_t* data_v,
    UType map_u, VType map_v)
{
    index_t ctr_u = 0;
    index_t ctr_v = 0;
    data_t result = 0.0;

    // Get index of first nonzero in u and v
    index_t k_u = map_to_global(local_u[ctr_u], map_u);
    index_t k_v = map_to_global(local_v[ctr_v], map_v);
    while (ctr_u < size_u && ctr_v < size_v)
    {
        // If indices of nonzeros are equal, add to product
        if (k_u == k_v)
        {
            result += data_u[ctr_u++] * data_v[ctr_v++];
            
            // If next element exists in each vector,
            // find index of next element
            if (ctr_u < size_u && ctr_v < size_v)
            {
                k_u = map_to_global(local_u[ctr_u], map_u);
                k_v = map_to_global(local_v[ctr_v], map_v);
            }
        }

        // If index of u > index of v, look at next in v
        else if (k_u > k_v)
        {
            ctr_v++;
            if (ctr_v < size_v)
            {
                k_v = map_to_global(local_v[ctr_v], map_v);
            }
        }

        // If index of v > index of u, look at next in u
        else
        {
            ctr_u++;
            if (ctr_u < size_u)
            {
                k_u = map_to_global(local_u[ctr_u], map_u);
            }
        }
    }
    return result;
}

/**************************************************************
 *****   Dot Product
 **************************************************************
 ***** Pulls rows/columns from matrices, and computes
 ***** dot product of these
 *****
 ***** Parameters
 ***** -------------
 ***** A : Matrix*
 *****    Matrix to extract row from ( u )
 ***** B : Matrix*
 *****    Matrix to extract column from ( v )
 ***** map_A : AType
 *****    Maps local indices of A to global
 ***** map_B : BType
 *****    Maps local indices of B to global
 ***** col_start : index_t 
 *****    Index of first element in column of A
 ***** col_end : index_t 
 *****    Index of first element in next column of A
 ***** row_start : index_t
 *****    Index of first element in row of B
 ***** row_end : index_t
 *****    Index of first element in next row of B
 **************************************************************/
template <typename AType, typename BType>
data_t dot(Matrix* A, Matrix* B, AType map_A, BType map_B, 
        index_t col_start, index_t col_end,
        index_t row_start, index_t row_end)
{
    index_t size_A = row_end - row_start;
    index_t size_B = col_end - col_start;

    index_t* local_A = &(A->indices[row_start]);
    index_t* local_B = &(B->indices[col_start]);

    data_t* data_A = &(A->data[row_start]);
    data_t* data_B = &(B->data[col_start]);

    return dot<AType, BType>(size_A, size_B, local_A, local_B,
        data_A, data_B, map_A, map_B);
}

/**************************************************************
 *****   Partial Sequential Matrix-Matrix Multiplication
 **************************************************************
 ***** Performs a partial matmult, multiplying Matrix A
 ***** by a single column of B
 *****
 ***** Parameters
 ***** -------------
 ***** A : Matrix*
 *****    Matrix to be multipled (on left)
 ***** B : Matrix*
 *****    Matrix to have single column multiplied (on right)
 ***** C : ParMatrix*
 *****    Parallel Matrix result is added to
 ***** map_A : AType
 *****    Maps local rows of A to global rows 
 ***** map_B : BType 
 *****    Maps local columns of B to global columns
 ***** map_C : CType
 *****    Maps local resulting column to global
 ***** col : index_t 
 *****    Column of B to be multiplied
 **************************************************************/
template <typename AType, typename BType, typename CType>
void seq_mm(Matrix* A, Matrix* B, ParMatrix* C, AType map_A,
        BType map_B, CType map_C, index_t col)
{
    index_t col_start = B->indptr[col];
    index_t col_end = B->indptr[col+1];
    for (index_t row = 0; row < A->n_rows; row++)
    {
        index_t row_start = A->indptr[row];
        index_t row_end = A->indptr[row+1];

        data_t cij = dot<AType, BType> (A, B, map_A,
             map_B, col_start, col_end, row_start, row_end);
        index_t global_col = map_to_global(col, map_C);
        C->add_value(row, global_col, cij);
    }
}

/**************************************************************
 *****   Sequential Matrix-Matrix Multiplication
 **************************************************************
 ***** Performs matrix-matrix multiplication A*B
 *****
 ***** Parameters
 ***** -------------
 ***** A : Matrix*
 *****    Matrix to be multipled (on left)
 ***** B : Matrix*
 *****    Matrix to be multiplied (on right)
 ***** C : ParMatrix*
 *****    Parallel Matrix result is added to
 ***** map_A : AType
 *****    Maps local rows of A to global rows 
 ***** map_B : BType 
 *****    Maps local columns of B to global columns
 ***** map_C : CType
 *****    Maps local resulting column to global
 **************************************************************/
template <typename AType, typename BType, typename CType>
void seq_mm(Matrix* A, Matrix* B, ParMatrix* C, AType map_A,
        BType map_B, CType map_C)
{
    data_t cij;
    index_t global_col;

    for (index_t col = 0; col < B->n_cols; col++)
    {
        seq_mm<AType, BType, CType> (A, B, C, map_A,
            map_B, map_C, col);
    }
}

