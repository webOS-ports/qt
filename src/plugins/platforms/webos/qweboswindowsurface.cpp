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
#include "qwebosglcontext.h"
#include "qweboswindowsurface.h"

#include <QtGui/QPlatformGLContext>

#include <QtOpenGL/private/qgl_p.h>
#include <QtOpenGL/private/qglpaintdevice_p.h>
#include <QWindowSystemInterface>
#include <QDebug>
//#include <palmimedefines.h>

QT_BEGIN_NAMESPACE

class QWebOSPaintDevice : public QGLPaintDevice
{
public:
    QWebOSPaintDevice(QWebOSGLContext* platformGLContext)
        : m_platformGLContext(platformGLContext)
    {
        qDebug()<<__PRETTY_FUNCTION__;
        m_context = QGLContext::fromPlatformGLContext(m_platformGLContext);
    }

    QSize size() const {
        qDebug()<<__PRETTY_FUNCTION__;
        return m_platformGLContext->surfaceSize();
    }

    QGLContext* context() const {
        qDebug()<<__PRETTY_FUNCTION__;
        return m_context;
    }

    QPaintEngine *paintEngine() const {
        qDebug()<<__PRETTY_FUNCTION__;
        return qt_qgl_paint_engine();
    }

private:
    QWebOSGLContext* m_platformGLContext;
    QGLContext *m_context;
};



QWebOSGLWindowSurface::QWebOSGLWindowSurface(QWebOSScreen *screen, QWidget *window, GMainLoop* loop)
    : QWindowSurface(window)
{
    qDebug()<<__PRETTY_FUNCTION__;

    m_platformGLContext = static_cast<QWebOSGLContext*>(window->platformWindow()->glContext());
    m_paintDevice = new QWebOSPaintDevice(m_platformGLContext);
    m_screen = screen;
}

void QWebOSGLWindowSurface::flush(QWidget *widget, const QRegion &region, const QPoint &offset)
{
    qDebug()<<__PRETTY_FUNCTION__;
    Q_UNUSED(region);
    Q_UNUSED(offset);

    // QGraphicsView contains a QWidget for its frame, even if it is not visible. Any repaint
    // on that frame widget will cause an extra buffer swap, causing problems. This filters out
    // that swap.
//    if(qobject_cast<QGraphicsView*>(widget))
//        return;
    widget->platformWindow()->glContext()->swapBuffers();
}


QWebOSGLWindowSurface::~QWebOSGLWindowSurface()
{
}

void QWebOSGLWindowSurface::resize(const QSize &size)
{
    qDebug()<<__PRETTY_FUNCTION__<<size;
    Q_UNUSED(size);
}
void QWebOSGLWindowSurface::beginPaint(const QRegion &region)
{
    qDebug()<<__PRETTY_FUNCTION__;
    Q_UNUSED(region);
}

void QWebOSGLWindowSurface::endPaint(const QRegion &region)
{
    qDebug()<<__PRETTY_FUNCTION__;
    Q_UNUSED(region);
}

#if 0

void QWebOSGLWindowSurface::slotSwapBuffers()
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
    if (m_channel && !(m_screen->isDirectRendering())) {
        m_channel->sendAsyncMessage(new ViewHost_UpdateFullWindow(routingId()));
    }
}

int QWebOSGLWindowSurface::routingId() const
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
    if (m_nrWindow)
        return m_nrWindow->GetBuffer(0)->Handle();
    return 0;
}

void QWebOSGLWindowSurface::onDisconnected()
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
    qWarning("%s", __PRETTY_FUNCTION__);
    exit(-1);
}

void QWebOSGLWindowSurface::serverConnected(PIpcChannel* channel)
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
    channel->setListener(this);

    m_nrWindow = m_screen->remoteWindow();

    setChannel(channel);

    QRect r = window()->platformWindow()->geometry();
    m_channel->sendAsyncMessage(new ViewHost_PrepareAddWindow(routingId(), 8, r.width(), r.height()));

    std::string winProps = "{ "
                           "  'fullScreen': false, "                     // defaults to false
                           "  'overlayNotificationsPosition': 'bottom', " // options are left, right, top, bottom
                           "  'subtleLightbar': true, "                 // defaults to false
                           "  'blockScreenTimeout': true, "             // defaults to false
                           "  'fastAccelerometer': true, "              // defaults to false
                           "  'suppressBannerMessages': false, "        // defaults to false
                           "  'hasPauseUi': true "                     // defaults to false
                           " }";
    m_channel->sendAsyncMessage(new ViewHost_SetWindowProperties(routingId(), winProps));

    m_channel->sendAsyncMessage(new ViewHost_AddWindow(routingId()));
}

void QWebOSGLWindowSurface::serverDisconnected()
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
    qWarning("%s", __PRETTY_FUNCTION__);
    exit(-1);
}


//Message handling
void QWebOSGLWindowSurface::onMessageReceived(const PIpcMessage& msg)
{
    bool msgIsOk;

    IPC_BEGIN_MESSAGE_MAP(QWebOSGLWindowSurface, msg, msgIsOk)
        IPC_MESSAGE_HANDLER(View_InputEvent, onInputEvent)
        IPC_MESSAGE_HANDLER(View_KeyEvent, onKeyEvent)
        IPC_MESSAGE_HANDLER(View_TouchEvent, onTouchEvent)
        IPC_MESSAGE_HANDLER(View_RemoveInputFocus, onRemoveInputFocus)
        IPC_MESSAGE_HANDLER(View_FullScreenEnabled, onFullScreenEnabled)
        IPC_MESSAGE_HANDLER(View_FullScreenDisabled, onFullScreenDisabled)
        IPC_MESSAGE_HANDLER(View_CommitText, onCommitText)
    IPC_END_MESSAGE_MAP()
}

