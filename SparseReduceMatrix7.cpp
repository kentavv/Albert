/******************************************************************/
/***  FILE :          SparseReduceMatrix.c                      ***/
/***  PROGRAMMER:     David Lee                                 ***/
/***  DATE WRITTEN:   April-August 1992.                        ***/
/***  PUBLIC ROUTINES:                                          ***/
/***                  SparseReduceMatrix()                      ***/
/***                  Get_Matrix_Element()                      ***/
/***                  Insert_Element()                          ***/
/***                  Delete_Element()                          ***/
/***                  Change_Element()                          ***/
/***                  Locate_Node()                             ***/
/***  PRIVATE ROUTINES:                                         ***/
/***                  SparseMultRow()                           ***/
/***                  SparseAddRow()                            ***/
/***                  SparseKnockOut()                          ***/
/***                  SparseInterchange()                       ***/
/***                  Insert_Node()                             ***/
/***                  Delete_Node()                             ***/
/***                  Change_Node()                             ***/
/***                  Row_Empty()                               ***/
/***  MODULE DESCRIPTION:                                       ***/
/***                   This module  reduces the sparse matrix   ***/
/***                   in row canonical form. This code is      ***/
/***                   similar to the code in ReduceMatrix.c    ***/
/******************************************************************/

#include <list>
#include <vector>
#include <algorithm>
#include <numeric>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <omp.h>

#include "SparseReduceMatrix7.h"
#include "Build_defs.h"
#include "Scalar_arithmetic.h"
#include "profile.h"
#include "memory_usage.h"

extern bool __record;
extern int __deg;
extern int __nn1;
extern int __nn2;

namespace SparseReduceMatrix7_ns {
    using std::max;
    using std::min;
    using std::list;
    using std::vector;
    using std::lower_bound;
//using std::random_shuffle;

    static bool SM_sort(const SparseRow &r1, const SparseRow &r2);

    class SparseMatrixCache {
    public:
        SparseMatrixCache() = delete;

        SparseMatrixCache(SparseMatrix &SM) : SM_(SM) {
            idx_.resize(SM_.size());
            std::iota(idx_.begin(), idx_.end(), 0);
        }

        SparseMatrixCache(const SparseMatrixCache &) = delete;

        SparseMatrixCache &operator=(const SparseMatrixCache &) = delete;

        SparseMatrix &SM_;
        vector<int> idx_;

        void sort() {
            // stable_sort(idx_.begin(), idx_.end(), [this](size_t i1, size_t i2) { return SM_sort(SM_[i1], SM_[i2]); });
            std::sort(idx_.begin(), idx_.end(), [this](size_t i1, size_t i2) { return SM_sort(SM_[i1], SM_[i2]); });
        }

        SparseRow &operator[](int i) {
            return SM_[idx_[i]];
        }

        const SparseRow &operator[](int i) const {
            //printf("%d %d\n", i, idx_.size());
            //printf("%d %d %d\n", i, idx_.size(), idx_[i]);
            return SM_[idx_[i]];
        }

        size_t size() const {
            return SM_.size();
        }
    };

    static void SparseMultRow(SparseMatrixCache &SM, int Row, Scalar Factor);

    static void SparseAddRow(SparseMatrixCache &SMC, Scalar Factor, int Row1, int Row2);

    static void SparseKnockOut(SparseMatrixCache &SMC, int row, int col, int last_row);

    Scalar Get_Matrix_Element(const SparseMatrixCache &SMC, int i, int j);

#if 0
    static void Print_Matrix(MAT_PTR Sparse_Matrix, int r, int c);
    static void Print_Rows(int Row1, int Row2, int nCols);
    static void Print_SLList(Node *SLHead_Ptr);
    static void Print_Node(NODE_PTR Prt_Node);
#endif

    static bool do_sort = true;
    static int sort_freq = 10;

//class RowStats {
//public:
//    int nz;
//    int fc;
//    int fe;
//};

    struct stats {
        //size_t n_zero_elements;
        size_t n_elements;
        size_t capacity;
        size_t n_zero_rows;
        size_t n_rows;
        size_t n_cols;
        int last_nextstairrow;
        int prev_col;
        int cur_col;
        time_t first_update;
        time_t prev_update;
        time_t cur_update;

