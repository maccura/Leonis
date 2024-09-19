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

#include "assayparamtabwidget.h"
#include "ui_Rangeparamwidget.h"

#include "AnalysisParamSindwidget.h"
#include "AnalysisParamIsewidget.h" 
#include "IseCaliSettingWidget.h"
#include "RangeParaWidget.h"
#include "analysisparamwidget.h"
#include "CalibrateSettingWidget.h"
#include "ImRangeParamWidget.h"

#include "QcParamSettingsWidget.h"
#include "ImCalibrateSettingWidget.h"
#include "ImAnalysisparamwidget.h"

#include "manager/UserInfoManager.h"
#include "shared/tipdlg.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/CommonInformationManager.h"
#include "src/common/Mlog/mlog.h"
#include "src/public/ch/ChAssayCode.h"

#include "thrift/DcsControlProxy.h"
#include "thrift/ise/IseLogicControlProxy.h"
#include "thrift/im/ImLogicControlProxy.h"
#include "thrift/ch/ChLogicControlProxy.h"
#include "src/common/common.h"


// 当发生错误时，弹出消息框后退出当前函数
#define OnErr(bErr, msg) \
if ((bErr))\
{\
	TipDlg(msg).exec();\
	return false;\
}

// 项目设置生化页面索引
enum CH_WIDGET_INDEX
{
	CH_ANALY_WIDGET,			// 分析
	CH_CALI_WIDGET,				// 校准
	CH_QC_WIDGET,				// 质控
	CH_RANG_WIDGET				// 范围
};

// 项目设置ise页面索引
enum ISE_WIDGET_INDEX
{
	ISE_ANALY_WIDGET,			// 分析
	ISE_CALI_WIDGET,			// 校准
    ISE_QC_WIDGET,			    // 质控
	ISE_RANG_WIDGET				// 范围
};

#define ADD_TAB(tabType, tabWidget) do {\
    TabItem tb(tabType, tabWidget);\
    addTab(tabWidget, tb.GetName());\
    m_tabInfo.push_back(tb);\
} while (false);

AssayParamTabWidget::AssayParamTabWidget(QWidget *parent)
    : QTabWidget(parent),
    m_bInit(false)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    m_itemType = 0;
	m_lastShowStyle = ShowStyle::EMPTY;

	m_analyParamWidget = new AnalysisParamWidget(this);
	//m_analyParamWidget->setVisible(false);

	m_analyParamSindWidget = new AnalysisParamSindWidget(this);
	m_analyParamSindWidget->setVisible(false);

	m_calibrateSettingWidget = new CalibrateSettingWidget(this);
	//m_calibrateSettingWidget->setVisible(false);

    m_rangeParamWidget = new CRangeParaWidget(this);
	//m_rangeParamWidget->setVisible(false);

	m_IseCaliSettingWidget = new IseCaliSettingWidget(this);
	m_IseCaliSettingWidget->setVisible(false);

	m_AnalysisParamIsewidget = new AnalysisParamIsewidget(this);
	m_AnalysisParamIsewidget->setVisible(false);

	m_ImRangeParamWidget = new ImRangeParamWidget(this);
	m_ImRangeParamWidget->setVisible(false);

    m_qcParamSettingsWidget = new QcParamSettingsWidget(this);
    m_qcParamSettingsWidget->setVisible(true);
	
	m_ImCaliSettingWidget = new ImCalibrateSettingWidget(this);
	m_ImCaliSettingWidget->setVisible(false);

	m_ImAnalysisParamWidget = new ImAnalysisParamWidget(this);
    m_ImAnalysisParamWidget->setVisible(false);
    m_ImAnalysisParamWidget->BindSetVersionCallBack(std::bind(&AssayParamTabWidget::ChangeCurrParamVersion, this, std::placeholders::_1));

	connect(this, SIGNAL(currentChanged(int)), this, SLOT(OnCurrentChanged(int)));

    connect(m_analyParamWidget, &AnalysisParamWidget::SampleVolChanged,
                m_calibrateSettingWidget, &CalibrateSettingWidget::OnDefaultSampleVolChanged);

    // 注册当前用户权限更新处理函数
    REGISTER_HANDLER(MSG_ID_SYSTEM_PERMISSTION_CHANGED, this, OnPermisionChanged);
}

