PROJECT(upload)

cmake_minimum_required(VERSION 2.8)

find_package(KDE4 REQUIRED)
find_package(KDevPlatform 1.6.60 REQUIRED)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 6)
set(VERSION_PATCH 60)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/version.h"
  @ONLY
)

include_directories(${KDEVPLATFORM_INCLUDE_DIR})

include_directories(
   ${KDE4_INCLUDES}
   ${CMAKE_CURRENT_BINARY_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(tests)

#plugin
set(kdevupload_PART_SRCS 
   kdevuploadplugin.cpp
   allprofilesmodel.cpp
   profilesfiletree.cpp
   uploaddialog.cpp
   uploadjob.cpp
   uploadprofiledlg.cpp
   uploadprofileitem.cpp
   uploadprofilemodel.cpp
   uploadprojectmodel.cpp
)
set(kdevupload_UI
    uploaddialog.ui
    uploadprofiledlg.ui
)
kde4_add_ui_files(kdevupload_PART_SRCS ${kdevupload_UI})

kde4_add_plugin(kdevupload WITH_PREFIX ${kdevupload_PART_SRCS})


target_link_libraries(kdevupload
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDE4_KIO_LIBS}
    ${KDE4_KUTILS_LIBS}
    ${KDE4_KFILE_LIBS}
    ${KDE4_KTEXTEDITOR_LIBS}
)

install(TARGETS kdevupload  DESTINATION ${PLUGIN_INSTALL_DIR})

configure_file(kdevupload.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevupload.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevupload.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES  kdevupload.rc DESTINATION ${DATA_INSTALL_DIR}/kdevupload)


#preferences-plugin
set(uploadpreferences_PART_SRCS 
   uploadpreferences.cpp
   uploadprofilemodel.cpp
   uploadprofileitem.cpp
   uploadprofiledlg.cpp
)
kde4_add_ui_files(uploadpreferences_PART_SRCS uploadpreferences.ui uploadprofiledlg.ui)

kde4_add_plugin(kcm_kdev_upload ${uploadpreferences_PART_SRCS})

target_link_libraries(kcm_kdev_upload
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDE4_KIO_LIBS}
    ${KDE4_KUTILS_LIBS}
    ${KDE4_KFILE_LIBS}
)

install(TARGETS kcm_kdev_upload  DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES  kcm_kdev_upload.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
