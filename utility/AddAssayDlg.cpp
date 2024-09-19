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
/// @file     AddAssayDlg.cpp
/// @brief    追加项目弹窗
///
/// @author   7951/LuoXin
/// @date     2022年8月16日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年8月16日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "AddAssayDlg.h"
#include "ui_AddAssayDlg.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/defs.h"
#include "src/common/common.h"
#include "shared/uicommon.h"
#include "shared/tipdlg.h"
#include "shared/uidef.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/CommonInformationManager.h"
#include "src/common/Keys.h"
#include "src/common/RapidjsonUtil/RapidjsonUtil.h"
#include "src/common/encryption.h"
#include "src/public/ConfigSerialize.h"
#include "src/public/SerializeUtil.hpp"
#include "src/public/ch/ChConfigSerialize.h"
#include "thrift/DcsControlProxy.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"

#include <QDateTime>
#include <QCryptographicHash>

// 当发生错误时，弹出消息框后退出当前函数
#define OnErr(bErr, msg) \
if ((bErr))\
{\
	TipDlg(msg).exec();\
	return false;\
}

// 项目信息列表的列
enum AssayTableViewColunm
{
	COLUNM_NUM,					// 序号
	COLUNM_ASSAY_NAME,			// 项目名称
	COLUNM_ASSAY_DEV_TYPE,		// 设备型号
	COLUNM_ASSAY_CODE,			// 项目编号
	COLUNM_ASSAY_UNIT			// 单位
};

AddAssayDlg::AddAssayDlg(QWidget *parent )
	: BaseDlg(parent)
	, ui(new Ui::AddAssayDlg)
	, m_aCRecorded(false)
{
	ui->setupUi(this);

	// 设置标题
	BaseDlg::SetTitleName(tr("新增"));

	// 输入限制
	ui->code_edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_POSITIVE_INT)));

    // 设置勾选框的名字
    ui->sample_source_ckbox_0->setObjectName("sample_source_ckbox_" + QString::number(tf::SampleSourceType::SAMPLE_SOURCE_TYPE_OTHER));
    ui->sample_source_ckbox_1->setObjectName("sample_source_ckbox_" + QString::number(tf::SampleSourceType::SAMPLE_SOURCE_TYPE_XQXJ));
    ui->sample_source_ckbox_2->setObjectName("sample_source_ckbox_" + QString::number(tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NY));
    ui->sample_source_ckbox_3->setObjectName("sample_source_ckbox_" + QString::number(tf::SampleSourceType::SAMPLE_SOURCE_TYPE_QX));
    ui->sample_source_ckbox_4->setObjectName("sample_source_ckbox_" + QString::number(tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NJY));
    ui->sample_source_ckbox_5->setObjectName("sample_source_ckbox_" + QString::number(tf::SampleSourceType::SAMPLE_SOURCE_TYPE_JMQJY));

	// 关联校验码登录
	connect(ui->checkCode_edit, &QLineEdit::editingFinished, this, &AddAssayDlg::OnCheckCodeFinished);
    // 更新勾选框
    connect(ui->version_edit, &QLineEdit::textEdited, this, &AddAssayDlg::OnUpdateCheckBox);

    ui->version_edit->setValidator(new QIntValidator(1, 9999, ui->version_edit));

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

AddAssayDlg::~AddAssayDlg()
{

}

void AddAssayDlg::LoadDataToDlg(AddAssagData data)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ClearCtrls();

	if (data.iAssayCode > 0)
	{
        SetTitleName(tr("编辑项目"));

		// 名称
		ui->name_edit->setText(QString::fromStdString(data.strAssayName));
		ui->name_edit->setEnabled(false);
		// 通道号
		ui->code_edit->setText(QString::number(data.iAssayCode));
		ui->code_edit->setEnabled(false);

		// 单位
		ui->result_unit_edit->setText(QString::fromStdString(data.strUnit));
        ui->result_unit_edit->setEnabled(false);

		// 校验码
		ui->checkCode_edit->setEnabled(false);

		// 样本类型
		for (int type : data.sampleSources)
		{
			auto box = QWidget::findChild<QCheckBox*>("sample_source_ckbox_" + QString::number(type));
			box->setChecked(true);
		}
	}

    // 版本号
    ui->version_edit->setText(QString::fromStdString(data.version));

    // 封闭项目才能修改参数版本
    ui->version_edit->setEnabled(data.iAssayCode >= ::tf::AssayCodeRange::CH_RANGE_MIN
        && data.iAssayCode < ::tf::AssayCodeRange::CH_OPEN_RANGE_MIN);
}

