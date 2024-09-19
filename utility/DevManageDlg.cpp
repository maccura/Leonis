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
/// @file     DevManageDlg.cpp
/// @brief    应用--系统--设备管理
///
/// @author   7951/LuoXin
/// @date     2022年9月30日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年9月30日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "DevManageDlg.h"
#include "ui_DevManageDlg.h" 
#include <QCheckBox>
#include <QStandardItemModel>
#include "thrift/DcsControlProxy.h"

#include "shared/CReadOnlyDelegate.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/uidef.h"
#include "shared/uicommon.h"
#include "shared/tipdlg.h"
#include "shared/CommonInformationManager.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"
#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"

enum DATA_COLUNM			// 报警信息在表格中的对应列
{
	COLUNM_CHECKBOX,		// 勾选框
	COLUNM_NUM,				// 序号
	COLUNM_SYSTEM,			// 系统
	COLUNM_MODEL,			// 模块
	COLUNM_TYPE,			// 型号
	COLUNM_IP,				// IP
	COLUNM_PORT,			// PORT
    COLUNM_MASK,			// 屏蔽状态
	COLUNM_POWER,			// 供电状态
	COLUNM_ID,				// 数据库主键
    COLUNM_SN				// 设备序列号
};

DevManageDlg::DevManageDlg(QWidget *parent)
	: BaseDlg(parent)
	, ui(new Ui::DevManageDlg)
	, m_checkBox(new QCheckBox(this))
	, m_tabViewMode(new QStandardItemModel)
    , m_powerOffStr(tr("下电"))
    , m_powerOnStr(tr("上电"))
    , m_ImReagLoadRuning{false}
{
	ui->setupUi(this);

	InitCtrls();
}

DevManageDlg::~DevManageDlg()
{

}

void DevManageDlg::LoadDataToCtrls()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 清空所有行
	m_tabViewMode->removeRows(0,m_tabViewMode->rowCount());

	::tf::DeviceInfoQueryResp DeviceResp;
	::tf::DeviceInfoQueryCond QueryDeviceCond;

	// 查询设备信息
	if (!DcsControlProxy::GetInstance()->QueryDeviceInfo(DeviceResp, QueryDeviceCond)
		|| DeviceResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
		|| DeviceResp.lstDeviceInfos.size() <= 0)
	{
		ULOG(LOG_ERROR, "%s() : GET device information failed", __FUNCTION__);
		return;
	}

	// 依次显示设备信息
	int currentRow = 0;
    auto CIM = CommonInformationManager::GetInstance();
    for (auto& device : DeviceResp.lstDeviceInfos)
    {
        // 设备管理不显示轨道设备，bug10833
        if (device.deviceType == ::tf::DeviceType::DEVICE_TYPE_TRACK)
        {
            continue;
        }

        if (device.commParam.connectParam.net.ip.empty())
        {
            ULOG(LOG_ERROR, "%s() : IP NULL", __FUNCTION__);
        }

        // 序号
        AddTextToTableView(m_tabViewMode, currentRow, COLUNM_NUM, QString::number(currentRow + 1))
        // 系统
        AddTextToTableView(m_tabViewMode, currentRow, COLUNM_SYSTEM, ConvertTfEnumToQString(device.deviceClassify))
        // 模块名称
        AddTextToTableView(m_tabViewMode, currentRow, COLUNM_MODEL, QString::fromStdString(device.groupName + device.name))       
        // IP
        AddTextToTableView(m_tabViewMode, currentRow, COLUNM_IP, QString::fromStdString(device.commParam.connectParam.net.ip))
        // 端口
        AddTextToTableView(m_tabViewMode, currentRow, COLUNM_PORT, QString::number(device.commParam.connectParam.net.port))
        // 屏蔽状态
        AddTextToTableView(m_tabViewMode, currentRow, COLUNM_MASK, device.masked ? tr("屏蔽") : tr("未屏蔽"))
        // 数据库主键
        AddTextToTableView(m_tabViewMode, currentRow, COLUNM_ID, QString::number(device.id))
        // 数据库主键
        AddTextToTableView(m_tabViewMode,currentRow, COLUNM_SN, QString::fromStdString(device.deviceSN))

        // 型号
        QStandardItem*si = new QStandardItem(((device.deviceType == tf::DeviceType::DEVICE_TYPE_C1000)
                ? ConvertTfEnumToQString(device.subDeviceType) : ConvertTfEnumToQString(device.deviceType)));
        si->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
        si->setData((int)device.subDeviceType, Qt::UserRole + 10);
        m_tabViewMode->setItem(currentRow, COLUNM_TYPE, si);

        // 勾选框
        QWidget* wid = new QWidget(ui->tableView);
        wid->setMinimumSize(44,44);
        QCheckBox* box = new QCheckBox(wid);
        box->setMinimumSize(20, 20);
        box->move(6, 2);
        connect(box, &QCheckBox::toggled, this, &DevManageDlg::OnCheckBoxStatusChanged);
        ui->tableView->setIndexWidget(m_tabViewMode->index(currentRow, COLUNM_CHECKBOX), wid);

        //bug0020901缺陷修复，若为M1则默认勾选,仅免疫单机版使用
        bool isPipi = DictionaryQueryManager::GetInstance()->GetPipeLine();
        if (QString::fromStdString(device.groupName + device.name) == "M1" && !isPipi)
        {
            box->setChecked(true);
        }

        QStandardItem* statusItem = new QStandardItem;
        // 获取仪器状态，如果未连接，则是下电状态，否则根据遮蔽状态显示
        if (device.status == ::tf::DeviceWorkState::DEVICE_STATUS_DISCONNECT)
        {
            statusItem->setText(m_powerOffStr);
        }
        else
        {
            statusItem->setText(m_powerOnStr);
        }

        statusItem->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
        m_tabViewMode->setItem(currentRow, COLUNM_POWER, statusItem);

        // 行号自增
        currentRow++;
    }

    m_checkBox->setChecked(false);
    OnCheckBoxStatusChanged(false);

    if (currentRow > 0 && !ui->tableView->selectionModel()->hasSelection())
    {
        ui->tableView->selectRow(0);
    }
}

