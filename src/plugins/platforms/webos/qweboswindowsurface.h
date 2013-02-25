
#ifndef QWebOSGLWindowSurface_H
#define QWebOSGLWindowSurface_H


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
    QWebOSGLWindowSurface(QWebOSScreen *screen, QWidget *window, GMainLoop* loop);
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
