/**
 *  Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
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


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <QTimer>
#include <QDebug>
#include <stdint.h>
#include <unistd.h>

#include "hybriscompositorclient.h"

struct compositor_ctrl_hdr {
    uint32_t windowId;
    uint32_t command;
};

HybrisCompositorClient::HybrisCompositorClient(QObject *parent)
    : QObject(parent),
      m_socketPath("/tmp/sysmgr_compositor")
{
    QTimer::singleShot(0, this, SLOT(init()));
}

HybrisCompositorClient::~HybrisCompositorClient()
{
}

void HybrisCompositorClient::init()
{
    struct sockaddr_un socketAddr;

    qDebug() << __PRETTY_FUNCTION__ << "Connecting to buffer compositor ...";

    m_socketFd = ::socket(PF_LOCAL, SOCK_STREAM, 0);
    if (m_socketFd < 0) {
        g_critical("%s: %d Failed to create socket: %s",
                   __PRETTY_FUNCTION__, __LINE__, strerror(errno));
        return false;
    }

    memset(&socketAddr, 0, sizeof(socketAddr));
    socketAddr.sun_family = AF_LOCAL;
    strncpy(socketAddr.sun_path, m_socketPath.c_str(), G_N_ELEMENTS(socketAddr.sun_path));
    socketAddr.sun_path[G_N_ELEMENTS(socketAddr.sun_path)-1] = '\0';

    if (::connect(m_socketFd, (struct sockaddr*) &socketAddr,
                  SUN_LEN(&socketAddr)) != 0) {
        g_critical("%s:%d Failed to connect to socket: %s",
                   __PRETTY_FUNCTION__, __LINE__, strerror(errno));
        emit serverDisconnected();
        return;
    }

    qDebug() << __PRETTY_FUNCTION__ << "Successfully connected to buffer compositor";

    emit serverConnected();
}

void HybrisCompositorClient::postBuffer(int winId, OffscreenNativeWindowBuffer *buffer)
{
    struct compositor_ctrl_hdr hdr;
    int ret;

    qDebug() << __PRETTY_FUNCTION__ << "About to push buffer to compositor ...";

    memset(&hdr, 0, sizeof(struct compositor_ctrl_hdr));
    hdr.windowId = winId;
    hdr.command = 1; /* default */

    ret = write(m_socketFd, &hdr, sizeof(struct compositor_ctrl_hdr));

    buffer->writeToFd(m_socketFd);
}
