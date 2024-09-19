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
/// @file     sensorstatedlg.cpp
/// @brief    传感器状态对话框
///
/// @author   4170/TangChuXian
/// @date     2024年2月22日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2024年2月22日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "sensorstatedlg.h"
#include "ui_sensorstatedlg.h"
#include "shared/uicommon.h"
#include "shared/tipdlg.h"
#include "shared/basedlg.h"
#include "shared/uidef.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "src/common/Mlog/mlog.h"
#include "src/public/DictionaryKeyName.h"
#include "src/dcs/interface/track/TrackLogicControlHandler.h"
#include "uidcsadapter/uidcsadapter.h"
#include "manager/DictionaryQueryManager.h"
#include "thrift/DcsControlProxy.h"
#include "iBITDebugger.h"
#include <QRadioButton>

SensorStateDlg::SensorStateDlg(QWidget *parent)
    : BaseDlg(parent),
      m_bInit(false),
	  m_bIsMonitor(false),
      m_ciDefaultRowCnt(20),
      m_ciDefaultColCnt(5)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化Ui对象
    ui = new Ui::SensorStateDlg();
    ui->setupUi(this);


    // 界面显示前初始化
    InitBeforeShow();
}

SensorStateDlg::~SensorStateDlg()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    delete ui;
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2024年2月22日，新建函数
///
void SensorStateDlg::InitBeforeShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 设置表格行列数
    ui->SensorStateTbl->setRowCount(m_ciDefaultRowCnt);
    ui->SensorStateTbl->setColumnCount(m_ciDefaultColCnt);

    // 设置表头
    // 设置表头名称, 和枚举值顺序对应
    std::vector<std::pair<SensorStateHeader, QString>> ssStateHeader = {
        { Ssh_Module, tr("模块") },
        { Ssh_FuncModule, tr("功能模块") },
        { Ssh_SensorNameOrTrackMd, tr("传感器名称/轨道模块") },
        { Ssh_SensorFunction, tr("传感器功能") },
        { Ssh_SensorState, tr("传感器状态") }
    };
    QStringList strHeaderList;
    for (auto it = ssStateHeader.begin(); it != ssStateHeader.end(); ++it)
    {
        strHeaderList.append(it->second);
    }
    ui->SensorStateTbl->setHorizontalHeaderLabels(strHeaderList);

    // 隐藏垂直表头
    ui->SensorStateTbl->verticalHeader()->setVisible(false);

    // 表格列宽自适应
    //ResizeTblColToContent(ui->SensorStateTbl);
	ui->SensorStateTbl->setColumnWidth(0, 90);
	ui->SensorStateTbl->setColumnWidth(1, 315);
	ui->SensorStateTbl->setColumnWidth(2, 315);
	ui->SensorStateTbl->setColumnWidth(3, 435);
	ui->SensorStateTbl->horizontalHeader()->setMinimumSectionSize(90);

    // 根据设备列表创建单选框
    QHBoxLayout* pHlayout = new QHBoxLayout(ui->DevListFrame);
    pHlayout->setMargin(0);
    pHlayout->setSpacing(10);
    pHlayout->addStretch(1);

    // 获取设备列表
	if (!DcsControlProxy::GetInstance()->LoadAllSubDeviceInfo(m_listSubDevice))
	{
		ULOG(LOG_ERROR, "%s(), GetDevNameList() failed", __FUNCTION__);
		return;
	}

	bool bPipeLine = DictionaryQueryManager::GetInstance()->GetPipeLine();
    // 依次添加具体设备
    QRadioButton* pFirstRBtn = Q_NULLPTR;
    for (const auto& deviceInfo : m_listSubDevice)
    {
		if (!bPipeLine && deviceInfo.deviceType == tf::DeviceType::DEVICE_TYPE_TRACK)
		{
			continue;
		}
        QString strCombineName = QString::fromStdString(deviceInfo.deviceName);

        QRadioButton* pRBtn = new QRadioButton(strCombineName);
		if (pFirstRBtn == Q_NULLPTR)
		{
			m_strCurDevInfo = deviceInfo;
			pRBtn->setChecked(true);
			pFirstRBtn = pRBtn;
		}

        connect(pRBtn, SIGNAL(clicked()), this, SLOT(OnDevRBtnClicked()));
        pHlayout->addWidget(pRBtn); 
    }

    // 第一个按钮检查
    if (pFirstRBtn == Q_NULLPTR)
    {
        ULOG(LOG_INFO, "%s(), device Empty", __FUNCTION__);
        return;
    }

    // 如果仅一个设备，不允许点击
    if (m_listSubDevice.size() <= 1)
    {
        pFirstRBtn->setEnabled(false);
    }
}

