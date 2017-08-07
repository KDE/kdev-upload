project(upload)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 9)
set(VERSION_PATCH 90)
# KDevplatform dependency version
set( KDEVPLATFORM_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )

configure_file( "${upload_SOURCE_DIR}/version.h.cmake" "${upload_BINARY_DIR}/version.h" @ONLY )

cmake_minimum_required(VERSION 2.8.12)
find_package (ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${upload_SOURCE_DIR}/cmake/)

include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMAddTests)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(FeatureSummary)

set(QT_MIN_VERSION "5.5.0")
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core Widgets)
set(KF5_DEP_VERSION "5.15.0")
find_package(KF5 ${KF5_DEP_VERSION} REQUIRED COMPONENTS TextEditor I18n KCMUtils JobWidgets Service Parts KIO CoreAddons ItemModels)
find_package(KDevPlatform ${KDEVPLATFORM_VERSION} REQUIRED)

include_directories(${KDEVPLATFORM_INCLUDE_DIR})

include_directories(
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

add_library(kdevupload_PART_SRCS ${kdevupload_UI})

qt5_add_resources(kdevupload_PART_SRCS kdevupload.qrc)
kdevplatform_add_plugin(kdevupload JSON kdevupload.json SOURCES ${kdevupload_PART_SRCS})

target_link_libraries(kdevupload
    KDev::Interfaces
    KDev::Project
    KDev::Serialization

    KF5::JobWidgets
    KF5::KCMUtils
    KF5::KIOCore
    KF5::KIOFileWidgets
    KF5::KIOWidgets
    KF5::KIONTLM
    KF5::CoreAddons
)

#preferences-plugin
set(uploadpreferences_PART_SRCS
   uploadpreferences.cpp
   uploadprofilemodel.cpp
   uploadprofileitem.cpp
   uploadprofiledlg.cpp
)
ki18n_wrap_ui(uploadpreferences_PART_SRCS uploadpreferences.ui uploadprofiledlg.ui uploaddialog.ui)
add_library(uploadpreferences_PART_SRCS uploadpreferences.ui uploadprofiledlg.ui uploaddialog.ui)

add_library(kcm_kdev_upload ${uploadpreferences_PART_SRCS})

target_link_libraries(kcm_kdev_upload
    KDev::Interfaces
    KDev::Util

    KF5::I18n
    KF5::KCMUtils
    KF5::Service
    KF5::Parts
    KF5::KIOCore
    KF5::KIOFileWidgets
    KF5::KIOWidgets
    KF5::KIONTLM
)

install(TARGETS kcm_kdev_upload  DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES  kcm_kdev_upload.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