AssayParamTabWidget::~AssayParamTabWidget()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
}

///
/// @brief
///     加载项目参数
///
/// @param[in]  item  用于定位数据
///
/// @par History:
/// @li 4170/TangChuXian，2020年11月4日，新建函数
///
void AssayParamTabWidget::LoadAssayParam(const AssayListModel::StAssayListRowItem& item, bool updateSampleType)
{
	ULOG(LOG_INFO, "%s(%d)", __FUNCTION__, item.assayCode);

    // 如果是非法数据，沿用上一个的类型处理,避免界面全空白。
    int iAssayClassfy = (item.assayCode == INVALID_ASSAY_CODE) ? m_currentItem.assayClassify : item.assayClassify;
	m_currentItem = item;
    m_currentItem.assayClassify = iAssayClassfy;

    // 质控品设置界面的数据加载
    m_qcParamSettingsWidget->LoadQcParameter(item);

    // 加载界面
	switch ((tf::AssayClassify::type)iAssayClassfy)
	{
	case tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY:
	{
		if (item.assayCode == ch::tf::g_ch_constants.ASSAY_CODE_SIND)
		{
			SwitchShowStyle(ShowStyle::SIND);
			m_analyParamSindWidget->LoadAnalysisParam(item);
			break;
		}

		SwitchShowStyle(ShowStyle::CHEMISTRY);
		m_analyParamWidget->LoadAnalysisParam(item, updateSampleType);
		m_calibrateSettingWidget->LoadAnalysisParam(item);
		m_rangeParamWidget->LoadRangeParameter(item, updateSampleType);

		break;
	}

	case tf::AssayClassify::ASSAY_CLASSIFY_ISE:
	{
		SwitchShowStyle(ShowStyle::NA_K_CL);
		m_AnalysisParamIsewidget->LoadAnalysisParam(item);
		m_IseCaliSettingWidget->LoadRangeParameter(item);
		m_rangeParamWidget->LoadRangeParameter(item);
		break;
	}

	case tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE:
	{
		SwitchShowStyle(ShowStyle::IMMUNE);
		m_ImAnalysisParamWidget->LoadAnalysisParam(item);
		m_ImRangeParamWidget->LoadRangeParameter(item);
		m_ImCaliSettingWidget->LoadAnalysisParam(item);
		break;
	}

	default:
		break;
	}
}

void AssayParamTabWidget::ReloadAssayParam()
{
    OnCurrentChanged(-1);
}

///
/// @brief
///     保存参数
///
///@param[in]    item 保存项目定位信息
///@param[in]    strOptLog 操作日志
///
/// @return    成功返回true 
///
/// @par History:
/// @li 6950/ChenFei，2022年05月19日，新建函数
///
/// @par History:
/// @li 1556/Chenjianlin，2024年3月7日，增加操作日志
///
bool AssayParamTabWidget::SaveAssayParam(const AssayListModel::StAssayListRowItem& item, QString& strOptLog)
{
    ULOG(LOG_INFO, "%s(%d)", __FUNCTION__, item.assayCode);
    // 如果项目id无效，直接返回true
    if (item.assayCode == INVALID_ASSAY_CODE)
    {
        return true;
    }
    // 根据项目类型    项目设置四个页面统一保存，bug10817
    switch ((tf::AssayClassify::type)item.assayClassify)
	{
	case tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY:
	{
		if (item.assayCode == ch::tf::g_ch_constants.ASSAY_CODE_SIND)
		{
            strOptLog = tr("修改项目") + item.name + tr("的参数");
			return m_analyParamSindWidget->SaveAnalysisParam(item);
		}

		return SaveChAssayParam(item, strOptLog);
	}
	case tf::AssayClassify::ASSAY_CLASSIFY_ISE:
	{
		strOptLog = tr("修改项目") + item.name + tr("的参数");
		return SaveIseAssayParam(item);
	}
	case tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE:
	{
		QString strDetail("");
		bool bRet = SaveImAssayParam(item, strDetail);
		if (!strDetail.isEmpty())
		{
			strOptLog = tr("修改项目") + item.name + tr("的参数");
			strOptLog += "(" + strDetail + ")";
		}
		return bRet;
	}
	default:
		break;
	}
	return false;
}

