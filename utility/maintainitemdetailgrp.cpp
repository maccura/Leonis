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
/// @file     maintainitemdetailgrp.cpp
/// @brief    维护按钮组
///
/// @author   4170/TangChuXian
/// @date     2023年3月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "maintainitemdetailgrp.h"
#include "devmaintaindetailitemwgt.h"
#include "uidcsadapter/uidcsadapter.h"
#include <QVBoxLayout>

#define MAINTAIN_ITEM_SIZE                                  QSize(930, 50)          // 维护项尺寸
#define LAYOUT_MARGIN                                       (15)                    // 布局单元外距
#define LAYOUT_SPACE                                        (6)                     // 网格间隔

MaintainItemDetailGrp::MaintainItemDetailGrp(QWidget *parent)
    : QScrollArea(parent),
      m_bInit(false),
      m_pContentWgt(new QFrame())
{
    // 显示之前初始化
    InitBeforeShow();
}

MaintainItemDetailGrp::~MaintainItemDetailGrp()
{
}

///
/// @brief
///     获取维护名列表
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月14日，新建函数
///
const QMap<QString, QStringList>& MaintainItemDetailGrp::GetMaintainNameList()
{
    return m_mapMaintainList;
}

///
/// @brief
///     设置维护列表
///
/// @param[in]  strMaintainList  维护列表
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月14日，新建函数
///
void MaintainItemDetailGrp::SetMaintainNameList(const QMap<QString, QStringList>& mapMaintainList)
{
    m_mapMaintainList = mapMaintainList;
    ResetAllMaintainStatusBtn();
}

///
/// @brief
///     设置维护状态
///
/// @param[in]  iRow            行号
/// @param[in]  enStatus        维护状态
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月14日，新建函数
///
void MaintainItemDetailGrp::SetMaintainStatus(int iRow, DevMaintainDetailItemWgt::DevMaintainItemStatus enStatus)
{
    // 获取对应的维护按钮
    auto it = m_mapMaintainToWgt.find(iRow);
    if (it == m_mapMaintainToWgt.end())
    {
        return;
    }

    // 设置维护状态
    it.value()->SetStatus(enStatus);
}

///
/// @brief
///     获取维护状态
///
/// @param[in]  iRow  行号
///
/// @return 维护状态
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月14日，新建函数
///
DevMaintainDetailItemWgt::DevMaintainItemStatus MaintainItemDetailGrp::GetMaintainStatus(int iRow)
{
    // 获取对应的维护按钮
    auto it = m_mapMaintainToWgt.find(iRow);
    if (it == m_mapMaintainToWgt.end())
    {
        return DevMaintainDetailItemWgt::DevMaintainItemStatus();
    }

    // 获取维护状态
    return it.value()->GetStatus();
}

///
/// @brief
///     设置维护时间
///
/// @param[in]  iRow  行号
/// @param[in]  strMaintainTime  维护时间
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月14日，新建函数
///
void MaintainItemDetailGrp::SetMaintainTime(int iRow, const QString& strMaintainTime)
{
    // 获取对应的维护按钮
    auto it = m_mapMaintainToWgt.find(iRow);
    if (it == m_mapMaintainToWgt.end())
    {
        return;
    }

    // 设置维护状态
    it.value()->SetMaintainTime(strMaintainTime);
}

///
/// @brief
///     获取维护进度
///
/// @param[in]  iRow  行号
///
/// @return 维护时间
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月14日，新建函数
///
QString MaintainItemDetailGrp::GetMaintainTime(int iRow)
{
    // 获取对应的维护按钮
    auto it = m_mapMaintainToWgt.find(iRow);
    if (it == m_mapMaintainToWgt.end())
    {
        return "";
    }

    // 获取维护状态
    return it.value()->GetMaintainTime();
}

