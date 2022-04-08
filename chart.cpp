/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Charts module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "rtumaster.h"

#include "chart.h"
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QtCore/QDebug>


#define SECOND_STEP 5

Chart::Chart(QGraphicsItem *parent, Qt::WindowFlags wFlags):
    QChart(QChart::ChartTypeCartesian, parent, wFlags),
//    m_seriesA(0),m_seriesB(0),m_seriesC(0),
    m_axisX(new QDateTimeAxis()),
    m_axisY(new QValueAxis()),
    m_step(0),
    m_x(QDateTime::currentDateTime())
//    m_ya(220), m_yb(220), m_yc(220)
{
    QObject::connect(&m_timer, &QTimer::timeout, this, &Chart::handleTimeout);
    m_timer.setInterval(SECOND_STEP*1000);

    /*****
    m_seriesA = new QSplineSeries(this);
    QPen phA(Qt::yellow);
    phA.setWidth(1);
    m_seriesA->setPen(phA);
    m_seriesA->append(m_x.toMSecsSinceEpoch(), m_ya);

    m_seriesB = new QSplineSeries(this);
    QPen phB(Qt::green);
    phB.setWidth(1);
    m_seriesB->setPen(phB);
    m_seriesB->append(m_x.toMSecsSinceEpoch(), m_yb);

    m_seriesC = new QSplineSeries(this);
    QPen phC(Qt::red);
    phC.setWidth(1);
    m_seriesC->setPen(phC);
    m_seriesC->append(m_x.toMSecsSinceEpoch(), m_yc);
    *****/



    addAxis(m_axisX,Qt::AlignBottom);
    addAxis(m_axisY,Qt::AlignLeft);

    /****
    addSeries(m_seriesA);
    addSeries(m_seriesB);
    addSeries(m_seriesC);

    m_seriesA->attachAxis(m_axisX);
    m_seriesA->attachAxis(m_axisY);

    m_seriesB->attachAxis(m_axisX);
    m_seriesB->attachAxis(m_axisY);

    m_seriesC->attachAxis(m_axisX);
    m_seriesC->attachAxis(m_axisY);
    ******/

    futureSecond = 100;
    m_axisX->setTickCount(10);
    m_axisX->setFormat("hh:mm:ss");
    m_axisX->setRange(m_x, QDateTime::currentDateTime().addSecs(futureSecond));

    m_axisY->setTickCount(6);
    m_axisY->setRange(190, 250);

    legend()->setVisible(true);
    legend()->setAlignment( Qt::AlignTop );
    setAnimationOptions(QChart::AllAnimations);

    // m_timer.start();
}

Chart::~Chart()
{

}

void Chart::addSlaveSeries(const SlaveItemConfig &itemcfg)
{
    int slaveAddr = itemcfg.nAddr;
    m_hashAddrToSeries[slaveAddr] = new QSplineSeries(this);
    m_hashAddrToSeries[slaveAddr]->setName( itemcfg.strLabel );
    addSeries(m_hashAddrToSeries[slaveAddr]);

    m_hashAddrToSeries[slaveAddr]->attachAxis(m_axisX);
    m_hashAddrToSeries[slaveAddr]->attachAxis(m_axisY);

    m_axisY->setTitleText( itemcfg.strUnit );

}

void Chart::updateSeries(const int slaveAddr, const float fVal)
{
//    qDebug() << "Chart::updateSeries  slaveAddr==" << slaveAddr << " value==" << fVal;
    //TODO lock
    m_hashAddrToRealValue[slaveAddr] = fVal;
}

void Chart::updateSeries(const int slaveAddr, const uint16 rawValue)
{

}

void Chart::startTimer()
{
    m_timer.start();
}

void Chart::handleTimeout()
{
    m_x = QDateTime::currentDateTime();
//    qDebug() << m_x.toMSecsSinceEpoch();

//    qreal newXPos = plotArea().width() / m_axisX->tickCount();
    qreal newXPos = (plotArea().width() / (futureSecond*1.0))*SECOND_STEP;
//    qDebug() << "new x pos " << newXPos;

//    qDebug() << m_axisX->max();


    //TODO lock
    QHash<int, QSplineSeries* >::const_iterator i = m_hashAddrToSeries.constBegin();
    while (i!=m_hashAddrToSeries.constEnd() ) {

        const int &slaveAddr = i.key();
        QSplineSeries * const &pSeries = i.value();

        if( m_x>m_axisX->max() )
        {
            pSeries->replace(m_last_x.toMSecsSinceEpoch(), m_hashAddrToLastValue[slaveAddr],
                             m_x.toMSecsSinceEpoch(), m_hashAddrToRealValue[slaveAddr]);
            scroll(newXPos, 0);
        }
        else
        {
//            qDebug() << "Chart::handleTimeout slaveAddr==" << slaveAddr << " realValue==" << m_hashAddrToRealValue[slaveAddr];
            pSeries->append(m_x.toMSecsSinceEpoch(), m_hashAddrToRealValue[slaveAddr]);
        }

        m_hashAddrToLastValue[slaveAddr] = m_hashAddrToRealValue[slaveAddr];

        ++i;
    }

    m_last_x = m_x;

    //if (m_x == 100)
        //m_timer.stop();
}
