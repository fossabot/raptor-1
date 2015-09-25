#include "spmv.hpp"

/**************************************************************
 *****   Sequential Matrix-Vector Multiplication
 **************************************************************
 ***** Performs matrix-vector multiplication on inner indices
 ***** y[inner] = alpha * A[inner, outer] * x[outer] + beta*y[inner]
 *****
 ***** Parameters
 ***** -------------
 ***** A : Matrix*
 *****    Matrix to be multipled
 ***** x : Vector*
 *****    Vector to be multiplied
 ***** y : Vector*
 *****    Vector result is added to
 ***** alpha : data_t
 *****    Scalar to multipy A*x by
 ***** beta : data_t
 *****    Scalar to multiply original y by
 **************************************************************/
void seq_inner_spmv(Matrix* A, Vector* x, Vector* y, data_t alpha, data_t beta)
{
    index_t alpha_zero = (fabs(alpha) < zero_tol);
    index_t alpha_one = (fabs(alpha - 1.0) < zero_tol);
    index_t alpha_neg_one = (fabs(alpha + 1.0) < zero_tol);

    index_t beta_zero = (fabs(beta) < zero_tol);
    index_t beta_one = (fabs(beta - 1.0) < zero_tol);
    index_t beta_neg_one = (fabs(beta + 1.0) < zero_tol);

    index_t* ptr = A->indptr.data();
    index_t* idx = A->indices.data();
    data_t* values = A->data.data();
    index_t num_cols = A->n_cols;
    index_t num_rows = A->n_rows;
    index_t n_outer = A->n_outer;
    index_t n_inner = A->n_inner;

    index_t ptr_start;
    index_t ptr_end;
    data_t x_val;
    data_t* y_data = y->data();
    data_t* x_data = x->data();


    if (alpha_one)
    {
        if (!beta_one)
        {
            for (index_t inner = 0; inner < n_inner; inner++)
            {
                y_data[inner] = beta * y_data[inner];
            }
        }
        for (index_t outer = 0; outer < n_outer; outer++)
        {
            ptr_start = ptr[outer];
            ptr_end = ptr[outer + 1];
            data_t x_val = x_data[outer];
            for (index_t j = ptr_start; j < ptr_end; j++)
            {
               index_t inner = idx[j];
                y_data[inner] += values[j] * x_val;
            }
        }
    }
    else if (alpha_neg_one)
    {
        if (!beta_one)
        {
            for (index_t inner = 0; inner < n_inner; inner++)
            {
                y_data[inner] = beta * y_data[inner];
            }
        }
        for (index_t outer = 0; outer < n_outer; outer++)
        {
            ptr_start = ptr[outer];
            ptr_end = ptr[outer + 1];
            data_t x_val = x_data[outer];
            for (index_t j = ptr_start; j < ptr_end; j++)
            {
                index_t inner = idx[j];
                y_data[inner] -= values[j] * x_val;
            }
        }
    }
    else if (alpha_zero)
    {
        if (beta_zero)
        {
            for (index_t inner = 0; inner < n_inner; inner++)
            {
                y_data[inner] = 0.0;
            }
        }
        else if (!beta_one)
        {
            for (index_t inner = 0; inner < n_inner; inner++)
            {
                y_data[inner] = beta * y_data[inner];
            }
        }
    }
    else
    {
        if (!beta_one)
        {
            for (index_t inner = 0; inner < n_inner; inner++)
            {
                y_data[inner] = beta * y_data[inner];
            }
        }
        for (index_t outer = 0; outer < n_outer; outer++)
        {
            ptr_start = ptr[outer];
            ptr_end = ptr[outer + 1];
            data_t x_val = x_data[outer];
            for (index_t j = ptr_start; j < ptr_end; j++)
            {
                index_t inner = idx[j];
                y_data[inner] += alpha * values[j] * x_val;
            }
        }
    }
}

