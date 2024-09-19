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
/// @file     QPatientDialog.h
/// @brief    病人信息
///
/// @author   5774/WuHongTao
/// @date     2022年5月24日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QPatientDialog.h"
#include "ui_QPatientDialog.h"
#include <QDateTime>
#include <QString>
#include <QKeyEvent>
#include "shared/uicommon.h"
#include "shared/tipdlg.h"
#include "shared/uidef.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/datetimefmttool.h"
#include "shared/CommonInformationManager.h"
#include "thrift/DcsControlProxy.h"
#include "manager/DictionaryQueryManager.h"
#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"
#include "QSampleAssayModel.h"
#include "src/public/SerializeUtil.hpp"
#include "src/public/ConfigSerialize.h"
#include "QHistorySampleAssayModel.h"

#define  TimeFormat "yyyy-MM-dd HH:mm:ss"
#define MAXLENGTH 25

QPatientDialog::QPatientDialog(QWidget *parent)
	: BaseDlg(parent)
{
    ui = new Ui::QPatientDialog();
	ui->setupUi(this);
	InitDialog();
}

QPatientDialog::~QPatientDialog()
{
}

///
/// @brief 刷新样本的病人对象信息
///
/// @param[in]  sampleDb  样本数据库主键
/// @param[in]  Isfirst   是否是第一条
/// @param[in]  Islast    是否最后一条
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月19日，新建函数
///
void QPatientDialog::UpdatePatientData(int sampleDb, bool Isfirst, bool Islast)
{
	// 设置默认状态
	SetDeFaultStatus();
	// 设置默认信息
	SetDefaultMessage();

	// 若是第一条，则上一条不能选择
	if (Isfirst)
	{
		ui->pre_btn->setEnabled(false);
	}

	// 最后一条
	if (Islast)
	{
		ui->next_btn->setEnabled(false);
	}

    ShowPatientInfo(sampleDb);
}

///
/// @brief 初始化患者的表格信息
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月19日，新建函数
///
void QPatientDialog::InitDialog()
{
	SetTitleName(tr("患者信息"));

	setModal(true);
	// 初始化性别下拉框
	AddTfEnumItemToComBoBox(ui->patient_gender, ::tf::Gender::GENDER_UNKNOWN);
	AddTfEnumItemToComBoBox(ui->patient_gender, ::tf::Gender::GENDER_MALE);
	AddTfEnumItemToComBoBox(ui->patient_gender, ::tf::Gender::GENDER_FEMALE);

	// 初始化年龄单位
    AddTfEnumItemToComBoBox(ui->patient_age_unit, ::tf::AgeUnit::AGE_UNIT_UNKNOWN);
	AddTfEnumItemToComBoBox(ui->patient_age_unit, ::tf::AgeUnit::AGE_UNIT_YEAR);
	AddTfEnumItemToComBoBox(ui->patient_age_unit, ::tf::AgeUnit::AGE_UNIT_MONTH);
	AddTfEnumItemToComBoBox(ui->patient_age_unit, ::tf::AgeUnit::AGE_UNIT_DAY);
	AddTfEnumItemToComBoBox(ui->patient_age_unit, ::tf::AgeUnit::AGE_UNIT_HOUR);

	// 保存
	connect(ui->save_btn, SIGNAL(clicked()), this, SLOT(OnSavePatientInfo()));
	// 取消
	connect(ui->cancel_btn, &QPushButton::clicked, this, [&]() {close(); });
	// 上一条按钮被选择
	connect(ui->pre_btn, SIGNAL(clicked()), this, SLOT(OnMovePreBtn()));
	// 下一条按钮被选择
	connect(ui->next_btn, SIGNAL(clicked()), this, SLOT(OnMoveNextBtn()));

	// 输入限制
	ui->gender_age->setValidator(new QRegExpValidator(QRegExp("^([1-9]\\d{0,2})$")));
    // 限制备注只能输入15个字符（源自禅道bug:3165）
    ui->patient_comment->lineEdit()->setMaxLength(15);
    // 限制姓名、病人id、电话号码、床号、医保账号、住院号的长度（源自免疫Mantis缺陷:18082）
    ui->patient_name->setMaxLength(50);
    ui->sample_medical_record_no->setMaxLength(50);
    ui->patient_phone_no->setMaxLength(20);
    ui->patient_bed_no->setMaxLength(20);
    ui->patient_account->setMaxLength(20);
    ui->patient_admission_no->setMaxLength(20);

    // 更新下拉框
    UpdateComBoBoxItem();
    UpdatePageItems();

    // 注册数据字典数据信息更新
    REGISTER_HANDLER(MSG_ID_PATINET_DATA_DICT_INFO_UPDATE, this, OnDataDictInfoUpdate);
    // 注册数据字典数据项更新
    REGISTER_HANDLER(MSG_ID_PATINET_DATA_DICT_ITEM_UPDATE, this, OnPatientSettingUpdated);
    // 注册患者信息位置更新
    REGISTER_HANDLER(MSG_ID_PATINET_INFO_POSITION_UPDATE, this, OnPatientSettingUpdated);
    // 监听样本信息
    REGISTER_HANDLER(MSG_ID_SAMPLE_INFO_UPDATE, this, OnSampleUpdate);
}

