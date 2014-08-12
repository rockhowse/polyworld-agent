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
                struct SimStepHeaderMsg {
                    int simStep;
                    int agentCount;
                    float sceneRotation;
                };

                struct SimAgentStepMsg {
                    long  agentNum;
                    float agentX;
                    float agentY;
                    float agentZ;
                    float agentYaw;
                };

                SimStepHeaderMsg *sshm = new SimStepHeaderMsg();

                in >> sshm->simStep
                   >> sshm->agentCount
                   >> sshm->sceneRotation;

                SimAgentStepMsg *sasm = new SimAgentStepMsg();
                qint64 agentNum;

                int numAgentSent = 0;

                // whiel we
                while(numAgentSent < sshm->agentCount) {

                    in >> agentNum
                       >> sasm->agentX
                       >> sasm->agentY
                       >> sasm->agentZ
                       >> sasm->agentYaw;

                    sasm->agentNum = (long) agentNum;

                    emit setStatus(tr("MSG_TYPE_STEP:").arg(messageType) +
                                   tr("[%1]").arg(sshm->simStep) +
                                   tr("/%1/").arg(sshm->sceneRotation) +
                                   tr("{%1,").arg(sshm->agentCount) +
                                   tr("%1}").arg(numAgentSent) +
                                   tr("a#[%1]").arg(sasm->agentNum) +
                                   tr("(%1,").arg(sasm->agentX)  +
                                   tr("%1,").arg(sasm->agentY) +
                                   tr("%1,)").arg(sasm->agentZ) +
                                   tr("(%1)").arg(sasm->agentYaw));
                    numAgentSent++;

                    emit moveAgent(sasm->agentNum, sasm->agentX, sasm->agentY, sasm->agentZ, sasm->agentYaw);
                }

                // let the app know what server step, number of agents and rotation is for server
                emit serverStep(sshm->simStep, sshm->agentCount, sshm->sceneRotation);

                emit setStatus("\n");

                delete(sasm);
                delete(sshm);
                break;
            }
            case MSG_TYPE_AGENT_BIRTH:
            {
                struct AgentBirthMsg {
                    long    agentNum;
                    float   agentHeight;
                    float   agentSize;
                };

                AgentBirthMsg *abm = new AgentBirthMsg();
                qint64 agentNum;

                in >> agentNum
                   >> abm->agentHeight
                   >> abm->agentSize;

                abm->agentNum = (long)agentNum;

                emit setStatus(tr("MSG_TYPE_AGENT_BIRTH:").arg(messageType) +
                               tr("[%1]").arg(abm->agentNum) +
                               tr("(%1,").arg(abm->agentHeight) +
                               tr("%1)").arg(abm->agentSize));

                emit agentBorn(abm->agentNum, abm->agentHeight, abm->agentSize);

                delete(abm);
                break;
            }
            case MSG_TYPE_AGENT_DEATH:
            {
                struct AgentDeathMsg {
                    long    agentNum;
                };

                AgentDeathMsg *adm = new AgentDeathMsg();
                qint64 agentNum;

                in >> agentNum;

                adm->agentNum = (long)agentNum;

                emit setStatus(tr("MSG_TYPE_AGENT_DEATH:").arg(messageType) +
                               tr("[%1]").arg(adm->agentNum));

                emit agentDied(adm->agentNum);

                delete(adm);
                break;
            }
            case MSG_TYPE_FOOD_ADD:
            {
                struct AddFoodPacket {
                    long    foodNum;
                    float   foodHeight;
                    float   foodX;
                    float   foodY;
                    float   foodZ;
                };

                AddFoodPacket *afp = new AddFoodPacket();
                qint64 foodNum;

                in >> foodNum
                   >> afp->foodHeight
                   >> afp->foodX
                   >> afp->foodY
                   >> afp->foodZ;

                afp->foodNum = (long)foodNum;

                emit setStatus(tr("MSG_TYPE_FOOD_ADD:").arg(messageType) +
                               tr("[%1]").arg(afp->foodNum) +
                               tr("{%1},").arg(afp->agentHeight) +
                               tr("(%1,").arg(afp->foodX) +
                               tr("%1,").arg(afp->foodY) +
                               tr("%1)").arg(afp->foodZ));

                emit foodAdded(afp->foodNum, afp->foodHeight, afp->foodX, afp->foodY, afp->foodZ);

                delete(afp);
                break;
            }
            case MSG_TYPE_FOOD_REMOVE:
            {
                struct FoodRemovePacket {
                    long    foodNum;
                };

                FoodRemovePacket *frp = new FoodRemovePacket();
                qint64 foodNum;

                in >> foodNum;

                frp->foodNum = (long)foodNum;

                emit setStatus(tr("MSG_TYPE_FOOD_REMOVE:").arg(messageType) +
                               tr("[%1]").arg(frp->foodNum));

                emit foodRemoved(frp->foodNum);

                delete(frp);
                break;
            }
        }
    }
}
