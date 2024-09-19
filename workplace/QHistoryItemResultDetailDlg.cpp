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
/// @file     QHistoryItemResultDetailDlg.cpp
/// @brief    结果详情对话框
///
/// @author   4170/TangChuXian
/// @date     2022年10月13日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月13日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "QHistoryItemResultDetailDlg.h"
#include "ui_QHistoryItemResultDetailDlg.h"
#include <QPushButton>
#include <QLabel>
#include <QTabWidget>
#include "src/db/HistoryData.hxx"
#include "imassayresultinfowidget.h"
#include "QHistoryAIIdentifyDlg.h"
#include "QAssayResultView.h"
#include "CReadOnlyDelegate.h"
#include "WorkpageCommon.h"
#include "QSampleAssayModel.h"

#include "shared/CommonInformationManager.h"
#include "shared/uicommon.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/datetimefmttool.h"
#include "shared/ThriftEnumTransform.h"

#include "thrift/DcsControlProxy.h"
#include "thrift/im/i6000/I6000LogicControlProxy.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "thrift/ise/ise1005/Ise1005LogicControlProxy.h"
#include "src/thrift/ise/gen-cpp/ise_constants.h"
#include "src/common/Mlog/mlog.h"
#include "manager/DictionaryQueryManager.h"
#include "QHistorySampleAssayModel.h"
#include "manager/UserInfoManager.h"

#define  STACK_WIDGET_INDEX_CH                               (0)          // 生化项目详情界面索引
#define  STACK_WIDGET_INDEX_IM                               (1)          // 免疫项目详情界面索引

#define PROPERTY_RETEST_ID									("property_retest_id")
#define PROPERTY_RETEST_VOLVE								("property_retest_volue")
#define PROPERTY_RETEST_STATUS								("property_retest_status")

Q_DECLARE_METATYPE(tf::TestItem)
Q_DECLARE_METATYPE(im::tf::AssayTestResult)
Q_DECLARE_METATYPE(ch::tf::AssayTestResult)
Q_DECLARE_METATYPE(ise::tf::AssayTestResult)
Q_DECLARE_METATYPE(::tf::HistoryBaseDataUserUseRetestUpdate)
Q_DECLARE_METATYPE(std::string)


QHistoryItemResultDetailDlg::QHistoryItemResultDetailDlg(QWidget *parent)
    : BaseDlg(parent)
	, m_AIIdentifyDlg(nullptr)
    , m_bInit(false)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化UI对象
    ui = new Ui::QHistoryItemResultDetailDlg();
    ui->setupUi(this);
    SetTitleName(tr("结果详情"));
    // 界面显示前初始化
    InitBeforeShow();
	InitAfterShow();
	
#if 0 // 历史数据不能设置复查结果； bug0027582 的修改；历史页面不管是什么权限角色，都不显示修改结果与保存按钮
    // bug26568 by zhangjing 修改为隐藏
    bool bSave = UserInfoManager::GetInstance()->IsPermisson(PSM_IM_HISTORY_MANUAL_EDITRESULT);
    ui->SaveBtn->setVisible(bSave);
    bool breset = UserInfoManager::GetInstance()->IsPermisson(PSM_IM_HISTORY_AS_RECHECK_RESLT);
    ui->button_setRetest->setVisible(breset);
#else
    ui->button_setRetest->hide();
    ui->SaveBtn->hide();
#endif

    ui->AssayResTbl->installEventFilter(this);
}

QHistoryItemResultDetailDlg::~QHistoryItemResultDetailDlg()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     清空内容
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月26日，新建函数
///
void QHistoryItemResultDetailDlg::ClearContents()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 清空样本结果表
    ui->AssayResTbl->clearContents();

    // 清空选中项目详情
    ClearSelItemInfo();

    // 清空病人信息
    ClearPatientInfo();
}

///
/// @brief 更新结果详情
///   
/// @param[in]  data	tuple结构：测试项名称，结果，最后一次复查结果
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年1月6日，新建函数
///
void QHistoryItemResultDetailDlg::Update(const std::tuple<int64_t, std::vector<std::shared_ptr<HistoryBaseDataItem>>, bool>& data)
{
	ClearContents();

	bool bIsPatient = std::get<2>(data);
	InitAssayResTable(bIsPatient);
	if (bIsPatient)
	{
		for (auto& item : ui->PatienInfoFrame->findChildren<QLabel*>())
		{
			item->show();
		}

		for (auto& item : ui->frame->findChildren<QLabel*>())
		{
			item->show();
		}
	}
	else
	{
		for (auto& item : ui->PatienInfoFrame->findChildren<QLabel*>())
		{
			item->hide();
		}

		for (auto& item : ui->frame->findChildren<QLabel*>())
		{
			item->hide();
		}
	}


	// 计算项目不显示，因此先去除其中的计算项目
    std::vector<std::shared_ptr<HistoryBaseDataItem>> tempAssayResLst = std::get<1>(data);
	for (auto it = tempAssayResLst.begin(); it != tempAssayResLst.end();)
	{
		if (CommonInformationManager::IsCalcAssay((*it)->m_assayCode))
		{
			it = tempAssayResLst.erase(it);
		}
		else
		{
			it++;
		}
	}
	
    // 行数自适应
    ui->AssayResTbl->setRowCount(tempAssayResLst.size());

	m_mHistoryDetailData = data;
    // 防止后期获取队列包含计算项目-bug27306
    std::get<1>(m_mHistoryDetailData) = tempAssayResLst;

    int iPrecision = 2;

	// 显示设置
	ResultPrompt resultTips = DictionaryQueryManager::GetInstance()->GetResultTips();

	for (int index = 0; index < tempAssayResLst.size(); ++index)
	{
        const std::shared_ptr<HistoryBaseDataItem>& item = tempAssayResLst[index];

		stAssayResult firstResult, retestResult;
		QHistorySampleAssayModel::GetAssayResult(*item, false, firstResult);
		QHistorySampleAssayModel::GetAssayResult(*item, true, retestResult);
        QString itemName = QString::fromStdString(item->m_itemName);

		// 小数点后保留几位小数(默认2位)
		int iPrecision = 2;
		// 获取当前单位与主单位的转化倍率
		double factor = 1.0;

		// 获取当前单位，按倍率转换并显示
		if (CommonInformationManager::IsCalcAssay(item->m_assayCode))
		{
			continue;
            std::shared_ptr<::tf::CalcAssayInfo> spCalcInfo = CommonInformationManager::GetInstance()->GetCalcAssayInfo(item->m_assayCode);
			if (spCalcInfo != nullptr)
			{
				itemName = QString::fromStdString(spCalcInfo->name);
				iPrecision = spCalcInfo->decimalDigit;
			}
		}
		else
		{
            std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(item->m_assayCode);
			if (spAssayInfo != nullptr)
			{
				itemName = QString::fromStdString(spAssayInfo->assayName);
				iPrecision = spAssayInfo->decimalPlace;

				for (const auto& unit : spAssayInfo->units)
				{
					if (unit.isCurrent)
					{
						factor = unit.factor;
                        break;
					}
				}
			}
		}
		auto nameItem = new QTableWidgetItem(itemName);
		nameItem->setTextAlignment(Qt::AlignCenter);
        ui->AssayResTbl->setItem(index, 0, nameItem);
        auto firstResultItem = new QTableWidgetItem(firstResult.m_result);
		firstResultItem->setTextAlignment(Qt::AlignCenter);
        firstResultItem->setData(Qt::UserRole + 1, firstResult.m_fontColorType);

        // 危机值
        if (resultTips.enabledCritical)
        {
            firstResultItem->setData(Qt::UserRole + DELEGATE_COLOR_OFFSET_POS, QColor(firstResult.m_backgroundColor));
        }
        ui->AssayResTbl->setItem(index, 1, firstResultItem);

        // 复查
		// 只有病人样本才有复查结果，校准、质控无复查
		if (bIsPatient)
		{
			auto retestResultItem = new QTableWidgetItem(retestResult.m_result);
			retestResultItem->setTextAlignment(Qt::AlignCenter);
			retestResultItem->setData(Qt::UserRole + 1, retestResult.m_fontColorType);

			// 危机值
			if (resultTips.enabledCritical)
			{
				retestResultItem->setData(Qt::UserRole + DELEGATE_COLOR_OFFSET_POS, QColor(retestResult.m_backgroundColor));
			}

			ui->AssayResTbl->setItem(index, 2, retestResultItem);
		}
	}

	// 默认选中第一行
    ui->AssayResTbl->setFocus();
	if (ui->AssayResTbl->rowCount() > 0)
	{
        ui->AssayResTbl->selectRow(0);
	}
}