bool AssayParamTabWidget::HasUnSaveChangedData(const AssayListModel::StAssayListRowItem& item)
{
    if (item.assayCode <= 0)
    {
        return false;
    }

    // 生化项目
    if (item.assayCode >= tf::AssayCodeRange::CH_RANGE_MIN
        && item.assayCode <= tf::AssayCodeRange::CH_OPEN_RANGE_MAX)
    {
        return m_analyParamWidget->HasUnSaveChangedData(item)
            || m_calibrateSettingWidget->HasUnSaveChangedData(item)
            || m_qcParamSettingsWidget->HasUnSaveChangedData(item)
            || m_rangeParamWidget->HasUnSaveChangedData(item);
    }
    // SIND
    else if (item.assayCode == ch::tf::g_ch_constants.ASSAY_CODE_SIND)
    {
        return m_analyParamSindWidget->HasUnSaveChangedData();
    }
    // ISE
    else if (item.assayCode >= ise::tf::g_ise_constants.ASSAY_CODE_ISE
            && item.assayCode <= ise::tf::g_ise_constants.ASSAY_CODE_CL)
    {
        return m_AnalysisParamIsewidget->HasUnSaveChangedData()
            || m_IseCaliSettingWidget->HasUnSaveChangedData(item)
            || m_qcParamSettingsWidget->HasUnSaveChangedData(item)
            || m_rangeParamWidget->HasUnSaveChangedData(item);
    }

    return false;
}

bool AssayParamTabWidget::SaveQcDoc(std::vector<std::shared_ptr<tf::QcDoc>>& changedQc)
{
    std::vector<tf::QcDocUpdate> notifyItems;
    for (const auto& qcItem : changedQc)
    {
        if (qcItem == nullptr)
        {
            continue;
        }

        tf::ResultLong ret;
        if (!DcsControlProxy::GetInstance()->ModifyQcDoc(ret, *qcItem) || ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_WARN, "Failed to save qc config to db.(%lld)", qcItem->id);
            return false;
        }

        tf::QcDocUpdate qcInfo;
        qcInfo.__set_db(qcItem->id);
        qcInfo.__set_updateType(tf::UpdateType::UPDATE_TYPE_MODIFY);
        notifyItems.push_back(qcInfo);
    }

    POST_MESSAGE(MSG_ID_QC_DOC_INFO_UPDATE, notifyItems);

    return true;
}

bool AssayParamTabWidget::CheckIsModifyCaliParams(const ch::tf::GeneralAssayInfo& newChGAI, const ch::tf::SpecialAssayInfo& newChSAI)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    QString tipStr;
    const auto& CIM = CommonInformationManager::GetInstance();
    const auto& oldChGAI = CIM->GetChemistryAssayInfo(newChGAI.assayCode, newChGAI.deviceType, newChGAI.version);
    if (oldChGAI != nullptr)
    {
        if (oldChGAI->reagentAspirateVols != newChGAI.reagentAspirateVols)
        {
            tipStr += tipStr.isEmpty() ? tr("试剂量") : tr("、试剂量");
        }

        if (oldChGAI->analysisMethod != newChGAI.analysisMethod)
        {
            tipStr += tipStr.isEmpty() ? tr("分析方法") : tr("、分析方法");
        }

        if (oldChGAI->reactionTime != newChGAI.reactionTime)
        {
            tipStr += tipStr.isEmpty() ? tr("反应时间") : tr("、反应时间");
        }
        
        if (oldChGAI->analyPoints != newChGAI.analyPoints)
        {
            tipStr += tipStr.isEmpty() ? tr("测光点") : tr("、测光点");
        }

        if (!oldChGAI->waveLengths.empty() && oldChGAI->waveLengths[0] != newChGAI.waveLengths[0])
        {
            tipStr += tipStr.isEmpty() ? tr("主波长") : tr("、主波长");
        }

        if (oldChGAI->waveLengths.size() > 1 && oldChGAI->waveLengths[1] != newChGAI.waveLengths[1])
        {
            tipStr += tipStr.isEmpty() ? tr("次波长") : tr("、次波长");
        }
       
        if (oldChGAI->reagentCode != newChGAI.reagentCode)
        {
            tipStr += tipStr.isEmpty() ? tr("试剂编号") : tr("、试剂编号");
        }

        if (oldChGAI->calibrationType != newChGAI.calibrationType)
        {
            tipStr += tipStr.isEmpty() ? tr("校准方法") : tr("、校准方法");
        }

        if (oldChGAI->caliQuantity != newChGAI.caliQuantity)
        {
            tipStr += tipStr.isEmpty() ? tr("校准点") : tr("、校准点");
        }

        if (oldChGAI->enableWeightCoeff != newChGAI.enableWeightCoeff)
        {
            tipStr += tipStr.isEmpty() ? tr("自动加权系数") : tr("、自动加权系数");
        }

        if (oldChGAI->caliAspirateVols != newChGAI.caliAspirateVols)
        {
            tipStr += tipStr.isEmpty() ? tr("稀释浓度设置") : tr("、稀释浓度设置");
        }
    }

    const auto& oldChSAI = CIM->GetChemistrySpecialAssayInfo(newChSAI.assayCode,
        newChSAI.deviceType, newChSAI.sampleSourceType, newChSAI.version);
    if (oldChSAI.size() == 1)
    {
        if (oldChSAI[0]->sampleAspirateVols != newChSAI.sampleAspirateVols)
        {
            tipStr += tipStr.isEmpty() ? tr("样本量") : tr("、样本量");
        }
    }

    if (tipStr.isEmpty())
    {
        return true;
    }

    tipStr = tr("修改了项目参数中的") + tipStr + tr(
        "，此修改会影响校准品浓度、质控品浓度，修改后原有的试剂瓶工作曲线将清空，请确认是否修改");
    if (TipDlg(tr("提示"), tipStr, TipDlgType::TWO_BUTTON).exec() == QDialog::Rejected)
    {
        return false;
    }

    return true;
}