        stats() : // n_zero_elements(0),
                n_elements(0),
                capacity(0),
                n_zero_rows(0),
                n_rows(0),
                n_cols(0),
                last_nextstairrow(0),
                prev_col(0),
                cur_col(0),
                first_update(0),
                prev_update(0),
                cur_update(0) {}

        void clear() {
            //n_zero_elements = 0;
            n_elements = 0;
            capacity = 0;
            n_zero_rows = 0;
            n_rows = 0;
            n_cols = 0;
            last_nextstairrow = 0;
            // These are not reset between updates because they are used
            // to calculate rates.
            //prev_col = 0;
            //cur_col = 0;
            //first_update = 0;
            //prev_update = 0;
            //cur_update = 0;
        }

        static void tp(float t) {
            if (t > 3600) {
                printf("%.02fh", t / 3600.);
            } else if (t > 60) {
                printf("%.02fm", t / 60.);
            } else {
                printf("%.02fs", t);
            }
        }

        void print() const {
            printf("\r\t\tne:%lu (%.1fMB)", n_elements, n_elements * sizeof(Node) / 1024. / 1024.);
#if 0
            if(n_zero_elements > 0) {
              printf(" ze:%lu", n_zero_elements);
            }
#endif
            if (n_elements != capacity) {
                printf(" ce:%lu", capacity);
            }
            printf("  zr:%lu  lr:%d/%lu  lc:%d/%lu",
                   n_zero_rows,
                   last_nextstairrow, n_rows,
                   cur_col, n_cols);
            {
                time_t dt = cur_update - first_update;
                if (dt > 0) {
                    printf("  tt:");
                    tp(dt);
                }
            }
            if (cur_col > 100) {
                int dt = cur_update - prev_update;
                if (dt != 0) {
                    float cps = (cur_col - prev_col + 1) / float(dt);
                    printf("  cps:%.02f", cps);

                    float eta = (n_cols - cur_col) / cps;
                    if (eta > 1) {
                        printf("  etr:");
                        tp(eta);
                    }
                }
            }
            printf("                    ");
            fflush(nullptr);
        }

        void update(const SparseMatrixCache &SM, int nextstairrow_, int last_col_, int nCols_, int timeout = -1,
                    bool do_print = false) {
            time_t t = time(nullptr);
            if (timeout != -1 && cur_update != 0 && t - cur_update < timeout) {
                return;
            }

            clear();
            if (first_update == 0) {
                first_update = t;
            }
            prev_update = cur_update;
            cur_update = t;

            n_rows = SM.size();
            n_cols = nCols_;
            last_nextstairrow = nextstairrow_;
            prev_col = cur_col;
            cur_col = last_col_;

            for (int ii = 0; ii < (int) SM.size(); ii++) {
                capacity += SM[ii].capacity();
                n_elements += SM[ii].size();

                if (SM[ii].empty()) {
                    n_zero_rows++;
                }

#if 0
                // There should be no zero elements
                for(int jj=0; jj<(int)SM[ii].size(); jj++) {
                  if(SM[ii][jj].getElement() == S_zero()) {
                    n_zero_elements++;
                  }
                }
#endif
            }

            if (do_print) {
                print();
            }
        }
    };

