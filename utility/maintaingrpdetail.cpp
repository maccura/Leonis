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
/// @file     maintaingrpdetail.cpp
/// @brief    维护组详情
///
/// @author   4170/TangChuXian
/// @date     2023年2月18日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月18日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "maintaingrpdetail.h"
#include "ui_maintaingrpdetail.h"
#include "maintaindatamng.h"
#include "shared/uicommon.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/CReadOnlyDelegate.h"
#include "uidcsadapter/uidcsadapter.h"
#include "uidcsadapter/abstractdevice.h"
#include "src/common/Mlog/mlog.h"
#include <QPushButton>
#include <QMap>

#define DEFAULT_ROW_CNT_OF_MAINTAIN_DETAIL_TABLE                     (14)           // 维护详情表默认行数
#define DEFAULT_COL_CNT_OF_MAINTAIN_DETAIL_TABLE                     (4)            // 维护详情表默认列数

#define COL_IDX_TIME_MAINTAIN_DETAIL_TABLE                           (3)            // 维护详情表时间列索引

MaintainGrpDetail::MaintainGrpDetail(const QString strMaintainName, QWidget *parent)
    : BaseDlg(parent),
      m_strMaintainName(strMaintainName),
      m_bInit(false),
      m_mtID(-1)
{
    // 初始化Ui对象
    ui = new Ui::MaintainGrpDetail();
    ui->setupUi(this);

    // 界面显示前初始化
    InitBeforeShow();
}

MaintainGrpDetail::~MaintainGrpDetail()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月18日，新建函数
///
void MaintainGrpDetail::InitBeforeShow()
{
    // 设置表格行数列数
    ui->MaintainDetailTbl->setRowCount(DEFAULT_ROW_CNT_OF_MAINTAIN_DETAIL_TABLE);
    ui->MaintainDetailTbl->setColumnCount(DEFAULT_COL_CNT_OF_MAINTAIN_DETAIL_TABLE);

    // 隐藏垂直表头
    ui->MaintainDetailTbl->verticalHeader()->setVisible(false);

    // 设置代理
    ui->MaintainDetailTbl->setItemDelegateForColumn(COL_IDX_TIME_MAINTAIN_DETAIL_TABLE, new CReadOnlyDelegate(this));
}

///
/// @brief
///     界面显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月18日，新建函数
///
void MaintainGrpDetail::InitAfterShow()
{
    // 初始化字符串资源
    InitStrResource();

    // 初始化信号槽连接
    InitConnect();

    // 初始化子控件
    InitChildCtrl();
}

///
/// @brief
///     初始化字符串资源
///
/// @par History:
/// @li 4170/TangChuXian，2022年1月17日，新建函数
///
void MaintainGrpDetail::InitStrResource()
{
    //ui->QcNameLab->setText(tr("质控品名称"));
    //ui->CloseBtn->setText(tr("关闭"));
}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月18日，新建函数
///
void MaintainGrpDetail::InitConnect()
{
    // 取消按钮被点击
    connect(ui->CloseBtn, SIGNAL(clicked()), this, SLOT(reject()));

    // 监听维护项时间更新
    REGISTER_HANDLER(MSG_ID_MAINTAIN_ITEM_TIME_UPDATE, this, OnMaintainItemTimeUpdate);

    // 监听维护项状态更新
    REGISTER_HANDLER(MSG_ID_MAINTAIN_ITEM_PHASE_UPDATE, this, OnMaintainItemStatusUpdate);
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月18日，新建函数
///
void MaintainGrpDetail::InitChildCtrl()
{
    // 设置标题
    SetTitleName(tr("维护详情"));

    // 设置表头
    QStringList strHeaderList;
    strHeaderList << tr("序号") << tr("维护项目") << tr("模块") << tr("完成时间");
    ui->MaintainDetailTbl->setHorizontalHeaderLabels(strHeaderList);

    // 设置表格选中模式为行选中，不可多选
    ui->MaintainDetailTbl->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->MaintainDetailTbl->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->MaintainDetailTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 设置表格列宽
    ui->MaintainDetailTbl->setColumnWidth(0, 100);
    ui->MaintainDetailTbl->setColumnWidth(1, 240);
    ui->MaintainDetailTbl->setColumnWidth(2, 755);

    // 更新维护详情表
    UpdateMaintainDetailTbl();

    // 表格列宽自适应
    // ResizeTblColToContent(ui->MaintainDetailTbl);
}

///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月18日，新建函数
///
void MaintainGrpDetail::showEvent(QShowEvent *event)
{
    // 让基类处理事件
    BaseDlg::showEvent(event);

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }
}