///
/// @brief 设置上一条数据按钮是否可用
///
/// @param[in]  bPreEnable  
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年1月9日，新建函数
///
void QHistoryItemResultDetailDlg::SetPreBtnEnable(bool bPreEnable)
{
	ui->PrevBtn->setEnabled(bPreEnable);
}

///
/// @brief 设置下一条数据按钮是否可用
///
/// @param[in]  bNextEnable  
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年1月9日，新建函数
///
void QHistoryItemResultDetailDlg::SetNextBtnEnable(bool bNextEnable)
{
	ui->NextBtn->setEnabled(bNextEnable);
}

///
/// @brief
///     清空选中项信息
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月20日，新建函数
///
void QHistoryItemResultDetailDlg::ClearSelItemInfo()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    auto FuncClearItemDeltail = [this](int tabIdx) {
        RFlagTabWidget* pChTabWgt = qobject_cast<RFlagTabWidget*>(ui->AssayDetailStkWgt->widget(tabIdx));
        if (pChTabWgt == Q_NULLPTR)
        {
            ULOG(LOG_WARN, "Not exist tabwidget.");
            return;
        }
        pChTabWgt->Clear();
    };

    // 清空选中生化项目项目详情信息
    FuncClearItemDeltail(STACK_WIDGET_INDEX_CH);

    // 清空选中免疫项目项目详情信息
    FuncClearItemDeltail(STACK_WIDGET_INDEX_IM);
}

///
/// @brief
///     清空选中项目病人信息
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月20日，新建函数
///
void QHistoryItemResultDetailDlg::ClearPatientInfo()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 清空病人信息
    ui->NameValLab->clear();
    ui->GenderValLab->clear();
    ui->AgeValLab->clear();
    ui->BarcodeValLab->clear();
    ui->SampNoValLab->clear();
    ui->CaseNoValLab->clear();
    ui->DepartmentValLab->clear();
    ui->VisitingDoctorValLab->clear();
    ui->DiagNosisValLab->clear();
    ui->RemarkValLab->clear();
}

///
/// @brief
///     显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月13日，新建函数
///
void QHistoryItemResultDetailDlg::InitBeforeShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 插入生化项目详情信息（待完善）
	RFlagTabWidget* pChTabWgt = new RFlagTabWidget();
	connect(pChTabWgt, &RFlagTabWidget::SignalCurrentTabIsRFlag, this, [&](bool d) {
		ui->button_setRetest->setEnabled(!d);
	});
    //pChTabWgt->AddTab(new QWidget(), tr("检测结果"));

    ui->AssayDetailStkWgt->insertWidget(STACK_WIDGET_INDEX_CH, pChTabWgt);

    // 插入免疫项目详情信息
	RFlagTabWidget* pImTabWgt = new RFlagTabWidget(this);
	connect(pImTabWgt, &RFlagTabWidget::SignalCurrentTabIsRFlag, this, [&](bool d) {
		ui->button_setRetest->setEnabled(!d);
	});
    pImTabWgt->AddTab(new ImAssayResultInfoWidget(), tr("检测结果"));
    pImTabWgt->AddTab(new ImAssayResultInfoWidget(), tr("复查结果"));
	// 联机的情况下，AI打开才显示
	bool isAi = QDataItemInfo::Instance().IsAiRecognition();
	// 是否联机
	bool isPipel = DictionaryQueryManager::GetInstance()->GetPipeLine();
	if (isAi && isPipel)
	{
		ui->button_AI->setVisible(true);
	}
	else
	{
		ui->button_AI->setVisible(false);
	}

    ui->AssayDetailStkWgt->insertWidget(STACK_WIDGET_INDEX_IM, pImTabWgt);

    // 默认显示生化
    ui->AssayDetailStkWgt->setCurrentIndex(STACK_WIDGET_INDEX_CH);
  
	// 清空信息
    ClearContents();
}

///
/// @brief
///     显示之后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月13日，新建函数
///
void QHistoryItemResultDetailDlg::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化信号槽连接
    InitConnect();

    // 初始化子控件
    InitChildCtrl();
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月13日，新建函数
///
void QHistoryItemResultDetailDlg::InitChildCtrl()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 历史数据结果详情界面删除按钮不可用
	ui->DelBtn->setEnabled(false);
	ui->DelBtn->setHidden(true);
}

///
/// @brief
///     初始化连接
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月13日，新建函数
///
void QHistoryItemResultDetailDlg::InitConnect()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 关闭按钮被点击
    connect(ui->CloseBtn, SIGNAL(clicked()), this, SLOT(reject()));

    // 上一条按钮被点击
    connect(ui->PrevBtn, SIGNAL(clicked()), this, SIGNAL(SigPrevBtnClicked()));

    // 下一条按钮被点击
    connect(ui->NextBtn, SIGNAL(clicked()), this, SIGNAL(SigNextBtnClicked()));

    // 删除按钮被点击
    connect(ui->DelBtn, SIGNAL(clicked()), this, SLOT(OnDelBtnClicked()));

    // 保存按钮被点击
    connect(ui->SaveBtn, SIGNAL(clicked()), this, SLOT(OnSaveBtnClicked()));
    
    // 恢复图例
    connect(ui->reset_curve, SIGNAL(clicked()), this, SLOT(OnResetCurve()));

	// 设置复查结果
	connect(ui->button_setRetest, SIGNAL(clicked()), this, SLOT(OnSetRetestResult()));

	// 视觉识别结果
	connect(ui->button_AI, SIGNAL(clicked()), this, SLOT(OnAIIdentify()));

    // 项目结果表选中项改变
    QItemSelectionModel* selItemModel = ui->AssayResTbl->selectionModel();
    if (selItemModel != Q_NULLPTR)
    {
        // 项目信息更新
        connect(selItemModel, SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(OnItemTblCurIndexChanged(const QModelIndex&)));
    }
}

