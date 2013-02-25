#ifndef QWEBOSGLCONTEXT_H
#define QWEBOSGLCONTEXT_H

#include <QPlatformGLContext>
#include <EGL/egl.h>
class QWebOSWindow;

class QWebOSGLContext : public QPlatformGLContext
{
public:
    //FIXME make singleton/lazy init
    static void initialize(EGLNativeDisplayType display);
    QWebOSGLContext(QWebOSWindow* platformWindow);
    ~QWebOSGLContext();
    //overloads from QlatformGLContext
    virtual void makeCurrent();
    virtual void doneCurrent();
    virtual void swapBuffers();
    virtual void* getProcAddress(const QString& procName);

    virtual QPlatformWindowFormat platformWindowFormat() const { return m_windowFormat; }
    QSize surfaceSize() const {
        return m_surfaceSize;
    }
    void rebuildSurface();
private:
    static EGLDisplay s_eglDisplay;
    EGLContext m_eglContext;
    // FIXME should the surface be part of a proper surface class
    EGLSurface m_eglSurface;
    EGLConfig m_eglConfig;
    QSize m_surfaceSize;
    QWebOSWindow* m_platformWindow;
    QPlatformWindowFormat m_windowFormat;
};
#endif