/**************************************************************
 *****   Partial Sequential Matrix-Vector Multiplication
 **************************************************************
 ***** Performs partial matrix-vector multiplication on inner indices
 ***** y[inner] = alpha * A[inner, outer] * x[outer] + beta*y[inner]
 ***** for a portion of the outer indices.
 *****
 ***** Parameters
 ***** -------------
 ***** A : Matrix*
 *****    Matrix to be multipled
 ***** x : Vector*
 *****    Vector to be multiplied
 ***** y : Vector*
 *****    Vector result is added to
 ***** alpha : data_t
 *****    Scalar to multipy A*x by
 ***** beta : data_t
 *****    Scalar to multiply original y by
 ***** outer_list : std::vector<index_t>
 *****    Outer indices to multiply
 **************************************************************/
void seq_inner_spmv(Matrix* A, Vector* x, Vector* y, data_t alpha, data_t beta, std::vector<index_t> outer_list)
{

    // Get MPI Information
    index_t rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    index_t alpha_zero = (fabs(alpha) < zero_tol);
    index_t alpha_one = (fabs(alpha - 1.0) < zero_tol);
    index_t alpha_neg_one = (fabs(alpha + 1.0) < zero_tol);

    index_t beta_zero = (fabs(beta) < zero_tol);
    index_t beta_one = (fabs(beta - 1.0) < zero_tol);
    index_t beta_neg_one = (fabs(beta + 1.0) < zero_tol);

    index_t* ptr = A->indptr.data();
    index_t* idx = A->indices.data();
    data_t* values = A->data.data();
    index_t num_cols = A->n_cols;
    index_t num_rows = A->n_rows;
    index_t n_outer = A->n_outer;
    index_t n_inner = A->n_inner;
    index_t num_nonzeros = A->nnz;

    index_t ptr_start;
    index_t ptr_end;
    data_t x_val;
    data_t* y_data = y->data();
    data_t* x_data = x->data();

    if (alpha_one)
    {
        if (!beta_one)
        {
            for (index_t inner = 0; inner < n_inner; inner++)
            {
                y_data[inner] = beta * y_data[inner];
            }
        }
        // Ax + y
        for (index_t i = 0; i < outer_list.size(); i++)
        {
            index_t outer = outer_list[i];
            ptr_start = ptr[outer];
            ptr_end = ptr[outer + 1];
            data_t x_val = x_data[i];

            for (index_t j = ptr_start; j < ptr_end; j++)
            {
                index_t inner = idx[j];
                y_data[inner] += values[j] * x_val;
            }
        }
    }
    else if (alpha_neg_one)
    {
        if (!beta_one)
        {
            for (index_t inner = 0; inner < n_inner; inner++)
            {
                y_data[inner] = beta * y_data[inner];
            }
        }
        for (index_t i = 0; i < outer_list.size(); i++)
        {
            index_t outer = outer_list[i];
            ptr_start = ptr[outer];
            ptr_end = ptr[outer + 1];
            data_t x_val = x_data[i];
            for (index_t j = ptr_start; j < ptr_end; j++)
            {
                index_t inner = idx[j];
                y_data[inner] -= values[j] * x_val;
            }
        }
    }
    else if (alpha_zero)
    {
        if (beta_zero)
        {
            for (index_t inner = 0; inner < n_inner; inner++)
            {
                y_data[inner] = 0.0;
            }
        }
        else if (!beta_one)
        {
            for (index_t inner = 0; inner < n_inner; inner++)
            {
                y_data[inner] = beta * y_data[inner];
            }
        }
    }
    else
    {
        if (!beta_one)
        {
            for (index_t inner = 0; inner < n_inner; inner++)
            {
                y_data[inner] = beta * y_data[inner];
            }
        }
        for (index_t i = 0; i < outer_list.size(); i++)
        {
            index_t outer = outer_list[i];
            ptr_start = ptr[outer];
            ptr_end = ptr[outer + 1];
            data_t x_val = x_data[i];
            for (index_t j = ptr_start; j < ptr_end; j++)
            {
                index_t inner = idx[j];
                y_data[inner] += alpha * values[j] * x_val;
            }
        }
    }
}

