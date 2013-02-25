/**
 *  Copyright (c) 2012 Hewlett-Packard Development Company, L.P.
 *                2013 Simon Busch <morphis@gravedo.de>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "qwebosintegration.h"

#include "qweboswindow.h"
#include "qweboswindowsurface.h"
#include "qwebosglcontext.h"
#include "qwebosscreen.h"

#include <QtGui/QPlatformWindow>
#include "qwebosclipboard.h"
#include "qwebosfontdatabase.h"
#include "qwebosnativeinterface.h"
#include <QtGui/QPlatformWindowFormat>
#include <QtGui/private/qpixmap_raster_p.h>
#include <QtOpenGL/private/qpixmapdata_egl_p.h>
#include <QtOpenGL/private/qpixmapdata_gl_p.h>
#include <EGL/egl.h>

#include <glib.h>
#include <QApplication>

QT_BEGIN_NAMESPACE

QPlatformClipboard* QWebOSIntegration::clipboard() const {
    return (QWebOSClipboard*) m_clipboard;
}

QWebOSIntegration::QWebOSIntegration()
    : m_fontDb(new QWebOSFontDatabase()),
      m_nativeInterface(new QWebOSNativeInterface()),
      m_primaryScreen(0)
{
    qDebug()<<__PRETTY_FUNCTION__;
    m_clipboard = new QWebOSClipboard();

    m_primaryScreen = new QWebOSScreen();

    m_screens.append(m_primaryScreen);
    QWebOSGLContext::initialize(EGL_DEFAULT_DISPLAY);
}

bool QWebOSIntegration::hasCapability(QPlatformIntegration::Capability cap) const
{
    qDebug()<<__PRETTY_FUNCTION__;
    switch (cap) {
        case ThreadedPixmaps:
            return true;
        default:
            return QPlatformIntegration::hasCapability(cap);
    }
}

QPixmapData *QWebOSIntegration::createPixmapData(QPixmapData::PixelType type) const
{
    qDebug() << __PRETTY_FUNCTION__;
    return new QGLPixmapData(type);
}

QPlatformWindow *QWebOSIntegration::createPlatformWindow(QWidget *widget, WId winId) const
{
    Q_UNUSED(winId);

    qDebug() << __PRETTY_FUNCTION__;

    return new QWebOSWindow(&m_compositorClient, widget, m_primaryScreen);
}


QWindowSurface *QWebOSIntegration::createWindowSurface(QWidget *widget, WId winId) const
{
    Q_UNUSED(winId);

    qDebug() << __PRETTY_FUNCTION__;

    if(widget->platformWindowFormat().windowApi() == QPlatformWindowFormat::OpenGL)
        return new QWebOSGLWindowSurface(m_primaryScreen, widget);
    else {
        qFatal("non-opengl windowsurface not implemented yet!");
        return 0;
    }
}

QPlatformFontDatabase* QWebOSIntegration::fontDatabase() const
{
    return m_fontDb;
}

QPlatformNativeInterface* QWebOSIntegration::nativeInterface() const
{
    return m_nativeInterface;
}

QT_END_NAMESPACE