///
/// @brief
///     界面显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2024年2月22日，新建函数
///
void SensorStateDlg::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化字符串资源
    InitStrResource();

    // 初始化子控件
    InitChildCtrl();

    // 初始化信号槽连接
    InitConnect();
}

///
/// @brief
///     初始化字符串资源
///
/// @par History:
/// @li 4170/TangChuXian，2022年1月17日，新建函数
///
void SensorStateDlg::InitStrResource()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 4170/TangChuXian，2024年2月22日，新建函数
///
void SensorStateDlg::InitConnect()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 关闭按钮被点击
    connect(ui->CloseBtn, SIGNAL(clicked()), this, SLOT(OnCloseBtnClicked()));

    // 开始监测按钮被点击
    connect(ui->StartWatchBtn, SIGNAL(clicked()), this, SLOT(OnStartWatchBtnClicked()));

    // 停止监测按钮被点击
    connect(ui->StopWatchBtn, SIGNAL(clicked()), this, SLOT(OnStopWatchBtnClicked()));

    // 监听执行结果
    REGISTER_HANDLER(MSG_ID_SENSOR_STATUS_UPDATE, this, OnSensorStateUpdate);
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2024年2月22日，新建函数
///
void SensorStateDlg::InitChildCtrl()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 设置标题
    SetTitleName(tr("传感器状态"));

    // 表格设置
    ui->SensorStateTbl->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->SensorStateTbl->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->SensorStateTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 更新调试流程表
    UpdateSensorStateTbl();

//     QStringList strHeaderList;
//     strHeaderList << tr("序号") << tr("登录名") << tr("用户名") << tr("用户权限") << tr("自动登录");
//     ui->UserTbl->setHorizontalHeaderLabels(strHeaderList);
//     ui->UserTbl->verticalHeader()->setVisible(false);
// 
//     // 设置表格选中模式为行选中，不可多选
//     ui->UserTbl->setSelectionBehavior(QAbstractItemView::SelectRows);
//     ui->UserTbl->setSelectionMode(QAbstractItemView::SingleSelection);
//     ui->UserTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 列宽设置
    //ui->UserTbl->setColumnWidth(0, 200);

    // 设置表格列宽比
//     QVector<double> vScale;
//     vScale << 1.0 << 1.0 << 1.0 << 1.0 << 1.0 << 1.0;
//     SetTblColWidthScale(ui->DefaultQcTbl, vScale);


    // 设置绝对列宽
//     QVector<double> vColWidth;
//     vColWidth << 162.0 << 160.0 << 161.0 << 160.0 << 161.0 << 131.0;
//     SetTblColAbsWidthScale(ui->DefaultQcTbl, vColWidth);

    // 更新用户信息表
    //UpdateUserInfoTbl();
}

///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月12日，新建函数
///
void SensorStateDlg::showEvent(QShowEvent *event)
{
    // 调用基类接口
    BaseDlg::showEvent(event);

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }

	// 设置轨道模块调试器
	std::shared_ptr<track::LogicControlHandler> spTrackHandler = ::track::LogicControlHandler::GetInstance();
	spTrackHandler->SetBITDebugger(_bitDebugger);
}

void SensorStateDlg::OnCloseBtnClicked()
{
	// 获取设备序列号
	if (m_bIsMonitor)
	{
		OnStopWatchBtnClicked();
	}

	//this->close();
	this->reject();
}

