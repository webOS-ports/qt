#ifndef QWEBOSWINDOW_H
#define QWEBOSWINDOW_H


#include <QPlatformWindow>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

#include <EGL/eglhybris.h>

#include <SysMgrEvent.h>
#include <SysMgrDefs.h>
#include <SysMgrKeyEventTraits.h>
#include <SysMgrTouchEventTraits.h>

#include "qwebosintegration.h"
typedef WId QWebOSWindowId;

class QWebOSScreen;
class QWebOSGLContext;
struct PIpcChannel;
class QSystemSemaphore;

class QWebOSWindow : public QPlatformWindow,
                     public OffscreenNativeWindow
{
public:
    QWebOSWindow(QWidget *w, QWebOSScreen *screen);

    void setGeometry(const QRect &);
    WId winId() const;

    QPlatformGLContext *glContext() const;
    void setVisible(bool visible);

#if 0
    // pure virtual overloads from NWindow
    virtual void Post(int Buffer);
    virtual void Wait(int* Buffer);
#endif
    virtual void postBuffer(OffscreenNativeWindowBuffer *buffer);

    // called through IPC from QWebOSScreen
    void handleFocus(bool focused);
    void handleResize(int width, int height, bool resizeBuffer);
    void handleFullScreenEnabled();
    void handleFullScreenDisabled();
    void handlePause();
    void handleResume();
    void handleInputEvent(const SysMgrEventWrapper& wrapper);
    void handleTouchEvent(const SysMgrTouchEvent& touchEvent);
    void handleKeyEvent(const SysMgrKeyEvent& keyEvent);
    void handleBufferConsumed(int key);
    PIpcChannel* channel() const; // Required by IPC_MESSAGE_FORWARD

    void createGLContext();
private:
    QWebOSScreen *m_screen;
    QWebOSGLContext* m_glcontext;
    WId m_winid;
    // FIXME should this be a system (IPC) semaphore between
    // sysmgr and the client? would reduce ipc message load.
    QSystemSemaphore* m_bufferSemaphore;

    static int m_nextWindowId;

};
QT_END_NAMESPACE
#endif // QWEBOSWINDOW_H