bool AssayParamTabWidget::SaveChAssayParam(const AssayListModel::StAssayListRowItem& item, QString& strOptLog)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    OnErr(item.assayCode >= ASSAY_CODE_TEST1 && item.assayCode <= ASSAY_CODE_TEST6,
        tr("此项目是特殊的测试项目不能修改！"));
    
    ::tf::GeneralAssayInfo gai;
    ::ch::tf::GeneralAssayInfo chGai;
    ::ch::tf::SpecialAssayInfo chSai;
    std::vector<std::shared_ptr<tf::QcDoc>> changedQc;

    if (!m_analyParamWidget->GetAnalysisParam(gai, chGai, chSai, item.assayCode)
        || !m_calibrateSettingWidget->GetAnalysisParam(chGai, item.assayCode)
        || !m_qcParamSettingsWidget->GetQcParameter(gai, changedQc)
        || !m_rangeParamWidget->GetAnalysisParam(chGai, chSai, item.assayCode))
    {
        ULOG(LOG_INFO, "GetAnalysisParam Failed! code:%d", item.assayCode);
        return false;
    }

    chGai.__set_linearExtension(m_rangeParamWidget->GetLinearExtension());

    if (!CheckIsModifyCaliParams(chGai, chSai))
    {
        ULOG(LOG_INFO, "User cancel save, this assay code:%d", item.assayCode);
        return false;
    }

    // 获取修改前的双向同测后项目
    int oldTwinsTestAssayCode = -1;
    const auto& CIM = CommonInformationManager::GetInstance();
    auto spSaiVec = CIM->GetChemistrySpecialAssayInfo(item.assayCode, 
            (tf::DeviceType::type)item.model, -1, item.version.toStdString());
    
    for (const auto& spSai : spSaiVec)
    {
        if (spSai->__isset.iTwinsTestAssayCode)
        {
            oldTwinsTestAssayCode = spSai->iTwinsTestAssayCode;
            break;
        }
    }

    gai.__set_assayClassify((tf::AssayClassify::type)item.assayClassify);

    // 新增项目时默认为不启用，编辑项目时默认为启用
    chGai.__set_enable(!spSaiVec.empty());

    const auto& spGai = CIM->GetAssayInfo(item.assayCode);
    
    // 编辑项目
    if (spGai != nullptr)
    {
        strOptLog = tr("修改项目") + item.name + tr("的参数");

        // 设置通用项目id
        gai.__set_id(spGai->id);

        const auto& oldGai = CIM->GetChemistryAssayInfo(item.assayCode, (tf::DeviceType::type)item.model, item.version.toStdString());
        if (oldGai != nullptr)
        {
            // 修改已有的参数版本的项目参数信息
            chGai.__set_id(oldGai->id);
        }

        // 检查项目的样本类型，删除未勾选的样本类型
        auto curSampleSources = m_analyParamWidget->GetCurAssaySampleSource();
        
        for (auto& spOldSai : spSaiVec)
        {
            if (spOldSai->sampleSourceType == chSai.sampleSourceType)
            {
                chSai.__set_id(spOldSai->id);
            }

            bool deleteAssay = false;
            if (!curSampleSources.contains(spOldSai->sampleSourceType))
            {
                ::ch::tf::SpecialAssayInfoQueryCond saiQc;
                saiQc.__set_id(spOldSai->id);
                if (!ch::LogicControlProxy::DeleteSpecialAssayInfo(saiQc))
                {
                    ULOG(LOG_ERROR, "delete ch asaay special params faield!");
                    OnErr(true, tr("保存分析参数失败！"));
                }

                deleteAssay = true;
                if (spOldSai->priority)
                {
                    // 设置当前的样本优先级为最高
                    chSai.__set_priority(true);
                }
            }
        }

        // 开始保存,先修改生化特殊项目信息
        auto ret = ch::LogicControlProxy::ModifyAssayInfo(gai, chGai, chSai);
        if (::tf::ThriftResult::THRIFT_RESULT_SUCCESS != ret)
        {
            if (::tf::ThriftResult::THRIFT_RESULT_INVALID_CONFIG_TWINS_TEST == ret)
            {
                OnErr(true, tr("值错误：双向同测"));
            }

            ULOG(LOG_ERROR, "save ch asaay params faield!");
            OnErr(true, tr("保存分析参数失败！"));
        }

        // 更新项目设置的分析页面的参数版本下拉框
        if (chSai.__isset.version && item.version.toStdString() != chSai.version)
        {
            m_analyParamWidget->UpdateVersionCommbox(item.version.toInt(), atoi(chSai.version.c_str()));
        }

        // 更新优先样本类型下拉框
        m_analyParamWidget->UpdateFirstSampleTypeCommbox(chSai.assayCode, chSai.deviceType, chSai.version);
        
        // 更新CommonInformationManager双向同测的缓存
        if (chSai.__isset.iTwinsTestAssayCode && chSai.iTwinsTestAssayCode != oldTwinsTestAssayCode)
        {
            CIM->ReloadAssayInfoChSpecialAfterEditTwins(item.assayCode, chSai.iTwinsTestAssayCode, oldTwinsTestAssayCode);
        }
    }
    // 新增项目
    else
    {
        strOptLog = tr("新增项目") + item.name + tr("的参数");

        // 新增项目的第一个新增的样本类型设置为最高优先级，设置当前的样本优先级为最高
        chSai.__set_priority(true);

        // 是否为开放项目
        gai.__set_openAssay(item.assayCode >= tf::AssayCodeRange::CH_OPEN_RANGE_MIN
                            && item.assayCode <= tf::AssayCodeRange::CH_OPEN_RANGE_MAX);

        // 新增项目的第一个参数版本一定是启用的
        chGai.__set_enable(true);

        // 保存
        if (!ch::LogicControlProxy::AddAssayInfo(gai, chGai, chSai))
        {
            ULOG(LOG_ERROR, "save ch asaay params faield!");
            OnErr(true, tr("保存分析参数失败！"));
        }

        // 更新优先样本类型下拉框
        m_analyParamWidget->UpdateFirstSampleTypeCommbox(chSai.assayCode, chSai.deviceType, chSai.version);       
    }

    // 保存质控设置
    if (!SaveQcDoc(changedQc))
    {
        ULOG(LOG_WARN, "Failed to save qc config to db.(code:%d)", item.assayCode);
        return false;
    }

    // 通知数据浏览界面刷新项目参数
    POST_MESSAGE(MSG_ID_USER_DEF_ASSAY_PARAM_UPDATE, item.assayCode);
    return true;
}

