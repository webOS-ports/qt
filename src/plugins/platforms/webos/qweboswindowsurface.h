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

#ifndef QWEBOSGLWINDOWSURFACE_H
#define QWEBOSGLWINDOWSURFACE_H


#include <QtGui/private/qwindowsurface_p.h>
#include "glib.h"

QT_BEGIN_NAMESPACE

class QWebOSWindow;
class QWebOSGLContext;
class QWebOSScreen;
class QWebOSGLWindowSurface : public QObject,
                            public QWindowSurface
{
    Q_OBJECT

public:
    QWebOSGLWindowSurface(QWebOSScreen *screen, QWidget *window);
    virtual ~QWebOSGLWindowSurface();

    QPaintDevice *paintDevice() { return m_paintDevice; }
    void flush(QWidget *widget, const QRegion &region, const QPoint &offset);
    void resize(const QSize &size);
    virtual void beginPaint(const QRegion &region);
    virtual void endPaint(const QRegion &region);

private:

    QPaintDevice *m_paintDevice;
    QWebOSScreen* m_screen;
    QTransform m_trans;
    QWebOSGLContext* m_platformGLContext;
};

QT_END_NAMESPACE

#endif // QWebOSGLWindowSurface_H
