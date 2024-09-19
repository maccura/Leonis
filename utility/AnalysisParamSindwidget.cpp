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

#include "AnalysisParamSindwidget.h"

#include <QLineEdit>
#include "ui_AnalysisParamSindwidget.h"
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/CommonInformationManager.h"
#include "manager/DictionaryQueryManager.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/common.h"
#include "thrift/DcsControlProxy.h"
#include "thrift/ch/ChLogicControlProxy.h"


#define  SAV_SHOW_FACTOR		(10)		// 样本吸取量显示倍率
#define  SIND_L_NAME			("l")		// 脂血(L)
#define	 SIND_H_NAME			("h")		// 溶血(H)
#define  SIND_I_NAME			("i")		// 黄疸(I)

// 当发生错误时，弹出消息框后退出当前函数
#define OnErr(bErr, msg) \
if ((bErr))\
{\
	TipDlg(msg).exec();\
	return false;\
}

///
/// @brief
///     加载项目参数
///
///@param[in]    item 项目信息
///
/// @return  成功返回true   
///
/// @par History:
/// @li 6950/ChenFei，2022年05月23日，新建函数
///
bool AnalysisParamSindWidget::LoadAnalysisParam(const AssayListModel::StAssayListRowItem& item)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 复位界面
    Reset();

    // 判断待加载项有效
    if (item.assayCode == INVALID_ASSAY_CODE)
    {
        return true;
    }

    // 查询通用项目信息
    const auto &CIM = CommonInformationManager::GetInstance();
    auto spGAI = CIM->GetAssayInfo(item.assayCode);
    if (spGAI == nullptr)
    {
        ULOG(LOG_ERROR, "GetAssayInfo() Failed!");
        return false;
    }

    // 查询生化通用项目信息
    auto spChGAI = CIM->GetChemistryAssayInfo(item.assayCode, (::tf::DeviceType::type)item.model);
    if (spChGAI == nullptr)
    {
        ULOG(LOG_ERROR, "GetChemistryAssayInfo() Failed!");
        return false;
    }

    // 查询生化特殊项目信息
    auto chSAIs = CIM->GetChemistrySpecialAssayInfo(ch::tf::g_ch_constants.ASSAY_CODE_L, (::tf::DeviceType::type)item.model, item.sampleType);
    if (chSAIs.empty())
    {
        ULOG(LOG_ERROR, "GetChemistrySpecialAssayInfo() Failed!");
        return false;
    }

    // 刷入控件

    // 项目名称
    ui->assayNameEdit->setText(QString::fromStdString(spGAI->assayName));

    // 参数版本
    ui->assayParamVerEdit->setText(QString::fromStdString(spChGAI->version));

    // 样本量
    ui->sampleVolEdit->setText(QString::number(double(chSAIs[0]->sampleAspirateVols[0].originalSample) / SAV_SHOW_FACTOR));

    // 试剂量
    ui->reagentVolEdit->setText(QString::number(spChGAI->reagentAspirateVols[0].reagent / SAV_SHOW_FACTOR));

    // 混匀等级
    ui->level_combox->setCurrentText(QString::number(spChGAI->reagentAspirateVols[0].mixingLevel));

    // 脂血设置名称
    auto spGAIL = CIM->GetAssayInfo(spGAI->subAssays[0]);
    if (spGAIL == nullptr)
    {
        ULOG(LOG_ERROR, "GetAssayInfo() Failed!");
        return false;
    }

    ui->lPrintNameEdit->setText(QString::fromStdString(spGAIL->printName));
    ui->lNameEdit->setText(QString::fromStdString(spGAIL->assayName));

    // 溶血设置名称
    auto spGAIH = CIM->GetAssayInfo(spGAI->subAssays[1]);
    if (spGAIH == nullptr)
    {
        ULOG(LOG_ERROR, "GetAssayInfo() Failed!");
        return false;
    }

    ui->hPrintNameEdit->setText(QString::fromStdString(spGAIH->printName));
    ui->hNameEdit->setText(QString::fromStdString(spGAIH->assayName));

    // 黄疸设置名称
    auto spGAII = CIM->GetAssayInfo(spGAI->subAssays[2]);
    if (spGAII == nullptr)
    {
        ULOG(LOG_ERROR, "GetAssayInfo() Failed!");
        return false;
    }

    ui->iPrintNameEdit->setText(QString::fromStdString(spGAII->printName));
    ui->iNameEdit->setText(QString::fromStdString(spGAII->assayName));

    // 加载系数
    if (!LoaCFTToCtrls())
    {
        ULOG(LOG_ERROR, "LoaCFTToCtrls() Failed!");
        return false;
    }

    // 加载血清指数的定性判断   
    LoadQualitative(spGAI->subAssays[0], SIND_L_NAME);
    LoadQualitative(spGAI->subAssays[1], SIND_H_NAME);
    LoadQualitative(spGAI->subAssays[2], SIND_I_NAME);

    return true;
}  

