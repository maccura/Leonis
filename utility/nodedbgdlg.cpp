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
/// @file     nodedbgdlg.cpp
/// @brief    节点调试对话框
///
/// @author   4170/TangChuXian
/// @date     2024年1月15日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月15日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "nodedbgdlg.h"
#include "ui_nodedbgdlg.h"
#include "ChNodeDebugWgt.h"
#include "ImNodeDebugWgt.h"
#include "TrkIOMDebugWgt.h"
#include "TrkRouterDebugWgt.h"
#include "TrkRWCDebugWgt.h"
#include "TrkDQIDebugWgt.h"

#include "thrift/DcsControlProxy.h"

#include "shared/tipdlg.h"
#include "shared/basedlg.h"
#include "shared/uidef.h"
#include "shared/CommonInformationManager.h"
#include "src/common/Mlog/mlog.h"
#include "src/public/DictionaryKeyName.h"
#include "manager/DictionaryQueryManager.h"
#include <QRadioButton>

NodeDbgDlg::NodeDbgDlg(QWidget *parent)
    : BaseDlg(parent),
      m_bInit(false)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化Ui对象
    ui = new Ui::NodeDbgDlg();
    ui->setupUi(this);

    // 界面显示前初始化
    InitBeforeShow();
}

NodeDbgDlg::~NodeDbgDlg()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    delete ui;

	// 断开轨道调试器
	if (DictionaryQueryManager::GetInstance()->GetPipeLine()
		&& (nullptr != _bitDebugger))
	{
		ULOG(LOG_INFO, "before _bitDebugger->end_bit_debugger()");
		_bitDebugger->end_bit_debugger();
	}
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月15日，新建函数
///
void NodeDbgDlg::InitBeforeShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 查询生化设备信息（无需查询ise设备，ise设备通过比色设备进行调试）
	auto chDevices = CIM_INSTANCE->GetDeviceFromType({ tf::DeviceType::DEVICE_TYPE_C1000 });
	if (!chDevices.empty())
	{
		m_chNodeDebugWgt = new ChNodeDebugWgt(this);
		ui->stackedWidget->addWidget(m_chNodeDebugWgt);
	}

	// 查询免疫设备信息
	auto imDevices = CIM_INSTANCE->GetDeviceFromType({ tf::DeviceType::DEVICE_TYPE_I6000 });
	if (!imDevices.empty())
	{
		m_imNodeDebugWgt = new ImNodeDebugWgt(this);
		ui->stackedWidget->addWidget(m_imNodeDebugWgt);
	}

	// 将生化和免疫设备汇总
	std::vector<std::shared_ptr<const tf::DeviceInfo>> chImDevices(chDevices);
	chImDevices.insert(chImDevices.end(), imDevices.begin(), imDevices.end());

	// 设备数量
	int devCount = chImDevices.size();
	// 第一个设备的QRadioButton
	QRadioButton* pFirstDevRbtn = Q_NULLPTR;

	// 构造QRadioButton
	for (const auto& dev : chImDevices)
	{
		auto btn = new	QRadioButton(this);
        if (pFirstDevRbtn == Q_NULLPTR)
        {
            pFirstDevRbtn = btn;
        }
		btn->setText(QString::fromStdString(dev->groupName + dev->name));
		btn->setProperty("devType", (int)dev->deviceType);
		btn->setProperty("devSN", QString::fromStdString(dev->deviceSN));
		connect(btn, &QRadioButton::clicked, this, &NodeDbgDlg::OnDeviceClicked);

		ui->horizontalLayout_devices->addWidget(btn);
	}

	// 判断是否为联机，创建轨道设备信息
	if (DictionaryQueryManager::GetInstance()->GetPipeLine()
		&& (nullptr != _bitDebugger))
	{
		ULOG(LOG_INFO, "before _bitDebugger->get_modules_info()");

		// 获取轨道模块元件信息
		std::map<std::string, ModuleInfo> modulesInfo;
		_bitDebugger->get_modules_info(modulesInfo);

		// 获取当前轨道节点信息
		std::vector<node_property> nodesProperty;
		if (!_bitDebugger->get_devices_properties(nodesProperty))
		{
			for (const auto& property : nodesProperty)
			{
				switch (property.node_speci)
				{
				case device_speciafication::Trk_IOM: {
					TrkIOMDebugWgt* iomWgt = new TrkIOMDebugWgt(this);
					QRadioButton* iomRadio = new QRadioButton(QString::fromStdString(property.node_speci_name) + "(" + QString::number(property.node_id) + ")", ui->widget);
					ui->horizontalLayout_devices->addWidget(iomRadio);
					devCount++;
					auto it = modulesInfo.find("IOM");
					if (it != modulesInfo.end())
					{
						it->second.nodeProperty = property;
						iomWgt->UpdateModuleList(it->second);
					}
					ui->stackedWidget->insertWidget(ui->stackedWidget->count(), iomWgt);
					connect(iomRadio, &QRadioButton::toggled, this, [this, iomWgt](bool checked) {
						if (checked)
						{
							ui->stackedWidget->setCurrentWidget(iomWgt);
						}
					});
					break;
				}
				case device_speciafication::Trk_ISE: {
					TrkRouterDebugWgt* routerWgt = new TrkRouterDebugWgt(this);
					QRadioButton* routerRadio = new QRadioButton("ST" + QString::fromStdString(property.node_speci_name) + "(" + QString::number(property.node_id) + ")", ui->widget);
					ui->horizontalLayout_devices->addWidget(routerRadio);
					devCount++;
					auto it = modulesInfo.find("ISE");
					if (it != modulesInfo.end())
					{
						it->second.nodeProperty = property;
						routerWgt->UpdateModuleList(it->second);
					}
					ui->stackedWidget->insertWidget(ui->stackedWidget->count(), routerWgt);
					connect(routerRadio, &QRadioButton::toggled, this, [this, routerWgt](bool checked) {
						if (checked)
						{
							ui->stackedWidget->setCurrentWidget(routerWgt);
						}
					});
					break;
				}
				case device_speciafication::Trk_ST04: {
					TrkRouterDebugWgt* routerWgt = new TrkRouterDebugWgt(this);
					QRadioButton* routerRadio = new QRadioButton(QString::fromStdString(property.node_speci_name) + "(" + QString::number(property.node_id) + ")", ui->widget);
					ui->horizontalLayout_devices->addWidget(routerRadio);
					devCount++;
					auto it = modulesInfo.find("ST04");
					if (it != modulesInfo.end())
					{
						it->second.nodeProperty = property;
						routerWgt->UpdateModuleList(it->second);
					}
					ui->stackedWidget->insertWidget(ui->stackedWidget->count(), routerWgt);
					connect(routerRadio, &QRadioButton::toggled, this, [this, routerWgt](bool checked) {
						if (checked)
						{
							ui->stackedWidget->setCurrentWidget(routerWgt);
						}
					});
					break;
				}
				case device_speciafication::Trk_ST05: {
					TrkRouterDebugWgt* routerWgt = new TrkRouterDebugWgt(this);
					QRadioButton* routerRadio = new QRadioButton(QString::fromStdString(property.node_speci_name) + "(" + QString::number(property.node_id) + ")", ui->widget);
					ui->horizontalLayout_devices->addWidget(routerRadio);
					devCount++;
					auto it = modulesInfo.find("ST05");
					if (it != modulesInfo.end())
					{
						it->second.nodeProperty = property;
						routerWgt->UpdateModuleList(it->second);
					}
					ui->stackedWidget->insertWidget(ui->stackedWidget->count(), routerWgt);
					connect(routerRadio, &QRadioButton::toggled, this, [this, routerWgt](bool checked) {
						if (checked)
						{
							ui->stackedWidget->setCurrentWidget(routerWgt);
						}
					});
					break;
				}
				case device_speciafication::Trk_RWC: {
					TrkRWCDebugWgt* rwcWgt = new TrkRWCDebugWgt(this);
					QRadioButton* rwcRadio = new QRadioButton(QString::fromStdString(property.node_speci_name) + "(" + QString::number(property.node_id) + ")", ui->widget);
					ui->horizontalLayout_devices->addWidget(rwcRadio);
					devCount++;
					auto it = modulesInfo.find("RWC");
					if (it != modulesInfo.end())
					{
						it->second.nodeProperty = property;
						rwcWgt->UpdateModuleList(it->second);
					}
					ui->stackedWidget->insertWidget(ui->stackedWidget->count(), rwcWgt);
					connect(rwcRadio, &QRadioButton::toggled, this, [this, rwcWgt](bool checked) {
						if (checked)
						{
							ui->stackedWidget->setCurrentWidget(rwcWgt);
						}
					});
					break;
				}
				case device_speciafication::Trk_DQI: {
					TrkDQIDebugWgt* dqiWgt = new TrkDQIDebugWgt(this);
					QRadioButton* dqiRadio = new QRadioButton(QString::fromStdString(property.node_speci_name) + "(" + QString::number(property.node_id) + ")", ui->widget);
					ui->horizontalLayout_devices->addWidget(dqiRadio);
					devCount++;
					auto it = modulesInfo.find("DQI");
					if (it != modulesInfo.end())
					{
						it->second.nodeProperty = property;
						dqiWgt->UpdateModuleList(it->second);
					}
					ui->stackedWidget->insertWidget(ui->stackedWidget->count(), dqiWgt);
					connect(dqiRadio, &QRadioButton::toggled, this, [this, dqiWgt](bool checked) {
						if (checked)
						{
							ui->stackedWidget->setCurrentWidget(dqiWgt);
						}
					});
					break;
				}
				default: {
					break;
				}
				}
			}
		}
	}
	
	// 第一个按钮为空直接返回
	if (pFirstDevRbtn == Q_NULLPTR)
	{
		return;
	}

	// 如果仅一个设备，不允许点击，防止重复进入界面导致刷新
	if (devCount <= 1)
	{
		pFirstDevRbtn->setEnabled(false);
	}

	// 选中第一个设备
	pFirstDevRbtn->setChecked(true);
	emit pFirstDevRbtn->clicked(true);
}

