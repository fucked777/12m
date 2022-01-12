TEMPLATE  = subdirs
CONFIG   += ordered \
    qt

SUBDIRS += \
    core \
    plugins

OTHER_FILES += \
    .qmake.conf \
    .clang-format

QT += widgets
