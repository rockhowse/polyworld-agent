/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>
#include <QtNetwork>

#include "MulticastReceiver.h"

MulticastReceiver::MulticastReceiver(QWidget *parent)
    : QDialog(parent)
{
    groupAddress = QHostAddress("239.255.43.21");

    statusLabel = new QLabel(tr("Listening for multicasted messages"));
    quitButton = new QPushButton(tr("&Quit"));

    udpSocket = new QUdpSocket(this);
    udpSocket->bind(QHostAddress::AnyIPv4, 45454, QUdpSocket::ShareAddress);
    udpSocket->joinMulticastGroup(groupAddress);

    connect(udpSocket, SIGNAL(readyRead()),
            this, SLOT(processPendingDatagrams()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(quitButton);
    buttonLayout->addStretch(1);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(statusLabel);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    setWindowTitle(tr("Multicast Receiver"));
}

void MulticastReceiver::processPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {

        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size());

        QDataStream in(&datagram, QIODevice::ReadOnly);
        in.setVersion(QDataStream::Qt_4_3);

        int messageType = -1;

        in >> messageType;

        switch(messageType) {
            case MSG_TYPE_STEP:
            {
                struct SimStepHeader {
                    int simStep;
                    int agentCount;
                    float sceneRotation;
                };

                struct SimAgentData {
                    long  agentNum;
                    float agentX;
                    float agentY;
                    float agentZ;
                    float agentYaw;
                };

                SimStepHeader *ssh = new SimStepHeader();

                in >> ssh->simStep
                   >> ssh->agentCount
                   >> ssh->sceneRotation;

                SimAgentData *sdp = new SimAgentData();
                qint64 agentNum;

                int numAgentSent = 0;

                // whiel we
                while(numAgentSent < ssh->agentCount) {

                    in >> agentNum
                       >> sdp->agentX
                       >> sdp->agentY
                       >> sdp->agentZ
                       >> sdp->agentYaw;

                    sdp->agentNum = (long) agentNum;

                    emit setStatus(tr("MSG_TYPE_STEP:").arg(messageType) +
                                   tr("[%1]").arg(ssh->simStep) +
                                   tr("{%1,").arg(ssh->agentCount) +
                                   tr("%1}").arg(numAgentSent) +
                                   tr("a#[%1]").arg(sdp->agentNum) +
                                   tr("(%1,").arg(sdp->agentX)  +
                                   tr("%1,").arg(sdp->agentY) +
                                   tr("%1,)").arg(sdp->agentZ) +
                                   tr("(%1)").arg(sdp->agentYaw));
                    numAgentSent++;

                    emit moveAgent(sdp->agentNum, sdp->agentX, sdp->agentY, sdp->agentZ, sdp->agentYaw);
                }

                // let the app know what server step, number of agents and rotation is for server
                emit serverStep(ssh->simStep, ssh->agentCount, ssh->sceneRotation);

                emit setStatus("\n");

                delete(sdp);
                delete(ssh);
                break;
            }
            case MSG_TYPE_AGENT_BIRTH:
            {
                struct AgentBirthPacket {
                    long    agentNum;
                    float   agentHeight;
                    float   agentSize;
                };

                AgentBirthPacket *abp = new AgentBirthPacket();
                qint64 agentNum;

                in >> agentNum
                   >> abp->agentHeight
                   >> abp->agentSize;

                abp->agentNum = (long)agentNum;

                emit setStatus(tr("MSG_TYPE_AGENT_BIRTH:").arg(messageType) +
                               tr("[%1]").arg(abp->agentNum) +
                               tr("(%1,").arg(abp->agentHeight) +
                               tr("%1)").arg(abp->agentSize));

                emit agentBorn(abp->agentNum, abp->agentHeight, abp->agentSize);

                delete(abp);
                break;
            }
            case MSG_TYPE_AGENT_DEATH:
            {
                struct AgentDeathPacket {
                    long    agentNum;
                };

                AgentDeathPacket *abp = new AgentDeathPacket();
                qint64 agentNum;

                in >> agentNum;

                abp->agentNum = (long)agentNum;

                emit setStatus(tr("MSG_TYPE_AGENT_DEATH:").arg(messageType) +
                               tr("[%1]").arg(abp->agentNum));

                emit agentDied(abp->agentNum);

                delete(abp);
                break;
            }
        }
    }
}
