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
/// @file     AssayCodeManagerDlg.cpp
/// @brief    通道号管理
///
/// @author   8580/GongZhiQiang
/// @date     2024年5月22日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li /LuoXin，2022年8月16日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "AssayCodeManagerDlg.h"
#include "ui_AssayCodeManagerDlg.h"
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "shared/CommonInformationManager.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "thrift/DcsControlProxy.h"


AssayCodeManagerDlg::AssayCodeManagerDlg(QWidget *parent )
	: BaseDlg(parent)
	, ui(new Ui::AssayCodeManagerDlg)
{
	ui->setupUi(this);

	// 初始化
	Init();
}

AssayCodeManagerDlg::~AssayCodeManagerDlg()
{

}

void AssayCodeManagerDlg::Init()
{
	// 设置标题
	BaseDlg::SetTitleName(tr("通道管理"));

	// 输入限制
	ui->code_edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_POSITIVE_INT)));

	// 取消
	connect(ui->reject_btn, &QPushButton::clicked, this, [&]() { this->reject(); });

	// 确认
	connect(ui->accept_btn, &QPushButton::clicked, this, &AssayCodeManagerDlg::OnAcceptBtnClicked);
}

void AssayCodeManagerDlg::OnAcceptBtnClicked()
{
	// 获取项目编号(并进行参数判断)
	int openAssayCode = ui->code_edit->text().toInt();
	if (openAssayCode < tf::AssayCodeRange::CH_OPEN_RANGE_MIN || openAssayCode > tf::AssayCodeRange::CH_OPEN_RANGE_MAX)
	{
		TipDlg(tr("提示"), tr("请输入正确的开放项目通道号！")).exec();
		return;
	}

	// 判断该项目是否使用
	auto assayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(openAssayCode);
	if (assayInfo != nullptr)
	{
		TipDlg(tr("提示"), tr("通道号“%1”正在使用，请删除后再关闭！").arg(QString::number(openAssayCode))).exec();
		return;
	}

	::tf::OpenAssayRecordQueryCond oarqc;
	oarqc.__set_assayCode(openAssayCode);

	// 查询是否有该通道号
	::tf::OpenAssayRecordQueryResp _return;
	DcsControlProxy::GetInstance()->QueryOpenAssayRecord(_return, oarqc);
	if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS ||
		_return.lstOpenAssayRecord.empty())
	{
		TipDlg(tr("提示"), tr("未开放通道号“%1”！").arg(QString::number(openAssayCode))).exec();
		return;
	}
	
	// 二次确认
	if (TipDlg(tr("提示"), tr("是否确认关闭“%1”通道号？").arg(QString::number(openAssayCode)), TipDlgType::TWO_BUTTON).exec() 
		== QDialog::Rejected)
	{
		this->reject();
	}

	// 删除开放项目记录表对应的编号
	if (!DcsControlProxy::GetInstance()->DeleteOpenAssayRecord(oarqc))
	{
		TipDlg(tr("提示"), tr("删除开放项目通道号“%1”失败！").arg(QString::number(openAssayCode))).exec();
		return;
	}

	this->accept();
}