void DevManageDlg::InitCtrls()
{
	// 设置标题
	SetTitleName(tr("设备管理"));

	// 初始化表格
	ui->tableView->setModel(m_tabViewMode);

	// 清空页面
	m_tabViewMode->clear();

	// 重新设置页面
	QStringList alarmHeaderListString;
	alarmHeaderListString << "" << tr("序号") << tr("系统") << tr("模块名称")
		<< tr("型号") << tr("IP地址") << tr("端口") << tr("屏蔽状态") << tr("供电状态") << "id" << "sn";
	m_tabViewMode->setHorizontalHeaderLabels(alarmHeaderListString);

	// 隐藏列
	ui->tableView->hideColumn(COLUNM_ID);
    ui->tableView->hideColumn(COLUNM_SN);

	// 表头加入勾选框
    m_checkBox->setMinimumSize(20, 20);
	UiCommon::Instance()->AddCheckboxToTableView(ui->tableView, m_checkBox);

    ui->tableView->setColumnWidth(COLUNM_CHECKBOX,43);
	ui->tableView->horizontalHeader()->setSectionResizeMode(COLUNM_CHECKBOX, QHeaderView::Fixed);
    ui->tableView->setColumnWidth(COLUNM_NUM, 80);
	ui->tableView->setColumnWidth(COLUNM_SYSTEM, 220);
	ui->tableView->setColumnWidth(COLUNM_MODEL, 160);
	ui->tableView->setColumnWidth(COLUNM_TYPE, 160);
	ui->tableView->setColumnWidth(COLUNM_IP, 200); 
	ui->tableView->setColumnWidth(COLUNM_MASK, 170);
	ui->tableView->setColumnWidth(COLUNM_POWER, 180);
	ui->tableView->horizontalHeader()->setStretchLastSection(true);

    connect(ui->power_btn, &QPushButton::clicked, this, &DevManageDlg::OnPowerDownBtnClicked);
    connect(ui->mask_btn, &QPushButton::clicked, this, &DevManageDlg::OnMaskBtnClicked);
    connect(ui->tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DevManageDlg::OnSelectRowChanged);
    connect(ui->front_btn, &QPushButton::clicked, this, [&] { UnlockFlap(tf::EnumFlapType::EM_FLAP_FRONT); });
    connect(ui->back_btn, &QPushButton::clicked, this, [&] { UnlockFlap(tf::EnumFlapType::EM_FLAP_BACK); });
    connect(ui->tableView, &QTableView::doubleClicked, this, &DevManageDlg::OntableViewDoubleClicked);

    // 注册设备状态改变处理槽函数
    REGISTER_HANDLER(MSG_ID_DEVS_STATUS_CHANGED, this, OnDevStateChange);
    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
    // 免疫试剂加卸载状态更新
    REGISTER_HANDLER(MSG_ID_REAGENT_LOAD_STATUS_UPDATE, this, OnRgntLoadStatusChanged);
}