    static void save_mat_image(int a, int b, int c, const SparseMatrix &SM, int nCols) {
        int mh = SM.size();
        int mw = nCols;

        const int iih = 2160;
        const int iiw = 3840;

        int ih = min(mh, iih);
        int iw = min(mw, iiw);

//  if(mh < ih || mw < iw) return;

        auto img = new unsigned char[ih * iw]();

        for (int r = 0; r < (int) SM.size(); r++) {
            for (int j = 0; j < (int) SM[r].size(); j++) {
                if (SM[r][j].getElement() != S_zero()) {
                    int col = SM[r][j].getColumn();

                    int y = r;
                    int x = col;
                    if (ih < mh) y = int(y / float(mh) * ih);
                    if (iw < mw) x = int(x / float(mw) * iw);
//if(x >= iw) abort();
//if(y >= ih) abort();
                    if (img[y * iw + x] < 255) {
                        img[y * iw + x]++;
                    }
                }
            }
        }

#if 0
        int mv = 0;
        for (int i = 0; i < ih * iw; i++) {
            mv = max(int(img[i]), mv);
        }

        for (int i = 0; i < ih * iw; i++) {
            if (img[i] != 0) {
                img[i] = int(float(img[i]) / float(mv) * (255 - 63) + 63 + .5);
            }
        }
#else
        for (int i = 0; i < ih * iw; i++) {
            if (img[i]) {
                img[i] = 255;
            }
        }
#endif

        {
            int s = 1;
            if (ih >= iw) {
                s = iih / ih;
            } else {
                s = iih / iw;
            }
            int hoff = (iih - s * ih) / 2;
            int woff = (iiw - s * iw) / 2;

            auto s_img = new unsigned char[iih * iiw]();

            for (int i = 0; i < ih; i++) {
                for (int j = 0; j < iw; j++) {
                    for (int ii = 0; ii < s; ii++) {
                        for (int jj = 0; jj < s; jj++) {
                            s_img[((i * s + hoff + ii) * iiw) + woff + j * s + jj] = img[i * iw + j];
                        }
                    }
                }
            }

            delete[] img;
            img = s_img;
            ih = iih;
            iw = iiw;
        }

        char fn[128];
        static int ind = 0;
        //sprintf(fn, "%d_%d_%d__%d_%d_%d__%d.pgm", __deg, __nn1, __nn2, a, b, c, ind);
        sprintf(fn, "%08d_%03d_%03d_%03d_%06d_%06d.pgm", ind, __deg, __nn1, __nn2, c, nCols);
        ind++;
        FILE *f = fopen(fn, "wb");
        fprintf(f, "P5\n%d %d 255\n", iw, ih);
        fwrite(img, 1, ih * iw, f);
        fclose(f);

        delete[] img;
    }

    static bool SM_sort(const SparseRow &r1, const SparseRow &r2) {
        {
            if (r1.empty()) return false;
            if (r2.empty()) return true;
            //auto a = r1.empty();
            //auto b = r2.empty();
            //if (!a && b) return true;
            //if (a && b) return false;
            //if (a && !b) return false;
        }

        auto r1i = r1.front();
        auto r2i = r2.front();
        {
            auto a = r1i.getColumn();
            auto b = r2i.getColumn();
            if (a < b) return true;
            if (a > b) return false;
        }
        {
            auto a = r1.size();
            auto b = r2.size();
#if 1
            // Generally results in greater sparsity
            if (a < b) return true;
            if (a > b) return false;
#else
            // Generally results in greater density, i.e. more non-zero intermediate entries
            if (a > b) return true;
            if (a < b) return false;
#endif
        }
        {
            auto a = r1i.getElement();
            auto b = r2i.getElement();
            if (a < b) return true;
            //if (a > b) return false;
        }

        return false;
    }