///
/// @brief
///     删除按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月13日，新建函数
///
void QHistoryItemResultDetailDlg::OnDelBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
}

///
/// @brief
///     保存按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月13日，新建函数
///
void QHistoryItemResultDetailDlg::OnSaveBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
	// 保存免疫数据
	if (ui->AssayDetailStkWgt->currentIndex() == STACK_WIDGET_INDEX_IM)
	{
		QTabWidget* pImTabWgt = qobject_cast<QTabWidget*>(ui->AssayDetailStkWgt->widget(STACK_WIDGET_INDEX_IM));
		if (pImTabWgt == Q_NULLPTR)
		{
			return;
		}

		auto assayResult = static_cast<ImAssayResultInfoWidget*>(pImTabWgt->currentWidget());
		if (assayResult == Q_NULLPTR)
		{
			return;
		}

		// 保存数据
		assayResult->Save();
	}
	// 保存生化数据
	else if (ui->AssayDetailStkWgt->currentIndex() == STACK_WIDGET_INDEX_CH)
	{
		QTabWidget* pChTabWidget = qobject_cast<QTabWidget*>(ui->AssayDetailStkWgt->widget(STACK_WIDGET_INDEX_CH));
		if (pChTabWidget == Q_NULLPTR)
		{
			ULOG(LOG_ERROR, "%s(), pChTabWidget == Q_NULLPTR,", __FUNCTION__);
			return;
		}

		auto assayResult = static_cast<QAssayResultView*>(pChTabWidget->currentWidget());
		if (assayResult == Q_NULLPTR)
		{
			return;
		}

		// 保存数据
		assayResult->SaveAssayResultEdit();
	}

    // 将样本信息更新消息发送到UI消息总线  
    if (std::get<1>(m_mHistoryDetailData).size() > 0)
    {
        std::vector<tf::SampleInfo> lst;
        int64_t  sampleId = std::get<1>(m_mHistoryDetailData).at(0)->m_sampleId;
        ::tf::SampleInfoQueryCond stuSampQryCond;
        ::tf::SampleInfoQueryResp stuSampQryResp;
        stuSampQryCond.__set_id(sampleId);
        stuSampQryCond.__set_containTestItems(true);
        // 执行查询条件
        if (DcsControlProxy::GetInstance()->QuerySampleInfo(stuSampQryCond, stuSampQryResp)
            && stuSampQryResp.result == ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
            && stuSampQryResp.lstSampleInfos.size() > 0)
        {
            auto stuSelSampleInfo = stuSampQryResp.lstSampleInfos[0];
            lst.push_back(stuSelSampleInfo);
            POST_MESSAGE(MSG_ID_SAMPLE_INFO_UPDATE, ::tf::UpdateType::type::UPDATE_TYPE_MODIFY, const_cast<std::vector<tf::SampleInfo>&>((lst)));
        }
    }
    // 数据被修改，通知上层刷新数据
    emit dataChanged();
}

///
/// @brief 恢复图例
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年12月6日，新建函数
///
void QHistoryItemResultDetailDlg::OnResetCurve()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	QTabWidget* pChTabWidget = qobject_cast<QTabWidget*>(ui->AssayDetailStkWgt->widget(STACK_WIDGET_INDEX_CH));
	if (pChTabWidget == Q_NULLPTR)
	{
		ULOG(LOG_ERROR, "%s(), pChTabWidget == Q_NULLPTR,", __FUNCTION__);
		return;
	}

	auto assayResult = static_cast<QAssayResultView*>(pChTabWidget->currentWidget());
	if (assayResult == Q_NULLPTR)
	{
		return;
	}

	// 重置曲线
	assayResult->ResetCurve();
}