///
/// @brief
///     开始监测按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月29日，新建函数
///
void SensorStateDlg::OnStartWatchBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 先停止之前的监控
	if (m_bIsMonitor)
	{
		OnStopWatchBtnClicked();
	}

	// 判断仪器状态
	// 获取仪器状态，如果未连接，则是下电状态，否则根据遮蔽状态显示
	// 执行查询
	// 查询当前选中项设备状态
	tf::DeviceInfoQueryResp devInfoResp;
	tf::DeviceInfoQueryCond devInfoCond;
	devInfoCond.__set_deviceSN(m_strCurDevInfo.deviceSN);
	if (!DcsControlProxy::GetInstance()->QueryDeviceInfo(devInfoResp, devInfoCond))
	{
		ULOG(LOG_WARN, "QueryDeviceInfo failed!");
		return;
	}

	if (devInfoResp.lstDeviceInfos.empty())
	{
		ULOG(LOG_WARN, "devInfoResp.lstDeviceInfos.empty()");
		return;
	}

	// 获取选中设备信息
	tf::DeviceInfo& deviceInfo = devInfoResp.lstDeviceInfos[0];

	// 如果仪器处于待机/预热/停机状态才能开启传感器监控
	if ((::tf::DeviceWorkState::DEVICE_STATUS_STANDBY != deviceInfo.status) &&
		(::tf::DeviceWorkState::DEVICE_STATUS_WARMUP != deviceInfo.status) &&
		(::tf::DeviceWorkState::DEVICE_STATUS_HALT != deviceInfo.status))
	{
		TipDlg(QObject::tr("请在仪器处于待机、维护（预热）或停机状态进行该操作！")).exec();
		return;
	}

	// 执行命令
	if (!DcsControlProxy::GetInstance()->StartSensorMonitor(m_strCurDevInfo))
    {
        ULOG(LOG_ERROR, "%s(), StartSensorMonitor() failed", __FUNCTION__);
        return;
    }

	m_bIsMonitor = true;
    // 按钮更新使能
    ui->StopWatchBtn->setEnabled(true);
    ui->StartWatchBtn->setEnabled(false);
}

///
/// @brief
///     停止监测按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月29日，新建函数
///
void SensorStateDlg::OnStopWatchBtnClicked()
{
    // 执行命令
    if (!DcsControlProxy::GetInstance()->StopSensorMonitor(m_strCurDevInfo))
    {
        ULOG(LOG_ERROR, "%s(), StartSensorMonitor() failed", __FUNCTION__);
        return;
    }

	m_bIsMonitor = false;
    // 按钮更新使能
    ui->StopWatchBtn->setEnabled(false);
    ui->StartWatchBtn->setEnabled(true);
}

///
/// @brief
///     设备单选框被点击
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月28日，新建函数
///
void SensorStateDlg::OnDevRBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取信号发送者
    QRadioButton* pRBtn = qobject_cast<QRadioButton*>(sender());
    if (pRBtn == Q_NULLPTR)
    {
        return;
    }

	// 先停止之前的监控
	if (m_bIsMonitor)
	{
		OnStopWatchBtnClicked();
	}

    // 如果更新当前设备名
    QString strDevName = pRBtn->text();
    for (const auto& devInfo : m_listSubDevice)
    {
        QString strCombineName = QString::fromStdString(devInfo.deviceName);

        if (strDevName != strCombineName)
        {
            continue;
        }

        m_strCurDevInfo = devInfo;
        break;
    }

    // 刷新表
    UpdateSensorStateTbl();
}

