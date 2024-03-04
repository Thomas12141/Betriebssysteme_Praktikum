cmake_minimum_required(VERSION 3.16.3)  # CMake version check
project(Bertriebssysteme) # Create project "Bertriebssysteme"
set(CMAKE_C_STANDARD 99)

#== ASAN ==#
# add_compile_options(-fsanitize=address)
# add_link_options(-fsanitize=address)

# Compiler Flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -pedantic -Wall -Werror -Wconversion -g")

# ADD Source files to Variable for executables
set(SOURCES_FOR_STARTER
        src/osmp_runner/osmp_run.c src/osmp_runner/osmp_run.h
    )
set(MAIN_SOURCES_FOR_EXECUTABLES
        src/osmp_library/OSMP.h
    )
set(LIBRARY_SRC_FILES
        src/osmp_library/osmplib.c src/osmp_library/osmplib.h
        src/osmp_library/OSMP.h
    )

# set sources for each executable
set(SOURCES_FOR_EXECUTABLE_EXAMPLE_BCAST
        src/osmp_executables/osmp_Bcast.c
        ${MAIN_SOURCES_FOR_EXECUTABLES}
    )
set(SOURCES_FOR_EXECUTABLE_EXAMPLE_SENDIRECV
        src/osmp_executables/osmp_SendIrecv.c
        ${MAIN_SOURCES_FOR_EXECUTABLES}
    )
set(SOURCES_FOR_EXECUTABLE_EXAMPLE_SENDRECV
        src/osmp_executables/osmp_SendRecv.c
        ${MAIN_SOURCES_FOR_EXECUTABLES}
    )

# Add Library taget
add_library(libosmp STATIC ${LIBRARY_SRC_FILES})

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(osmp_run ${SOURCES_FOR_STARTER})
add_executable(osmpExecutable_Bcast ${SOURCES_FOR_EXECUTABLE_EXAMPLE_BCAST} )
add_executable(osmpExecutable_SendIrecv ${SOURCES_FOR_EXECUTABLE_EXAMPLE_SENDIRECV} )
add_executable(osmpExecutable_SendRecv ${SOURCES_FOR_EXECUTABLE_EXAMPLE_SENDRECV} )

# Link Library to executables
set(LIBRARIES
        pthread
        libosmp
        rt)

target_link_libraries(osmp_run ${LIBRARIES})
target_link_libraries(osmpExecutable_Bcast ${LIBRARIES})
target_link_libraries(osmpExecutable_SendIrecv ${LIBRARIES})
target_link_libraries(osmpExecutable_SendRecv ${LIBRARIES})