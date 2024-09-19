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


#include "CalibrateSettingWidget.h"
#include "ui_CalibrateSettingWidget.h"
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/CommonInformationManager.h"
#include "model/CaliVolsTableModel.h"
#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/thrift/ise/gen-cpp/ise_constants.h"
#include "manager/UserInfoManager.h"
#include <QtWidgets/QCheckBox>

#define  INVALID_ASSAY_CODE  (-1)

// 当发生错误时，弹出消息框后退出当前函数
#define OnErr(bErr, msg) \
if ((bErr))\
{\
	TipDlg(msg).exec();\
	return false;\
}

CalibrateSettingWidget::CalibrateSettingWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CalibrateSettingWidget),
    m_currentCode(-1),
    m_defaultSampleVol(0)
{
    ui->setupUi(this);
	InitCtrls();
}

CalibrateSettingWidget::~CalibrateSettingWidget()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     加载指定项目的分析参数
///
/// @param[in]  item  数据
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月24日，新建函数
///
bool CalibrateSettingWidget::LoadAnalysisParam(const AssayListModel::StAssayListRowItem& item)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 复位控件
	Reset();

	// 判断待加载项有效
	if (item.assayCode == INVALID_ASSAY_CODE)
	{
		return true;
	}

	// 查询通用项目信息
	const auto &CIM = CommonInformationManager::GetInstance();
	auto GAI = CIM->GetAssayInfo(item.assayCode);

    UpdateCtrlsEnabled(GAI == nullptr || GAI->openAssay);

    m_currentCode = item.assayCode;
	if (GAI == nullptr)
	{
		ULOG(LOG_WARN, "GetAssayInfo() Failed!");
		return false;
	}

	// 查询生化通用项目信息
	auto chGAI = CIM->GetChemistryAssayInfo(item.assayCode, (::tf::DeviceType::type)item.model, item.version.toStdString());
	if (chGAI == nullptr)
	{
		ULOG(LOG_ERROR, "Please save() Failed!");
		return false;
	}

	// 查询生化特殊项目信息
	auto speAssayInfos = CIM->GetChemistrySpecialAssayInfo(item.assayCode, (::tf::DeviceType::type)item.model, -1, item.version.toStdString());
	if (speAssayInfos.empty())
	{
		ULOG(LOG_ERROR, "GetChemistrySpecialAssayInfo() Failed!");
		return false;
	}

	std::shared_ptr<ch::tf::SpecialAssayInfo> chSAI;

	// 更新样本源下拉框
	for (auto ai : speAssayInfos)
	{
		if (ai->priority)
		{
			chSAI = ai;
		}
	}
	
	if (chSAI == nullptr)
	{
		ULOG(LOG_ERROR, "Get chSpecialAssayInfo Failed!");
		return false;
	}
    
	// 校准设置
    if (chGAI->caliQuantity > 1)
    {
        ui->caliTypeCbBox->setCurrentText(ConvertTfEnumToQString(chGAI->calibrationType));
        ui->caliPointsEdit->setText(QString::number(chGAI->caliQuantity));
    }
	
    // 加权系数
    ui->auto_weight_coeff_ckbox->setChecked(chGAI->enableWeightCoeff);

	// 校准结果管理
    int iData = chGAI->caliFactorRecordTime.iLotValidTime;
    ui->lot_valid_time_edit->setText(iData == 0 ? "" : QString::number(iData));

    iData = chGAI->caliFactorRecordTime.iBaseValidTime;
    ui->lot_base_time_edit->setText(iData == 0 ? "" : QString::number(iData));

	ui->cali_failed_ckbox->setChecked(chGAI->autoMaskWhenCaliFail);
    ui->lot_num_manager_ckbox->setChecked(chGAI->enableLotNumManage);

	// 换批
	ui->change_over_lot_cbox->setCurrentText(ConvertTfEnumToQString(chGAI->caliSuggest.changeOverLot));
	ui->change_over_lot_ckbox->setChecked(chGAI->caliSuggest.enableChangeOverLot);
	// 换瓶
	ui->change_over_bottle_cbox->setCurrentText(ConvertTfEnumToQString(chGAI->caliSuggest.changeOverBottle));
	ui->change_over_bottle_ckbox->setChecked(chGAI->caliSuggest.enableChangeOverBottle);
	
	// 瓶曲线
	ui->bottl_timeout_ckbox->setChecked(chGAI->caliSuggest.timeoutBottle.enable);
    iData = chGAI->caliSuggest.timeoutBottle.timeout;
	ui->cali_timeout_edit_bottle->setText(iData == 0 ? "" : QString::number(iData));
	ui->cali_timeout_mode_cbox_bottle->setCurrentText(ConvertTfEnumToQString(chGAI->caliSuggest.timeoutBottle.caliMode));
	
	//批曲线
	ui->lot_timeout_ckbox->setChecked(chGAI->caliSuggest.timeoutLot.enable);
    iData = chGAI->caliSuggest.timeoutLot.timeout;
	ui->cali_timeout_edit_lot->setText(iData == 0 ? "" : QString::number(iData));
	ui->cali_timeout_mode_cbox_lot->setCurrentText(ConvertTfEnumToQString(chGAI->caliSuggest.timeoutLot.caliMode));
	
	// 质控失控
	ui->single_qc_cali_mode_ckbox->setChecked(chGAI->caliSuggest.qcViolation.enableSingleCaliMode);
	ui->single_qc_cali_mode_cbox->setCurrentText(ConvertTfEnumToQString(chGAI->caliSuggest.qcViolation.singleCaliMode));
	ui->union_qc_cali_mode_ckbox->setChecked(chGAI->caliSuggest.qcViolation.enableUnionCaliMode);
	ui->union_qc_cali_mode_cbox->setCurrentText(ConvertTfEnumToQString(chGAI->caliSuggest.qcViolation.unionCaliMode));

	// 校准检查参数
    iData = chGAI->caliChkParam.offsetLimit.sdLimit;
	ui->offset_sd_limit_edit->setText(iData == INT_MAX ? "" : QString::number(iData));

    iData = chGAI->caliChkParam.s1AbsLimit.lower;
	ui->s1lbs_limit_lower_edit->setText(iData == INT_MIN ? "" : QString::number(iData));

    iData = chGAI->caliChkParam.s1AbsLimit.upper;
	ui->s1lbs_limit_upper_edit->setText(iData == INT_MAX ? "" : QString::number(iData));

    iData = chGAI->caliChkParam.duplicateLimit.rate;
	ui->dup_licate_limit_rate_edit->setText(iData == 0 ? "" : QString::number(iData));

    iData = chGAI->caliChkParam.duplicateLimit.abs;
	ui->dup_licate_limit_abs_edit->setText(iData == INT_MAX ? "" : QString::number(iData));

    iData = chGAI->caliChkParam.sensitiveLimit.lower;
	ui->sensitive_limit_lower_edit->setText(iData == INT_MIN ? "" : QString::number(iData));

    iData = chGAI->caliChkParam.sensitiveLimit.upper;
	ui->sensitive_limit_upper_edit->setText(iData == INT_MAX ? "" : QString::number(iData));

    double dData = chGAI->caliChkParam.kCoefficient;
	ui->caliConstEdit_K_5->setText(IsEqual(dData, 0.0) ? "" : UiCommon::DoubleToQString(dData));

	// 稀释浓度设定
	auto caliAspirateVols = chGAI->caliAspirateVols;
	caliAspirateVols.resize(chGAI->caliQuantity);
	CaliVolsTableModel::Instance().SetData(caliAspirateVols);

	return true;
}