void DevManageDlg::UnlockFlap(int type)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 遍历所有行，获得勾选的设置
    std::vector<std::string> devs;
    for (int i = 0; i < m_tabViewMode->rowCount(); i++)
    {
        // 修改选中行的状态
        QWidget* wid = qobject_cast<QWidget*>(ui->tableView->indexWidget(m_tabViewMode->index(i, COLUNM_CHECKBOX)));
        QCheckBox* box = wid->findChild<QCheckBox*>();
        if (box->isChecked())
        {
            // 只能在待机或者停机的状态下解锁翻盖，bug0025903
            std::string sn = m_tabViewMode->data(m_tabViewMode->index(i, COLUNM_SN)).toString().toStdString();
            auto devInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(sn);
            if (devInfo == nullptr)
            {
                TipDlg(tr("解锁失败！")).exec();
                return;
            }

            //免疫后翻盖非停止状态不能解锁
            if (devInfo->deviceType == tf::DeviceType::DEVICE_TYPE_I6000)
            {
                if (type == 2)
                {
                    if (devInfo->status != ::tf::DeviceWorkState::DEVICE_STATUS_HALT)
                    {
                        TipDlg(tr("仪器[%1]不是非停止状态，不能解锁设置的翻盖")
                            .arg(QString::fromStdString(devInfo->groupName + devInfo->name))).exec();
                        return;
                    }
                }
            }
            // 电解质不能解锁翻盖bug:14408
            else if (devInfo->deviceType == tf::DeviceType::DEVICE_TYPE_ISE1005)
            {
                TipDlg(tr("电解质设备不能通过软件解锁设备翻盖")).exec();
                return;
            }
            else if(devInfo->status != ::tf::DeviceWorkState::DEVICE_STATUS_HALT
                && devInfo->status != ::tf::DeviceWorkState::DEVICE_STATUS_STANDBY)
            {
                TipDlg(tr("仪器[%1]不是非停止状态，不能解锁设置的翻盖")
                    .arg(QString::fromStdString(devInfo->groupName + devInfo->name))).exec();
                return;
            }
            //免疫前翻盖正在执行试剂加卸载不能解锁
            if (devInfo->deviceType == tf::DeviceType::DEVICE_TYPE_I6000)
            {
                if (type == 1)
                {
                    if (m_ImReagLoadRuning)
                    {
                        TipDlg(tr("仪器[%1]正在进行试剂加卸载，不能解锁设置的翻盖")
                            .arg(QString::fromStdString(devInfo->groupName + devInfo->name))).exec();
                        return;
                    }
                }
            }
            //查询当前是否存在试剂加卸载动作
            else if (m_ImReagLoadRuning)
            {
                TipDlg(tr("仪器[%1]正在进行试剂加卸载，不能解锁设置的翻盖")
                    .arg(QString::fromStdString(devInfo->groupName + devInfo->name))).exec();
                return;
            }
            devs.push_back(sn);
        }
    }

    // 检查状态都是可解锁翻盖后，所有勾选框设置未勾选
    for (int i = 0; i < m_tabViewMode->rowCount(); i++)
    {
        // 修改选中行的状态
        QWidget* wid = qobject_cast<QWidget*>(ui->tableView->indexWidget(m_tabViewMode->index(i, COLUNM_CHECKBOX)));
        QCheckBox* box = wid->findChild<QCheckBox*>();
        box->setChecked(false);
    }

     // 执行操作
    DcsControlProxy::GetInstance()->UnlockFlap(devs, (tf::EnumFlapType::type)type);

    // 显示结果
    QString tipStr;
    if (type == tf::EnumFlapType::EM_FLAP_FRONT)
    {
        tipStr = tr("前翻盖已解锁！");
    }
    else
    {
        tipStr = tr("后翻盖已解锁！");
    }

    QCustomDialog tipDlg(this, 1000, tipStr);
    tipDlg.move(tipDlg.x() - 105, tipDlg.y() - 40);
    tipDlg.exec();
}

///
/// @brief
///     试剂仓加卸载是否正在运行
///
/// @param[in]  strDevSn  设备序列号
/// @param[in]  bRunning  是否正在运行
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月30日，新建函数
///
void DevManageDlg::OnRgntLoadStatusChanged(QString strDevSn, bool bRunning)
{
    ULOG(LOG_INFO, "%s(%s,%d)", __FUNCTION__, strDevSn.toStdString().c_str(), int(bRunning));
    m_ImReagLoadRuning = bRunning;
}

