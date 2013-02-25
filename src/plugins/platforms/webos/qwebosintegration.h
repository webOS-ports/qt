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

#ifndef WEBOSINTEGRATION_H
#define WEBOSINTEGRATION_H


#include <QtGui/QPlatformIntegration>
#include <QtGui/QPlatformScreen>

#include <glib.h>

#include "hybriscompositorclient.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QWebOSClipboard;
class QWebOSFontDatabase;
class QWebOSScreen;
class QWebOSNativeInterface;

class QWebOSIntegration : public QPlatformIntegration
{
public:
    QWebOSIntegration();
    bool hasCapability(QPlatformIntegration::Capability cap) const;
    QPixmapData *createPixmapData(QPixmapData::PixelType type) const;
    QPlatformWindow *createPlatformWindow(QWidget *widget, WId winId) const;
    QWindowSurface *createWindowSurface(QWidget *widget, WId winId) const;

    QList<QPlatformScreen *> screens() const { return m_screens; }

    virtual QPlatformNativeInterface* nativeInterface() const;

    QPlatformFontDatabase *fontDatabase() const;
    virtual QPlatformClipboard *clipboard() const;
private:
    QWebOSFontDatabase *m_fontDb;
    QList<QPlatformScreen *> m_screens;
    QWebOSScreen *m_primaryScreen;
    QWebOSClipboard *m_clipboard;
    QWebOSNativeInterface *m_nativeInterface;
    HybrisCompositorClient m_compositorClient;
};

QT_END_NAMESPACE
QT_END_HEADER

#endif