bool CalibrateSettingWidget::GetAnalysisParam(ch::tf::GeneralAssayInfo& chGAI, int assayCode)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 校准设置
	bool bOk = false;
	chGAI.__set_calibrationType((ch::tf::CalibrationType::type)ui->caliTypeCbBox->currentData().toInt());
	OnErr(ui->caliTypeCbBox->currentIndex() == -1, tr("值错误：校准类型"));
    int iData = ui->caliPointsEdit->text().toInt(&bOk);
    OnErr(iData < 2 || iData > 6, tr("值错误：2≤校准点≤6"));
	chGAI.__set_caliQuantity(iData);

    // 加权系数
    chGAI.__set_enableWeightCoeff(ui->auto_weight_coeff_ckbox->isChecked());

	// 校准检查参数
	ch::tf::CaliCheckParam CCP;

    QString strData = ui->offset_sd_limit_edit->text();
	CCP.offsetLimit.sdLimit = strData.isEmpty() ? INT_MAX : strData.toInt(&bOk);
	OnErr(!bOk, tr("值错误：标准差限值检查值"));

    strData = ui->s1lbs_limit_lower_edit->text();
	CCP.s1AbsLimit.lower = strData.isEmpty() ? INT_MIN : strData.toInt(&bOk);
	OnErr(!bOk, tr("值错误：S1吸光度限值检查值吸光度的下限"));

    strData = ui->s1lbs_limit_upper_edit->text();
    CCP.s1AbsLimit.upper = strData.isEmpty() ? INT_MAX : strData.toInt(&bOk);
    OnErr(!bOk, tr("值错误：S1吸光度限值检查值吸光度的上限"));
    OnErr(CCP.s1AbsLimit.upper < CCP.s1AbsLimit.lower && !ui->s1lbs_limit_lower_edit->text().isEmpty()
            && !ui->s1lbs_limit_upper_edit->text().isEmpty(),tr("值错误：S1吸光度限值检查值的上限不能小于下限"));

    OnErr(CCP.s1AbsLimit.lower == INT_MIN && CCP.s1AbsLimit.upper != INT_MAX
            , tr("请输入S1吸光度限值检查值的下限"));

    OnErr(CCP.s1AbsLimit.lower != INT_MIN && CCP.s1AbsLimit.upper == INT_MAX
        , tr("请输入S1吸光度限值检查值的上限"));

    OnErr(CheckEditorIsZero(ui->dup_licate_limit_rate_edit), tr("值错误：重复性限制检查不能为0"));
	CCP.duplicateLimit.rate = ui->dup_licate_limit_rate_edit->text().toInt();

    strData = ui->dup_licate_limit_abs_edit->text();
	CCP.duplicateLimit.abs = strData.isEmpty() ? INT_MAX : strData.toInt(&bOk);
	OnErr(!bOk, tr("值错误：重复性限制检查值"));

    strData = ui->sensitive_limit_lower_edit->text();
	CCP.sensitiveLimit.lower = strData.isEmpty() ? INT_MIN : strData.toInt(&bOk);
	OnErr(!bOk, tr("值错误：灵敏度限值检查的下限"));

    strData = ui->sensitive_limit_upper_edit->text();
	CCP.sensitiveLimit.upper = strData.isEmpty() ? INT_MAX : strData.toInt(&bOk);
	OnErr(!bOk, tr("值错误：灵敏度限值检查的上限"));
    OnErr(!ui->sensitive_limit_upper_edit->text().isEmpty() && !ui->sensitive_limit_lower_edit->text().isEmpty()
        && CCP.sensitiveLimit.upper < CCP.sensitiveLimit.lower,tr("值错误：灵敏度限值检查的上限不能小于下限"));

    OnErr(CCP.sensitiveLimit.lower == INT_MIN && CCP.sensitiveLimit.upper != INT_MAX
        , tr("请输入灵敏度限值检查的下限"));

    OnErr(CCP.sensitiveLimit.lower != INT_MIN && CCP.sensitiveLimit.upper == INT_MAX
        , tr("请输入灵敏度限值检查的上限"));

	CCP.kCoefficient = ui->caliConstEdit_K_5->text().toDouble();
    OnErr(!ui->caliConstEdit_K_5->text().isEmpty() && IsEqual(CCP.kCoefficient, 0.0),
            tr("值错误：K系数差别不能为0"));

	chGAI.__set_caliChkParam(CCP);

	// 校准结果
    // 成为批曲线时间限必须大于0
    QString tempData = ui->lot_valid_time_edit->text();
    OnErr(!tempData.isEmpty() && tempData.toInt() <= 0, tr("值错误：成为批曲线时间限必须大于0"));
	ch::tf::CaliFactorRecordTime CFRT;
	CFRT.iLotValidTime = tempData.toInt();

    // 成为基准曲线时间限必须大于0
    tempData = ui->lot_base_time_edit->text();
    OnErr(!tempData.isEmpty() && tempData.toInt() <= 0, tr("值错误：成为基准曲线时间限必须大于0"));
    CFRT.iBaseValidTime = tempData.toInt();

	chGAI.__set_caliFactorRecordTime(CFRT);
    chGAI.__set_enableLotNumManage(ui->lot_num_manager_ckbox->isChecked());
    OnErr(!chGAI.enableLotNumManage
        && chGAI.assayCode >= ::tf::AssayCodeRange::CH_RANGE_MIN
        && chGAI.assayCode < ::tf::AssayCodeRange::CH_OPEN_RANGE_MIN
        , tr("值错误：封闭项目必须启用批号管理"));

	// 校准屏蔽
	chGAI.__set_autoMaskWhenCaliFail(ui->cali_failed_ckbox->isChecked());

	// 校准建议
	ch::tf::CaliSuggest CS;

	CS.__set_enableChangeOverLot(ui->change_over_lot_ckbox->isChecked());
	CS.__set_changeOverLot((tf::CaliMode::type)ui->change_over_lot_cbox->currentData().toInt());
	CS.__set_enableChangeOverBottle(ui->change_over_bottle_ckbox->isChecked());
	CS.__set_changeOverBottle((tf::CaliMode::type)ui->change_over_bottle_cbox->currentData().toInt());

    // 瓶校准为智能校准时批校准也必须为智能校准
    OnErr(CS.changeOverBottle != CS.changeOverLot
        && CS.changeOverBottle == ::tf::CaliMode::CALI_MODE_INTELLIGENT
        , tr("值错误：换瓶校准的方法设置为智能校准，换批校准的方法也必须为智能校准"));
    
    OnErr(CS.changeOverBottle != CS.changeOverLot
        && CS.changeOverLot == ::tf::CaliMode::CALI_MODE_INTELLIGENT
        , tr("值错误：换批校准的方法设置为智能校准，换瓶校准的方法也必须为智能校准"));

	// 瓶曲线
    OnErr(CheckEditorIsZero(ui->cali_timeout_edit_bottle), tr("值错误：瓶校准周期不能为0"));
	CS.timeoutBottle.enable = ui->bottl_timeout_ckbox->isChecked();
	CS.timeoutBottle.timeout = ui->cali_timeout_edit_bottle->text().toInt();
	CS.timeoutBottle.timeUnit = tf::TimeUnit::TIME_UNIT_DAY;

	OnErr(ui->cali_timeout_mode_cbox_bottle->currentIndex() == -1, tr("值错误： 瓶曲线校准执行方法"));
    CS.timeoutBottle.caliMode = (::tf::CaliMode::type)ui->cali_timeout_mode_cbox_bottle->currentData().toInt();

	// 批曲线
    OnErr(CheckEditorIsZero(ui->cali_timeout_edit_lot), tr("值错误：批校准周期不能为0"));
	CS.timeoutLot.enable = ui->lot_timeout_ckbox->isChecked();
    CS.timeoutLot.timeout = ui->cali_timeout_edit_lot->text().toInt();
	CS.timeoutLot.timeUnit = tf::TimeUnit::TIME_UNIT_DAY;

	OnErr(ui->cali_timeout_mode_cbox_lot->currentIndex() == -1, tr("值错误： 批曲线校准执行方法"));
    CS.timeoutLot.caliMode = (::tf::CaliMode::type)ui->cali_timeout_mode_cbox_lot->currentData().toInt();

	// 质控失控
	::tf::QcViolation qcViolation;

	qcViolation.enableSingleCaliMode = ui->single_qc_cali_mode_ckbox->isChecked();

	OnErr(ui->single_qc_cali_mode_cbox->currentIndex() == -1, tr("值错误： 单质控品失控校准方法"));
	qcViolation.singleCaliMode = (::tf::CaliMode::type)ui->single_qc_cali_mode_cbox->currentData().toInt();

	qcViolation.enableUnionCaliMode = ui->union_qc_cali_mode_ckbox->isChecked();

	OnErr(ui->union_qc_cali_mode_cbox->currentIndex() == -1, tr("值错误： 联合质控品失控校准方法"));
    qcViolation.unionCaliMode = (::tf::CaliMode::type)ui->union_qc_cali_mode_cbox->currentData().toInt();

	CS.__set_qcViolation(qcViolation);

	chGAI.__set_caliSuggest(CS);

	// 稀释浓度设置
    auto sav = CaliVolsTableModel::Instance().GetData();
    for (auto& item : sav)
    {
        // 样本量不能为0
        OnErr(item.originalSample <= 0, tr("值错误：稀释浓度设置的样本量必须大于0"));
    }

    if (!UiCommon::Instance()->CheckReagentVolumeIsValid(chGAI.reagentAspirateVols, sav, true))
    {
        return false;
    }
    
    chGAI.__set_caliAspirateVols(sav);
    
	return true;
}

