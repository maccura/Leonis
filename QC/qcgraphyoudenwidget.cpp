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
/// @file     qcgraphyoudenwidget.cpp
/// @brief    质控youden图界面
///
/// @author   4170/TangChuXian
/// @date     2021年6月18日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月18日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "qcgraphyoudenwidget.h"
#include <memory>
#include <QStandardItemModel>
#include <QVariant>
#include <QRadioButton>
#include <QScrollBar>
#include <QTimer>
#include <QFileDialog>
#include <QButtonGroup>
#include <QBuffer>
#include "ui_qcgraphyoudenwidget.h"
#include "ui_qcoutctrldlg.h"
#include "ui_mcqcyoudengraphic.h"

#include "shared/uicommon.h"
#include "shared/messagebus.h"
#include "shared/tipdlg.h"
#include "shared/statusitemdelegate.h"
#include "shared/uidef.h"
#include "shared/msgiddef.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/mcpixmapitemdelegate.h"
#include "shared/datetimefmttool.h"
#include "shared/CommonInformationManager.h"
#include "shared/FileExporter.h"

#include "qcoutctrldlg.h"
#include "mcqcyoudengraphic.h"
#include "ch/chQcResultDetailsDlg.h"
#include "im/imqcitemrltdetaildlg.h"
#include "uidcsadapter/uidcsadapter.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/TimeUtil.h"
#include "manager/UserInfoManager.h"
#include "Serialize.h"
#include "printcom.hpp"

#define  DEFAULT_ROW_CNT_OF_QC_RESULT_TABLE              (20)          // 质控结果表默认行数
#define  DEFAULT_COL_CNT_OF_QC_RESULT_TABLE              (17)          // 质控结果表默认列数

#define  COL_INDEX_INDICATE_OF_QC_RESULT_TABLE           (0)           // 质控结果表指向列索引
#define  COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE          (1)           // 质控结果表计算列索引
#define  COL_INDEX_QC_TIME_OF_QC_RESULT_TABLE            (3)           // 质控结果表质控时间列索引
#define  COL_INDEX_STATE_OF_QC_RESULT_TABLE              (5)           // 质控结果表状态列索引
#define  COL_INDEX_QC_NUM_OF_QC_RESULT_TABLE             (7)           // 质控结果表质控品编号列索引
#define  COL_INDEX_QC_NAME_OF_QC_RESULT_TABLE            (8)           // 质控结果表质控品名称列索引
#define  COL_INDEX_QC_LOT_OF_QC_RESULT_TABLE             (12)          // 质控结果表质控品批号列索引
#define  COL_INDEX_DB_NO_OF_QC_RESULT_TABLE              (16)          // 质控结果表数据库主键列索引

#define  X_SD_CV_PRECISION                               (3)           // 靶值标准差精度

#define  DEFAULT_COL_CNT_OF_QC_TP_RLT_PRINT_TABLE        (3)           // 打印表默认列数

Q_DECLARE_METATYPE(TWIN_QC_RESULT_INFO)

QcGraphYoudenWidget::QcGraphYoudenWidget(QWidget *parent)
    : QWidget(parent),
      m_bInit(false),
      m_iCurPrintItemIndex(-1)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化UI对象
    ui = new Ui::QcGraphYoudenWidget();
    ui->setupUi(this);

    // 界面显示前初始化
    InitBeforeShow();

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

QcGraphYoudenWidget::~QcGraphYoudenWidget()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月14日，新建函数
///
void QcGraphYoudenWidget::InitBeforeShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 初始化界面子设备按钮组
	m_pSubDevReadioBtnGroup = new QButtonGroup(this);
	m_pSubDevReadioBtnGroup->setExclusive(true);
	ui->groupBox_subDevs->setStyleSheet("QGroupBox {border: none;}");
	connect(m_pSubDevReadioBtnGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), this, [=](QAbstractButton *button) {
		if (button == nullptr)
		{
			return;
		}
		// 转换为radiobutton
		QRadioButton* clickedButton = (QRadioButton*)button;
		if (clickedButton->isChecked() && m_strCurSubDevReadioName != clickedButton->text())
		{
			m_strCurSubDevReadioName = clickedButton->text();
			OnQcCondChanged();
		}
	});

    // 质控结果表
    m_pTblModel = new QStandardItemModel(this);
    //m_pTblModel->setRowCount(DEFAULT_ROW_CNT_OF_QC_RESULT_TABLE);
    m_pTblModel->setColumnCount(DEFAULT_COL_CNT_OF_QC_RESULT_TABLE);
    ui->QcResultTbl->setModel(m_pTblModel);
    ui->QcResultTbl->hideColumn(COL_INDEX_DB_NO_OF_QC_RESULT_TABLE);

    // 为特定列设置状态代理
    ui->QcResultTbl->setItemDelegateForColumn(COL_INDEX_STATE_OF_QC_RESULT_TABLE, new StatusItemDelegate(this));
    ui->QcResultTbl->setItemDelegateForColumn(COL_INDEX_QC_TIME_OF_QC_RESULT_TABLE, new CReadOnlyDelegate(this));
    //ui->QcResultTbl->setFocusPolicy(Qt::NoFocus);

    // 设置图标居中代理
    ui->QcResultTbl->setItemDelegateForColumn(COL_INDEX_INDICATE_OF_QC_RESULT_TABLE, new McPixmapItemDelegate(ui->QcResultTbl));
    ui->QcResultTbl->setItemDelegateForColumn(COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE, new McPixmapItemDelegate(ui->QcResultTbl));
}

///
/// @brief
///     显示之后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月14日，新建函数
///
void QcGraphYoudenWidget::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化字符串资源
    InitStrResource();

    // 初始化信号槽连接
    InitConnect();

    // 初始化子控件
    InitChildCtrl();
}

///
/// @brief
///     初始化字符串资源
///
/// @par History:
/// @li 4170/TangChuXian，2022年1月17日，新建函数
///
void QcGraphYoudenWidget::InitStrResource()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
//     ui->AssayLab->setText(LoadStrFromLanguage(CHAR_CODE::IDS_PRINT_TYPE_ASSAY));
//     ui->QcDateLab->setText(LoadStrFromLanguage(CHAR_CODE::IDS_QC_DATE));
//     ui->DetailBtn->setText(LoadStrFromLanguage(CHAR_CODE::IDS_TITLE_RESULT_DETAIL));
//     ui->PrintBtn->setText(LoadStrFromLanguage(CHAR_CODE::IDS_MAINT_PRINT));
//     ui->OutCtrlBtn->setText(LoadStrFromLanguage(CHAR_CODE::IDS_QC_OUT_CTRL_HANDLE));
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月14日，新建函数
///
void QcGraphYoudenWidget::InitChildCtrl()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 绑定起止日期编辑框
    BindQcDateEdit(ui->QcStartDateEdit, ui->QcEndDateEdit);

    // 初始化日期控件
    ui->QcStartDateEdit->setCalendarPopup(true);
    ui->QcEndDateEdit->setCalendarPopup(true);

    // 失控处理按钮禁能
    ui->OutCtrlBtn->setEnabled(false);
    ui->DetailBtn->setEnabled(false);
	ui->PrintBtn->setEnabled(false);
	ui->export_btn->setEnabled(false);

    // 质控结果表
    // 设置表头
    QStringList strQcResHeaderList;
    strQcResHeaderList << tr("") << tr("计算") << tr("序号") << tr("质控时间") << tr("结果") << tr("状态") << tr("失控规则") << tr("质控品编号") << tr("质控品名称") << tr("质控品简称") << tr("质控品类型") << tr("质控品水平") << tr("质控品批号") << tr("靶值") << tr("标准差") << tr("操作者");
    m_pTblModel->setHorizontalHeaderLabels(strQcResHeaderList);
    m_pTblModel->horizontalHeaderItem(COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE)->setData(UI_TEXT_COLOR_HEADER_MARK, Qt::TextColorRole);

    // 设置表格选中模式为行选中，不可多选，不可编辑
    ui->QcResultTbl->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->QcResultTbl->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->QcResultTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 隐藏垂直表头
    ui->QcResultTbl->verticalHeader()->setVisible(false);

    // 设置列宽适应内容
    ResizeTblColToContent(ui->QcResultTbl);
	ResizeQcResultTableIconColumn();

    // 更新设备名
    UpdateDevNameList();

    // 更新项目列表
    UpdateAssayNameList();

    // 更新质控结果
    UpdateQcResult();

    // 更新质控图
    UpdateQcGraphic();

    // 通知质控图刷新(用于调试)
//     REGISTER_HANDLER(MSG_ID_QC_GRAPH_UPDATE, this, UpdateAllQcInfo);
// 
//     // 监听质控结果更新
//     REGISTER_HANDLER(MSG_ID_QC_ADD_RESULT, this, OnQcResultUpdate);
// 
//     // 监听质控信息更新
//     REGISTER_HANDLER(MSG_ID_QC_ITEM_UPDATE, this, UpdateAllQcInfo);
// 
//     // 监听结果单位更新
//     REGISTER_HANDLER(MSG_ID_ASSAY_PARAM_UPDATE, this, UpdateAllQcInfo);
// 
//     // 监听联合质控项更新
//     REGISTER_HANDLER(MSG_ID_UPDATE_TWIN_PLOT_ITEM, this, UpdateQcTwinItem);
// 
//     // 项目信息更新
//     REGISTER_HANDLER(MSG_ID_UTILITY_ASSAY_UPDATE, this, UpdateQcTwinItem);
//     REGISTER_HANDLER(MSG_ID_UTILITY_ASSAY_DELETE, this, UpdateQcTwinItem);
}