bool AssayParamTabWidget::SaveIseAssayParam(const AssayListModel::StAssayListRowItem& item)
{
    ::tf::GeneralAssayInfo gai;
    ::ise::tf::GeneralAssayInfo iseGai;
    ::ise::tf::SpecialAssayInfo iseSai;
    std::vector<std::shared_ptr<tf::QcDoc>> changedQc;

    if (!m_AnalysisParamIsewidget->GetAnalysisParam(gai, iseGai, item.assayCode)
        || !m_IseCaliSettingWidget->GetAnalysisParam(iseGai, iseSai, item.assayCode)
        || !m_qcParamSettingsWidget->GetQcParameter(gai, changedQc)
        || !m_rangeParamWidget->GetAnalysisParam(iseGai, iseSai, item.assayCode))
    {
        ULOG(LOG_ERROR, "GetAnalysisParam Failed! code:%d", item.assayCode);
        return false;
    }

    // 获取当前项目的id
    auto& CIM = CommonInformationManager::GetInstance();
    gai.__set_id(CIM->GetAssayInfo(item.assayCode)->id);
    iseGai.__set_id(CIM->GetIseAssayInfo(item.assayCode, (tf::DeviceType::type)item.model, item.version.toStdString())->id);

    auto spSai = CIM->GetIseSpecialAssayInfo(item.assayCode,
        (tf::SampleSourceType::type)iseSai.sampleSourceType,(tf::DeviceType::type)item.model);

    if (spSai != nullptr)
    {
       iseSai.__set_id(spSai->id);
    }

    // 修改数据
    if (!DcsControlProxy::GetInstance()->ModifyGeneralAssayInfo(gai)
        || !ise::LogicControlProxy::ModifyAssayConfigInfo(iseGai)
        || !ise::LogicControlProxy::ModifySpecialAssayInfo(iseSai))
    {
        ULOG(LOG_ERROR, "%s : ModifyAssayConfigInfo Failed!", __FUNCTION__);
        OnErr(true, tr("保存分析参数失败！"));
    }

    // 保存质控设置
    if (!SaveQcDoc(changedQc))
    {
        ULOG(LOG_WARN, "Failed to save qc config to db.(code:%d)", item.assayCode);
        OnErr(true, tr("保存分析参数失败！"));
    }

    // 通知数据浏览界面刷新项目参数
    POST_MESSAGE(MSG_ID_USER_DEF_ASSAY_PARAM_UPDATE, item.assayCode);
    return true;
}