void CalibrateSettingWidget::Reset()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    for (auto child : findChildren<QLineEdit *>())
    {
        child->clear();
    }

    for (auto child : findChildren<QComboBox *>())
    {
        child->setCurrentIndex(0);
    }

    for (auto child : findChildren<QCheckBox *>())
    {
        child->setChecked(false);
    }

    ui->lot_num_manager_ckbox->setChecked(true);
    std::vector<ch::tf::SampleAspirateVol> data;
    CaliVolsTableModel::Instance().SetData(data);
}

///
/// @brief
///     初始化控件
///
/// @par History:
/// @li 6950/ChenFei，2022年05月23日，新建函数
///
void CalibrateSettingWidget::InitCtrls()
{
	// 校准方法
    ui->caliTypeCbBox->clear();
    AddTfEnumItemToComBoBox(ui->caliTypeCbBox, ::ch::tf::CalibrationType::CALIBRATION_TYPE_LINEAR_TWO_POINTS);
    AddTfEnumItemToComBoBox(ui->caliTypeCbBox, ::ch::tf::CalibrationType::CALIBRATION_TYPE_LINEAR_MULTI_POINTS);
    AddTfEnumItemToComBoBox(ui->caliTypeCbBox, ::ch::tf::CalibrationType::CALIBRATION_TYPE_LOGIT_LOG3P);
    AddTfEnumItemToComBoBox(ui->caliTypeCbBox, ::ch::tf::CalibrationType::CALIBRATION_TYPE_LOGIT_LOG4P);
    AddTfEnumItemToComBoBox(ui->caliTypeCbBox, ::ch::tf::CalibrationType::CALIBRATION_TYPE_LOGIT_LOG5P);
    AddTfEnumItemToComBoBox(ui->caliTypeCbBox, ::ch::tf::CalibrationType::CALIBRATION_TYPE_EXPONENTIAL);
    AddTfEnumItemToComBoBox(ui->caliTypeCbBox, ::ch::tf::CalibrationType::CALIBRATION_TYPE_SPLINE);
    AddTfEnumItemToComBoBox(ui->caliTypeCbBox, ::ch::tf::CalibrationType::CALIBRATION_TYPE_PCHIP);

	for (auto box : QWidget::findChildren<QComboBox*>())
	{
		if (box->objectName() == "caliTypeCbBox" || box->objectName() == "weight_coeff_method_combox")
		{
			continue;
		}

        // 自动校准设置的方法不可选择不设置，bug10635
		//AddTfEnumItemToComBoBox(box, ::tf::CaliMode::CALI_MODE_NONE); 
		AddTfEnumItemToComBoBox(box, ::tf::CaliMode::CALI_MODE_BLANK);
		AddTfEnumItemToComBoBox(box, ::tf::CaliMode::CALI_MODE_FULLDOT);
	}

    AddTfEnumItemToComBoBox(ui->change_over_bottle_cbox, ::tf::CaliMode::CALI_MODE_INTELLIGENT);
    AddTfEnumItemToComBoBox(ui->change_over_lot_cbox, ::tf::CaliMode::CALI_MODE_INTELLIGENT);

	// 稀释浓度设置表
	ui->caliVolsView->verticalHeader()->setVisible(false);
	ui->caliVolsView->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->caliVolsView->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->caliVolsView->setModel(&CaliVolsTableModel::Instance());
	ui->caliVolsView->horizontalHeader()->setMinimumSectionSize(123);
    ComboBoxDelegate* dgt = new ComboBoxDelegate(ui->caliVolsView);
    dgt->setItemText(QStringList({"0","1", "2", "3", "4", "5", "6", "7","8","9", "10", "11", "12", "13", "14", "15", "16"}));
    ui->caliVolsView->setItemDelegateForColumn((int)CaliVolsTableModel::COL::LEVEL, dgt);

	SetCtrlsRegExp();

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);

	connect(ui->caliPointsEdit, &QLineEdit::textChanged, this, &CalibrateSettingWidget::OnsetTableViewRows);
}