bool AnalysisParamSindWidget::LoadQualitative(int assayCode, QString flag)
{
    auto spSais = CommonInformationManager::GetInstance()->
        GetChemistrySpecialAssayInfo(assayCode, ::tf::DeviceType::DEVICE_TYPE_C1000);
    if (spSais.empty() || spSais[0] == nullptr)
    {
        ULOG(LOG_ERROR, "GetChemistrySpecialAssayInfo() By Code[%d] Failed!", assayCode);
        return false;
    }

    QCheckBox* box = QWidget::findChild<QCheckBox*>(QString("enable_ckbox_") + flag);
    box->setChecked(spSais[0]->qualitativeJudge.bUsing);

    int i=1;
    for (auto& qj : spSais[0]->qualitativeJudge.vecItems)
    {
        QString seq = QString::number(i);
        QLineEdit* tagEdit = QWidget::findChild<QLineEdit*>(QString("qualitative_tag_linedit_%1_%2").arg(flag).arg(seq));
        QLineEdit* lowEdit = QWidget::findChild<QLineEdit*>(QString("qualitative_low_linedit_%1_%2").arg(flag).arg(seq));
        QLineEdit* upEdit = QWidget::findChild<QLineEdit*>(QString("qualitative_up_linedit_%1_%2").arg(flag).arg(seq));

        tagEdit->setText(QString::fromStdString(qj.tag));
        lowEdit->setText(qj.lowerConc == std::numeric_limits<double>::lowest() ? "" : QString::number(qj.lowerConc, 'f', 2));
        upEdit->setText(qj.upperConc == DBL_MAX ? "" : QString::number(qj.upperConc, 'f', 2));

        i++;
    }

    return true;
}

bool AnalysisParamSindWidget::GetQualitative(QString flag, ::tf::QualitativeJudge& qj)
{
    std::vector<::tf::QualitativeItem> items;
    for (int i=1; i<7; i++)
    {
        QString seq = QString::number(i);
        QLineEdit* tagEdit = QWidget::findChild<QLineEdit*>(QString("qualitative_tag_linedit_%1_%2").arg(flag).arg(seq));
        QLineEdit* lowEdit = QWidget::findChild<QLineEdit*>(QString("qualitative_low_linedit_%1_%2").arg(flag).arg(seq));
        QLineEdit* upEdit = QWidget::findChild<QLineEdit*>(QString("qualitative_up_linedit_%1_%2").arg(flag).arg(seq));

        ::tf::QualitativeItem item;
        item.__set_tag(tagEdit->text().toStdString());

        double temp = lowEdit->text().toDouble();
        item.__set_lowerConc(IsEqual(temp,0.0) ? std::numeric_limits<double>::lowest() : temp);

        temp = upEdit->text().toDouble();
        item.__set_upperConc(IsEqual(temp, 0.0) ? DBL_MAX : temp);

        items.push_back(std::move(item));
    }

    if (!CheckQualitative(items))
    {
        ULOG(LOG_INFO, "CheckQualitative Failed!");
        return false;
    }

    QCheckBox* box = QWidget::findChild<QCheckBox*>(QString("enable_ckbox_")+flag);
    qj.__set_bUsing(box->isChecked());
    qj.__set_vecItems(std::move(items));

    return true;
}