/**************************************************************
 *****   Sequential Matrix-Vector Multiplication
 **************************************************************
 ***** Performs matrix-vector multiplication on outer indices
 ***** y[outer] = alpha * A[outer, inner] * x[inner] + beta*y[outer]
 *****
 ***** Parameters
 ***** -------------
 ***** A : Matrix*
 *****    Matrix to be multipled
 ***** x : Vector*
 *****    Vector to be multiplied
 ***** y : Vector*
 *****    Vector result is added to
 ***** alpha : data_t
 *****    Scalar to multipy A*x by
 ***** beta : data_t
 *****    Scalar to multiply original y by
 **************************************************************/
void seq_outer_spmv(Matrix* A, Vector* x, Vector* y, data_t alpha, data_t beta)
{
    // Get MPI Information
    index_t rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    index_t alpha_zero = (fabs(alpha) < zero_tol);
    index_t alpha_one = (fabs(alpha - 1.0) < zero_tol);
    index_t alpha_neg_one = (fabs(alpha + 1.0) < zero_tol);

    index_t beta_zero = (fabs(beta) < zero_tol);
    index_t beta_one = (fabs(beta - 1.0) < zero_tol);
    index_t beta_neg_one = (fabs(beta + 1.0) < zero_tol);

    index_t* ptr = A->indptr.data();
    index_t* idx = A->indices.data();
    data_t* values = A->data.data();
    index_t num_cols = A->n_cols;
    index_t num_rows = A->n_rows;
    index_t n_outer = A->n_outer;
    index_t n_inner = A->n_inner;
    index_t num_nonzeros = A->nnz;

    data_t* y_data = y->data();
    data_t* x_data = x->data();

    index_t ptr_start;
    index_t ptr_end;

    if (alpha_one)
    {
        if (beta_one)
        {
            //Ax + y
            for (index_t outer = 0; outer < n_outer; outer++)
            {
                ptr_start = ptr[outer];
                ptr_end = ptr[outer+1];

                for (index_t j = ptr_start; j < ptr_end; j++)
                {
                    index_t inner = idx[j];
                    y_data[outer] += values[j] * x_data[inner];
                }
            }
        }
        else if (beta_zero)
        {
            //Ax
            for (index_t outer = 0; outer < n_outer; outer++)
            {
                ptr_start = ptr[outer];
                ptr_end = ptr[outer+1];

                if (ptr_start < ptr_end)
                {
                    y_data[outer] = values[ptr_start] * x_data[idx[ptr_start]];
                }
                else
                {
                    y_data[outer] = 0.0;
                }

                for (index_t j = ptr_start + 1; j < ptr_end; j++)
                {
                    index_t inner = idx[j];
                    y_data[outer] += values[j] * x_data[inner];
                }
            }
        }
        else
        {
            //Ax + beta * y
            for (index_t outer = 0; outer < n_outer; outer++)
            {
                ptr_start = ptr[outer];
                ptr_end = ptr[outer+1];

                y_data[outer] *= beta;

                for (index_t j = ptr_start + 1; j < ptr_end; j++)
                {
                    index_t inner = idx[j];
                    y_data[outer] += values[j] * x_data[inner];
                }
            }
        }
    }
    else if (alpha_neg_one)
    {
        if (beta_one)
        {
            //-Ax + y
            for (index_t outer = 0; outer < n_outer; outer++)
            {
                ptr_start = ptr[outer];
                ptr_end = ptr[outer+1];

                for (index_t j = ptr_start; j < ptr_end; j++)
                {
                    index_t inner = idx[j];
                    y_data[outer] -= values[j] * x_data[inner];
                }
            }
        }
        else if (beta_zero)
        {
            //Ax
            for (index_t outer = 0; outer < n_outer; outer++)
            {
                ptr_start = ptr[outer];
                ptr_end = ptr[outer+1];

                if (ptr_start < ptr_end)
                {
                    y_data[outer] = - (values[ptr_start] * x_data[idx[ptr_start]]);
                }
                else
                {
                    y_data[outer] = 0.0;
                }

                for (index_t j = ptr_start + 1; j < ptr_end; j++)
                {
                    index_t inner = idx[j];
                    y_data[outer] -= values[j] * x_data[inner];
                }
            }
        }
        else
        {
            //Ax + beta * y
            for (index_t outer = 0; outer < n_outer; outer++)
            {
                ptr_start = ptr[outer];
                ptr_end = ptr[outer+1];

                y_data[outer] *= beta;

                for (index_t j = ptr_start + 1; j < ptr_end; j++)
                {
                    index_t inner = idx[j];
                    y_data[outer] -= values[j] * x_data[inner];
                }
            }
        }
    }
    else if (alpha_zero)
    {
        if (beta_zero)
        {
            //return 0
            for (index_t outer = 0; outer < n_outer; outer++)
            {
                y_data[outer] = 0.0;
            }
        }
        else if (!beta_one)
        {
            //beta * y
            for (index_t outer = 0; outer < n_outer; outer++)
            {
                y_data[outer] *= beta;
            }
        }
    }
    else
    {
        if (beta_one)
        {
            //alpha*Ax + y
            for (index_t outer = 0; outer < n_outer; outer++)
            {
                ptr_start = ptr[outer];
                ptr_end = ptr[outer+1];

                for (index_t j = ptr_start; j < ptr_end; j++)
                {
                    index_t inner = idx[j];
                    y_data[outer] += alpha * values[j] * x_data[inner];
                }
            }
        }
        else if (beta_zero)
        {
            //alpha*Ax
            for (index_t outer = 0; outer < n_outer; outer++)
            {
                ptr_start = ptr[outer];
                ptr_end = ptr[outer+1];

                if (ptr_start < ptr_end)
                {
                    y_data[outer] = alpha * values[ptr_start] * x_data[idx[ptr_start]];
                }
                else
                {
                    y_data[outer] = 0.0;
                }

                for (index_t j = ptr_start + 1; j < ptr_end; j++)
                {
                    index_t inner = idx[j];
                    y_data[outer] += alpha * values[j] * x_data[inner];
                }
            }
        }
        else
        {
            //alpha * Ax + beta * y
            for (index_t outer = 0; outer < n_outer; outer++)
            {
                ptr_start = ptr[outer];
                ptr_end = ptr[outer+1];

                y_data[outer] *= beta;

                for (index_t j = ptr_start + 1; j < ptr_end; j++)
                {
                    index_t inner = idx[j];
                    y_data[outer] += alpha * values[j] * x_data[inner];
                }
            }
        }
    }
}