///
/// @brief 设置表单初始状态
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月19日，新建函数
///
void QPatientDialog::SetDeFaultStatus()
{
	m_sampleDb = -1;
	m_patientId = -1;
	ui->pre_btn->setEnabled(true);
	ui->next_btn->setEnabled(true);
	ui->save_btn->setEnabled(false);
	ui->cancel_btn->setEnabled(true);
}

///
/// @brief 设置病人默认信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月20日，新建函数
///
void QPatientDialog::SetDefaultMessage()
{
	for (auto edit : QWidget::findChildren<QLineEdit*>())
	{
		edit->clear();
	}

	for (auto box : QWidget::findChildren<QNoteComboBox*>())
	{
		box->setCurrentIndex(-1);
	}
}

bool QPatientDialog::GetCustomData(int type, QStringList& customList, QMap<QString, QString>& noteMap)
{
    customList.clear();
    noteMap.clear();
    std::vector<CustomSetRowData> veccs;
    if (DictionaryQueryManager::GetCustomSetData(type, veccs))
    {
        for (const CustomSetRowData& item : veccs)
        {
            const QString& strData = QString::fromStdString(item.strData);
            customList.push_back(strData);
            noteMap[QString::fromStdString(item.strMnemonic)] = strData;
        }
        return true;
    }

    bool ret = CommonInformationManager::GetInstance()->GetDefaultDictData(type, veccs);
    if (ret)
    {
        for (auto item : veccs)
        {
            customList.push_back(QString::fromStdString(item.strData));
        }
    }

	return ret;
}

void QPatientDialog::UpdateComBoBoxItem()
{
	QStringList data;
    QMap<QString, QString> noteMap;

    // 患者类型
    /*if (GetCustomData(tf::PatientFields::PATIENT_TYPE, data))
    {
        ui->patient_sample_type->clear();
        ui->patient_sample_type->addItems(data);
    }*/
    // 临床诊断
    if (GetCustomData(tf::PatientFields::PATIENT_DIAGNOSIS, data, noteMap))
    {
        ui->patient_diagnosis->clear();
        ui->patient_diagnosis->addItems(data);
        ui->patient_diagnosis->SetNoteMap(noteMap);
    }
    // 送检科室
    if (GetCustomData(tf::PatientFields::PATIENT_INSPECT_DEPARTMENT, data, noteMap))
    {
        ui->patient_department->clear();
        ui->patient_department->addItems(data);
        ui->patient_department->SetNoteMap(noteMap);
    }
    // 送检医师
    if (GetCustomData(tf::PatientFields::PATIENT_INSPECT_DOCTOR, data, noteMap))
    {
        ui->patient_tester->clear();
        ui->patient_tester->addItems(data);
        ui->patient_tester->SetNoteMap(noteMap);
    }
    // 检验医师
    if (GetCustomData(tf::PatientFields::PATIENT_INSPECTOR, data, noteMap))
    {
        ui->patient_doctor->clear();
        ui->patient_doctor->addItems(data);
        ui->patient_doctor->SetNoteMap(noteMap);
    }
    // 收费类型
    if (GetCustomData(tf::PatientFields::PATIENT_COST_TYPE, data, noteMap))
    {
        ui->patient_cost_type->clear();
        ui->patient_cost_type->addItems(data);
        ui->patient_cost_type->SetNoteMap(noteMap);
    }
    // 审核医师
    if (GetCustomData(tf::PatientFields::PATIENT_AUDITOR, data, noteMap))
    {
        ui->patient_auditor->clear();
        ui->patient_auditor->addItems(data);
        ui->patient_auditor->SetNoteMap(noteMap);
    }
    // 病区
    if (GetCustomData(tf::PatientFields::PATIENT_INPATIENT_AREA, data, noteMap))
    {
        ui->patient_inpatient->clear();
        ui->patient_inpatient->addItems(data);
        ui->patient_inpatient->SetNoteMap(noteMap);
    }
    // 主治医生
    if (GetCustomData(tf::PatientFields::PATIENT_ATTENDING_DOCTOR, data, noteMap))
    {
        ui->patient_attending->clear();
        ui->patient_attending->addItems(data);
        ui->patient_attending->SetNoteMap(noteMap);
    }
    // 备注 
    if (GetCustomData(tf::PatientFields::PATIENT_COMMENT, data, noteMap))
    {
        ui->patient_comment->clear();
        ui->patient_comment->addItems(data);
        ui->patient_comment->SetNoteMap(noteMap);
    }

    // 自定义设置1-5
    for (int i = 1; i <= 5; ++i)
    {
        if (GetCustomData(tf::PatientFields::PATIENT_CUSTOM_BASE + i, data, noteMap))
        {
            QNoteComboBox* combo = findChild<QNoteComboBox*>("customInfo_" + QString::number(i));
            if (combo)
            {
                combo->clear();
                combo->addItems(data);
                combo->SetNoteMap(noteMap);
            }
        }
    }
}