bool AnalysisParamSindWidget::CheckQualitative(const std::vector<::tf::QualitativeItem> & items)
{
    // 检查定性判断设置的值是否合法
    for (int i = 0; i < 6; i++)
    {
        OnErr(items[i].lowerConc > items[i].upperConc, tr("定性判断的单项的低值不能大于高值"));

        for (int j = i + 1; j < 6; j++)
        {
            OnErr(items[i].tag == items[j].tag && !items[j].tag.empty(), tr("定性判断的标记不能相同"));

            OnErr(items[i].lowerConc != std::numeric_limits<double>::lowest()
                && ((items[j].lowerConc != std::numeric_limits<double>::lowest() && items[i].lowerConc > items[j].lowerConc)
                    && (items[j].upperConc != DBL_MAX && items[i].lowerConc < items[j].upperConc))
                , tr("定性判断的范围不能重叠"));

            OnErr(items[i].upperConc != DBL_MAX  && items[j].lowerConc != std::numeric_limits<double>::lowest()
                && items[i].upperConc > items[j].lowerConc, tr("定性判断的范围不能重叠"));
        }

        if (i != 0 && i != 5)
        {
            OnErr(!items[i].tag.empty() &&
                (items[i].lowerConc == std::numeric_limits<double>::lowest() || items[i].upperConc == DBL_MAX)
                , tr("定性判断的范围设置错误"));

            OnErr(items[i].tag.empty() &&
                (items[i].lowerConc != std::numeric_limits<double>::lowest() || items[i].upperConc != DBL_MAX)
                , tr("定性判断的范围设置错误"));
        }
        else
        {
            OnErr(!items[i].tag.empty() &&
                items[i].lowerConc == std::numeric_limits<double>::lowest() && items[i].upperConc == DBL_MAX
                , tr("定性判断的范围设置错误"));

            OnErr(items[i].tag.empty() &&
                (!(items[i].lowerConc == std::numeric_limits<double>::lowest() && items[i].upperConc == DBL_MAX))
                , tr("定性判断的范围设置错误"));
        }
    }

    return true;
}

///
/// @brief
///     保存项目信息
///
///@param[in]   item 项目信息 
///
/// @return   成功返回true  
///
/// @par History:
/// @li 6950/ChenFei，2022年05月23日，新建函数
///
bool AnalysisParamSindWidget::SaveAnalysisParam(const AssayListModel::StAssayListRowItem& item)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (!CheckUserInputParams())
    {
        ULOG(LOG_INFO, "User input params invalid!");
        return false;
    }

	const auto &CIM = CommonInformationManager::GetInstance();
	// 查询生化通用项目信息
	auto spChGAI = CIM->GetChemistryAssayInfo(item.assayCode,::tf::DeviceType::DEVICE_TYPE_C1000);
	if (spChGAI == nullptr)
	{
		ULOG(LOG_ERROR, "GetChemistryAssayInfo() Failed!");
		OnErr(true, tr("保存失败"));
	}

	// 修改试剂吸取量
	::ch::tf::GeneralAssayInfo chGAI;
	chGAI.__set_id(spChGAI->id);

    int iVol = ui->reagentVolEdit->text().toInt() * SAV_SHOW_FACTOR;
    OnErr(iVol < REAGENT_PROBE_MIN_VOL || iVol > REAGENT_PROBE_MAX_VOL || (iVol % 5 != 0),
            tr("10≤试剂量≤150，步进值0.5"));

	auto RAV = spChGAI->reagentAspirateVols;
	RAV[0].reagent = iVol;

    RAV[0].__set_mixingLevel(ui->level_combox->currentText().toInt());

	chGAI.__set_reagentAspirateVols(RAV);
	if (!ch::LogicControlProxy::ModifyAssayConfigInfo(chGAI))
	{
		ULOG(LOG_ERROR, "ModifyAssayConfigInfo() Failed!");
		OnErr(true, tr("保存失败"));
	}

	// 系数
	std::vector<int32_t> vecSerumFactors;
	for (auto& edit : ui->CFT_groupBox->findChildren<QLineEdit*>())
	{
		vecSerumFactors.push_back(edit->text().toInt());
	}
	if (!DictionaryQueryManager::SaveChSerumIndexFactor(vecSerumFactors))
	{
		ULOG(LOG_ERROR, "Failed to save chserumindexfactor config!");
		OnErr(true, tr("保存失败"));
	}

	auto odlspGAI = CIM->GetAssayInfo(item.assayCode);
    if (!SaveSindSubAsssayParams(odlspGAI->subAssays[0], SIND_L_NAME)
        || !SaveSindSubAsssayParams(odlspGAI->subAssays[1], SIND_H_NAME)
        || !SaveSindSubAsssayParams(odlspGAI->subAssays[2], SIND_I_NAME))
    {
        return false;
    }

    // 通知数据浏览界面刷新项目参数
    POST_MESSAGE(MSG_ID_USER_DEF_ASSAY_PARAM_UPDATE, item.assayCode);

 	return true;
}

