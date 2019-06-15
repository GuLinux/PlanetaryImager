file(GLOB planetaryimager-widgets-SRCS *.cpp *.ui controls/*.cpp)
set(
    planetaryimager-widgets-SRCS
    cameracontrolswidget.cpp
    configurationdialog.cpp 
    histogramstatswidget.cpp
    localfilesystembrowser.cpp
    savefileconfiguration.cpp
    camerainfowidget.cpp
    editroidialog.cpp
    histogramwidget.cpp
    recordingpanel.cpp
    statusbarinfowidget.cpp
)
set(
    planetaryimager-widgets-UI
    cameracontrolswidget.ui
    configurationdialog.ui
    editroidialog.ui
    histogramstatswidget.ui
    histogramwidget.ui
    recordingpanel.ui
    savefileconfiguration.ui
    statusbarinfowidget.ui
)

set(
    controls-SRCS
    controls/boolcontrolwidget.cpp
    controls/controlwidget.cpp
    controls/durationcontrolwidget.cpp
    controls/menucontrolwidget.cpp
    controls/numbercontrolwidget.cpp
)

if(HAVE_LIBINDI)
    set(planetaryimager-widgets-SRCS ${planetaryimager-widgets-SRCS} mount_widget.cpp mount_dialog.cpp)
    set(planetaryimager-widgets-UI ${planetaryimager-widgets-UI} mount_dialog.ui mountwidget.ui)
endif()

add_library(planetaryimager-widgets STATIC ${planetaryimager-widgets-SRCS} ${planetaryimager-widgets-UI} ${controls-SRCS})
add_frontend_dependencies(planetaryimager-widgets)
