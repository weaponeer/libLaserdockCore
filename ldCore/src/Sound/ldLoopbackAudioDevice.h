//
//  ldLoopbackAudioDevice.h
//  ldCore
//
//  Created by Sergey Gavrushkin on 09/08/16.
//  Copyright (c) 2016 Wicked Lasers. All rights reserved.
//

#ifndef ldCore__ldLoopbackAudioDevice__
#define ldCore__ldLoopbackAudioDevice__

#include <QtCore/QThread>

#include "ldCore/Sound/ldSoundInterface.h"

class ldLoopbackAudioDeviceWorker;

class ldLoopbackAudioDevice : public ldSoundInterface
{
    Q_OBJECT
public:
    static QStringList getAvailableOutputDevices();

    explicit ldLoopbackAudioDevice(QString device, QObject *parent = 0);
    virtual ~ldLoopbackAudioDevice();

    // ldSoundInterface
    virtual QAudioFormat getAudioFormat() const override;

    QString device() const;

public slots:
    void startCapture();
    void stopCapture();

signals:
    void error(QString error);

private:
    QThread *m_worker_thread = NULL;
    ldLoopbackAudioDeviceWorker *m_worker = NULL;

    QString m_device;
};

#endif