    int SparseReduceMatrix(SparseMatrix &SM, int nCols, int *Rank) {
        memory_usage_init(nCols);

        if (SM.empty() || nCols == 0) {
            return OK;
        }

        SparseMatrixCache SMC(SM);

        if (do_sort) SMC.sort();

        //printf("s:%d c:%d ", (int)SM.size(), (int)SM.capacity());
        /* Search for the rightmost nonzero element */
        /* Dependent on the current stairrow */

        stats s1;
        SMC.sort();
        s1.update(SMC, 0, 0, nCols, -1, true);

        float nper = .1;
        if (__record) {
            save_mat_image(0, 0, 0, SM, nCols);
        }

        int nextstairrow = 0;
        int last_row = SMC.size();
        for (int i = 0; i < nCols; i++) {
            memory_usage_update(i);

            putchar('\n');
            Profile p("Iteration");
            int j;
            {
                Profile p("find next");
                for (j = nextstairrow; j < last_row; j++) {
                    if (Get_Matrix_Element(SMC, j, i) != S_zero()) {
                        break;
                    }
                }
            }
            /* When found interchange and then try to knockout any nonzero
               elements in the same column */

#define DEBUG_MATRIX 0

#if DEBUG_MATRIX
            printf("\nCol:%d/%d j:%d nextstairrow:%d nRows:%d reducing?:%d\n", i, nCols, j, nextstairrow, SM.size(), j < (int) SM.size());
            {
                printf("Start\n");
                for (int i = 0; i < (int) SM.size(); i++) {
                    for (int j = 0; j<(int)nCols; j++) {
                        Scalar s = Get_Matrix_Element(SM, i, j);
                        printf(" %3d", s);
                    }
                    putchar('\n');
                }
            }
#endif
            if (j < last_row) {
                SMC[nextstairrow].swap(SMC[j]);
#if DEBUG_MATRIX
                {
                    printf("After swap\n");
                    for (int i = 0; i < (int) SMC.size(); i++) {
                        for (int j = 0; j<(int)nCols; j++) {
                            Scalar s = Get_Matrix_Element(SM, i, j);
                            printf(" %3d", s);
                        }
                        putchar('\n');
                    }
                }
#endif
                {
                    char s[128];
                    sprintf(s, "Knockout %d/%d %d/%d/%d", i, nCols, nextstairrow, last_row, SMC.size());
                    //Profile p0("c");
                    Profile p(s);

                    SparseKnockOut(SMC, nextstairrow, i, last_row);
                }
                for (int iii = last_row; iii > 0; iii--) {
                    if (!SMC[iii - 1].empty()) {
                        last_row = iii;
                        break;
                    }
                }
//            printf("%d %d\n", SM.size(), last_row);
//            if (do_sort) sort(SM.begin() + nextstairrow + 1, SM.end(), SM_sort);
                if (do_sort) SMC.sort();

                if (__record) {
                    printf(">> %d", nextstairrow);
                    long aa = 0;
                    long bb = 0;
                    for (int ii = nextstairrow; ii < SMC.size(); ii++) {
//                    printf("(%.2f %.2f) ", SMC[i].size() / float(nCols - nextstairrow) * 100, SM[ii].size() * 4 / float(nCols - nextstairrow + 4));
                        if (!SM[ii].empty()) {
                            int a = SM[ii].size() * 4;
                            int b = (nCols - SMC[ii].front().getColumn() + 1) * 1 + 4;
                            printf(" %.2f", a / float(b));
                            aa += a;
                            bb += b;
                        }
                    }
                    printf("\n  %.2f\n", aa / float(bb));
                }

                {
                    //Profile p1("3");
//            printf("%d %d\n", SMC.size(), last_row);
                    if (i % sort_freq == 0) {
                        Profile p1("sort");
                        if (do_sort) {
                            //       Profile("sort");
                            SMC.sort();
//                        sort(SM.begin() + nextstairrow + 1, SM.begin() + last_row, SM_sort);
                        }
                    }
                }

#if DEBUG_MATRIX
                {
                    printf("After reduce\n");
                    for (int i = 0; i < (int) SM.size(); i++) {
                        for (int j = 0; j<(int)nCols; j++) {
                            Scalar s = Get_Matrix_Element(SM, i, j);
                            printf(" %3d", s);
                        }
                        putchar('\n');
                    }
                }
#endif
                nextstairrow++;
            }

            if (__record && (1 || i / float(nCols) > nper)) {
                nper += .1;
                save_mat_image(0, 1, i, SM, nCols);
            }

            {
//            Profile p("update");
                s1.update(SMC, nextstairrow, i, nCols, 60, true);
            }
        }
        *Rank = nextstairrow;
        SMC.sort();
        s1.update(SMC, nextstairrow, nCols, nCols, -1, true);

        sort(SM.begin(), SM.end(), SM_sort);

        putchar('\n');

        if (__record) {
            save_mat_image(0, 2, nCols, SM, nCols);
        }

#if DEBUG_MATRIX
        {
            printf("Final\n");
            for (int i = 0; i < (int) SM.size(); i++) {
                for (int j = 0; j<(int)nCols; j++) {
                    Scalar s = Get_Matrix_Element(SM, i, j);
                    printf(" %3d", s);
                }
                putchar('\n');
            }
        }
#endif

        return OK;
    }


    void SparseMultRow(SparseMatrixCache &SMC, int Row, Scalar Factor) {
        /* Step thru row ... multiplying each element by the factor */
        for (auto ii = SMC[Row].begin(); ii != SMC[Row].end(); ii++) {
            ii->setElement(S_mul(ii->getElement(), Factor));
        }
    }