void QWebOSGLWindowSurface::onCommitText(std::string s)
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
    QWidget* focus = QApplication::focusWidget();
    if(focus)
    {
        QString str(s.c_str());
        for(QString::iterator iter = str.begin(); iter != str.end(); iter++)
        {
            QWindowSystemInterface::handleKeyEvent(window(), QEvent::KeyPress, 0, 0, *iter);
            QWindowSystemInterface::handleKeyEvent(window(), QEvent::KeyRelease, 0, 0, *iter);
        }
    }
}

void QWebOSGLWindowSurface::onRemoveInputFocus()
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
    qApp->postEvent(window(), new QEvent(QEvent::CloseSoftwareInputPanel));
}

void QWebOSGLWindowSurface::onFullScreenEnabled()
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
    qWarning("Full screen enabled");
    m_screen->directRendering(true);
    window()->update();
}
void QWebOSGLWindowSurface::onFullScreenDisabled()
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
    qWarning("Full screen disabled");
    m_screen->directRendering(false);
    window()->update();
}

void QWebOSGLWindowSurface::onKeyEvent(const SysMgrKeyEvent& e)
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
    QKeyEvent key = e.qtEvent();
    QWindowSystemInterface::handleKeyEvent(window(), key.type(), key.key(), key.modifiers(), key.text());
}

void QWebOSGLWindowSurface::onTouchEvent(const SysMgrTouchEvent& te)
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
    QList<QWindowSystemInterface::TouchPoint> touchPoints;
    for (int i = 0; i < te.numTouchPoints; i++) {

        QWindowSystemInterface::TouchPoint touchPoint;
        QPoint pt(te.touchPoints[i].x, te.touchPoints[i].y);
        QRect rc = window()->platformWindow()->geometry();

        touchPoint.id = te.touchPoints[i].id;
        touchPoint.isPrimary = te.touchPoints[i].isPrimary;
        touchPoint.normalPosition = QPointF((qreal)pt.x() / (qreal)rc.width(), (qreal)pt.y() / (qreal)rc.height());
        touchPoint.area = QRectF(pt, QSize(1,1));
        touchPoint.pressure = 1;
        touchPoint.state = static_cast<Qt::TouchPointState>(te.touchPoints[i].state);

        if (touchPoint.isPrimary) {

            QWindowSystemInterface::handleMouseEvent(window(), pt, pt, (touchPoint.state != Qt::TouchPointReleased ? Qt::LeftButton : Qt::NoButton));
        }

        touchPoints.append(touchPoint);
    }

    QWindowSystemInterface::handleTouchEvent(window(), static_cast<QEvent::Type>(te.type), QTouchEvent::TouchScreen, touchPoints);
}

void QWebOSGLWindowSurface::onInputEvent(const SysMgrEventWrapper& wrapper) 
{
    qDebug() << "\t\t\t\t\**************"<< __PRETTY_FUNCTION__ << "****************";
    SysMgrEvent* e = wrapper.event;
    QPoint mousePos;
    switch(e->type) 
    {
        case SysMgrEvent::Accelerometer:
            m_trans.reset();
            switch(e->orientation) 
            {
                case SysMgrEvent::Orientation_Up:
                    m_trans.rotate(0);
                    break;
                case SysMgrEvent::Orientation_Down:
                    m_trans.rotate(180);
                    break;
                case SysMgrEvent::Orientation_Left:
                    m_trans.rotate(90);
                    break;
                case SysMgrEvent::Orientation_Right:
                    m_trans.rotate(270);
                    break;
                default:
                    //qWarning("Unhandled orientation");
                    break;
            }
            break;
        case SysMgrEvent::PenDown:
#ifdef DEBUG_INPUT
            printf("Pen Down\n");
#endif
            mousePos = QPoint(e->x, e->y);
            QWindowSystemInterface::handleMouseEvent(window(), mousePos, mousePos, Qt::LeftButton);
            break;
        case SysMgrEvent::PenUp:
#ifdef DEBUG_INPUT
            printf("Pen Up\n");
#endif
            mousePos = QPoint(e->x, e->y);
            QWindowSystemInterface::handleMouseEvent(window(), mousePos, mousePos, Qt::NoButton);
            break;
        case SysMgrEvent::PenMove:
#ifdef DEBUG_INPUT
            printf("Pen Move\n");
#endif
            mousePos = QPoint(e->x, e->y);
            QWindowSystemInterface::handleMouseEvent(window(), mousePos, mousePos, Qt::LeftButton);
            break;
        case SysMgrEvent::GestureStart:
#ifdef DEBUG_INPUT
            printf("Gesture start\n");
#endif
            break;
        case SysMgrEvent::GestureEnd:
#ifdef DEBUG_INPUT
            printf("Gesture end\n");
#endif
            break;
        case SysMgrEvent::GestureCancel:
#ifdef DEBUG_INPUT
            printf("Gesture cancel\n");
#endif
            break;
        default:
            printf("Unhandled event type\n");
            break;
    }
}
#endif
QT_END_NAMESPACE