///
/// @brief
///     界面显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月15日，新建函数
///
void NodeDbgDlg::InitAfterShow()
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
void NodeDbgDlg::InitStrResource()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月15日，新建函数
///
void NodeDbgDlg::InitConnect()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 关闭按钮被点击
    connect(ui->CloseBtn, SIGNAL(clicked()), this, SLOT(OnCloseBtnClicked()));

    // 导出按钮被点击
    connect(ui->btn_export, SIGNAL(clicked()), this, SLOT(OnExportBtnClicked()));

    // 导入按钮被点击
    connect(ui->btn_import, SIGNAL(clicked()), this, SLOT(OnImportBtnClicked()));
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月15日，新建函数
///
void NodeDbgDlg::InitChildCtrl()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 设置标题
    SetTitleName(tr("节点调试"));
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
void NodeDbgDlg::showEvent(QShowEvent *event)
{
    // 调用基类接口
    BaseDlg::showEvent(event);

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }

	if (DictionaryQueryManager::GetInstance()->GetPipeLine()
		&& (nullptr != _bitDebugger))
	{
		ULOG(LOG_INFO, "before _bitDebugger->register_bit_callback()");
		//注册轨道调试执行结果回调
		_bitDebugger->register_bit_callback(OutputData, EventHandle, DebugerLog);
		_bitDebugger->begin_bit_debugger();
	}
}