void AddAssayDlg::ClearCtrls()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 清空控件
	for (auto box : QWidget::findChildren<QCheckBox*>())
	{
		box->setChecked(false);
	}

	for (auto edit : QWidget::findChildren<QLineEdit*>())
	{
		edit->clear();
		edit->setEnabled(true);
	}

    SetTitleName(tr("新增项目"));

}

void AddAssayDlg::OnCheckCodeFinished()
{
	// 如果没有使能，则不需要进行校验
	if (!ui->checkCode_edit->isEnabled())
	{
		return;
	}

	// 解析
	QString checkCode = ui->checkCode_edit->text();
	auto codeInfo = GetCheckCodeInfo(checkCode.toStdString());
	if (codeInfo.assayCode == -1)
	{
		return;
	}

	// 更新
	ui->code_edit->setText(QString::number(codeInfo.assayCode));
}

void AddAssayDlg::OnUpdateCheckBox()
{
    int code = ui->code_edit->text().toInt();
    auto ver = ui->version_edit->text().toStdString();

    for (const auto& box : QWidget::findChildren<QCheckBox*>())
    {
        box->setChecked(false);
    }

    for (const auto& sai : CommonInformationManager::GetInstance()->GetChemistrySpecialAssayInfo(
        code, tf::DeviceType::DEVICE_TYPE_C1000, -1, ver))
    {
        auto box = QWidget::findChild<QCheckBox*>("sample_source_ckbox_" + QString::number(sai->sampleSourceType));
        box->setChecked(true);
    }

    // 开放项目参数版本只能是1
    if (code >= ::tf::AssayCodeRange::CH_OPEN_RANGE_MIN
        && code <= ::tf::AssayCodeRange::CH_OPEN_RANGE_MAX)
    {
        ui->version_edit->setText("1");
        ui->version_edit->setEnabled(false);
    }
    else
    {
        ui->version_edit->setEnabled(true);
    }
}

void AddAssayDlg::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    bool isShow = UserInfoManager::GetInstance()->GetLoginUserInfo()->type == ::tf::UserType::USER_TYPE_TOP;
    ui->version_edit->setVisible(isShow);
    ui->label_6->setVisible(isShow);
}

///
/// @brief
///     保存开放项目记录
///
/// @param[in]  assayCode  项目通道号
/// @param[in]  checkCode  校验码
///
/// @return	成功返回true;
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年12月6日，新建函数
///
bool AddAssayDlg::SaveOpenAssayRecord(const int& assayCode, const QString& checkCode)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 参数检查
	if (checkCode.isEmpty() || assayCode < 0)
	{
		ULOG(LOG_ERROR, "Invalid params");
		return false;
	}

	// 新增开放项目记录
	tf::OpenAssayRecord oar;
	oar.__set_assayCode(assayCode);
	oar.__set_checkCode(checkCode.toStdString());

	QString dateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
	oar.__set_recordTime(dateTime.toStdString());

	tf::ResultLong ret;
	DcsControlProxy::GetInstance()->AddOpenAssayRecord(ret, oar);
	if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "AddOpenAssayRecord Failed");
		return false;
	}

	return true;
}

///
/// @brief
///     检查开放项目校验码
///
/// @return		数据合法返回true;
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年12月6日，新建函数
///
bool AddAssayDlg::CheckCode(QString& info)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 清空信息
	info = "";

	// 获取通道号 (参数已经检查过)
	int assayCode = ui->code_edit->text().toInt();
	m_aCRecorded = false;

	// 查询是否包含
	::tf::OpenAssayRecordQueryCond   oarQryCond;
	::tf::OpenAssayRecordQueryResp   oarQryResp;
	oarQryCond.__set_assayCode(assayCode);

	// 查询
	if (!DcsControlProxy::GetInstance()->QueryOpenAssayRecord(oarQryResp, oarQryCond)
		|| oarQryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS )
	{
		ULOG(LOG_ERROR, "QueryOpenAssayRecord Failed");
		info = tr("获取开放项目记录失败");
		return false;
	}

	// 如果有该项目通道号的记录，说明已经售卖，不必校验
	if (!oarQryResp.lstOpenAssayRecord.empty())
	{
		m_aCRecorded = true;
		return true;
	}

	// 获取UUID
    std::string strUUID;
    if (!DictionaryQueryManager::GetSystemUUID(strUUID))
    {
        info = tr("获取系统信息失败");
        return false;
    }
	
	// 获取校验码
	QString checkCode = ui->checkCode_edit->text();
	if (checkCode.isEmpty())
	{
		info = tr("请输入校验码");
		return false;
	}

	// 进行校验
	auto codeInfo = GetCheckCodeInfo(checkCode.toStdString());
	if (codeInfo.assayCode != assayCode ||
		codeInfo.serialNo != strUUID)
	{
		info = tr("校验失败");
		return false;
	}

    // 如果开放项目没有记录则添加
    if (!SaveOpenAssayRecord(assayCode, ui->checkCode_edit->text()))
    {
        ULOG(LOG_ERROR, "%s : SaveOpenAssayRecord Failed", __FUNCTION__);
        return false;
    }
    
	return true;
}

