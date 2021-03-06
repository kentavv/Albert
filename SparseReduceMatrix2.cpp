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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <omp.h>

#include "SparseReduceMatrix.h"
#include "Build_defs.h"
#include "Scalar_arithmetic.h"
#include "profile.h"
#include "memory_usage.h"

namespace SparseReduceMatrix2_ns {
    using std::list;
    using std::vector;
    using std::lower_bound;
    using std::sort;
    using std::max;
//using std::random_shuffle;

    struct Node2 {
        Node2(int e_ = 0, int c_ = 0) : e(e_), c(c_) {};

        uint32_t e: 8;
        uint32_t c: 24;

        Scalar getElement() const {
            return e;
        }

        void setElement(Scalar v) {
            e = v;
        }

        int getRow() const {
            return c;
        }

        void setRow(int v) {
            c = v;
        }
    };

    static bool cmp_row(const Node2 &n1, const Node2 &n2) { return n1.c < n2.c; }

    inline bool operator==(const Node2 &n1, const Node2 &n2) {
        return n1.e == n2.e && n1.c == n2.c;
    }

//inline bool operator<(const Node2 &n1, const Node2 &n2) {
//    return n1.c < n2.c || (n1.c == n2.c && n1.e < n2.e);
//}
    inline bool operator<(const Node2 &n1, const Node2 &n2) {
        return n1.c < n2.c;
    }

    typedef std::vector<Node2> SparseCol;
    typedef std::vector<SparseCol> SparseMatrix2;

    static void SparseMultRow(SparseMatrix &SM, int Row, Scalar Factor);

    static void SparseAddRow(SparseMatrix &SM, Scalar Factor, int Row1, int Row2);

    static void SparseKnockOut(SparseMatrix2 &SM, int row, int col, int nRows);

    static Scalar Get_Matrix_Element2(const SparseMatrix2 &SM, int i, int j);

#if 0
    static void Print_Matrix(MAT_PTR Sparse_Matrix, int r, int c);
    static void Print_Rows(int Row1, int Row2, int nCols);
    static void Print_SLList(Node *SLHead_Ptr);
    static void Print_Node(NODE_PTR Prt_Node);
#endif

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

