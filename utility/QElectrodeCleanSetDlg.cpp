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
/// @file     QElectrodeCleanSetDlg.h
/// @brief    电极清洗设置
///
/// @author   8580/GongZhiQiang
/// @date     2024年3月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "QElectrodeCleanSetDlg.h"
#include "ui_QElectrodeCleanSetDlg.h"
#include "manager/DictionaryQueryManager.h"
#include "thrift/DcsControlProxy.h"
#include "uidcsadapter/uidcsadapter.h"
#include "uidcsadapter/abstractdevice.h"
#include "src/common/Mlog/mlog.h"
#include "src/public/ConfigSerialize.h"
#include "src/public/SerializeUtil.hpp"
#include "src/public/ise/IseConfigSerialize.h"
#include "shared/uicommon.h"
#include "shared/tipdlg.h"
#include "maintaindatamng.h"
#include <QHBoxLayout>
#include <QCheckBox>

QElectrodeCleanSetDlg::QElectrodeCleanSetDlg(QWidget *parent)
	: BaseDlg(parent),
	m_bInit(false)
{
    ui = new Ui::QElectrodeCleanSetDlg();
	ui->setupUi(this);

	// 显示前初始化
	InitBeforeShow();
}

QElectrodeCleanSetDlg::~QElectrodeCleanSetDlg()
{
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月14日，新建函数
///
void QElectrodeCleanSetDlg::InitBeforeShow()
{
	// 设置标题
	SetTitleName(tr("电极清洗设置"));

	// 加载设备
	AddIseDev();

	// 限制输入
	for (auto edit : findChildren<QLineEdit*>())
	{
		edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_POSITIVE_INT)));
	}
}


///
/// @brief
///     界面显示后初始化
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月14日，新建函数
///
void QElectrodeCleanSetDlg::InitAfterShow()
{
	// 初始化信号槽连接
	InitConnect();

	// 初始化子控件
	InitChildCtrl();

}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月14日，新建函数
///
void QElectrodeCleanSetDlg::InitConnect()
{
	// 确定按钮被点击
	connect(ui->OkBtn, SIGNAL(clicked()), this, SLOT(OnOkBtnClicked()));

	// 取消按钮被点击
	connect(ui->CancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月14日，新建函数
///
void QElectrodeCleanSetDlg::InitChildCtrl()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ElectrodeCleanSet ecs;
	if (!DictionaryQueryManager::GetElectrodeCleanSet(ecs))
	{
		ULOG(LOG_ERROR, "Failed to ElectrodeCleanSet.");
		return;
	}

	// 遍历设备
	for (auto devInfo : ecs.mapNeedWashModule)
	{
		for (auto model : devInfo.second)
		{
			// 查找
			QPair<QString, int> devKey = { QString::fromStdString(devInfo.first), model };
			if (m_devMap.contains(devKey))
			{
				m_devMap[devKey]->setChecked(true);
			}
		}
	}

	// 设置参数
	ui->edit_rackWash->setText(ecs.washRackBarcode.c_str());
	//ui->edit_cleanTimes->setText((ecs.iWashTimes < 0) ? "" : QString::number(ecs.iWashTimes));
	//ui->edit_mixedSerumTestTimes->setText((ecs.iMixedSerumTestTimes < 0) ? "" : QString::number(ecs.iMixedSerumTestTimes));

}

///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月14日，新建函数
///
void QElectrodeCleanSetDlg::showEvent(QShowEvent *event)
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
/// @brief 添加ISE设备
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月14日，新建函数
///
void QElectrodeCleanSetDlg::AddIseDev()
{
	// 获取所有设备
	QStringList strDevNameList;
	if (!gUiAdapterPtr()->GetGroupDevNameListWithTrack(strDevNameList))
	{
		return;
	}

	// 设备列表
	QHBoxLayout* pHlayout = new QHBoxLayout(ui->DevFrame);
	pHlayout->setMargin(0);
	pHlayout->setSpacing(8);

	auto AddDevCheckBox = [&](const QString& devName, const QString& devSN,const int& modelIndex){
		QCheckBox* pDevCheckBox = new QCheckBox(devName);
		pDevCheckBox->setFixedHeight(ui->DevFrame->height());
		pDevCheckBox->setChecked(false);
		pDevCheckBox->setFocusPolicy(Qt::NoFocus);
		pHlayout->addWidget(pDevCheckBox);
		m_devMap.insert({ devSN , modelIndex }, pDevCheckBox);

	};

	// 依次添加具体设备
	for (const auto& strDevName : strDevNameList)
	{

		if (gUiAdapterPtr()->GetDeviceClasssify(strDevName) == DEVICE_CLASSIFY_ISE)
		{
			std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(strDevName);
			if (spIDev == Q_NULLPTR)
			{
				continue;
			}

			// 获取设备序列号
			QString devSn = gUiAdapterPtr()->GetDevSnByName(strDevName);

			// 如果模块数小于等于1
			if (spIDev->ModuleCount() <= 1)
			{
				AddDevCheckBox(strDevName, devSn, 1);
				continue;
			}

			// 有多个模块
			for (int i = 0; i < spIDev->ModuleCount(); i++)
			{
				QString strNewSubDevName = strDevName + QChar(QChar('A').unicode() + i);
				AddDevCheckBox(strNewSubDevName, devSn, i + 1);
			}
		}
	}
}

///
/// @brief
///     确定按钮槽函数
///
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月14日，新建函数
///
void QElectrodeCleanSetDlg::OnOkBtnClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ElectrodeCleanSet ecs;
	
	// 清洗架
	QString strWashRackNum = ui->edit_rackWash->text();
	ecs.washRackBarcode = strWashRackNum.toStdString();
	
	//// 清洗次数
	//QString strCleanTimes = ui->edit_cleanTimes->text();
	//ecs.iWashTimes = strCleanTimes.isEmpty() ? -1 : strCleanTimes.toInt();

	//// 混合血清测试次数
	//QString strMSTT = ui->edit_mixedSerumTestTimes->text();
	//ecs.iMixedSerumTestTimes = strMSTT.isEmpty() ? -1 : strMSTT.toInt();

	// 设备选择
	QMapIterator<QPair<QString, int>, QCheckBox*> iter(m_devMap);
	while (iter.hasNext()) {
		iter.next();

		// 判空
		if (iter.value() == nullptr)
		{
			continue;
		}

		// 勾选了
		if (iter.value()->isChecked())
		{
			std::string sn = iter.key().first.toStdString();
			int moduleIndex = iter.key().second;

			auto mIndex = ecs.mapNeedWashModule.find(sn);
			if (mIndex != ecs.mapNeedWashModule.end())
			{
				// 增加模块
				ecs.mapNeedWashModule[sn].insert(moduleIndex);
			}
			else
			{
				// 增加模块和Sn
				ecs.mapNeedWashModule.insert({ sn, { moduleIndex } });
			}
		}
	}

	// 同步修改维护项默认设置和字典配置信息
	auto item = MaintainDataMng::GetInstance()->GetMaintainItemByType(tf::MaintainItemType::MAINTAIN_ITEM_ISE_CLEAN_ELECTRODE);
	item.__set_param(Encode2Json(ecs));
	if (!DictionaryQueryManager::SaveElectrodeCleanSet(ecs) || 
		!DcsControlProxy::GetInstance()->ModifyMaintainItem(item))
	{
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("提示"), tr("保存电极清洗设置失败！"), TipDlgType::SINGLE_BUTTON));
		pTipDlg->exec();
		return;
	}

	// 退出
	this->accept();
}


