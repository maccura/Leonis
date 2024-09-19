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
/// @file     alarmShieldDlg.cpp
/// @brief    主界面->告警对话框->报警屏蔽列表
///
/// @author   7951/LuoXin
/// @date     2023年6月12日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2023年6月12日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "alarmShieldDlg.h"
#include "ui_alarmShieldDlg.h"
#include <QToolTip>
#include <QHelpEvent>
#include <QStandardItemModel>
#include "alarmModel.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "shared/CommonInformationManager.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "src/public/ConfigDefine.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/defs.h"
#include "thrift/DcsControlProxy.h"
#include "manager/UserInfoManager.h"
#include "src/thrift/track/gen-cpp/track_types.h"

enum DATA_COLUNM			// 信息在表格中的对应列
{
    COLUNM_DEV_NAME,		// 模块
    COLUNM_DEV_SN,          // 设备序列号
    COLUNM_DEV_TYPE,        // 设备类型
    COLUNM_CODE,			// 报警码
    COLUNM_LEVEL,			// 报警等级
    COLUNM_ALARM_NAME		// 报警标题
};

alarmShieldDlg::alarmShieldDlg(QWidget *parent /*= Q_NULLPTR*/)
    : BaseDlg(parent)
    , ui(new Ui::alarmShieldDlg)
{
    m_tabViewMode = new QStandardItemModel(this);
    ui->setupUi(this);
    Init();
}

alarmShieldDlg::~alarmShieldDlg()
{

}

QPushButton* alarmShieldDlg::GetDisAlarmShieldBtnPtr()
{
    return ui->dis_shiel_btn;
}

void alarmShieldDlg::Init()
{
    // 设置标题
    SetTitleName(tr("屏蔽列表"));
    // 初始化表格
    ui->tableView->setModel(m_tabViewMode);
    m_tabViewMode->setHorizontalHeaderLabels({tr("模块"), tr("序列号"), tr("设备型号"), tr("报警代码"), tr("报警级别"), tr("报警名称")});
    ui->tableView->hideColumn(COLUNM_DEV_SN);
    ui->tableView->hideColumn(COLUNM_DEV_TYPE);
	ui->tableView->setColumnWidth(COLUNM_DEV_NAME, 140);
	ui->tableView->horizontalHeader()->setMinimumSectionSize(100);
	ui->tableView->horizontalHeader()->setStretchLastSection(true);

    // 解除报警屏蔽
    connect(ui->dis_shiel_btn, SIGNAL(clicked()), this, SLOT(OnDisAlarmShield()));
    // 表格选中行改变
    // 选中行改变
    connect(ui->tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [&]
    {
        ui->dis_shiel_btn->setEnabled(ui->tableView->selectionModel()->hasSelection());
    });

    // 注册事件处理对象
    ui->tableView->viewport()->installEventFilter(this);

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

void alarmShieldDlg::showEvent(QShowEvent *event)
{
    BaseDlg::showEvent(event);
    
    ui->dis_shiel_btn->setEnabled(ui->tableView->selectionModel()->hasSelection());
}

bool alarmShieldDlg::eventFilter(QObject* target, QEvent* event)
{
    if (event->type() == QEvent::ToolTip)
    {
        QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
        if (helpEvent != Q_NULLPTR)
        {
            auto index = ui->tableView->indexAt(helpEvent->pos());
            if (index.isValid())
            {
                auto rawData = ui->tableView->model()->data(index, Qt::DisplayRole);
                QString tipContent = rawData.toString();
                QToolTip::showText(helpEvent->globalPos(), tipContent);
            }
        }
    }

    return BaseDlg::eventFilter(target, event);
}
 
void alarmShieldDlg::LoadCodeToview(const AlarmShieldInfo& asi)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    m_tabViewMode->removeRows(0,m_tabViewMode->rowCount());

    int iRow = 0;
    for (auto iter : asi.mapCodeAndDev)
    {
        for (auto asiItem : iter.second)
        {
            QString name(tr("控制单元"));

            const auto& devInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(asiItem.devSn);
            if (devInfo == nullptr && asiItem.devSn != CONTROL_UNIT_NAME)
            {
                ULOG(LOG_ERROR, "GetDeviceInfo Failed By SN:%s", asiItem.devSn);
                return;
            }

            if (devInfo != nullptr)
            {
                name = QString::fromStdString(devInfo->groupName + devInfo->name);
            }

            if (asiItem.devType >= ::tf::DeviceType::DEVICE_TYPE_TRACK)
            {
                name += QString::fromStdString("-" + asiItem.subName);
            }

            // 获取报警对应的标题
            auto title = DcsControlProxy::GetInstance()->GetAlarmTitleByCode((::tf::DeviceType::type)asiItem.devType, iter.first);
            AddTextToTableView(m_tabViewMode, iRow, COLUNM_DEV_NAME, name);
            AddTextToTableView(m_tabViewMode, iRow, COLUNM_DEV_SN, QString::fromStdString(asiItem.devSn));
            AddTextToTableView(m_tabViewMode, iRow, COLUNM_DEV_TYPE, QString::number(asiItem.devType));
            AddTextToTableView(m_tabViewMode, iRow, COLUNM_LEVEL, tr("注意"));
            AddTextToTableView(m_tabViewMode, iRow, COLUNM_ALARM_NAME, QString::fromStdString(title));

            // 报警码补0
            QStringList tempList = QString::fromStdString(iter.first).split("-");
            QString codeStr = UiCommon::AlarmCodeToString(tempList[0].toInt(), tempList[1].toInt(), tempList[2].toInt());
            AddTextToTableView(m_tabViewMode, iRow, COLUNM_CODE, codeStr);

            iRow++;
        }
    }
}

