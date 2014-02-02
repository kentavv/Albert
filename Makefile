#Makefile for the Albert program.
#
# Trent Whiteley September 1993
BIN=../bin

#CFLAGS=  -g -p
CFLAGS=  -O
LDFLAGS= -lcurses -ltermcap -lm

OBJECTS= driver.o Get_Command.o Po_create_poly.o Po_routines.o Id_routines.o \
         Ty_routines.o Generators.o Field.o Po_parse_poly.o Help.o \
         Po_expand_poly.o Strings.o Po_parse_exptext.o Po_semantics.o \
         Po_syn_stack.o Po_prod_bst.o Memory_routines.o \
         Build.o Scalar_arithmetic.o Alg_elements.o Mult_table.o \
         Basis_table.o CreateMatrix.o ExtractMatrix.o \
         ReduceMatrix.o Type_table.o GenerateEquations.o Multpart.o \
         CreateSubs.o PerformSub.o SparseReduceMatrix.o node_mgt.o \
	 initGlobals.o

albert:			$(OBJECTS)
			gcc $(CFLAGS) -o $@ $(OBJECTS) $(LDFLAGS)
driver.o:		driver.c Build_defs.h Po_parse_exptext.h Id_routines.h Get_Command.h \
			Basis_table.h
			gcc $(CFLAGS) -c driver.c
Get_Command.o:		Get_Command.c Get_Command.h
			gcc $(CFLAGS) -c Get_Command.c
Po_create_poly.o:	Po_create_poly.c Po_create_poly.h Po_parse_poly.h \
			Po_semantics.h Po_prod_bst.h Po_parse_exptext.h
			gcc $(CFLAGS) -c Po_create_poly.c
Po_routines.o:		Po_routines.c Build_defs.h Po_parse_exptext.h Debug.h \
			Alg_elements.h
			gcc $(CFLAGS) -c Po_routines.c
Id_routines.o:		Id_routines.c Id_routines.h Po_parse_exptext.h
			gcc $(CFLAGS) -c Id_routines.c
Ty_routines.o:		Ty_routines.c Po_parse_exptext.h Ty_routines.h
			gcc $(CFLAGS) -c Ty_routines.c
Generators.o:		Generators.c Build_defs.h Po_parse_exptext.h
			gcc $(CFLAGS) -c Generators.c
Field.o:		Field.c Field.h Build_defs.h
			gcc $(CFLAGS) -c Field.c
Po_parse_poly.o:	Po_parse_poly.c Po_parse_poly.h Po_syn_stack.h \
			Po_semantics.h Po_prod_bst.h
			gcc $(CFLAGS) -c Po_parse_poly.c
Help.o:			Help.c Help.h
			gcc $(CFLAGS) -c Help.c
Po_expand_poly.o:	Po_expand_poly.c Po_parse_poly.h Po_semantics.h
			gcc $(CFLAGS) -c Po_expand_poly.c
Strings.o:		Strings.c
			gcc $(CFLAGS) -c Strings.c
Po_parse_exptext.o:	Po_parse_exptext.c Po_parse_exptext.h
			gcc $(CFLAGS) -c Po_parse_exptext.c
Po_semantics.o:		Po_semantics.c Po_parse_poly.h Po_syn_stack.h \
			Po_semantics.h
			gcc $(CFLAGS) -c Po_semantics.c
Po_syn_stack.o:		Po_syn_stack.c Po_syn_stack.h
			gcc $(CFLAGS) -c Po_syn_stack.c
Po_prod_bst.o:		Po_prod_bst.c Po_prod_bst.h
			gcc $(CFLAGS) -c Po_prod_bst.c
Memory_routines.o:	Memory_routines.c Po_parse_poly.h Po_prod_bst.h \
			Po_parse_exptext.h Id_routines.h
			gcc $(CFLAGS) -c Memory_routines.c
Build.o:		Build.c Build_defs.h Mult_table.h CreateMatrix.h \
			Po_parse_exptext.h Id_routines.h Sparse_structs.h \
			Sparse_defs.h Debug.h
			gcc $(CFLAGS) -c Build.c
Scalar_arithmetic.o:	Scalar_arithmetic.c Build_defs.h
			gcc $(CFLAGS) -c Scalar_arithmetic.c
Alg_elements.o:		Alg_elements.c Build_defs.h Alg_elements.h
			gcc $(CFLAGS) -c Alg_elements.c
Mult_table.o:		Mult_table.c Build_defs.h Alg_elements.h Mult_table.h
			gcc $(CFLAGS) -c Mult_table.c
Basis_table.o:		Basis_table.c Build_defs.h Basis_table.h
			gcc $(CFLAGS) -c Basis_table.c
CreateMatrix.o:		CreateMatrix.c Build_defs.h CreateMatrix.h \
			Sparse_structs.h Sparse_defs.h
			gcc $(CFLAGS) -c CreateMatrix.c
ExtractMatrix.o:	ExtractMatrix.c Build_defs.h CreateMatrix.h \
			Mult_table.h Sparse_structs.h Sparse_defs.h
			gcc $(CFLAGS) -c ExtractMatrix.c
ReduceMatrix.o:		ReduceMatrix.c Build_defs.h CreateMatrix.h
			gcc $(CFLAGS) -c ReduceMatrix.c
Type_table.o:		Type_table.c Build_defs.h Type_table.h
			gcc $(CFLAGS) -c Type_table.c
GenerateEquations.o:	GenerateEquations.c Build_defs.h CreateMatrix.h \
			Po_parse_exptext.h Debug.h
			gcc $(CFLAGS) -c GenerateEquations.c
Multpart.o:		Multpart.c Build_defs.h Type_table.h CreateMatrix.h \
			Po_parse_exptext.h Debug.h
			gcc $(CFLAGS) -c Multpart.c
CreateSubs.o:		CreateSubs.c Build_defs.h Type_table.h CreateMatrix.h \
			Po_parse_exptext.h Debug.h
			gcc $(CFLAGS) -c CreateSubs.c
PerformSub.o:		PerformSub.c Build_defs.h Alg_elements.h CreateMatrix.h \
			Po_parse_exptext.h PerformSub.h Debug.h
			gcc $(CFLAGS) -c PerformSub.c
SparseReduceMatrix.o:	SparseReduceMatrix.c Build_defs.h Sparse_structs.h \
			Sparse_defs.h
			gcc $(CFLAGS) -c SparseReduceMatrix.c
node_mgt.o:		node_mgt.c Sparse_structs.h Sparse_defs.h Debug.h node_mgt.h
			gcc $(CFLAGS) -c node_mgt.c
initGlobals.o:		initGlobals.c Build_defs.h Basis_table.h Mult_table.h
			gcc $(CFLAGS) -c initGlobals.c






install: albert
	/bin/rm -f ${BIN}/albert4
	cp albert ${BIN}/albert4
	chmod 775 ${BIN}/albert4