///
/// @brief
///     维护项状态更新
///
/// @param[in]  strDevSn        设备序列号
/// @param[in]  devModuleIndex  设备模块号（ISE维护专用，0表示所有模块，≥1表示针对指定的模块）
/// @param[in]  lGrpId			维护组ID
/// @param[in]  enItemType      维护项类型
/// @param[in]  enPhaseType     阶段类型
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月16日，新建函数
///
void MaintainGrpDetail::OnMaintainItemStatusUpdate(QString strDevSn, int devModuleIndex, long long lGrpId, tf::MaintainItemType::type enItemType, tf::MaintainResult::type enPhaseType)
{
    Q_UNUSED(strDevSn);
	Q_UNUSED(devModuleIndex);
	Q_UNUSED(lGrpId);
    Q_UNUSED(enItemType);
    Q_UNUSED(enPhaseType);
    // 更新维护详情表
    UpdateMaintainDetailTbl();
}

///
/// @brief
///     维护项时间更新
///
/// @param[in]  strDevSn        设备序列号
/// @param[in]  lGrpId          维护组ID
/// @param[in]  enItemType      维护项类型
/// @param[in]  strTime         时间
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月16日，新建函数
///
void MaintainGrpDetail::OnMaintainItemTimeUpdate(QString strDevSn, long long lGrpId, tf::MaintainItemType::type enItemType, QString strTime)
{
    Q_UNUSED(strDevSn);
    Q_UNUSED(lGrpId);
    Q_UNUSED(enItemType);
    Q_UNUSED(strTime);
    // 更新维护详情表
    UpdateMaintainDetailTbl();
}

