/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
void trans64(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int temp = 0;
    int d = 0;
    if (N == 32)
    {
        for (int blockC = 0; blockC < N; blockC+=8)
            for (int blockR = 0; blockR < N; blockR+=8)
            {
                for (int i = blockR; i < blockR + 8; ++i)
                {
                    for (int j = blockC; j < blockC + 8; ++j)
                        if (i != j) 
                            B[j][i] = A[i][j];
                        else 
                        {
                            temp = A[i][j];
                            d = i;
                        }
                    if (blockC == blockR) B[d][d] = temp;
                }
            }
    }
    else if (N == 64)
    {
        trans64(M, N, A, B);
    }
    else 
    {
        for (int blockC = 0; blockC < M; blockC+=8)
            for (int blockR = 0; blockR < N; blockR+=8)
            {
                for (int i = blockR; (i < N) && (i < blockR + 8); ++i)
                {
                    for (int j = blockC; (j < M) && (j < blockC + 8); ++j)
                        if (i != j) 
                            B[j][i] = A[i][j];
                        else 
                        {
                            temp = A[i][j];
                            d = i;
                        }
                    if (blockC == blockR) B[d][d] = temp;
                }
            }
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 
void trans64(int M, int N, int A[N][M], int B[M][N])
{
    int t0, t1, t2, t3, t4, t5, t6, t7, l;

    for (int blockR = 0; blockR < N; blockR+=8)
        for (int blockC = 0; blockC < M; blockC+=8)
        {
            for (int i = blockR; i < blockR + 4; ++i)
            {
                t0 = A[i][blockC+0];
                t1 = A[i][blockC+1];
                t2 = A[i][blockC+2];
                t3 = A[i][blockC+3];
                t4 = A[i][blockC+4];
                t5 = A[i][blockC+5];
                t6 = A[i][blockC+6];
                t7 = A[i][blockC+7];

                B[blockC+0][i] = t0;
                B[blockC+1][i] = t1;
                B[blockC+2][i] = t2;
                B[blockC+3][i] = t3;

                B[blockC+0][i+4] = t4;
                B[blockC+1][i+4] = t5;
                B[blockC+2][i+4] = t6;
                B[blockC+3][i+4] = t7;
            }
            for (int i = blockC; i < blockC + 4; ++i)
            {
                t0 = A[blockR+4][i];
                t1 = A[blockR+5][i];
                t2 = A[blockR+6][i];
                t3 = A[blockR+7][i];

                t4 = B[i][blockR+4];
                t5 = B[i][blockR+5];
                t6 = B[i][blockR+6];
                t7 = B[i][blockR+7];

                B[i][blockR+4] = t0;
                B[i][blockR+5] = t1;
                B[i][blockR+6] = t2;
                B[i][blockR+7] = t3;

                B[blockR+4][i+0] = t4;
                B[blockR+4][i+1] = t5;
                B[blockR+4][i+2] = t6;
                B[blockR+4][i+3] = t7;

                for (l = 0; l < 4; ++l) 
                {
	                B[i+4][i+l+4] = A[i+l+4][i+4];
	            }
            }
            //A[blockR+4][blockC+4]
            /*for (int i = blockR+4; i < blockR + 8; ++i)
            {
                for (int j = blockC + 4; blockC + 8; ++j)
                    if (i != j) 
                        B[j][i] = A[i][j];
                    else 
                    {
                        t0 = A[i][j];
                        t1 = i;
                    }
                if (blockC == blockR) B[t1][t1] = t0;
            }*/
        }
}

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

