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
/// @file     BackupOrderWidget.cpp
/// @brief    应用--检测--备用订单
///
/// @author   7951/LuoXin
/// @date     2022年9月20日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年9月20日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "BackupOrderWidget.h"
#include "ui_BackupOrderWidget.h"
#include "src/common/Mlog/mlog.h"
#include "shared/uicommon.h"
#include "shared/tipdlg.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/CommonInformationManager.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"
#include "BackupOrderAddAssayDlg.h"
#include "UtilityCommonFunctions.h"
#include "thrift/DcsControlProxy.h"

// 当发生错误时，弹出消息框后退出当前函数
#define ReportErr(bErr, msg) \
if ((bErr))\
{\
	TipDlg(msg).exec();\
	return;\
}

#define		DOUBLE_LAB_DISTANCE_V			(3)						// 竖向两个标签之间的间距
#define		DOUBLE_LAB_DISTANCE_H			(4)						// 横向两个标签之间的间距
#define		LAB_DISTANCE					(15)					// 标签文字的左右边距
#define		LAB_RIGHT_DISTANCE				(-16)					// 标签到右边按钮的距离
#define		LAB_HIGHT						(44)					// 标签高度
#define		LAB_WIDTH_MIN					(86)					// 标签最小宽度
#define		BTN_HIGHT						(16)					// 关闭按钮高度
#define		SCROLL_BAR_WIDTH				(13)					// 滚动条宽度
#define     SCROLL_AREA_BORDER              (2)                     // 滚动区域内控间的边距
#define     SCROLL_AREA_BOTTOM_BORDER       (7)                     // 滚动区域底部的边距
#define     SAMPLE_WIDGET_NAME              ("sample_widget_")      // 样本源窗口名称前缀

BackupOrderWidget::BackupOrderWidget(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::BackupOrderWidget)
	, m_BackupOrderAddAssayDlg(nullptr)
	, m_currentSampleSourceType(-1)
	, m_bInit(false)
{
	ui->setupUi(this);

	// 仅免疫不显示血清指数
	auto type = CommonInformationManager::GetInstance()->GetSoftWareType();
	if (!(type == CHEMISTRY || type == CHEMISTRY_AND_IMMUNE))
	{
		ui->sind_ckbox->hide();
		ui->label_2->hide();
        ui->label_9->hide();
        ui->scrollArea_5->hide();
	}

    ui->label_2->setAttribute(Qt::WA_TransparentForMouseEvents);

    // 设置样本源窗口中控件的名字
    ui->scrollArea_0->setObjectName("scrollArea_" + QString::number(::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_OTHER));
    ui->scrollArea_1->setObjectName("scrollArea_" + QString::number(::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_XQXJ));
    ui->scrollArea_2->setObjectName("scrollArea_" + QString::number(::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NY));
    ui->scrollArea_3->setObjectName("scrollArea_" + QString::number(::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_QX));
    ui->scrollArea_4->setObjectName("scrollArea_" + QString::number(::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NJY));
    ui->scrollArea_5->setObjectName("scrollArea_" + QString::number(::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_JMQJY));

    ui->scrollAreaWidgetContents_0->setObjectName("scrollAreaWidgetContents_" + QString::number(::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_OTHER));
    ui->scrollAreaWidgetContents_1->setObjectName("scrollAreaWidgetContents_" + QString::number(::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_XQXJ));
    ui->scrollAreaWidgetContents_2->setObjectName("scrollAreaWidgetContents_" + QString::number(::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NY));
    ui->scrollAreaWidgetContents_3->setObjectName("scrollAreaWidgetContents_" + QString::number(::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_QX));
    ui->scrollAreaWidgetContents_4->setObjectName("scrollAreaWidgetContents_" + QString::number(::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NJY));
    ui->scrollAreaWidgetContents_5->setObjectName("scrollAreaWidgetContents_" + QString::number(::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_JMQJY));

    ui->sample_widget_0->setObjectName(SAMPLE_WIDGET_NAME + QString::number(::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_OTHER));
    ui->sample_widget_1->setObjectName(SAMPLE_WIDGET_NAME + QString::number(::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_XQXJ));
    ui->sample_widget_2->setObjectName(SAMPLE_WIDGET_NAME + QString::number(::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NY));
    ui->sample_widget_3->setObjectName(SAMPLE_WIDGET_NAME + QString::number(::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_QX));
    ui->sample_widget_4->setObjectName(SAMPLE_WIDGET_NAME + QString::number(::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NJY));
    ui->sample_widget_5->setObjectName(SAMPLE_WIDGET_NAME + QString::number(::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_JMQJY));

    ui->add_btn_0->setObjectName("add_btn_" + QString::number(::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_OTHER));
    ui->add_btn_1->setObjectName("add_btn_" + QString::number(::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_XQXJ));
    ui->add_btn_2->setObjectName("add_btn_" + QString::number(::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NY));
    ui->add_btn_3->setObjectName("add_btn_" + QString::number(::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_QX));
    ui->add_btn_4->setObjectName("add_btn_" + QString::number(::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NJY));
    ui->add_btn_5->setObjectName("add_btn_" + QString::number(::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_JMQJY));

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, UpdateCtrlsEnabled);
}

