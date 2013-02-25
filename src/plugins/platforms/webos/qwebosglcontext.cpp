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
#include <QDebug>
#include "qwebosglcontext.h"
#include "../eglconvenience/qeglconvenience.h"
#include "../eglconvenience/qeglplatformcontext.h"

#ifdef __cplusplus
extern "C" {
#endif
    EGLDisplay* disp = NULL;
    void setEglSwapInterval(int interval) {
        eglSwapInterval(disp, interval);
    }
#ifdef __cplusplus
}
#endif

EGLDisplay QWebOSGLContext::s_eglDisplay = 0;

static void QWebOSGLContext::initialize(EGLNativeDisplayType display)
{
    qDebug() << __PRETTY_FUNCTION__;
    EGLint major, minor;

   if (!eglBindAPI(EGL_OPENGL_ES_API)) {
        qWarning("Could not bind GL_ES API\n");
        qFatal("EGL error");
    }

    s_eglDisplay = eglGetDisplay(display);
    if (s_eglDisplay == EGL_NO_DISPLAY)
        qDebug() << "Could not open egl display\n";

    qDebug() << "Opened display" << s_eglDisplay;

    if (!eglInitialize(s_eglDisplay, &major, &minor))
        qDebug() << "Could not initialize egl display";

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

    if (!qgetenv("QT_QPA_EGLFS_MULTISAMPLE").isEmpty())
        m_windowFormat.setSampleBuffers(true);

    EGLBoolean eglResult = eglBindAPI(EGL_OPENGL_ES_API);
    if (eglResult != EGL_TRUE)
        qFatal("QWebOS: failed to set EGL API, err=%d", eglGetError());

    m_eglConfig = q_configFromQPlatformWindowFormat(s_eglDisplay, m_windowFormat);

    EGLint contextAttrs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    m_eglContext = eglCreateContext(s_eglDisplay, m_eglConfig, EGL_NO_CONTEXT, contextAttrs);
    if (m_eglContext == EGL_NO_CONTEXT)
        qFatal("QWebOS: failed to create EGL context, err=%d", eglGetError());

    rebuildSurface();
}

QWebOSGLContext::~QWebOSGLContext()
{
   qDebug() << __PRETTY_FUNCTION__;

   if (m_eglContext != EGL_NO_CONTEXT)
        eglDestroyContext(s_eglDisplay, m_eglContext);

   destroySurface();
}

void QWebOSGLContext::rebuildSurface()
{
   qDebug() << __PRETTY_FUNCTION__;

    if (m_eglSurface != EGL_NO_SURFACE)
        eglDestroySurface(s_eglDisplay, m_eglSurface);

    m_eglSurface = eglCreateWindowSurface(s_eglDisplay, m_eglConfig,
                    static_cast<ANativeWindow*>(m_platformWindow),
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

    EGLint w, h;
    eglQuerySurface(s_eglDisplay, m_eglSurface, EGL_WIDTH, &w);
    eglQuerySurface(s_eglDisplay, m_eglSurface, EGL_HEIGHT, &h);
    m_surfaceSize = QSize(w,h);
    qDebug() <<"surface size" << w << h;
}

void QWebOSGLContext::destroySurface()
{
   qDebug() << __PRETTY_FUNCTION__;

    if (m_eglSurface != EGL_NO_SURFACE) {
        EGLBoolean eglResult = eglDestroySurface(s_eglDisplay, m_eglSurface);
        if (eglResult != EGL_TRUE)
            qFatal("QWebOS: failed to destroy EGL surface, err=%d", eglGetError());
    }
}


void QWebOSGLContext::swapBuffers()
{
    qDebug() << __PRETTY_FUNCTION__;

    EGLBoolean eglResult = eglBindAPI(EGL_OPENGL_ES_API);
    if (eglResult != EGL_TRUE)
        qFatal("QWebOS: failed to set EGL API, err=%d", eglGetError());

    eglResult = eglSwapBuffers(s_eglDisplay, m_eglSurface);
    if (eglResult != EGL_TRUE)
        qFatal("QWebOS: failed to swap EGL buffers, err=%d", eglGetError());
}

void QWebOSGLContext::makeCurrent()
{
    EGLBoolean eglResult = eglBindAPI(EGL_OPENGL_ES_API);
    if (eglResult != EGL_TRUE)
        qFatal("QWebOS: failed to set EGL API, err=%d", eglGetError());

    QPlatformGLContext::makeCurrent();

    eglResult = eglMakeCurrent(s_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext);

    if (eglResult != EGL_TRUE)
        qFatal("QWebOS: failed to set current EGL context, err=%X", eglGetError());
}

void QWebOSGLContext::doneCurrent()
{
    QPlatformGLContext::doneCurrent();

    EGLBoolean eglResult = eglBindAPI(EGL_OPENGL_ES_API);
    if (eglResult != EGL_TRUE)
        qFatal("QWebOS: failed to set EGL API, err=%d", eglGetError());

    eglResult = eglMakeCurrent(s_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (eglResult != EGL_TRUE)
        qFatal("QWebOS: failed to clear current EGL context, err=%d", eglGetError());
}

void* QWebOSGLContext::getProcAddress(const QString& procName)
{
    EGLBoolean eglResult = eglBindAPI(EGL_OPENGL_ES_API);
    if (eglResult != EGL_TRUE)
        qFatal("QWebOS: failed to set EGL API, err=%d", eglGetError());

    return (void *)eglGetProcAddress( procName.toAscii().constData() );
}

