cmake_minimum_required(VERSION 3.17)
project(wordgame C)

set(CMAKE_C_STANDARD 99)
add_compile_options(-g -Wall -Wextra -pedantic)

# set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_STATIC_LIBRARY_SUFFIX})
# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS} -mmacosx-version-min=11.0")


set(INCLUDE_DIR include)
include_directories (${INCLUDE_DIR})     # make sure your .h all inside.
link_directories(${CMAKE_SOURCE_DIR}/libs)

find_package(Curses REQUIRED)
find_package(Threads REQUIRED)

# find_library(CURSES_LIBRARY Curses REQUIRED
# # NAMES libcurses.a libform.a libform_g.a libformw.a libformw_g.a libmenu.a libmenu_g.a libmenuw.a libmenuw_g.a libncurses++.a libncurses++w.a libncurses++w_g.a libncurses.a libncurses_g.a libncursesw.a libncursesw_g.a libpanel.a libpanel_g.a libpanelw.a libpanelw_g.a
# NAMES libcurses.a libncurses.a libncursesw.a
# HINTS "/usr/local/opt/ncurses/lib")


include_directories(${CURSES_INCLUDE_DIR})
include_directories(${THREADS_INCLUDE_DIR})
add_executable(chat client.c color_init.c gui.c)
add_executable(server server.c)
add_executable(game game.c)
add_executable(gameserver gameserver.c)
add_executable(wordgame wordgame.c)

target_link_libraries(chat  ${CURSES_LIBRARIES} /Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/lib/libpanel.tbd ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(game mylist.a)
target_link_libraries(gameserver mylist.a)
# target_link_libraries(${PROJECT_NAME} ${CURSES_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
# target_link_libraries(wordgame ${CURSES_LIBRARIES})