BackupOrderWidget::~BackupOrderWidget()
{

}

void BackupOrderWidget::LoadDataToCtrls()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	BackupOrderConfig boc;
    if (!DictionaryQueryManager::TakeBackupOrderConfig(boc))
    {
        ULOG(LOG_WARN, "Failed to get standby order!");
        return;
    }

	// 删除所有的标签
	for (auto lab : findChildren<CustomLab*>())
	{
		delete lab;
	}

    // 重新设置窗口大小
    for (auto& srl : findChildren<QScrollArea*>(QRegularExpression("scrollArea_*")))
    {
        for (auto& wid : srl->findChildren<QWidget*>(QRegularExpression("sample_widget_*")))
        {
            wid->setFixedHeight(srl->height() - SCROLL_AREA_BORDER);
        }
    }

	ui->sind_ckbox->setChecked(boc.autoTestSind);
    ui->get_order_fail_ckbox->setChecked(boc.getOrderFailUseBackupOrder);
    ui->scan_fail_ckbox->setChecked(boc.scanFailUseBackupOrder);
    ui->order_null_ckbox->setChecked(boc.orderNullUseBackupOrder);

    for (auto& btn : QWidget::findChildren<QPushButton*>(QRegularExpression("add_btn_*")))
    {
        btn->move(DOUBLE_LAB_DISTANCE_H, DOUBLE_LAB_DISTANCE_V);
    }

    for (auto each : ::tf::_SampleSourceType_VALUES_TO_NAMES)
    {
        std::vector<int32_t> codes(boc.sampleSrcType2TestItems[each.first].begin(), boc.sampleSrcType2TestItems[each.first].end());
        ShowConfigAssay(codes, each.first);
    }

    m_sampleSrcType2Profiles = boc.sampleSrcType2Profiles;
}

void BackupOrderWidget::showEvent(QShowEvent *event)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 基类先处理
	QWidget::showEvent(event);

    if (!m_bInit)
    {
        InitCtrls();
        m_bInit = true;
    }

	LoadDataToCtrls();
}

void BackupOrderWidget::InitCtrls()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	connect(ui->flat_save, SIGNAL(clicked()), this, SLOT(OnSaveBtnClicked()));

	// 项目添加按钮
    if (m_BackupOrderAddAssayDlg ==nullptr)
    {
        m_BackupOrderAddAssayDlg = new BackupOrderAddAssayDlg(this);
        connect(m_BackupOrderAddAssayDlg->GetSaveBtnPtr(), SIGNAL(clicked()), this, SLOT(OnSaveAddAssay()));
    }

    for (auto btn : QWidget::findChildren<QPushButton*>(QRegularExpression("add_btn_*")))
    {
        connect(btn, &QPushButton::clicked, this, &BackupOrderWidget::OnAddAssayBtnClicked);
        btn->move(DOUBLE_LAB_DISTANCE_H, DOUBLE_LAB_DISTANCE_V);
    }

    // 响应项目变更消息
    REGISTER_HANDLER(MSG_ID_ASSAY_CODE_MANAGER_UPDATE, this, OnProcAssayInfoUpdate);
}

