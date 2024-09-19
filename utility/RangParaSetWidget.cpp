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
/// @file     RangeParaWidget.h
/// @brief    项目参考范围参数配置
///
/// @author   7951/LuoXin
/// @date     2022年6月13日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年6月13日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "RangParaSetWidget.h"
#include "ui_RangParaSetWidget.h"
#include "QPushButton"
#include "QComboBox"
#include <QTimer>

#include "shared/uicommon.h"
#include "shared/tipdlg.h"
#include "shared/uidef.h"
#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"

// 当发生错误时，弹出消息框后退出当前函数
#define ReportErr(bErr, msg) \
if ((bErr))\
{\
	TipDlg(msg).exec();\
	return false;\
}


CRangParaSetWidget::CRangParaSetWidget(RangeSetParamDlgType rangeDlgType, QWidget *parent)
    : BaseDlg(parent)
    , m_rangeDlgType(rangeDlgType)
    , m_defaultSampleSource(-1)
{
    ui = new Ui::CRangParaSetWidget();
    ui->setupUi(this);

    // 输入限制
    ui->lower_age_edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_POSITIVE_INT)));
    ui->upper_age_edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_POSITIVE_INT)));
    ui->ref_lower_age_edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_FLOAT_RANG)));
    ui->ref_upper_age_edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_FLOAT_RANG)));   
    ui->ref_critical_lower_rang->setValidator(new QRegExpValidator(QRegExp(UI_REG_FLOAT_RANG)));  
    ui->ref_critical_upper_rang->setValidator(new QRegExpValidator(QRegExp(UI_REG_FLOAT_RANG)));

    initCombox();

    // 安装事件过滤器
    ui->str_refrange_edit->installEventFilter(this);
    ui->ref_lower_age_edit->installEventFilter(this);
    ui->ref_upper_age_edit->installEventFilter(this);
    ui->ref_critical_lower_rang->installEventFilter(this);
    ui->ref_critical_upper_rang->installEventFilter(this);

    // 编辑框类型处理
    if (rangeDlgType != RPD_Im)
    {
        ui->str_refrange_edit->hide();
        ui->verticalLayout->addSpacing(58);
        this->resize(this->width(), this->height() - 58);
        this->update();
    }
}

CRangParaSetWidget::~CRangParaSetWidget()
{
}

bool CRangParaSetWidget::CheckUserInputData()
{
    // 缺省设置被选中
    bool isSetDefault = ui->auto_default_cbox->currentText() == STR_AUTO_DEFAULT_Y;
    if (isSetDefault)
    {
        ReportErr((!ui->lower_age_edit->text().isEmpty() || !ui->upper_age_edit->text().isEmpty()), tr("值错误：缺省时不设置年龄范围"));
        ReportErr(((tf::Gender::type)ui->gender_cbox->currentIndex() != tf::Gender::type::GENDER_UNKNOWN), tr("值错误：缺省时性别设置为未知"));
    }
    else
    {
        // 年龄      
        ReportErr(ui->lower_age_edit->text().isEmpty(), tr("请输入年龄下限"));
        ReportErr(ui->upper_age_edit->text().isEmpty(), tr("请输入年龄上限"));
        ReportErr(ui->age_unit_cbox->currentText().isEmpty() , tr("请选择年龄单位"));
        ReportErr(ui->lower_age_edit->text().toInt() > ui->upper_age_edit->text().toInt(), tr("值错误：年龄下限不能大于年龄上限"));
    }

    // 危急范围
    if (!ui->ref_critical_lower_rang->text().isEmpty() && !ui->ref_critical_upper_rang->text().isEmpty())
    {
        double reflower = ui->ref_critical_lower_rang->text().toDouble();
        double refupper = ui->ref_critical_upper_rang->text().toDouble();
        ReportErr((reflower > refupper) || IsEqual(reflower, refupper), tr("值错误：危急低值必须小于危急高值"));
    }

    // 参考范围
    if (!ui->ref_lower_age_edit->text().isEmpty() || !ui->ref_upper_age_edit->text().isEmpty())
    {
        double reflower = ui->ref_lower_age_edit->text().toDouble();
        double refupper = ui->ref_upper_age_edit->text().toDouble();
        ReportErr(ui->ref_lower_age_edit->text().isEmpty(), tr("请输入参考值范围下限"));
        ReportErr(ui->ref_upper_age_edit->text().isEmpty(), tr("请输入参考值范围上限"));
        ReportErr((reflower > refupper) || IsEqual(reflower, refupper), tr("值错误：参考值范围下限必须小于参考值范围上限"));
    }

    ReportErr(ui->str_refrange_edit->text().isEmpty() && ui->ref_critical_lower_rang->text().isEmpty() && ui->ref_critical_upper_rang->text().isEmpty() &&
        ui->ref_lower_age_edit->text().isEmpty() && ui->ref_upper_age_edit->text().isEmpty(), tr("请输入参考值范围或者危急范围"));

    return true;
}