void NodeDbgDlg::OnDeviceClicked()
{
	auto btn = qobject_cast<QRadioButton*>(QObject::sender());
	if (btn == nullptr)
	{
		return;
	}

	if (!btn->isChecked())
	{
		return;
	}

	m_currSelDevSn.clear();
	string devSn = btn->property("devSN").toString().toStdString();
	int devType = btn->property("devType").toInt();
	m_currSelDevSn = devSn;

	// 设备类型
	switch (devType)
	{
	case tf::DeviceType::type::DEVICE_TYPE_C1000:
		if (m_chNodeDebugWgt != nullptr)
		{
			m_chNodeDebugWgt->UpdateUi(devSn);
			ui->stackedWidget->setCurrentWidget(m_chNodeDebugWgt);
		}
		break;
	case tf::DeviceType::type::DEVICE_TYPE_I6000:
		if (m_imNodeDebugWgt != nullptr)
		{
			m_imNodeDebugWgt->UpdateUi(devSn);
			ui->stackedWidget->setCurrentWidget(m_imNodeDebugWgt);
		}
		break;
	default:
		break;
	}
}

///
/// @brief
///     导出按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月10日，新建函数
///
void NodeDbgDlg::OnExportBtnClicked()
{
	if (m_currSelDevSn.empty())
	{
		return;
	}

	// 根据设备序列号获取设备类型
	auto spDev = CommonInformationManager::GetInstance()->GetDeviceInfo(m_currSelDevSn);
	if (spDev == nullptr)
	{
		return;
	}

	switch (spDev->deviceType)
	{
		// 生化设备
	case tf::DeviceType::DEVICE_TYPE_C1000:
		if (m_chNodeDebugWgt != Q_NULLPTR)
		{
			m_chNodeDebugWgt->Export(m_currSelDevSn);
		}
		break;
		// 免疫设备
	case tf::DeviceType::DEVICE_TYPE_I6000:
		if (m_imNodeDebugWgt != Q_NULLPTR)
		{
			m_imNodeDebugWgt->ExportFIle();
		}
		break;
	default:
		break;
	}
}