///
/// @brief
///     项目结果表当前选中项改变
///
/// @param[in]  curIndex 当前选中项索引
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月20日，新建函数
///
void QHistoryItemResultDetailDlg::OnItemTblCurIndexChanged(const QModelIndex& curIndex)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (!curIndex.isValid())
    {
        ULOG(LOG_WARN, "Invalid model index.");
		return;
	}
	if (std::get<1>(m_mHistoryDetailData).size() <= curIndex.row())
    {
        ULOG(LOG_WARN, "Invalid index of history detaildata.");
		return;
	}
	const auto &historyItem = std::get<1>(m_mHistoryDetailData).at(curIndex.row());
	if (historyItem == nullptr)
    {
        ULOG(LOG_WARN, "Null history item.");
		return;
	}

	auto sampleID = std::get<0>(m_mHistoryDetailData);
	const auto& itemID = std::get<1>(m_mHistoryDetailData).at(curIndex.row())->m_itemId;
	auto retestID = std::get<1>(m_mHistoryDetailData).at(curIndex.row())->m_userUseRetestResultID;
	// 根据选中项目是生化还是免疫切换到不同界面
	auto deviceSN = std::get<1>(m_mHistoryDetailData).at(curIndex.row())->m_deviceSN;

	// 样本状态
	// 历史数据中的测试项都已检测完成
	QString strStatus;
	if (historyItem->m_retest)
	{
		strStatus += QString(" ") + tr("复查");
	}
	strStatus += QString(" ") + tr("已完成");
	if (historyItem->m_sendLis)
	{
		strStatus += QString(" ") + tr("已上传");
	}
	strStatus = strStatus.trimmed();
	ui->label_2->setText(strStatus);

	// 样本类型
	ui->label_4->setText(ThriftEnumTrans::GetSourceTypeName(historyItem->m_sampleSourceType));
	// 架号/位置号
	ui->label_6->setText(QString::fromStdString(historyItem->m_pos));

	// 根据设备编号查询设备信息表确定是免疫还是生化
	// 根据设备编号查询设备类型区分是生化项目还是免疫项目还是ISE项目
	::tf::DeviceInfoQueryResp _deviceReturn;
	::tf::DeviceInfoQueryCond deviceDiqc;
	deviceDiqc.__set_deviceSN(deviceSN);
	DcsControlProxy::GetInstance()->QueryDeviceInfo(_deviceReturn, deviceDiqc);
	// 查询设备失败时退出
	if (_deviceReturn.result != ::tf::ThriftResult::type::THRIFT_RESULT_SUCCESS || _deviceReturn.lstDeviceInfos.size() != 1)
    {
        ULOG(LOG_WARN, "Failed to query device info by SN:%s.", deviceSN.c_str());
		return;
	}
	auto &deviceInfo = _deviceReturn.lstDeviceInfos[0];
	// 免疫项目
	if (deviceInfo.deviceClassify == ::tf::AssayClassify::type::ASSAY_CLASSIFY_IMMUNE)
	{
		RFlagTabWidget* pImTabWgt = dynamic_cast<RFlagTabWidget*>(ui->AssayDetailStkWgt->widget(STACK_WIDGET_INDEX_IM));
		if (pImTabWgt == Q_NULLPTR)
        {
            ULOG(LOG_WARN, "Null im tab widget at tbindex %d.", STACK_WIDGET_INDEX_IM);
			return;
		}

		// 免疫界面没有恢复比例按钮、AI识别按钮
		ui->reset_curve->setVisible(false);
		ui->button_AI->setVisible(false);
		ui->AssayDetailStkWgt->setCurrentIndex(STACK_WIDGET_INDEX_IM);

		// 查询结果表，根据查询创建检测结果和复查结果界面
		::im::tf::AssayTestResultQueryResp _return;
		::im::tf::AssayTestResultQueryCond trqc;
		trqc.__set_sampleInfoId(sampleID);
		trqc.__set_testItemId(itemID);
		::im::i6000::LogicControlProxy::QueryAssayTestResult(_return, trqc);
		if (_return.result != ::tf::ThriftResult::type::THRIFT_RESULT_SUCCESS || _return.lstAssayTestResult.empty())
        {
            im::tf::AssayTestResult nullImItem;
            ShowIMResult({ nullImItem }, historyItem, pImTabWgt, retestID);
			return;
		}

		// 当存在多次测试时，需要去除无结果的测试记录 add by wzx 20240402
		if (_return.lstAssayTestResult.size() > 1)
		{
			for (auto it = _return.lstAssayTestResult.begin(); it != _return.lstAssayTestResult.end(); )
			{
				if (it->conc < 0)
				{
					it = _return.lstAssayTestResult.erase(it);
				}
				else
				{
					it++;
				}

                // 保留一个结果，以保证展示页面存在数据
                if (_return.lstAssayTestResult.size() == 1)
                {
                    break;
                }
			}
		}

		// 界面完成时间显示为最后一次检测的完成时间
		std::string endTimeStr;
		std::for_each(_return.lstAssayTestResult.cbegin(), _return.lstAssayTestResult.cend(), [&endTimeStr](const im::tf::AssayTestResult& result)
		{
			if (endTimeStr < result.endTime)
			{
				endTimeStr = result.endTime;
			}
		});
		ui->label_endtime->setText(ToCfgFmtDateTime(QString::fromStdString(endTimeStr)));

		ShowIMResult(_return.lstAssayTestResult, historyItem, pImTabWgt, retestID);
	}
	// 生化项目
	else if (deviceInfo.deviceClassify == ::tf::AssayClassify::type::ASSAY_CLASSIFY_CHEMISTRY)
	{
		QTabWidget* pChTabWidget = qobject_cast<QTabWidget*>(ui->AssayDetailStkWgt->widget(STACK_WIDGET_INDEX_CH));
		if (pChTabWidget == Q_NULLPTR)
		{
			ULOG(LOG_ERROR, "%s(), pChTabWidget == Q_NULLPTR,", __FUNCTION__);
			return;
		}
		ui->reset_curve->setVisible(true);
		//ui->button_AI->setVisible(true);
		ui->AssayDetailStkWgt->setCurrentIndex(STACK_WIDGET_INDEX_CH);
		ch::tf::AssayTestResultQueryCond queryAssaycond;
		ch::tf::AssayTestResultQueryResp assayTestResult;
		// 根据测试项目的ID来查询结果
		queryAssaycond.__set_testItemId(itemID);

		if (!ch::c1005::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond)
			|| assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
			|| assayTestResult.lstAssayTestResult.empty())
        {
            ULOG(LOG_WARN, "Failed to query ch testresult item by id:%lld.", itemID);
			return;
		}

		// 界面完成时间显示为最后一次检测的完成时间
		std::string endTimeStr;
		std::for_each(assayTestResult.lstAssayTestResult.cbegin(), assayTestResult.lstAssayTestResult.cend(), [&endTimeStr](const ch::tf::AssayTestResult& result)
		{
			if (endTimeStr < result.endTime)
			{
				endTimeStr = result.endTime;
			}
		});
		ui->label_endtime->setText(ToCfgFmtDateTime(QString::fromStdString(endTimeStr)));

		auto rawAssayResultVec = assayTestResult.lstAssayTestResult;
		std::vector<ch::tf::AssayTestResult> dstResult;
		for (const auto& assayResult : rawAssayResultVec)
		{
			if (!assayResult.__isset.endTime)
			{
				continue;
			}

			// 若结果无法计算则不显示
			if (dstResult.empty() && assayResult.resultStatusCodes.find(u8"Calc.?") != std::string::npos)
			{
				continue;
			}

			dstResult.push_back(assayResult);
		}

		// 若经过筛选（结果无法计算的项目的不显示，导致没有结果可以显示），则显示最后一个
		if (dstResult.empty() && !rawAssayResultVec.empty())
		{
			// 获取最后一个项目
			int size = rawAssayResultVec.size();
			auto assayResult = rawAssayResultVec[size - 1];
			// 同时要求也是有时间的
			if (assayResult.__isset.endTime)
			{
				dstResult.push_back(rawAssayResultVec[0]);
			}
		}

		ShowCHResult(dstResult, historyItem, pChTabWidget, retestID);
	}
	// ISE项目
	else if (deviceInfo.deviceClassify == ::tf::AssayClassify::type::ASSAY_CLASSIFY_ISE)
	{
		// ISE结果详情界面显示在生化界面中
		QTabWidget* pChTabWidget = qobject_cast<QTabWidget*>(ui->AssayDetailStkWgt->widget(STACK_WIDGET_INDEX_CH));
		if (pChTabWidget == Q_NULLPTR)
		{
			ULOG(LOG_ERROR, "%s(), pChTabWidget == Q_NULLPTR,", __FUNCTION__);
			return;
		}
		// modify bug0011297 by wuht(ise的结果详情页面没有恢复原图和AI)
		ui->reset_curve->setVisible(false);
		ui->button_AI->setVisible(false);
		ui->AssayDetailStkWgt->setCurrentIndex(STACK_WIDGET_INDEX_CH);

		ise::tf::AssayTestResultQueryCond queryAssaycond;
		ise::tf::AssayTestResultQueryResp assayTestResult;
		// 根据测试项目的ID来查询结果
		queryAssaycond.__set_testItemId(itemID);

		if (!ise::ise1005::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond)
			|| assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
			|| assayTestResult.lstAssayTestResult.empty())
        {
            ULOG(LOG_WARN, "Failed to query ise1005 testresult item by id:%lld.", itemID);
			return;
		}

		// 界面完成时间显示为最后一次检测的完成时间
		std::string endTimeStr;
		std::for_each(assayTestResult.lstAssayTestResult.cbegin(), assayTestResult.lstAssayTestResult.cend(), [&endTimeStr](const ise::tf::AssayTestResult& result)
		{
			if (endTimeStr < result.endTime)
			{
				endTimeStr = result.endTime;
			}
		});
		ui->label_endtime->setText(ToCfgFmtDateTime(QString::fromStdString(endTimeStr)));
		// modify bug 0014073 by wuht
		auto rawAssayResultVec = assayTestResult.lstAssayTestResult;
		std::vector<ise::tf::AssayTestResult> dstResult;
		for (const auto& assayResult : rawAssayResultVec)
		{
			if (!assayResult.__isset.endTime)
			{
				continue;
			}

			// 若结果无法计算则不显示
			if (dstResult.empty() && assayResult.resultStatusCodes.find(u8"Calc.?") != std::string::npos)
			{
				continue;
			}

			dstResult.push_back(assayResult);
		}

		// 若经过筛选（结果无法计算的项目的不显示，导致没有结果可以显示），则显示最后一个
		if (dstResult.empty() && !rawAssayResultVec.empty())
		{
			// 获取最后一个项目
			int size = rawAssayResultVec.size();
			auto assayResult = rawAssayResultVec[size - 1];
			// 同时要求也是有时间的
			if (assayResult.__isset.endTime)
			{
				dstResult.push_back(rawAssayResultVec[0]);
			}
		}

		ShowISEResult(dstResult, historyItem, pChTabWidget, retestID);
	}
}

