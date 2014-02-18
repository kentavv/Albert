#Makefile for the Albert program.
#
# Trent Whiteley September 1993
BIN=../bin

CC=gcc
CXX=g++

#CFLAGS=-MMD
#CFLAGS=-g -O0 -Wall
#CFLAGS=-g -O -Wall
CFLAGS=-g -O2 -Wall -fopenmp
#CFLAGS=-g -O2 -Wall -fopenmp -fprofile-generate -fprofile-correction
#CFLAGS=-g -O2 -Wall -fopenmp -fprofile-use -fprofile-correction
#CFLAGS=-g -O3 -flto -Wall

CXXFLAGS=$(CFLAGS)
#CXXFLAGS=$(CFLAGS) -std=c++11

LDFLAGS=-fopenmp
#LDFLAGS=-fopenmp -fprofile-generate -fprofile-correction
#LDFLAGS=-fopenmp -fprofile-use -fprofile-correction
#LDFLAGS=-g -O3 -flto
#LIBS=-lcurses -ltermcap -lm
LIBS=

C_FILES=$(wildcard *.c)
CPP_FILES=$(wildcard *.cpp)
OBJECTS=$(notdir $(C_FILES:.c=.o) $(CPP_FILES:.cpp=.o))

albert: $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

doxygen: $(wildcard *.c *.h *.cpp)
	doxygen doxygen_config

install: albert
	/bin/rm -f ${BIN}/albert4
	cp albert ${BIN}/albert4
	chmod 775 ${BIN}/albert4


clean:
	- rm -f albert *.o *.d *~ *# *.core core
	- rm -f cachegrind.out.* callgrind.out.*
	- rm -rf doxygen

#-include $(OBJFILES:.o=.d)

Alg_elements.o: Alg_elements.cpp Alg_elements.h Build_defs.h \
 Scalar_arithmetic.h Memory_routines.h Po_prod_bst.h Mult_table.h
Basis_table.o: Basis_table.cpp Basis_table.h Build_defs.h Generators.h \
 Po_parse_exptext.h Help.h Memory_routines.h Po_prod_bst.h Type_table.h
Build.o: Build.cpp Build.h Id_routines.h Po_parse_exptext.h Type_table.h \
 Build_defs.h Basis_table.h ExtractMatrix.h CreateMatrix.h \
 GenerateEquations.h Mult_table.h Alg_elements.h Scalar_arithmetic.h \
 ReduceMatrix.h SparseReduceMatrix.h Debug.h
CreateMatrix.o: CreateMatrix.cpp CreateMatrix.h Build_defs.h \
 Basis_table.h Memory_routines.h Po_prod_bst.h Scalar_arithmetic.h \
 SparseReduceMatrix.h Type_table.h pair_present.h
CreateSubs.o: CreateSubs.cpp CreateSubs.h Build_defs.h CreateMatrix.h \
 Po_parse_exptext.h Type_table.h Memory_routines.h Po_prod_bst.h \
 PerformSub.h GenerateEquations.h Debug.h
driver.o: driver.cpp driver.h Build_defs.h Basis_table.h Build.h \
 Id_routines.h Po_parse_exptext.h Type_table.h Field.h Generators.h \
 Get_Command.h Help.h Memory_routines.h Po_prod_bst.h Po_create_poly.h \
 Po_routines.h Scalar_arithmetic.h Ty_routines.h Mult_table.h \
 Alg_elements.h
ExtractMatrix.o: ExtractMatrix.cpp ExtractMatrix.h Build_defs.h \
 CreateMatrix.h Basis_table.h Memory_routines.h Po_prod_bst.h \
 Mult_table.h Alg_elements.h Scalar_arithmetic.h SparseReduceMatrix.h \
 Type_table.h
Field.o: Field.cpp Field.h Build_defs.h
GenerateEquations.o: GenerateEquations.cpp GenerateEquations.h \
 Build_defs.h CreateMatrix.h Po_parse_exptext.h Memory_routines.h \
 Po_prod_bst.h Multpart.h CreateSubs.h Debug.h Type_table.h