void BackupOrderWidget::AddLabToWidget(const QString& text, int code,QWidget* widget)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 一个项目只能添加一次
    if (widget->findChild<CustomLab*>(text) != nullptr)
    {
        return;
    }

	CustomLab* lab = new CustomLab(code, text, widget);
	lab->show();

	// 获取添加标签
	QString btnName = widget->objectName().replace(SAMPLE_WIDGET_NAME, "add_btn_");
	auto addBtn = widget->findChild<QPushButton*>(btnName);
    auto scrollArea = QWidget::findChild<QScrollArea*>(btnName.replace("add_btn_", "scrollArea_"));

	UpDateAllLabPos(widget);

	// 标签的删除按钮
	connect(lab, &CustomLab::checked, this, &BackupOrderWidget::OnDeleteAssay);
}

void BackupOrderWidget::UpDateAllLabPos(QWidget* widget)
{
	// 重置添加按钮的位置
	auto addBtn = widget->findChild<QPushButton*>(widget->objectName().replace(SAMPLE_WIDGET_NAME, "add_btn_"));
	addBtn->move(DOUBLE_LAB_DISTANCE_H, DOUBLE_LAB_DISTANCE_V);
	int x = DOUBLE_LAB_DISTANCE_H;
	int y = DOUBLE_LAB_DISTANCE_V;

	// 滚动窗口
    auto scrollArea = QWidget::findChild<QScrollArea*>(widget->objectName().replace(SAMPLE_WIDGET_NAME, "scrollArea_"));
	int widgetCurrentWidth = scrollArea->width() - SCROLL_AREA_BORDER - SCROLL_BAR_WIDTH;

	for (auto lab : widget->findChildren<CustomLab*>())
	{
		// 判断是否需要换行
		if (x + lab->width() + DOUBLE_LAB_DISTANCE_H > widgetCurrentWidth)
		{
			x = DOUBLE_LAB_DISTANCE_H;
			y += lab->height() + DOUBLE_LAB_DISTANCE_V;
		}

		// 设置下一个标签的位置
		lab->move(x, y);
		x += lab->width() + DOUBLE_LAB_DISTANCE_H;
	}

		// 判断是否需要换行
		if (x + addBtn->width() > widgetCurrentWidth)
		{
			x = DOUBLE_LAB_DISTANCE_H;
			y += addBtn->height() + DOUBLE_LAB_DISTANCE_V;
		}
		addBtn->move(x, y);

        int ht = addBtn->y() + addBtn->height() + SCROLL_AREA_BOTTOM_BORDER;
        widget->setFixedHeight(ht > scrollArea->height() ? ht  : scrollArea->height() - SCROLL_AREA_BORDER);
}

void BackupOrderWidget::ShowConfigAssay(std::vector<int32_t>& assayCodes, int sampleSource)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 获取通用项目信息
	auto& mapAssayInfo = CommonInformationManager::GetInstance()->GetGeneralAssayCodeMap();
	if (mapAssayInfo.empty())
	{
		ULOG(LOG_INFO, "GetChAssayMaps Failed !");
	}

	// 获取需要添加按钮的位置
	auto wg = QWidget::findChild<QWidget*>(SAMPLE_WIDGET_NAME + QString::number(sampleSource));

    if (wg != nullptr)
    {
        // 找出旧项目列表中不存在新项目列表中的项目，删除对应标签
        for (auto lab : wg->findChildren<CustomLab*>())
        {           
            if (std::find(assayCodes.begin(), assayCodes.end(), lab->GetAssayCode()) == assayCodes.end())
            {
                DeleteLab(lab);
            }
        }

        // 添加标签
        for (auto code : assayCodes)
        {
            // 没找到项目信息，可能已经被删除
            auto iter = mapAssayInfo.find(code);
            if (iter == mapAssayInfo.end())
            {
                continue;
            }

            QString text = QString::fromStdString(iter->second->assayName);
            AddLabToWidget(text, code, wg);
        }
    }
}

