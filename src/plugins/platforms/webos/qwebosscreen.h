
#ifndef QWEBOSSCREEN_H
#define QWEBOSSCREEN_H

#include <QPlatformScreen>

#include <QtCore/QTextStream>
#include <QHash>

#include <PIpcClient.h>
#include <PIpcChannelListener.h>


QT_BEGIN_NAMESPACE

class QPlatformGLContext;
class QEGLPlatformContext;
class QWebOSWindow;
class QEventLoop;

/*
 * I chose to be the platform screen to be our IPC endpoint from sysmgr
 * as it should only exist once per application and seems to fit the interface best
 */
class QWebOSScreen : public QPlatformScreen
                   , public PIpcClient
                   , public PIpcChannelListener
{
public:
    QWebOSScreen(const std::string& ipcServer,
                 const std::string& name,
                 GMainLoop* loop);
    ~QWebOSScreen() {}

    QRect geometry() const;
    int depth() const;
    QImage::Format format() const;

    void directRendering(bool enable);
    bool isDirectRendering() { return m_direct; }

    QWebOSWindow* platformWindowFromId(WId id);
    void addWindow(QWebOSWindow* window);

private:
    // implementing PIpcClient interface pure virtual functions
    virtual void serverConnected(PIpcChannel* channel);
    virtual void serverDisconnected();

    // implementing PIpcChannelListener interface pure virtual functions
    virtual void onMessageReceived(const PIpcMessage& msg);
    virtual void onDisconnected();

private:
    QHash<WId, QWebOSWindow*> m_windowMap;

    QRect m_geometry;
    int m_depth;
    QImage::Format m_format;
    bool m_direct;
};

QT_END_NAMESPACE
#endif // QWEBOSSCREEN_H
