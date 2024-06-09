cmake_minimum_required(VERSION 3.16.3)  # CMake version check
project(Bertriebssysteme) # Create project "Bertriebssysteme"
set(CMAKE_C_STANDARD 99)

#== ASAN ==#
# add_compile_options(-fsanitize=address)
# add_link_options(-fsanitize=address)

#== Compiler Flags ==#
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -pedantic -Wall -Werror -Wextra -Wconversion -g")

#== ADD Source files to Variable for executables ==#
set(SOURCES_FOR_STARTER
    src/osmp_runner/osmp_run.c src/osmp_runner/osmp_run.h
)
set(MAIN_SOURCES_FOR_EXECUTABLES
    src/osmp_library/OSMP.h
)
set(LIBRARY_SRC_FILES
    src/osmp_library/osmplib.c src/osmp_library/osmplib.h
    src/osmp_library/OSMP.h
    src/osmp_library/logger.c src/osmp_library/logger.c
)

#== set sources for each executable ==#
#== If you want to add a new executable, copy one of the following blocks and modify it ==#
set(SOURCES_FOR_EXECUTABLE_EXAMPLE_SENDIRECV
    src/osmp_executables/osmpExecutable_SendIRecv.c
    ${MAIN_SOURCES_FOR_EXECUTABLES}
)
set(SOURCES_FOR_EXECUTABLE_EXAMPLE_SENDRECV
    src/osmp_executables/osmpExecutable_SendRecv.c
    ${MAIN_SOURCES_FOR_EXECUTABLES}
)
set(SOURCES_FOR_EXECUTABLE_BARRIER
        src/osmp_executables/osmpExecutable_Barrier.c
        ${MAIN_SOURCES_FOR_EXECUTABLES}
)
set(SOURCES_FOR_EXECUTABLE_GATHER
        src/osmp_executables/osmpExecutable_Gather.c
        ${MAIN_SOURCES_FOR_EXECUTABLES}
)

#== Add executable target with source files listed in SOURCE_FILES variable ==#
#== If you added a new executable, add a new add_executable line here ==#
add_executable(osmp_run ${SOURCES_FOR_STARTER})
add_executable(osmpExecutable_SendIRecv ${SOURCES_FOR_EXECUTABLE_EXAMPLE_SENDIRECV} )
add_executable(osmpExecutable_SendRecv ${SOURCES_FOR_EXECUTABLE_EXAMPLE_SENDRECV} )
add_executable(osmpExecutable_Barrier ${SOURCES_FOR_EXECUTABLE_BARRIER} )
add_executable(osmpExecutable_Gather ${SOURCES_FOR_EXECUTABLE_GATHER} )

#== Add Library taget ==#
add_library(libosmp STATIC ${LIBRARY_SRC_FILES})
add_library(liblogger STATIC ${LIBRARY_SRC_FILES})

#== Link Library to executables ==#
#== If you added a new executable, add a new target_link_libraries line here ==#
set(LIBRARIES
        pthread
        libosmp
        rt
        liblogger)

target_link_libraries(osmp_run ${LIBRARIES})
target_link_libraries(osmpExecutable_SendIRecv ${LIBRARIES})
target_link_libraries(osmpExecutable_SendRecv ${LIBRARIES})
target_link_libraries(osmpExecutable_Barrier ${LIBRARIES})
target_link_libraries(osmpExecutable_Gather ${LIBRARIES})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")