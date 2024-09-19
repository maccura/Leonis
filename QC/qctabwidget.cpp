/***************************************************************************
*   This file is part of the leonis project                               *
*   Copyright (C) 2024 by Mike Medical Electronics Co., Ltd               *
*   zhouguangming@maccura.com                                             *
*                                                                         *
**                   GNU General Public License Usage                    **
*                                                                         *
*   This library is free software: you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation, either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*   You should have received a copy of the GNU General Public License     *
*   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
*                                                                         *
**                  GNU Lesser General Public License                    **
*                                                                         *
*   This library is free software: you can redistribute it and/or modify  *
*   it under the terms of the GNU Lesser General Public License as        *
*   published by the Free Software Foundation, either version 3 of the    *
*   License, or (at your option) any later version.                       *
*   You should have received a copy of the GNU Lesser General Public      *
*   License along with this library.                                      *
*   If not, see <http://www.gnu.org/licenses/>.                           *
*                                                                         *
*   This library is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
****************************************************************************/

///////////////////////////////////////////////////////////////////////////
/// @file     qctabwidget.cpp
/// @brief    质控tab界面
///
/// @author   4170/TangChuXian
/// @date     2022年12月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "qctabwidget.h"
#include "qcapplywidget.h"
#include "qcgraphljwidget.h"
#include "qcgraphyoudenwidget.h"
#include "ch/QQcSettingPage.h"   // 待完善
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/qryconddisplaywgt.h"
#include <QHBoxLayout>
#include <QRadioButton>
#include <QSignalMapper>
#include <QTabBar>

#define TAB_INDEX_QC_APPLY               0            // 质控申请tab索引
#define TAB_INDEX_QC_GRAPH_LJ            1            // 质控LJ图tab索引
#define TAB_INDEX_QC_GRAPH_YOUDEN        2            // 质控YOUDEN图tab索引
#define TAB_INDEX_QC_CFG                 3            // 质控设置tab索引

#define CORNER_WIDGET_LEFT_MARGIN        (10)         // 角落窗口左边距

QcTabWidget::QcTabWidget(QWidget *parent)
    : QTabWidget(parent),
      m_bInit(false),
      m_pCornerWidget(Q_NULLPTR)
{
    // 显示之前初始化
    InitBeforeShow();
}

QcTabWidget::~QcTabWidget()
{
}

///
/// @brief
///     显示之前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月15日，新建函数
///
void QcTabWidget::InitBeforeShow()
{
    // 试剂加载器的更新
    REGISTER_HANDLER(MSG_ID_QC_TAB_CORNER_DISPLAY_UPDATE, this, OnTabCornerUpdate);

    // 插入tab页面
    insertTab(TAB_INDEX_QC_APPLY, new QcApplyWidget(this), tr("质控申请"));
    insertTab(TAB_INDEX_QC_GRAPH_LJ, new QcGraphLJWidget(this), tr("Levey-Jennings图"));
    insertTab(TAB_INDEX_QC_GRAPH_YOUDEN, new QcGraphYoudenWidget(this), tr("Twin Plot图"));
    insertTab(TAB_INDEX_QC_CFG, new QQcSettingPage(this), tr("质控设置"));
}

///
/// @brief 界面显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月14日，新建函数
///
void QcTabWidget::InitAfterShow()
{
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(OnTabInexChanged(int)));
}

///
/// @brief 窗口显示事件
///     
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月14日，新建函数
///
void QcTabWidget::showEvent(QShowEvent *event)
{
    // 让基类处理事件
    QTabWidget::showEvent(event);

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }
}