void DevManageDlg::OnPowerDownBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 缓存上下电标志
    bool isPowerOff = ui->power_btn->text() == m_powerOffStr;

    // 遍历所有行
    QVector<QString> devSnVec;
    for (int i = 0; i < m_tabViewMode->rowCount(); i++)
    {
        // 修改选中行的状态
        QWidget* wid = qobject_cast<QWidget*>(ui->tableView->indexWidget(m_tabViewMode->index(i, COLUNM_CHECKBOX)));
        QCheckBox* box = wid->findChild<QCheckBox*>();
        if (box->isChecked())
        {
            devSnVec.push_back(m_tabViewMode->data(m_tabViewMode->index(i, COLUNM_SN)).toString());
        }
    }

    // 查询当前选中项设备状态
    tf::DeviceInfoQueryResp devInfoResp;
    tf::DeviceInfoQueryCond devInfoCond;

    // 执行查询
    if (!DcsControlProxy::GetInstance()->QueryDeviceInfo(devInfoResp, devInfoCond))
    {
        ULOG(LOG_WARN, "QueryDeviceInfo failed!");
        return;
    }

    // 获取选中设备信息
    QString statusErr;
    QMap<QString, QString> canOptDev2BindDevs;
    std::vector<std::string> opertDevs;
    for (const auto& stuTfDevInfo : devInfoResp.lstDeviceInfos)
    {
        // 对比设备序列号，查看是否选中设备
        if (!devSnVec.contains(QString::fromStdString(stuTfDevInfo.deviceSN)))
        {
            continue;
        }

        // 检查是否含有ISE设备
        if (stuTfDevInfo.deviceType == ::tf::DeviceType::DEVICE_TYPE_ISE1005
            || stuTfDevInfo.deviceType == ::tf::DeviceType::DEVICE_TYPE_C1000)
        {
            auto spDev = CommonInformationManager::GetInstance()->GetBindDeviceInfoBySn(stuTfDevInfo.deviceSN);
            if (spDev != nullptr && !devSnVec.contains(QString::fromStdString(spDev->deviceSN)))
            {
                canOptDev2BindDevs[QString::fromStdString(stuTfDevInfo.groupName + stuTfDevInfo.name)]
                    = QString::fromStdString(spDev->groupName + spDev->name);
                continue;
            }
        }

        // 设备运行、维护不能下电。断开连接才能上电
        if ((isPowerOff && (devIsRun(stuTfDevInfo) ||
             ::tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN == stuTfDevInfo.status))
            || (!isPowerOff && ::tf::DeviceWorkState::DEVICE_STATUS_DISCONNECT != stuTfDevInfo.status))
        {
            if (!statusErr.isEmpty())
            {
                statusErr += "、";
            }

            statusErr += QString::fromStdString(stuTfDevInfo.groupName + stuTfDevInfo.name);
            continue;
        }

        opertDevs.push_back(stuTfDevInfo.deviceSN);
    }

    QString errInfo;
    QString operStr = isPowerOff ? tr("下电") : tr("上电");

    // 提示绑定设备未勾选不能下电
    for (auto iter = canOptDev2BindDevs.begin();  iter != canOptDev2BindDevs.end(); iter++)
    {
        errInfo += tr("对设备[") + iter.key() + ("]") + operStr 
            + tr(",需同时勾选设备[") + iter.value() + ("]\n");
    }

    if (!statusErr.isEmpty())
    {
        errInfo += tr("设备[") + statusErr + ("]当前的状态不能") + operStr + "\n";
    }

    // 当前存在设备不能执行操作
    bool needTip = true;
    if (!errInfo.isEmpty())
    {
        if (opertDevs.empty())
        {
            // 所有的设备都不能操作，提示后返回
            TipDlg(tr("设备管理"), errInfo).exec();
            return;
        }
        else
        {
            // 部分的设备都不能操作，提示后用户可选择让其中可操作的设备执行
            if (TipDlg(tr("设备管理"), errInfo + tr("是否对其他设备执行操作")
                , TipDlgType::TWO_BUTTON).exec() == QDialog::Rejected)
            {
                return;
            }

            needTip = false;
        }
    }

    // 提示确认 0020260: [应用] 设备管理界面选择模块点击“屏蔽”按钮未弹出确认提示框，模块直接被屏蔽 add by chenjianlin 20230911
    if (needTip &&
        TipDlg(tr("设备管理"), isPowerOff ? tr("确定下电勾选的仪器？")
            : tr("确定上电勾选的仪器？"), TipDlgType::TWO_BUTTON).exec() == QDialog::Rejected)
    {
        return;
    }

    // 执行操作
    if (isPowerOff)
    {
        // 这里只是消息发送的结果，并不是真正开机或关机的结果。
        if (!DcsControlProxy::GetInstance()->DevicePowerOff(opertDevs))
        {
            TipDlg(tr("设备管理"), tr("下电失败")).exec();
        }
    }
    else
    {
        // 这里只是消息发送的结果，并不是真正开机或关机的结果。
        if (!DcsControlProxy::GetInstance()->DevicePowerOn(opertDevs))
        {
            TipDlg(tr("设备管理"), tr("上电失败")).exec();
        }
    }
}

