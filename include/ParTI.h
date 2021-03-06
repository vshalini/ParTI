/*
    This file is part of ParTI!.

    ParTI! is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    ParTI! is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with ParTI!.
    If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PARTI_H_INCLUDED
#define PARTI_H_INCLUDED

#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Define sptScalar as 32-bit float
 *
 * You can adjust this type to suit the application
 */
typedef float sptScalar;

/**
 * Dense dynamic array of scalars
 */
typedef struct {
    size_t    len;   /// length
    size_t    cap;   /// capacity
    sptScalar *data; /// data
} sptVector;

/**
 * Dense dynamic array of size_t's
 */
typedef struct {
    size_t len;   /// length
    size_t cap;   /// capacity
    size_t *data; /// data
} sptSizeVector;

/**
 * Dense matrix type
 */
typedef struct {
    size_t    nrows;   /// # rows
    size_t    ncols;   /// # columns
    size_t    cap;     /// # of allocated rows
    size_t    stride;  /// ncols rounded up to 8
    sptScalar *values; /// values, length cap*stride
} sptMatrix;

/**
 * Sparse matrix type, COO format
 */
typedef struct {
    size_t        nrows;  /// # rows
    size_t        ncols;  /// # colums
    size_t        nnz;    /// # non-zeros
    sptSizeVector rowind; /// row indices, length nnz
    sptSizeVector colind; /// column indices, length nnz
    sptVector     values; /// non-zero values, length nnz
} sptSparseMatrix;

/**
 * Sparse tensor type
 */
typedef struct {
    size_t        nmodes;      /// # modes
    size_t        *sortorder;  /// the order in which the indices are sorted
    size_t        *ndims;      /// size of each mode, length nmodes
    size_t        nnz;         /// # non-zeros
    sptSizeVector *inds;       /// indices of each element, length [nmodes][nnz]
    sptVector     values;      /// non-zero values, length nnz
} sptSparseTensor;

/**
 * Semi-sparse tensor type
 * The chosen mode is dense, while other modes are sparse.
 * Can be considered as "sparse tensor of dense fiber".
 * The "fiber" here can be defined as a vector of elements that have indices
 * only different in the last mode.
 */
typedef struct {
    size_t        nmodes; /// # Modes, must >= 2
    size_t        *ndims; /// size of each mode, length nmodes
    size_t        mode;   /// the mode where data is stored in dense format
    size_t        nnz;    /// # non-zero fibers
    sptSizeVector *inds;  /// indices of each dense fiber, length [nmodes][nnz], the mode-th value is ignored
    size_t        stride; /// ndims[mode] rounded up to 8
    sptMatrix     values; /// dense fibers, size nnz*ndims[mode]
} sptSemiSparseTensor;


/**
 * An opaque data type to store a specific time point, using either CPU or GPU clock.
 */
typedef struct sptTagTimer *sptTimer;

typedef enum {
    SPTERR_NO_ERROR       = 0,
    SPTERR_UNKNOWN        = 1,
    SPTERR_SHAPE_MISMATCH = 2,
    SPTERR_VALUE_ERROR    = 3,
    SPTERR_ZERO_DIVISION  = 4,
    SPTERR_NO_MORE        = 99,
    SPTERR_OS_ERROR       = 0x10000,
    SPTERR_CUDA_ERROR     = 0x20000,
} SptError;

int sptGetLastError(const char **module, const char **file, unsigned *line, const char **reason);
void sptClearLastError(void);
void spt_Panic(const char *file, unsigned line, const char *expr);
/**
 * The assert function that always execute even when `NDEBUG` is set
 *
 * Quick & dirty error checking. Useful when writing small programs.
 */
#define sptAssert(expr) ((expr) ? (void) 0 : spt_Panic(__FILE__, __LINE__, #expr))

/* Helper function for pure C module */
int sptCudaSetDevice(int device);
int sptCudaGetLastError(void);

/* Timer functions, using either CPU or GPU timer */
int sptNewTimer(sptTimer *timer, int use_cuda);
int sptStartTimer(sptTimer timer);
int sptStopTimer(sptTimer timer);
double sptElapsedTime(const sptTimer timer);
double sptPrintElapsedTime(const sptTimer timer, const char *name);
int sptFreeTimer(sptTimer timer);