bool QPatientDialog::CheckUserData()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 送检时间和采样时间从Lis上获取，不用检查格式
    /*QDateTime&& inspectTime = QDateTime::fromString(ui->patient_inspectTime->text(), "yyyy/MM/dd hh:mm:ss");
    ReportErr(!ui->patient_inspectTime->text().isEmpty()
        && !inspectTime.isValid(), tr("送检时间输入格式错误，清空输入内容，查看示例格式"))

    QDateTime&& samplingTime = QDateTime::fromString(ui->patient_samplingTime->text(), "yyyy/MM/dd hh:mm:ss");
    ReportErr(!ui->patient_samplingTime->text().isEmpty()
        && !samplingTime.isValid(), tr("采样时间输入格式错误，清空输入内容，查看示例格式"))*/

	return true;
}

void QPatientDialog::UpdatePageItems()
{
    // 把所有的控件先保存下来，用于以后的显示使用
    QLayoutItem *child;
    while ((child = ui->gridLayout->takeAt(0)) != 0) {
        if (child->spacerItem() != Q_NULLPTR)
            m_bottomSpacer = child;
        else
        {
            QLayout* l = child->layout();
           
            if (l != Q_NULLPTR)
            {
                SetLayoutVisible(l, false);
                m_patientCtrls.insert(l->objectName(), child);
            }
        }
    }

    std::vector<DisplaySet> savedata;
    if (!DictionaryQueryManager::GetUiDispalySet(savedata))
    {
        ULOG(LOG_ERROR, "Failed to get DisplaySet.");
        return;
    }

    for (auto& d : savedata)
    {
        if (d.type == PATIENTTYPE)
        {
            OnShowDisplaySet(d);
            break;
        }
    }
}

void QPatientDialog::SetLayoutVisible(QLayout * layout, bool visible)
{
    if (layout == Q_NULLPTR)
        return;

    for (int i = 0; i < layout->count(); ++i)
    {
        auto cItem = layout->itemAt(i);
        if (cItem->widget())
            cItem->widget()->setVisible(visible);
        // 年龄有下级layout，递归一次
        else if (cItem->layout())
            SetLayoutVisible(cItem->layout(), visible);
    }
}

