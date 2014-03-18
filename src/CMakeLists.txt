set(finnapomodorod_SRC
  main.cpp
)

add_definitions(-Wall
  -Wextra
  -Wsequence-point
  -Wnarrowing
)

link_directories(
  ${GLIBMM_LIBRARY_DIRS}
  ${GIOMM_LIBRARY_DIRS}
)

include_directories(
  ${GLIBMM_INCLUDE_DIRS}
  ${GIOMM_INCLUDE_DIRS}
)

add_executable(${PROJECT_NAME}
  ${finnapomodorod_SRC}
)

target_link_libraries(${PROJECT_NAME}
  ${GLIBMM_LIBRARIES}
  ${GIOMM_LIBRARIES}
)