///
/// @brief
///     初始化连接
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月14日，新建函数
///
void QcGraphYoudenWidget::InitConnect()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 质控查询条件改变
	connect(ui->AssayCombo, &QFilterComboBox::currentTextChanged, this, [this](const QString &text) {
		// 已经清空过，则不清空
		if (m_pTblModel->rowCount() <= 0 && ui->AssayCombo->findText(text) == -1)
		{
			return;
		}
		OnQcCondChanged();
	});
    connect(ui->QcStartDateEdit, SIGNAL(dateChanged(const QDate&)), this, SLOT(OnQcCondChanged()));
    connect(ui->QcEndDateEdit, SIGNAL(dateChanged(const QDate&)), this, SLOT(OnQcCondChanged()));

    // 质控结果表细节按钮被点击
    connect(ui->DetailBtn, SIGNAL(clicked()), this, SLOT(OnDetailBtnClicked()));

    // 失控处理按钮被点击
    connect(ui->OutCtrlBtn, SIGNAL(clicked()), this, SLOT(OnOutCtrlBtnClicked()));

    // 打印按钮被点击
    connect(ui->PrintBtn, SIGNAL(clicked()), this, SLOT(OnPrintBtnCliked()));

    // 导出按钮被点击
    connect(ui->export_btn, SIGNAL(clicked()), this, SLOT(OnExportBtnClicked()));

    // 质控结果表单元格被点击
    connect(ui->QcResultTbl, SIGNAL(clicked(const QModelIndex&)), this, SLOT(OnQcResultTblItemClicked(const QModelIndex&)));

    // 质控结果表选中项索引改变
    QItemSelectionModel* selItemModel = ui->QcResultTbl->selectionModel();
    if (selItemModel != Q_NULLPTR)
    {
        connect(selItemModel, SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(OnQcResultItemSelChanged(const QModelIndex&, const QModelIndex&)));
    }

    // 监听质控图选中点改变
    connect(ui->QcGraphic, SIGNAL(SigSelPointChanged(McQcYoudenGraphic::CURVE_ID, long long)), this, SLOT(OnQcGraphicSelPtChanged(McQcYoudenGraphic::CURVE_ID, long long)));

    // 显示子点复选框状态改变
    connect(ui->QcGraphic->ui->ShowNoCalcQcCB, SIGNAL(stateChanged(int)), this, SLOT(UpdateSetPtGraph()));

    // 监听质控品信息更新
    REGISTER_HANDLER(MSG_ID_QC_DOC_INFO_UPDATE, this, OnQcDocInfoInfoUpdate);

    // 项目信息更新
    REGISTER_HANDLER(MSG_ID_ASSAY_CODE_MANAGER_UPDATE, this, OnAssayInfoUpdate);

    // 角落按钮索引更新
    REGISTER_HANDLER(MSG_ID_QC_TAB_CORNER_INDEX_UPDATE, this, OnTabCornerIndexChanged);

    // 质控结果更新
    REGISTER_HANDLER(MSG_ID_QC_RESULT_UPDATE, this, OnQcRltUpdate);

    // 质控规则更新
    REGISTER_HANDLER(MSG_ID_QC_RULE_UPDATE, this, OnAssayInfoUpdate);
}

///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月14日，新建函数
///
void QcGraphYoudenWidget::showEvent(QShowEvent *event)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 让基类处理事件
    QWidget::showEvent(event);

    // 更新菜单位置
    POST_MESSAGE(MSG_ID_CURRENT_MODEL_NAME, QString("> ") + tr("质控") + QString(" > ") + tr("Twin Plot图"));

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }
    else
    {
        // 通知设备列表更新
        POST_MESSAGE(MSG_ID_QC_TAB_CORNER_DISPLAY_UPDATE, m_strDevNameList, m_strDevNameList.indexOf(m_strCurDevGroupName.isEmpty() ? m_strCurDevName : m_strCurDevGroupName));
    }
}

///
/// @brief
///     更新质控结果
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月9日，新建函数
///
void QcGraphYoudenWidget::UpdateQcResult()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 清空联合索引映射
    m_mapUnionIndex.clear();

    // 获取原本选中的数据库主键
    std::int64_t iSelDbNo = UI_INVALID_DB_NO;
    QModelIndex selIndex = ui->QcResultTbl->currentIndex();
    if (selIndex.isValid() && (m_pTblModel->item(selIndex.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE) != Q_NULLPTR))
    {
        iSelDbNo = m_pTblModel->item(selIndex.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE)->text().toLongLong();
    }

    // 清空表格信息
    m_pTblModel->removeRows(0, m_pTblModel->rowCount());
    //m_pTblModel->setRowCount(DEFAULT_ROW_CNT_OF_QC_RESULT_TABLE);

    // 禁用失控按钮
    ui->OutCtrlBtn->setEnabled(false);
    ui->DetailBtn->setEnabled(false);
	ui->PrintBtn->setEnabled(false);
	ui->export_btn->setEnabled(false);

    // 初始化质控信息和查询条件
    QList<TWIN_QC_RESULT_INFO>      qryResp;
    QC_RESULT_QUERY_COND            qryCond;

	// 组合名称和当前名称不一致则为组合设备
	QString devCombineName = m_strCurSubDevReadioName;
	int iModuleNo = 1;
	if (devCombineName != m_strCurDevName)
	{
		// ISE特殊处理
		if (gUiAdapterPtr()->GetDeviceClasssify(m_strCurDevName, m_strCurDevGroupName) == DEVICE_CLASSIFY_ISE)
		{
			// 索引名称
			QString modelIndexName = devCombineName.remove(m_strCurDevName);
			if (modelIndexName.size() == 1 &&
				modelIndexName.at(0).unicode() >= QChar('A').unicode() && modelIndexName.at(0).unicode() <= QChar('Z').unicode())
			{
				iModuleNo = modelIndexName.at(0).unicode() - QChar('A').unicode() + 1;
			}
		}
		else
		{
			// 组合设备，需要分解
			m_strCurDevName = devCombineName.remove(m_strCurDevGroupName);
		}
	}

	// 项目名称
	QString strCurAssayName = ui->AssayCombo->currentText();

    // 构造查询条件
    qryCond.startDate = ui->QcStartDateEdit->date();
    qryCond.endDate = ui->QcEndDateEdit->date();
    qryCond.strAssayName = strCurAssayName;

    // 加载项目下拉框
    QStringList strAssayNameList;
    if (!gUiAdapterPtr(m_strCurDevName, m_strCurDevGroupName, iModuleNo)->QueryQcYoudenRltInfo(qryCond, qryResp))
    {
        ULOG(LOG_WARN, "%s(), QueryQcYoudenRltInfo failed.", __FUNCTION__);
        return;
    }
    // 显示精度
    int pricisionNum = CommonInformationManager::GetInstance()->GetPrecisionNum(strCurAssayName.toStdString());

    // 把质控信息填充到表格中
    int iRow = 0;
    int iSelRow = -1;
    for (const auto& stuQcRltInfo : qryResp)
    {
        // 行数自增
        if (iRow >= m_pTblModel->rowCount())
        {
            m_pTblModel->setRowCount(iRow + 1);
        }

        // 更新单元项
        m_pTblModel->setItem(iRow, 0, new QStandardItem(""));                                                                           // 是否指向
        m_pTblModel->setItem(iRow, 1, new QStandardItem(""));                                                                           // 是否计算
        m_pTblModel->setItem(iRow, 2, new QStandardItem(QString::number(stuQcRltInfo.iUnionIdx)));                                      // 序号
        m_pTblModel->setItem(iRow, 3, new QStandardItem(stuQcRltInfo.strQcTime));                                                       // 质控时间
        m_pTblModel->setItem(iRow, 4, new QStandardItem(QString::number(stuQcRltInfo.dQcResult, 'f', pricisionNum)));					// 结果
		m_pTblModel->setItem(iRow, 5, new QStandardItem(""));																			// 状态（代理实现）
		m_pTblModel->setItem(iRow, 6, new QStandardItem(stuQcRltInfo.strOutCtrlRule));                                                  // 失控规则
        m_pTblModel->setItem(iRow, 7, new QStandardItem(stuQcRltInfo.strQcNo));                                                         // 质控品编号
        m_pTblModel->setItem(iRow, 8, new QStandardItem(stuQcRltInfo.strQcName));                                                       // 质控品名称
        m_pTblModel->setItem(iRow, 9, new QStandardItem(stuQcRltInfo.strQcBriefName));                                                  // 质控品简称
        m_pTblModel->setItem(iRow, 10, new QStandardItem(stuQcRltInfo.strQcSourceType));                                                // 质控品类型
        m_pTblModel->setItem(iRow, 11, new QStandardItem(stuQcRltInfo.strQcLevel));                                                     // 质控品水平
        m_pTblModel->setItem(iRow, 12, new QStandardItem(stuQcRltInfo.strQcLot));                                                       // 质控品批号
        m_pTblModel->setItem(iRow, 13, new QStandardItem(QString::number(stuQcRltInfo.dQcTargetVal, 'f', pricisionNum)));               // 靶值
        m_pTblModel->setItem(iRow, 14, new QStandardItem(QString::number(stuQcRltInfo.dQcSD, 'f', pricisionNum)));                      // SD
        m_pTblModel->setItem(iRow, 15, new QStandardItem(stuQcRltInfo.strOperator));                                                    // 操作者

        // 保存数据库主键及数据
        m_pTblModel->setItem(iRow, COL_INDEX_DB_NO_OF_QC_RESULT_TABLE, new QStandardItem(stuQcRltInfo.strID));
        m_pTblModel->item(iRow, COL_INDEX_DB_NO_OF_QC_RESULT_TABLE)->setData(QVariant::fromValue<TWIN_QC_RESULT_INFO>(stuQcRltInfo));

        // 记录联合索引
        auto it = m_mapUnionIndex.find(stuQcRltInfo.iUnionIdx);
        if (it == m_mapUnionIndex.end())
        {
            // 不存在则添加
            it = m_mapUnionIndex.insert(stuQcRltInfo.iUnionIdx, QList<int>());
        }
        it.value().push_back(iRow);
        m_pTblModel->item(iRow, COL_INDEX_INDICATE_OF_QC_RESULT_TABLE)->setData(stuQcRltInfo.iUnionIdx);
        m_pTblModel->item(iRow, COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE)->setData(stuQcRltInfo.bIsX);

        // 更新是否计算
        if (stuQcRltInfo.bCalculated)
        {
            m_pTblModel->item(iRow, COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE)->setIcon(QIcon(":/Leonis/resource/image/icon-select.png"));
        }
        else
        {
            m_pTblModel->item(iRow, COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE)->setIcon(QIcon());
        }

        // 根据失控状态判断单元格颜色
		m_pTblModel->item(iRow, COL_INDEX_STATE_OF_QC_RESULT_TABLE)->setData(ConvertTfEnumToQString(stuQcRltInfo.enQcState), Qt::DisplayRole);
        // 失控
        if (stuQcRltInfo.enQcState == QC_STATE_OUT_OF_CTRL)
        {
            // 红色
           m_pTblModel->item(iRow, COL_INDEX_STATE_OF_QC_RESULT_TABLE)->setData(StatusItemDelegate::STATE_ERROR, BK_STATE_ROLE);
			
		   // 有处理措施或失控原因则加标识
		   if (!stuQcRltInfo.strSolution.isEmpty() || !stuQcRltInfo.strOutCtrlReason.isEmpty())
		   {
			   m_pTblModel->item(iRow, COL_INDEX_STATE_OF_QC_RESULT_TABLE)->setData(QPixmap(QString(OUT_CONTROL_HAND_PNG)), BK_FLAG_ROLE);
		   }
        }
        else if (stuQcRltInfo.enQcState == QC_STATE_WARNING)
        {
            // 橙色
            m_pTblModel->item(iRow, COL_INDEX_STATE_OF_QC_RESULT_TABLE)->setData(StatusItemDelegate::STATE_WARNING, BK_STATE_ROLE);
        }

        // 如果与原本选中项数据库主键匹配，则选中该行
        if (stuQcRltInfo.strID.split("-").last().toLongLong() == iSelDbNo)
        {
            iSelRow = iRow;
        }

        // 行号自增
        ++iRow;
    }

    // 选中行有效，选中对应行
    if (iSelRow >= 0)
    {
        ui->QcResultTbl->selectRow(iSelRow);
    }

    // 表格内容居中
    SetTblTextAlign(m_pTblModel, Qt::AlignCenter);

    // 滚动到最后一条
    ui->QcResultTbl->scrollTo(m_pTblModel->index(iRow - 1, 0), QAbstractItemView::PositionAtBottom);

    // 根据内容调整列宽
    ResizeTblColToContent(ui->QcResultTbl);
	ResizeQcResultTableIconColumn();

    // 根据结果数更新打印导出按钮使能状态
    if (qryResp.isEmpty())
    {
        ui->PrintBtn->setEnabled(false);
        ui->export_btn->setEnabled(false);
    }
    else
    {
        ui->PrintBtn->setEnabled(true);
        ui->export_btn->setEnabled(true);
    }

    // 更新水平滚动条显示
    QTimer::singleShot(0, this, SLOT(UpdateQcRltTblHScroll()));
}

