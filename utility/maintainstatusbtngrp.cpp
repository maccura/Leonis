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
/// @file     maintainstatusbtngrp.cpp
/// @brief    维护按钮组
///
/// @author   4170/TangChuXian
/// @date     2023年2月16日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月16日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "maintainstatusbtngrp.h"
#include "manager/UserInfoManager.h"
#include <QGridLayout>

#define COL_CNT_OF_GRID_LAYOUT                              (7)                     // 网格布局列数
#define ROW_CNT_OF_GRID_LAYOUT_PER_PAGE                     (5)                     // 网格布局每页行数
#define MAINTAIN_BTN_SIZE                                   QSize(261, 87)          // 维护按钮尺寸
#define GRID_MARGIN                                         (10)                    // 网格单元外距
#define GRID_SPACE                                          (5)                    // 网格间隔

MaintainStatusBtnGrp::MaintainStatusBtnGrp(QWidget *parent)
    : QScrollArea(parent),
      m_bInit(false),
      m_iColCnt(COL_CNT_OF_GRID_LAYOUT),
      m_pContentWgt(new QFrame())
{
    // 显示之前初始化
    InitBeforeShow();
}

MaintainStatusBtnGrp::~MaintainStatusBtnGrp()
{
}

///
/// @brief
///     获取维护名列表
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月16日，新建函数
///
const QStringList& MaintainStatusBtnGrp::GetMaintainNameList()
{
    return m_strMaintainList;
}

///
/// @brief
///     设置维护列表
///
/// @param[in]  strMaintainList     维护列表
/// @param[in]  bIsItem             是否是单项维护
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月16日，新建函数
///
void MaintainStatusBtnGrp::SetMaintainNameList(const QStringList& strMaintainList, bool bIsItem /*= false*/)
{
    m_bMtItem = bIsItem;
    m_strMaintainList = strMaintainList;
    ResetAllMaintainStatusBtn();
}

///
/// @brief
///     设置维护类型
///
/// @param[in]  strMaintainName  维护名
/// @param[in]  enType           类型
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月17日，新建函数
///
void MaintainStatusBtnGrp::SetMaintainType(const QString& strMaintainName, MaintainStatusBtn::BtnMaintainType enType)
{
    // 获取对应的维护按钮
    auto it = m_mapMaintainToWgt.find(strMaintainName);
    if (it == m_mapMaintainToWgt.end())
    {
        return;
    }

    // 设置维护类型
    it.value()->SetMaintainType(enType);
}

///
/// @brief
///     获取维护类型
///
/// @param[in]  strMaintainName  维护名
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月17日，新建函数
///
MaintainStatusBtn::BtnMaintainType MaintainStatusBtnGrp::GetMaintainType(const QString& strMaintainName)
{
    // 获取对应的维护按钮
    auto it = m_mapMaintainToWgt.find(strMaintainName);
    if (it == m_mapMaintainToWgt.end())
    {
        return MaintainStatusBtn::BtnMaintainType();
    }

    // 获取维护类型
    return it.value()->GetMaintainType();
}

