#include "qweboswindow.h"
#include <QDebug>
#include "qwebosglcontext.h"
#include "../eglconvenience/qeglconvenience.h"
#include "../eglconvenience/qeglplatformcontext.h"

#ifdef __cplusplus
extern "C" {
#endif
    EGLDisplay* disp = NULL;
    void setEglSwapInterval(int interval) {
        //printf("Turning vsync %s\n", interval ? "on" : "off");
        eglSwapInterval(disp, interval);
    }
#ifdef __cplusplus
}
#endif

EGLDisplay QWebOSGLContext::s_eglDisplay = 0;
static void QWebOSGLContext::initialize(EGLNativeDisplayType display)
{
    qDebug()<<__PRETTY_FUNCTION__;
    EGLint major, minor;

   if (!eglBindAPI(EGL_OPENGL_ES_API)) {
        qWarning("Could not bind GL_ES API\n");
        qFatal("EGL error");
    }

    s_eglDisplay = eglGetDisplay(display);
    if (s_eglDisplay == EGL_NO_DISPLAY) {
        qDebug() << "Could not open egl display\n";
    }
    qDebug() << "Opened display" << s_eglDisplay;

    if (!eglInitialize(s_eglDisplay, &major, &minor)) {
        qDebug() << "Could not initialize egl display";
    }

    qDebug("Initialized display %d %d\n", major, minor);

    int swapInterval = 1;
    QByteArray swapIntervalString = qgetenv("QT_QPA_EGLFS_SWAPINTERVAL");
    if (!swapIntervalString.isEmpty()) {
        bool ok;
        swapInterval = swapIntervalString.toInt(&ok);
        if (!ok)
            swapInterval = 1;
    }

    eglSwapInterval(s_eglDisplay, swapInterval);
}


QWebOSGLContext::QWebOSGLContext(QWebOSWindow* platformWindow)
        : m_platformWindow(platformWindow)
        , m_eglSurface(EGL_NO_SURFACE)
{
    qDebug() << __PRETTY_FUNCTION__;

    m_windowFormat = QPlatformWindowFormat::defaultFormat();

    m_windowFormat.setWindowApi(QPlatformWindowFormat::OpenGL);

    QByteArray depthString = qgetenv("QT_QPA_EGLFS_DEPTH");
    if (depthString.toInt() == 16) {
        m_windowFormat.setDepth(16);
        m_windowFormat.setRedBufferSize(5);
        m_windowFormat.setGreenBufferSize(6);
        m_windowFormat.setBlueBufferSize(5);
    }
    else {
        m_windowFormat.setDepth(32);
        m_windowFormat.setRedBufferSize(8);
        m_windowFormat.setGreenBufferSize(8);
        m_windowFormat.setBlueBufferSize(8);
    }

    if (!qgetenv("QT_QPA_EGLFS_MULTISAMPLE").isEmpty()) {
        m_windowFormat.setSampleBuffers(true);
    }

    EGLBoolean eglResult = eglBindAPI(EGL_OPENGL_ES_API);
    if (eglResult != EGL_TRUE) {
        qFatal("QBB: failed to set EGL API, err=%d", eglGetError());
    }

    m_eglConfig = q_configFromQPlatformWindowFormat(s_eglDisplay, m_windowFormat);

    EGLint contextAttrs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    m_eglContext = eglCreateContext(s_eglDisplay, m_eglConfig, EGL_NO_CONTEXT, contextAttrs);
    if (m_eglContext == EGL_NO_CONTEXT) {
        qFatal("QBB: failed to create EGL context, err=%d", eglGetError());
    }

    rebuildSurface();
}

