cmake_minimum_required(VERSION 3.10)

project(ces41lex VERSION 0.1 LANGUAGES C)

include(CMakePrintHelpers)
cmake_print_variables(CMAKE_VERSION)

SET(DOPARSE TRUE CACHE BOOL "if false, bison is not used, and only lexical analysis is performed")
SET(BISON_EXECUTABLE "/opt/homebrew/opt/bison/bin/bison")
SET(CMAKE_BUILD_TYPE Debug)

if(DOPARSE)
    find_package(BISON REQUIRED)
endif()
find_package(FLEX REQUIRED)

find_library(FL_LIBRARY NAMES fl PATHS /opt/homebrew/opt/flex/lib /usr/local/opt/flex/lib /usr/lib)

SET(CES41_SRC "src" CACHE FILEPATH "Directory with student sources")

include_directories(  ${CMAKE_CURRENT_BINARY_DIR} include lib)

FILE(GLOB labSrc ${CES41_SRC}/*.c  )
FILE(GLOB lablib lib/*.c  )

FLEX_TARGET(scanner ${CES41_SRC}/cminus.l  ${CMAKE_CURRENT_BINARY_DIR}/lexer.c )
if(DOPARSE)
    BISON_TARGET(myparser ${CES41_SRC}/cminus.y ${CMAKE_CURRENT_BINARY_DIR}/parser.c)
    ADD_FLEX_BISON_DEPENDENCY(scanner myparser)
endif()

message("   * DOPARSE = ${DOPARSE}")
message("   * Flex OUT = ${FLEX_scanner_OUTPUTS}")
if(DOPARSE)
    message("   * BisonOUT = ${BISON_myparser_OUTPUTS}")
else()
    message("   * Bison NOT CALLED. Only Lexical Analysis")
endif()
message("   * SRC FILES = ${labSrc}")

#include_directories(${CMAKE_CURRENT_BINARY_DIR})

if(DOPARSE)
    add_executable(mycmcomp
            ${labSrc}
            ${lablib}
            ${BISON_myparser_OUTPUTS}
            ${FLEX_scanner_OUTPUTS}
            src/hash.c
    )
    target_include_directories(mycmcomp PUBLIC ${CES41_SRC})
    target_link_libraries(mycmcomp ${FL_LIBRARY})
else()
    add_executable(mycmcomp
            ${labSrc}
            ${lablib}
            ${FLEX_scanner_OUTPUTS}
            src/hash.c
    )
    target_include_directories(mycmcomp PUBLIC ${CES41_SRC})
    target_link_libraries(mycmcomp ${FLEX_LIBRARIES} ${FL_LIBRARY})
endif()

add_custom_target(runmycmcomp ALL
        COMMENT "running mycmcomp"
        COMMAND ../scripts/runcmcomp
        DEPENDS mycmcomp
        VERBATIM
        USES_TERMINAL
)

add_custom_target(rundiff ALL
        COMMENT "running diff"
        COMMAND ../scripts/rundiff
        DEPENDS runmycmcomp
        VERBATIM
        USES_TERMINAL
)

add_custom_target(ddiff
        COMMENT "running detail diff"
        COMMAND ../scripts/rundetaildiff
        DEPENDS runmycmcomp
        VERBATIM
        USES_TERMINAL
)

add_custom_target(lexdiff
        COMMENT "running lex diff (no syntax errors)"
        COMMAND ../scripts/runLEXdiff
        DEPENDS runmycmcomp
        VERBATIM
        USES_TERMINAL
)

########## compiling the tiny compiler  #############

if (DOPARSE)
    FLEX_TARGET(tinyscanner TinyGeracaoCodigo/tiny.l  ${CMAKE_CURRENT_BINARY_DIR}/tinylexer.c )
    BISON_TARGET(tinyparser TinyGeracaoCodigo/tiny.y ${CMAKE_CURRENT_BINARY_DIR}/tinyparser.c)
    ADD_FLEX_BISON_DEPENDENCY(tinyscanner tinyparser)
    FILE(GLOB tinycode TinyGeracaoCodigo/*.c  )
    add_executable(tiny
            ${tinycode}
            ${lablib}
            ${BISON_tinyparser_OUTPUTS}
            ${FLEX_tinyscanner_OUTPUTS}
    )
    target_include_directories(tiny PUBLIC TinyGeracaoCodigo )
    target_link_libraries(tiny ${FL_LIBRARY})
else()
    FLEX_TARGET(tinyscanner TinyFlex/tiny.l  ${CMAKE_CURRENT_BINARY_DIR}/tinylexer.c )
    FILE(GLOB tinycode TinyFlex/*.c  )
    add_executable(tiny
            ${tinycode}
            ${lablib}
            ${FLEX_tinyscanner_OUTPUTS}
    )
    target_include_directories(tiny PUBLIC TinyFlex )
    target_link_libraries(tiny ${FLEX_LIBRARIES} ${FL_LIBRARY})
endif()
