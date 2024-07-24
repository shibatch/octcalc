add_library(octcore octcore.cpp)
target_link_libraries(octcore tlfloat)
add_dependencies(octcore ext_tlfloat)

add_executable(octcli octcli.cpp)
target_link_libraries(octcli octcore)
add_dependencies(octcli ext_tlfloat)

if (WIN32)
  add_executable(octcalc WIN32 octgui.cpp main.cpp octcalc.rc)
  target_link_libraries(octcalc octcore)
  add_dependencies(octcalc ext_tlfloat)
  target_link_libraries(octcalc Qt6::Widgets)

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

    install(
      FILES LGPL3Qt.txt
      DESTINATION "${INSTALL_BINDIR}"
      COMPONENT libqt
      )
  endif()

  install(
    TARGETS octcalc octcli
    DESTINATION "${INSTALL_BINDIR}"
    COMPONENT runtime
    )
else()
  add_executable(octcalc octgui.cpp main.cpp)
  target_link_libraries(octcalc octcore)
  add_dependencies(octcalc ext_tlfloat)
  target_link_libraries(octcalc Qt6::Widgets)

  if (LC_CMAKE_BUILD_TYPE STREQUAL "debug")
    target_compile_definitions(octcalc PRIVATE DEBUG=1)
  endif()

  install(
    TARGETS octcalc octcli
    DESTINATION "${INSTALL_BINDIR}"
    COMPONENT runtime
    )
endif()