///
/// @brief
///     设置维护状态
///
/// @param[in]  strMaintainName  维护名
/// @param[in]  enStatus         维护状态
///
/// @return 
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月17日，新建函数
///
void MaintainStatusBtnGrp::SetMaintainStatus(const QString& strMaintainName, MaintainStatusBtn::BtnMaintainStatus enStatus)
{
    // 获取对应的维护按钮
    auto it = m_mapMaintainToWgt.find(strMaintainName);
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
/// @param[in]  strMaintainName  维护名
///
/// @return 维护状态
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月17日，新建函数
///
MaintainStatusBtn::BtnMaintainStatus MaintainStatusBtnGrp::GetMaintainStatus(const QString& strMaintainName)
{
    // 获取对应的维护按钮
    auto it = m_mapMaintainToWgt.find(strMaintainName);
    if (it == m_mapMaintainToWgt.end())
    {
        return MaintainStatusBtn::BtnMaintainStatus();
    }

    // 获取维护状态
    return it.value()->GetStatus();
}

///
/// @brief
///     设置设备类别
///
/// @param[in]  strMaintainName  维护名
/// @param[in]  enStatus         维护状态
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月19日，新建函数
///
void MaintainStatusBtnGrp::SetDevClassify(const QString& strMaintainName, MaintainStatusBtn::BtnDevClassify enDevClassify)
{
    // 获取对应的维护按钮
    auto it = m_mapMaintainToWgt.find(strMaintainName);
    if (it == m_mapMaintainToWgt.end())
    {
        return;
    }

    // 设置维护状态
    it.value()->SetDevClassify(enDevClassify);
}

///
/// @brief
///     获取设备类别
///
/// @param[in]  strMaintainName  维护名
///
/// @return 维护状态
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月19日，新建函数
///
MaintainStatusBtn::BtnDevClassify MaintainStatusBtnGrp::GetDevClassify(const QString& strMaintainName)
{
    // 获取对应的维护按钮
    auto it = m_mapMaintainToWgt.find(strMaintainName);
    if (it == m_mapMaintainToWgt.end())
    {
        return MaintainStatusBtn::BtnDevClassify();
    }

    // 获取维护状态
    return it.value()->GetDevClassify();
}

///
/// @brief
///     设置维护进度
///
/// @param[in]  strMaintainName  维护名
/// @param[in]  dProgress        进度百分百，1表示百分之百，0表示百分之0
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月17日，新建函数
///
void MaintainStatusBtnGrp::SetMaintainProgress(const QString& strMaintainName, double dProgress)
{
    // 获取对应的维护按钮
    auto it = m_mapMaintainToWgt.find(strMaintainName);
    if (it == m_mapMaintainToWgt.end())
    {
        return;
    }

    // 设置维护进度
    it.value()->SetProgress(dProgress);
}

///
/// @brief
///     获取维护进度
///
/// @param[in]  strMaintainName  维护名
///
/// @return 维护进度
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月17日，新建函数
///
double MaintainStatusBtnGrp::GetMaintainProgress(const QString& strMaintainName)
{
    // 获取对应的维护按钮
    auto it = m_mapMaintainToWgt.find(strMaintainName);
    if (it == m_mapMaintainToWgt.end())
    {
        return 1.0;
    }

    // 获取维护进度
    return it.value()->GetProgress();
}

///
/// @brief
///     设置维护进度是否可见
///
/// @param[in]  strMaintainName     进度名
/// @param[in]  bVisible            是否可见
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月17日，新建函数
///
void MaintainStatusBtnGrp::SetMaintainProgressVisible(const QString& strMaintainName, bool bVisible)
{
    // 获取对应的维护按钮
    auto it = m_mapMaintainToWgt.find(strMaintainName);
    if (it == m_mapMaintainToWgt.end())
    {
        return;
    }

    // 设置进度是否显示
    it.value()->SetProgressVisible(bVisible);
}

///
/// @brief
///     获取维护进度是否可见
///
/// @param[in]  strMaintainName  维护名
///
/// @return true表示可见
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月17日，新建函数
///
bool MaintainStatusBtnGrp::IsMaintainProgressVisible(const QString& strMaintainName)
{
    // 获取对应的维护按钮
    auto it = m_mapMaintainToWgt.find(strMaintainName);
    if (it == m_mapMaintainToWgt.end())
    {
        return false;
    }

    // 获取进度是否显示
    return it.value()->IsProgressVisible();
}

///
/// @brief
///     显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月16日，新建函数
///
void MaintainStatusBtnGrp::InitBeforeShow()
{
    // 设置内容窗口
    this->setWidget(m_pContentWgt);

    // 无水平滚动条
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 获取总行数
    int iRow = 1;
    int iCol = 1;
    if (!m_strMaintainList.isEmpty())
    {
        iRow = (m_strMaintainList.size() - 1) / m_iColCnt + 1;
        iCol = m_strMaintainList.size() < m_iColCnt ? m_strMaintainList.size() : m_iColCnt;
    }

    // 根据行数设置高度
    m_pContentWgt->setGeometry(0, 0, (GRID_MARGIN * 2) + ((iCol - 1) * GRID_SPACE) + (MAINTAIN_BTN_SIZE.width() * iCol),
        (2 * GRID_MARGIN) + ((iRow - 1) * GRID_SPACE) + (iRow * MAINTAIN_BTN_SIZE.height()));

	this->setStyleSheet("MaintainStatusBtnGrp{background: #fff;};QFrame{background: #fff;}");

    // 使用水平布局
    QGridLayout* pGridLayout = new QGridLayout(m_pContentWgt);
    pGridLayout->setMargin(GRID_MARGIN);
    pGridLayout->setSpacing(GRID_SPACE);

    // 构造设备状态控件
    int iMtIdx = 0;
    for (const auto& strMaintainName : m_strMaintainList)
    {
        // 创建对象
        MaintainStatusBtn* pBtn = new MaintainStatusBtn();
        pBtn->setFixedSize(MAINTAIN_BTN_SIZE);
        pBtn->SetMaintainName(strMaintainName);

        // 加入布局
        pGridLayout->addWidget(pBtn, iMtIdx / m_iColCnt, iMtIdx % m_iColCnt);

        // 加入映射表
        m_mapMaintainToWgt.insert(strMaintainName, pBtn);

        // 索引自增
        ++iMtIdx;
    }
}

///
/// @brief
///     显示之后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月16日，新建函数
///
void MaintainStatusBtnGrp::InitAfterShow()
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
/// @li 4170/TangChuXian，2023年2月16日，新建函数
///
void MaintainStatusBtnGrp::InitChildCtrl()
{
}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月16日，新建函数
///
void MaintainStatusBtnGrp::InitConnect()
{
}

///
/// @brief
///     重置所有设备状态控件
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月16日，新建函数
///
void MaintainStatusBtnGrp::ResetAllMaintainStatusBtn()
{
    // 使用水平布局
    QGridLayout* pGridLayout = qobject_cast<QGridLayout*>(m_pContentWgt->layout());
    if (pGridLayout == Q_NULLPTR)
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
        pGridLayout->removeWidget(it.value());

        // 释放控件
        it.value()->deleteLater();
    }

    // 清空映射表
    m_mapMaintainToWgt.clear();

    // 获取总行数
    int iRow = 1;
    int iCol = 1;
    if (!m_strMaintainList.isEmpty())
    {
        iRow = (m_strMaintainList.size() - 1) / m_iColCnt + 1;
        iCol = m_strMaintainList.size() < m_iColCnt ? m_strMaintainList.size() : m_iColCnt;
    }

    // 根据行数设置高度
    m_pContentWgt->setGeometry(0, 0, (GRID_MARGIN * 2) + ((iCol - 1) * GRID_SPACE) + (MAINTAIN_BTN_SIZE.width() * iCol),
        (2 * GRID_MARGIN) + ((iRow - 1) * GRID_SPACE) + (iRow * MAINTAIN_BTN_SIZE.height()));

    // 如果是维护单项，特殊排序
    QStringList strItemTypeUserList;
    QStringList strItemTypeEng1List;
    QStringList strItemTypeEng2List;
    QStringList strItemTypeEng3List;
    strItemTypeUserList << tr("仪器复位") << tr("免疫试剂扫描") << tr("试剂混匀") << tr("整机管路填充") << tr("测试结束维护") << tr("底物管路填充") << tr("磁分离管路填充")
        << tr("清除样本架") << tr("清除反应杯") << tr("底物停用管路维护") << tr("仪器关机") << tr("清除管路填充管") << tr("在线配液自检") << tr("故障自动修复");
    strItemTypeEng1List << tr("整机管路清洗") << tr("底物管路清洗") << tr("磁分离管路清洗") << tr("样本针洗针") << tr("试剂针洗针") << tr("洗针池灌注") << tr("在线配液排空")
        << tr("整机管路排空") << tr("添加管路填充管");
    strItemTypeEng2List << tr("样本针特殊清洗") << tr("抽液针特殊清洗") << tr("整机针特殊清洗") << tr("整机针浸泡");
    strItemTypeEng3List << tr("样本针堵针自检") << tr("第一磁分离堵针自检") << tr("第二磁分离堵针自检") << tr("第一磁分离注液量自检") << tr("第二磁分离注液量自检")
        << tr("底物A注液量自检") << tr("底物B注液量自检") << tr("第一试剂针注液量自检") << tr("第二试剂针注液量自检");

    // 如果是单项维护，特殊排序
    if (m_bMtItem)
    {
        // 行偏移
        int iRowOffset = 0;

        // 先排第一组
        {
            // 构造设备状态控件
            int iMtIdx = 0;
            for (const QString& strMaintainName : strItemTypeUserList)
            {
                // 不包含则跳过
                if (!m_strMaintainList.contains(strMaintainName))
                {
                    continue;
                }

                // 创建对象
                MaintainStatusBtn* pBtn = new MaintainStatusBtn();
                pBtn->setFixedSize(MAINTAIN_BTN_SIZE);
                pBtn->SetMaintainName(strMaintainName);

                // 连接信号槽
                connect(pBtn, SIGNAL(clicked()), this, SLOT(OnMtStatusBtnClicked()));

                // 加入布局
                pGridLayout->addWidget(pBtn, iMtIdx / m_iColCnt, iMtIdx % m_iColCnt);

                // 加入映射表
                m_mapMaintainToWgt.insert(strMaintainName, pBtn);

                // 索引自增
                ++iMtIdx;
            }

            if (iMtIdx > 0)
            {
                iRowOffset = (iMtIdx - 1) / m_iColCnt + 1;
            }
        }

        // 根据行数设置高度
        iRow = iRowOffset;
        m_pContentWgt->setGeometry(0, 0, (GRID_MARGIN * 2) + ((iCol - 1) * GRID_SPACE) + (MAINTAIN_BTN_SIZE.width() * iCol),
            (2 * GRID_MARGIN) + ((iRow - 1) * GRID_SPACE) + (iRow * MAINTAIN_BTN_SIZE.height()));

        // 初始化成员变量（当前登录用户）
        std::shared_ptr<tf::UserInfo> spLoginUserInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
        if ((spLoginUserInfo == Q_NULLPTR) || (spLoginUserInfo->type < tf::UserType::USER_TYPE_ENGINEER))
        {
            return;
        }

        // 排第二组
        {
            // 构造设备状态控件
            int iMtIdx = 0;
            for (const QString& strMaintainName : strItemTypeEng1List)
            {
                // 不包含则跳过
                if (!m_strMaintainList.contains(strMaintainName))
                {
                    continue;
                }

                // 创建对象
                MaintainStatusBtn* pBtn = new MaintainStatusBtn();
                pBtn->setFixedSize(MAINTAIN_BTN_SIZE);
                pBtn->SetMaintainName(strMaintainName);

                // 连接信号槽
                connect(pBtn, SIGNAL(clicked()), this, SLOT(OnMtStatusBtnClicked()));

                // 加入布局
                pGridLayout->addWidget(pBtn, iMtIdx / m_iColCnt + iRowOffset, iMtIdx % m_iColCnt);

                // 加入映射表
                m_mapMaintainToWgt.insert(strMaintainName, pBtn);

                // 索引自增
                ++iMtIdx;
            }

            if (iMtIdx > 0)
            {
                iRowOffset += (iMtIdx - 1) / m_iColCnt + 1;
            }
        }

        // 排第三组
        {
            // 构造设备状态控件
            int iMtIdx = 0;
            for (const QString& strMaintainName : strItemTypeEng2List)
            {
                // 不包含则跳过
                if (!m_strMaintainList.contains(strMaintainName))
                {
                    continue;
                }

                // 创建对象
                MaintainStatusBtn* pBtn = new MaintainStatusBtn();
                pBtn->setFixedSize(MAINTAIN_BTN_SIZE);
                pBtn->SetMaintainName(strMaintainName);

                // 连接信号槽
                connect(pBtn, SIGNAL(clicked()), this, SLOT(OnMtStatusBtnClicked()));

                // 加入布局
                pGridLayout->addWidget(pBtn, iMtIdx / m_iColCnt + iRowOffset, iMtIdx % m_iColCnt);

                // 加入映射表
                m_mapMaintainToWgt.insert(strMaintainName, pBtn);

                // 索引自增
                ++iMtIdx;
            }

            if (iMtIdx > 0)
            {
                iRowOffset += (iMtIdx - 1) / m_iColCnt + 1;
            }
        }

        // 排第四组
        {
            // 构造设备状态控件
            int iMtIdx = 0;
            for (const QString& strMaintainName : strItemTypeEng3List)
            {
                // 不包含则跳过
                if (!m_strMaintainList.contains(strMaintainName))
                {
                    continue;
                }

                // 创建对象
                MaintainStatusBtn* pBtn = new MaintainStatusBtn();
                pBtn->setFixedSize(MAINTAIN_BTN_SIZE);
                pBtn->SetMaintainName(strMaintainName);

                // 连接信号槽
                connect(pBtn, SIGNAL(clicked()), this, SLOT(OnMtStatusBtnClicked()));

                // 加入布局
                pGridLayout->addWidget(pBtn, iMtIdx / m_iColCnt + iRowOffset, iMtIdx % m_iColCnt);

                // 加入映射表
                m_mapMaintainToWgt.insert(strMaintainName, pBtn);

                // 索引自增
                ++iMtIdx;
            }

            if (iMtIdx > 0)
            {
                iRowOffset += (iMtIdx - 1) / m_iColCnt + 1;
            }
        }

        // 排剩余
        {
            // 构造设备状态控件
            int iMtIdx = 0;
            for (const QString& strMaintainName : m_strMaintainList)
            {
                // 包含则跳过
                if (strItemTypeUserList.contains(strMaintainName) ||
                    strItemTypeEng1List.contains(strMaintainName) || 
                    strItemTypeEng2List.contains(strMaintainName) ||
                    strItemTypeEng3List.contains(strMaintainName))
                {
                    continue;
                }

                // 创建对象
                MaintainStatusBtn* pBtn = new MaintainStatusBtn();
                pBtn->setFixedSize(MAINTAIN_BTN_SIZE);
                pBtn->SetMaintainName(strMaintainName);

                // 连接信号槽
                connect(pBtn, SIGNAL(clicked()), this, SLOT(OnMtStatusBtnClicked()));

                // 加入布局
                pGridLayout->addWidget(pBtn, iMtIdx / m_iColCnt + iRowOffset, iMtIdx % m_iColCnt);

                // 加入映射表
                m_mapMaintainToWgt.insert(strMaintainName, pBtn);

                // 索引自增
                ++iMtIdx;
            }

            if (iMtIdx > 0)
            {
                iRowOffset += (iMtIdx - 1) / m_iColCnt + 1;
            }
        }

        // 设置行数
        iRow = iRowOffset;
        m_pContentWgt->setGeometry(0, 0, (GRID_MARGIN * 2) + ((iCol - 1) * GRID_SPACE) + (MAINTAIN_BTN_SIZE.width() * iCol),
            (2 * GRID_MARGIN) + ((iRow - 1) * GRID_SPACE) + (iRow * MAINTAIN_BTN_SIZE.height()));

        return;
    }

    // 构造设备状态控件
    int iMtIdx = 0;
    for (const QString& strMaintainName : m_strMaintainList)
    {
        // 创建对象
        MaintainStatusBtn* pBtn = new MaintainStatusBtn();
        pBtn->setFixedSize(MAINTAIN_BTN_SIZE);
        pBtn->SetMaintainName(strMaintainName);

        // 连接信号槽
        connect(pBtn, SIGNAL(clicked()), this, SLOT(OnMtStatusBtnClicked()));

        // 加入布局
        pGridLayout->addWidget(pBtn, iMtIdx / m_iColCnt, iMtIdx % m_iColCnt);

        // 加入映射表
        m_mapMaintainToWgt.insert(strMaintainName, pBtn);

        // 索引自增
        ++iMtIdx;
    }
}

///
/// @brief
///     重写显示事件
///
/// @param[in]  event  事件
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月16日，新建函数
///
void MaintainStatusBtnGrp::showEvent(QShowEvent* event)
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

///
/// @brief
///     维护状态按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月18日，新建函数
///
void MaintainStatusBtnGrp::OnMtStatusBtnClicked()
{
    // 获取信号发送者
    MaintainStatusBtn* pBtn = qobject_cast<MaintainStatusBtn*>(sender());
    if (pBtn == Q_NULLPTR)
    {
        return;
    }

    emit SigMtStatusBtnClicked(pBtn->GetMaintainName());
}