bool AnalysisParamSindWidget::SaveSindSubAsssayParams(int assayCode, QString flag)
{
    // 修改样本吸取量
    double iVol = ui->sampleVolEdit->text().toDouble() * SAV_SHOW_FACTOR;
    OnErr(iVol < SAMPLE_PROBE_MIN_VOL || iVol > SAMPLE_PROBE_MAX_VOL,
        tr("%1≤样本量≤%2").arg(QString::number(SAMPLE_PROBE_MIN_VOL / 10)).arg(QString::number(SAMPLE_PROBE_MAX_VOL / 10)));

    // 获取旧项目
    const auto& CIM = CommonInformationManager::GetInstance();
    auto odlspSAIs = CIM->GetChemistrySpecialAssayInfo(assayCode, ::tf::DeviceType::DEVICE_TYPE_C1000);   
    if (odlspSAIs.empty())
    {
        ULOG(LOG_ERROR, "GetIseSpecialAssayInfo(code:%d) Failed!", assayCode);
        OnErr(true, tr("保存失败"));
    }

    ::ch::tf::SpecialAssayInfo SAI;
    // 定性判断
    {
        ::tf::QualitativeJudge qj;
        if (!GetQualitative(flag, qj))
        {
            ULOG(LOG_INFO, "GetQualitative() Failed!");
            return false;
        }

        SAI.__set_qualitativeJudge(std::move(qj));
    }

    // 样本量
    ::ch::tf::SampleAspirateVol sav;
    sav.originalSample = iVol;
    SAI.__set_sampleAspirateVols({ sav });

    SAI.__set_id(odlspSAIs[0]->id);
    SAI.__set_assayCode(odlspSAIs[0]->assayCode);
    if (tf::ThriftResult::THRIFT_RESULT_SUCCESS != ch::LogicControlProxy::ModifySpecialAssayInfo(SAI))
    {
        ULOG(LOG_ERROR, "ModifySpecialAssayInfo() Failed!");
        OnErr(true, tr("保存失败"));
    }

    // 获取旧项目的id
    const auto& odlspGAI = CIM->GetAssayInfo(assayCode);
    if (odlspGAI == nullptr)
    {
        ULOG(LOG_ERROR, "GetAssayInfo() Failed!");
        OnErr(true, tr("保存失败"));
    }

    QLineEdit* printNameEdit = QWidget::findChild<QLineEdit*>(flag + "PrintNameEdit");
    QLineEdit* nameEdit = QWidget::findChild<QLineEdit*>(flag + "NameEdit");

    // 保存参数
    ::tf::GeneralAssayInfo spGAIL;
    spGAIL.__set_id(odlspGAI->id);
    spGAIL.__set_printName(printNameEdit->text().toUtf8().data());
    spGAIL.__set_assayName(nameEdit->text().toUtf8().data());
    if (!DcsControlProxy::GetInstance()->ModifyGeneralAssayInfo(spGAIL))
    {
        ULOG(LOG_ERROR, "ModifyGeneralAssayInfo() Failed!");
        OnErr(true, tr("保存失败"));
    }

    return true;
}