///
/// @brief
///     tab页面角落窗口更新
///
/// @param[in]  strDevNameList  设备列表
/// @param[in]  iSelDevIdx      当前选中设备索引
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月15日，新建函数
///
void QcTabWidget::OnTabCornerUpdate(QStringList strDevNameList, int iSelDevIdx)
{
    // 如果角落窗口不为空，则释放
    if (m_pCornerWidget != Q_NULLPTR)
    {
        m_pCornerWidget->deleteLater();
    }

    // 构建角落窗口
    int iCornerX = this->x() + this->tabBar()->width() + CORNER_WIDGET_LEFT_MARGIN;
    int iCornerY = this->y();
    int iCornerWidth = this->width() - this->tabBar()->width() - CORNER_WIDGET_LEFT_MARGIN;
    int iCornerHeight = this->tabBar()->height();
    m_pCornerWidget = new QWidget(this->parentWidget());
    m_pCornerWidget->setGeometry(iCornerX, iCornerY, iCornerWidth, iCornerHeight);

    // 如果只有一个设备按钮，则不做处理
    int iDevBtnCnt = 0;
    for (const QString& strDevName : strDevNameList)
    {
        // 构建单选框
        QRadioButton *pRBtn;
        if (!strDevName.isEmpty())
        {
            ++iDevBtnCnt;
        }
    }

    // 如果设备数小于等于1，则不显示设备选择界面，直接返回
//     if (iDevBtnCnt <= 1)
//     {
//         return;
//     }

    // 构建新的角落窗口
    //QryCondDisplayWgt*  pQryCondWgt     = new QryCondDisplayWgt();
    QSignalMapper*      pSignalMapper   = new QSignalMapper(m_pCornerWidget);
    QHBoxLayout*        pHLayout        = new QHBoxLayout(m_pCornerWidget);
    int                 iBtnIdx         = 0;
    pHLayout->setSpacing(20);
    //pHLayout->setMargin(10);
    pHLayout->setContentsMargins(10, 2, 31, 0);
//     pHLayout->addWidget(new QLabel("测试下"));
//     pHLayout->addWidget(new QLabel("测试下2"));
//     pHLayout->addWidget(new QRadioButton("测试下3"));
    //pHLayout->addWidget(pQryCondWgt);
    //pQryCondWgt->SetText(strQryCond);
    pHLayout->addStretch(1);
   // connect(pQryCondWgt, SIGNAL(SigResetQry()), this, SLOT(OnQryCondReset()));
    for (const QString& strDevName : strDevNameList)
    {
        // 如果设备数小于等于1，则不显示设备
        if (iDevBtnCnt <= 1)
        {
            continue;
        }

        // 构建单选框
        QRadioButton *pRBtn;
        if (strDevName.isEmpty())
        {
            pRBtn = new QRadioButton(tr("全部"));
        }
        else
        {
            pRBtn = new QRadioButton(strDevName);
        }

        // 索引匹配则选中
        if (iBtnIdx == iSelDevIdx)
        {
            pRBtn->setChecked(true);
        }

        // 添加到水平布局中
        pHLayout->addWidget(pRBtn);

        // 连接信号槽
        pSignalMapper->setMapping(pRBtn, iBtnIdx);
        connect(pRBtn, SIGNAL(clicked()), pSignalMapper, SLOT(map()));

        // 索引自增
        ++iBtnIdx;
    }
    //pHLayout->addSpacing(20);

    // 临时处理（主界面窗口尺寸存在问题，暂时添加一个工具窗口调整cornerWidget宽度）
//     QWidget* pToolWgt = new QWidget();
//     pToolWgt->setFixedWidth(160);
//     pHLayout->addWidget(pToolWgt);

    // 连接信号映射器
    connect(pSignalMapper, SIGNAL(mapped(int)), this, SLOT(OnCornerRBtnClicked(int)));

    // 设置角落窗口
    m_pCornerWidget->show();
}

///
/// @brief
///     角落单元按钮被点击
///
/// @param[in]  iBtnIdx 按钮索引
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月15日，新建函数
///
void QcTabWidget::OnCornerRBtnClicked(int iBtnIdx)
{
    // 通知角落按钮索引更新
    POST_MESSAGE(MSG_ID_QC_TAB_CORNER_INDEX_UPDATE, iBtnIdx);
}

///
/// @brief
///     tab索引改变（临时添加函数，后续删除）
///
/// @param[in]  iTabIdx  tab索引
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月22日，新建函数
///
void QcTabWidget::OnTabInexChanged(int iTabIdx)
{
    // 释放掉之前的角落窗口
    if (m_pCornerWidget != Q_NULLPTR)
    {
        if (iTabIdx == TAB_INDEX_QC_CFG)
        {
            m_pCornerWidget->hide();
        }
        else
        {
            m_pCornerWidget->show();
        }
    }
}

///
/// @brief
///     查询条件重置
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月15日，新建函数
///
void QcTabWidget::OnQryCondReset()
{
    // 通知角落按钮索引更新
    POST_MESSAGE(MSG_ID_QC_TAB_CORNER_QRY_COND_RESET);
}