///
/// @brief
///     更新质控图(Youden图)
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月29日，新建函数
///
void QcGraphYoudenWidget::UpdateQcGraphic()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 重置质控图
    ui->QcGraphic->Reset();

    // 通过质控结果获取对应的点集
    QMap<int, QVector<QPointF>> mapCurvePoints;
    QMap<int, QVector<long long>> mapCurvePointsID;
    GetYoudenPointsFromQcResult(mapCurvePoints, &mapCurvePointsID);

    // 将质控曲线点集设置到质控图中
    SetYoudenPointsToQcGraphic(mapCurvePoints, &mapCurvePointsID);
}

///
/// @brief
///     更新质控图但不更新选中效果(Youden图)
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月29日，新建函数
///
void QcGraphYoudenWidget::UpdateQcGraphicButSel()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 重置质控图
    ui->QcGraphic->ResetButSel();

    // 通过质控结果获取对应的点集
    QMap<int, QVector<QPointF>> mapCurvePoints;
    QMap<int, QVector<long long>> mapCurvePointsID;
    GetYoudenPointsFromQcResult(mapCurvePoints, &mapCurvePointsID);

    // 将质控曲线点集设置到质控图中
    SetYoudenPointsToQcGraphic(mapCurvePoints, &mapCurvePointsID);
}

///
/// @brief
///     更新所有质控信息
///
/// @par History:
/// @li 4170/TangChuXian，2021年10月25日，新建函数
///
void QcGraphYoudenWidget::UpdateAllQcInfo()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 更新质控结果
    UpdateQcResult();

    // 更新质控图
    UpdateQcGraphic();
}

///
/// @brief
///     更新联合质控项目
///
/// @par History:
/// @li 4170/TangChuXian，2021年12月9日，新建函数
///
void QcGraphYoudenWidget::UpdateQcTwinItem()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    QString strSelAssayName = ui->AssayCombo->currentText();

    // 加载项目下拉框
    QStringList strAssayNameList;
    if (!gUiAdapterPtr(m_strCurDevName, m_strCurDevGroupName)->GetQcTpAssayList(strAssayNameList))
    {
        ULOG(LOG_ERROR, "%s()", __FUNCTION__);
    }

	// 将项目列表添加到下拉框,并调整
	ui->AssayCombo->clear();

	//根据组设备获取子设备
	QStringList strSubModelList;
	if (!gUiAdapterPtr(m_strCurDevName, m_strCurDevGroupName)->GetSubDevCombineNameList(strSubModelList))
	{
		ULOG(LOG_ERROR, "%s()", __FUNCTION__);
	}

	UpdateSubDevReadioBtnGroup(strSubModelList);
	ui->AssayCombo->addItems(strAssayNameList);

    // 选中之前选中项
    if (ui->AssayCombo->findText(strSelAssayName) >= 0)
    {
        ui->AssayCombo->setCurrentText(strSelAssayName);
    }

}

///
/// @brief
///     质控结果更新
///
/// @param[in]  uAssayCode  项目编号
///
/// @par History:
/// @li 4170/TangChuXian，2021年10月27日，新建函数
///
void QcGraphYoudenWidget::OnQcResultUpdate(unsigned int uAssayCode)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    if (ui->AssayCombo->currentData().toUInt() != uAssayCode)
    {
        ULOG(LOG_INFO, "%s(), ignore", __FUNCTION__);
        return;
    }

    // 更新质控信息
    UpdateAllQcInfo();
}

///
/// @brief
///     加载质控失控信息
///
/// @param[in]  pQcOutCtrlDlg  质控失控对话框
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月13日，新建函数
///
bool QcGraphYoudenWidget::LoadQcOutCtrlInfo(QcOutCtrlDlg* pQcOutCtrlDlg)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 检查参数
    if (pQcOutCtrlDlg == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s(), invalid param.", __FUNCTION__);
        return false;
    }

    // 判断质控结果当前选中项是否为有效索引
    QModelIndex selIndex = ui->QcResultTbl->currentIndex();
    if (!selIndex.isValid() || (m_pTblModel->item(selIndex.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE) == Q_NULLPTR))
    {
        ULOG(LOG_WARN, "%s(), select invalid index!", __FUNCTION__);
        return false;
    }

    // 获取选中质控结果数据
    TWIN_QC_RESULT_INFO stuQcResult = m_pTblModel->item(selIndex.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE)->data().value<TWIN_QC_RESULT_INFO>();

    // 将结果信息加载到界面上
	pQcOutCtrlDlg->InitViewInfo(ui->AssayCombo->currentText().toStdString(), stuQcResult);

    return true;
}

///
/// @brief
///     保存质控失控信息
///
/// @param[in]  pQcOutCtrlDlg  质控失控对话框
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月13日，新建函数
///
bool QcGraphYoudenWidget::SaveQcOutCtrlInfo(QcOutCtrlDlg* pQcOutCtrlDlg)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 检查参数
    if (pQcOutCtrlDlg == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s(), invalid param.", __FUNCTION__);
        return false;
    }

    // 判断质控结果当前选中项是否为有效索引
    QModelIndex selIndex = ui->QcResultTbl->currentIndex();
    if (!selIndex.isValid() || (m_pTblModel->item(selIndex.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE) == Q_NULLPTR))
    {
        ULOG(LOG_ERROR, "%s(), select invalid index!.", __FUNCTION__);
        return false;
    }

    // 获取选中质控结果数据并更新
    TWIN_QC_RESULT_INFO stuQcResult = m_pTblModel->item(selIndex.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE)->data().value<TWIN_QC_RESULT_INFO>();

    // 获取失控原因和处理措施
    QString strReason = pQcOutCtrlDlg->ui->OutCtrlReasonEdit->toPlainText();
    QString strDeal = pQcOutCtrlDlg->ui->SolutionEdit->toPlainText();

    // 保存失控处理
    if (!gUiAdapterPtr(m_strCurDevName,m_strCurDevGroupName)->UpdateQcOutCtrlReasonAndSolution(stuQcResult, strReason, strDeal))
    {
        ULOG(LOG_ERROR, "%s(), UpdateQcOutCtrlReasonAndSolution failed!!", __FUNCTION__);
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("保存失败!")));
        pTipDlg->exec();
        return false;
    }

    // 更新缓存的质控结果数据
    m_pTblModel->item(selIndex.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE)->setData(QVariant::fromValue<TWIN_QC_RESULT_INFO>(stuQcResult));

	// 更新标识
	if (!strDeal.isEmpty() || !strReason.isEmpty())
	{
		m_pTblModel->item(selIndex.row(), COL_INDEX_STATE_OF_QC_RESULT_TABLE)->setData(QPixmap(QString(OUT_CONTROL_HAND_PNG)), BK_FLAG_ROLE);
	}
	else
	{
		m_pTblModel->item(selIndex.row(), COL_INDEX_STATE_OF_QC_RESULT_TABLE)->setData(QPixmap(), BK_FLAG_ROLE);
	}

    return true;
}

