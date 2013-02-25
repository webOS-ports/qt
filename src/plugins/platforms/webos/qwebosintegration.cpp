/**
 *  Copyright (c) 2012 Hewlett-Packard Development Company, L.P.
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
#include <QtGui/QPlatformWindowFormat>
#include <QtGui/private/qpixmap_raster_p.h>
#include <QtOpenGL/private/qpixmapdata_egl_p.h>
#include <EGL/egl.h>

#include <glib.h>
#include <QApplication>

QT_BEGIN_NAMESPACE

QPlatformClipboard* QWebOSIntegration::clipboard() const {
    return (QWebOSClipboard*)m_clipboard;
}

QWebOSIntegration::QWebOSIntegration()
    : mFontDb(new QWebOSFontDatabase())
    , m_primaryScreen(0)
{
    qDebug()<<__PRETTY_FUNCTION__;
    m_clipboard = new QWebOSClipboard();

    m_gContext = g_main_context_default();
    m_gMainLoop = g_main_loop_new(m_gContext, TRUE); //not running
    m_primaryScreen = new QWebOSScreen("sysmgr","some qpa app", m_gMainLoop);

    mScreens.append(m_primaryScreen);
    QWebOSGLContext::initialize(1 /* /dev/fb1 */);
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
    qDebug()<<__PRETTY_FUNCTION__;
    // FIXME right pixmap data? depends on paint engine or window api?
    return new QRasterPixmapData(type);
}

QPlatformWindow *QWebOSIntegration::createPlatformWindow(QWidget *widget, WId winId) const
{
    qDebug()<<__PRETTY_FUNCTION__;
    Q_UNUSED(winId); //FIXME what is winId - really unused?
    return new QWebOSWindow(widget, m_primaryScreen); //FIXME should this be differentiated into EGL and non EGL window types
}


QWindowSurface *QWebOSIntegration::createWindowSurface(QWidget *widget, WId winId) const
{
    qDebug()<<__PRETTY_FUNCTION__<<winId;
    Q_UNUSED(winId); //FIXME what is winId - really unused?
    if(widget->platformWindowFormat().windowApi() == QPlatformWindowFormat::OpenGL)
        return new QWebOSGLWindowSurface(m_primaryScreen, widget, m_gMainLoop);
    else {
        qFatal("non-opengl windowsurface not implemented yet!");
        return 0; // FIXME software window surface
    }
}

QPlatformFontDatabase *QWebOSIntegration::fontDatabase() const
{
    return mFontDb;
}

QT_END_NAMESPACE
