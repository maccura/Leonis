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
/// @file     SoftwareVersionManageDlg.cpp
/// @brief    软件版本弹窗
///
/// @author   7951/LuoXin
/// @date     2022年9月16日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年9月16日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include <QDateTime>
#include <QStandardItemModel>
#include "SoftwareVersionDlg.h"
#include "SoftwareVersionManageDlg.h"
#include "ui_SoftwareVersionManageDlg.h"
#include "shared/CommonInformationManager.h"
#include "shared/uidef.h"
#include "shared/uicommon.h"
#include "src/common/Mlog/mlog.h"
#include "src/leonis/thrift/DcsControlProxy.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"


// 表格列
enum TABLEWIDGET_COL
{
    NUMBER_COL,             // 序号
    NAME_COL,               // 模块
    VERSION_COL             // 版本号
};

SoftwareVersionManageDlg::SoftwareVersionManageDlg(QWidget *parent)
    : BaseDlg(parent)
    , ui(new Ui::SoftwareVersionManageDlg)
{
    ui->setupUi(this);
    BaseDlg::SetTitleName(tr("详情"));

    // 初始化设备按钮
    const auto& CIM = CommonInformationManager::GetInstance();
    for (auto& iter : CIM->GetDeviceMaps())
    {
        // 非联机版不显示轨道
        if (!DictionaryQueryManager::GetInstance()->GetPipeLine()
            && iter.second->deviceType == ::tf::DeviceType::DEVICE_TYPE_TRACK)
        {
            continue;
        }

        QString name = QString::fromStdString(iter.second->groupName + iter.second->name);
        QWidget* wid = new QWidget(this);
        wid->setProperty("SN", QString::fromStdString(iter.first));
        ui->tabWidget->addTab(wid, name);
    }

    connect(ui->tabWidget, &QTabWidget::currentChanged, this, [=](int index)
    {
        std::string sn = ui->tabWidget->widget(index)->property("SN").toString().toStdString();
        UpdateTableView(sn);
    });

    // 初始化表格
    m_tabViewMode = new QStandardItemModel();
    m_tabViewMode->setHorizontalHeaderLabels({ tr("序号"), tr("模块"), tr("版本号") });
    ui->tableView->setModel(m_tabViewMode);
	ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	ui->tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
	ui->tableView->setColumnWidth(0, 120); 
}

SoftwareVersionManageDlg::~SoftwareVersionManageDlg()
{

}

void SoftwareVersionManageDlg::showEvent(QShowEvent *event)
{
    if (m_tabViewMode->rowCount() <= 0)
    {
        std::string sn = ui->tabWidget->widget(ui->tabWidget->currentIndex())->property("SN").toString().toStdString();
        UpdateTableView(sn);
    }
}

void SoftwareVersionManageDlg::UpdateTableView(std::string devSn)
{
    m_tabViewMode->removeRows(0, m_tabViewMode->rowCount());
    
    std::map<std::string, std::map<std::string, std::string>> result;
    if (!DcsControlProxy::GetInstance()->QueryDeviceVersion(result))
    {
        ULOG(LOG_ERROR, "Query device version failed !");
        return;
    }

    if (result.find(devSn) == result.end())
    {
        ULOG(LOG_INFO, "Device[%s] have not version info !");
        return;
    }

	// 资源转换（免疫仪器需要支持多语言） [5/24/2024 WangZhiNang 4058]
	ConvertKey(devSn, result[devSn]);

    int i = 1;
    for (auto& iter : result[devSn])
    {
        AddTextToTableView(m_tabViewMode, i - 1, NUMBER_COL, QString::number(i));
        AddTextToTableView(m_tabViewMode, i - 1, NAME_COL, QString::fromStdString(iter.first));
        AddTextToTableView(m_tabViewMode, i - 1, VERSION_COL, QString::fromStdString(iter.second));
        i++;
    }
}

void SoftwareVersionManageDlg::ConvertKey(std::string devSn, std::map<std::string, std::string>& result)
{
	// 查询仪器类型
	::tf::DeviceInfoQueryResp _deviceReturn;
	::tf::DeviceInfoQueryCond deviceDiqc;
	deviceDiqc.__set_deviceSN(devSn);
	DcsControlProxy::GetInstance()->QueryDeviceInfo(_deviceReturn, deviceDiqc);
	if (::tf::ThriftResult::type::THRIFT_RESULT_SUCCESS != _deviceReturn.result || _deviceReturn.lstDeviceInfos.size() <= 0)
	{
		return;
	}

	// 处理免疫仪器多语言转换
	int iDeviceClassify = _deviceReturn.lstDeviceInfos[0].deviceClassify;
	switch (iDeviceClassify)
	{
	case ::tf::AssayClassify::type::ASSAY_CLASSIFY_IMMUNE:
	{
		// 免疫传上来的是枚举字符串->需转换成多语言资源
		std::map<std::string, std::string> mapVersions;
		for (const auto& m: result)
		{
			auto key = UiCommon::Instance()->ConvertTfEnumToString(m.first).toStdString();
			mapVersions[key] = m.second;
		}

		// 赋值map数据
		result.swap(mapVersions);
	}
	break;
	}

	return;
}

