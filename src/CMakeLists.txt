include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/third-party)

set(HEADERS
    animation/animationfactory.h
    animation/animator.h
    animation/disabledanimator.h
    animation/enabledanimator.h
    animation/expansionanimationfactory.h
    animation/floatanimationfactory.h
    geometrymanager/expansiongeometrymanager.h
    geometrymanager/floatgeometrymanager.h
    geometrymanager/geometrymanager.h
    ipc/dbusservice.h
    ipc/fcitxvirtualkeyboardserviceproxy.h
    ipc/requestmerger.h
    ipc/keyboardserviceproxy.h
    ipc/ukuimenuserviceproxy.h
    ipc/fcitxqtdbustypes.h
    ipc/fcitxcontrollerserviceproxy.h
    localsettings/localsettings.h
    localsettings/viewlocalsettings.h
    screenwatcher.h
    themewatcher.h
    virtualkeyboard/placementmodemanager.h
    virtualkeyboard/virtualkeyboardmanager.h
    virtualkeyboard/virtualkeyboardmodel.h
    virtualkeyboard/virtualkeyboardstrategy.h
    virtualkeyboard/virtualkeyboardview.h
    virtualkeyboard/shadowborderitem.h
    virtualkeyboardentry/fcitxvirtualkeyboardservice.h
    virtualkeyboardentry/floatbutton.h
    virtualkeyboardentry/floatbuttonmanager.h
    virtualkeyboardentry/floatbuttonstrategy.h
    virtualkeyboardentry/virtualkeyboardentrymanager.h
    virtualkeyboardentry/virtualkeyboardtrayicon.h
    virtualkeyboardentry/trayiconstrategy.h
    virtualkeyboardsettings/virtualkeyboardsettings.h
    log.h
    messagehandler.h
    commandlinehandler.h
    errorhandler.h
    workspaceadjuster/layershellworkspaceadjuster.h
)

set(SOURCES
    animation/animationfactory.cpp
    animation/disabledanimator.cpp
    animation/enabledanimator.cpp
    animation/expansionanimationfactory.cpp
    animation/floatanimationfactory.cpp
    geometrymanager/expansiongeometrymanager.cpp
    geometrymanager/floatgeometrymanager.cpp
    geometrymanager/geometrymanager.cpp
    ipc/dbusservice.cpp
    ipc/fcitxvirtualkeyboardserviceproxy.cpp
    ipc/requestmerger.cpp
    ipc/keyboardserviceproxy.cpp
    ipc/ukuimenuserviceproxy.cpp
    ipc/fcitxqtdbustypes.cpp
    ipc/fcitxcontrollerserviceproxy.cpp
    localsettings/viewlocalsettings.cpp
    main.cpp
    screenwatcher.cpp
    themewatcher.cpp
    virtualkeyboard/placementmodemanager.cpp
    virtualkeyboard/virtualkeyboardmanager.cpp
    virtualkeyboard/virtualkeyboardmodel.cpp
    virtualkeyboard/virtualkeyboardview.cpp
    virtualkeyboard/virtualkeyboardviewstate.cpp
    virtualkeyboard/shadowborderitem.cpp
    virtualkeyboardentry/floatbutton.cpp
    virtualkeyboardentry/floatbuttonmanager.cpp
    virtualkeyboardentry/virtualkeyboardentrymanager.cpp
    virtualkeyboardentry/virtualkeyboardtrayicon.cpp
    virtualkeyboardsettings/virtualkeyboardsettings.cpp
    workspaceadjuster/x11workspaceadjuster.cpp
    workspaceadjuster/waylandworkspaceadjuster.cpp
    workspaceadjuster/layershellworkspaceadjuster.cpp
    log.cpp
    messagehandler.cpp
    commandlinehandler.cpp
    errorhandler.cpp
)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES} ${RESOURCES})
target_link_libraries(${PROJECT_NAME}
                Qt${QT_VERSION_MAJOR}::Core
                Qt${QT_VERSION_MAJOR}::Gui
                Qt${QT_VERSION_MAJOR}::Widgets
                Qt${QT_VERSION_MAJOR}::Concurrent
                Qt${QT_VERSION_MAJOR}::DBus
                Qt${QT_VERSION_MAJOR}::Quick
                Qt${QT_VERSION_MAJOR}::QuickControls2
                KF${QT_VERSION_MAJOR}::WindowSystem
                Fcitx5::Core
                ${GSettings_QT_LIBRARIES}
                qtsingleapplication
                spdlog::spdlog
            )
if (QT_VERSION_MAJOR EQUAL 6)
        target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::Core5Compat)
endif()

if (ENABLE_LAYER_SHELL AND LayerShellQt_FOUND)
    target_link_libraries(${PROJECT_NAME} LayerShellQt::Interface)
endif()

install(TARGETS ${PROJECT_NAME}  DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