void CalibrateSettingWidget::SetCtrlsRegExp()
{
    // 校准点
    ui->caliPointsEdit->setValidator(new QRegExpValidator(QRegExp("^([2-6])$")));
    // 成为批曲线时间限(小时)
    ui->lot_valid_time_edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_POSITIVE_INT)));
    // 成为基准曲线时间限(小时)
    ui->lot_base_time_edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_POSITIVE_INT)));
    // 瓶校准周期(天)
    ui->cali_timeout_edit_bottle->setValidator(new QRegExpValidator(QRegExp(UI_REG_POSITIVE_INT)));
    // 批校准周期(天)
    ui->cali_timeout_edit_lot->setValidator(new QRegExpValidator(QRegExp(UI_REG_POSITIVE_INT)));

    // 偏移允许吸光度
    ui->offset_sd_limit_edit->setValidator(new QIntValidator(ui->offset_sd_limit_edit));
    // 校准品1吸光度范围
    ui->s1lbs_limit_lower_edit->setValidator(new QIntValidator(ui->s1lbs_limit_lower_edit));
    ui->s1lbs_limit_upper_edit->setValidator(new QIntValidator(ui->s1lbs_limit_upper_edit));
    // 离散性允许吸光度
    ui->dup_licate_limit_rate_edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_POSITIVE_INT)));
    ui->dup_licate_limit_abs_edit->setValidator(new QIntValidator(ui->dup_licate_limit_abs_edit));
    // 灵敏度允许吸光度
    ui->sensitive_limit_lower_edit->setValidator(new QIntValidator(ui->sensitive_limit_lower_edit));
    ui->sensitive_limit_upper_edit->setValidator(new QIntValidator(ui->sensitive_limit_upper_edit));
    // K系数差别
    ui->caliConstEdit_K_5->setValidator(new QDoubleValidator(0.0, 99999999.99, 2,ui->caliConstEdit_K_5));	
}