        void update(const SparseMatrix2 &SM, int nextstairrow_, int last_col_, int nCols_, int timeout = -1,
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

    static int SparseReduceMatrix2_(SparseMatrix2 &SM, int nRows, int *Rank);

    int SparseReduceMatrix(SparseMatrix &SM, int nCols, int *Rank) {
        memory_usage_init(nCols);

        if (SM.empty() || nCols == 0 || (SM.size() == 1 && SM[0].empty())) {
            return OK;
        }

        SparseMatrix SMa = SM;

        SparseMatrix2 SM2;
        SM2.resize(nCols);
        for (int i = 0; i < (int) SM.size(); i++) {
            SparseCol tmp;
            for (auto j = SM[i].cbegin(); j != SM[i].cend(); j++) {
                SM2[j->getColumn()].push_back(Node2(j->getElement(), i));
            }
        }
        for (auto i = SM2.begin(); i != SM2.end(); i++) {
            sort(i->begin(), i->end());
        }

        int nRows = SM.size();

        {
            SM.clear();
            int nrows = 0;
            for (auto i = SM2.cbegin(); i != SM2.cend(); i++) {
                if (!i->empty()) {
                    nrows = max(nrows, i->back().getRow() + 1);
                }
            }
            SM.resize(nrows);
            for (int i = 0; i < (int) SM2.size(); i++) {
                for (auto j = SM2[i].cbegin(); j != SM2[i].cend(); j++) {
                    SM[j->getRow()].push_back(Node(j->getElement(), i));
                }
            }
            if (SM != SMa) {
                abort();
            }
        }

        int rv = SparseReduceMatrix2_(SM2, nRows, Rank);

        SM.clear();
//    int nrows = 0;
//    for (auto i = SM2.cbegin(); i != SM2.cend(); i++) {
//        if (!i->empty()) {
//            nrows = max(nrows, i->back().getRow() + 1);
//        }
//    }
        SM.resize(nRows);
        for (int i = 0; i < (int) SM2.size(); i++) {
            for (auto j = SM2[i].cbegin(); j != SM2[i].cend(); j++) {
                SM[j->getRow()].push_back(Node(j->getElement(), i));
            }
        }

#define DEBUG_MATRIX 0

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

        return rv;
    }

    int SparseReduceMatrix2_(SparseMatrix2 &SM, int nRows, int *Rank) {
        if (SM.empty()) {
            return OK;
        }

//random_shuffle(SM.begin(), SM.end());

        //printf("s:%d c:%d ", (int)SM.size(), (int)SM.capacity());
        /* Search for the rightmost nonzero element */
        /* Dependent on the current stairrow */

        int nCols = SM.size();

        stats s1;
        s1.update(SM, 0, 0, nCols, -1, true);

        int nextstairrow = 0;
        for (int i = 0; i < nCols; i++) {
            memory_usage_update(i);

            int j;
            for (j = nextstairrow; j < nRows; j++) {
                if (Get_Matrix_Element2(SM, j, i) != S_zero()) {
                    break;
                }
            }
            /* When found interchange and then try to knockout any nonzero
               elements in the same column */

#if DEBUG_MATRIX
            printf("\nCol:%d/%d j:%d nextstairrow:%d nRows:%d reducing?:%d\n", i, nCols, j, nextstairrow, nRows,
                   j < (int) nRows);
            {
                printf("Start\n");
                for (int i = 0; i < (int) nRows; i++) {
                    for (int j = 0; j < (int) SM.size(); j++) {
                        Scalar s = Get_Matrix_Element2(SM, i, j);
                        printf(" %3d", s);
                    }
                    putchar('\n');
                }
            }
#endif

            if (j < nRows) {
//            SM[nextstairrow].swap(SM[j]);
                if (nextstairrow != j) {
#pragma omp parallel for shared(SM, i, j, nextstairrow) schedule(dynamic, 10) default(none)
                    for (int iii = i; iii < SM.size(); iii++) {
                        auto ii = SM.begin() + iii;

#if 0
                        auto jj1 = ii->end();
                        auto jj2 = ii->end();
                        for (auto jj = ii->begin(); jj != ii->end(); jj++) {
                            if (jj->getRow() == j) jj1 = jj;
                            if (jj->getRow() == nextstairrow) jj2 = jj;
                            if (jj1 != ii->end() && jj2 != ii->end()) break;
                            if (jj->getRow() > j && jj->getRow() > nextstairrow) break;
                        }
#else
                        auto jj1 = std::lower_bound(ii->begin(), ii->end(), Node2(0, j));
                        auto jj2 = std::lower_bound(ii->begin(), ii->end(), Node2(0, nextstairrow));
                        if (jj1 != ii->end() && jj1->getRow() != j) jj1 = ii->end();
                        if (jj2 != ii->end() && jj2->getRow() != nextstairrow) jj2 = ii->end();
#endif
                        if (jj1 != ii->end() && jj2 != ii->end()) {
                            auto t = jj1->getElement();
                            jj1->setElement(jj2->getElement());
                            jj2->setElement(t);
                        } else if (jj1 != ii->end() && jj2 == ii->end()) {
#if 0
                            jj1->setRow(nextstairrow);
                            sort(ii->begin(), ii->end());
#else
                            Node2 n(jj1->getElement(), nextstairrow);
                            ii->erase(jj1);
                            ii->insert(std::upper_bound(ii->begin(), ii->end(), n), n);
#endif
                        } else if (jj1 == ii->end() && jj2 != ii->end()) {
#if 0
                            jj2->setRow(j);
                            sort(ii->begin(), ii->end());
#else
                            Node2 n(jj2->getElement(), j);
                            ii->erase(jj2);
                            ii->insert(std::upper_bound(ii->begin(), ii->end(), n), n);
#endif
                        }
                    }
                }

#if DEBUG_MATRIX
                {
                    printf("After swap\n");
                    for (int i = 0; i < (int) nRows; i++) {
                        for (int j = 0; j < (int) SM.size(); j++) {
                            Scalar s = Get_Matrix_Element2(SM, i, j);
                            printf(" %3d", s);
                        }
                        putchar('\n');
                    }
                }
#endif

                SparseKnockOut(SM, nextstairrow, i, nRows);

#if DEBUG_MATRIX
                {
                    printf("After reduce\n");
                    for (int i = 0; i < (int) nRows; i++) {
                        for (int j = 0; j < (int) SM.size(); j++) {
                            Scalar s = Get_Matrix_Element2(SM, i, j);
                            printf(" %3d", s);
                        }
                        putchar('\n');
                    }
                }
#endif
                nextstairrow++;
            }

            s1.update(SM, nextstairrow, i, nCols, 60, true);
        }
        *Rank = nextstairrow;
        s1.update(SM, nextstairrow, nCols, nCols, -1, true);

        putchar('\n');

        return OK;
    }


    void SparseMultRow(SparseMatrix &SM, int Row, Scalar Factor) {
        /* Step thru row ... multiplying each element by the factor */
        for (auto ii = SM[Row].begin(); ii != SM[Row].end(); ii++) {
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
    void SparseAddRow(SparseMatrix &SM, Scalar Factor, int Row1, int Row2) {
        /* check for zero factor */

        if (Factor == S_zero()) {
            return;
        }

        /* get the beginning of the two rows to work with */

        const SparseRow &r1 = SM[Row1];
        SparseRow &r2 = SM[Row2];

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

    void SparseKnockOut(SparseMatrix2 &SM, int row, int col, int nRows) {
        Scalar x = Get_Matrix_Element2(SM, row, col);
        Scalar x_inv = S_inv(x);
        SparseRow sr;
        for (int j = col; j < (int) SM.size(); j++) {
#if 1
            auto ii = lower_bound(SM[j].begin(), SM[j].end(), Node2(0, row), cmp_row);
            if (ii != SM[j].end() && ii->getRow() == row) {
                ii->setElement(S_mul(ii->getElement(), x_inv));
                sr.push_back(Node(ii->getElement(), j));
            }
#else
            for (auto ii = SM[j].begin(); ii != SM[j].end() && ii->getRow() <= row; ii++) {
                if (ii->getRow() == row) {
                    ii->setElement(S_mul(ii->getElement(), x_inv));
                    sr.push_back(Node(ii->getElement(), j));
                }
            }
#endif
        }

        /* try to knockout elements in column in the rows above */

        const auto ss = SM[col];

//    printf("sr.size:%d\n", sr.size());
#pragma omp parallel for shared(sr, nRows, row, SM, ss) schedule(dynamic, 10) default(none)
        for (int j0 = 0; j0 < (int) sr.size(); j0++) {
            Scalar e = sr[j0].getElement();
            int j = sr[j0].getColumn();

            vector<Node2> tmp;
            auto iii = ss.begin();
            auto &ss2 = SM[j];
            auto iii2 = ss2.begin();
            for (int i = 0; i < nRows; i++) {
                if (i == row) {
                    tmp.push_back(Node2(e, i));
                    continue;
                }

                Scalar x3 = S_zero();
                for (; iii != ss.end() && iii->getRow() < i; iii++) {
                }
                if (iii != ss.end() && iii->getRow() == i) {
                    x3 = iii->getElement();
                }

                Scalar x3b = S_zero();
                for (; iii2 != ss2.end() && iii2->getRow() < i; iii2++) {
                }
                if (iii2 != ss2.end() && iii2->getRow() == i) {
                    x3b = iii2->getElement();
                }
                Scalar x5;
                if (x3 == S_zero()) {
                    x5 = x3b;
                } else {
                    x5 = S_add(x3b, S_mul(S_minus(x3), e));
                }
                if (x5 != S_zero()) {
                    tmp.push_back(Node2(x5, i));
                }
            }
//        SparseCol(tmp.begin(), tmp.end()).swap(SM[j]); // shrink capacity while assigning
            SM[j] = tmp;
        }
//
//    if (x != S_one()) {
//        /* if the rightmost element in the current row is not one then multiply*/
////        SparseMultRow(SM, row, S_inv(x));
////        S_mul(ii->getElement(), Factor)
//    }
//
//    for (int j = 0; j < (int) SM.size(); j++) {
//        if (j != row) {
//            SparseAddRow(SM, S_minus(Get_Matrix_Element2(SM, j, col)), row, j);
//        }
//    }
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

#if 1

    Scalar Get_Matrix_Element2(const SparseMatrix2 &SM, int i, int j) {
        auto ii = lower_bound(SM[j].begin(), SM[j].end(), Node2(0, i), cmp_row);
        if (ii != SM[j].end() && ii->getRow() == i) {
            return ii->getElement();
        }

        return S_zero();
    }

#else

    Scalar Get_Matrix_Element2(const SparseMatrix2 &SM, int i, int j) {
        /* either return the element at location i,j or return a zero */
        for (auto ii = SM[j].cbegin(); ii != SM[j].cend() && ii->getRow() <= i; ii++) {
            if (ii->getRow() == i) return ii->getElement();
        }
        return S_zero();
    }

#endif

#if 0

    void Print_SLList(Node *SLHead_Ptr) {
        Node *Prt_Ptr;

        Prt_Ptr = SLHead_Ptr;

        printf("\nColumn :");
        while (Prt_Ptr != NULL) {
            printf(" %3d", Prt_Ptr->column);
            Prt_Ptr = Prt_Ptr->Next_Node;
        }

        Prt_Ptr = SLHead_Ptr;

        printf("\n");
        printf("Element:");
        while (Prt_Ptr != NULL) {
            printf(" %3d", Prt_Ptr->element);
            Prt_Ptr = Prt_Ptr->Next_Node;
        }
        printf("\n");
        printf("\n");
    }

    void Print_Node(NODE_PTR Prt_Node) {
        if (Prt_Node == NULL) {
            printf("NULL\n");
            return;
        }
        printf("Node element:%d\tcolumn:%d\n", Prt_Node->element,
               Prt_Node->column);
    }

#endif

}

int SparseReduceMatrix2(SparseMatrix &SM, int nCols, int *Rank) {
    return SparseReduceMatrix2_ns::SparseReduceMatrix(SM, nCols, Rank);
}
