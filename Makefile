#Makefile for the Albert program.
#
# Trent Whiteley September 1993
BIN=../bin

CC=gcc

#CFLAGS=-MMD
#CFLAGS=-g -O0 -Wall
#CFLAGS=-g -O -Wall
CFLAGS=-g -O2 -Wall
#CFLAGS=-g -O3 -flto -Wall

LDFLAGS=
#LDFLAGS=-g -O3 -flto
LIBS=-lcurses -ltermcap -lm

C_FILES=$(wildcard *.c)
OBJECTS=$(notdir $(C_FILES:.c=.o))

albert: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

doxygen: $(wildcard *.c *.h)
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

Alg_elements.o: Alg_elements.c Build_defs.h Alg_elements.h \
 Memory_routines.h Po_prod_bst.h Mult_table.h Scalar_arithmetic.h
Basis_table.o: Basis_table.c Basis_table.h Build_defs.h Generators.h \
 Po_parse_exptext.h Help.h Memory_routines.h Po_prod_bst.h Type_table.h
Build.o: Build.c Build.h Id_routines.h Type_table.h Build_defs.h \
 Basis_table.h ExtractMatrix.h CreateMatrix.h Sparse_structs.h \
 Sparse_defs.h GenerateEquations.h Po_parse_exptext.h Mult_table.h \
 Alg_elements.h node_mgt.h ReduceMatrix.h SparseReduceMatrix.h Debug.h
CreateMatrix.o: CreateMatrix.c CreateMatrix.h Sparse_structs.h \
 Build_defs.h Sparse_defs.h Basis_table.h Memory_routines.h Po_prod_bst.h \
 Scalar_arithmetic.h SparseReduceMatrix.h Type_table.h
CreateSubs.o: CreateSubs.c CreateSubs.h Build_defs.h CreateMatrix.h \
 Sparse_structs.h Sparse_defs.h Po_parse_exptext.h Type_table.h \
 Memory_routines.h Po_prod_bst.h PerformSub.h GenerateEquations.h Debug.h
driver.o: driver.c driver.h Build_defs.h Basis_table.h Build.h \
 Id_routines.h Type_table.h Field.h Generators.h Po_parse_exptext.h \
 Get_Command.h Help.h initGlobals.h Memory_routines.h Po_prod_bst.h \
 Po_create_poly.h Po_routines.h Scalar_arithmetic.h Ty_routines.h \
 Mult_table.h Alg_elements.h
ExtractMatrix.o: ExtractMatrix.c ExtractMatrix.h Build_defs.h \
 CreateMatrix.h Sparse_structs.h Sparse_defs.h Basis_table.h \
 Memory_routines.h Po_prod_bst.h Mult_table.h Alg_elements.h \
 Scalar_arithmetic.h SparseReduceMatrix.h Type_table.h
Field.o: Field.c Field.h Build_defs.h
GenerateEquations.o: GenerateEquations.c GenerateEquations.h Build_defs.h \
 CreateMatrix.h Sparse_structs.h Sparse_defs.h Po_parse_exptext.h \
 Memory_routines.h Po_prod_bst.h Multpart.h CreateSubs.h Debug.h \
 Type_table.h
Generators.o: Generators.c Generators.h Build_defs.h Po_parse_exptext.h
getchar.o: getchar.c getchar.h
Get_Command.o: Get_Command.c Get_Command.h Memory_routines.h \
 Po_prod_bst.h Strings.h Type_table.h Build_defs.h getchar.h
Help.o: Help.c Help.h Help_pri.h
Id_routines.o: Id_routines.c Id_routines.h Memory_routines.h \
 Po_prod_bst.h Po_parse_exptext.h Po_routines.h
initGlobals.o: initGlobals.c initGlobals.h Build_defs.h Basis_table.h \
 Memory_routines.h Po_prod_bst.h Mult_table.h Alg_elements.h
Memory_routines.o: Memory_routines.c Memory_routines.h Po_prod_bst.h \
 Po_parse_poly.h Po_parse_exptext.h Id_routines.h
Multpart.o: Multpart.c Multpart.h Build_defs.h CreateSubs.h \
 CreateMatrix.h Sparse_structs.h Sparse_defs.h Po_parse_exptext.h \
 Type_table.h Memory_routines.h Po_prod_bst.h Debug.h
Mult_table.o: Mult_table.c Mult_table.h Build_defs.h Alg_elements.h \
 Help.h Memory_routines.h Po_prod_bst.h Scalar_arithmetic.h Basis_table.h
node_mgt.o: node_mgt.c node_mgt.h Sparse_structs.h Build_defs.h \
 Sparse_defs.h Memory_routines.h Po_prod_bst.h Debug.h
PerformSub.o: PerformSub.c PerformSub.h Build_defs.h CreateMatrix.h \
 Sparse_structs.h Sparse_defs.h GenerateEquations.h Po_parse_exptext.h \
 Alg_elements.h Memory_routines.h Po_prod_bst.h Debug.h \
 Scalar_arithmetic.h
Po_create_poly.o: Po_create_poly.c Po_create_poly.h Po_parse_exptext.h \
 Po_create_poly_pri.h Po_expand_poly.h Memory_routines.h Po_prod_bst.h \
 Po_parse_poly.h Po_semantics.h Strings.h
Po_expand_poly.o: Po_expand_poly.c Po_expand_poly.h Memory_routines.h \
 Po_prod_bst.h Po_parse_poly.h Po_semantics.h Ty_routines.h \
 Po_parse_exptext.h
Po_parse_exptext.o: Po_parse_exptext.c Po_parse_exptext.h \
 Memory_routines.h Po_prod_bst.h
Po_parse_poly.o: Po_parse_poly.c Po_parse_poly.h Po_parse_poly_pri.h \
 Po_syn_stack.h Po_semantics.h Po_prod_bst.h
Po_prod_bst.o: Po_prod_bst.c Po_prod_bst.h Memory_routines.h
Po_routines.o: Po_routines.c Po_routines.h Po_parse_exptext.h \
 Build_defs.h Generators.h Debug.h Alg_elements.h Memory_routines.h \
 Po_prod_bst.h Scalar_arithmetic.h getchar.h
Po_semantics.o: Po_semantics.c Po_parse_poly.h Po_syn_stack.h \
 Po_semantics.h Memory_routines.h Po_prod_bst.h
Po_syn_stack.o: Po_syn_stack.c Po_syn_stack.h Po_parse_poly.h
ReduceMatrix.o: ReduceMatrix.c ReduceMatrix.h Build_defs.h CreateMatrix.h \
 Sparse_structs.h Sparse_defs.h Scalar_arithmetic.h
Scalar_arithmetic.o: Scalar_arithmetic.c Scalar_arithmetic.h Build_defs.h \
 driver.h
SparseReduceMatrix.o: SparseReduceMatrix.c SparseReduceMatrix.h \
 Build_defs.h Sparse_structs.h Sparse_defs.h Scalar_arithmetic.h \
 node_mgt.h
Strings.o: Strings.c Strings.h Memory_routines.h Po_prod_bst.h
Type_table.o: Type_table.c Type_table.h Build_defs.h Basis_table.h \
 Memory_routines.h Po_prod_bst.h
Ty_routines.o: Ty_routines.c Ty_routines.h Po_parse_exptext.h \
 Ty_routines_pri.h
