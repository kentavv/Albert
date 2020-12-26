cmake_minimum_required(VERSION 3.16)
project(albert)

set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx")

add_executable(albert
        Alg_elements.cpp
        Alg_elements.h
        Basis_table.cpp
        Basis_table.h
        Build.cpp
        Build_defs.h
        Build.h
        CreateMatrix.cpp
        CreateMatrix.h
        CreateSubs.cpp
        CreateSubs.h
        Debug.h
        driver.cpp
        driver.h
        ExtractMatrix.cpp
        ExtractMatrix.h
        Field.cpp
        Field.h
        GenerateEquations.cpp
        GenerateEquations.h
        Generators.cpp
        Generators.h
        Get_Command.cpp
        Get_Command.h
        Help.cpp
        Help.h
        Help_pri.h
        Id_routines.cpp
        Id_routines.h
        Memory_routines.cpp
        Memory_routines.h
        Multpart.cpp
        Multpart.h
        Mult_table.cpp
        Mult_table.h
        PerformSub.cpp
        PerformSub.h
        Po_create_poly.cpp
        Po_create_poly.h
        Po_create_poly_pri.h
        Po_expand_poly.cpp
        Po_expand_poly.h
        Po_parse_exptext.cpp
        Po_parse_exptext.h
        Po_parse_poly.cpp
        Po_parse_poly.h
        Po_parse_poly_pri.h
        Po_prod_bst.cpp
        Po_prod_bst.h
        Po_routines.cpp
        Po_routines.h
        Po_semantics.cpp
        Po_semantics.h
        Po_syn_stack.cpp
        Po_syn_stack.h
        profile.h
        Scalar_arithmetic.cpp
        Scalar_arithmetic.h
        SparseReduceMatrix.cpp
        SparseReduceMatrix.h
        SparseReduceMatrix2.cpp
        SparseReduceMatrix2.h
        SparseReduceMatrix3.cpp
        SparseReduceMatrix3.h
        SparseReduceMatrix4.cpp
        SparseReduceMatrix4.h
        Strings.cpp
        Strings.h
        Type_table.cpp
        Type_table.h
        Ty_routines.cpp
        Ty_routines.h
        Ty_routines_pri.h
        matrix_reduce.cpp
        matrix_reduce.h
        matrix_reduce_float.cpp
        matrix_reduce_float.h
        )

find_package(OpenMP REQUIRED)
#find_package(readline REQUIRED)
target_link_libraries(albert PRIVATE OpenMP::OpenMP_CXX readline)