/* Dense array */
size_t sptMaxSizeArray(size_t const * const indices, size_t const size);
void spt_DumpArray(const size_t array[], size_t length, size_t start_index, FILE *fp);

/* Dense vector, aka variable length array */
int sptNewVector(sptVector *vec, size_t len, size_t cap);
int sptConstantVector(sptVector * const vec, sptScalar const val);
int sptCopyVector(sptVector *dest, const sptVector *src);
int sptAppendVector(sptVector *vec, sptScalar value);
int sptAppendVectorWithVector(sptVector *vec, const sptVector *append_vec);
int sptResizeVector(sptVector *vec, size_t size);
void sptFreeVector(sptVector *vec);
int sptDumpVector(sptVector *vec, FILE *fp);

/* Dense vector, with size_t type */
int sptNewSizeVector(sptSizeVector *vec, size_t len, size_t cap);
int sptConstantSizeVector(sptSizeVector * const vec, size_t const num);
int sptCopySizeVector(sptSizeVector *dest, const sptSizeVector *src);
int sptAppendSizeVector(sptSizeVector *vec, size_t value);
int sptAppendSizeVectorWithVector(sptSizeVector *vec, const sptSizeVector *append_vec);
int sptResizeSizeVector(sptSizeVector *vec, size_t value);
void sptFreeSizeVector(sptSizeVector *vec);
int sptDumpSizeVector(sptSizeVector *vec, FILE *fp);

/* Dense matrix */
int sptNewMatrix(sptMatrix *mtx, size_t nrows, size_t ncols);
int sptRandomizeMatrix(sptMatrix *mtx, size_t nrows, size_t ncols);
int sptIdentityMatrix(sptMatrix *mtx);
int sptConstantMatrix(sptMatrix * const mtx, sptScalar const val);
int sptCopyMatrix(sptMatrix *dest, const sptMatrix *src);
void sptFreeMatrix(sptMatrix *mtx);
int sptAppendMatrix(sptMatrix *mtx, const sptScalar values[]);
int sptResizeMatrix(sptMatrix *mtx, size_t newsize);
int sptSparseTensorToMatrix(sptMatrix *dest, const sptSparseTensor *src);
int sptDumpMatrix(sptMatrix *mtx, FILE *fp);
int sptMatrixDotMul(sptMatrix const * A, sptMatrix const * B, sptMatrix const * C);
int sptMatrixDotMulSeq(size_t const mode, size_t const nmodes, sptMatrix ** mats);
int sptOmpMatrixDotMulSeq(size_t const mode, size_t const nmodes, sptMatrix ** mats);
int sptCudaMatrixDotMulSeq(
    size_t const mode,
    size_t const nmodes,
    const size_t rank,
    const size_t stride,
    sptScalar ** dev_ata);
int sptMatrix2Norm(sptMatrix * const A, sptScalar * const lambda);
int sptOmpMatrix2Norm(sptMatrix * const A, sptScalar * const lambda);
int sptCudaMatrix2Norm(
    size_t const nrows,
    size_t const ncols,
    size_t const stride,
    sptScalar * const dev_vals,
    sptScalar * const dev_lambda);

/* Sparse matrix */
int sptNewSparseMatrix(sptSparseMatrix *mtx, size_t nrows, size_t ncols);
int sptCopySparseMatrix(sptSparseMatrix *dest, const sptSparseMatrix *src);
void sptFreeSparseMatrix(sptSparseMatrix *mtx);

/* Sparse tensor */
int sptNewSparseTensor(sptSparseTensor *tsr, size_t nmodes, const size_t ndims[]);
int sptCopySparseTensor(sptSparseTensor *dest, const sptSparseTensor *src);
void sptFreeSparseTensor(sptSparseTensor *tsr);
int sptLoadSparseTensor(sptSparseTensor *tsr, size_t start_index, FILE *fp);
int sptDumpSparseTensor(const sptSparseTensor *tsr, size_t start_index, FILE *fp);
void sptSparseTensorSortIndex(sptSparseTensor *tsr, int force);
void sptSparseTensorSortIndexAtMode(sptSparseTensor *tsr, size_t mode, int force);

/**
 * epsilon is a small positive value, every -epsilon < x < x would be considered as zero
 */
int sptSemiSparseTensorToSparseTensor(sptSparseTensor *dest, const sptSemiSparseTensor *src, sptScalar epsilon);

