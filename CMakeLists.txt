cmake_minimum_required(VERSION 3.0)
project(upload)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 9)
set(VERSION_PATCH 90)
# KDevplatform dependency version
set( KDEVPLATFORM_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )

configure_file( "${upload_SOURCE_DIR}/version.h.cmake" "${upload_BINARY_DIR}/version.h" @ONLY )

find_package (ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${upload_SOURCE_DIR}/cmake/)

include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMAddTests)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(FeatureSummary)
include(ECMQtDeclareLoggingCategory)
include(KDECMakeSettings)
include(WriteBasicConfigVersionFile)

set(QT_MIN_VERSION "5.5.0")
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core Widgets)
set(KF5_DEP_VERSION "5.15.0")
find_package(KF5 ${KF5_DEP_VERSION} REQUIRED COMPONENTS Config TextEditor I18n KCMUtils JobWidgets Service Parts KIO CoreAddons ItemModels XmlGui)
find_package(KDevPlatform ${KDEVPLATFORM_VERSION} REQUIRED)

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
   uploadpreferences.cpp
)
set(kdevupload_UI
    uploaddialog.ui
    uploadprofiledlg.ui
    uploadpreferences.ui
)

ecm_qt_declare_logging_category(projectfilter_SRCS
    HEADER debug.h
    IDENTIFIER PLUGIN_PROJECTFILTER
    CATEGORY_NAME "kdevelop.plugins.upload"
)

ki18n_wrap_ui(uploadpreferences_PART_SRCS uploadpreferences.ui uploadprofiledlg.ui uploaddialog.ui)
kconfig_add_kcfg_files( kdevupload_PART_SRCS uploadconfig.kcfgc )

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