void QPatientDialog::ShowPatientInfo(int sampleId)
{
    // 查询样本信息以获取病人信息数据库主键
    ::tf::SampleInfoQueryCond sampQryCond;
    ::tf::SampleInfoQueryResp sampQryResp;
    sampQryCond.__set_id(sampleId);
    sampQryCond.__set_containTestItems(true);

	auto doFunction = &QPatientDialog::SaveButtonStatus;
	ExitDoit<QPatientDialog, decltype(doFunction)> exitFunction(this, doFunction);

    // 执行查询条件
    if (!DcsControlProxy::GetInstance()->QuerySampleInfo(sampQryCond, sampQryResp) || sampQryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "query sample Info failed!");
        return;
    }

    // 将查询到的样本信息结果更新到界面
    if (sampQryResp.lstSampleInfos.empty())
    {
        ULOG(LOG_INFO, "no sample Info data");
        return;
    }

    m_sampleDb = sampleId;
    m_sampleInfo = sampQryResp.lstSampleInfos[0];
    m_readOnly = m_sampleInfo.audit;
    SetSeqNoAndBarcodeEditRule();
    // 样本号
    ui->sample_no->setText(QString::fromStdString(m_sampleInfo.seqNo));
    // 样本条码
    ui->sample_bar->setText(QString::fromStdString(m_sampleInfo.barcode));

    // 未测试完成的项目，不能修改
    if (m_sampleInfo.status != tf::SampleStatus::SAMPLE_STATUS_TESTED)
    {
        ui->sample_no->setEnabled(false);
        ui->sample_bar->setEnabled(false);
    }
    else
    {
        ui->sample_no->setEnabled(true);
        ui->sample_bar->setEnabled(true);
    }

    // 查询病人信息
    ::tf::PatientInfoQueryCond patienQryCond;
    ::tf::PatientInfoQueryResp patienQryResp;
    patienQryCond.__set_id(m_sampleInfo.patientInfoId);

    m_patientId = -1;
    // 患者类型
    ui->patient_sample_type->setText(sampQryResp.lstSampleInfos[0].stat ? tr("急诊") : tr("常规"));

    // 执行查询条件
    if (!DcsControlProxy::GetInstance()->QueryPatientInfo(patienQryResp, patienQryCond) || patienQryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "query patient Info failed!");
        return;
    }

    // 将查询到的病人信息结果更新到界面
    if (patienQryResp.lstPatientInfos.empty())
    {
        ULOG(LOG_INFO, "no patient Info data");
        return;
    }

    tf::PatientInfo patientInfo = patienQryResp.lstPatientInfos[0];
    m_patientId = patientInfo.id;

    // 病例号
    ui->sample_medical_record_no->setText(QString::fromStdString(patientInfo.medicalRecordNo));
    // 临床诊断
    ui->patient_diagnosis->setCurrentText(QString::fromStdString(patientInfo.diagnosis));
    // 病人姓名
    ui->patient_name->setText(QString::fromStdString(patientInfo.name));
    // 病人年龄
    if (patientInfo.age > 0)
    {
        ui->gender_age->setText(QString::number(patientInfo.age));
    }
    // 年龄单位
    ui->patient_age_unit->setCurrentIndex(ui->patient_age_unit->findData(patientInfo.ageUnit));
    // 病人性别
    ui->patient_gender->setCurrentIndex(ui->patient_gender->findData(patientInfo.gender));
    // 送检时间
    ui->patient_inspectTime->setText(ToCfgFmtDateTime(QString::fromStdString(patientInfo.inspectTime)));
    // 送检科室
    ui->patient_department->setCurrentText(QString::fromStdString(patientInfo.department));
    // 送检医生
    ui->patient_doctor->setCurrentText(QString::fromStdString(patientInfo.doctor));
    // 采样时间
    ui->patient_samplingTime->setText(ToCfgFmtDateTime(QString::fromStdString(patientInfo.samplingTime)));
    // 检验医师
    ui->patient_tester->setCurrentText(QString::fromStdString(patientInfo.tester));
    // 住院号
    ui->patient_admission_no->setText(QString::fromStdString(patientInfo.admissionNo));
    // 收费类型
    ui->patient_cost_type->setCurrentText(QString::fromStdString(patientInfo.costType));
    // 审核医师
    ui->patient_auditor->setCurrentText(QString::fromStdString(patientInfo.auditor));
    // 病区
    ui->patient_inpatient->setCurrentText(QString::fromStdString(patientInfo.inpatientArea));
    // 医保账号
    ui->patient_account->setText(QString::fromStdString(patientInfo.accountNo));
    // 主治医生
    ui->patient_attending->setCurrentText(QString::fromStdString(patientInfo.attendingDoctor));
    // 床号
    ui->patient_bed_no->setText(QString::fromStdString(patientInfo.bedNo));
    // 出生日期
    ui->patient_birthday->setDateTimeStr(QString::fromStdString(patientInfo.birthday));
    // 备注信息
    ui->patient_comment->setCurrentText(QString::fromStdString(patientInfo.comment));
    // 电话
    ui->patient_phone_no->setText(QString::fromStdString(patientInfo.phoneNo));

    // 处理自定义信息
    // 如果原有自定义信息和现有自定义类型不一致，则不显示
    std::vector<PatientCustomInfo> pcInfo;
    if (!DecodeJson(pcInfo, patientInfo.customInfo))
    {
        ULOG(LOG_ERROR, "DecodeJson PatientCustomInfo Failed");
        return;
    }

    auto Func_SetText = [](QLabel* const label, QComboBox* const combo, const std::string& itemName, const std::string& itemData)
    {
        // 如果原来的自定义设置和现在的自定义设置已经不一样了，就不需要显示
        QString && text = QString::fromStdString(itemName);
        if (label->text() == text)
        {
            combo->setCurrentText(QString::fromStdString(itemData));
        }
    };

    // 设置自定义数据
    for (const auto& info : pcInfo)
    {
        switch (info.infoType)
        {
            case tf::PatientFields::PATIENT_CUSTOM_BASE + 1:
                Func_SetText(ui->customInfo_lab_1, ui->customInfo_1, info.itemName, info.itemData);
                break;
            case tf::PatientFields::PATIENT_CUSTOM_BASE + 2:
                Func_SetText(ui->customInfo_lab_2, ui->customInfo_2, info.itemName, info.itemData);
                break;
            case tf::PatientFields::PATIENT_CUSTOM_BASE + 3:
                Func_SetText(ui->customInfo_lab_3, ui->customInfo_3, info.itemName, info.itemData);
                break;
            case tf::PatientFields::PATIENT_CUSTOM_BASE + 4:
                Func_SetText(ui->customInfo_lab_4, ui->customInfo_4, info.itemName, info.itemData);
                break;
            case tf::PatientFields::PATIENT_CUSTOM_BASE + 5:
                Func_SetText(ui->customInfo_lab_5, ui->customInfo_5, info.itemName, info.itemData);
                break;
        }
    }
}