///
/// @brief 
///
/// @param[in]  item		item 保存项目定位信息
/// @param[in]  strOptLog  	strOptLog 操作日志
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2024年3月7日，增加操作日志中的细节描述信息
///
bool AssayParamTabWidget::SaveImAssayParam(const AssayListModel::StAssayListRowItem& item, QString& strOptLog)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 从CommMgr中获取项目信息
    std::shared_ptr<CommonInformationManager> &assayMgr = CommonInformationManager::GetInstance();
    std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>> vecImAssayInfo;
    assayMgr->GetImmuneAssayInfos(vecImAssayInfo, item.assayCode, ::tf::DeviceType::DEVICE_TYPE_I6000);
    auto spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(item.assayCode);
    if (spAssayInfo == nullptr)
    {
        return false;
    }

    // 拷贝成临时对象
    std::shared_ptr<::tf::GeneralAssayInfo> tmpAssayInfo = std::make_shared<::tf::GeneralAssayInfo>(*spAssayInfo);
    std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>> tmpImAssayInfo;
    for (const std::shared_ptr<::im::tf::GeneralAssayInfo>& imPs : vecImAssayInfo)
    {
        tmpImAssayInfo.push_back(std::make_shared<::im::tf::GeneralAssayInfo>(*imPs));
    }

	// 此项目的质控信息
	tf::QcDocQueryResp qcDocRep;
	DcsControlProxy::GetInstance()->QueryQcDocByAssayCode(qcDocRep, item.assayCode);

    // 获取各个界面的设置
    std::vector<std::shared_ptr<tf::QcDoc>> changedQc;
    bool bAnasy = m_ImAnalysisParamWidget->GetAnalysisParam(item, tmpAssayInfo, tmpImAssayInfo, changedQc);
    bool bRange = m_ImRangeParamWidget->GetRangeParameter(item, tmpImAssayInfo);
    bool bCali = m_ImCaliSettingWidget->GetAnalysisParam(item, tmpImAssayInfo);
    bool bQc = m_qcParamSettingsWidget->GetQcParameter(*tmpAssayInfo, changedQc);

    if (!bAnasy || !bRange || !bCali || !bQc) // 
    {
        ULOG(LOG_WARN, "Failed to get assayinfo settings.");
        return false;
    }

    // 保存通用项目信息
    if (!DcsControlProxy::GetInstance()->ModifyGeneralAssayInfo(*tmpAssayInfo))
    {
        ULOG(LOG_WARN, "Failed to save assayinfo settings.");
        return false;
    }

	// 获取免疫项目操作细节信息
	GetImAssayOptLog(strOptLog, tmpAssayInfo, tmpImAssayInfo, changedQc, qcDocRep.lstQcDocs);

    // 0726冒烟测试必改缺陷，修改项目名称之后，映射信息错误——mod_tcx
    //*spAssayInfo = *tmpAssayInfo;
    assayMgr->UpdateGnrAssayInfo(tmpAssayInfo);

    // 保存免疫项目信息
    int idx = 0;
    for (const auto& imAssay : tmpImAssayInfo)
    {
        if (!im::LogicControlProxy::ModifyGeneralAssayInfo(*imAssay))
        {
            ULOG(LOG_WARN, "Failed to save imAssayinfo settings.");
            idx++;
            continue;
        }
        if (idx < vecImAssayInfo.size())
        {
            *(vecImAssayInfo[idx]) = *imAssay;
            idx++;
        }
    }

    // 保存质控设置
    if (!SaveQcDoc(changedQc))
    {
        ULOG(LOG_WARN, "Failed to save qc config to db.(code:%d)", item.assayCode);
        return false;
    }

    // 通知数据浏览界面刷新项目参数
    POST_MESSAGE(MSG_ID_USER_DEF_ASSAY_PARAM_UPDATE, item.assayCode);
    return true;
}