OpenAssayCodeCheck AddAssayDlg::GetCheckCodeInfo(const std::string& code)
{
	OpenAssayCodeCheck ret;

	// 反base64
	std::string strBase64Out;
	if (!Base64Decode(code, strBase64Out))
	{
		ULOG(LOG_ERROR, "%s : OpenAssayCodeCheck Base64Decode Failed", __FUNCTION__);
		return ret;
	}

	const unsigned char key[] = OPEN_ASSAY_CODE_KEYS;
	std::string info = rc4_decrypt(key, sizeof(key), reinterpret_cast<const unsigned char*>(strBase64Out.c_str()), strBase64Out.size());
	if (!DecodeJson(ret, info))
	{
		ULOG(LOG_ERROR, "%s : OpenAssayCodeCheck DecodeJson Failed", __FUNCTION__);
	}

	return ret;
}

bool AddAssayDlg::CheckUserData()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 至少选择一个样本源
	bool checkOne = false;
	for (QCheckBox* box : QWidget::findChildren<QCheckBox*>())
	{
		if (box->isChecked())
		{
			checkOne = true;
			break;
		}
	}
    OnErr(!checkOne, tr("至少选择一种样本源"));

    // 项目名称
    QString tempStr = ui->name_edit->text().trimmed();
    OnErr(tempStr.isEmpty(), tr("请输入项目名称"))

    int code = ui->code_edit->text().toInt();

    // 新增项目才检查项目编号范围
    if (UserInfoManager::GetInstance()->GetLoginUserInfo()->type == ::tf::UserType::USER_TYPE_TOP)
    {
        // 开发者权限可新增封闭项目
        OnErr((code < ::tf::AssayCodeRange::CH_RANGE_MIN
            || code > ::tf::AssayCodeRange::CH_OPEN_RANGE_MAX)
            , tr("通道号错误，1≤通道号≤899"));
    }
    else
    {
        // 非开发者权限只能新增开放项目
        OnErr((code < ::tf::AssayCodeRange::CH_OPEN_RANGE_MIN
            || code > ::tf::AssayCodeRange::CH_OPEN_RANGE_MAX)
            , tr("通道号错误，801≤通道号≤899"));
    }

    const auto& CIM = CommonInformationManager::GetInstance();
    OnErr(CIM->CheckAssayNameExisted(code, ui->name_edit->text().toStdString()),(tr("项目名称重复:当前项目名称为空或与其他项目的名称重复")))

	// 结果单位
	OnErr(ui->result_unit_edit->text().isEmpty(), tr("请输入结果单位"));

    // 版本号
    OnErr(ui->version_edit->text().isEmpty(), tr("请输入结果单位"));

	// 检查当前编号是否已存在
	const auto& asssayInfo = CIM->GetAssayInfo(code);
	OnErr(asssayInfo != nullptr && ui->code_edit->isEnabled(), 
			tr("通道号错误，当前通道号已存在"))

	bool falg = false;
	for (auto box : QWidget::findChildren<QCheckBox*>())
	{
		if (box->isChecked())
		{
			falg = true;
			break;
		}
	}
	OnErr(!falg, tr("请勾选样本源类型"))

	// 新增才进行校验
	if (ui->checkCode_edit->isEnabled())
	{
		// 检查校验码
		QString info = "";
		if (!CheckCode(info))
		{
			OnErr(true, info)
		}
	}
	
	return true;
}

bool AddAssayDlg::GetModifyData(AddAssagData& data)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (!CheckUserData())
	{
		ULOG(LOG_INFO, "User Input Data Invalid");
		return false;
	}

	// 项目编号
    data.iAssayCode = ui->code_edit->text().toInt();
    data.strAssayName = ui->name_edit->text().toStdString();
    data.strUnit = ui->result_unit_edit->text().toStdString();
    data.version = ui->version_edit->text().toStdString();

	for (auto box : QWidget::findChildren<QCheckBox*>())
	{
		if (box->isChecked())
		{			
            auto tempList = box->objectName().split("_");
            int sampleSource = tempList[tempList.size() - 1].toInt();
            data.sampleSources.emplace_back(sampleSource);
		}	
	}

    return true;
}

QPushButton* AddAssayDlg::GetSaveBtnPtr()
{
    return ui->save_btn;
}
