# Add a gtest executable for the seq module
add_executable(seq_test
    seq.cpp
    lit.cpp
)
target_link_libraries(seq_test PRIVATE GTest::gtest_main)
target_include_directories(seq_test PRIVATE ${GTest_INCLUDE_DIRS} ../include)