/**************************************************************
 *****   Partial Sequential Matrix-Vector Multiplication
 **************************************************************
 ***** Performs partial matrix-vector multiplication on inner indices
 ***** y[outer] = alpha * A[outer, inner] * x[inner] + beta*y[outer]
 ***** for a portion of the outer indices.
 *****
 ***** Parameters
 ***** -------------
 ***** A : Matrix*
 *****    Matrix to be multipled
 ***** x : Vector*
 *****    Vector to be multiplied
 ***** y : Vector*
 *****    Vector result is added to
 ***** alpha : data_t
 *****    Scalar to multipy A*x by
 ***** beta : data_t
 *****    Scalar to multiply original y by
 ***** outer_list : std::vector<index_t>
 *****    Outer indices to multiply
 **************************************************************/
void seq_outer_spmv(Matrix* A, Vector* x, Vector* y, data_t alpha, data_t beta, std::vector<index_t> outer_list)
{
    // Get MPI Information
    index_t rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    index_t alpha_zero = (fabs(alpha) < zero_tol);
    index_t alpha_one = (fabs(alpha - 1.0) < zero_tol);
    index_t alpha_neg_one = (fabs(alpha + 1.0) < zero_tol);

    index_t beta_zero = (fabs(beta) < zero_tol);
    index_t beta_one = (fabs(beta - 1.0) < zero_tol);
    index_t beta_neg_one = (fabs(beta + 1.0) < zero_tol);

    index_t* ptr = A->indptr.data();
    index_t* idx = A->indices.data();
    data_t* values = A->data.data();
    index_t num_cols = A->n_cols;
    index_t num_rows = A->n_rows;
    index_t n_outer = A->n_outer;
    index_t n_inner = A->n_inner;
    index_t num_nonzeros = A->nnz;

    data_t* y_data = y->data();
    data_t* x_data = x->data();

    index_t ptr_start;
    index_t ptr_end;

    if (alpha_one)
    {
        if (beta_one)
        {
            //Ax + y
            for (index_t i = 0; i < outer_list.size(); i++)
            {
                index_t outer = outer_list[i];
                ptr_start = ptr[outer];
                ptr_end = ptr[outer+1];

                for (index_t j = ptr_start; j < ptr_end; j++)
                {
                    index_t inner = idx[j];
                    y_data[i] += values[j] * x_data[inner];
                }
            }
        }
        else
        {
            //Ax + beta*y
            for (index_t i = 0; i < outer_list.size(); i++)
            {
                index_t outer = outer_list[i];
                ptr_start = ptr[outer];
                ptr_end = ptr[outer+1];

                y_data[i] *= beta;

                for (index_t j = ptr_start; j < ptr_end; j++)
                {
                    index_t inner = idx[j];
                    y_data[i] += values[j] * x_data[inner];
                }
            }
        }
    }
    else if (alpha_neg_one)
    {
        if (beta_one)
        {
            //Ax + y
            for (index_t i = 0; i < outer_list.size(); i++)
            {
                index_t outer = outer_list[i];
                ptr_start = ptr[outer];
                ptr_end = ptr[outer+1];

                for (index_t j = ptr_start; j < ptr_end; j++)
                {
                    index_t inner = idx[j];
                    y_data[i] -= values[j] * x_data[inner];
                }
            }
        }
        else
        {
            //Ax + beta*y
            for (index_t i = 0; i < outer_list.size(); i++)
            {
                index_t outer = outer_list[i];
                ptr_start = ptr[outer];
                ptr_end = ptr[outer+1];

                y_data[i] *= beta;

                for (index_t j = ptr_start; j < ptr_end; j++)
                {
                    index_t inner = idx[j];
                    y_data[i] -= values[j] * x_data[inner];
                }
            }
        }
    }
    else if (alpha_zero)
    {
        if (beta_zero)
        {
            //return 0
            for (index_t i = 0; i < outer_list.size(); i++)
            {
                y_data[i] = 0.0;
            }
        }
        else if (!beta_one)
        {
            //beta * y
            for (index_t i = 0; i < outer_list.size(); i++)
            {
                y_data[i] *= beta;
            }
        }
    }
    else
    {
        if (beta_one)
        {
            //Ax + y
            for (index_t i = 0; i < outer_list.size(); i++)
            {
                index_t outer = outer_list[i];
                ptr_start = ptr[outer];
                ptr_end = ptr[outer+1];

                for (index_t j = ptr_start; j < ptr_end; j++)
                {
                    index_t inner = idx[j];
                    y_data[i] += alpha * values[j] * x_data[inner];
                }
            }
        }
        else
        {
            //Ax + beta*y
            for (index_t i = 0; i < outer_list.size(); i++)
            {
                index_t outer = outer_list[i];
                ptr_start = ptr[outer];
                ptr_end = ptr[outer+1];

                y_data[i] *= beta;

                for (index_t j = ptr_start; j < ptr_end; j++)
                {
                    index_t inner = idx[j];
                    y_data[i] += alpha * values[j] * x_data[inner];
                }
            }
        }
    }
}