///
/// @brief
///     构造Youden曲线的日内质控点集
///
/// @param[out]  mapCurvePoints     曲线的点集映射(key为曲线ID，value为对应点集)
/// @param[out]  pMapCurvePointsID  曲线的点集ID映射(key为曲线ID，value为对应点集ID)
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月29日，新建函数
///
bool QcGraphYoudenWidget::GetYoudenPointsFromQcResult(QMap<int, QVector<QPointF>> &mapCurvePoints, QMap<int, QVector<long long>>* pMapCurvePointsID)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 不为空则清空
    if (pMapCurvePointsID != Q_NULLPTR)
    {
        pMapCurvePointsID->clear();
        pMapCurvePointsID->insert(McQcYoudenGraphic::CURVE_ID_QC_1, QVector<long long>());
        pMapCurvePointsID->insert(McQcYoudenGraphic::CURVE_ID_QC_2, QVector<long long>());
        pMapCurvePointsID->insert(McQcYoudenGraphic::CURVE_ID_QC_NO_CALC, QVector<long long>());
    }

    // 初始化点集映射
    mapCurvePoints.clear();
    mapCurvePoints.insert(McQcYoudenGraphic::CURVE_ID_QC_1, QVector<QPointF>());
    mapCurvePoints.insert(McQcYoudenGraphic::CURVE_ID_QC_2, QVector<QPointF>());
    mapCurvePoints.insert(McQcYoudenGraphic::CURVE_ID_QC_NO_CALC, QVector<QPointF>());

    // 初始化质控信息和查询条件
    QList<TWIN_QC_RESULT_INFO>      qryResp;
    QC_RESULT_QUERY_COND            qryCond;

	// 组合名称和当前名称不一致则为组合设备
	QString devCombineName = m_strCurSubDevReadioName;
	int iModuleNo = 1;
	if (devCombineName != m_strCurDevName)
	{
		// ISE特殊处理
		if (gUiAdapterPtr()->GetDeviceClasssify(m_strCurDevName, m_strCurDevGroupName) == DEVICE_CLASSIFY_ISE)
		{
			// 索引名称
			QString modelIndexName = devCombineName.remove(m_strCurDevName);
			if (modelIndexName.size() == 1 &&
				modelIndexName.at(0).unicode() >= QChar('A').unicode() && modelIndexName.at(0).unicode() <= QChar('Z').unicode())
			{
				iModuleNo = modelIndexName.at(0).unicode() - QChar('A').unicode() + 1;
			}
		}
		else
		{
			// 组合设备，需要分解
			m_strCurDevName = devCombineName.remove(m_strCurDevGroupName);
		}
	}

	// 项目名称
	QString strCurAssayName = ui->AssayCombo->currentText();

    // 构造查询条件
    qryCond.startDate = ui->QcStartDateEdit->date();
    qryCond.endDate = ui->QcEndDateEdit->date();
    qryCond.strAssayName = strCurAssayName;

    // 加载项目下拉框
    QStringList strAssayNameList;
    if (!gUiAdapterPtr(m_strCurDevName, m_strCurDevGroupName, iModuleNo)->QueryQcYoudenRltInfo(qryCond, qryResp))
    {
        ULOG(LOG_WARN, "%s(), QueryQcYoudenRltInfo failed.", __FUNCTION__);
        return false;
    }

    // 判断是否为当前日期
    auto IsCurrentDate = [](boost::posix_time::ptime ptDate)
    {
		if (ptDate.is_not_a_date_time())
		{
			return false;
		}

		// 获取当前日期，判断是不是最新点
		QDate curDate = QDate::currentDate();
		if (ptDate.date().day() == curDate.day() &&
			ptDate.date().month() == curDate.month() &&
			ptDate.date().year() == curDate.year())
		{
			return true;
		}
		
		return false;
    };

    // 遍历联合质控数据
    QMap<int, QPointF> mapYoudenQcPt;
    QMap<int, long long> mapYoudenQcPtXID;
    QMap<int, QPair<boost::posix_time::ptime, boost::posix_time::ptime>> mapStuYoudenDate;
    for (const auto& stuYoudenInfo : qryResp)
    {
        // 计算坐标值
        double dResult = stuYoudenInfo.dQcResult;
        double dVal = (dResult - stuYoudenInfo.dQcTargetVal) / stuYoudenInfo.dQcSD;

        // 坐标最大绝对值3.9
        if (dVal > 3.9)
        {
            dVal = 3.9;
        }
        else if (dVal < -3.9)
        {
            dVal = -3.9;
        }

        // 查找对应联合编号对应的点
        auto it = mapYoudenQcPt.find(stuYoudenInfo.iUnionIdx);
        if (it == mapYoudenQcPt.end())
        {
            // 构造结构体
            QPair<boost::posix_time::ptime, boost::posix_time::ptime> pairDate;

            // 构造联合质控坐标点
            QPointF ptYouden;
            if (stuYoudenInfo.bIsX)
            {
                ptYouden.setX(dVal);
                pairDate.first = TimeStringToPosixTime(stuYoudenInfo.strQcTime.toStdString());
                mapYoudenQcPtXID.insert(stuYoudenInfo.iUnionIdx, stuYoudenInfo.strID.split("-").last().toLongLong());
            }
            else
            {
                ptYouden.setY(dVal);
                pairDate.second = TimeStringToPosixTime(stuYoudenInfo.strQcTime.toStdString());
            }

            // 插入坐标点
            mapYoudenQcPt.insert(stuYoudenInfo.iUnionIdx, ptYouden);
            mapStuYoudenDate.insert(stuYoudenInfo.iUnionIdx, pairDate);
            continue;
        }

        // 更新坐标
        if (stuYoudenInfo.bIsX)
        {
            it.value().setX(dVal);
            mapStuYoudenDate[stuYoudenInfo.iUnionIdx].first = TimeStringToPosixTime(stuYoudenInfo.strQcTime.toStdString());
        }
        else
        {
            it.value().setY(dVal);
            mapStuYoudenDate[stuYoudenInfo.iUnionIdx].second = TimeStringToPosixTime(stuYoudenInfo.strQcTime.toStdString());
        }

        // 判断是否为不计算点
        if (!stuYoudenInfo.bCalculated)
        {
            mapCurvePoints[McQcYoudenGraphic::CURVE_ID_QC_NO_CALC].push_back(it.value());
        }
        else
        {
            // 判断是不是最新点
            if (IsCurrentDate(mapStuYoudenDate[stuYoudenInfo.iUnionIdx].first) ||
                IsCurrentDate(mapStuYoudenDate[stuYoudenInfo.iUnionIdx].second))
            {
                mapCurvePoints[McQcYoudenGraphic::CURVE_ID_QC_2].push_back(it.value());
            }
            else
            {
                mapCurvePoints[McQcYoudenGraphic::CURVE_ID_QC_1].push_back(it.value());
            }
        }

        // 增加ID
        if (pMapCurvePointsID == Q_NULLPTR)
        {
            continue;
        }

        // 获取X轴ID
        long long iXptID;
        if (stuYoudenInfo.bIsX)
        {
            iXptID = stuYoudenInfo.strID.split("-").last().toLongLong();
        }
        else
        {
            auto itID = mapYoudenQcPtXID.find(stuYoudenInfo.iUnionIdx);
            if (itID == mapYoudenQcPtXID.end())
            {
                continue;
            }
            iXptID = itID.value();
        }

        // 获取引用
        QMap<int, QVector<long long>>& mapCurvePointsID = *pMapCurvePointsID;
        // 判断是否为不计算点
        if (!stuYoudenInfo.bCalculated)
        {
            mapCurvePointsID[McQcYoudenGraphic::CURVE_ID_QC_NO_CALC].push_back(iXptID);
        }
        else
        {
            // 判断是不是最新点
            if (IsCurrentDate(mapStuYoudenDate[stuYoudenInfo.iUnionIdx].first) ||
                IsCurrentDate(mapStuYoudenDate[stuYoudenInfo.iUnionIdx].second))
            {
                mapCurvePointsID[McQcYoudenGraphic::CURVE_ID_QC_2].push_back(iXptID);
            }
            else
            {
                mapCurvePointsID[McQcYoudenGraphic::CURVE_ID_QC_1].push_back(iXptID);
            }
        }
    }

    return true;
}

///
/// @brief
///     将曲线上的点集映射设置到质控图中
///
/// @param[out]  mapCurvePoints     曲线的点集映射(key为曲线ID，value为对应点集)
/// @param[out]  pMapCurvePointsID  曲线的点集ID映射(key为曲线ID，value为对应点集ID)
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月29日，新建函数
///
bool QcGraphYoudenWidget::SetYoudenPointsToQcGraphic(const QMap<int, QVector<QPointF>> &mapCurvePoints, QMap<int, QVector<long long>>* pMapCurvePointsID)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 遍历曲线点集映射，将点集设置到对应曲线
    for (auto it = mapCurvePoints.begin(); it != mapCurvePoints.end(); it++)
    {
        if (pMapCurvePointsID == Q_NULLPTR)
        {
            ui->QcGraphic->SetPoints(McQcYoudenGraphic::CURVE_ID(it.key()), it.value());
            continue;
        }

        QMap<int, QVector<long long>>& mapCurvePointsID = *pMapCurvePointsID;
        auto itPtID = mapCurvePointsID.find(it.key());
        if (itPtID == mapCurvePointsID.end())
        {
            ui->QcGraphic->SetPoints(McQcYoudenGraphic::CURVE_ID(it.key()), it.value());
            continue;
        }

        ui->QcGraphic->SetPoints(McQcYoudenGraphic::CURVE_ID(it.key()), it.value(), itPtID.value());
    }

    return true;
}

///
/// @brief
///     质控条件改变
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月10日，新建函数
///
void QcGraphYoudenWidget::OnQcCondChanged()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 更新质控结果表
    UpdateQcResult();

    // 更新质控图
    UpdateQcGraphic();
}

///
/// @brief
///     细节按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月10日，新建函数
///
void QcGraphYoudenWidget::OnDetailBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 判断质控结果当前选中项是否为有效索引
    QModelIndex selIndex = ui->QcResultTbl->currentIndex();
    if (!selIndex.isValid() || (m_pTblModel->item(selIndex.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE) == Q_NULLPTR))
    {
        ULOG(LOG_WARN, "%s(), select invalid index!", __FUNCTION__);
        return;
    }

    // 获取选中质控结果数据
    TWIN_QC_RESULT_INFO stuQcResult = m_pTblModel->item(selIndex.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE)->data().value<TWIN_QC_RESULT_INFO>();

    // 判断当前设备类型
    if (gUiAdapterPtr()->GetDeviceClasssify(m_strCurDevName, m_strCurDevGroupName) == DEVICE_CLASSIFY_CHEMISTRY)
    {
        // 生化的质控结果数据，弹出生化的结果详情对话框
        std::shared_ptr<chQcResultDetailsDlg> pQcResultDetailsDlg(new chQcResultDetailsDlg(this));
        pQcResultDetailsDlg->setChAssayResultID(stuQcResult.strRltDetailID.toLongLong(), stuQcResult.bCalculated, stuQcResult.enQcState);

        // 弹出对话框
        if (pQcResultDetailsDlg->exec() == QDialog::Rejected)
        {
            ULOG(LOG_INFO, "%s(), Canceled.", __FUNCTION__);
            return;
        }
    }
	else if (gUiAdapterPtr()->GetDeviceClasssify(m_strCurDevName, m_strCurDevGroupName) == DEVICE_CLASSIFY_ISE)
	{
		// ISE的质控结果数据，弹出生化的结果详情对话框
		std::shared_ptr<chQcResultDetailsDlg> pQcResultDetailsDlg(new chQcResultDetailsDlg(this));
		pQcResultDetailsDlg->setIseAssayResultID(stuQcResult.strRltDetailID.toLongLong(), stuQcResult.bCalculated, stuQcResult.enQcState);

		// 弹出对话框
		if (pQcResultDetailsDlg->exec() == QDialog::Rejected)
		{
			ULOG(LOG_INFO, "%s(), Canceled.", __FUNCTION__);
			return;
		}

	}
    else if (gUiAdapterPtr()->GetDeviceClasssify(m_strCurDevName, m_strCurDevGroupName) == DEVICE_CLASSIFY_IMMUNE)
    {
        // 免疫的质控结果数据，弹出免疫的结果详情对话框
        std::shared_ptr<ImQcItemRltDetailDlg> pQcResultDetailsDlg(new ImQcItemRltDetailDlg(this));
        pQcResultDetailsDlg->SetItemRltDetailId(stuQcResult.strRltDetailID.toLongLong(), stuQcResult.bCalculated);

        // 弹出对话框
        if (pQcResultDetailsDlg->exec() == QDialog::Rejected)
        {
            ULOG(LOG_INFO, "%s(), Canceled.", __FUNCTION__);
            return;
        }
    }
}