///
/// @brief
///     更新传感器状态表
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月28日，新建函数
///
void SensorStateDlg::UpdateSensorStateTbl()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 加载传感器状态
    std::vector< ::tf::ModuleSensorInfo> vSensorInfo;
    if (!DcsControlProxy::GetInstance()->LoadSensorInfo(vSensorInfo, m_strCurDevInfo))
    {
        ULOG(LOG_ERROR, "%s(), LoadSensorInfo() failed", __FUNCTION__);
        return;
    }

	if (vSensorInfo.empty())
	{
		ui->StopWatchBtn->setEnabled(false);
		ui->StartWatchBtn->setEnabled(false);
	}
	else
	{
		ui->StopWatchBtn->setEnabled(false);
		ui->StartWatchBtn->setEnabled(true);
	}
    // 清空内容
    ui->SensorStateTbl->setRowCount(0);

    // 更新表格
    int iRow = 0;
    for (const ::tf::ModuleSensorInfo& stuModuleInfo : vSensorInfo)
    {
        // 行自增
        if (iRow >= ui->SensorStateTbl->rowCount())
        {
            ui->SensorStateTbl->setRowCount(iRow + 1);
        }

        // 模块名
        ui->SensorStateTbl->setItem(iRow, 0, new QTableWidgetItem(QString::fromStdString(stuModuleInfo.muduleName)));

        // 遍历传感器
        int iModuleStartRow = iRow;
        for (const tf::SensorInfo& stuSensorInfo : stuModuleInfo.sensorInfo)
        {
            // 行自增
            if (iRow >= ui->SensorStateTbl->rowCount())
            {
                ui->SensorStateTbl->setRowCount(iRow + 1);
            }

            // 模块名
            ui->SensorStateTbl->setItem(iRow, 1, new QTableWidgetItem(QString::fromStdString(stuSensorInfo.funBlock)));

            // 传感器状态信息
            int iFuncModuleStartRow = iRow;
            for (const tf::SubSensorInfo& stuSubSensorInfo : stuSensorInfo.sensorStatusInfos)
            {
                // 行自增
                if (iRow >= ui->SensorStateTbl->rowCount())
                {
                    ui->SensorStateTbl->setRowCount(iRow + 1);
                }

                // 传感器内容
                ui->SensorStateTbl->setItem(iRow, 2, new QTableWidgetItem(QString::fromStdString(stuSubSensorInfo.sensorName)));
                ui->SensorStateTbl->setItem(iRow, 3, new QTableWidgetItem(QString::fromStdString(stuSubSensorInfo.sensorFunction)));
                ui->SensorStateTbl->setItem(iRow, 4, new QTableWidgetItem(""));

                // 记录ID
                ui->SensorStateTbl->item(iRow, Ssh_SensorState)->setData(Qt::UserRole, stuSubSensorInfo.key);

                // 行自增
                ++iRow;
            }

            // 设置行合并
            ui->SensorStateTbl->setSpan(iFuncModuleStartRow, 1, iRow - iFuncModuleStartRow, 1);
        }

        // 设置行合并
        ui->SensorStateTbl->setSpan(iModuleStartRow, 0, iRow - iModuleStartRow, 1);
    }

    // 更新状态
    UpdateTblSensorStatus();

    // 列宽自适应
    //ResizeTblColToContent(ui->SensorStateTbl);
}

///
/// @brief
///     传感器状态更新
///
/// @param[in]  strDevSn    设备序列号
/// @param[in]  mapStatus   传感器状态
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月29日，新建函数
///
void SensorStateDlg::OnSensorStateUpdate(const tf::SubDeviceInfo deviceInfo, QMap<int, tf::EmSensorResultType::type> mapStatus)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 更新缓存
	QString strName = QString::fromStdString(deviceInfo.deviceName);
    auto it = m_mapStatusBuffer.find(strName);
    if (it == m_mapStatusBuffer.end())
    {
        it = m_mapStatusBuffer.insert(strName, mapStatus);
    }
    else
    {
        for (auto itStatus = mapStatus.begin(); itStatus != mapStatus.end(); itStatus++)
        {
            it.value().insert(itStatus.key(), itStatus.value());
        }
    }

    // 更新表格传感器状态
    UpdateTblSensorStatus();
}

///
/// @brief
///     更新表格传感器状态
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月29日，新建函数
///
void SensorStateDlg::UpdateTblSensorStatus()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取设备序列号
	QString strName = QString::fromStdString(m_strCurDevInfo.deviceName);
    // 查找对应的索引
    auto it = m_mapStatusBuffer.find(strName);
    if (it == m_mapStatusBuffer.end())
    {
        ULOG(LOG_INFO, "%s(), ignore", __FUNCTION__);
        return;
    }

    // 遍历表格
    for (int iRow = 0; iRow < ui->SensorStateTbl->rowCount(); iRow++)
    {
        // 获取状态项
        QTableWidgetItem* pItem = ui->SensorStateTbl->item(iRow, Ssh_SensorState);
        if (pItem == Q_NULLPTR)
        {
            continue;
        }

        // 获取索引
        int iIdx = pItem->data(Qt::UserRole).toInt();
        auto itStatus = it.value().find(iIdx);
        if (itStatus == it.value().end())
        {
            continue;
        }

        // 将状态转换为字符串
        pItem->setText(ConvertTfEnumToQString(itStatus.value()));
    }
}
