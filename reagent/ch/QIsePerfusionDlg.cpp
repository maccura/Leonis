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
/// @file     QIsePerfusionDlg.cpp
/// @brief    ISE液路灌注弹窗
///
/// @author   7951/LuoXin
/// @date     2023年4月7日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2023年4月7日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QIsePerfusionDlg.h"
#include "ui_QIsePerfusionDlg.h"
#include "shared/tipdlg.h"
#include "src/leonis/utility/maintaindatamng.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/RapidjsonUtil/RapidjsonUtil.h"
#include "src/public/SerializeUtil.hpp"
#include "src/public/ch/ChConfigSerialize.h"
#include "src/public/ise/IseConfigSerialize.h"
#include "src/public/ise/IseConfigDefine.h"
#include "thrift/DcsControlProxy.h"
#include <QTimer>

QIsePerfusionDlg::QIsePerfusionDlg(QWidget *parent /*= Q_NULLPTR*/)
	: BaseDlg(parent)
	, ui(new Ui::QIsePerfusionDlg)
{
	ui->setupUi(this);

	// 设置标题
	BaseDlg::SetTitleName(tr("ISE管路填充"));
	// 保存按钮被点击
	connect(ui->save_btn, SIGNAL(clicked()), this, SLOT(OnSaveBtnClicked()));

	ui->buffer_ckbox->setText(ISE_BUF_NAME);
	ui->standard_solution_ckbox->setText(ISE_IS_NAME);
}

QIsePerfusionDlg::~QIsePerfusionDlg()
{

}

void QIsePerfusionDlg::LoadDataToDlg(std::vector<std::shared_ptr<const ::tf::DeviceInfo>> vecDevs)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 将布局中所有的控件移除
	QLayoutItem *child = nullptr;
	while ((child = ui->gridLayout->takeAt(0)) != nullptr)
	{
		delete child->widget();
		delete child;
	}

	// 添加设备到布局
	int count = 0;
	for (const auto& spDi : vecDevs)
	{
		// 只处理ISE设备
		if (spDi->deviceType != ::tf::DeviceType::DEVICE_TYPE_ISE1005)
		{
			continue;
		}

		for (int i = 0; i < spDi->moduleCount; i++)
		{
			QString devName;
			if (spDi->moduleCount == 1)
			{
				devName = QString::fromStdString(spDi->name);
			}
			else
			{
				QString modele = (i == 0) ? "A" : "B";
				devName = QString::fromStdString(spDi->name) + modele;
			}

			QCheckBox* box = new QCheckBox(this);
			// 防御设备为非待机态的
			box->setEnabled(spDi->status == tf::DeviceWorkState::DEVICE_STATUS_STANDBY);
			box->setProperty("deviceSN", QString::fromStdString(spDi->deviceSN));
			box->setProperty("modelIndex", (spDi->moduleCount == 1) ? 0 : i + 1);
			box->setText(devName);

			ui->gridLayout->addWidget(box, count / 3, count % 3, Qt::AlignCenter);
			count++;
		}
	}

	// 获取默认值
	tf::MaintainItem  maintainItem = MaintainDataMng::GetInstance()->GetMaintainItemByType(::tf::MaintainItemType::MAINTAIN_ITEM_ISE_FLUID_PERFUSION);
	
	MiIseFluidPerfusionParams mifpp;
	if (!DecodeJson(mifpp, maintainItem.param))
	{
		ULOG(LOG_ERROR, "DecodeJson for MiIseFluidPerfusionParams failed!");
		return;
	}

	// 设置默认参数
	ui->standard_solution_ckbox->setChecked(mifpp.bEnableStandardSolution);
	ui->buffer_ckbox->setChecked(mifpp.bEnableBuffer);

	ui->widget_unit->hide();
	if (count == 0)
	{
		ULOG(LOG_WARN, "No ise device!");
		return;
	}

	// 如果只有一个ise模块 需要隐藏单元,且设置为选中
	if (count == 1)
	{
		// 默认选中
		qobject_cast<QCheckBox*>(ui->gridLayout->itemAt(0)->widget())->setChecked(true);
	}
	else
	{
		ui->widget_unit->show();
	}
}

