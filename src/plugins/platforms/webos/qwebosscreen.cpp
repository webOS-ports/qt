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

#include "qwebosscreen.h"
#include "qweboswindow.h"
#include <QDebug>
#include <QCoreApplication>
#define MESSAGES_INTERNAL_FILE "SysMgrMessagesInternal.h"
#include <PIpcMessageMacros.h>
#include <PIpcChannel.h>

QT_BEGIN_NAMESPACE

//FIXME should we get all the data - like screen size - from the sysmgr server after connection
QWebOSScreen::QWebOSScreen(const std::string& ipcServer,
                           const std::string& name,
                           GMainLoop* loop)
    : PIpcClient(ipcServer, name, loop)
    , m_depth(32)
    , m_format(QImage::Format_Invalid)
    , m_direct(false)
{
    m_geometry=QRect(0,0,1024,768); //FIXME get actual framebuffer size?
    qWarning() << "using hardcoded screen size in" << __PRETTY_FUNCTION__ << m_geometry;
    qDebug() << ipcServer.c_str() << name.c_str();
    g_main_loop_run(mainLoop());
    qDebug() << "processed events";
}

QRect QWebOSScreen::geometry() const
{
    return m_geometry;
}

int QWebOSScreen::depth() const
{
    return m_depth;
}

QImage::Format QWebOSScreen::format() const
{
    return m_format;
}

// implementing PIpcClient interface pure virtual functions
void QWebOSScreen::serverConnected(PIpcChannel* channel) {
    g_main_loop_quit(mainLoop());
    qDebug()<< __PRETTY_FUNCTION__<<channel;
    setChannel(channel);

}

void QWebOSScreen::serverDisconnected() {
    qDebug()<< __PRETTY_FUNCTION__;
    setChannel(0);
}

// implementing PIpcChannelListener interface pure virtual functions
void QWebOSScreen::onMessageReceived(const PIpcMessage& msg)
{
    qDebug()<< __PRETTY_FUNCTION__;
    QWebOSWindow *w = 0;
    if (!(w = platformWindowFromId(msg.routing_id())))
        return; // could't find target window

    bool msgIsOk;    
    IPC_BEGIN_MESSAGE_MAP(QWebOSWindow, msg, msgIsOk)
        IPC_MESSAGE_FORWARD(View_Focus, w, QWebOSWindow::handleFocus)
        IPC_MESSAGE_FORWARD(View_Resize, w, QWebOSWindow::handleResize)
        IPC_MESSAGE_FORWARD(View_FullScreenEnabled, w, QWebOSWindow::handleFullScreenEnabled)
        IPC_MESSAGE_FORWARD(View_FullScreenDisabled, w, QWebOSWindow::handleFullScreenDisabled)
        IPC_MESSAGE_FORWARD(View_Pause, w, QWebOSWindow::handlePause)
        IPC_MESSAGE_FORWARD(View_Resume, w, QWebOSWindow::handleResume)
        IPC_MESSAGE_FORWARD(View_InputEvent, w, QWebOSWindow::handleInputEvent)
        IPC_MESSAGE_FORWARD(View_TouchEvent, w, QWebOSWindow::handleTouchEvent)
        IPC_MESSAGE_FORWARD(View_KeyEvent, w, QWebOSWindow::handleKeyEvent)
    IPC_END_MESSAGE_MAP()
}

void QWebOSScreen::onDisconnected()
{
    qDebug()<< __PRETTY_FUNCTION__;
}


QWebOSWindow* QWebOSScreen::platformWindowFromId(WId id)
{
    qDebug()<< __PRETTY_FUNCTION__;
    return m_windowMap.value(id, 0);
}

void QWebOSScreen::addWindow(QWebOSWindow* window)
{
    qDebug()<< __PRETTY_FUNCTION__;
    m_windowMap[window->winId()] = window;
}

QT_END_NAMESPACE