///
/// @brief 设置为复查结果
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年7月3日，新建函数
///
void QHistoryItemResultDetailDlg::OnSetRetestResult()
{
	auto curIndex = ui->AssayResTbl->currentIndex();
	if (!curIndex.isValid())
	{
		return;
	}

	QTabWidget* tabWgt = qobject_cast<QTabWidget*>(ui->AssayDetailStkWgt->currentWidget());
	if (tabWgt == nullptr)
	{
		return;
	}

	auto curTabWidget = tabWgt->currentWidget();
	if (curTabWidget == nullptr)
	{
		return;
	}

	auto retestID = curTabWidget->property(PROPERTY_RETEST_ID).toLongLong();
	if (retestID == -1)
	{
		return;
	}

	auto retestVolue = curTabWidget->property(PROPERTY_RETEST_VOLVE).toString().toStdString();
	auto retestStatus = curTabWidget->property(PROPERTY_RETEST_STATUS).toString().toStdString();

	const auto& itemID = std::get<1>(m_mHistoryDetailData).at(curIndex.row())->m_itemId;
	const auto& sampleID = std::get<1>(m_mHistoryDetailData).at(curIndex.row())->m_sampleId;
	const auto& deviceSN = std::get<1>(m_mHistoryDetailData).at(curIndex.row())->m_deviceSN;

	::tf::ResultLong _return;
	::tf::HistoryBaseDataUserUseRetestUpdate ri;

	ri.__set_itemId(itemID);
	ri.__set_userUseRetestResultId(retestID);
	ri.__set_reTestResult(retestVolue);
	ri.__set_reResultStatu(retestStatus);

	auto bRet = DcsControlProxy::GetInstance()->UpdateHistoryBaseDataUserUseRetestInfo(_return, ri);
	if (bRet)
	{
		std::get<1>(m_mHistoryDetailData)[curIndex.row()]->m_userUseRetestResultID = retestID;
		std::get<1>(m_mHistoryDetailData)[curIndex.row()]->m_reTestResult = retestVolue;
		std::get<1>(m_mHistoryDetailData)[curIndex.row()]->m_reTestResultStatu = retestStatus;

		auto tuple = std::tuple<std::string, std::string, int64_t>{ retestVolue, retestStatus, retestID };
		emit SigRetestResultChanged(tuple);
		POST_MESSAGE(MSG_ID_HIS_RETEST_RESULT_UPDATE, deviceSN, sampleID, ri);

		QTabWidget* tabWgt = qobject_cast<QTabWidget*>(ui->AssayDetailStkWgt->currentWidget());
		if (tabWgt == nullptr)
		{
			return;
		}
		auto tabIndex = tabWgt->currentIndex();
		// 更新后tabWidget会清空，因此需要再设一下当前窗口索引
		Update(m_mHistoryDetailData);
		tabWgt->setCurrentIndex(tabIndex);
	}
}

///
/// @brief 视觉识别结果
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年7月24日，新建函数
///
void QHistoryItemResultDetailDlg::OnAIIdentify()
{
	if (m_AIIdentifyDlg == nullptr)
	{
		m_AIIdentifyDlg = new QHistoryAIIdentifyDlg(this);
	}

	m_AIIdentifyDlg->exec();
}

///
/// @brief  设置lable显示
///
/// @param[in]  lab  lab控件
/// @param[in]  labTitle  lab控件(标题)
/// @param[in]  strItemName  字符串
///
/// @return 
///
/// @par History:
/// @li 1226/zhangjing，2024年9月2日，新建函数
///
void QHistoryItemResultDetailDlg::SetItemName(QLabel *lab, QLabel *labTitle, const QString& strItemName)
{
    if (nullptr == lab)
    {
        return;
    }
    int widthTitle = 0;
    if (nullptr != labTitle)
    {
        widthTitle = labTitle->width() + 2;
    }
    int width = ui->RemarkLab->width() - widthTitle;
    lab->setText(strItemName);
    lab->setToolTip(strItemName);
    QFontMetrics fm(lab->font());
    if (width > 0 && (fm.width(strItemName) >= width))
    {
        // 判断是否长度过长需要显示省略号
        QString newText = fm.elidedText(strItemName, Qt::ElideRight, width);
        lab->setText(newText);
    }
}

void QHistoryItemResultDetailDlg::CutTipstring(const QString& src, QString& dst, int width)
{
    if (width <= 0)
    {
        dst = src;
        return;
    }

    QFont font("Source Han Sans CN Regular");
    font.setPixelSize(16);
    QFontMetrics fm(font);
    int currentWidth = 0;
    int row = 0;
    for (auto& data : src)
    {
        int charWidth = fm.width(data);
        if ((currentWidth + charWidth) > width)
        {
            row++;
            if (row > 1)
            {
                dst += "...";
                break;
            }
            dst += "\n";
            currentWidth = 0;
        }

        currentWidth += charWidth;
        dst += data;
    }

    if (dst.endsWith("\n"))
    {
        dst.chop(1);
    }
}