///
/// @brief
///     导入按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月10日，新建函数
///
void NodeDbgDlg::OnImportBtnClicked()
{
	if (m_currSelDevSn.empty())
	{
		return;
	}

	// 根据设备序列号获取设备类型
	auto spDev = CommonInformationManager::GetInstance()->GetDeviceInfo(m_currSelDevSn);
	if (spDev == nullptr)
	{
		return;
	}

	switch (spDev->deviceType)
	{
		// 生化设备
	case tf::DeviceType::DEVICE_TYPE_C1000:
		if (m_chNodeDebugWgt != Q_NULLPTR)
		{
			m_chNodeDebugWgt->Import(m_currSelDevSn);
		}
		break;
		// 免疫设备
	case tf::DeviceType::DEVICE_TYPE_I6000:
		if (m_imNodeDebugWgt != Q_NULLPTR)
		{
			m_imNodeDebugWgt->ImportFile();
		}
		break;
	default:
		break;
	}
}

///
/// @brief
///     关闭按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月11日，新建函数
///
void NodeDbgDlg::OnCloseBtnClicked()
{
    // 直接触发停机
    reject();

    // 执行停止,进来之前已经判断了，只有待机或者停止才能进行调试
    //sDcsControlProxy::GetInstance()->StopTest();
}

void OutputData(MLINEDEBUG::OutputData* datas, int size)
{
	if (std::strlen(datas->key))
	{
		ULOG(LOG_INFO, "PID(%s) Status(%s) FlowName(%s)", datas->key, datas->value, datas->flowName);
		//调用DCSproxy
	}
}

void EventHandle(MLINEDEBUG::FlowDebugStatus event)
{
	ULOG(LOG_INFO, "NodeID(%s) FlowName(%s) DBGStatus(%d)", event.nodes->nodeId, event.flowName, event.status);
	// 弹框提示
	if (event.status == 2)
	{
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(event.flowName + QObject::tr(" 执行成功!")));
		pTipDlg->exec();
	}
	else if (event.status == 3)
	{
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(event.flowName + QObject::tr(" 执行失败!")));
		pTipDlg->exec();
	}
}

void DebugerLog(const char* log)
{
	ULOG(LOG_DEBUG, log);
}