///
/// @brief 界面显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月24日，新建函数
///
void AssayParamTabWidget::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
}

///
/// @brief 窗口显示事件
///     
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月24日，新建函数
///
void AssayParamTabWidget::showEvent(QShowEvent *event)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 让基类处理事件
    QTabWidget::showEvent(event);

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }
}

void AssayParamTabWidget::ChangeCurrParamVersion(const QString& strVer)
{
    if (strVer.isEmpty())
    {
        return;
    }
    m_currentItem.version = strVer;
};

///
/// @brief
///     切换界面的显示
///
///@param[in]    style	新的样式 
///
/// @return     
///
/// @par History:
/// @li 6950/ChenFei，2022年05月24日，新建函数
///
void AssayParamTabWidget::SwitchShowStyle(ShowStyle style)
{
	if (style == m_lastShowStyle)
	{
		return;
	}

	this->clear();
    m_tabInfo.clear();

	switch (style)
	{
	case ShowStyle::CHEMISTRY: {
		// 分析
        ADD_TAB(TabAnalsys, m_analyParamWidget);
		// 校准
        ADD_TAB(TabCali, m_calibrateSettingWidget);
        // 质控页签
        ADD_TAB(TabQc, m_qcParamSettingsWidget);
		// 范围
        ADD_TAB(TabRange, m_rangeParamWidget);
		break;
	}

    case ShowStyle::SIND: {
        ADD_TAB(TabAnalsys, m_analyParamSindWidget);
		break;
	}

	case ShowStyle::NA_K_CL: {
		// 分析
        ADD_TAB(TabAnalsys, m_AnalysisParamIsewidget);
		// 校准
        ADD_TAB(TabCali, m_IseCaliSettingWidget);
        // 质控
        ADD_TAB(TabQc, m_qcParamSettingsWidget);
		// 范围
        ADD_TAB(TabRange, m_rangeParamWidget);
		break;

	case ShowStyle::IMMUNE:{
		// 分析
        ADD_TAB(TabAnalsys, m_ImAnalysisParamWidget);
		// 校准
        ADD_TAB(TabCali, m_ImCaliSettingWidget);
        // 质控页签
        ADD_TAB(TabQc, m_qcParamSettingsWidget);
        // 范围
        ADD_TAB(TabRange, m_ImRangeParamWidget);
		break;
	}
	}
	default:
		break;
	}

    OnPermisionChanged();
	m_lastShowStyle = style;
}