///
/// @brief
///     失控处理按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月13日，新建函数
///
void QcGraphYoudenWidget::OnOutCtrlBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 创建项目详细对话框
    std::shared_ptr<QcOutCtrlDlg> pQcOutCtrlDlg(new QcOutCtrlDlg(this));

    // 加载失控处理信息
    LoadQcOutCtrlInfo(pQcOutCtrlDlg.get());

    // 弹出对话框
    if (pQcOutCtrlDlg->exec() == QDialog::Rejected)
    {
        ULOG(LOG_INFO, "%s(), Canceled.", __FUNCTION__);
        return;
    }

    // 保存失控处理信息
    SaveQcOutCtrlInfo(pQcOutCtrlDlg.get());
}

///
/// @brief 获取打印导出数据
///
/// @par History:
/// @li 6889/ChenWei，2023年12月14日，新建函数
///
bool QcGraphYoudenWidget::GetPrintExportInfo(QCYDInfo& info)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 打印图片
    ui->QcGraphic->SetPrintMode(true);
    QImage printImg(ui->QcGraphic->ui->QcGraphic->size(), QImage::Format_RGB32);
    ui->QcGraphic->ui->QcGraphic->render(&printImg);
    ui->QcGraphic->SetPrintMode(false);

    // 构造查询时间区间
    // 设置起止日期
    QDate startDate = ui->QcStartDateEdit->date();
    QDate endDate = ui->QcEndDateEdit->date();
    QString strDateRange = startDate.toString(UI_DATE_FORMAT) + " - " + endDate.toString(UI_DATE_FORMAT);
    info.strTimeQuantum = strDateRange.toStdString();                               // 质控日期
    info.strModelName = GetCurrentSelectedDevName().toStdString();                  // 模块名称
    info.strItemName = ui->AssayCombo->currentText().toStdString();                 // 项目名称
    auto assayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(ui->AssayCombo->currentText().toStdString());

    // 显示精度
    int pricisionNum = 2;
    if (assayInfo == nullptr)
    {
		ULOG(LOG_INFO, "%s(), assayInfo is empty.", __FUNCTION__);
		return false;
    }
	info.strItemName = assayInfo->printName;
	pricisionNum = CommonInformationManager::GetInstance()->GetPrecisionNum(assayInfo->assayName);

    // 判断是否显示不计算点
    bool bShowNoCalcPt = ui->QcGraphic->ui->ShowNoCalcQcCB->isChecked();

     // 遍历质控结果
    for (int iRow = 0; iRow < m_pTblModel->rowCount(); iRow++)
    {
        // 获取item
        QStandardItem* pItem = m_pTblModel->item(iRow, COL_INDEX_DB_NO_OF_QC_RESULT_TABLE);
        if (pItem == Q_NULLPTR)
        {
            continue;
        }

        // 获取选中质控结果数据
        TWIN_QC_RESULT_INFO stuQcResult = pItem->data().value<TWIN_QC_RESULT_INFO>();

        // 如果不显示不计算点，则忽略不计算的结果
        if (!bShowNoCalcPt && !stuQcResult.bCalculated)
        {
            continue;
        }

        QCResult result;
        result.strDisplayIndex = QString::number(stuQcResult.iUnionIdx).toStdString();						// 显示序号
        result.strQCDate = stuQcResult.strQcTime.toStdString();				// 质控时间
        result.strResult = QString::number(stuQcResult.dQcResult, 'f', pricisionNum).toStdString();			// 结果值
        info.vecResult.push_back(result);
    }

    // 判断是否有数据需要打印
    if (info.vecResult.empty())
    {
        // 弹框提示导出完成
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("没有可以打印的数据!")));
        pTipDlg->exec();
        return false;
    }

    QByteArray ba;
    QBuffer buff(&ba);
    buff.open(QIODevice::WriteOnly);
    printImg.save(&buff, "PNG");
    QString strPrintImg(ba.toBase64());
    info.strTPImage = strPrintImg.toStdString();

    return true;
}

///
/// @brief
///     打印按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2022年7月29日，新建函数
///
void QcGraphYoudenWidget::OnPrintBtnCliked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 判空提示
	if (m_pTblModel->rowCount() <= 0)
	{
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("没有可以打印的数据!")));
		pTipDlg->exec();
		return;
	}

    QCYDInfo info;
    info.strInstrumentModel = " ";                                                         // 仪器型号
    QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    info.strPrintTime = strPrintTime.toStdString();                                             // 打印时间
    if (!GetPrintExportInfo(info))
    {
        return;
    }

    // 调用打印接口
    ULOG(LOG_INFO, "GetJsonString");
    std::string strInfo = GetJsonString(info);
    ULOG(LOG_INFO, "Print datas : %s", strInfo);
    std::string unique_id;
    int irect = printcom::printcom_async_print(strInfo, unique_id);
    ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);

    // 弹框提示打印结果
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("数据已发送到打印机!")));
    pTipDlg->exec();
    return;
}

///
/// @brief
///     导出按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月11日，新建函数
///
void QcGraphYoudenWidget::OnExportBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
	// 判空提示
	if (m_pTblModel->rowCount() <= 0)
	{
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("没有可以导出的数据!")));
		pTipDlg->exec();
		return;
	}

    QCYDInfo Info;
    if (!GetPrintExportInfo(Info))
    {
        return;
    }

    // 导出显示项目简称
    auto assayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(ui->AssayCombo->currentText().toStdString());
    if (assayInfo == nullptr)
    {
        ULOG(LOG_ERROR, "%s(), assayInfo is empty.", __FUNCTION__);
        return;
    }
    Info.strItemName = assayInfo->assayName;

    // 弹出保存文件对话框
    QString strFilepath = QFileDialog::getSaveFileName(this, tr("保存为..."), QString(), tr("EXCEL files (*.xlsx);;CSV files (*.csv);;PDF files (*.pdf)"));
    if (strFilepath.isEmpty())
    {
        //ALOGI << "Export canceled!";
        return;
    }

    bool bRect = false;
    QFileInfo FileInfo(strFilepath);
    QString strSuffix = FileInfo.suffix();
    if (strSuffix == "pdf")
    {
        QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
        Info.strPrintTime = strPrintTime.toStdString();
        std::string strInfo = GetJsonString(Info);
        ULOG(LOG_INFO, "Print datas : %s", strInfo);
        std::string unique_id;
        QString strDirPath = QCoreApplication::applicationDirPath();
        QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportQCTPInfo.lrxml";
        int irect = printcom::printcom_async_assign_export(strInfo, strFilepath.toStdString(), strUserTmplatePath.toStdString(), unique_id);
        ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
        bRect = true;
    }
    else
    {
        std::shared_ptr<FileExporter> pFileEpt(new FileExporter());
        bRect = pFileEpt->ExportTpInfo(Info, strFilepath);
    }
    
    // 弹框提示导出
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(bRect ? tr("导出完成!") : tr("导出失败！")));
    pTipDlg->exec();
}

///
/// @brief
///     更新设备列表
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月19日，新建函数
///
void QcGraphYoudenWidget::UpdateDevNameList()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取设备列表
    if (!gUiAdapterPtr()->GetGroupDevNameList(m_strDevNameList))
    {
        ULOG(LOG_ERROR, "%s(), GetGroupDevNameList() failed", __FUNCTION__);
        return;
    }
	if (m_strDevNameList.isEmpty())
	{
		ULOG(LOG_ERROR, "%s(), m_strDevNameList is empty", __FUNCTION__);
		return;
	}

	// 是否有子设备
	QString devNameTemp = m_strDevNameList.front();
	if (gUiAdapterPtr()->WhetherContainOtherSubDev(devNameTemp))
	{
		// 有子设备
		m_strCurDevGroupName = devNameTemp;

		// 查找子设备
		QStringList subDevList;
		if (!gUiAdapterPtr()->GetSubDevNameList(m_strCurDevGroupName, subDevList))
		{
			ULOG(LOG_ERROR, "%s(), GetGroupDevNameList() failed", __FUNCTION__);
			return;
		}

		// 默认当前设备组为第一个设备
		if (subDevList.isEmpty())
		{
			ULOG(LOG_ERROR, "%s(), subDevList is empty", __FUNCTION__);
			return;
		}

		m_strCurDevName = subDevList.front();
	}
	else
	{
		// 没有子设备
		m_strCurDevName = devNameTemp;
		m_strCurDevGroupName = "";
	}

    // 通知设备列表更新
    POST_MESSAGE(MSG_ID_QC_TAB_CORNER_DISPLAY_UPDATE, m_strDevNameList, m_strDevNameList.indexOf(m_strCurDevGroupName.isEmpty() ? m_strCurDevName : m_strCurDevGroupName));
}

