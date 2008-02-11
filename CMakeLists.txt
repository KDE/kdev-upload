include_directories(
   ${KDEVPLATFORM_INCLUDE_DIR}
   ${KDEVPLATFORM_INCLUDE_DIR}/interfaces
   ${KDEVPLATFORM_INCLUDE_DIR}/shell
   ${KDEVPLATFORM_INCLUDE_DIR}/project
)

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
    uploadpreferences.ui
)
kde4_add_ui_files(kdevupload_PART_SRCS ${kdevupload_UI})

kde4_add_plugin(kdevupload WITH_PREFIX ${kdevupload_PART_SRCS})


target_link_libraries(kdevupload kdevquanta kdevplatforminterfaces kdevplatformproject ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS})

install(TARGETS kdevupload  DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES  kdevupload.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
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

target_link_libraries(kcm_kdev_upload kdevquanta kdevplatforminterfaces ${KDE4_KDECORE_LIBS})

install(TARGETS kcm_kdev_upload  DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES  kcm_kdev_upload.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