Generators.o: Generators.cpp Generators.h Build_defs.h Po_parse_exptext.h
getchar.o: getchar.cpp getchar.h
Get_Command.o: Get_Command.cpp Get_Command.h Memory_routines.h \
 Po_prod_bst.h Strings.h Type_table.h Build_defs.h getchar.h
Help.o: Help.cpp Help.h Help_pri.h
Id_routines.o: Id_routines.cpp Id_routines.h Po_parse_exptext.h \
 Memory_routines.h Po_prod_bst.h Po_routines.h
Memory_routines.o: Memory_routines.cpp Memory_routines.h Po_prod_bst.h \
 Po_parse_poly.h Po_parse_exptext.h Id_routines.h
Multpart.o: Multpart.cpp Multpart.h Build_defs.h CreateSubs.h \
 CreateMatrix.h Po_parse_exptext.h Type_table.h Memory_routines.h \
 Po_prod_bst.h Debug.h
Mult_table.o: Mult_table.cpp Mult_table.h Build_defs.h Alg_elements.h \
 Scalar_arithmetic.h Help.h Memory_routines.h Po_prod_bst.h Basis_table.h
pair_present.o: pair_present.cpp pair_present.h
PerformSub.o: PerformSub.cpp PerformSub.h Build_defs.h CreateMatrix.h \
 GenerateEquations.h Po_parse_exptext.h Alg_elements.h \
 Scalar_arithmetic.h Memory_routines.h Po_prod_bst.h Debug.h
Po_create_poly.o: Po_create_poly.cpp Po_create_poly.h Po_parse_exptext.h \
 Po_create_poly_pri.h Po_expand_poly.h Memory_routines.h Po_prod_bst.h \
 Po_parse_poly.h Po_semantics.h Strings.h
Po_expand_poly.o: Po_expand_poly.cpp Po_expand_poly.h Memory_routines.h \
 Po_prod_bst.h Po_parse_poly.h Po_semantics.h Ty_routines.h \
 Po_parse_exptext.h
Po_parse_exptext.o: Po_parse_exptext.cpp Po_parse_exptext.h \
 Memory_routines.h Po_prod_bst.h
Po_parse_poly.o: Po_parse_poly.cpp Po_parse_poly.h Po_parse_poly_pri.h \
 Po_syn_stack.h Po_semantics.h Po_prod_bst.h
Po_prod_bst.o: Po_prod_bst.cpp Po_prod_bst.h Memory_routines.h
Po_routines.o: Po_routines.cpp Po_routines.h Po_parse_exptext.h \
 Build_defs.h Generators.h Debug.h Alg_elements.h Scalar_arithmetic.h \
 Memory_routines.h Po_prod_bst.h
Po_semantics.o: Po_semantics.cpp Po_parse_poly.h Po_syn_stack.h \
 Po_semantics.h Memory_routines.h Po_prod_bst.h
Po_syn_stack.o: Po_syn_stack.cpp Po_syn_stack.h Po_parse_poly.h
ReduceMatrix.o: ReduceMatrix.cpp ReduceMatrix.h Build_defs.h \
 CreateMatrix.h Scalar_arithmetic.h
Scalar_arithmetic.o: Scalar_arithmetic.cpp Scalar_arithmetic.h \
 Build_defs.h driver.h
SparseReduceMatrix.o: SparseReduceMatrix.cpp SparseReduceMatrix.h \
 CreateMatrix.h Build_defs.h Scalar_arithmetic.h
Strings.o: Strings.cpp Strings.h Memory_routines.h Po_prod_bst.h
Type_table.o: Type_table.cpp Type_table.h Build_defs.h Basis_table.h \
 Memory_routines.h Po_prod_bst.h
Ty_routines.o: Ty_routines.cpp Ty_routines.h Po_parse_exptext.h \
 Ty_routines_pri.h