bool AnalysisParamSindWidget::HasUnSaveChangedData()
{
    // 查询生化通用项目信息
    auto spChGAI = CommonInformationManager::GetInstance()->GetChemistryAssayInfo(
            ch::tf::g_ch_constants.ASSAY_CODE_SIND, ::tf::DeviceType::DEVICE_TYPE_C1000);
    if (spChGAI == nullptr)
    {
        return false;
    }

    	// 系数
	std::vector<int32_t> vecSerumFactors;
	for (auto& edit : ui->CFT_groupBox->findChildren<QLineEdit*>())
	{
		vecSerumFactors.push_back(edit->text().toInt());
	}

    std::vector<int32_t> OldVecSerumFactors;
	if (!DictionaryQueryManager::GetChSerumIndexFactor(OldVecSerumFactors))
	{
        return false;
	}

    if (OldVecSerumFactors != vecSerumFactors)
    {
        return true;
    }

    return (SubHasUnSaveChangedData(ch::tf::g_ch_constants.ASSAY_CODE_L, SIND_L_NAME)
        || SubHasUnSaveChangedData(ch::tf::g_ch_constants.ASSAY_CODE_H, SIND_H_NAME)
        || SubHasUnSaveChangedData(ch::tf::g_ch_constants.ASSAY_CODE_I, SIND_I_NAME));
}

bool AnalysisParamSindWidget::SubHasUnSaveChangedData(int assayCode, QString flag)
{
    // 获取旧项目
    const auto& CIM = CommonInformationManager::GetInstance();
    auto oldspSAIs = CIM->GetChemistrySpecialAssayInfo(assayCode, ::tf::DeviceType::DEVICE_TYPE_C1000);
    if (oldspSAIs.empty())
    {
        return false;
    }

    ::tf::QualitativeJudge qj;
    if (!GetQualitative(flag, qj))
    {
        return false;
    }

    if (qj != oldspSAIs[0]->qualitativeJudge)
    {
        return true;
    }

    // 样本量
    int iVol = ui->sampleVolEdit->text().toDouble() * SAV_SHOW_FACTOR;
    return oldspSAIs[0]->sampleAspirateVols[0].originalSample != iVol;
}

void AnalysisParamSindWidget::Reset()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	auto children = this->children();

	for (auto child : children)
	{
		auto edit = dynamic_cast<QLineEdit *>(child);
		if (edit != nullptr)
		{
            edit->setText("");
			continue;
		}
	}
}

///
/// @brief
///     加载分析参数界面系数A~F
///
///
/// @return 
///
/// @par History:
/// @li 7951/LuoXin，2022年6月20日，新建函数
///
bool AnalysisParamSindWidget::LoaCFTToCtrls()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	std::vector<int>tempData;
    if (!DictionaryQueryManager::GetChSerumIndexFactor(tempData))
    {
        ULOG(LOG_WARN, "Failed to get chserumindexfactor config.");
        return false;
    }

	// 设置编辑框内容
	int tempIter = 0;
	for (auto& eidt : ui->CFT_groupBox->findChildren<QLineEdit*>())
	{
        eidt->setText(QString::number(tempData[tempIter]));
		tempIter++;
	}

	return true;
}

void AnalysisParamSindWidget::SetCtrlsRegExp()
{
	for (auto edit : findChildren<QLineEdit*>())
	{
		// 限制输入正整数
		if (edit->objectName().startsWith("CFT_edit_"))
		{
			edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_POSITIVE_INT), edit));
		}

		// 限制输入正浮点数
		if (edit->objectName().contains("low")|| edit->objectName().contains("up"))
		{
			edit->setValidator(new QDoubleValidator(0.0, UI_DEFAULT_DOUBLE_MIX, 2, edit));
		}
	}

    // 样本量1-25
    ui->sampleVolEdit->setValidator(new QDoubleValidator(
        SAMPLE_PROBE_MIN_VOL / 10, SAMPLE_PROBE_MAX_VOL / 10, 1, ui->sampleVolEdit));
    // 试剂量10-150
    ui->reagentVolEdit->setValidator(new QDoubleValidator(
        REAGENT_PROBE_MIN_VOL / 10, REAGENT_PROBE_MAX_VOL / 10, 1, ui->reagentVolEdit));

}