void QIsePerfusionDlg::OnSaveBtnClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 维护数据（所有选中设备列表和模块index）
	std::vector<::tf::DevicekeyInfo> maintainDatas;
	int childCount = ui->gridLayout->count();
	for (int i = 0; i < childCount; ++i)
	{
		auto cb = qobject_cast<QCheckBox*>(ui->gridLayout->itemAt(i)->widget());
		if (cb->isChecked())
		{
			::tf::DevicekeyInfo dsmi;
			dsmi.__set_sn(cb->property("deviceSN").toString().toStdString());
			dsmi.__set_modelIndex(cb->property("modelIndex").toInt());

			// 查找是否已经设置了该设备但是为不同模块
			auto& iter = std::find_if(maintainDatas.begin(), maintainDatas.end(), [&dsmi](auto& iter)
			{
				return (dsmi.sn == iter.sn && dsmi.modelIndex != iter.modelIndex);
			});

			// 存在SN相同，并且模块不同的，则设置为整机
			if (iter != maintainDatas.end())
			{
				iter->__set_modelIndex(0);
			}
			else
			{
				maintainDatas.push_back(std::move(dsmi));
			}

		}
	}

	// 未选中设备不执行
	if (maintainDatas.empty())
	{
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("请选择设备.")));
		pTipDlg->exec();
		return;
	}

	auto dcp = DcsControlProxy::GetInstance();

	// 查询ISE灌注的维护组id
	::tf::MaintainGroupQueryCond mgqc;
	::tf::MaintainGroupQueryResp mgqr;
	mgqc.__set_groupType(tf::MaintainGroupType::MAINTAIN_GROUP_SINGLE);
	if (!dcp->QueryMaintainGroup(mgqr, mgqc)
		||mgqr.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
		|| mgqr.lstMaintainGroups.empty())
	{
		ULOG(LOG_ERROR, "QueryMaintainGroup Failed!");
		return;
	}

	// 查找ISE灌注的维护组
	int groupId = -1;
	tf::MaintainGroup stuMtGrp;
	for (const auto& group : mgqr.lstMaintainGroups)
	{
		if (group.items.size() == 1 && group.items[0].itemType == tf::MaintainItemType::MAINTAIN_ITEM_ISE_FLUID_PERFUSION)
		{
			groupId = group.id;
			stuMtGrp = group;
			break;
		}
	}

	// 未找到对应维护组
	if (groupId == -1)
	{
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("未找到ISE管路填充维护组.")));
		pTipDlg->exec();
		return;
	}

	// 获取参数
	bool bES = ui->standard_solution_ckbox->isChecked();
	bool bEB = ui->buffer_ckbox->isChecked();

	// 修改ISE灌注对应维护项的参数信息
	auto encodeParam = MaintainDataMng::GetInstance()->GetIseFluidPerfusionParam(bES, bEB);

	// 转换失败
	if (encodeParam.empty())
	{
		ULOG(LOG_ERROR, "MaintainDataMng::GetInstance()->GetIseFluidPerfusionParam failed.");
		return;
	}

	// 执行维护
	tf::MaintainExeParams exeParams;
	exeParams.__set_groupId(groupId);
	exeParams.__set_lstDev(maintainDatas);
	exeParams.__set_itemParams({ { tf::MaintainItemType::MAINTAIN_ITEM_ISE_FLUID_PERFUSION, encodeParam } });
	dcp->Maintain(exeParams);

	close();
}

///
/// @brief
///     填充次数文本改变
///
/// @par History:
/// @li 4170/TangChuXian，2023年10月12日，新建函数
///
void QIsePerfusionDlg::OnFillCntTextChanged()
{
}

///
/// @brief
///     事件过滤器
///
/// @param[in]  obj  ui对象
/// @param[in]  evt  事件对象
///
/// @return true表示已处理
///
/// @par History:
/// @li 4170/TangChuXian，2023年10月12日，新建函数
///
bool QIsePerfusionDlg::eventFilter(QObject *obj, QEvent *evt)
{
    return BaseDlg::eventFilter(obj, evt);
}