///
/// @brief 初始化病人信息
///
/// @param[in]  patient  
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年1月4日，新建函数
///
void QHistoryItemResultDetailDlg::InitPatientInfo(const ::tf::PatientInfo& patientInfo)
{   
	// 姓名
    SetItemName(ui->NameValLab, ui->NameLab, QString::fromStdString(patientInfo.name));
	// 性别不合法时为未赋值
	if (patientInfo.gender == -1)
	{
		ui->GenderValLab->clear();
	}
	else
	{
		ui->GenderValLab->setText(ThriftEnumTrans::GetGender(patientInfo.gender, tr("未知")));
	}
	// 年龄单位不合法时为未赋值
	if (patientInfo.ageUnit == -1)
	{
		ui->AgeValLab->clear();
	}
	else
	{
		ui->AgeValLab->setText(QString::number(patientInfo.age) + ThriftEnumTrans::GetAgeUnit(patientInfo.ageUnit, tr("岁")));
	}
	// 病历号
    SetItemName(ui->CaseNoValLab, ui->CaseNoLab, QString::fromStdString(patientInfo.medicalRecordNo));
    // 送检科室
    SetItemName(ui->DepartmentValLab, ui->DepartmentLab, QString::fromStdString(patientInfo.department));
    // 主治医师
    SetItemName(ui->VisitingDoctorValLab, ui->VisitingDoctorLab, QString::fromStdString(patientInfo.attendingDoctor));
    // 临床诊断
    QString showData;
    CutTipstring(QString::fromStdString(patientInfo.diagnosis), showData, ui->DiagNosisValLab->width() - 40);
    ui->DiagNosisValLab->setText(showData);
    ui->DiagNosisValLab->setToolTip(QString::fromStdString(patientInfo.diagnosis));
    // 备注
    SetItemName(ui->RemarkValLab, nullptr, QString::fromStdString(patientInfo.comment));

    m_patientInfo = patientInfo;
}

