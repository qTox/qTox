################################################################################
#
# :: Installation
#
################################################################################

if(APPLE)
  set(MACOSX_BUNDLE_SHORT_VERSION_STRING 1.4.1)
  set(SHORT_VERSION ${MACOSX_BUNDLE_SHORT_VERSION_STRING})
  set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/osx/info.plist")

  find_path(MACDEPLOYQT_PATH macdeployqt PATH_SUFFIXES bin)
  if(NOT MACDEPLOYQT_PATH)
    message(FATAL_ERROR "Could not find macdeployqt for OSX bundling. You can point MACDEPLOYQT_PATH to it's path.")
  endif()

  set(BUNDLE_PATH "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app")

  install(CODE "
  message(STATUS \"Creating app bundle\")
  execute_process(COMMAND ${MACDEPLOYQT_PATH}/macdeployqt ${BUNDLE_PATH} -no-strip)
  message(STATUS \"Updating library paths\")
  execute_process(COMMAND ${CMAKE_SOURCE_DIR}/osx/macfixrpath ${BUNDLE_PATH})
  message(STATUS \"Creating dmg image\")
  execute_process(COMMAND hdiutil create -volname ${PROJECT_NAME} -srcfolder ${BUNDLE_PATH} -ov -format UDZO ${PROJECT_NAME}.dmg)
  " COMPONENT Runtime
  )
else()
  install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "bin")
  install(FILES "res/qTox.appdata.xml" DESTINATION "share/appdata")
  install(FILES "qTox.desktop" DESTINATION "share/applications")

  # Install application icons according to the XDG spec
  set(ICON_SIZES 14 16 22 24 32 36 48 64 72 96 128 192 256 512)
  foreach(size ${ICON_SIZES})
    set(path_from "img/icons/${size}x${size}/qtox.png")
    set(path_to "share/icons/hicolor/${size}x${size}/apps/")
    install(FILES ${path_from} DESTINATION ${path_to})
  endforeach(size)

  install(FILES "img/icons/qtox.svg" DESTINATION "share/icons/hicolor/scalable/apps")
endif()
