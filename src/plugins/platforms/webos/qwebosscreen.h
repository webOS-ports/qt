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

#ifndef QWEBOSSCREEN_H
#define QWEBOSSCREEN_H

#include <QPlatformScreen>
#include <QtCore/QTextStream>
#include <QHash>


QT_BEGIN_NAMESPACE

class QPlatformGLContext;
class QEGLPlatformContext;
class QWebOSWindow;

class QWebOSScreen : public QPlatformScreen
{
public:
    QWebOSScreen();
    ~QWebOSScreen() {}

    QRect geometry() const;
    int depth() const;
    QImage::Format format() const;

    void directRendering(bool enable);
    bool isDirectRendering() { return m_direct; }

private:
    QRect m_geometry;
    int m_depth;
    QImage::Format m_format;
    bool m_direct;
};

QT_END_NAMESPACE
#endif // QWEBOSSCREEN_H