    template<typename T, class Allocator>
    void shrink_capacity(std::vector<T, Allocator> &v) {
        std::vector<T, Allocator>(v.begin(), v.end()).swap(v);
    }

/*********************************************************************/
/* Three things can happen with the SparseAddRow routine ....
   First there is a target row and a row which is multiplied by a factor
   and added to the target row. 
   1. The result is nonzero and there is no column in the target row so add 
      a new node.
   2. The result is nonzero and there is a column in the target row so change
      the value in the node.
   3. The result is zero and there is a column in the target row so delete
      the node.
*/
/*********************************************************************/
    void SparseAddRow(SparseMatrixCache &SMC, Scalar Factor, int Row1, int Row2) {
        /* check for zero factor */

        if (Factor == S_zero()) {
            return;
        }

        /* get the beginning of the two rows to work with */

        const SparseRow &r1 = SMC[Row1];
        SparseRow &r2 = SMC[Row2];

        SparseRow tmp;
        tmp.reserve(r1.size() + r2.size());

        auto r1i = r1.cbegin();
        auto r2i = r2.cbegin();

        for (; r1i != r1.cend() && r2i != r2.cend();) {
            if (r1i->getColumn() == r2i->getColumn()) {
                Scalar x = S_add(r2i->getElement(), S_mul(Factor, r1i->getElement()));
                if (x != S_zero()) {
                    Node n = *r1i;
                    n.setElement(x);
                    tmp.push_back(n);
                }
                r1i++;
                r2i++;
            } else if (r1i->getColumn() < r2i->getColumn()) {
                Scalar x = S_mul(Factor, r1i->getElement());
                //if(x != S_zero()) {
                Node n = *r1i;
                n.setElement(x);
                tmp.push_back(n);
                //}
                r1i++;
            } else { //if(r1i->column > r2i->column) {
                tmp.push_back(*r2i);
                r2i++;
            }
        }

        // append r2 with remaining r1 nodes
        for (; r1i != r1.cend(); r1i++) {
            Scalar x = S_mul(Factor, r1i->getElement());
            //if(x != S_zero()) {
            Node n = *r1i;
            n.setElement(x);
            tmp.push_back(n);
            //}
        }

        // append r2 with remaining r1 nodes
        for (; r2i != r2.cend(); r2i++) {
            tmp.push_back(*r2i);
        }
        SparseRow(tmp.begin(), tmp.end()).swap(r2); // shrink capacity while assigning
        //r2 = SparseRow(tmp.begin(), tmp.end());
        //shrink_capacity(tmp);
        //printf("<%d %d %d %d>", (int)r2.size(), (int)r2.capacity(),  (int)tmp.size(), (int)tmp.capacity());
        //r2 = tmp;
        //r2.swap(tmp);
    }

    void SparseKnockOut(SparseMatrixCache &SMC, int row, int col, int last_row) {
        Scalar x = Get_Matrix_Element(SMC, row, col);
        if (x != S_one()) {
            /* if the rightmost element in the current row is not one then multiply*/
            SparseMultRow(SMC, row, S_inv(x));
        }

        /* try to knockout elements in column in the rows above */

#pragma omp parallel for shared(SMC, row, col, last_row) schedule(static, 50) default(none)
//    for (int j = 0; j < (int) SM.size(); j++) {
        for (int j = 0; j < last_row; j++) {
            if (j != row) {
                SparseAddRow(SMC, S_minus(Get_Matrix_Element(SMC, j, col)), row, j);
            }
        }
    }

#if 0

    void Print_Matrix(MAT_PTR Sparse_Matrix, int r, int c) {
        int i, row, col;
        Node *Row_Head_Ptr, *Row_Element_Ptr;

        if (Sparse_Matrix == NULL) {
            return;
        }
        for (row = 0; row < r; row++) {
            Row_Head_Ptr = Sparse_Matrix[row];
            Row_Element_Ptr = Row_Head_Ptr;
            if (Row_Element_Ptr == NULL) {
                printf("EMPTY ROW %d\n", row);
            } else {
                while (Row_Element_Ptr != NULL) {
                    printf("%4d", Row_Element_Ptr->element);
                    Row_Element_Ptr = Row_Element_Ptr->Next_Node;
                }
                printf("\n");
            }
        }
        printf("\n");
        for (row = 0; row < r; row++) {
            Row_Head_Ptr = Sparse_Matrix[row];
            Row_Element_Ptr = Row_Head_Ptr;

            for (col = 0; (col < c); col++) {
                if (Row_Element_Ptr != NULL) {
                    if (Row_Element_Ptr->column != col) {
                        printf("   0");
                    } else {
                        printf("%4d", Row_Element_Ptr->element);
                        if (Row_Element_Ptr->Next_Node != NULL)
                            Row_Element_Ptr = Row_Element_Ptr->Next_Node;
                    }
                } else {
                    printf("   0");
                }
            }
            printf("\n");
        }
        printf("\n");
    }