void CalibrateSettingWidget::UpdateCtrlsEnabled(bool isOpenAssay)
{
    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();

    // 设置校准方法和校准点
    ui->caliTypeCbBox->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_CALI_METHOD));
    ui->caliPointsEdit->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_CALI_POINT));
    // 自动加权系数
    ui->auto_weight_coeff_ckbox->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_WEIGHT_COEFF));
    // 成为基准曲线时间限
    ui->lot_base_time_edit->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_BASE_TIEM_LIMIT));
    // 成为批曲线时间限
    ui->lot_valid_time_edit->setEnabled(isOpenAssay || userPms->IsPermisson(PMS_BATCH_CURVL_TIME_LIMIT));
    // 瓶校准周期
    ui->cali_timeout_edit_bottle->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_TIMEOUT_BOTTLE));
    // 批校准周期
    ui->cali_timeout_edit_lot->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_TIMEOUT_LOT));
    // 批号管理
    ui->lot_num_manager_ckbox->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_LOT_NUM));
    // 校准屏蔽
    ui->cali_failed_ckbox->setEnabled(isOpenAssay || userPms->IsPermisson(PMS_CALI_SHEILD));
    // 自动校准设置
    ui->CalSetting_change_over->setEnabled(isOpenAssay || userPms->IsPermisson(PMS_SETTINGS_AUTO_CALI));
    ui->groupBox_5->setEnabled(isOpenAssay || userPms->IsPermisson(PMS_SETTINGS_AUTO_CALI));
    ui->groupBox_7->setEnabled(isOpenAssay || userPms->IsPermisson(PMS_SETTINGS_AUTO_CALI));
    // 稀释浓度设置
    ui->CalSetting_caliVols->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_CALI_VOLS));
    // 校准检查范围
    ui->CalSetting_range->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_CALI_RANGE));
}