///
/// @brief
///     显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月14日，新建函数
///
void MaintainItemDetailGrp::InitBeforeShow()
{
    // 设置内容窗口
    this->setWidget(m_pContentWgt);

    // 无垂直滚动条
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // 获取总行数
    int iRow = 0;
    for (auto it = m_mapMaintainList.begin(); it != m_mapMaintainList.end(); it++)
    {
        iRow += it.value().size();
    }

    // 行数至少为1
    if (iRow <= 0)
    {
        iRow = 1;
    }

    // 根据行数设置高度
    m_pContentWgt->setGeometry(0, 0, (2 * LAYOUT_MARGIN) + MAINTAIN_ITEM_SIZE.width(), (2 *LAYOUT_MARGIN) + ((iRow - 1) * LAYOUT_SPACE) + (iRow * MAINTAIN_ITEM_SIZE.height()));

    // 背景白色
    this->setStyleSheet("MaintainItemDetailGrp{background: #fff;};QFrame{background: #fff;}");

    // 使用垂直布局
    QVBoxLayout* pVLayout = new QVBoxLayout(m_pContentWgt);
    pVLayout->setMargin(LAYOUT_MARGIN);
    pVLayout->setSpacing(LAYOUT_SPACE);

    // 添加维护项
    int iItemRow = 0;
    auto AddDetailItemWgt = [this, pVLayout, &iItemRow](const QString& strDevName, const QString& strMtItemName)
    {
        // 创建对象
        DevMaintainDetailItemWgt* pItemDetail = new DevMaintainDetailItemWgt();
        pItemDetail->setFixedSize(MAINTAIN_ITEM_SIZE);
        pItemDetail->SetDevName(strDevName);
        pItemDetail->SetItemName(strMtItemName);

        // 加入布局
        pVLayout->addWidget(pItemDetail);

        // 插入容器
        m_mapMaintainToWgt.insert(iItemRow++, pItemDetail);

        // 监听重新维护信号
        connect(pItemDetail, SIGNAL(SigReMaintain(QString)), this, SIGNAL(SigReMaintain(QString)));
    };

    // 构造设备状态控件
    for (auto it = m_mapMaintainList.begin(); it != m_mapMaintainList.end(); it++)
    {
        // 遍历维护详情列表
        for (const auto& strMaintainName : it.value())
        {
            // 添加维护项
            AddDetailItemWgt(it.key(), strMaintainName);
        }
    }
}

///
/// @brief
///     显示之后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月14日，新建函数
///
void MaintainItemDetailGrp::InitAfterShow()
{
    // 初始化子控件
    InitChildCtrl();

    // 初始化信号槽连接
    InitConnect();
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月14日，新建函数
///
void MaintainItemDetailGrp::InitChildCtrl()
{
}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月14日，新建函数
///
void MaintainItemDetailGrp::InitConnect()
{
    
}

///
/// @brief
///     重置所有设备状态控件
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月14日，新建函数
///
void MaintainItemDetailGrp::ResetAllMaintainStatusBtn()
{
    // 使用垂直布局
    QVBoxLayout* pVLayout = qobject_cast<QVBoxLayout*>(m_pContentWgt->layout());
    if (pVLayout == Q_NULLPTR)
    {
        return;
    }

    // 遍历映射表
    for (auto it = m_mapMaintainToWgt.begin(); it != m_mapMaintainToWgt.end(); it++)
    {
        // 控件为空则跳过
        if (it.value() == Q_NULLPTR)
        {
            continue;
        }

        // 从布局中移除设备状态控件
        pVLayout->removeWidget(it.value());

        // 释放控件
        it.value()->deleteLater();
    }

    // 清空映射表
    m_mapMaintainToWgt.clear();

    // 获取总行数
    int iRow = 0;
    for (auto it = m_mapMaintainList.begin(); it != m_mapMaintainList.end(); it++)
    {
        iRow += it.value().size();
    }

    // 行数至少为1
    if (iRow <= 0)
    {
        iRow = 1;
    }

    // 根据行数设置高度
    m_pContentWgt->setGeometry(0, 0, (2 * LAYOUT_MARGIN) + MAINTAIN_ITEM_SIZE.width(), (2 * LAYOUT_MARGIN) + ((iRow - 1) * LAYOUT_SPACE) + (iRow * MAINTAIN_ITEM_SIZE.height()));

    // 添加维护项
    int iItemRow = 0;
    auto AddDetailItemWgt = [this, pVLayout, &iItemRow](const QString& strDevName, const QString& strMtItemName)
    {
        // 创建对象
        DevMaintainDetailItemWgt* pItemDetail = new DevMaintainDetailItemWgt();
        pItemDetail->setFixedSize(MAINTAIN_ITEM_SIZE);
        pItemDetail->SetDevName(strDevName);
        pItemDetail->SetItemName(strMtItemName);

        // 加入布局
        pVLayout->addWidget(pItemDetail);

        // 插入容器
        m_mapMaintainToWgt.insert(iItemRow++, pItemDetail);

        // 监听重新维护信号
        connect(pItemDetail, SIGNAL(SigReMaintain(QString)), this, SIGNAL(SigReMaintain(QString)));
    };

    // 构造设备状态控件
    for (auto it = m_mapMaintainList.begin(); it != m_mapMaintainList.end(); it++)
    {
        // 遍历维护详情列表
        for (const auto& strMaintainName : it.value())
        {
            // 添加维护项
            AddDetailItemWgt(it.key(), strMaintainName);
        }
    }
}

///
/// @brief
///     重写显示事件
///
/// @param[in]  event  事件
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月14日，新建函数
///
void MaintainItemDetailGrp::showEvent(QShowEvent* event)
{
    // 基类处理
    QWidget::showEvent(event);

    // 如果是第一次显示则初始化, 显示后初始化
    if (!m_bInit)
    {
        // 显示之后初始化
        m_bInit = true;
        InitAfterShow();
    }
}