void BackupOrderWidget::OnSaveBtnClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

    bool hasChecked = false;
    for (auto& box : ui->backup_widget->findChildren<QCheckBox*>())
    {
        if (box->isChecked())
        {
            hasChecked = true;
            break;
        }
    }

	BackupOrderConfig boc;
    boc.autoTestSind = ui->sind_ckbox->isChecked();
    boc.getOrderFailUseBackupOrder = ui->get_order_fail_ckbox->isChecked();
    boc.scanFailUseBackupOrder = ui->scan_fail_ckbox->isChecked();
    boc.orderNullUseBackupOrder = ui->order_null_ckbox->isChecked();
    boc.sampleSrcType2Profiles = m_sampleSrcType2Profiles;

    bool hasBackupCode = false;
	for (auto each : ::tf::_SampleSourceType_VALUES_TO_NAMES)
	{
		QString widgetName = SAMPLE_WIDGET_NAME;
		widgetName += QString::number(each.first);
		
        std::pair<int, std::set<int>> type2testItems;
        type2testItems.first = each.first;
		auto wg = QWidget::findChild<QWidget*>(widgetName);

		for (auto lab : wg->findChildren<CustomLab*>())
		{
            type2testItems.second.insert(lab->GetAssayCode());		
		}

        boc.sampleSrcType2TestItems.insert(type2testItems);

        if (!type2testItems.second.empty())
        {
            hasBackupCode = true;
        }
	}

    // 启用备用订单但是为设置项目
    ReportErr(hasChecked && !hasBackupCode, tr("未设置备用订单项目，不能使用备用订单"));
    
    if (!UiCommon::SaveBackupOrderConfig(boc))
    {
        ReportErr(true, tr("保存失败"));
    }

	POST_MESSAGE(MSG_ID_DICTIONARY_UPDATE);
}

void BackupOrderWidget::OnSaveAddAssay()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

     std::vector<int32_t> singleCodes;
     std::vector<int64_t> profiles;
    m_currentSampleSourceType;
	m_BackupOrderAddAssayDlg->GetUserConfigData(singleCodes, profiles);

    m_sampleSrcType2Profiles[m_currentSampleSourceType] = std::set<int64_t>(profiles.begin(), profiles.end());

	ShowConfigAssay(singleCodes, m_currentSampleSourceType);

	m_currentSampleSourceType = -1;
}

void BackupOrderWidget::OnDeleteAssay()
{
    auto delLab = dynamic_cast<CustomLab*>(sender());
    int code = delLab->GetAssayCode();

    QWidget* wg = dynamic_cast<QWidget*>(delLab->parent());
    auto tempList = wg->findChild<QPushButton*>()->objectName().split("_");

    auto& profiles = m_sampleSrcType2Profiles[tempList[tempList.size() - 1].toInt()];
    for (auto& iter = profiles.begin(); iter != profiles.end();)
    {
        ::tf::ProfileInfoQueryResp _return;
        ::tf::ProfileInfoQueryCond piqc;
        piqc.__set_id((*iter) -UI_PROFILESTART);
        if (!DcsControlProxy::GetInstance()->QueryProfileInfo(_return, piqc)
            || _return.lstProfileInfos.empty())
        {
            iter++;
            continue;
        }

        auto& subCodes = _return.lstProfileInfos[0].subItems;
        if (std::find(subCodes.begin(), subCodes.end(), code) != subCodes.end())
        {
            iter = profiles.erase(iter);
        }
        else
        {
            iter++;
        }       
    }

    DeleteLab(delLab);
}

///
/// @bref
///		响应项目信息发生变更
///
/// @par History:
/// @li 8276/huchunli, 2023年7月25日，新建函数
///
void BackupOrderWidget::OnProcAssayInfoUpdate()
{
    LoadDataToCtrls();
}