bool CRangParaSetWidget::getSetParaData(tf::AssayReferenceItem &item)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (!CheckUserInputData())
    {
        ULOG(LOG_INFO, "The User Data Invalid");
        return false;
    }

    // 是否缺省
    item.__set_bAutoDefault(ui->auto_default_cbox->currentText() == STR_AUTO_DEFAULT_Y);

    // 样本类型
    item.__set_sampleSourceType(ui->samp_source_cbox->currentData().toInt());

    // 性别
    item.__set_enGender((tf::Gender::type)ui->gender_cbox->currentData().toInt());

    // 年龄下限
    QString strAgelower = ui->lower_age_edit->text();
    item.__set_iLowerAge(strAgelower.isEmpty() ? INT_MIN : strAgelower.toInt());

    // 年龄上限
    QString strAgeupper = ui->upper_age_edit->text();
    item.__set_iUpperAge(strAgeupper.isEmpty() ? INT_MAX : strAgeupper.toInt());

    // 年龄单位
    item.__set_enAgeUnit((tf::AgeUnit::type)ui->age_unit_cbox->currentIndex());

    // 参考范围
    QString strRefLower = ui->ref_lower_age_edit->text();
    item.__set_fLowerRefRang(strRefLower.isEmpty() ? std::numeric_limits<double>::lowest() : strRefLower.toDouble());
    QString strRefUpper = ui->ref_upper_age_edit->text();
    item.__set_fUpperRefRang(strRefUpper.isEmpty() ? DBL_MAX : strRefUpper.toDouble());

    //危急范围
    QString strCriticalLower = ui->ref_critical_lower_rang->text();
    item.__set_fLowerCrisisRang(strCriticalLower.isEmpty() ? std::numeric_limits<double>::lowest() : strCriticalLower.toDouble());
    QString strCriticalUpper = ui->ref_critical_upper_rang->text();
    item.__set_fUpperCrisisRang(strCriticalUpper.isEmpty() ? DBL_MAX : strCriticalUpper.toDouble());


    // 参考值范围应该在危急值之外
    // 2024年3月11日与何洋确定，危急值不包含边界，参考值包含边界
    ReportErr(item.fLowerRefRang != std::numeric_limits<double>::lowest() && 
        (item.fLowerRefRang < item.fLowerCrisisRang || item.fLowerRefRang > item.fUpperCrisisRang),
        tr("值错误：参考值范围的低值应大于等于危机低值，且小于等于危机高值，即需满足危急低值≤参考值范围的低值≤危急高值"));

    ReportErr(item.fUpperRefRang != DBL_MAX &&
        (item.fUpperRefRang < item.fLowerCrisisRang || item.fUpperRefRang > item.fUpperCrisisRang),
        tr("值错误：参考值范围的高值应大于等于危机低值，且小于等于危机高值，即需满足危急低值≤参考值范围的高值≤危急高值"));

    item.__set_strRefRange((ui->str_refrange_edit->text()).toStdString());

    return true;
}

void CRangParaSetWidget::initCombox()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 样本类型
    ui->samp_source_cbox->clear();
    ui->samp_source_cbox->addItem(TransSampleName(-1), -1);
    UiCommon::Instance()->AssignSampleCombox(ui->samp_source_cbox);

    // 性别
    AddTfEnumItemToComBoBox(ui->gender_cbox, ::tf::Gender::GENDER_MALE);
    AddTfEnumItemToComBoBox(ui->gender_cbox, ::tf::Gender::GENDER_FEMALE);
    AddTfEnumItemToComBoBox(ui->gender_cbox, ::tf::Gender::GENDER_UNKNOWN);

    // 年龄单位
    for (auto each : ::tf::_AgeUnit_VALUES_TO_NAMES)
    {
        auto type = (::tf::AgeUnit::type)each.first;
        AddTfEnumItemToComBoBox(ui->age_unit_cbox, type);
    }

    // 缺省值标记下拉框
    connect(ui->auto_default_cbox, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentTextChanged),
                this, &CRangParaSetWidget::OnDefaultValueFlagChanged);
}

