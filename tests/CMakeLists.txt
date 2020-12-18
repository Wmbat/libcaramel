cmake_minimum_required( VERSION 3.14...3.17 FATAL_ERROR )

CPMAddPackage(
    NAME doctest
    VERSION 2.4.0
    GITHUB_REPOSITORY onqtam/doctest
    GIT_TAG 2.4.0
)

add_executable(libcaramel_test)

set_target_properties(libcaramel_test PROPERTIES CXX_EXTENSIONS OFF)

target_compile_features(libcaramel_test PRIVATE cxx_std_20)

target_compile_options(libcaramel_test
    PRIVATE
        $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:-o0 -g -Wall -Wextra -Werror -fsanitize=address -fprofile-instr-generate -fcoverage-mapping>
        $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:RELEASE>>:-o3>

        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:--coverage -o0 -g -Wall -Wextra -Werror -fprofile-arcs -ftest-coverage>
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:RELEASE>>:-o3>)

target_link_libraries(libcaramel_test
    PUBLIC
        libcaramel::libcaramel
        doctest::doctest

    PRIVATE
        $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:-fcoverage-mapping>
        $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:-fprofile-instr-generate>
        $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:DEBUG>>:-fsanitize=address>
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:-fprofile-arcs>
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:DEBUG>>:-ftest-coverage>)

target_sources(libcaramel_test PRIVATE basics/driver.cpp)

add_test(NAME libcaramel_test COMMAND libcaramel_test)
