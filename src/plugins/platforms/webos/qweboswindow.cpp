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

#include "qweboswindow.h"
#include "qwebosscreen.h"
#include "qwebosglcontext.h"
#include "hybriscompositorclient.h"

#include <QtGui/QWindowSystemInterface>
#include <QApplication>
#include <QSystemSemaphore>

#include <QElapsedTimer>
#include <QDebug>

#define MESSAGES_INTERNAL_FILE "SysMgrMessagesInternal.h"
#include <PIpcMessageMacros.h>

QT_BEGIN_NAMESPACE

#define Q_WEBOS_DEBUG 1

QWebOSWindow::QWebOSWindow(HybrisCompositorClient *client, QWidget *widget, QWebOSScreen *screen)
    : QPlatformWindow(widget),
      OffscreenNativeWindow(720, 1224),
      m_screen(screen),
      m_glcontext(0),
      m_client(client),
      m_winid(-1)
{
    qDebug() << __PRETTY_FUNCTION__ << "****************"<< widget << screen;
}

void QWebOSWindow::setGeometry(const QRect& rect)
{
    qDebug() << __PRETTY_FUNCTION__ << "****************"<<rect;

    // FIXME we need to rebuild the geometry of your OffscreenNativeWindow and it's
    // buffers here!

    QWindowSystemInterface::handleGeometryChange(this->widget(), rect);

    // Since toplevels are fullscreen, propegate the screen size back to the widget
    widget()->setGeometry(rect);
    QPlatformWindow::setGeometry(rect);
}

WId QWebOSWindow::winId() const
{
    return m_winid;
}

QPlatformGLContext *QWebOSWindow::glContext() const
{
    qDebug()<<__PRETTY_FUNCTION__;
    if(!m_glcontext) {
        qDebug()<<"lazy-initializing GL context!";
        const_cast<QWebOSWindow*>(this)->createGLContext();
    }
    return m_glcontext;
}

void QWebOSWindow::createGLContext()
{
    QPlatformWindowFormat format = widget()->platformWindowFormat();

    if (m_glcontext == NULL && format.windowApi() == QPlatformWindowFormat::OpenGL) {
        m_glcontext = new QWebOSGLContext( const_cast<QWebOSWindow*>(this) );
        qDebug() << __PRETTY_FUNCTION__ << "created" << m_glcontext;
    }
    else {
        qDebug() << __PRETTY_FUNCTION__ << "NO gl context created, format.windowAPI is" << format.windowApi();
    }
}

void QWebOSWindow::setVisible(bool visible)
{
    qDebug() << "QWebOSWindow::setVisible - " << visible;
    QPlatformWindow::setVisible(visible);
}

void QWebOSWindow::postBuffer(OffscreenNativeWindowBuffer *buffer)
{
    qDebug() << __PRETTY_FUNCTION__;

    // Only post buffer when we have assigned a valid window id as otherwise
    // the compositor can't associate the buffer with any active window
    if (m_winid != -1) {
        m_client->postBuffer(m_winid, buffer);
    }
}

QT_END_NAMESPACE