AlarmShieldInfo alarmShieldDlg::GetDisAlarmShieldCodeMap()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    AlarmShieldInfo asi;

    for (auto& rowIndex : ui->tableView->selectionModel()->selectedRows())
    {
        int currentRow = rowIndex.row();
        std::string DevSn = m_tabViewMode->data(m_tabViewMode->index(currentRow, COLUNM_DEV_SN)).toString().toStdString();

        // 获取报警码
        QString QStrCode = m_tabViewMode->data(m_tabViewMode->index(currentRow, COLUNM_CODE)).toString();

        // 获取设备类型
        int devType = m_tabViewMode->data(m_tabViewMode->index(currentRow, COLUNM_DEV_TYPE)).toInt();

        // 获取设备名称
        QString name;
        if (devType > ::tf::DeviceType::DEVICE_TYPE_TRACK)
        {
            name = m_tabViewMode->data(m_tabViewMode->index(currentRow, COLUNM_DEV_NAME)).toString();
            name.remove(QString::fromStdString(CommonInformationManager::GetInstance()->GetDeviceName(DevSn)) + "-");
        }


        QStringList tempList = QStrCode.split("-");
        if (tempList.size() != 3)
        {
            continue;
        }

        // 去除报警码的0
        QString codeTemp = QString::number(tempList[0].toInt()) + QString::number(tempList[1].toInt()) + QString::number(tempList[2].toInt());
        std::string code = codeTemp.toStdString();

        // 组装数据
        if (asi.mapCodeAndDev.find(code) == asi.mapCodeAndDev.end())
        {
            asi.mapCodeAndDev[code] = { AlarmShieldInfoItem(DevSn, devType, name.toStdString()) };
        }
        else
        {
            asi.mapCodeAndDev[code].insert(AlarmShieldInfoItem(DevSn, devType, name.toStdString()));
        }
    }
    return asi;
}

void alarmShieldDlg::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();
    // 解除报警屏蔽按钮
    ui->dis_shiel_btn->setVisible(userPms->IsPermissionShow(PSM_IM_GENERAL_ALARMUNSHILED));
}