int sptNewSemiSparseTensor(sptSemiSparseTensor *tsr, size_t nmodes, size_t mode, const size_t ndims[]);
int sptCopySemiSparseTensor(sptSemiSparseTensor *dest, const sptSemiSparseTensor *src);
void sptFreeSemiSparseTensor(sptSemiSparseTensor *tsr);
int sptSparseTensorToSemiSparseTensor(sptSemiSparseTensor *dest, const sptSparseTensor *src, size_t mode);
int sptSemiSparseTensorSortIndex(sptSemiSparseTensor *tsr);
/**
 * Set indices of a semi-sparse according to a reference sparse
 * Call sptSparseTensorSortIndexAtMode on ref first
 */
int sptSemiSparseTensorSetIndices(sptSemiSparseTensor *dest, sptSizeVector *fiberidx, sptSparseTensor *ref);


/* Sparse tensor unary operations */
int sptSparseTensorMulScalar(sptSparseTensor *X, sptScalar a);
int sptSparseTensorDivScalar(sptSparseTensor *X, sptScalar a);
/* Sparse tensor binary operations */
int sptSparseTensorAdd(sptSparseTensor *Z, const sptSparseTensor *X, const sptSparseTensor *Y);
int sptSparseTensorSub(sptSparseTensor *Z, const sptSparseTensor *X, const sptSparseTensor *Y);
int sptSparseTensorDotMul(sptSparseTensor *Z, const sptSparseTensor *X, const sptSparseTensor *Y);
int sptSparseTensorDotMulEq(sptSparseTensor *Z, const sptSparseTensor *X, const sptSparseTensor *Y);
int sptOmpSparseTensorDotMulEq(sptSparseTensor *Z, const sptSparseTensor *X, const sptSparseTensor *Y);
int sptCudaSparseTensorDotMulEq(sptSparseTensor *Z, const sptSparseTensor *X, const sptSparseTensor *Y);
int sptSparseTensorDotDiv(sptSparseTensor *Z, const sptSparseTensor *X, const sptSparseTensor *Y);

int sptSparseTensorMulMatrix(sptSemiSparseTensor *Y, sptSparseTensor *X, const sptMatrix *U, size_t mode);
int sptOmpSparseTensorMulMatrix(sptSemiSparseTensor *Y, sptSparseTensor *X, const sptMatrix *U, size_t mode);
int sptCudaSparseTensorMulMatrix(sptSemiSparseTensor *Y, sptSparseTensor *X, const sptMatrix *U, size_t mode);
int sptCudaSparseTensorMulMatrixOneKernel(sptSemiSparseTensor *Y, sptSparseTensor *X, const sptMatrix *U, size_t mode, size_t const impl_num, size_t const smen_size);

/**
 * Kronecker product
 */
int sptSparseTensorKroneckerMul(sptSparseTensor *Y, const sptSparseTensor *A, const sptSparseTensor *B);

/**
 * Khatri-Rao product
 */
int sptSparseTensorKhatriRaoMul(sptSparseTensor *Y, const sptSparseTensor *A, const sptSparseTensor *B);

/**
 * Matricized tensor times Khatri-Rao product.
 */
int sptMTTKRP(sptSparseTensor const * const X,
    sptMatrix * mats[],     // mats[nmodes] as temporary space.
    size_t const mats_order[],    // Correspond to the mode order of X.
    size_t const mode,
    sptVector * scratch);
int sptOmpMTTKRP(sptSparseTensor const * const X,
    sptMatrix * mats[],     // mats[nmodes] as temporary space.
    size_t const mats_order[],    // Correspond to the mode order of X.
    size_t const mode,
    sptVector * scratch);
int sptCudaMTTKRP(sptSparseTensor const * const X,
    sptMatrix ** const mats,    // mats[nmodes] as temporary space.
    sptSizeVector const * const mats_order, // Correspond to the mode order of X.
    size_t const mode);
int sptCudaMTTKRPOneKernel(
    sptSparseTensor const * const X,
    sptMatrix ** const mats,     // mats[nmodes] as temporary space.
    size_t * const mats_order,    // Correspond to the mode order of X.
    size_t const mode,
    size_t const impl_num);

#ifdef __cplusplus
}
#endif

#endif
