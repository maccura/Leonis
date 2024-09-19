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
/// @file     QSampleDetectDlg.cpp
/// @brief    样本针酸碱余量探测弹窗
///
/// @author   7997/XuXiaoLong
/// @date     2023年10月27日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7997/XuXiaoLong，2023年10月27日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QSampleDetectDlg.h"
#include "ui_QSampleDetectDlg.h"
#include "shared/tipdlg.h"
#include "src/common/Mlog/mlog.h"
#include "src/public/ise/IseConfigDefine.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include <QCheckBox>

QSampleDetectDlg::QSampleDetectDlg(QWidget *parent /*= Q_NULLPTR*/)
	: BaseDlg(parent)
	, ui(new Ui::QSampleDetectDlg)
{
	ui->setupUi(this);

	// 设置标题
	BaseDlg::SetTitleName(tr("余量检测"));
	// 保存按钮被点击
	connect(ui->save_btn_dlg, SIGNAL(clicked()), this, SLOT(OnSaveBtnClicked()));
	// 设置提示内容
	ui->label->setText(tr("本次检测仅针对ISE %1和%2。").arg(ISE_SAM_DETERGENT_ACIDITY_NAME)
		.arg(ISE_SAM_DETERGENT_ALKALINITY_NAME));
}

QSampleDetectDlg::~QSampleDetectDlg()
{

}

void QSampleDetectDlg::LoadDataToDlg(std::vector<std::shared_ptr<const ::tf::DeviceInfo>> vecDevs)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 将布局中所有的控件移除
	QLayoutItem *child = nullptr;
	while ((child = ui->gridLayout->takeAt(0)) != nullptr)
	{
		delete child->widget();
		delete child;
	}

	// 添加所有设备到表格
	int i = 0;
	for (auto spDi : vecDevs)
	{
		QCheckBox *cb = new QCheckBox(this);
		cb->setText(QString::fromStdString(spDi->groupName + spDi->name));
		cb->setProperty("deviceSN", QString::fromStdString(spDi->deviceSN));
		cb->setProperty("deviceType", (int)spDi->deviceType);
		ui->gridLayout->addWidget(cb, i / 5, i % 5);
		i++;
	}	
}

void QSampleDetectDlg::OnSaveBtnClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 所有选中设备列表和类型
	std::vector<std::pair<std::string, int>> vecPairDevs;
	int childCount = ui->gridLayout->count();
	for (int i = 0; i < childCount; ++i)
	{
		auto cb = qobject_cast<QCheckBox*>(ui->gridLayout->itemAt(i)->widget());
		if (cb->isChecked())
		{
			auto deviceSN = cb->property("deviceSN").toString().toStdString();
			auto deviceType = cb->property("deviceType").toInt();
			vecPairDevs.push_back({ deviceSN, deviceType });
		}
	}

	// 未选中设备不执行
	if (vecPairDevs.empty())
	{
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("请选择设备.")));
		pTipDlg->exec();
		return;
	}

	emit signalRemainDetect(vecPairDevs);

	close();
}