void QWebOSGLContext::rebuildSurface()
{
    if(m_eglSurface != EGL_NO_SURFACE)
        eglDestroySurface(s_eglDisplay, m_eglSurface);

    m_eglSurface = eglCreateWindowSurface(s_eglDisplay, m_eglConfig,
                    static_cast<OffscreenNativeWindow*>(m_platformWindow),
                    NULL);

    if (m_eglSurface == EGL_NO_SURFACE) {
        qDebug("Could not create the egl surface: error = 0x%x\n", eglGetError());
        eglTerminate(s_eglDisplay);
        qFatal("EGL error");
    }

    qDebug() << "egl surface created: " << m_eglSurface;

    EGLint temp;
    EGLint attribList[] = {
        EGL_CONTEXT_CLIENT_VERSION,
        2, // GLES version 2
        EGL_NONE
    };

    // this is queried through the NWindow interface of the platformWindow!
    EGLint w, h;
    eglQuerySurface(s_eglDisplay, m_eglSurface, EGL_WIDTH, &w);
    eglQuerySurface(s_eglDisplay, m_eglSurface, EGL_HEIGHT, &h);
    m_surfaceSize = QSize(w,h);
    qDebug() <<"surface size" << w << h;
}

QWebOSGLContext::~QWebOSGLContext()
{
    qDebug()<<__PRETTY_FUNCTION__;
}

void QWebOSGLContext::swapBuffers()
{
    qDebug()<<__PRETTY_FUNCTION__;

    // set current rendering API
    EGLBoolean eglResult = eglBindAPI(EGL_OPENGL_ES_API);
    if (eglResult != EGL_TRUE) {
        qFatal("QBB: failed to set EGL API, err=%d", eglGetError());
    }

    // post EGL surface to window
    eglResult = eglSwapBuffers(s_eglDisplay, m_eglSurface);
    if (eglResult != EGL_TRUE) {
        qFatal("QBB: failed to swap EGL buffers, err=%d", eglGetError());
    }

/* FIXME copied from qbb qpa - relevant?
 * // resize surface if window was resized
    QSize s = mPlatformWindow->geometry().size();
    if (s != mSurfaceSize) {
        resizeSurface(s);
    }
*/
}

void QWebOSGLContext::makeCurrent() 
{
/* FIXME copied from QBB qpa - relevant?
 * if (!mPlatformWindow->hasBuffers()) {
        // NOTE: create a dummy EGL surface since Qt will call makeCurrent() before
        // setting the geometry on the associated window
        mSurfaceSize = QSize(1, 1);
        mPlatformWindow->setBufferSize(mSurfaceSize);
        createSurface();
    }*/

    EGLBoolean eglResult = eglBindAPI(EGL_OPENGL_ES_API);
    if (eglResult != EGL_TRUE) {
        qFatal("QBB: failed to set EGL API, err=%d", eglGetError());
    }

    QPlatformGLContext::makeCurrent();

    eglResult = eglMakeCurrent(s_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext);

    if (eglResult != EGL_TRUE) {
        qFatal("QBB: failed to set current EGL context, err=%X", eglGetError());
    }
}

void QWebOSGLContext::doneCurrent() {
    // call the parent method
    QPlatformGLContext::doneCurrent();

    // set current rendering API
    EGLBoolean eglResult = eglBindAPI(EGL_OPENGL_ES_API);
    if (eglResult != EGL_TRUE) {
        qFatal("QBB: failed to set EGL API, err=%d", eglGetError());
    }

    // clear curent EGL context and unbind EGL surface
    eglResult = eglMakeCurrent(s_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (eglResult != EGL_TRUE) {
        qFatal("QBB: failed to clear current EGL context, err=%d", eglGetError());
    }
}

void* QWebOSGLContext::getProcAddress(const QString& procName)
{
    // set current rendering API
    EGLBoolean eglResult = eglBindAPI(EGL_OPENGL_ES_API);
    if (eglResult != EGL_TRUE) {
        qFatal("QBB: failed to set EGL API, err=%d", eglGetError());
    }

    // lookup EGL extension function pointer
    return (void *)eglGetProcAddress( procName.toAscii().constData() );
}