///
/// @brief
///     更新维护详情表
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月30日，新建函数
///
void MaintainGrpDetail::UpdateMaintainDetailTbl()
{
    // 清空表格内容
    ui->MaintainDetailTbl->clearContents();

    // 获取当前维护组
    tf::MaintainGroup stuMtGrp = MaintainDataMng::GetInstance()->GetMaintainGrpByName(m_strMaintainName);

    // 获取所有维护详情
    QList<tf::LatestMaintainInfo> detailLst;
    if (!MaintainDataMng::GetInstance()->GetAllMaintainDetail(detailLst))
    {
        ULOG(LOG_ERROR, "%s(), MaintainDataMng::GetInstance()->GetAllMaintainDetail failed.", __FUNCTION__);
        return;
    }

    // 构造以维护单项为目标的表格
    struct MaintainDetail
    {
        tf::MaintainItemType::type enItemType;
        QMap<QString, tf::MaintainResult::type> mapDevMtStatus;
        QString strCompleteTime;
    };

    // 建立维护单项和维护执行结果的映射
    QMap<tf::MaintainItemType::type, MaintainDetail> mapMaintainItem;

    // 遍历维护详情
    for (const auto& stuDevMaintainDetail : detailLst)
    {
        // 维护组不匹配则跳过
        if (stuDevMaintainDetail.groupId != stuMtGrp.id)
        {
            continue;
        }

        // 获取设备名
        QPair<QString, QString> strDevNamePair = gUiAdapterPtr()->GetDevNameBySn(QString::fromStdString(stuDevMaintainDetail.deviceSN));
        if (!strDevNamePair.first.isEmpty())
        {
            continue;
        }

        // 获取设备名
        QString strDevName = strDevNamePair.second.isEmpty() ? strDevNamePair.first : (strDevNamePair.second + strDevNamePair.first);

        for (const auto& stuMaintainDetail : stuDevMaintainDetail.resultDetail)
        {
            MaintainDetail stuDetail;
            auto it = mapMaintainItem.find(stuMaintainDetail.itemType);
            if (it != mapMaintainItem.end())
            {
                stuDetail = it.value();
            }

            // 更新维护详情
            stuDetail.enItemType = stuMaintainDetail.itemType;
            stuDetail.mapDevMtStatus.insert(strDevName, stuMaintainDetail.result);
            if (stuDetail.strCompleteTime.isEmpty() || (stuDetail.strCompleteTime < QString::fromStdString(stuMaintainDetail.exeTime)))
            {
                stuDetail.strCompleteTime = QString::fromStdString(stuMaintainDetail.exeTime);
            }

            // 更新维护单项详情表
            mapMaintainItem.insert(stuDetail.enItemType, stuDetail);
        }
    }

    // 获取富文本
    auto GetRichText = [](const QMap<QString, tf::MaintainResult::type>& devMaintainStatusMap)
    {
        QStringList strRichTextList;
        QString strWaitRichFormat = QString("<font style=' color: #565656;'>%1</font>");
        QString strSuccRichFormat = QString("<font style=' color: #05b30d;'>%1</font>");
        QString strFailRichFormat = QString("<font style=' color: #e41313;'>%1</font>");
        for (auto it = devMaintainStatusMap.begin(); it != devMaintainStatusMap.end(); it++)
        {
            // 根据维护状态生成富文本
            if (it.value() == tf::MaintainResult::MAINTAIN_RESULT_SUCCESS)
            {
                strRichTextList.push_back(strSuccRichFormat.arg(it.key()));
            }
            else if (it.value() == tf::MaintainResult::MAINTAIN_RESULT_FAIL)
            {
                strRichTextList.push_back(strFailRichFormat.arg(it.key()));
            }
            else
            {
                strRichTextList.push_back(strWaitRichFormat.arg(it.key()));
            }
        }

        // 待完善（富文本中间插入任意个空格始终只显示一个，暂不知道原因）
        return strRichTextList.join(" ");
    };

    // 将维护详情信息填入表格中
    int iRow = 0;
    for (auto it = mapMaintainItem.begin(); it != mapMaintainItem.end(); it++)
    {
        // 行数自适应
        if (iRow >= ui->MaintainDetailTbl->rowCount())
        {
            ui->MaintainDetailTbl->setRowCount(iRow + 1);
        }

        // 获取富文本
        QString strDevStatusRichText = GetRichText(it.value().mapDevMtStatus);

        // 获取维护项名称
        auto strSubItem = MaintainDataMng::GetInstance()->GetMaintainItemNameByType(it.value().enItemType);

        // 设置表格内容
        ui->MaintainDetailTbl->setItem(iRow, 0, new QTableWidgetItem(QString::number(iRow + 1)));                       // 序号
        ui->MaintainDetailTbl->setItem(iRow, 1, new QTableWidgetItem(strSubItem));                                      // 维护项目
        ui->MaintainDetailTbl->setItem(iRow, 2, new QTableWidgetItem(" "));                                             // 模块
        ui->MaintainDetailTbl->setItem(iRow, 3, new QTableWidgetItem(it.value().strCompleteTime));                      // 完成时间

                                                                                                                        // 设置富文本
        QLabel* pDevStatusLab = new QLabel();
        pDevStatusLab->setText(strDevStatusRichText);
        ui->MaintainDetailTbl->setCellWidget(iRow, 2, pDevStatusLab);

        // 行号自增
        ++iRow;
    }
}
