project(isolation_tester)

add_executable(isolation_tester isolation_tester.cpp)
target_link_libraries(isolation_tester cap)

install(TARGETS isolation_tester
    CONFIGURATIONS Release
    RUNTIME_DESTINATION bin)