bool AnalysisParamSindWidget::CheckUserInputParams()
{
    OnErr(ui->CFT_edit_A->text().isEmpty(), tr("系数A不能为空"));
    OnErr(ui->CFT_edit_B->text().isEmpty(), tr("系数B不能为空"));
    OnErr(ui->CFT_edit_C->text().isEmpty(), tr("系数C不能为空"));
    OnErr(ui->CFT_edit_D->text().isEmpty(), tr("系数D不能为空"));
    OnErr(ui->CFT_edit_E->text().isEmpty(), tr("系数E不能为空"));
    OnErr(ui->CFT_edit_F->text().isEmpty(), tr("系数F不能为空"));

    // 检查项目简称不能为空
    OnErr(ui->lNameEdit->text().isEmpty() || ui->hNameEdit->text().isEmpty() || ui->iNameEdit->text().isEmpty(),
        tr("脂血(L)、溶血(H)、黄疸(I)的简称不能为空"));

    // 检查打印名称不能为空
    OnErr(ui->lPrintNameEdit->text().isEmpty() || ui->hPrintNameEdit->text().isEmpty() || ui->iPrintNameEdit->text().isEmpty(),
        tr("脂血(L)、溶血(H)、黄疸(I)的打印名称不能为空"));

    // 检查项目的简称不能重复
    OnErr(ui->lNameEdit->text() == ui->hNameEdit->text() || ui->lNameEdit->text() == ui->iNameEdit->text()
        || ui->hNameEdit->text() == ui->iNameEdit->text(), tr("简称错误：脂血(L)、溶血(H)、黄疸(I)的简称不可重复"));

    // 检查打印名称不可重复
    OnErr(ui->lPrintNameEdit->text() == ui->hPrintNameEdit->text() || ui->lPrintNameEdit->text() == ui->iPrintNameEdit->text()
        || ui->hPrintNameEdit->text() == ui->iPrintNameEdit->text(), tr("打印名称错误：脂血(L)、溶血(H)、黄疸(I)的打印名称不可重复"));

    // 检查项目的简称是否已经存在
    const auto& CIM = CommonInformationManager::GetInstance();
    OnErr(CIM->CheckAssayNameExisted(::ch::tf::g_ch_constants.ASSAY_CODE_L, ui->lNameEdit->text().toStdString()),
        tr("简称错误：%1已经存在").arg(ui->lNameEdit->text()));
    OnErr(CIM->CheckAssayNameExisted(::ch::tf::g_ch_constants.ASSAY_CODE_H, ui->hNameEdit->text().toStdString()),
        tr("简称错误：%1已经存在").arg(ui->hNameEdit->text()));
    OnErr(CIM->CheckAssayNameExisted(::ch::tf::g_ch_constants.ASSAY_CODE_I, ui->iNameEdit->text().toStdString()),
        tr("简称错误：%1已经存在").arg(ui->iNameEdit->text()));

    // 检查项目的打印名称是否已经存在
    OnErr(CIM->CheckAssayNameExisted(::ch::tf::g_ch_constants.ASSAY_CODE_L, ui->lPrintNameEdit->text().toStdString()),
        tr("打印名称错误：%1已经存在").arg(ui->lPrintNameEdit->text()));
    OnErr(CIM->CheckAssayNameExisted(::ch::tf::g_ch_constants.ASSAY_CODE_H, ui->hPrintNameEdit->text().toStdString()),
        tr("打印名称错误：%1已经存在").arg(ui->hPrintNameEdit->text()));
    OnErr(CIM->CheckAssayNameExisted(::ch::tf::g_ch_constants.ASSAY_CODE_I, ui->iPrintNameEdit->text().toStdString()),
        tr("打印名称错误：%1已经存在").arg(ui->iPrintNameEdit->text()));

    return true;
}

AnalysisParamSindWidget::AnalysisParamSindWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AnalysisParamSindWidget)
{
    ui->setupUi(this);

	SetCtrlsRegExp();

    // 调整窗口高度
    if (CommonInformationManager::GetInstance()->GetSoftWareType() != CHEMISTRY_AND_IMMUNE)
    {
        ui->label_2->hide();
    }
}

AnalysisParamSindWidget::~AnalysisParamSindWidget()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}