void QPatientDialog::SaveButtonStatus()
{
	ui->save_btn->setEnabled(false);
	auto editBoxs = this->findChildren<QLineEdit*>();
	for (const auto& editBox : editBoxs)
	{
		connect(editBox, &QLineEdit::textChanged, this, [&](const QString& text)
		{
			ui->save_btn->setEnabled(true);
		});
	}

	auto selectBoxs = this->findChildren<QComboBox*>();
	for (const auto& selectBox : selectBoxs)
	{
		connect(selectBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [&](int index)
		{
			ui->save_btn->setEnabled(true);
		});
	}

	connect(ui->patient_birthday, &QDateEdit::dateChanged, this, [&](const QDate& date)
	{
		ui->save_btn->setEnabled(true);
	});
}

void QPatientDialog::SetSeqNoAndBarcodeEditRule()
{
    ::tf::TestMode::type smodel = (::tf::TestMode::type)DictionaryQueryManager::GetInjectionModel();

    // 条码模式
    if (smodel == ::tf::TestMode::BARCODE_MODE)
    {
        // 样本条码的限制(条码模式)
        ui->sample_no->setValidator(Q_NULLPTR);
        ui->sample_no->setMaxLength(MAXLENGTH);
        ui->sample_bar->setValidator(new QRegExpValidator(QRegExp(UI_REG_SAMPLE_BAR_ASCII), this));
    }
    // 样本架模式
    else if (smodel == ::tf::TestMode::RACK_MODE)
    {
        ui->sample_bar->setValidator(Q_NULLPTR);
        ui->sample_bar->setMaxLength(MAXLENGTH);
        ui->sample_no->setValidator(new QRegExpValidator(QRegExp(UI_REG_SAMPLE_NUM), this));
    }
    // 序号模式
    else
    {
        ui->sample_bar->setValidator(Q_NULLPTR);
        ui->sample_bar->setMaxLength(MAXLENGTH);
        ui->sample_no->setValidator(new QRegExpValidator(QRegExp(UI_REG_SAMPLE_NUM), this));
    }
}

void QPatientDialog::keyPressEvent(QKeyEvent * e)
{
    // 屏蔽键盘上的两个回车键按钮
    // 此案件会造成上一条按钮被触发
    if (e->key() == Qt::Key_Return ||
        e->key() == Qt::Key_Enter)
        return;

    QDialog::keyPressEvent(e);
}