/**************************************************************
 *****   Sequential Matrix-Vector Multiplication
 **************************************************************
 ***** Performs partial matrix-vector multiplication, calling
 ***** method appropriate for matrix format
 *****
 ***** Parameters
 ***** -------------
 ***** A : Matrix*
 *****    Matrix to be multipled
 ***** x : Vector*
 *****    Vector to be multiplied
 ***** y : Vector*
 *****    Vector result is added to
 ***** alpha : data_t
 *****    Scalar to multipy A*x by
 ***** beta : data_t
 *****    Scalar to multiply original y by
 **************************************************************/
void sequential_spmv(Matrix* A, Vector* x, Vector* y, data_t alpha, data_t beta)
{
    if (A->format == CSR)
    {
        seq_outer_spmv(A, x, y, alpha, beta);
    }
    else
    {
        seq_inner_spmv(A, x, y, alpha, beta);
    }   
}

/**************************************************************
 *****   Partial Sequential Matrix-Vector Multiplication
 **************************************************************
 ***** Performs partial matrix-vector multiplication, calling
 ***** method appropriate for matrix format
 *****
 ***** Parameters
 ***** -------------
 ***** A : Matrix*
 *****    Matrix to be multipled
 ***** x : Vector*
 *****    Vector to be multiplied
 ***** y : Vector*
 *****    Vector result is added to
 ***** alpha : data_t
 *****    Scalar to multipy A*x by
 ***** beta : data_t
 *****    Scalar to multiply original y by
 ***** outer_list : std::vector<index_t>
 *****    Outer indices to multiply
 **************************************************************/