void DevManageDlg::OnMaskBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 缓存屏蔽标志
    bool isMask = ui->mask_btn->text() == tr("屏蔽");

	// 遍历所有行
    std::map<int, std::string> rowAndSnMap;
	for (int i = 0; i < m_tabViewMode->rowCount(); i++)
	{
		// 修改选中行的状态
        QWidget* wid = qobject_cast<QWidget*>(ui->tableView->indexWidget(m_tabViewMode->index(i, COLUNM_CHECKBOX)));
        QCheckBox* box = wid->findChild<QCheckBox*>();
		if (box->isChecked())
		{
            rowAndSnMap[i] = m_tabViewMode->data(m_tabViewMode->index(i, COLUNM_SN)).toString().toStdString();
		}
	}

    // 2024年9月6日，与周锐讨论确定：仪器运行中不能屏蔽设备
    const auto& CIM = CIM_INSTANCE;
    for (const auto& iter : rowAndSnMap)
    {
        auto spDev = CIM->GetDeviceInfo(iter.second);
        if (spDev != nullptr && devIsRun(*spDev))
        {
            TipDlg(tr("设备管理"), tr("勾选的设备存在运行中的仪器，不能屏蔽？")).exec();
            return;
        }
    }

    // 提示确认 0020260: [应用] 设备管理界面选择模块点击“屏蔽”按钮未弹出确认提示框，模块直接被屏蔽 add by chenjianlin 20230911
    if (TipDlg(tr("设备管理"), isMask ? tr("确定屏蔽选中的仪器？") : tr("确定解除选中仪器的屏蔽状态？"), TipDlgType::TWO_BUTTON).exec() == QDialog::Rejected)
    {
        return;
    }

    // 屏蔽仪器
    for (const auto& iter : rowAndSnMap)
    {
        ::tf::DeviceInfo di;
        di.__set_deviceSN(iter.second);
        di.__set_masked(isMask);
        if (CommonInformationManager::GetInstance()->ModifyDeviceInfo(di))
        {
            m_tabViewMode->item(iter.first, COLUNM_MASK)->setText(isMask ? tr("屏蔽") : tr("未屏蔽"));
        }
    }

    OnCheckBoxStatusChanged(m_checkBox->isChecked());
}

void DevManageDlg::OnCheckBoxStatusChanged(bool checked)
{
    ui->mask_btn->setEnabled(true);
    ui->power_btn->setEnabled(true);

    // 遍历所有行
    int selectNum = 0;
    QString maskStatus;
    QString powerStatus;
    for (int i = 0; i < m_tabViewMode->rowCount(); i++)
    {
        // 修改选中行的状态
        QWidget* wid = qobject_cast<QWidget*>(ui->tableView->indexWidget(m_tabViewMode->index(i, COLUNM_CHECKBOX)));
        QCheckBox* box = wid->findChild<QCheckBox*>();
        if (box->isChecked())
        {
            // 设置屏蔽按钮的状态
            if (maskStatus.isEmpty())
            {
                maskStatus = m_tabViewMode->data(m_tabViewMode->index(i, COLUNM_MASK)).toString();
            }
            else
            {
                if (maskStatus != m_tabViewMode->data(m_tabViewMode->index(i, COLUNM_MASK)).toString())
                {
                    ui->mask_btn->setEnabled(false);
                } 
            }

            // 设置电源按钮的状态
            if (powerStatus.isEmpty())
            {
                powerStatus = m_tabViewMode->data(m_tabViewMode->index(i, COLUNM_POWER)).toString();
            }
            else
            {
                if (powerStatus != m_tabViewMode->data(m_tabViewMode->index(i, COLUNM_POWER)).toString())
                {
                    ui->power_btn->setEnabled(false);
                }
            }

            selectNum++;
        }
    }

    ui->power_btn->setText(powerStatus == m_powerOffStr ? m_powerOnStr : m_powerOffStr);
    ui->btn_widget->setEnabled(selectNum);
    m_checkBox->setChecked(m_tabViewMode->rowCount() == selectNum);
    ui->mask_btn->setText(maskStatus == tr("屏蔽") ? tr("解除屏蔽") : tr("屏蔽"));
}

