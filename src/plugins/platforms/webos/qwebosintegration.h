
#ifndef WEBOSINTEGRATION_H
#define WEBOSINTEGRATION_H


#include <QtGui/QPlatformIntegration>
#include <QtGui/QPlatformScreen>

#include <glib.h>
QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QWebOSClipboard;
class QWebOSFontDatabase;
class QWebOSScreen;

class QWebOSIntegration : public QPlatformIntegration
{
public:
    QWebOSIntegration();
    bool hasCapability(QPlatformIntegration::Capability cap) const;
    QPixmapData *createPixmapData(QPixmapData::PixelType type) const;
    QPlatformWindow *createPlatformWindow(QWidget *widget, WId winId) const;
    QWindowSurface *createWindowSurface(QWidget *widget, WId winId) const;

    QList<QPlatformScreen *> screens() const { return mScreens; }

    QPlatformFontDatabase *fontDatabase() const;
    virtual QPlatformClipboard *clipboard() const;
private:
    QWebOSFontDatabase *mFontDb;
    QList<QPlatformScreen *> mScreens;
    QWebOSScreen *m_primaryScreen;
    bool m_offscreen;
    QWebOSClipboard* m_clipboard;
    //FIXME right place for those? or put inside screen?
    GMainContext* m_gContext;
    GMainLoop* m_gMainLoop;
};

QT_END_NAMESPACE
QT_END_HEADER

#endif