///
/// @brief 显示免疫项目结果
///
/// @param[in]  result  免疫项目结果
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年2月3日，新建函数
///
void QHistoryItemResultDetailDlg::ShowIMResult(const std::vector<im::tf::AssayTestResult>& result, const std::shared_ptr<HistoryBaseDataItem>& historyItem, QTabWidget* pImTabWgt, int64_t retestId)
{
    ULOG(LOG_INFO, __FUNCTION__);

	RFlagTabWidget* pRFlagImTabWgt = dynamic_cast<RFlagTabWidget*>(pImTabWgt);
	if (pRFlagImTabWgt == nullptr)
    {
        ULOG(LOG_WARN, "Null imtab widget.");
		return;
	}
	pRFlagImTabWgt->Clear();

	// 项目测试结果查询时已按结束时间升序排序，第一个为检测结果，其余为复查结果
    int64_t patientID = historyItem->m_patientId;
	int64_t sampleInfoId = -1;
	std::string	sampleNo;	// 样本号
	std::string barcode;// 样本条码

	std::vector<int> vecFlag;
	if (retestId == -1 && result.size() > 1)
	{
		vecFlag.push_back(result.size() - 1);
	}

	for (int index = 0; index < result.size(); ++index)
	{		
		sampleInfoId = sampleInfoId == -1 ? result.at(index).sampleInfoId : sampleInfoId;
		sampleNo = sampleNo.empty() ? result.at(index).seqNo : sampleNo;
		barcode = barcode.empty() ? result.at(index).barcode : barcode;

		auto wgt = new ImAssayResultInfoWidget();
		QString title;
		if (index == 0)
		{
			title = tr("检测结果");
			wgt->setProperty(PROPERTY_RETEST_ID, -1);
		}
		else
		{
			index>0? title = tr("复查结果") + QString::number(index): title = tr("复查结果");
			wgt->setProperty(PROPERTY_RETEST_ID, result.at(index).id);
			wgt->setProperty(PROPERTY_RETEST_VOLVE, TranseConc(result.at(index).conc));
			wgt->setProperty(PROPERTY_RETEST_STATUS, QString::fromStdString(result.at(index).resultStatusCodes));

			// 判断是否应当设置标记
			if (retestId == result.at(index).id && retestId != -1)
			{
				vecFlag.push_back(index);
			}
		}
		bool bIsFirst = index == 0 ? true : false;
		bool bIsLastReTest = (index == result.size() - 1 && index > 0) ? true : false;
		// 病人样本可编辑，校准、质控不可编辑
		bool bVolEditEnable = result.at(index).sampType == ::tf::SampleType::SAMPLE_TYPE_PATIENT ? !historyItem->m_bCheck : false;
		wgt->UpdateDetailData(result.at(index), bVolEditEnable, bIsFirst, bIsLastReTest);
		pRFlagImTabWgt->AddTab(wgt, title);
	}

	// 样本条码
	ui->BarcodeValLab->setText(QString::fromStdString(barcode));
    ui->BarcodeValLab->setToolTip(QString::fromStdString(barcode));
	// 样本号
	ui->SampNoValLab->setText(QString::fromStdString(sampleNo));

	pRFlagImTabWgt->SetFlag(vecFlag);

	// 初始化病人信息
	//if (!bInitPatient)
	{
		if (patientID >= 0)
		{
			::tf::PatientInfoQueryResp _return;
			::tf::PatientInfoQueryCond piqc;
			piqc.__set_id(patientID);
			DcsControlProxy::GetInstance()->QueryPatientInfo(_return, piqc);
			if (_return.result == ::tf::ThriftResult::type::THRIFT_RESULT_SUCCESS && _return.lstPatientInfos.size() > 0)
			{
				const auto& patientInfo = _return.lstPatientInfos.at(0);
				// 设置界面显示病人信息
				InitPatientInfo(patientInfo);
			}
		}
		// 历史数据中的病人ID无效，再找样本表中的病人ID
		else
		{
			// 查询样本信息以获取病人信息数据库主键
			::tf::SampleInfoQueryCond sampQryCond;
			::tf::SampleInfoQueryResp sampQryResp;
			sampQryCond.__set_id(sampleInfoId);
			if (!DcsControlProxy::GetInstance()->QuerySampleInfo(sampQryCond, sampQryResp) || sampQryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
			{
				ULOG(LOG_ERROR, "query sample Info failed!");
				return;
			}
			if (sampQryResp.lstSampleInfos.size() <= 0)
            {
                ULOG(LOG_WARN, "Empty sampleinfo query result!");
				return;
			}

			::tf::SampleInfo sampleInfo = sampQryResp.lstSampleInfos[0];
			// 样本信息中病人ID仍无效，则不再查询病人信息
			if (sampleInfo.patientInfoId < 0)
			{
				ULOG(LOG_ERROR, "patient id invalid!");
				return;
			}

			// 查询病人信息
			::tf::PatientInfoQueryCond patienQryCond;
			::tf::PatientInfoQueryResp patienQryResp;
			patienQryCond.__set_id(sampleInfo.patientInfoId);
			if (!DcsControlProxy::GetInstance()->QueryPatientInfo(patienQryResp, patienQryCond) || patienQryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
			{
				ULOG(LOG_ERROR, "query patient Info failed!");
				return;
			}

			if (patienQryResp.lstPatientInfos.empty())
			{
				ULOG(LOG_INFO, "no patient Info data");
				return;
			}

			tf::PatientInfo patientInfo = patienQryResp.lstPatientInfos[0];
			// 设置界面显示病人信息
			InitPatientInfo(patientInfo);
		}
	}
}

///
/// @brief 显示生化项目结果
///
/// @param[in]  result  生化项目结果
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年2月3日，新建函数
///
void QHistoryItemResultDetailDlg::ShowCHResult(const std::vector<ch::tf::AssayTestResult>& result, const std::shared_ptr<HistoryBaseDataItem>& historyItem, QTabWidget* pChTabWgt, int64_t retestId)
{
	RFlagTabWidget* pRFlagChTabWgt = dynamic_cast<RFlagTabWidget*>(pChTabWgt);
	if (pRFlagChTabWgt == nullptr)
	{
		return;
	}

	pRFlagChTabWgt->Clear();

	// 根据样本ID查询样本
	::tf::SampleInfoQueryCond stuSampQryCond;
	::tf::SampleInfoQueryResp stuSampQryResp;
	stuSampQryCond.__set_id(static_cast<qlonglong>(historyItem->m_sampleId));
	stuSampQryCond.__set_containTestItems(true);

	// 获取样本信息
	//tf::SampleInfo& stuSelSampleInfo = stuSampQryResp.lstSampleInfos[0];
	int64_t patientID = historyItem->m_patientId;
	std::string	sampleNo = historyItem->m_seqNo;	// 样本号
	std::string barcode = historyItem->m_barcode;// 样本条码

	std::vector<int> vecFlag;
	if (retestId == -1 && result.size() > 1)
	{
		vecFlag.push_back(result.size() - 1);
	}
	// 依次显示结果
	for (int index = 0; index < result.size(); ++index)
	{
		auto resultCurve = new QAssayResultView(this);

		const auto& assayResult = result[index];
		QString title;
		if (index == 0)
		{
			title = tr("检测结果");
			resultCurve->setProperty(PROPERTY_RETEST_ID, -1);
		}
		else
		{
			title = tr("复查结果") + QString::number(index);
			resultCurve->setProperty(PROPERTY_RETEST_ID, static_cast<qlonglong>(assayResult.id));
			resultCurve->setProperty(PROPERTY_RETEST_VOLVE, TranseConc(assayResult.conc));
			resultCurve->setProperty(PROPERTY_RETEST_STATUS, QString::fromStdString(assayResult.resultStatusCodes));

			// 判断是否应当设置标记
			if (retestId == assayResult.id)
			{
				vecFlag.push_back(index);
			}
		}

		// 消息传递
		connect(resultCurve, SIGNAL(dataChanged()), this, SIGNAL(dataChanged()));
		// 默认是生化结果
		tf::TestItem testItem;
		testItem.__set_assayCode(-1);

		tf::SampleInfo stuSelSampleInfo;
		::tf::SampleType::type type;
		switch ((::tf::HisSampleType::type)historyItem->m_sampleType)
		{
		case ::tf::HisSampleType::SAMPLE_SOURCE_NM:
		case ::tf::HisSampleType::SAMPLE_SOURCE_EM:
			type = ::tf::SampleType::SAMPLE_TYPE_PATIENT;
			break;
		case ::tf::HisSampleType::SAMPLE_SOURCE_QC:
			type = ::tf::SampleType::SAMPLE_TYPE_QC;
			break;
		case ::tf::HisSampleType::SAMPLE_SOURCE_CL:
			type = ::tf::SampleType::SAMPLE_TYPE_CALIBRATOR;
			break;
		default:
			break;
		}

		stuSelSampleInfo.__set_sampleType(type);
		stuSelSampleInfo.__set_seqNo(historyItem->m_seqNo);
		stuSelSampleInfo.__set_barcode(historyItem->m_barcode);
		// 历史数据historyItem中未查询样本注册时间，这里时间为无效时间 add by wzx-20240204
		stuSelSampleInfo.__set_registerTime("1970-01-01");
		resultCurve->SetSampleInfo(stuSelSampleInfo, false);
		resultCurve->SetAssayResult(assayResult.id, testItem);
		pRFlagChTabWgt->AddTab(resultCurve, title);
	}

	// 样本条码
	ui->BarcodeValLab->setText(QString::fromStdString(barcode));
	// 样本号
	ui->SampNoValLab->setText(QString::fromStdString(sampleNo));

	pRFlagChTabWgt->SetFlag(vecFlag);

	::tf::PatientInfoQueryResp _return;
	::tf::PatientInfoQueryCond piqc;
	piqc.__set_id(patientID);
	DcsControlProxy::GetInstance()->QueryPatientInfo(_return, piqc);
	if (_return.result == ::tf::ThriftResult::type::THRIFT_RESULT_SUCCESS && _return.lstPatientInfos.size() > 0)
	{
		const auto& patientInfo = _return.lstPatientInfos.at(0);
		// 设置界面显示病人信息
		InitPatientInfo(patientInfo);
	}
}

///
/// @brief 显示ISE项目结果
///
/// @param[in]  result  ISE项目结果
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年2月3日，新建函数
///
void QHistoryItemResultDetailDlg::ShowISEResult(const std::vector<ise::tf::AssayTestResult>& result, const std::shared_ptr<HistoryBaseDataItem>& historyItem, QTabWidget* pIseTabWgt, int64_t retestId)
{
	RFlagTabWidget* pRFlagIseTabWgt = dynamic_cast<RFlagTabWidget*>(pIseTabWgt);
	if (pRFlagIseTabWgt == nullptr)
	{
		return;
	}

	pRFlagIseTabWgt->Clear();
	// 根据样本ID查询样本
	::tf::SampleInfoQueryCond stuSampQryCond;
	::tf::SampleInfoQueryResp stuSampQryResp;
	stuSampQryCond.__set_id(historyItem->m_sampleId);
	stuSampQryCond.__set_containTestItems(true);

	// 执行查询条件
	//if (!DcsControlProxy::GetInstance()->QuerySampleInfo(stuSampQryCond, stuSampQryResp)
	//	|| stuSampQryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
	//	|| stuSampQryResp.lstSampleInfos.empty())
	//{
	//	ULOG(LOG_ERROR, "%s(), QuerySampleInfo failed.", __FUNCTION__);
	//	return;
	//}

	// 获取样本信息
	//tf::SampleInfo& stuSelSampleInfo = stuSampQryResp.lstSampleInfos[0];
	int64_t patientID = historyItem->m_patientId;
	std::string	sampleNo = historyItem->m_seqNo;	// 样本号
	std::string barcode = historyItem->m_barcode;// 样本条码

	std::vector<int> vecFlag;
	if (retestId == -1 && result.size() > 1)
	{
		vecFlag.push_back(result.size() - 1);
	}
	// 依次显示结果
	for (int index = 0; index < result.size(); ++index)
	{
		auto resultCurve = new QAssayResultView(this);

		const auto& assayResult = result[index];

		QString title;
		if (index == 0)
		{
			title = tr("检测结果");
			resultCurve->setProperty(PROPERTY_RETEST_ID, -1);
		}
		else
		{
			title = tr("复查结果") + QString::number(index);
			resultCurve->setProperty(PROPERTY_RETEST_ID, assayResult.id);
			resultCurve->setProperty(PROPERTY_RETEST_VOLVE, TranseConc(assayResult.conc));
			resultCurve->setProperty(PROPERTY_RETEST_STATUS, QString::fromStdString(assayResult.resultStatusCodes));

			// 判断是否应当设置标记
			if (retestId == assayResult.id)
			{
				vecFlag.push_back(index);
			}
		}
		// 消息传递
		connect(resultCurve, SIGNAL(dataChanged()), this, SIGNAL(dataChanged()));

		tf::TestItem testItem;
		testItem.__set_assayCode(-1);

		tf::SampleInfo stuSelSampleInfo;
		::tf::SampleType::type type;
		switch ((::tf::HisSampleType::type)historyItem->m_sampleType)
		{
			case ::tf::HisSampleType::SAMPLE_SOURCE_NM:
			case ::tf::HisSampleType::SAMPLE_SOURCE_EM:
				type = ::tf::SampleType::SAMPLE_TYPE_PATIENT;
				break;
			case ::tf::HisSampleType::SAMPLE_SOURCE_QC:
				type = ::tf::SampleType::SAMPLE_TYPE_QC;
				break;
			case ::tf::HisSampleType::SAMPLE_SOURCE_CL:
				type = ::tf::SampleType::SAMPLE_TYPE_CALIBRATOR;
				break;
			default:
				break;
		}

		stuSelSampleInfo.__set_sampleType(type);
		stuSelSampleInfo.__set_seqNo(historyItem->m_seqNo);
		stuSelSampleInfo.__set_barcode(historyItem->m_barcode);
		resultCurve->SetSampleInfo(stuSelSampleInfo, false);
		resultCurve->SetIseAssayResult(assayResult);
		pRFlagIseTabWgt->AddTab(resultCurve, title);
	}
	// 样本条码
	ui->BarcodeValLab->setText(QString::fromStdString(barcode));
	// 样本号
	ui->SampNoValLab->setText(QString::fromStdString(sampleNo));

	pRFlagIseTabWgt->SetFlag(vecFlag);

	::tf::PatientInfoQueryResp _return;
	::tf::PatientInfoQueryCond piqc;
	piqc.__set_id(patientID);
	DcsControlProxy::GetInstance()->QueryPatientInfo(_return, piqc);
	if (_return.result == ::tf::ThriftResult::type::THRIFT_RESULT_SUCCESS && _return.lstPatientInfos.size() > 0)
	{
		const auto& patientInfo = _return.lstPatientInfos.at(0);
		// 设置界面显示病人信息
		InitPatientInfo(patientInfo);
	}
}

///
/// @brief 根据是否是病人样本类型初始化结果表
///
/// @param[in]  bIsPatient  
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2024年2月1日，新建函数
///
void QHistoryItemResultDetailDlg::InitAssayResTable(bool bIsPatient)
{
	QStringList strHeaderList;
	strHeaderList << tr("项目名称") << tr("检测结果");
	if (bIsPatient)
	{
		strHeaderList << tr("复查结果");
	}
	
	// 初始化项目表
	ui->AssayResTbl->setColumnCount(strHeaderList.size());
	ui->AssayResTbl->verticalHeader()->setVisible(false);
	ui->AssayResTbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui->AssayResTbl->setItemDelegateForColumn(1, new CReadOnlyDelegate(this));
	if (strHeaderList.size() == 3)
	{
		ui->AssayResTbl->setItemDelegateForColumn(2, new CReadOnlyDelegate(this));
	}

	// 设置表头
	ui->AssayResTbl->setHorizontalHeaderLabels(strHeaderList);

	// 设置表格选中模式为行选中，不可多选，不可编辑
	ui->AssayResTbl->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->AssayResTbl->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->AssayResTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
}


///
/// @brief
///     重写显示事件
///
/// @param[in]  event  
///
/// @return 
///
/// @par History:
/// @li 1226/zhangjing，2024年9月2日，新建函数
///
void QHistoryItemResultDetailDlg::showEvent(QShowEvent* event)
{
    // 基类处理
    BaseDlg::showEvent(event);

    // 如果是第一次显示则初始化
    if (!m_bInit)
    {
        InitPatientInfo(m_patientInfo);
        m_bInit = true;
    }
}

///
/// @brief 构造函数
///
/// @param[in]  parent  
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年7月26日，新建函数
///
TabPage::TabPage(QWidget* parent /*= nullptr*/) :QWidget(parent), m_iTotalPage(1), m_iCurPage(1)
{
	auto hLayout = new QHBoxLayout();
	hLayout->setContentsMargins(0, 0, 0, 0);
	m_preButton = new QPushButton;
	m_nextButton = new QPushButton;

	m_preButton->setFixedSize(40, 40);
	m_nextButton->setFixedSize(40, 40);

	auto pageLayout = new QHBoxLayout();
	pageLayout->setContentsMargins(0, 0, 0, 0);
	m_curPageLabel = new QLabel(QString::number(m_iCurPage));
	m_totalPageLabel = new QLabel(QString::number(m_iTotalPage));
	pageLayout->addWidget(m_curPageLabel);
	pageLayout->addWidget(new QLabel("/"));
	pageLayout->addWidget(m_totalPageLabel);

	hLayout->addWidget(m_preButton);
	hLayout->addLayout(pageLayout);
	hLayout->addWidget(m_nextButton);

	this->setLayout(hLayout);

	connect(m_preButton, &QPushButton::clicked, this, &TabPage::SignalPreButtonClicked);
	connect(m_nextButton, &QPushButton::clicked, this, &TabPage::SignalNextButtonClicked);
}

///
/// @brief 设置总页数
///
/// @param[in]  iTotalPage  总页数
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年7月26日，新建函数
///
void TabPage::SetTotalPage(int iTotalPage)
{
	if (iTotalPage <= 0)
	{
		return;
	}

	m_iTotalPage = iTotalPage;
	m_totalPageLabel->setText(QString::number(m_iTotalPage));
	// 只有一页时隐藏
	if (m_iTotalPage == 1 && m_iCurPage == 1)
	{
		this->hide();
	}
}

///
/// @brief 设置当前页数
///
/// @param[in]  iCurPage  当前页数
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年7月26日，新建函数
///
void TabPage::SetCurPage(int iCurPage)
{
	if (iCurPage <= 0)
	{
		return;
	}

	m_iCurPage = iCurPage;
	m_curPageLabel->setText(QString::number(iCurPage));
	// 只有一页时隐藏
	if (m_iTotalPage == 1 && m_iCurPage == 1)
	{
		this->hide();
	}
}