    void Print_Rows(int Row1, int Row2, int nCols) {
        int i, row, col;
        NODE_PTR Row1_Ptr;
        NODE_PTR Row2_Ptr;

        Row1_Ptr = Matrix_Base_Ptr[Row1];
        Row2_Ptr = Matrix_Base_Ptr[Row2];
        for (col = 0; (col < nCols); col++) {
            if (Row1_Ptr != NULL) {
                if (Row1_Ptr->column != col) {
                    printf("   0");
                } else {
                    printf("%4d", Row1_Ptr->element);
                    if (Row1_Ptr->Next_Node != NULL)
                        Row1_Ptr = Row1_Ptr->Next_Node;
                }
            } else {
                printf("   0");
            }
        }
        printf("\n");
        for (col = 0; (col < nCols); col++) {
            if (Row2_Ptr != NULL) {
                if (Row2_Ptr->column != col) {
                    printf("   0");
                } else {
                    printf("%4d", Row2_Ptr->element);
                    if (Row2_Ptr->Next_Node != NULL)
                        Row2_Ptr = Row2_Ptr->Next_Node;
                }
            } else {
                printf("   0");
            }
        }
        printf("\n");
    }

#endif

#if 0

    static bool cmp_column(const Node &n1, const Node &n2) { return n1.column < n2.column; }

#if 0

    struct A {
        bool operator()(const Node &n1, const Node &n2) const { return n1.column < n2.column; }
    };

#endif

    Scalar Get_Matrix_Element(const SparseMatrix &SM, int i, int j) {
        Node n;
        n.column = j;

        SparseRow::const_iterator ii = lower_bound(SM[i].begin(), SM[i].end(), n, cmp_column);
        //SparseRow::const_iterator ii = lower_bound(SM[i].begin(), SM[i].end(), n, A());
        //SparseRow::const_iterator ii = lower_bound(SM[i].begin(), SM[i].end(), n);
        if (ii != SM[i].end() && ii->column == j) {
            return ii->element;
        }

        return S_zero();
    }

#else

//Scalar Get_Matrix_Element(const SparseMatrix &SM, int i, int j) {
//    /* either return the element at location i,j or return a zero */
//    for (auto ii = SM[i].cbegin(); ii != SM[i].cend() && ii->getColumn() <= j; ii++) {
//        if (ii->getColumn() == j) return ii->getElement();
//    }
//    return S_zero();
//}

    Scalar Get_Matrix_Element(const SparseMatrixCache &SMC, int i, int j) {
        /* either return the element at location i,j or return a zero */
        for (auto ii = SMC[i].cbegin(); ii != SMC[i].cend() && ii->getColumn() <= j; ii++) {
            if (ii->getColumn() == j) return ii->getElement();
        }
        return S_zero();
    }

#endif

#if 0

    void Print_SLList(Node *SLHead_Ptr) {
        Node *Prt_Ptr;

        Prt_Ptr = SLHead_Ptr;

        printf("\nColumn :");
        while (Prt_Ptr != nullptr) {
            printf(" %3d", Prt_Ptr->column);
            Prt_Ptr = Prt_Ptr->Next_Node;
        }

        Prt_Ptr = SLHead_Ptr;

        printf("\n");
        printf("Element:");
        while (Prt_Ptr != nullptr) {
            printf(" %3d", Prt_Ptr->element);
            Prt_Ptr = Prt_Ptr->Next_Node;
        }
        printf("\n");
        printf("\n");
    }

    void Print_Node(NODE_PTR Prt_Node) {
        if (Prt_Node == nullptr) {
            printf("NULL\n");
            return;
        }
        printf("Node element:%d\tcolumn:%d\n", Prt_Node->element,
               Prt_Node->column);
    }

#endif

}

int SparseReduceMatrix7(SparseMatrix &SM, int nCols, int *Rank) {
    return SparseReduceMatrix7_ns::SparseReduceMatrix(SM, nCols, Rank);
}
