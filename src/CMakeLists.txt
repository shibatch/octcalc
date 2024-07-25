add_library(octcore octcore.cpp)
target_link_libraries(octcore tlfloat)
add_dependencies(octcore ext_tlfloat)

if (WIN32)
  add_executable(octcalc WIN32 octgui.cpp main.cpp octcalc.rc)
  target_link_libraries(octcalc octcore Qt6::Widgets)
  add_dependencies(octcalc ext_tlfloat)

  if (ENABLE_WIX)
    add_custom_command(TARGET octcalc POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E remove_directory "${CMAKE_BINARY_DIR}/qtDeploy/"
      COMMAND "${WINDEPLOYQT_EXECUTABLE}"
      --no-compiler-runtime
      --no-system-d3d-compiler
      --no-system-dxc-compiler
      --no-translations
      --dir "${CMAKE_BINARY_DIR}/qtDeploy/" $<TARGET_FILE:octcalc>
      )
    set_property(INSTALL "bin/$<TARGET_FILE_NAME:octcalc>" PROPERTY CPACK_START_MENU_SHORTCUTS "OctCalc")

    install(
      DIRECTORY "${CMAKE_BINARY_DIR}/qtDeploy/"
      DESTINATION "${INSTALL_BINDIR}"
      COMPONENT libqt
      )
  endif()
else()
  add_executable(octcalc octgui.cpp main.cpp)
  target_link_libraries(octcalc octcore Qt6::Widgets)
  add_dependencies(octcalc ext_tlfloat)

  if (LC_CMAKE_BUILD_TYPE STREQUAL "debug")
    target_compile_definitions(octcalc PRIVATE DEBUG=1)
  endif()
endif()

install(
  TARGETS octcalc
  DESTINATION "${INSTALL_BINDIR}"
  COMPONENT runtime
  )

install(
  FILES licenses.txt
  DESTINATION "${INSTALL_DATADIR}"
  COMPONENT libqt
  )

add_executable(octcalc_test octgui.cpp main.cpp)
target_compile_definitions(octcalc_test PRIVATE TEST=1)
target_link_libraries(octcalc_test octcore Qt6::Widgets Qt6::Test)
add_dependencies(octcalc_test ext_tlfloat)
add_test(NAME test_octcalc COMMAND octcalc_test -platform offscreen)