void DevManageDlg::OnDevStateChange(tf::DeviceInfo deviceInfo)
{
    ULOG(LOG_INFO, "%s(%s)", __FUNCTION__, ToString(deviceInfo));

    // 过滤轨道设备
    if (deviceInfo.deviceType == tf::DeviceType::DEVICE_TYPE_TRACK)
    {
        return;
    }

    // 遍历所有行
    for (int i = 0; i < m_tabViewMode->rowCount(); i++)
    {
        // 更新设备的状态
        if (m_tabViewMode->data(m_tabViewMode->index(i, COLUNM_SN)).toString().toStdString() == deviceInfo.deviceSN)
        {
            // 获取仪器状态，如果未连接，则是下电状态，否则根据遮蔽状态显示
            if (deviceInfo.status == ::tf::DeviceWorkState::type::DEVICE_STATUS_DISCONNECT)
            {
                m_tabViewMode->item(i, COLUNM_POWER)->setText(m_powerOffStr);
            }
            else
            {
                m_tabViewMode->item(i, COLUNM_POWER)->setText(m_powerOnStr);
            }
        }
    }

    OnCheckBoxStatusChanged(m_checkBox->isChecked());
}

void DevManageDlg::OnSelectRowChanged()
{
    for (int row = 0; row < m_tabViewMode->rowCount(); row++)
    {
        QWidget* wid = qobject_cast<QWidget*>(ui->tableView->indexWidget(m_tabViewMode->index(row, COLUNM_CHECKBOX)));

        if (wid != nullptr)
        {
            wid->findChild<QCheckBox*>()->setChecked(false);
        }
    }

    auto rowList = ui->tableView->selectionModel()->selectedRows();
    for (auto rowIndex : rowList)
    {
        QWidget* wid = qobject_cast<QWidget*>(ui->tableView->indexWidget(m_tabViewMode->index(rowIndex.row(), COLUNM_CHECKBOX)));
        if (wid != nullptr)
        {
            wid->findChild<QCheckBox*>()->setChecked(true);
        }
    }

    m_checkBox->setChecked((rowList.size() == m_tabViewMode->rowCount()));      
}

void DevManageDlg::OntableViewDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid() || index.column() != COLUNM_TYPE)
    {
        return;
    }

    int type = m_tabViewMode->item(index.row(), index.column())->data(Qt::UserRole + 10).toInt();
    QString tipText(tr("%1:数据最大存储量为").arg(ConvertTfEnumToQString((::tf::DeviceType::type)type)));
    switch (type)
    {
        case ::tf::DeviceType::DEVICE_TYPE_C400:
            tipText += "2000万";
            break;
        case ::tf::DeviceType::DEVICE_TYPE_C410:
            tipText += "2200万";
            break;
        case ::tf::DeviceType::DEVICE_TYPE_C600:
            tipText += "2400万";
            break;
        case ::tf::DeviceType::DEVICE_TYPE_C610:
            tipText += "2600万";
            break;
        case ::tf::DeviceType::DEVICE_TYPE_C1000:
            tipText += "3000万";
            break;
        case ::tf::DeviceType::DEVICE_TYPE_C1100:
            tipText += "2800万";
            break;
        case ::tf::DeviceType::DEVICE_TYPE_C2000:
            tipText += "4000万";
            break;
        case ::tf::DeviceType::DEVICE_TYPE_C2200:
            tipText += "4200万";
            break;
        default:
            return;
    }

    TipDlg(tipText).exec();
}

void DevManageDlg::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    const auto& uim = UserInfoManager::GetInstance();

    ui->power_btn->setVisible(uim->IsPermisson(PSM_IM_DEVMGR_POWEROFF));
    ui->mask_btn->setVisible(uim->IsPermisson(PSM_IM_DEVMGR_SHIELD));
}