QString CRangParaSetWidget::TransSampleName(int iSampleType)
{
    if (iSampleType == -1)
    {
        return (m_rangeDlgType == RPD_Im) ? "" : tr("全部");
    }

    return ConvertTfEnumToQString((::tf::SampleSourceType::type)(iSampleType));
}

void CRangParaSetWidget::SetSampleTypeCommbox(int sampleSource)
{
    m_defaultSampleSource = sampleSource;

    ui->samp_source_cbox->setCurrentText(TransSampleName(sampleSource));
    ui->samp_source_cbox->setEnabled(sampleSource == -1);
}

QPushButton * CRangParaSetWidget::getSaveBtn()
{
    return ui->ok_btn;
}

void CRangParaSetWidget::loadRangParaSetWidget(const tf::AssayReferenceItem &item)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 是否缺省
    if (item.bAutoDefault)
    {
        ui->auto_default_cbox->setCurrentText(STR_AUTO_DEFAULT_Y);

        // 性别
        ui->gender_cbox->setCurrentIndex(ui->gender_cbox->findData((int)::tf::Gender::GENDER_UNKNOWN));
        // 年龄范围
        ui->lower_age_edit->clear();
        ui->upper_age_edit->clear();
        ui->age_unit_cbox->setCurrentIndex(-1);
    }
    else
    {
        ui->auto_default_cbox->setCurrentText(STR_AUTO_DEFAULT_N);

        // 性别
        ui->gender_cbox->setCurrentIndex(ui->gender_cbox->findData((int)item.enGender));
        // 年龄范围
        ui->lower_age_edit->setText(item.iLowerAge == INT_MIN ? "" : QString::number(item.iLowerAge));
        ui->upper_age_edit->setText(item.iUpperAge == INT_MAX ? "" : QString::number(item.iUpperAge));
        ui->age_unit_cbox->setCurrentIndex(item.enAgeUnit);
    }

    // 样本类型
    ui->samp_source_cbox->setCurrentIndex(ui->samp_source_cbox->findData(item.sampleSourceType));

    // 参考范围
    ui->ref_lower_age_edit->setText(item.fLowerRefRang == std::numeric_limits<double>::lowest() ? "" : UiCommon::DoubleToQString(item.fLowerRefRang));
    ui->ref_upper_age_edit->setText(item.fUpperRefRang == DBL_MAX ? "" : UiCommon::DoubleToQString(item.fUpperRefRang));
    
    // 危急范围
    ui->ref_critical_lower_rang->setText(item.fLowerCrisisRang == std::numeric_limits<double>::lowest() ? "" : UiCommon::DoubleToQString(item.fLowerCrisisRang));
    ui->ref_critical_upper_rang->setText(item.fUpperCrisisRang == DBL_MAX ? "" : UiCommon::DoubleToQString(item.fUpperCrisisRang));

    // 参考范围
    ui->str_refrange_edit->setText(QString::fromStdString(item.strRefRange));
}

bool CRangParaSetWidget::eventFilter(QObject *obj, QEvent *event)
{
    // 如果对象为参考范围编辑框
    if ((obj == ui->str_refrange_edit) && (event->type() == QEvent::MouseButtonPress))
    {
        if (!ui->ref_lower_age_edit->text().isEmpty() || !ui->ref_upper_age_edit->text().isEmpty() ||
            !ui->ref_critical_lower_rang->text().isEmpty() || !ui->ref_critical_upper_rang->text().isEmpty())
        {
            // 通知弹框提示
            QTimer::singleShot(0, this, SLOT(OnRefRangeEditFocusIn()));
        }
    }

    // 如果对象为参考范围编辑框
    if (((obj == ui->ref_lower_age_edit) || (obj == ui->ref_upper_age_edit) ||
        (obj == ui->ref_critical_lower_rang) || (obj == ui->ref_critical_upper_rang)) &&
        (event->type() == QEvent::MouseButtonPress))
    {
        if (!ui->str_refrange_edit->text().isEmpty())
        {
            // 通知弹框提示
            QTimer::singleShot(0, this, SLOT(OnRefRangeEditFocusIn()));
        }
    }

    // 由基类函数处理
    return BaseDlg::eventFilter(obj, event);
}