///
/// @brief 获取更改操作细节信息
///
/// @param[in]  strOptLog  返回的日志记录信息
/// @param[in]  spAssayInfoUi  界面修改的项目信息
/// @param[in]  vecImAssayInfoUi  界面修改的免疫项目信息
/// @param[in]  spChangeQc  界面修改的项目质控信息
/// @param[in]  vecQcDocs  修改前的项目质控信息
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2024年3月7日，新建函数
///
void AssayParamTabWidget::GetImAssayOptLog(QString& strOptLog, const std::shared_ptr<tf::GeneralAssayInfo> spAssayInfoUi, const std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>>& vecImAssayInfoUi, const std::vector<std::shared_ptr<tf::QcDoc>>& spChangeQc, const std::vector<tf::QcDoc>& vecQcDocs)
{
	// 参数检查
	if (nullptr == spAssayInfoUi)
	{
		ULOG(LOG_WARN, "The input param is nullptr.");
		return;
	}
	if (nullptr == m_ImAnalysisParamWidget)
	{
		ULOG(LOG_WARN, "The im analysis parame widget is nullptr.");
		return;
	}
	if (nullptr == m_ImCaliSettingWidget)
	{
		ULOG(LOG_WARN, "The im calibrate setting widget is nullptr.");
		return;
	}
	if (nullptr == m_qcParamSettingsWidget)
	{
		ULOG(LOG_WARN, "The im qc param setting widget is nullptr.");
		return;
	}
	if (nullptr == m_ImRangeParamWidget)
	{
		ULOG(LOG_WARN, "The im range param widget is nullptr.");
		return;
	}
	// 免疫项目基础信息
	m_ImAnalysisParamWidget->GetImAssayOptLog(strOptLog, spAssayInfoUi, vecImAssayInfoUi);
	// 免疫项目校准参数信息
	m_ImCaliSettingWidget->GetImAssayOptLog(strOptLog, spAssayInfoUi, vecImAssayInfoUi);
	// 质控参数设置信息
	m_qcParamSettingsWidget->GetImAssayOptLog(strOptLog, spAssayInfoUi, spChangeQc, vecQcDocs);
	// 免疫参数范围（复查）设置界面信息
	m_ImRangeParamWidget->GetImAssayOptLog(strOptLog, spAssayInfoUi, vecImAssayInfoUi);
}

void AssayParamTabWidget::OnCurrentChanged(int index)
{
	ULOG(LOG_INFO, "%s(%d)", __FUNCTION__, m_currentItem.assayCode);

	if (m_currentItem.assayCode == INVALID_ASSAY_CODE)
	{
		return;
	}

    // 加载界面
    switch ((tf::AssayClassify::type)m_currentItem.assayClassify)
    {
        // 四个页面一起保存，切换tab页的时候可能就不需要刷新页面了
        case tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE:
            {
                m_ImAnalysisParamWidget->LoadAnalysisParam(m_currentItem);
                m_ImRangeParamWidget->LoadRangeParameter(m_currentItem);
                m_ImCaliSettingWidget->LoadAnalysisParam(m_currentItem);
                break;
            }

        default:
            break;
    }
}

///
/// @bref
///		当前用户权限变更处理
///
/// @par History:
/// @li 8276/huchunli, 2023年4月3日，新建函数
///
void AssayParamTabWidget::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 如果用户还没有登录时，初始化不进行删除页签操作
    SEG_NO_LOGIN_RETURN;

    // 处理前暂存当前页签
    QWidget* pCurrentTab = this->currentWidget();

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();
}

void AssayParamTabWidget::OnchemistryPage()
{
    m_itemType = 0;
	SwitchShowStyle(ShowStyle::CHEMISTRY);
}

void AssayParamTabWidget::OnimmunePage()
{
    m_itemType = 1;
	SwitchShowStyle(ShowStyle::IMMUNE);
}

QString AssayParamTabWidget::TabItem::GetName() const
{
    switch (m_tbType)
    {
    case AssayParamTabWidget::TabAnalsys:
        return tr("分析");
    case AssayParamTabWidget::TabCali:
        return tr("校准");
    case AssayParamTabWidget::TabQc:
        return tr("质控");
    case AssayParamTabWidget::TabRange:
        return tr("范围");
    default:
        break;
    }

    return "";
}