///
/// @brief
///     更新项目名列表
///
/// @par History:
/// @li 4170/TangChuXian，2023年1月3日，新建函数
///
void QcGraphYoudenWidget::UpdateAssayNameList()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    auto AddAssayComboItems = [this](const QStringList& strAssayNameList)
    {
        // 遍历项目名列表
        for (const auto& strAssayName : strAssayNameList)
        {
            // 获取项目信息
            auto spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(strAssayName.toStdString());
            if (spAssayInfo == Q_NULLPTR)
            {
                continue;
            }

            // 添加到下拉框中
            ui->AssayCombo->addItem(strAssayName, spAssayInfo->assayCode);
        }
    };

    // 阻塞项目下拉框信号
    ui->AssayCombo->blockSignals(true);

    // 获取当前选中项目编号和设备名称
    int iSelAssayCode = -1;
    if (ui->AssayCombo->currentData().isValid())
    {
        iSelAssayCode = ui->AssayCombo->currentData().toInt();
    }

    // 加载项目下拉框
    QStringList strAssayNameList;
    if (!gUiAdapterPtr(m_strCurDevName, m_strCurDevGroupName)->GetQcTpAssayList(strAssayNameList))
    {
        ULOG(LOG_ERROR, "%s()", __FUNCTION__);
    }

	// 将项目列表添加到下拉框,并调整
	ui->AssayCombo->clear();

	// 更新子设备列表
	QStringList strSubModelList;
	if (!gUiAdapterPtr(m_strCurDevName, m_strCurDevGroupName)->GetSubDevCombineNameList(strSubModelList))
	{
		ULOG(LOG_ERROR, "%s()", __FUNCTION__);
	}
	UpdateSubDevReadioBtnGroup(strSubModelList);

	// 更新项目下拉框，如果之前选中项存在则选中
    AddAssayComboItems(strAssayNameList);

    // 如果之前选中项存在则选中
    int iSelIndex = ui->AssayCombo->findData(iSelAssayCode);
    if (iSelIndex >= 0 && iSelAssayCode > 0)
    {
        ui->AssayCombo->setCurrentIndex(iSelIndex);
    }
    else
    {
        // 更新质控结果表
        UpdateQcResult();

        // 更新质控图
        UpdateQcGraphic();
    }

    // 阻塞项目下拉框信号
    ui->AssayCombo->blockSignals(false);
}

///
/// @brief
///     质控结果表单元格被点击
///
/// @param[in]  index  点击单元格索引
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月16日，新建函数
///
void QcGraphYoudenWidget::OnQcResultTblItemClicked(const QModelIndex& index)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 如果点击单元项不为是否计算或者是空行，则返回
    if ((index.column() != COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE) || (m_pTblModel->item(index.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE) == Q_NULLPTR))
    {
        // 忽略点击
        ULOG(LOG_INFO, "%s(), ignore clicked", __FUNCTION__);
        return;
    }

    // 没有权限直接返回
    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();
    if (!userPms->IsPermisson(PSM_QC_TWINPLOT_CALCCHECK))
    {
        // 没有权限操作
        ULOG(LOG_WARN, "%s(), no permission", __FUNCTION__);
        return;
    }

    // 获取选中质控结果数据
    TWIN_QC_RESULT_INFO stuSelQcResult = m_pTblModel->item(index.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE)->data().value<TWIN_QC_RESULT_INFO>();

    // 如果是否计算未改变，则忽略
    bool bCalcChecked = !m_pTblModel->item(index.row(), COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE)->icon().isNull();
    if (stuSelQcResult.bCalculated == bCalcChecked)
    {
        // 忽略点击
        ULOG(LOG_INFO, "%s(), check state did not changed.", __FUNCTION__);
        m_pTblModel->item(index.row(), COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE)->setIcon(bCalcChecked ? QIcon() : QIcon(":/Leonis/resource/image/icon-select.png"));
        bCalcChecked = !bCalcChecked;
    }

    // 构造查询条件
    QC_RESULT_QUERY_COND qryCond;

    // 获取对应项结果ID
    std::int64_t iRecordIdX;
    std::int64_t iRecordIdY;
    int iSelDisplayIndex = m_pTblModel->item(index.row(), COL_INDEX_INDICATE_OF_QC_RESULT_TABLE)->data().toInt();
    int iUnionRow = -1;
    for (int iRow = 0; iRow < m_pTblModel->rowCount(); iRow++)
    {
        // 获取对应项
        QStandardItem* pItem = m_pTblModel->item(iRow, COL_INDEX_INDICATE_OF_QC_RESULT_TABLE);
        if (pItem == Q_NULLPTR)
        {
            continue;
        }

        // 获取质控联合索引
        int iDisplayIndex = pItem->data().toInt();
        if (iDisplayIndex != iSelDisplayIndex)
        {
            continue;
        }

        // 记录联合索引行
        if (index.row() != iRow)
        {
            iUnionRow = iRow;
        }

        // 获取质控结果
        TWIN_QC_RESULT_INFO stuQcResult = m_pTblModel->item(iRow, COL_INDEX_DB_NO_OF_QC_RESULT_TABLE)->data().value<TWIN_QC_RESULT_INFO>();
        bool bX = m_pTblModel->item(iRow, COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE)->data().toBool();
        if (bX)
        {
            iRecordIdX = stuQcResult.strID.split("-").last().toLongLong();
        }
        else
        {
            iRecordIdY = stuQcResult.strID.split("-").last().toLongLong();
        }

        // 更新选中状态
        m_pTblModel->item(iRow, COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE)->setIcon(!bCalcChecked ? QIcon() : QIcon(":/Leonis/resource/image/icon-select.png"));
    }

    // 执行更新计算点
    if (!gUiAdapterPtr(m_strCurDevName,m_strCurDevGroupName)->UpdateQcCalcPoint(stuSelQcResult, bCalcChecked))
    {
        ULOG(LOG_ERROR, "%s(), UpdateQcCalcPoint failed!!", __FUNCTION__);
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("操作失败!")));
        pTipDlg->exec();
        m_pTblModel->item(index.row(), COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE)->setIcon(bCalcChecked ? QIcon() : QIcon(":/Leonis/resource/image/icon-select.png"));
        if (iUnionRow >= 0)
        {
            m_pTblModel->item(iUnionRow, COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE)->setIcon(bCalcChecked ? QIcon() : QIcon(":/Leonis/resource/image/icon-select.png"));
        }
        return;
    }

    // 更新质控结果表数据
    m_pTblModel->item(index.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE)->setData(QVariant::fromValue<TWIN_QC_RESULT_INFO>(stuSelQcResult));

    // 更新对应联合质控所在行数据
    if (iUnionRow >= 0)
    {
        // 获取质控结果
        TWIN_QC_RESULT_INFO stuQcResult = m_pTblModel->item(iUnionRow, COL_INDEX_DB_NO_OF_QC_RESULT_TABLE)->data().value<TWIN_QC_RESULT_INFO>();
        stuQcResult.bCalculated = stuSelQcResult.bCalculated;
        m_pTblModel->item(iUnionRow, COL_INDEX_DB_NO_OF_QC_RESULT_TABLE)->setData(QVariant::fromValue<TWIN_QC_RESULT_INFO>(stuQcResult));
    }

    // 更新youden图
    UpdateQcGraphicButSel();

    // 更新youden图
    UpdateSetPtGraph();
}

///
/// @brief
///     质控图选中点改变
///
/// @param[in]  enCurveID  选中点所在曲线ID
/// @param[in]  iPtID      选中点ID
///
/// @par History:
/// @li 4170/TangChuXian，2022年5月25日，新建函数
///
void QcGraphYoudenWidget::OnQcGraphicSelPtChanged(McQcYoudenGraphic::CURVE_ID enCurveID, long long iPtID)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 如果曲线无效，则清空选项
    if (enCurveID == McQcYoudenGraphic::CURVE_ID_SIZE)
    {
        ui->QcResultTbl->setCurrentIndex(QModelIndex());
        return;
    }

    // 选中对应ID的选项
    for (int iRow = 0; iRow < m_pTblModel->rowCount(); iRow++)
    {
        // 空行则跳过
        if (m_pTblModel->item(iRow, COL_INDEX_DB_NO_OF_QC_RESULT_TABLE) == Q_NULLPTR)
        {
            continue;
        }

        // 获取数据
        TWIN_QC_RESULT_INFO stuQcResult = m_pTblModel->item(iRow, COL_INDEX_DB_NO_OF_QC_RESULT_TABLE)->data().value<TWIN_QC_RESULT_INFO>();

        // ID匹配则选中
        if (stuQcResult.strID.split("-").last().toLongLong() == iPtID)
        {
            ui->QcResultTbl->selectRow(iRow);
            break;
        }
    }
}