void CRangParaSetWidget::ShowDlg(RangeSetParamDlgEditType eType)
{
    m_editType = eType;

    switch (eType)
    {
    case Et_Add:
        SetTitleName(tr("新增"));
        break;
    case Et_Modify:
        SetTitleName(tr("修改"));
        break;
    default:
        break;
    }

    ui->gender_cbox->setCurrentIndex(ui->gender_cbox->findData((int)::tf::Gender::GENDER_UNKNOWN));
    ui->age_unit_cbox->setCurrentIndex(::tf::AgeUnit::AGE_UNIT_UNKNOWN);
    ui->auto_default_cbox->setCurrentText(STR_AUTO_DEFAULT_N);

    for (auto edit : QWidget::findChildren<QLineEdit*>())
    {
        edit->clear();
    }

    show();
}

void CRangParaSetWidget::OnRefRangeEditFocusIn()
{
    // 单参考区级被编辑
    if (ui->str_refrange_edit->hasFocus() && (!ui->ref_lower_age_edit->text().isEmpty() || !ui->ref_upper_age_edit->text().isEmpty() ||
        !ui->ref_critical_lower_rang->text().isEmpty() || !ui->ref_critical_upper_rang->text().isEmpty()))
    {
        // 弹框提示
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("当前为多参考区间，是否清空已设置单参考区间的值？"), TipDlgType::TWO_BUTTON));
        if (pTipDlg->exec() == QDialog::Accepted)
        {
            ui->ref_lower_age_edit->clear();
            ui->ref_upper_age_edit->clear();
            ui->ref_critical_lower_rang->clear();
            ui->ref_critical_upper_rang->clear();
        }
        else
        {
            ui->str_refrange_edit->clearFocus();
        }
    }

    // 多参考区级被编辑
    if ((ui->ref_critical_lower_rang->hasFocus() || ui->ref_critical_upper_rang->hasFocus() ||
        ui->ref_lower_age_edit->hasFocus() || ui->ref_upper_age_edit->hasFocus()) && !ui->str_refrange_edit->text().isEmpty())
    {
        // 弹框提示
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("当前为单参考区间，是否清空已设置多参考区间的值？"), TipDlgType::TWO_BUTTON));
        if (pTipDlg->exec() == QDialog::Accepted)
        {
            ui->str_refrange_edit->clear();
        }
        else
        {
            ui->ref_lower_age_edit->clearFocus();
            ui->ref_upper_age_edit->clearFocus();
            ui->ref_critical_lower_rang->clearFocus();
            ui->ref_critical_upper_rang->clearFocus();
        }
    }
}

void CRangParaSetWidget::OnDefaultValueFlagChanged(const QString& text)
{
    ui->gender_cbox->setEnabled(text != STR_AUTO_DEFAULT_Y);
    ui->upper_age_edit->setEnabled(text != STR_AUTO_DEFAULT_Y);
    ui->lower_age_edit->setEnabled(text != STR_AUTO_DEFAULT_Y);
    ui->age_unit_cbox->setEnabled(text != STR_AUTO_DEFAULT_Y);

    if (m_rangeDlgType == RPD_Im)
    {
        // BUG	0022621: [应用] 项目设置-范围界面“是否缺省”，选择“N”，样本类型下拉框不应该置灰的修改
        ui->samp_source_cbox->setEnabled(text != STR_AUTO_DEFAULT_Y);
    }
    else
    {
        // TODO:@7951/LuoXin confirm.
        ui->samp_source_cbox->setEnabled(text != STR_AUTO_DEFAULT_Y && m_rangeDlgType == CALC && m_defaultSampleSource == -1);
    }

    if (text == STR_AUTO_DEFAULT_Y)
    {
        ui->gender_cbox->setCurrentIndex(ui->gender_cbox->findData((int)::tf::Gender::GENDER_UNKNOWN));
        ui->lower_age_edit->clear();
        ui->upper_age_edit->clear();
        ui->age_unit_cbox->setCurrentIndex(0);
        ui->samp_source_cbox->setCurrentText(TransSampleName(m_defaultSampleSource));
    }
}