void sequential_spmv(Matrix* A, Vector* x, Vector* y, data_t alpha, data_t beta, std::vector<index_t> col_list)
{
    if (A->format == CSR)
    {
        
    }
    else
    {
        seq_inner_spmv(A, x, y, alpha, beta, col_list);
    }   
}

/**************************************************************
 *****   Sequential Transpose Matrix-Vector Multiplication
 **************************************************************
 ***** Performs partial transpose matrix-vector multiplication, 
 ***** calling method appropriate for matrix format
 *****
 ***** Parameters
 ***** -------------
 ***** A : Matrix*
 *****    Matrix to be multipled
 ***** x : Vector*
 *****    Vector to be multiplied
 ***** y : Vector*
 *****    Vector result is added to
 ***** alpha : data_t
 *****    Scalar to multipy A*x by
 ***** beta : data_t
 *****    Scalar to multiply original y by
 **************************************************************/
void sequential_spmv_T(Matrix* A, Vector* x, Vector* y, data_t alpha, data_t beta)
{
    if (A->format == CSR)
    {
        seq_inner_spmv(A, x, y, alpha, beta);
    }
    else
    {
        seq_outer_spmv(A, x, y, alpha, beta);
    }   
}

/**************************************************************
 *****   Partial Sequential Transpose Matrix-Vector Multiplication
 **************************************************************
 ***** Performs partial transpose matrix-vector multiplication,
 ***** calling method appropriate for matrix format
 *****
 ***** Parameters
 ***** -------------
 ***** A : Matrix*
 *****    Matrix to be multipled
 ***** x : Vector*
 *****    Vector to be multiplied
 ***** y : Vector*
 *****    Vector result is added to
 ***** alpha : data_t
 *****    Scalar to multipy A*x by
 ***** beta : data_t
 *****    Scalar to multiply original y by
 ***** outer_list : std::vector<index_t>
 *****    Outer indices to multiply
 **************************************************************/
void sequential_spmv_T(Matrix* A, Vector* x, Vector* y, data_t alpha, data_t beta, std::vector<index_t> col_list)
{
    if (A->format == CSR)
    {

    }
    else
    {
        seq_outer_spmv(A, x, y, alpha, beta, col_list);
    }   
}