///
/// @brief
///     质控结果表选中项改变
///
/// @param[in]  curIdx      当前选中索引
/// @param[in]  prevIdx     前一次选中索引
///
/// @par History:
/// @li 4170/TangChuXian，2021年11月24日，新建函数
///
void QcGraphYoudenWidget::OnQcResultItemSelChanged(const QModelIndex& curIdx, const QModelIndex& prevIdx)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 失控处理按钮禁能
    ui->OutCtrlBtn->setEnabled(false);
    ui->DetailBtn->setEnabled(false);
	//ui->PrintBtn->setEnabled(false);
	//ui->export_btn->setEnabled(false);

    // 质控图清空选中效果
    ui->QcGraphic->SelPointByID(McQcYoudenGraphic::CURVE_ID_SIZE, UI_INVALID_DB_NO);

    // 更新联合索引标志
    auto FuncUpdateUnionIdxFlag = [this](int iUnionIdx, bool bVisible)
    {
        auto it = m_mapUnionIndex.find(iUnionIdx);
        if (it == m_mapUnionIndex.end())
        {
            return;
        }

        for (int iRow : it.value())
        {
            // 空行则跳过
            QStandardItem* pItem = m_pTblModel->item(iRow, COL_INDEX_INDICATE_OF_QC_RESULT_TABLE);
            if (pItem == Q_NULLPTR)
            {
                continue;
            }

            // 更新图标
            if (!bVisible)
            {
                // 清空图标
                //pItem->setData(QVariant(), Qt::DisplayRole);
                pItem->setIcon(QIcon());
                //pItem->setText("");
            }
            else
            {
                // 设置联合标志图标
                //pItem->setData(QImage(":/Leonis/resource/image/icon-union-qc.png"), Qt::DisplayRole);
                pItem->setIcon(QIcon(":/Leonis/resource/image/icon-union-qc.png"));
                //pItem->setText("▶");
            }
        }
    };

    // 上一次选中有效
    if (prevIdx.isValid() && (m_pTblModel->item(prevIdx.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE) != Q_NULLPTR))
    {
        // 隐藏上一次选中联合索引
        int iHideIconUnionIndex = m_pTblModel->item(prevIdx.row(), COL_INDEX_INDICATE_OF_QC_RESULT_TABLE)->data().toInt();
        FuncUpdateUnionIdxFlag(iHideIconUnionIndex, false);
    }

    // 如果当前没有选中项或选中项是空行，则返回
    if (!curIdx.isValid() || (m_pTblModel->item(curIdx.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE) == Q_NULLPTR))
    {
        // 忽略点击
        ULOG(LOG_INFO, "%s(), select invalid index.", __FUNCTION__);
        return;
    }

    // 获取选中质控结果数据取得联合标志对应索引并设置标志
    int iShowIconUnionIndex = m_pTblModel->item(curIdx.row(), COL_INDEX_INDICATE_OF_QC_RESULT_TABLE)->data().toInt();
    FuncUpdateUnionIdxFlag(iShowIconUnionIndex, true);

    // 在控时候，失控处理按钮禁用（警告需确认）
    TWIN_QC_RESULT_INFO stuQcResult = m_pTblModel->item(curIdx.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE)->data().value<TWIN_QC_RESULT_INFO>();
    if (stuQcResult.enQcState == QC_STATE_OUT_OF_CTRL)
    {
        ui->OutCtrlBtn->setEnabled(true);
    }

    ui->DetailBtn->setEnabled(true);
	//ui->PrintBtn->setEnabled(true);
	//ui->export_btn->setEnabled(true);

    // 获取对应的两个结果
    TWIN_QC_RESULT_INFO stuQcResultX;
    TWIN_QC_RESULT_INFO stuQcResultY;
    auto it = m_mapUnionIndex.find(iShowIconUnionIndex);
    if (it == m_mapUnionIndex.end())
    {
        // 忽略
        ULOG(LOG_INFO, "%s(), No union index.", __FUNCTION__);
        return;
    }

    // 遍历索引
    for (int iRow : it.value())
    {
        // 空行则跳过
        QStandardItem* pItem = m_pTblModel->item(iRow, COL_INDEX_DB_NO_OF_QC_RESULT_TABLE);
        if (pItem == Q_NULLPTR)
        {
            continue;
        }

        // 判断是否为X
        bool bX = m_pTblModel->item(iRow, COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE)->data().toBool();
        if (bX)
        {
            stuQcResultX = pItem->data().value<TWIN_QC_RESULT_INFO>();
        }
        else
        {
            stuQcResultY = pItem->data().value<TWIN_QC_RESULT_INFO>();
        }
    }

    // 如果X和Y不完整，则返回
    if (stuQcResultX.strID.split("-").last().toLongLong() < 0 || stuQcResultY.strID.split("-").last().toLongLong() < 0)
    {
        // 忽略
        ULOG(LOG_INFO, "%s(), No X and Y.", __FUNCTION__);
        return;
    }

    // 判断是否为当前日期
    auto IsCurrentDate = [](boost::posix_time::ptime ptDate)
    {
        // 获取当前日期，判断是不是最新点
        QDate curDate = QDate::currentDate();
        if (ptDate.date().day() == curDate.day() &&
            ptDate.date().month() == curDate.month() &&
            ptDate.date().year() == curDate.year())
        {
            return true;
        }

        return false;
    };

    // 质控图选中对应点
    if (!stuQcResult.bCalculated)
    {
        // 如果显示不计算点
        if (ui->QcGraphic->ui->ShowNoCalcQcCB->isChecked())
        {
            ui->QcGraphic->SelPointByID(McQcYoudenGraphic::CURVE_ID_QC_NO_CALC, stuQcResultX.strID.split("-").last().toLongLong());
        }
    }
    else
    {
        // 判断是不是最新点
        if (IsCurrentDate(TimeStringToPosixTime(stuQcResultX.strQcTime.toStdString())) ||
            IsCurrentDate(TimeStringToPosixTime(stuQcResultY.strQcTime.toStdString())))
        {
            ui->QcGraphic->SelPointByID(McQcYoudenGraphic::CURVE_ID_QC_2, stuQcResultX.strID.split("-").last().toLongLong());
        }
        else
        {
            ui->QcGraphic->SelPointByID(McQcYoudenGraphic::CURVE_ID_QC_1, stuQcResultX.strID.split("-").last().toLongLong());
        }
    }

    // 质控质控时间和结果
    // 获取质控结果
    QString strQcRes = tr("结果：") + QString::number(stuQcResultX.dQcResult) + "/" + QString::number(stuQcResultY.dQcResult);
    QStringList strTextList;
    strTextList << strQcRes;
    ui->QcGraphic->SetSelPointTipText(strTextList);
}

///
/// @brief
///     更新选中点质控图显示
///
/// @par History:
/// @li 4170/TangChuXian，2024年5月15日，新建函数
///
void QcGraphYoudenWidget::UpdateSetPtGraph()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    QModelIndex curIdx = ui->QcResultTbl->currentIndex();
    OnQcResultItemSelChanged(curIdx, QModelIndex());
}

///
/// @brief
///     请求获取打印模板变量值
///
/// @param[in]  iVarID       变量ID
/// @param[in]  iSeries      系列号
/// @param[in]  iItemOffset  第几份
///
/// @par History:
/// @li 4170/TangChuXian，2022年6月30日，新建函数
///
void QcGraphYoudenWidget::OnReqPrintTmpVarData(int iVarID, int iSeries, int iItemOffset)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 检测参数
//     if (iSeries != UI_PRINT_SERIES_QC_YOUDEN_GRAPHIC || iItemOffset < 0)
//     {
//         // 系列号不匹配
//         ALOGI << "iSeries do not match";
//         return;
//     }
// 
//     if (!m_mapPrintVarGetFunctions.contains(iVarID))
//     {
//         return;
//     }
// 
//     PrintDataMng::SetRetOfDataVal(iVarID, m_mapPrintVarGetFunctions[iVarID]());
}

///
/// @brief
///     请求获取打印模板表格变量表头
///
/// @param[in]  iVarID       变量ID
/// @param[in]  iSeries      系列号
/// @param[in]  iItemOffset  第几份
///
/// @par History:
/// @li 4170/TangChuXian，2022年6月30日，新建函数
///
void QcGraphYoudenWidget::OnReqPrintTmpVarTblHeaderData(int iVarID, int iSeries, int iItemOffset)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 检测参数
//     if (iSeries != UI_PRINT_SERIES_QC_YOUDEN_GRAPHIC || iItemOffset < 0)
//     {
//         // 系列号不匹配
//         ALOGI << "iSeries do not match";
//         return;
//     }
// 
//     // 判断表格类型
//     if (iVarID != PRINT_TMP_VAR_TBL_ID_QC_YOUDEN_RESULT_TBL)
//     {
//         // 系列号不匹配
//         ALOGI << "iVarID do not match";
//         return;
//     }
// 
//     // 设置表头
//     QMap<int, QString> mapHeader;
//     mapHeader[0] = LoadStrFromLanguage(CHAR_CODE::IDS_NO);              // 序号
//     mapHeader[1] = LoadStrFromLanguage(CHAR_CODE::IDS_QC_TIME);         // 质控时间
//     mapHeader[2] = LoadStrFromLanguage(CHAR_CODE::IDS_RLT);             // 结果
//     mapHeader[3] = LoadStrFromLanguage(CHAR_CODE::IDS_STATUS);          // 状态
//     mapHeader[4] = LoadStrFromLanguage(CHAR_CODE::IDS_QC_RULE);         // 失控规则
//     mapHeader[5] = LoadStrFromLanguage(CHAR_CODE::IDS_QCNAME);          // 质控品名称
//     mapHeader[6] = LoadStrFromLanguage(CHAR_CODE::IDS_CONTROL_ID);      // 质控品编号
//     mapHeader[7] = LoadStrFromLanguage(CHAR_CODE::IDS_QCLOT);           // 质控品批号
//     mapHeader[8] = LoadStrFromLanguage(CHAR_CODE::IDS_TARGET);          // 靶值
//     mapHeader[9] = LoadStrFromLanguage(CHAR_CODE::IDS_CONTROL_SD);      // 标准差
//     mapHeader[10] = LoadStrFromLanguage(CHAR_CODE::IDS_OPERATOR_EX);    // 操作者
// 
//     // 设置返回值
//     PrintDataMng::SetRetOfTblHeader(iVarID, mapHeader);
}

///
/// @brief
///     请求获取打印模板表格变量单元格
///
/// @param[in]  iVarID       变量ID
/// @param[in]  iSeries      系列号
/// @param[in]  iRow         行号
/// @param[in]  iCol         列号
/// @param[in]  iItemOffset  第几份
///
/// @par History:
/// @li 4170/TangChuXian，2022年6月30日，新建函数
///
void QcGraphYoudenWidget::OnReqPrintTmpVarTblItemData(int iVarID, int iSeries, int iRow, int iCol, int iItemOffset)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 检测参数
//     if (iSeries != UI_PRINT_SERIES_QC_YOUDEN_GRAPHIC || iItemOffset < 0)
//     {
//         // 系列号不匹配
//         ALOGI << "iSeries do not match";
//         return;
//     }
// 
//     // 判断表格类型
//     if (iVarID != PRINT_TMP_VAR_TBL_ID_QC_YOUDEN_RESULT_TBL)
//     {
//         // 系列号不匹配
//         ALOGI << "iVarID do not match";
//         return;
//     }
// 
//     // 判断行数列数是否合法
//     if (iRow >= m_vecPrintQcInfo.size() || iCol >= m_mapQcInfoGetFunctions.count())
//     {
//         ALOGW << "iRow or iCol is invalid.";
//         return;
//     }
// 
//     // 设置返回值
//     PrintDataMng::SetRetOfTblItem(iVarID, m_mapQcInfoGetFunctions[iCol](iRow));
}

