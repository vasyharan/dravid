add_executable(test-snapshot main.cc)
target_compile_options(test-snapshot PRIVATE -Wall)
target_compile_features(test-snapshot PRIVATE cxx_std_17)
target_include_directories(test-snapshot PUBLIC ${lang_SOURCE_DIR})
target_link_libraries(test-snapshot compiler doctest cxxopts stdc++fs ${EXTRA_LIBS})
add_sanitizers(test-snapshot)

add_test(NAME snapshot
  COMMAND test-snapshot ${CMAKE_CURRENT_SOURCE_DIR}/_fixtures)
