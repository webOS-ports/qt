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

#include "qweboswindow.h"
#include "qwebosscreen.h"
#include "qwebosglcontext.h"

#include <QtGui/QWindowSystemInterface>
#include <QApplication>
#include <QSystemSemaphore>

#include <PIpcChannel.h>
#include <QElapsedTimer>
#include <QDebug>

#define MESSAGES_INTERNAL_FILE "SysMgrMessagesInternal.h"
#include <PIpcMessageMacros.h>

QT_BEGIN_NAMESPACE

// adreno egl only accepts 3 or two!
#define NUM_BUFFERS 3

#define Q_WEBOS_DEBUG 1

int QWebOSWindow::m_nextWindowId = 1;

QWebOSWindow::QWebOSWindow(QWidget *w, QWebOSScreen *screen)
    : QPlatformWindow(w),
      OffscreenNativeWindow(widget()->width(), widget()->height()),
      m_screen(screen),
      m_glcontext(0),
      m_bufferSemaphore(0)
{
    qDebug() << __PRETTY_FUNCTION__ << "****************"<<w<<screen;
}

void QWebOSWindow::setGeometry(const QRect& rect)
{
    qDebug() << __PRETTY_FUNCTION__ << "****************"<<rect;

    // FIXME we need to rebuild the geometry of your OffscreenNativeWindow and it's
    // buffers here!

    if(m_glcontext) {
        m_glcontext->rebuildSurface();
        // decongestion - we provided 3 new buffers after rebuilding the surface.
        // the old buffers should be evicted from the sysmgr cache and freed
        if(m_bufferSemaphore)
            m_bufferSemaphore->release(NUM_BUFFERS-1);
    }

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

void QWebOSWindow::handleFocus(bool focused)
{
#ifdef Q_WEBOS_DEBUG
    qDebug() << "QWebOSWindow::handleFocus " << focused;
#endif
    if (focused)
        QWindowSystemInterface::handleWindowActivated(widget());
    else
        QWindowSystemInterface::handleWindowActivated(0);
}

void QWebOSWindow::handleResize(int width, int height, bool resizeBuffer)
{
#ifdef Q_WEBOS_DEBUG
    qDebug() << "QWebOSWindow::handleResize - width: " << width << ", height: " << height << ", resizeBuffer: " << resizeBuffer;
#else
    Q_UNUSED(resizeBuffer)
#endif
    QRect rect(QPoint(), QSize(width, height));

    QPlatformWindow::setGeometry(rect);
    QWindowSystemInterface::handleGeometryChange(widget(), rect);
}

void QWebOSWindow::handleFullScreenEnabled()
{
#ifdef Q_WEBOS_DEBUG
    qDebug() << "QWebOSWindow::handleFullScreenEnabled";
#endif
}

void QWebOSWindow::handleFullScreenDisabled()
{
#ifdef Q_WEBOS_DEBUG
    qDebug() << "QWebOSWindow::handleFullScreenDisabled";
#endif
}

void QWebOSWindow::handlePause()
{
#ifdef Q_WEBOS_DEBUG
    qDebug() << "QWebOSWindow::handlePause";
#endif
}

void QWebOSWindow::handleResume()
{
#ifdef Q_WEBOS_DEBUG
    qDebug() << "QWebOSWindow::handleResume";
#endif
}

void QWebOSWindow::handleInputEvent(const SysMgrEventWrapper& wrapper)
{
#ifdef Q_WEBOS_DEBUG
    qDebug() << "QWebOSWindow::handleInputEvent";
#endif
    SysMgrEvent* e = wrapper.event;
    QPoint mousePos;
    switch(e->type)
    {
        case SysMgrEvent::Accelerometer:
            break;
        case SysMgrEvent::PenFlick:
            mousePos = QPoint(e->x, e->y);
            QWindowSystemInterface::handleWheelEvent(widget(),mousePos,mousePos,e->z,Qt::Vertical);
            break;
        case SysMgrEvent::PenPressAndHold:
            mousePos = QPoint(e->x, e->y);
            QWindowSystemInterface::handleMouseEvent(widget(), mousePos, mousePos, Qt::NoButton);
            break;
        case SysMgrEvent::PenDown:
            mousePos = QPoint(e->x, e->y);
            QWindowSystemInterface::handleMouseEvent(widget(), mousePos, mousePos, Qt::LeftButton);
            break;
        case SysMgrEvent::PenUp:
            mousePos = QPoint(e->x, e->y);
            QWindowSystemInterface::handleMouseEvent(widget(), mousePos, mousePos, Qt::NoButton);
            break;
        case SysMgrEvent::PenMove:
            mousePos = QPoint(e->x, e->y);
            QWindowSystemInterface::handleMouseEvent(widget(), mousePos, mousePos, Qt::LeftButton);
            break;
        case SysMgrEvent::GestureStart:
            break;
        case SysMgrEvent::GestureEnd:
            break;
        case SysMgrEvent::GestureCancel:
            break;
        default:
            break;
    }
}

void QWebOSWindow::handleTouchEvent(const SysMgrTouchEvent& touchEvent)
{
#ifdef Q_WEBOS_DEBUG
    qDebug() << "QWebOSWindow::handleTouchEvent";
#endif
    QEvent::Type type = QEvent::None;

    QList<QWindowSystemInterface::TouchPoint> touchPoints;
    for (unsigned int i = 0; i < touchEvent.numTouchPoints; i++) {
        QWindowSystemInterface::TouchPoint touchPoint;
        QPoint pt(touchEvent.touchPoints[i].x, touchEvent.touchPoints[i].y);

        // get size of screen which contains window
        QPlatformScreen *platformScreen = QPlatformScreen::platformScreenForWidget(widget());
        QSizeF screenSize = platformScreen->physicalSize();

        touchPoint.id = touchEvent.touchPoints[i].id;

        // update cached position of current touch point
        touchPoint.normalPosition = QPointF( static_cast<qreal>(pt.x()) / screenSize.width(), static_cast<qreal>(pt.y()) / screenSize.height() );
        touchPoint.area = QRectF( pt.x(), pt.y(), 0.0, 0.0 );
        touchPoint.pressure = 1;

        touchPoint.state = static_cast<Qt::TouchPointState>(touchEvent.touchPoints[i].state);

        // FIXME: what if the touchpoints have different states? does this ever happen?
        switch (touchPoint.state) {
            case Qt::TouchPointPressed:
                type = QEvent::TouchBegin;
                break;
            case Qt::TouchPointMoved:
                type = QEvent::TouchUpdate;
                break;
            case Qt::TouchPointReleased:
                type = QEvent::TouchEnd;
                break;
        }

        touchPoints.append(touchPoint);

        QWindowSystemInterface::handleMouseEvent(widget(), pt, pt, (touchPoint.state != Qt::TouchPointReleased ? Qt::LeftButton : Qt::NoButton));
    }

    // determine event type and update state of current touch point

    QWindowSystemInterface::handleTouchEvent(widget(), type, QTouchEvent::TouchScreen, touchPoints);
//    QWindowSystemInterface::handleTouchEvent(widget(), m_client->touchDevice(), touchPoints);
}

void QWebOSWindow::handleKeyEvent(const SysMgrKeyEvent& keyEvent)
{
#ifdef Q_WEBOS_DEBUG
    qDebug() << "QWebOSWindow::handleKeyEvent";
#endif
    QKeyEvent ev = keyEvent.qtEvent();
    Qt::Key key;
    if (ev.key() == 0x01200001) key = Qt::Key_Backspace;
    else key = (Qt::Key)ev.key();
    QWindowSystemInterface::handleKeyEvent(widget(), ev.type(), key, ev.modifiers(), ev.text(), 0, 0);
}

PIpcChannel* QWebOSWindow::channel() const
{
    return m_screen->channel();
}

void QWebOSWindow::setVisible(bool visible)
{
    qDebug() << "QWebOSWindow::setVisible - " << visible;

    if(!channel()) {
        qWarning() << "not connected to window manager...";
        return;
    }

    if (visible) {
            QSize size = geometry().size();

            int cardType = (1 << 3);
            channel()->sendAsyncMessage(new ViewHost_PrepareAddWindow(cardType, size.width(), size.height(), &m_winid));

            m_bufferSemaphore = new QSystemSemaphore(QString("EGLWindow%1").arg(m_winid),
                                                     NUM_BUFFERS - 1, QSystemSemaphore::Create);
            m_screen->addWindow(this);

            // no json in Qt 4.8... this will do for this simple stuff for now
            // mostly constants anyway
            QString winProps(
                    "{"
                    "fullScreen:%0, "
                    "overlayNotificationsPosition:\"bottom\", "
                    "subtleLightBar:true, "
                    "blockScreenTimeout:true, "
                    "fastAccelerometer:true, "
                    "suppressBannerMessages:false "
                    "hasPauseUi:false "
                    "}");
            winProps = winProps.arg((widget()->windowState() & Qt::WindowFullScreen) != 0);
            channel()->sendAsyncMessage(new ViewHost_SetWindowProperties(winId(), winProps.toStdString()));
            channel()->sendAsyncMessage(new ViewHost_AddWindow(winId()));
            channel()->sendAsyncMessage(new ViewHost_FocusWindow(winId()));

            QPlatformWindow::setGeometry(QRect(QPoint(), size));
    }

    QPlatformWindow::setVisible(visible);
}

void QWebOSWindow::postBuffer(OffscreenNativeWindowBuffer *buffer)
{
    qDebug() << __PRETTY_FUNCTION__;
}

#if 0
// pure virtual overloads from NWindow
void QWebOSWindow::Wait(int* buffer)
{
    // we rely on the buffers being painted, consumed and acquired in order, so
    // we already know the next buffer
    qDebug()<<"trying to acquire buffer"<<*buffer<<GetBuffer(*buffer)->Handle();
    QElapsedTimer t0;
    t0.start();
    m_bufferSemaphore->acquire();
    qDebug()<<"waited for buffer msecs:"<<t0.elapsed();
//    GetBuffer(*buffer)->Lock();
    qDebug()<<__PRETTY_FUNCTION__<<(buffer ? *buffer : 0);
}

void QWebOSWindow::Post(int buffer)
{
    qDebug()<<__PRETTY_FUNCTION__<<buffer;
    NPixmap* buf = GetBuffer(buffer);
    qDebug()<<"Post Async:"
                <<"win"<<winId()
                <<"handle"<<buf->Handle()
                <<"width"<<buf->Width()
                <<"height"<<buf->Height()
                <<"pitch"<<buf->Pitch();
    channel()->sendAsyncMessage(new ViewHost_PostBuffer(
                winId()
                buf->Handle(),
                buf->Width(),
                buf->Height(),
                buf->Pitch()
                ));
}
#endif

QT_END_NAMESPACE
