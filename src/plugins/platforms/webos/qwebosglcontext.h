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

#ifndef QWEBOSGLCONTEXT_H
#define QWEBOSGLCONTEXT_H

#include <QPlatformGLContext>
#include <EGL/egl.h>
class QWebOSWindow;

class QWebOSGLContext : public QPlatformGLContext
{
public:
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

    static EGLDisplay display() { return s_eglDisplay; }

private:
    static EGLDisplay s_eglDisplay;
    EGLContext m_eglContext;
    // FIXME should the surface be part of a proper surface class
    EGLSurface m_eglSurface;
    EGLConfig m_eglConfig;
    QSize m_surfaceSize;
    QWebOSWindow* m_platformWindow;
    QPlatformWindowFormat m_windowFormat;

    void rebuildSurface();
    void destroySurface();
};
#endif