bool CalibrateSettingWidget::HasUnSaveChangedData(const AssayListModel::StAssayListRowItem& item)
{
    const auto& CIM = CommonInformationManager::GetInstance();
    const auto& spCai = CIM->GetChemistryAssayInfo(item.assayCode,
            (::tf::DeviceType::type)item.model, item.version.toStdString());

    if (spCai == nullptr)
    {
        ULOG(LOG_INFO, "Please save failed");
        return true;
    }

    // 校准方法、校准点
    if (spCai->calibrationType != ui->caliTypeCbBox->currentData().toInt()
        || spCai->caliQuantity != ui->caliPointsEdit->text().toInt())
    {
        ULOG(LOG_INFO, "Please save calibrationType and caliQuantity");
        return true;
    }

    // 自动加权系数
    if (spCai->enableWeightCoeff != ui->auto_weight_coeff_ckbox->isChecked())
    {
        ULOG(LOG_INFO, "Please save enableWeightCoeff");
        return true;
    }

    // 成为基准曲线时间限、成为批曲线时间限
    if (spCai->caliFactorRecordTime.iBaseValidTime != ui->lot_base_time_edit->text().toInt()
        || spCai->caliFactorRecordTime.iLotValidTime != ui->lot_valid_time_edit->text().toInt())
    {
        ULOG(LOG_INFO, "Please save caliFactorRecordTime");
        return true;
    }

    // 瓶校准周期、批校准周期
    if (spCai->caliSuggest.timeoutBottle.timeout != ui->cali_timeout_edit_bottle->text().toInt()
        || spCai->caliSuggest.timeoutLot.timeout != ui->cali_timeout_edit_lot->text().toInt())
    {
        ULOG(LOG_INFO, "Please save timeoutBottle and  timeoutLot");
        return true;
    }

    // 批号管理、校准屏蔽
    if (spCai->enableLotNumManage != ui->lot_num_manager_ckbox->isChecked()
        || spCai->autoMaskWhenCaliFail != ui->cali_failed_ckbox->isChecked())
    {
        ULOG(LOG_INFO, "Please save enableLotNumManage and autoMaskWhenCaliFail");
        return true;
    }

    // 换瓶校准、换批校准
    if (spCai->caliSuggest.enableChangeOverBottle != ui->change_over_bottle_ckbox->isChecked()
        || spCai->caliSuggest.enableChangeOverLot != ui->change_over_lot_ckbox->isChecked()
        || spCai->caliSuggest.changeOverBottle != ui->change_over_bottle_cbox->currentData().toInt()
        || spCai->caliSuggest.changeOverLot != ui->change_over_lot_cbox->currentData().toInt())
    {
        ULOG(LOG_INFO, "Please save change bottle and lot");
        return true;
    }

    //  瓶曲线过期、批曲线过期
    if (spCai->caliSuggest.timeoutBottle.enable != ui->bottl_timeout_ckbox->isChecked()
        || spCai->caliSuggest.timeoutLot.enable != ui->lot_timeout_ckbox->isChecked()
        || spCai->caliSuggest.timeoutBottle.caliMode != ui->cali_timeout_mode_cbox_bottle->currentData().toInt()
        || spCai->caliSuggest.timeoutLot.caliMode != ui->cali_timeout_mode_cbox_lot->currentData().toInt())
    {
        ULOG(LOG_INFO, "Please save timeout bottle and lot");
        return true;
    }

    // 单质控品失控、联合质控品失控	
    if (spCai->caliSuggest.qcViolation.enableSingleCaliMode != ui->single_qc_cali_mode_ckbox->isChecked()
        || spCai->caliSuggest.qcViolation.enableUnionCaliMode != ui->union_qc_cali_mode_ckbox->isChecked()
        || spCai->caliSuggest.qcViolation.singleCaliMode != ui->single_qc_cali_mode_cbox->currentData().toInt()
        || spCai->caliSuggest.qcViolation.unionCaliMode != ui->union_qc_cali_mode_cbox->currentData().toInt())
    {
        ULOG(LOG_INFO, "Please save qcViolation");
        return true;
    }

    // 稀释浓度设置
    if (spCai->caliAspirateVols != CaliVolsTableModel::Instance().GetData())
    {
        ULOG(LOG_INFO, "Please save caliAspirateVols");
        return true;
    }

    // 标准差限值检查
    QString strData = ui->offset_sd_limit_edit->text();
    if (spCai->caliChkParam.offsetLimit.sdLimit != (strData.isEmpty() ? INT_MAX : strData.toInt()))
    {
        ULOG(LOG_INFO, "Please save offsetLimit sdLimit");
        return true;
    }

    // S1吸光度限值检查
    strData = ui->s1lbs_limit_lower_edit->text();
    QString  strDataUp = ui->s1lbs_limit_upper_edit->text();
    if (spCai->caliChkParam.s1AbsLimit.lower != (strData.isEmpty() ? INT_MIN : strData.toInt())
        || spCai->caliChkParam.s1AbsLimit.upper != (strDataUp.isEmpty() ? INT_MAX : strDataUp.toInt()))
    {
        ULOG(LOG_INFO, "Please save caliChkParam s1AbsLimit");
        return true;
    }

    // 重复性限制检查
    strData = ui->dup_licate_limit_abs_edit->text();
    if (spCai->caliChkParam.duplicateLimit.rate != ui->dup_licate_limit_rate_edit->text().toInt()
        || spCai->caliChkParam.duplicateLimit.abs != (strData.isEmpty() ? INT_MAX : strData.toInt()))
    {
        ULOG(LOG_INFO, "Please save caliChkParam duplicateLimit");
        return true;
    }

    // 灵敏度限值检查
    strData = ui->sensitive_limit_lower_edit->text();
    strDataUp = ui->sensitive_limit_upper_edit->text();
    if (spCai->caliChkParam.sensitiveLimit.lower != (strData.isEmpty() ? INT_MIN : strData.toInt())
        || spCai->caliChkParam.sensitiveLimit.upper != (strDataUp.isEmpty() ? INT_MAX : strDataUp.toInt()))
    {
        ULOG(LOG_INFO, "Please save caliChkParam sensitiveLimit");
        return true;
    }

    // K系数差别
    if (spCai->caliChkParam.kCoefficient != ui->caliConstEdit_K_5->text().toDouble())
    {
        ULOG(LOG_INFO, "Please save kCoefficient");
        return true;
    }

    return false;
}

void CalibrateSettingWidget::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    // 查询通用项目信息
    const auto &CIM = CommonInformationManager::GetInstance();
    auto GAI = CIM->GetAssayInfo(m_currentCode);
    UpdateCtrlsEnabled(GAI == nullptr || GAI->openAssay);
}

///
/// @brief
///     设置表格行数
///
/// @par History:
/// @li 7951/LuoXin，2022年6月23日，新建函数
///
void CalibrateSettingWidget::OnsetTableViewRows(const QString& text)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (text.toInt() >= 0)
    {
        // 设置默认的稀释浓度样本量
		auto caliAspirateVols = CaliVolsTableModel::Instance().GetData();
		caliAspirateVols.resize(text.toInt());
        for (auto&item : caliAspirateVols)
        {
            if (item.originalSample <= 0)
            {
                item.originalSample = m_defaultSampleVol;
            }
        }

		CaliVolsTableModel::Instance().SetData(caliAspirateVols);
	} 
}

void CalibrateSettingWidget::OnDefaultSampleVolChanged(double vol)
{
    m_defaultSampleVol = vol * 10;

    OnsetTableViewRows(ui->caliPointsEdit->text());
}