///
/// @brief 保存病人信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月19日，新建函数
///
void QPatientDialog::OnSavePatientInfo()
{
	if (m_readOnly)
	{
		TipDlg(tr("提示"), tr("样本已审核，不可修改！")).exec();
		return;
	}

	// 用户输入的数据不合法，或没有对应信息，则返回
	if (!CheckUserData() || m_sampleDb == -1)
	{
		return;
	}

	tf::PatientInfo patientInfoSave;
	// 设置病历号
	patientInfoSave.__set_medicalRecordNo(ui->sample_medical_record_no->text().toUtf8().toStdString());
    // 患者类型
    //patientInfoSave.__set_type(ui->patient_sample_type->currentText().toUtf8().toStdString());
    // 临床诊断
    patientInfoSave.__set_diagnosis(ui->patient_diagnosis->currentText().toUtf8().toStdString());
	// 患者姓名
    patientInfoSave.__set_name(ui->patient_name->text().toStdString());
    // 设置病人年龄
    patientInfoSave.__set_age(ui->gender_age->text().toInt());

    // 年龄单位
    if (ui->patient_age_unit->currentIndex() != -1)
    {
        ::tf::AgeUnit::type ageUnit = static_cast<::tf::AgeUnit::type>(ui->patient_age_unit->currentData().toInt());
        patientInfoSave.__set_ageUnit(ageUnit);
    }
	// 病人的性别
	if (ui->patient_gender->currentIndex() != -1)
	{
		::tf::Gender::type gender = static_cast<::tf::Gender::type>(ui->patient_gender->currentData().toInt());
		patientInfoSave.__set_gender(gender);
    }
    //送检时间(从Lis上获取，不用推送)
    //patientInfoSave.__set_inspectTime(ui->patient_inspectTime->text().toStdString());
    // 送检科室
    patientInfoSave.__set_department(ui->patient_department->currentText().toUtf8().toStdString());
    // 送检医师
    patientInfoSave.__set_doctor(ui->patient_doctor->currentText().toUtf8().toStdString());
    //采样时间(从Lis上获取，不用推送)
    //patientInfoSave.__set_samplingTime(ui->patient_samplingTime->text().toStdString());
    // 检验医师
    patientInfoSave.__set_tester(ui->patient_tester->currentText().toUtf8().toStdString());
    // 设置住院号
    patientInfoSave.__set_admissionNo(ui->patient_admission_no->text().toUtf8().toStdString());
    // 收费类型
    patientInfoSave.__set_costType(ui->patient_cost_type->currentText().toUtf8().toStdString());
    // 审核医师
    patientInfoSave.__set_auditor(ui->patient_auditor->currentText().toUtf8().toStdString());
	// 病区
    patientInfoSave.__set_inpatientArea(ui->patient_inpatient->currentText().toUtf8().toStdString());
    // 医保账号
    patientInfoSave.__set_accountNo(ui->patient_account->text().toUtf8().toStdString());
    // 主治医生
    patientInfoSave.__set_attendingDoctor(ui->patient_attending->currentText().toUtf8().toStdString());
	// 床号
    patientInfoSave.__set_bedNo(ui->patient_bed_no->text().toUtf8().toStdString());
    // 出生日期
    if (!ui->patient_birthday->isNull())
        patientInfoSave.__set_birthday(ui->patient_birthday->date().toString("yyyy-MM-dd 00:00:00").toStdString());
    // 备注
    patientInfoSave.__set_comment(ui->patient_comment->currentText().toUtf8().toStdString());
	// 电话
	patientInfoSave.__set_phoneNo(ui->patient_phone_no->text().toUtf8().toStdString());
    // 自定义信息
    std::vector<PatientCustomInfo> pcInfos;
    for (int i = 1; i <= 5; ++i)
    {
        QNoteComboBox* item = ui->widget1->findChild<QNoteComboBox*>(QString("customInfo_%1").arg(i));
        QLabel* label = ui->widget1->findChild<QLabel*>(QString("customInfo_lab_%1").arg(i));
        if (item && !item->currentText().isEmpty())
        {
            PatientCustomInfo info;
            info.infoType = tf::PatientFields::PATIENT_CUSTOM_BASE + i;
            info.itemData = item->currentText().toUtf8().toStdString();
            if (label)
                info.itemName = label->text().toUtf8().toStdString();

            pcInfos.push_back(std::move(info));
        }
    }
    std::string xml;
    if (!Encode2Json(xml, pcInfos))
        ULOG(LOG_ERROR, "Encode2Json PatientCustomInfo Failed");
    else
        patientInfoSave.__set_customInfo(xml);
    
    int updatePatientId = -1;

	// 判断病人信息数据库主键是否有效
	if (m_patientId < 0)
	{
		// 病人信息数据库主键无效，需要添加病人信息
		::tf::ResultLong retLong;
		if (!DcsControlProxy::GetInstance()->AddPatientInfo(retLong, patientInfoSave, m_sampleDb) || retLong.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
		{
			ULOG(LOG_ERROR, "AddPatientInfo() failed!");
			return;
		}
        m_patientId = retLong.value;
        updatePatientId = m_patientId;
	}
	else
	{
		// 更新病人信息
		patientInfoSave.__set_id(m_patientId);
		if (!DcsControlProxy::GetInstance()->ModifyPatientInfo(patientInfoSave))
		{
			ULOG(LOG_ERROR, "ModifyPatientInfo() failed!");
		}
		updatePatientId = m_patientId;
	}

    // 处理样本信息变更逻辑
    // 开始更新样本信息
    std::string curSeqNo = ui->sample_no->text().toStdString();
    std::string curBarcode = ui->sample_bar->text().toStdString();
    ::tf::SampleInfoQueryCond sampQryCond;
    ::tf::SampleInfoQueryResp sampQryResp;
    sampQryCond.__set_id(m_sampleDb);
    sampQryCond.__set_containTestItems(true);

    // 执行查询条件
    if (!DcsControlProxy::GetInstance()->QuerySampleInfo(sampQryCond, sampQryResp)
        || sampQryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || sampQryResp.lstSampleInfos.empty())
    {
        ULOG(LOG_ERROR, "fail to execute QuerySampleInfo");
        return;
    }

    ::tf::SampleInfo sampleInfo;
    if (updatePatientId > 0)
    {
        sampQryResp.lstSampleInfos[0].__set_patientInfoId(updatePatientId);
        sampleInfo.__set_patientInfoId(updatePatientId);
    }

    // 暂时不实现样本号和样本条码的修改，此功能前置条件较多
    /*if (ui->sample_no->isEnabled())
        sampQryResp.lstSampleInfos[0].__set_seqNo(curSeqNo);

    if (ui->sample_bar->isEnabled())
        sampQryResp.lstSampleInfos[0].__set_barcode(curBarcode);*/

	auto seqText = ui->sample_no->text().toStdString();
	auto barText = ui->sample_bar->text().toStdString();
	if (seqText.empty() && barText.empty())
	{
		ULOG(LOG_ERROR, "%s(%s)", __FUNCTION__, tr(u8"条码和样本号同时为空"));
		TipDlg(tr("警告"), tr("条码和样本号不能同时为空")).exec();
		return;
	}

	auto testMode = sampQryResp.lstSampleInfos[0].testMode;
	if (tf::TestMode::BARCODE_MODE != testMode && seqText.empty())
	{
		TipDlg(tr("警告"), tr("样本号/样本架模式下的样本号不能为空")).exec();
		return;
	}

	if (tf::TestMode::BARCODE_MODE == testMode && barText.empty())
	{
		TipDlg(tr("警告"), tr("条码模式下条码不能为空")).exec();
		return;
	}

    // 2.是否修改样本号和样本条码
    // 如果样本序号或者样本条码可编辑，且有发生变化
    if (seqText !=  sampQryResp.lstSampleInfos[0].seqNo ||
		barText != sampQryResp.lstSampleInfos[0].barcode ||
        // 或者需要更新病人id（修改了病人信息必须刷新样本bug0012586）
        (updatePatientId >= 0))
    {
		auto sampleNow = sampQryResp.lstSampleInfos[0];
		// 条码模式
		if (sampleNow.testMode == ::tf::TestMode::BARCODE_MODE)
		{
			auto sampleData = DataPrivate::Instance().GetSampleByBarCode(sampleNow.testMode, curBarcode, tf::SampleType::SAMPLE_TYPE_PATIENT);
			if (sampleData.has_value() && sampleData.value().id != m_sampleDb)
			{
				ULOG(LOG_ERROR,"%s(barcode %s: %s)", __FUNCTION__, curBarcode, tr("has exist"));
				TipDlg(tr("警告"), tr("存在相同的条码")).exec();
				return;
			}
		}
		// 样本架模式
		// 序号模式
		else
		{
			auto sampleData = DataPrivate::Instance().GetSampleBySeq(sampleNow.testMode, curSeqNo, tf::SampleType::SAMPLE_TYPE_PATIENT, sampleNow.stat);
			if (sampleData.has_value() && sampleData.value().id != m_sampleDb)
			{
				ULOG(LOG_ERROR, "%s(seqNo %s: %s)", __FUNCTION__, curSeqNo, tr("has exist"));
				TipDlg(tr("警告"), tr("存在相同的序号")).exec();
				return;
			}
		}

        ::tf::SampleInfo sampleInfo;
        sampleInfo.__set_id(sampQryResp.lstSampleInfos[0].id);
        sampleInfo.__set_patientInfoId(updatePatientId);
		sampleInfo.__set_barcode(curBarcode);
		sampleInfo.__set_seqNo(curSeqNo);
        // 执行查询条件
        if (!DcsControlProxy::GetInstance()->ModifySampleInfo(sampleInfo))
        {
            ULOG(LOG_ERROR, "fail to execute ModifySampleInfo");
            return;
        }
    }

	QCustomDialog tipDlg(this, 1000, tr("保存成功"));
	tipDlg.exec();

	// 更新历史数据中的病人ID
	QHistorySampleAssayModel::UpdatePatienInfo(sampQryResp.lstSampleInfos[0].id, m_patientId, ui->patient_name->text().toStdString(),\
		ui->sample_medical_record_no->text().toUtf8().toStdString());
}

///
/// @brief 向上移动
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月24日，新建函数
///
void QPatientDialog::OnMovePreBtn()
{
	emit MoveButtonPressed(-1);
}

///
/// @brief 向下移动
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月24日，新建函数
///
void QPatientDialog::OnMoveNextBtn()
{
	emit MoveButtonPressed(1);
}

void QPatientDialog::OnShowDisplaySet(DisplaySet set)
{
    // 按照位置排序一次
    std::sort(set.displayItems.begin(), set.displayItems.end(), [](
        const DisplaySetItem& a, const DisplaySetItem& b) {
        return a.postion < b.postion;
    });

    // 清空当前显示的所有控件
    QLayoutItem *child;
    while ((child = ui->gridLayout->takeAt(0)) != 0) {
            QLayout* l = child->layout();

            if (l != Q_NULLPTR)
                SetLayoutVisible(l, false);
    }

    for (auto& item : set.displayItems)
        SetItemGeometryAndData(item, item.enable, item.postion);

    // 增加弹簧
    ui->gridLayout->addItem(m_bottomSpacer, ui->gridLayout->rowCount(), 0);
}

void QPatientDialog::OnPatientSettingUpdated(DisplaySet set)
{
    OnShowDisplaySet(set);
    ShowPatientInfo(m_sampleDb);
}

void QPatientDialog::OnDataDictInfoUpdate(/*std::string dictKey*/)
{
    UpdateComBoBoxItem();
    ShowPatientInfo(m_sampleDb);
}

void QPatientDialog::SetItemGeometryAndData(const DisplaySetItem& item, bool enable, int pos)
{
    // 如果不显示，或者控件位置不对，直接返回
    if (!enable || pos < 0)
        return;
    
    // 初始化基础数据
    const int columnCount = 4;          // 页面按照几列排列

    // 设置控件的几何位置
    int col = pos % 4;                  // 计算显示在第几列
    int row = pos / 4;                  // 计算显示在第几行

    // 确认是否有此控件
    const QString&& itemName = QString("patinetType_%1").arg(item.type);
    if (m_patientCtrls.find(itemName) == m_patientCtrls.end())
        return;

    QLayout* l = m_patientCtrls[itemName]->layout();

    if (l != Q_NULLPTR)
    {
        SetLayoutVisible(l, true);
        ui->gridLayout->addItem(m_patientCtrls[itemName], row, col);

        for (int i = 0; i < l->count(); ++i)
        {
            // 跳过异常子项
            QLayoutItem* childLay = l->itemAt(i);
            if (childLay == Q_NULLPTR || childLay->widget() == Q_NULLPTR)
                continue;

            // 设置字典标签名称
            QWidget* widget = childLay->widget();
            if (widget != Q_NULLPTR)
            {
                QLabel* label = qobject_cast<QLabel*>(widget);
                if (label)
                {
                    QString name;
                    if ((::tf::PatientFields::type)item.type < tf::PatientFields::PATIENT_CUSTOM_BASE)
                        name = std::move(ConvertTfEnumToQString((::tf::PatientFields::type)item.type));

                    // 如果没有获取到，则使用数据库的名称
                    if (name.isEmpty())
                        name = std::move(QString::fromStdString(item.name));

                    label->setText(name);
                }
            }
        }
    }
}

void QPatientDialog::OnSampleUpdate(tf::UpdateType::type enUpdateType, std::vector<tf::SampleInfo, std::allocator<tf::SampleInfo>> vSIs)
{
    // 患者信息界面显示过程中，不会出现样本被删除的情
    // 样本的测试状态可能会发生变化，（例如：正在测试 -> 测试完成）
    if (enUpdateType != ::tf::UpdateType::type::UPDATE_TYPE_MODIFY)
        return;

    for (const auto& s : vSIs)
    {
        if (s.id == m_sampleDb)
        {
            if (s.status != tf::SampleStatus::SAMPLE_STATUS_TESTED)
            {
                ui->sample_no->setEnabled(false);
                ui->sample_bar->setEnabled(false);
            }
            else
            {
                ui->sample_no->setEnabled(true);
                ui->sample_bar->setEnabled(true);
            }
        }
    }
}
