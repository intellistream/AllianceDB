# adding the Google_Tests_run target
#set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-std=c++20 -Wall -Werror=return-type -Werror=unused-variable -Werror=unused-parameter")
enable_testing()
macro(add_catch_test appName SOURCE_FILES SOURCE_LIBS)
    get_property(SOURCE_FILES_LOCAL GLOBAL PROPERTY "${PROP_NAME}_SOURCE_PROP")
    set(${SOURCE_FILES} ${SOURCE_FILES_LOCAL})
    add_executable(${appName} ${SOURCE_FILES})
    target_link_libraries(${appName} ${SOURCE_LIBS})
    add_test(
            NAME catch_${appName}
            COMMAND $<TARGET_FILE:${appName}> --success
    )
endmacro()
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config_IMA.csv
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config_AI_TRAIN.csv
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config_Normal.csv
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config_fileDataLoader.csv
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/datasets/rTuple.csv
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/datasets/sTuple.csv
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/torchscripts
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
#add_executable(simpleTest SystemTest/SimpleTest.cpp)
add_catch_test(mswjTest SystemTest/MSWJTest.cpp IntelliStreamOoOJoin)
add_catch_test(iawjTest SystemTest/IAWJTest.cpp IntelliStreamOoOJoin)
add_catch_test(imaTest SystemTest/IMATest.cpp IntelliStreamOoOJoin)
add_catch_test(pecjTest SystemTest/PECJTest.cpp IntelliStreamOoOJoin)
add_catch_test(aiPretrainTest SystemTest/AIPretrainTest.cpp IntelliStreamOoOJoin)
add_catch_test(aiInferenceTest SystemTest/AIInferenceTest.cpp IntelliStreamOoOJoin)
add_catch_test(iawjSelTest SystemTest/IAWJSelTest.cpp IntelliStreamOoOJoin)
add_catch_test(lazyIawjSelTest SystemTest/LazyIAWJSelTest.cpp IntelliStreamOoOJoin)
add_catch_test(sviTest SystemTest/SVITest.cpp IntelliStreamOoOJoin)
add_catch_test(sviInferenceTest SystemTest/LinearSVIInferenceTest.cpp IntelliStreamOoOJoin)
add_catch_test(sviPretrainTest SystemTest/LinearSVIPretrainTest.cpp IntelliStreamOoOJoin)
add_catch_test(sviCLTest SystemTest/LinearSVIContinualLearningTest.cpp IntelliStreamOoOJoin)
add_catch_test(keyPartitionTest SystemTest/KeyPartitionTest.cpp IntelliStreamOoOJoin)
add_catch_test(roundRobinTest SystemTest/RoundRobinTest.cpp IntelliStreamOoOJoin)
#add_executable(iawjSelTest SystemTest/IAWJSelTest.cpp)
#add_library(TestFunction SystemTest/TestFunction.cpp)

message(STATUS " Libraries: ${LIBRARIES}")
#target_link_libraries(TestFunction
#IntelliStreamOoOJoin
#)