void BackupOrderWidget::OnAddAssayBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    auto tempList = sender()->objectName().split("_");
    m_currentSampleSourceType = tempList[tempList.size() - 1].toInt();

    // 获取当前点击的新增按钮属于哪个窗口
    QString widgetName = SAMPLE_WIDGET_NAME + QString::number(m_currentSampleSourceType);
    auto wid = QWidget::findChild<QWidget*>(widgetName);

    //获取已经选中的项目编号
    std::vector<int> assayCodes;
    for (auto& lab : wid->findChildren<CustomLab*>())
    {
        assayCodes.push_back(lab->GetAssayCode());
    }

    // 组合项目
    std::vector<int64_t> profiles(m_sampleSrcType2Profiles[m_currentSampleSourceType].begin(), m_sampleSrcType2Profiles[m_currentSampleSourceType].end());

    m_BackupOrderAddAssayDlg->ShowAddAssayDlg(assayCodes, profiles, m_currentSampleSourceType);
}

void BackupOrderWidget::UpdateCtrlsEnabled()
{
    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    auto& uim = UserInfoManager::GetInstance();

    ui->sind_ckbox->setEnabled(uim->IsPermisson(PSM_TESTSET_BACKUPORDER_AUTOXQIDX));
    ui->backup_widget->setEnabled(uim->IsPermisson(PSM_TESTSET_BACKUPORDER_USE));
    ui->widget3->setEnabled(uim->IsPermisson(PSM_TESTSET_BACKUPORDER_USE));
    ui->flat_save->setEnabled(uim->IsPermisson(PSM_TESTSET_BACKUPORDER_USE));
}

void BackupOrderWidget::DeleteLab(CustomLab* delLab)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 获取添加按钮
	QWidget* wg = dynamic_cast<QWidget*>(delLab->parent());
	auto addBtn = wg->findChild<QPushButton*>();
    auto minHight = QWidget::findChild<QScrollArea*>(wg->objectName().replace(SAMPLE_WIDGET_NAME, "scrollArea_"))->height() - SCROLL_AREA_BORDER;

	if (addBtn->x() == DOUBLE_LAB_DISTANCE_H
		|| addBtn->x() - delLab->width() - DOUBLE_LAB_DISTANCE_H < DOUBLE_LAB_DISTANCE_H)
	{
		// 重新设置窗口大小
		int h = addBtn->y() < minHight ? minHight : addBtn->y();
		wg->setFixedHeight(h);
	}

	delete delLab;

	UpDateAllLabPos(wg);
}

CustomLab::CustomLab(int assayCode, QString assayName, QWidget *parent)
	: QLabel(parent)
	, m_assayCode(assayCode)
	, m_btn(new QPushButton(this))
	, m_label(new QLabel(this))
{
	Init(assayName);

	connect(m_btn, &QPushButton::clicked, this, [&]() {emit checked(); });
}

void CustomLab::setBtnSize(int size)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 设置底层标签
	setFixedWidth(m_label->width() + LAB_RIGHT_DISTANCE + size);

	// 设置按钮
	m_btn->setGeometry(this->width() - size, 0, size, size);
}

void CustomLab::Init(QString assayName)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

    setObjectName(assayName);

	// 设置顶层标签
	m_label->setAlignment(Qt::AlignCenter);
	m_label->setText(assayName);

	// 设置宽度
	m_label->adjustSize();

	int tempWid = m_label->width() + LAB_RIGHT_DISTANCE + BTN_HIGHT + LAB_DISTANCE*2;
	int wid = tempWid < LAB_WIDTH_MIN ? LAB_WIDTH_MIN : tempWid;

	m_label->setGeometry(0, 0, wid - LAB_RIGHT_DISTANCE- BTN_HIGHT,LAB_HIGHT);

	// 设置底层标签
	setFixedHeight(LAB_HIGHT);
	setFixedWidth(wid);

	// 设置按钮
	m_btn->setGeometry(this->width() - BTN_HIGHT, 0, BTN_HIGHT, BTN_HIGHT);
	m_btn->raise();
}
