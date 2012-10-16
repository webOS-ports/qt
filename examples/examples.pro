TEMPLATE      = subdirs
SUBDIRS       = \
                statemachine

!contains(QT_CONFIG, no-gui) {
    SUBDIRS  += \
                effects
}

contains(QT_CONFIG, webkit):SUBDIRS += webkit

symbian: SUBDIRS = \
                graphicsview \
                itemviews \
                network \
                painting \
                widgets \
                draganddrop \
                mainwindows \
                sql \
                uitools \
                animation \
                gestures \
                xml

contains(DEFINES, QT_NO_GESTURES): SUBDIRS -= gestures

!contains(QT_CONFIG, no-gui):contains(QT_CONFIG, multimedia) {
    SUBDIRS += multimedia
}

contains(QT_CONFIG, phonon):!static: SUBDIRS += phonon
contains(QT_CONFIG, opengl): SUBDIRS += opengl
contains(QT_CONFIG, openvg): SUBDIRS += openvg
contains(QT_CONFIG, dbus): SUBDIRS += dbus
win32:!win32-g++*: SUBDIRS += activeqt
contains(QT_CONFIG, xmlpatterns):!contains(QT_CONFIG, no-gui): SUBDIRS += xmlpatterns
contains(DEFINES, QT_NO_CURSOR): SUBDIRS -= mainwindows

# install
sources.files = README *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]
INSTALLS += sources