///
/// @brief
///     请求获取打印模板表格变量行列数
///
/// @param[in]  iVarID       变量ID
/// @param[in]  iSeries      系列号
/// @param[in]  iItemOffset  第几份
///
/// @par History:
/// @li 4170/TangChuXian，2022年6月30日，新建函数
///
void QcGraphYoudenWidget::OnReqPrintTmpVarTblRowColCnt(int iVarID, int iSeries, int iItemOffset)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 检测参数
//     if (iSeries != UI_PRINT_SERIES_QC_YOUDEN_GRAPHIC || iItemOffset < 0)
//     {
//         // 系列号不匹配
//         ALOGI << "iSeries do not match";
//         return;
//     }
// 
//     // 判断表格类型
//     if (iVarID != PRINT_TMP_VAR_TBL_ID_QC_YOUDEN_RESULT_TBL)
//     {
//         // 系列号不匹配
//         ALOGI << "iVarID do not match";
//         return;
//     }
// 
//     // 初始化质控信息和查询条件
//     if (m_iCurPrintItemIndex != iItemOffset)
//     {
//         m_iCurPrintItemIndex = iItemOffset;
//         m_vecPrintQcInfo.clear();
//         UI::UI_QC_QUERY_COND    qryCond;
// 
//         // 构造查询条件
//         // 获取起止日期
//         boost::gregorian::date startDate(ui->QcStartDateEdit->date().year(),
//             ui->QcStartDateEdit->date().month(), ui->QcStartDateEdit->date().day());
//         boost::gregorian::date endDate(ui->QcEndDateEdit->date().year(),
//             ui->QcEndDateEdit->date().month(), ui->QcEndDateEdit->date().day());
// 
//         // 设置起止日期
//         qryCond.startDate = startDate;
//         qryCond.endDate = endDate;
//         qryCond.strItemName = ui->AssayCombo->currentText().toStdString();
//         qryCond.uItemCode = ui->AssayCombo->currentData().toInt();
// 
//         // 获取所有联合质控数据
//         if (!Hazel::DisplayYoudenInfo(qryCond, m_vecPrintQcInfo))
//         {
//             ALOGE << "failed to excute DisplayYoudenInfo.";
//             return;
//         }
// 
//     }
// 
//     // 更新打印表格
//     //UpdatePrintTbl(vecQcInfo);
// 
//     // 设置返回值
//     //PrintDataMng::SetRetOfTblRowColCnt(iVarID, QPair<int, int>(m_pPrintTblQcTPResult->rowCount(), m_pPrintTblQcTPResult->columnCount()));
//     PrintDataMng::SetRetOfTblRowColCnt(iVarID, QPair<int, int>(m_vecPrintQcInfo.size(), m_mapQcInfoGetFunctions.count()));
}

///
/// @brief
///     请求获取打印模板系列号打印份数
///
/// @param[in]  iSeries      系列号
///
/// @par History:
/// @li 4170/TangChuXian，2022年6月30日，新建函数
///
void QcGraphYoudenWidget::OnReqPrintTmpSeriesPrintCnt(int iSeries)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 检测参数
//     if (iSeries != UI_PRINT_SERIES_QC_YOUDEN_GRAPHIC)
//     {
//         // 系列号不匹配
//         ALOGI << "iSeries do not match";
//         return;
//     }
// 
//     // 只能打印一份
//     PrintDataMng::SetRetOfPrintCnt(iSeries, 1);
}

///
/// @brief
///     请求打印模板图片变量
///
/// @param[in]  iVarID       变量ID
/// @param[in]  iSeries      系列
/// @param[in]  iItemOffset  第几份
///
/// @par History:
/// @li 4170/TangChuXian，2022年7月5日，新建函数
///
void QcGraphYoudenWidget::OnReqPrintTmpVarImg(int iVarID, int iSeries, int iItemOffset)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 检测参数
//     if (iSeries != UI_PRINT_SERIES_QC_YOUDEN_GRAPHIC)
//     {
//         // 系列号不匹配
//         ALOGI << "iSeries do not match";
//         return;
//     }
// 
//     // 检查ID
//     if (iVarID != PRINT_TMP_VAR_IMG_ID_QC_YOUDEN_RESULT_IMG)
//     {
//         // ID不匹配
//         ALOGI << "iSeries do not match";
//         return;
//     }
// 
//     // 导出图片
//     ui->QcGraphic->SetPrintMode(true);
//     QImage printImg(ui->QcGraphic->ui->QcGraphic->size(), QImage::Format_RGB32);
//     ui->QcGraphic->ui->QcGraphic->render(&printImg);
//     ui->QcGraphic->SetPrintMode(false);
// 
//     // 设置返回值
//     printImg.save("D:/i800/trunk/I 800Analyzer/test2.png", "PNG");
//     PrintDataMng::SetRetOfVarImg(iVarID, printImg);
}

///
/// @brief
///     Tab角落按钮索引改变
///
/// @param[in]  iBtnIdx  按钮索引
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月15日，新建函数
///
void QcGraphYoudenWidget::OnTabCornerIndexChanged(int iBtnIdx)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 如果界面未显示，则不响应
    if (!isVisible())
    {
        return;
    }

    if (iBtnIdx < 0 || iBtnIdx >= m_strDevNameList.size())
    {
        ULOG(LOG_ERROR, "%s(), invalid param", __FUNCTION__);
        return;
    }

	QString devNameTemp = m_strDevNameList.at(iBtnIdx);

	if (gUiAdapterPtr()->WhetherContainOtherSubDev(devNameTemp))
	{
		// 有子设备
		m_strCurDevGroupName = devNameTemp;

		// 查找子设备
		QStringList subDevList;
		if (!gUiAdapterPtr()->GetSubDevNameList(m_strCurDevGroupName, subDevList))
		{
			ULOG(LOG_ERROR, "%s(), GetGroupDevNameList() failed", __FUNCTION__);
			return;
		}

		// 默认当前设备组为第一个设备
		if (subDevList.isEmpty())
		{
			ULOG(LOG_ERROR, "%s(), subDevList is empty", __FUNCTION__);
			return;
		}

		m_strCurDevName = subDevList.front();
	}
	else
	{
		// 没有子设备
		m_strCurDevName = devNameTemp;
		m_strCurDevGroupName = "";
	}

    // 更新项目名列表
    UpdateAssayNameList();

    // 更新质控结果
    UpdateQcResult();

    // 更新质控图
    UpdateQcGraphic();
}

///
/// @brief
///     质控品信息更新
///
/// @param[in]  vQcDoc  质控品信息
///
/// @par History:
/// @li 4170/TangChuXian，2023年1月4日，新建函数
///
void QcGraphYoudenWidget::OnQcDocInfoInfoUpdate(std::vector<tf::QcDocUpdate, std::allocator<tf::QcDocUpdate>> vQcDoc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    Q_UNUSED(vQcDoc);
    // 更新项目列表
    UpdateAssayNameList();

    // 更新质控结果表
    UpdateQcResult();

    // 更新质控图
    UpdateQcGraphic();
}

///
/// @brief
///     质控结果更新
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月14日，新建函数
///
void QcGraphYoudenWidget::OnQcRltUpdate(QString devSn, int64_t qcDocId, QVector<int64_t> changedId)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 更新项目列表
    UpdateAssayNameList();

    // 更新质控结果表
    UpdateQcResult();

    // 更新质控图
    UpdateQcGraphic();
}

///
/// @brief
///     更新质控结果表水平滚动显示
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月25日，新建函数
///
void QcGraphYoudenWidget::UpdateQcRltTblHScroll()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 记录水平滚动条滑块位置
    int iHScrollPos = ui->QcResultTbl->horizontalScrollBar()->value();

    // 更新滚动
    ui->QcResultTbl->horizontalScrollBar()->setValue(0);
    ui->QcResultTbl->horizontalScrollBar()->setValue(iHScrollPos);
}

///
/// @brief
///     项目信息更新
///
/// @par History:
/// @li 4170/TangChuXian，2023年12月21日，新建函数
///
void QcGraphYoudenWidget::OnAssayInfoUpdate()
{
    ULOG(LOG_INFO, __FUNCTION__);
    // 更新项目列表
    UpdateAssayNameList();

    // 更新质控结果表
    UpdateQcResult();

    // 更新质控图
    UpdateQcGraphic();
}

///
/// @bref
///		响应权限变化
///
/// @par History:
/// @li 8276/huchunli, 2023年8月18日，新建函数
///
void QcGraphYoudenWidget::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);
    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    // 权限限制
    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();
    ui->QcGraphic->ui->ShowNoCalcQcCB->setVisible(userPms->IsPermisson(PSM_QC_TWINPLOT_SHOWNOCALCPT));

    // 导出
    userPms->IsPermissionShow(PSM_QC_TWINPLOT_EXPORT) ? ui->export_btn->show() : ui->export_btn->hide();
}

///
/// @brief
///     更新设备选择按钮
///
/// @param[in]  devList  新的子设备列表
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年6月16日，新建函数
///
void QcGraphYoudenWidget::UpdateSubDevReadioBtnGroup(const QStringList& subDevList)
{
	// 判断设备
	if (subDevList.size() > 1) ui->groupBox_subDevs->setVisible(true);
	else                       ui->groupBox_subDevs->setVisible(false);

	// 清空组
	QList<QAbstractButton *> oldList = m_pSubDevReadioBtnGroup->buttons();
	foreach(auto btn, oldList) {
		m_pSubDevReadioBtnGroup->removeButton(btn);
	}

	// 清空界面
	QLayoutItem* item;
	while ((item = ui->horizontalLayout_subDevs->takeAt(0)) != nullptr) {
		delete item->widget();
		delete item;
	}

	// 将按钮添加进组
	bool initFirst = false;
	foreach(auto dev, subDevList) {
		// 创建
		QRadioButton *devBtn = new QRadioButton();
		devBtn->setText(dev);
		devBtn->setMinimumHeight(40);

		// 添加进组
		m_pSubDevReadioBtnGroup->addButton(devBtn);
		if (!initFirst)
		{
			devBtn->setChecked(true);
			emit m_pSubDevReadioBtnGroup->buttonClicked(devBtn);
			initFirst = true;
		}

		// 添加进布局
		ui->horizontalLayout_subDevs->addWidget(devBtn);

	}
	ui->horizontalLayout_subDevs->addSpacing(15);

}

///
/// @brief
///     获取当前选中设备的名称
///
///
/// @return  设备名称（可用于打印或导出）
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年9月19日，新建函数
///
QString QcGraphYoudenWidget::GetCurrentSelectedDevName()
{
	ULOG(LOG_INFO, __FUNCTION__);

	return m_strCurSubDevReadioName;
}

///
/// @brief 对图标列进行特殊处理
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年12月13日，新建函数
///
void QcGraphYoudenWidget::ResizeQcResultTableIconColumn()
{
	ui->QcResultTbl->setColumnWidth(0, 20);
	ui->QcResultTbl->setColumnWidth(5, 80);

	// 图标列不可修改
	ui->QcResultTbl->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	ui->QcResultTbl->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Fixed);

}
