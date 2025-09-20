kcoreaddons_add_plugin(shakecursorextreme INSTALL_NAMESPACE "kwin/effects/plugins")

target_sources(shakecursorextreme PRIVATE
    main.cpp
    shakecursor.cpp
    shakedetector.cpp
)

kconfig_add_kcfg_files(shakecursorextreme
    shakecursorconfig.kcfgc
)

#Dave, I hate it when people use org.kde for things not in tree, what sort of example is this.
qt6_add_qml_module(shakecursorextreme
    NO_PLUGIN
    URI org.kde.kwin.shakecursor
    RESOURCE_PREFIX /qt/qml
    QML_FILES
        qml/Main.qml
)


target_link_libraries(shakecursorextreme PRIVATE
    KWin::kwin

    KF6::ConfigGui
    Qt::Quick
)
