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
/// @file     qcgraphljwidget.cpp
/// @brief    L-J质控图界面
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

#include "qcgraphljwidget.h"
#include <memory>
#include <algorithm>
#include <QStandardItemModel>
#include <QVariant>
#include <QTimer>
#include <QRadioButton>
#include <QFileDialog>
#include <QButtonGroup>
#include <QBuffer>
#include <QTableView>
#include <QPainter>

#include "ui_qcgraphljwidget.h"
#include "ui_qcoutctrldlg.h"
#include "ui_mcqcljgraphic.h"

#include "shared/CommonInformationManager.h"
#include "shared/uicommon.h"
#include "shared/uidef.h"
#include "shared/messagebus.h"
#include "shared/tipdlg.h"
#include "shared/statusitemdelegate.h"
#include "shared/msgiddef.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/mcpixmapitemdelegate.h"
#include "shared/datetimefmttool.h"
#include "shared/FileExporter.h"
#include "shared/DataManagerQc.h"

#include "uidcsadapter/uidcsadapter.h"
#include "qcoutctrldlg.h"
#include "ch/chQcResultDetailsDlg.h"
#include "im/imqcitemrltdetaildlg.h"
#include "src/common/TimeUtil.h"
#include "src/common/Mlog/mlog.h"
#include "manager/UserInfoManager.h"
#include "Serialize.h"
#include "printcom.hpp"
#include "qcprintsetdlg.h"


#define  DEFAULT_ROW_CNT_OF_QC_INFO_TABLE                (20)          // 质控信息表默认行数
#define  DEFAULT_COL_CNT_OF_QC_INFO_TABLE                (15)          // 质控信息表默认列数

#define  COL_INDEX_GRAPHIC_INDEX_OF_QC_INFO_TABLE        (0)           // 质控信息表图表列索引
#define  COL_INDEX_CALCUL_SD_OF_QC_INFO_TABLE            (11)          // 质控信息表计算SD列索引
#define  COL_INDEX_NUM_OF_QC_INFO_TABLE                  (13)          // 质控信息表数量列索引
#define  COL_INDEX_DB_NO_OF_QC_INFO_TABLE                (14)          // 质控信息表数据库主键列索引

#define  DEFAULT_ROW_CNT_OF_QC_RESULT_TABLE              (20)          // 质控结果表默认行数
#define  DEFAULT_COL_CNT_OF_QC_RESULT_TABLE              (10)          // 质控结果表默认列数

#define  COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE          (0)           // 质控结果表是否计算列索引
#define  COL_INDEX_QC_TIME_OF_QC_RESULT_TABLE            (2)           // 质控结果表质控时间列索引
#define  COL_INDEX_STATE_OF_QC_RESULT_TABLE              (4)           // 质控结果表状态列索引
#define  COL_INDEX_DB_NO_OF_QC_RESULT_TABLE              (9)           // 质控结果表数据库主键列索引

#define  COMBO_INDEX_OF_GRAPHIC_EMPTY                    (0)           // 下拉框空图表索引
#define  COMBO_INDEX_OF_GRAPHIC_1                        (1)           // 下拉框图表1索引
#define  COMBO_INDEX_OF_GRAPHIC_2                        (2)           // 下拉框图表2索引
#define  COMBO_INDEX_OF_GRAPHIC_3                        (3)           // 下拉框图表3索引
#define  COMBO_INDEX_OF_GRAPHIC_4                        (4)           // 下拉框图表3索引

#define  TAB_INDEX_OF_GRAPHIC_1                          (0)           // tab窗口图表1索引
#define  TAB_INDEX_OF_GRAPHIC_2                          (1)           // tab窗口图表2索引
#define  TAB_INDEX_OF_GRAPHIC_3                          (2)           // tab窗口图表3索引
#define  TAB_INDEX_OF_GRAPHIC_4                          (3)           // tab窗口图表3索引

#define  X_SD_CV_PRECISION                               (3)           // 靶值标准差精度

#define  DEFAULT_COL_CNT_OF_QC_LJ_RLT_PRINT_TABLE        (3)           // 打印表默认列数

Q_DECLARE_METATYPE(QC_DOC_CONC_INFO)
Q_DECLARE_METATYPE(QC_RESULT_INFO)

GraphComboBox::GraphComboBox(QWidget *parent /*= Q_NULLPTR*/)
	: QComboBox(parent)
{
	// 默认选择框第一项为空
	addItem("");
}

GraphComboBox::~GraphComboBox()
{

}

void GraphComboBox::paintEvent(QPaintEvent *e)
{
	QComboBox::paintEvent(e);

	// 当选择第一项时，设置默认字体
	if (currentIndex() == 0) {
		QPainter painter(this);
		painter.setPen(Qt::gray); // 灰色
		painter.setFont(font());
		painter.drawText(rect().x()+10, rect().y(), rect().width(), rect().height(), Qt::AlignLeft | Qt::AlignVCenter, tr("请选择")); // 绘制灰色文本
	}

}


QcGraphLJWidget::QcGraphLJWidget(QWidget *parent)
    : QWidget(parent),
      m_bInit(false)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化UI对象
    ui = new Ui::QcGraphLJWidget();
    ui->setupUi(this);

    // 界面显示前初始化
    InitBeforeShow();

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

QcGraphLJWidget::~QcGraphLJWidget()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     重绘控件视图
///
///
/// @return 
///
/// @par History:
/// @li 5220/SunChangYan，2021年11月5日，新建函数
///
// void QcGraphLJWidget::RepaintCtrlViews()
// {
//     // 获取当前登陆用户信息
//     USER_INFO useInfo = Hazel::GetCurUser();
//     switch (useInfo.usrType)
//     {
//     case USERTYPE::USERTYPE_SUPER_DEVELOPER:
//     case USERTYPE::USERTYPE_ENGINEER:
//     case USERTYPE::USERTYPE_ADMINISTRATOR:
//     {
//         ui->TargetValUpdateBtn->setVisible(true);
//         ui->QcResultTbl1->showColumn(COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE);
//         ui->QcResultTbl2->showColumn(COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE);
//         ui->QcResultTbl3->showColumn(COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE);
//         ui->QcResultTbl4->showColumn(COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE);
//         ui->QcGraphic->ui->ShowNoCalcQcCB->setVisible(true);
//     }
//     break;
//     case USERTYPE::USERTYPE_OPERATOR:
//     {
//         ui->TargetValUpdateBtn->setVisible(false);
//         ui->QcResultTbl1->hideColumn(COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE);
//         ui->QcResultTbl2->hideColumn(COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE);
//         ui->QcResultTbl3->hideColumn(COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE);
//         ui->QcResultTbl4->hideColumn(COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE);
//         ui->QcGraphic->ui->ShowNoCalcQcCB->setVisible(false);
//     }
//     break;
//     default:
//         break;
//     }
// }

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月8日，新建函数
///
void QcGraphLJWidget::InitBeforeShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化质控LJ打印表
    //InitPrintTbl();

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

    // 添加按钮组
    QButtonGroup *pBtnGrp = new QButtonGroup(this);
    pBtnGrp->addButton(ui->QcXDayBtn);
    pBtnGrp->addButton(ui->QcXCntBtn);
    pBtnGrp->setExclusive(true);

    // 绑定起止日期编辑框
    BindQcDateEdit(ui->QcStartDateEdit, ui->QcEndDateEdit);

    // 初始化日期控件
    ui->QcStartDateEdit->setCalendarPopup(true);
    ui->QcEndDateEdit->setCalendarPopup(true);

    // 质控信息表
    // 设置默认行数和列数
    //ui->QcInfoTable->setRowCount(DEFAULT_ROW_CNT_OF_QC_INFO_TABLE);
    ui->QcInfoTable->setColumnCount(DEFAULT_COL_CNT_OF_QC_INFO_TABLE);

    // 隐藏数据库主键列
    ui->QcInfoTable->hideColumn(COL_INDEX_DB_NO_OF_QC_INFO_TABLE);

    // 质控结果表
    m_pTbl1Model = new QStandardItemModel(this);
    m_pTbl2Model = new QStandardItemModel(this);
    m_pTbl3Model = new QStandardItemModel(this);
    m_pTbl4Model = new QStandardItemModel(this);
    //m_pTbl1Model->setRowCount(DEFAULT_ROW_CNT_OF_QC_RESULT_TABLE);
    m_pTbl1Model->setColumnCount(DEFAULT_COL_CNT_OF_QC_RESULT_TABLE);
    //m_pTbl2Model->setRowCount(DEFAULT_ROW_CNT_OF_QC_RESULT_TABLE);
    m_pTbl2Model->setColumnCount(DEFAULT_COL_CNT_OF_QC_RESULT_TABLE);
    //m_pTbl3Model->setRowCount(DEFAULT_ROW_CNT_OF_QC_RESULT_TABLE);
    m_pTbl3Model->setColumnCount(DEFAULT_COL_CNT_OF_QC_RESULT_TABLE);
    //m_pTbl4Model->setRowCount(DEFAULT_ROW_CNT_OF_QC_RESULT_TABLE);
    m_pTbl4Model->setColumnCount(DEFAULT_COL_CNT_OF_QC_RESULT_TABLE);
    ui->QcResultTbl1->setModel(m_pTbl1Model);
    ui->QcResultTbl2->setModel(m_pTbl2Model);
    ui->QcResultTbl3->setModel(m_pTbl3Model);
    ui->QcResultTbl4->setModel(m_pTbl4Model);
    ui->QcResultTbl1->hideColumn(COL_INDEX_DB_NO_OF_QC_RESULT_TABLE);
    ui->QcResultTbl2->hideColumn(COL_INDEX_DB_NO_OF_QC_RESULT_TABLE);
    ui->QcResultTbl3->hideColumn(COL_INDEX_DB_NO_OF_QC_RESULT_TABLE);
    ui->QcResultTbl4->hideColumn(COL_INDEX_DB_NO_OF_QC_RESULT_TABLE);

    // 为特定列设置状态代理
    ui->QcResultTbl1->setItemDelegateForColumn(COL_INDEX_STATE_OF_QC_RESULT_TABLE, new StatusItemDelegate(this));
    ui->QcResultTbl2->setItemDelegateForColumn(COL_INDEX_STATE_OF_QC_RESULT_TABLE, new StatusItemDelegate(this));
    ui->QcResultTbl3->setItemDelegateForColumn(COL_INDEX_STATE_OF_QC_RESULT_TABLE, new StatusItemDelegate(this));
    ui->QcResultTbl4->setItemDelegateForColumn(COL_INDEX_STATE_OF_QC_RESULT_TABLE, new StatusItemDelegate(this));

    // 日期代理
    ui->QcResultTbl1->setItemDelegateForColumn(COL_INDEX_QC_TIME_OF_QC_RESULT_TABLE, new CReadOnlyDelegate(this));
    ui->QcResultTbl2->setItemDelegateForColumn(COL_INDEX_QC_TIME_OF_QC_RESULT_TABLE, new CReadOnlyDelegate(this));
    ui->QcResultTbl3->setItemDelegateForColumn(COL_INDEX_QC_TIME_OF_QC_RESULT_TABLE, new CReadOnlyDelegate(this));
    ui->QcResultTbl4->setItemDelegateForColumn(COL_INDEX_QC_TIME_OF_QC_RESULT_TABLE, new CReadOnlyDelegate(this));

    // 打印模板数据请求
//     REGISTER_HANDLER(MSG_ID_GET_PRINT_TMP_VAR_LAB_TBL_VAL, this, OnReqPrintTmpVarData);
//     REGISTER_HANDLER(MSG_ID_GET_PRINT_TMP_VAR_TBL_HEADER_VAL, this, OnReqPrintTmpVarTblHeaderData);
//     REGISTER_HANDLER(MSG_ID_GET_PRINT_TMP_VAR_TBL_ITEM_VAL, this, OnReqPrintTmpVarTblItemData);
//     REGISTER_HANDLER(MSG_ID_GET_PRINT_TMP_VAR_TBL_ROW_COL_CNT_VAL, this, OnReqPrintTmpVarTblRowColCnt);
//     REGISTER_HANDLER(MSG_ID_GET_PRINT_TMP_SERIES_PRINT_CNT, this, OnReqPrintTmpSeriesPrintCnt);
//     REGISTER_HANDLER(MSG_ID_GET_PRINT_TMP_VAR_IMG_VAL, this, OnReqPrintTmpVarImg);
}

///
/// @brief
///     显示之后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月8日，新建函数
///
void QcGraphLJWidget::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化字符串资源
    InitStrResource();

    // 初始化信号槽连接
    InitConnect();

    // 初始化子控件
    InitChildCtrl();

    // 重绘用户视图
/*    RepaintCtrlViews();*/

	// 重绘质控结果表
	UpdateQcResult();
}

///
/// @brief
///     初始化字符串资源
///
/// @par History:
/// @li 4170/TangChuXian，2022年1月17日，新建函数
///
void QcGraphLJWidget::InitStrResource()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
//     ui->AssayLab->setText(LoadStrFromLanguage(CHAR_CODE::IDS_PRINT_TYPE_ASSAY));
//     ui->QcDateLab->setText(LoadStrFromLanguage(CHAR_CODE::IDS_QC_DATE));
//     ui->DetailBtn->setText(LoadStrFromLanguage(CHAR_CODE::IDS_TITLE_RESULT_DETAIL));
//     ui->PrintBtn->setText(LoadStrFromLanguage(CHAR_CODE::IDS_MAINT_PRINT));
//     ui->OutCtrlBtn->setText(LoadStrFromLanguage(CHAR_CODE::IDS_QC_OUT_CTRL_HANDLE));
//     ui->TargetValUpdateBtn->setText(LoadStrFromLanguage(CHAR_CODE::IDS_BTN_UPDATE_SD));
//     ui->QcResultTabWidget->setTabText(ui->QcResultTabWidget->indexOf(ui->QcResultTbl1Widget), LoadStrFromLanguage(CHAR_CODE::IDS_QC_GRAPHIC_1));
//     ui->QcResultTabWidget->setTabText(ui->QcResultTabWidget->indexOf(ui->QcResultTbl2Widget), LoadStrFromLanguage(CHAR_CODE::IDS_QC_GRAPHIC_2));
//     ui->QcResultTabWidget->setTabText(ui->QcResultTabWidget->indexOf(ui->QcResultTbl3Widget), LoadStrFromLanguage(CHAR_CODE::IDS_QC_GRAPHIC_3));
//     ui->QcResultTabWidget->setTabText(ui->QcResultTabWidget->indexOf(ui->QcResultTbl4Widget), LoadStrFromLanguage(CHAR_CODE::IDS_QC_GRAPHIC_4));
// 
//     // 判断是日内质控还是日间质控
//     if (UI::QC_GRAPHIC_LJ_AXIS_X(ui->QcTypeCombo->currentData().toInt()) == UI::QC_GRAPHIC_LJ_AXIS_X_DAY)
//     {
//         // 横坐标为日间质控
//         ui->QcGraphic->ui->XAxisNameLab->setText(LoadStrFromLanguage(CHAR_CODE::IDS_QC_DAY_COUNT));
//     }
//     else if (UI::QC_GRAPHIC_LJ_AXIS_X(ui->QcTypeCombo->currentData().toInt()) == UI::QC_GRAPHIC_LJ_AXIS_X_COUNT)
//     {
//         // 横坐标为日内质控
//         ui->QcGraphic->ui->XAxisNameLab->setText(LoadStrFromLanguage(CHAR_CODE::IDS_QC_TIMES_COUNT));
//     }
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月8日，新建函数
///
void QcGraphLJWidget::InitChildCtrl()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 更新文本和图标
    ui->PopBtn->setText(tr("展开"));
    ui->PopBtn->setProperty("pop_status", false);
    ui->PopBtn->setIcon(QIcon(":/Leonis/resource/image/icon-pop-push-down.png"));

    // 失控处理按钮禁能
    ui->TargetValUpdateBtn->setEnabled(false);
    ui->OutCtrlBtn->setEnabled(false);
    ui->DetailBtn->setEnabled(false);
	ui->PrintBtn->setEnabled(false);
	ui->export_btn->setEnabled(false);

    // 质控信息表
    // 设置表头
    QStringList strQcInfoHeaderList;
    strQcInfoHeaderList << tr("选择") << tr("质控品编号") << tr("质控品名称") << tr("质控品简称") << tr("质控品类型") << tr("质控品水平") << tr("质控品批号") << tr("靶值") << tr("SD") << ("CV%") << tr("计算靶值") << tr("计算SD") << tr("计算CV%") << tr("数量") << "db_no";
    ui->QcInfoTable->setHorizontalHeaderLabels(strQcInfoHeaderList);

    // 设置表格选中模式为行选中，不可多选，不可编辑, 尾列拉伸，不可拉伸
    ui->QcInfoTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->QcInfoTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->QcInfoTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->QcInfoTable->horizontalHeader()->setStretchLastSection(true);
/*	ui->QcInfoTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);*/

    // 隐藏垂直表头
    ui->QcInfoTable->verticalHeader()->setVisible(false);

	// 设置表格列宽
	ui->QcInfoTable->setColumnWidth(0, 95);
	ui->QcInfoTable->setColumnWidth(1, 120);
	ui->QcInfoTable->setColumnWidth(2, 130);
    ui->QcInfoTable->setColumnWidth(3, 85);
	ui->QcInfoTable->setColumnWidth(4, 90);
	ui->QcInfoTable->setColumnWidth(5, 90);
	ui->QcInfoTable->setColumnWidth(6, 90);
	ui->QcInfoTable->setColumnWidth(7, 70);
	ui->QcInfoTable->setColumnWidth(8, 70);
	ui->QcInfoTable->setColumnWidth(9, 70);
	ui->QcInfoTable->setColumnWidth(10, 70);
	ui->QcInfoTable->setColumnWidth(11, 70);
	ui->QcInfoTable->setColumnWidth(12, 70);
	ui->QcInfoTable->setColumnWidth(13, 70);
	ui->QcInfoTable->horizontalHeader()->setStretchLastSection(true);

	// 选择框固定，不可调整
	ui->QcInfoTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);

    // 列宽自适应
    //ResizeTblColToContent(ui->QcInfoTable);

    // 设置表格列宽比
	// QVector<double> vQcInfoColWidScale;
	// vQcInfoColWidScale << 1.2 << 1.1 << 1.1 << 1.1 << 0.9 << 0.9 << 1.0 << 1.0 << 1.1 << 1.1 << 0.5;
	// SetTblColWidthScale(ui->QcInfoTable, vQcInfoColWidScale);

    // 质控结果表
    // 设置表头
    ui->QcResultTabWidget->setCurrentIndex(TAB_INDEX_OF_GRAPHIC_4);
    ui->QcResultTabWidget->setCurrentIndex(TAB_INDEX_OF_GRAPHIC_3);
    ui->QcResultTabWidget->setCurrentIndex(TAB_INDEX_OF_GRAPHIC_2);
    ui->QcResultTabWidget->setCurrentIndex(TAB_INDEX_OF_GRAPHIC_1);
    QStringList strQcResHeaderList;
    strQcResHeaderList << tr("计算") << tr("序号") << tr("质控时间") << tr("结果") << tr("状态") << tr("失控规则") << tr("靶值") << tr("标准差") << tr("操作者");
    m_pTbl1Model->setHorizontalHeaderLabels(strQcResHeaderList);
    m_pTbl2Model->setHorizontalHeaderLabels(strQcResHeaderList);
    m_pTbl3Model->setHorizontalHeaderLabels(strQcResHeaderList);
    m_pTbl4Model->setHorizontalHeaderLabels(strQcResHeaderList);
    m_pTbl1Model->horizontalHeaderItem(COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE)->setData(UI_TEXT_COLOR_HEADER_MARK, Qt::TextColorRole);
    m_pTbl2Model->horizontalHeaderItem(COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE)->setData(UI_TEXT_COLOR_HEADER_MARK, Qt::TextColorRole);
    m_pTbl3Model->horizontalHeaderItem(COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE)->setData(UI_TEXT_COLOR_HEADER_MARK, Qt::TextColorRole);
    m_pTbl4Model->horizontalHeaderItem(COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE)->setData(UI_TEXT_COLOR_HEADER_MARK, Qt::TextColorRole);

    // 设置代理
    ui->QcResultTbl1->setItemDelegateForColumn(COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE, new McPixmapItemDelegate(ui->QcResultTbl1));
    ui->QcResultTbl2->setItemDelegateForColumn(COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE, new McPixmapItemDelegate(ui->QcResultTbl2));
    ui->QcResultTbl3->setItemDelegateForColumn(COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE, new McPixmapItemDelegate(ui->QcResultTbl3));
    ui->QcResultTbl4->setItemDelegateForColumn(COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE, new McPixmapItemDelegate(ui->QcResultTbl4));

    // 设置无焦点
    ui->QcResultTbl1->setFocusPolicy(Qt::NoFocus);
    ui->QcResultTbl2->setFocusPolicy(Qt::NoFocus);
    ui->QcResultTbl3->setFocusPolicy(Qt::NoFocus);
    ui->QcResultTbl4->setFocusPolicy(Qt::NoFocus);

    // 隐藏垂直表头
    ui->QcResultTbl1->verticalHeader()->setVisible(false);
    ui->QcResultTbl2->verticalHeader()->setVisible(false);
    ui->QcResultTbl3->verticalHeader()->setVisible(false);
    ui->QcResultTbl4->verticalHeader()->setVisible(false);

    // 设置表格选中模式为行选中，不可多选，不可编辑
    ui->QcResultTbl1->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->QcResultTbl1->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->QcResultTbl1->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->QcResultTbl2->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->QcResultTbl2->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->QcResultTbl2->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->QcResultTbl3->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->QcResultTbl3->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->QcResultTbl3->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->QcResultTbl4->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->QcResultTbl4->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->QcResultTbl4->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 设置列宽适应内容
//     ui->QcResultTbl1->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
//     ui->QcResultTbl2->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
//     ui->QcResultTbl3->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
//     ui->QcResultTbl4->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    // 最小列宽
    // 根据内容调整列宽
    ResizeTblColToContent(ui->QcResultTbl1);
    ResizeTblColToContent(ui->QcResultTbl2);
    ResizeTblColToContent(ui->QcResultTbl3);
    ResizeTblColToContent(ui->QcResultTbl4);
	ResizeQcResultTableIconColumn(ui->QcResultTbl1);
	ResizeQcResultTableIconColumn(ui->QcResultTbl2);
	ResizeQcResultTableIconColumn(ui->QcResultTbl3);
	ResizeQcResultTableIconColumn(ui->QcResultTbl4);

    // 设置表格列宽比
//     QVector<double> vQcResColWidScale;
//     vQcResColWidScale << 1.0 << 1.0 << 1.0 << 1.0 << 1.0;
//     SetTblColWidthScale(ui->QcResultTbl1, vQcResColWidScale);
//     SetTblColWidthScale(ui->QcResultTbl2, vQcResColWidScale);
//     SetTblColWidthScale(ui->QcResultTbl3, vQcResColWidScale);

    // 更新设备名
    UpdateDevNameList();

    // 更新项目列表
    UpdateAssayNameList();

    // 更新质控信息
    UpdateQcInfo();

    // 更新质控结果
    UpdateQcResult();

    // 更新质控图
    UpdateQcGraphic();
}

///
/// @brief
///     初始化连接
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月8日，新建函数
///
void QcGraphLJWidget::InitConnect()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 质控查询条件改变
	connect(ui->AssayCombo, &QFilterComboBox::currentTextChanged, this, [this](const QString &text) {
		// 已经清空过，则不清空
		if (m_mapGraphicComboBuffer.isEmpty() && ui->AssayCombo->findText(text) == -1)
		{
			return;
		}
		OnQcCondChanged();
	});
    connect(ui->QcStartDateEdit, SIGNAL(dateChanged(const QDate&)), this, SLOT(OnQcCondChanged()));
    connect(ui->QcEndDateEdit, SIGNAL(dateChanged(const QDate&)), this, SLOT(OnQcCondChanged()));

    // 展开按钮被点击
    connect(ui->PopBtn, SIGNAL(clicked()), this, SLOT(OnPopBtnClicked()));

    // 质控图类型改变
    connect(ui->QcXDayBtn, SIGNAL(clicked()), this, SLOT(OnQcGraphicTypeChanged()), Qt::QueuedConnection);
    connect(ui->QcXCntBtn, SIGNAL(clicked()), this, SLOT(OnQcGraphicTypeChanged()), Qt::QueuedConnection);

    // 质控结果表细节按钮被点击
    connect(ui->DetailBtn, SIGNAL(clicked()), this, SLOT(OnDetailBtnClicked()));

    // 失控处理按钮被点击
    connect(ui->OutCtrlBtn, SIGNAL(clicked()), this, SLOT(OnOutCtrlBtnClicked()));

	// 质控信息表被选中
    QItemSelectionModel* selItemQcInfoModel = ui->QcInfoTable->selectionModel();
    if (selItemQcInfoModel != Q_NULLPTR)
    {
        connect(selItemQcInfoModel, SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(OnQcInfoTableItemSelChanged(const QModelIndex&, const QModelIndex&)));
    }

    // 更新靶值SD按钮被点击
    connect(ui->TargetValUpdateBtn, SIGNAL(clicked()), this, SLOT(OnTargetValUpdateBtnClicked()));

    // 打印按钮被点击
    connect(ui->PrintBtn, SIGNAL(clicked()), this, SLOT(OnPrintBtnClicked()));

    // 导出按钮被点击
    connect(ui->export_btn, SIGNAL(clicked()), this, SLOT(OnExportBtnClicked()));

    // 质控结果表单元格被点击
    connect(ui->QcResultTbl1, SIGNAL(clicked(const QModelIndex&)), this, SLOT(OnQcResultTblItemClicked(const QModelIndex&)));
    connect(ui->QcResultTbl2, SIGNAL(clicked(const QModelIndex&)), this, SLOT(OnQcResultTblItemClicked(const QModelIndex&)));
    connect(ui->QcResultTbl3, SIGNAL(clicked(const QModelIndex&)), this, SLOT(OnQcResultTblItemClicked(const QModelIndex&)));
    connect(ui->QcResultTbl4, SIGNAL(clicked(const QModelIndex&)), this, SLOT(OnQcResultTblItemClicked(const QModelIndex&)));

    // 质控结果表选中项索引改变
    QItemSelectionModel* selItemModel1 = ui->QcResultTbl1->selectionModel();
    QItemSelectionModel* selItemModel2 = ui->QcResultTbl2->selectionModel();
    QItemSelectionModel* selItemModel3 = ui->QcResultTbl3->selectionModel();
    QItemSelectionModel* selItemModel4 = ui->QcResultTbl4->selectionModel();
    connect(ui->QcResultTabWidget, SIGNAL(currentChanged(int)), this, SLOT(OnQcResultItemSelChanged()));
    if ((selItemModel1 != Q_NULLPTR) && (selItemModel2 != Q_NULLPTR) && (selItemModel3 != Q_NULLPTR) && (selItemModel4 != Q_NULLPTR))
    {
        connect(selItemModel1, SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(OnQcResultItemSelChanged()));
        connect(selItemModel2, SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(OnQcResultItemSelChanged()));
        connect(selItemModel3, SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(OnQcResultItemSelChanged()));
        connect(selItemModel4, SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(OnQcResultItemSelChanged()));
    }

    // LJ图显示不计算点或显示子点复选框改变
    connect(ui->QcGraphic->ui->ShowNoCalcQcCB, SIGNAL(clicked()), this, SLOT(OnQcGraphicShowPointCBClicked()));
    connect(ui->QcGraphic->ui->ShowSubPointCB, SIGNAL(clicked()), this, SLOT(OnQcGraphicShowPointCBClicked()));

    // 监听质控图选中点改变
    connect(ui->QcGraphic, SIGNAL(SigSelPointChanged(McQcLjGraphic::CURVE_ID, long long)), this, SLOT(OnQcGraphicSelPtChanged(McQcLjGraphic::CURVE_ID, long long)));

    // 下拉框缓存
    connect(ui->QcInfoTable->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(OnQcInfoTblDisplayRangeChanged()));
    connect(ui->QcInfoTable->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(UpdateQcInfoCombo()));
    connect(ui->PopBtn, SIGNAL(clicked()), this, SLOT(OnQcInfoTblDisplayRangeChanged()));
    connect(ui->PopBtn, SIGNAL(clicked()), this, SLOT(UpdateQcInfoCombo()));

    // 通知质控图刷新(用于调试)
//     REGISTER_HANDLER(MSG_ID_QC_GRAPH_UPDATE, this, UpdateAllQcInfo);
// 
//     // 监听质控结果更新
//     REGISTER_HANDLER(MSG_ID_QC_ADD_RESULT, this, OnQcResultUpdate);
// 
//     // 监听质控信息更新
//     REGISTER_HANDLER(MSG_ID_QC_ITEM_UPDATE, this, UpdateAllQcInfo);
// 
//     // 监听登陆用户变化
//     REGISTER_HANDLER(MSG_ID_LOGIN_USERINFO_CHANGED, this, RepaintCtrlViews);
// 
//     // 监听结果单位更新
//     REGISTER_HANDLER(MSG_ID_ASSAY_PARAM_UPDATE, this, UpdateAllQcInfo);
// 
//     // 项目信息更新
//     REGISTER_HANDLER(MSG_ID_UTILITY_ASSAY_UPDATE, this, OnAssayInfoUpdate);
//     REGISTER_HANDLER(MSG_ID_UTILITY_ASSAY_DELETE, this, OnAssayInfoUpdate);

    // 监听质控品信息更新
    REGISTER_HANDLER(MSG_ID_QC_DOC_INFO_UPDATE, this, OnQcDocInfoInfoUpdate);

    // 项目信息更新
    REGISTER_HANDLER(MSG_ID_ASSAY_CODE_MANAGER_UPDATE, this, OnAssayInfoUpdate);

    // 角落按钮索引更新
    REGISTER_HANDLER(MSG_ID_QC_TAB_CORNER_INDEX_UPDATE, this, OnTabCornerIndexChanged);

    // 质控结果更新
    REGISTER_HANDLER(MSG_ID_QC_RESULT_UPDATE, this, OnQcRltUpdate);
}

///
/// @brief
///     初始化打印表
///
/// @par History:
/// @li 4170/TangChuXian，2022年7月7日，新建函数
///
void QcGraphLJWidget::InitPrintTbl()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化打印表
    m_pPrintTblQcLjResult = new QStandardItemModel(this);

    // 设置默认列数
    m_pPrintTblQcLjResult->setRowCount(0);
    m_pPrintTblQcLjResult->setColumnCount(DEFAULT_COL_CNT_OF_QC_LJ_RLT_PRINT_TABLE);

    // 设置表头
//     QStringList strHeaderList;
//     strHeaderList << LoadStrFromLanguage(CHAR_CODE::IDS_NO) << LoadStrFromLanguage(CHAR_CODE::IDS_DATE) << LoadStrFromLanguage(CHAR_CODE::IDS_RLT);
//     m_pPrintTblQcLjResult->setHorizontalHeaderLabels(strHeaderList);
}

///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月8日，新建函数
///
void QcGraphLJWidget::showEvent(QShowEvent *event)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 让基类处理事件
    QWidget::showEvent(event);

    // 更新菜单位置
    POST_MESSAGE(MSG_ID_CURRENT_MODEL_NAME, QString("> ") + tr("质控") + QString(" > ") + tr("Levey-Jennings图"));

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

	// 更新质控图(日期格式更新)
	UpdateQcGraphic();
}

///
/// @brief
///     更新设备列表
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月19日，新建函数
///
void QcGraphLJWidget::UpdateDevNameList()
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

    // 通知设备列表更新(显示组的)
    POST_MESSAGE(MSG_ID_QC_TAB_CORNER_DISPLAY_UPDATE, m_strDevNameList, m_strDevNameList.indexOf(m_strCurDevGroupName.isEmpty() ? m_strCurDevName : m_strCurDevGroupName));
}

///
/// @brief
///     更新质控信息
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月9日，新建函数
///
void QcGraphLJWidget::UpdateQcInfo()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 记录原图表选项
    QMap<QString, int> mapItemComboIndex;
    // 遍历质控信息表，加载对应质控结果表
    for (int iRow = 0; iRow < ui->QcInfoTable->rowCount(); iRow++)
    {
        // 获取对应项
        QTableWidgetItem* pItem = ui->QcInfoTable->item(iRow, COL_INDEX_DB_NO_OF_QC_INFO_TABLE);
        if (pItem == Q_NULLPTR)
        {
            continue;
        }

        // 获取对应下拉框
        QComboBox* pCombo = qobject_cast<QComboBox*>(ui->QcInfoTable->cellWidget(iRow, COL_INDEX_GRAPHIC_INDEX_OF_QC_INFO_TABLE));
        if (pCombo == Q_NULLPTR)
        {
            continue;
        }

        // 如果对应图表为空，则跳过
        if (pCombo->currentData().toInt() == COMBO_INDEX_OF_GRAPHIC_EMPTY)
        {
            continue;
        }

        // 获取对应质控信息
        QC_DOC_CONC_INFO stuQcInfo = ui->QcInfoTable->item(iRow, COL_INDEX_DB_NO_OF_QC_INFO_TABLE)->data(Qt::UserRole).value<QC_DOC_CONC_INFO>();

        // 如果数据库小于0则跳过(待完善)
        if (stuQcInfo.strID.isEmpty())
        {
            continue;
        }

        // 记录项目图表映射
        mapItemComboIndex.insert(stuQcInfo.strID, pCombo->currentData().toInt());
    }

    // 清空表格内容
    ClearQcInfoCombo();
    ui->QcInfoTable->clearContents();

    // 初始化质控信息和查询条件
    QList<QC_DOC_CONC_INFO>     qryResp;
    QC_CONC_INFO_QUERY_COND     qryCond;

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
    // 设置起止日期
    qryCond.startDate = ui->QcStartDateEdit->date();
    qryCond.endDate = ui->QcEndDateEdit->date();
    qryCond.strAssayName = strCurAssayName;

    // 执行查询
    if (!gUiAdapterPtr(m_strCurDevName, m_strCurDevGroupName, iModuleNo)->QueryQcDocConcInfo(qryCond, qryResp))
    {
        ULOG(LOG_WARN, "QueryQcDocConcInfo failed.");
        ui->QcInfoTable->setRowCount(0);
        return;
    }

    // 调整行数
    ui->QcInfoTable->setRowCount(qryResp.size());

	// 展示精度
	int precisonNum = CommonInformationManager::GetInstance()->GetPrecisionNum(strCurAssayName.toStdString());

    // 把质控信息填充到表格中
    int iRow = 0;
    for (auto& stuQcInfo : qryResp)
    {
        // 行数自增
        if (iRow >= ui->QcInfoTable->rowCount())
        {
            ui->QcInfoTable->setRowCount(iRow + 1);
        }

        // 更新单元项
        ui->QcInfoTable->setItem(iRow, 0, new QTableWidgetItem(""));                                // 图表项
        ui->QcInfoTable->setItem(iRow, 1, new QTableWidgetItem(stuQcInfo.strQcNo));                 // 质控品编号
        ui->QcInfoTable->setItem(iRow, 2, new QTableWidgetItem(stuQcInfo.strQcName));               // 质控品名称
        ui->QcInfoTable->setItem(iRow, 3, new QTableWidgetItem(stuQcInfo.strQcBriefName));          // 质控品简称
        ui->QcInfoTable->setItem(iRow, 4, new QTableWidgetItem(stuQcInfo.strQcSourceType));         // 质控品类型
        ui->QcInfoTable->setItem(iRow, 5, new QTableWidgetItem(stuQcInfo.strQcLevel));              // 质控品水平
        ui->QcInfoTable->setItem(iRow, 6, new QTableWidgetItem(stuQcInfo.strQcLot));                // 质控品批号
		ui->QcInfoTable->setItem(iRow, 7, new QTableWidgetItem(QString::number(stuQcInfo.dQcTargetVal, 'f', precisonNum)));          // 靶值
		ui->QcInfoTable->setItem(iRow, 8, new QTableWidgetItem(QString::number(stuQcInfo.dQcSD, 'f', precisonNum)));                 // SD
		ui->QcInfoTable->setItem(iRow, 9, new QTableWidgetItem(QString::number(stuQcInfo.dQcCV, 'f', precisonNum)));                 // CV%
		ui->QcInfoTable->setItem(iRow, 10, new QTableWidgetItem(QString::number(stuQcInfo.dQcCalcTargetVal, 'f', precisonNum)));     // 计算靶值
		ui->QcInfoTable->setItem(iRow, 11, new QTableWidgetItem(QString::number(stuQcInfo.dQcCalcSD, 'f', precisonNum)));            // 计算SD
		ui->QcInfoTable->setItem(iRow, 12, new QTableWidgetItem(QString::number(stuQcInfo.dQcCalcCV, 'f', precisonNum)));            // 计算CV%
        ui->QcInfoTable->setItem(iRow, 13, new QTableWidgetItem(stuQcInfo.strQcRltCount));          // 数量
        ui->QcInfoTable->setItem(iRow, 14, new QTableWidgetItem(stuQcInfo.strID));                  // 数据库主键

        // 添加数据
        ui->QcInfoTable->item(iRow, COL_INDEX_DB_NO_OF_QC_INFO_TABLE)->setData(Qt::UserRole, QVariant::fromValue<QC_DOC_CONC_INFO>(stuQcInfo));

        // 第一列插入下拉框
		GraphComboBox* pQcGraphicCombo = new GraphComboBox();
        m_mapGraphicComboBuffer.insert(iRow, pQcGraphicCombo);
		pQcGraphicCombo->setItemData(0, COMBO_INDEX_OF_GRAPHIC_EMPTY);
        pQcGraphicCombo->addItem(tr("图表1"), COMBO_INDEX_OF_GRAPHIC_1);
        pQcGraphicCombo->addItem(tr("图表2"), COMBO_INDEX_OF_GRAPHIC_2);
        pQcGraphicCombo->addItem(tr("图表3"), COMBO_INDEX_OF_GRAPHIC_3);
        pQcGraphicCombo->addItem(tr("图表4"), COMBO_INDEX_OF_GRAPHIC_4);
        //pQcGraphicCombo->SetTextAlignment(Qt::AlignCenter);
        //ui->QcInfoTable->setCellWidget(iRow, COL_INDEX_GRAPHIC_INDEX_OF_QC_INFO_TABLE, pQcGraphicCombo);

        // 原记录为空，前四行默认为图表1、图表2、图表3，图表4
        if (mapItemComboIndex.isEmpty())
        {
            // 默认为空
            pQcGraphicCombo->setCurrentIndex(pQcGraphicCombo->findData(COMBO_INDEX_OF_GRAPHIC_EMPTY));
			
//             if (iRow == 0)
//             {
//                 pQcGraphicCombo->setCurrentIndex(pQcGraphicCombo->findData(COMBO_INDEX_OF_GRAPHIC_1));
//             }
//             else if (iRow == 1)
//             {
//                 pQcGraphicCombo->setCurrentIndex(pQcGraphicCombo->findData(COMBO_INDEX_OF_GRAPHIC_2));
//             }
//             else if (iRow == 2)
//             {
//                 pQcGraphicCombo->setCurrentIndex(pQcGraphicCombo->findData(COMBO_INDEX_OF_GRAPHIC_3));
//             }
//             else if (iRow == 3)
//             {
//                 pQcGraphicCombo->setCurrentIndex(pQcGraphicCombo->findData(COMBO_INDEX_OF_GRAPHIC_4));
//             }
        }
        else
        {
            auto it = mapItemComboIndex.find(stuQcInfo.strID);
            if (it != mapItemComboIndex.end())
            {
                pQcGraphicCombo->setCurrentIndex(pQcGraphicCombo->findData(it.value()));
            }
        }

        // 连接信号槽
        connect(pQcGraphicCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(OnGraphicComboChanged()));

        // 行号自增
        ++iRow;
    }

    // 更新下拉框
    OnQcInfoTblDisplayRangeChanged();
    UpdateQcInfoCombo();

    // 表格内容居中
    SetTblTextAlign(ui->QcInfoTable, Qt::AlignCenter);

    // 表格列宽自适应
    //ResizeTblColToContent(ui->QcInfoTable);

    // 更新靶值SD按钮使能状态更新
    UpdateTargetValSDBtnEnable();
}

///
/// @brief
///     切换项目导致更新质控信息
///
/// @par History:
/// @li 4170/TangChuXian，2021年11月21日，新建函数
///
void QcGraphLJWidget::UpdateQcInfoByAssayChanged()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 清空表格内容
    ClearQcInfoCombo();
    ui->QcInfoTable->clearContents();

    // 初始化质控信息和查询条件
    QList<QC_DOC_CONC_INFO>     qryResp;
    QC_CONC_INFO_QUERY_COND     qryCond;

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
    // 设置起止日期
    qryCond.startDate = ui->QcStartDateEdit->date();
    qryCond.endDate = ui->QcEndDateEdit->date();
    qryCond.strAssayName = strCurAssayName;

    // 执行查询
    if (!gUiAdapterPtr(m_strCurDevName, m_strCurDevGroupName, iModuleNo)->QueryQcDocConcInfo(qryCond, qryResp))
    {
        ULOG(LOG_WARN, "QueryQcDocConcInfo failed.");
        ui->QcInfoTable->setRowCount(0);
        return;
    }

    // 调整行数
    ui->QcInfoTable->setRowCount(qryResp.size());

	// 展示精度
	int precisonNum = CommonInformationManager::GetInstance()->GetPrecisionNum(strCurAssayName.toStdString());

    // 把质控信息填充到表格中
    int iRow = 0;
    for (auto& stuQcInfo : qryResp)
    {
        // 行数自增
        if (iRow >= ui->QcInfoTable->rowCount())
        {
            ui->QcInfoTable->setRowCount(iRow + 1);
        }

        // 更新单元项
        ui->QcInfoTable->setItem(iRow, 0, new QTableWidgetItem(""));                                // 图表项
        ui->QcInfoTable->setItem(iRow, 1, new QTableWidgetItem(stuQcInfo.strQcNo));                 // 质控品编号
        ui->QcInfoTable->setItem(iRow, 2, new QTableWidgetItem(stuQcInfo.strQcName));               // 质控品名称
        ui->QcInfoTable->setItem(iRow, 3, new QTableWidgetItem(stuQcInfo.strQcBriefName));          // 质控品简称
        ui->QcInfoTable->setItem(iRow, 4, new QTableWidgetItem(stuQcInfo.strQcSourceType));         // 质控品类型
        ui->QcInfoTable->setItem(iRow, 5, new QTableWidgetItem(stuQcInfo.strQcLevel));              // 质控品水平
        ui->QcInfoTable->setItem(iRow, 6, new QTableWidgetItem(stuQcInfo.strQcLot));                // 质控品批号
        ui->QcInfoTable->setItem(iRow, 7, new QTableWidgetItem(QString::number(stuQcInfo.dQcTargetVal, 'f', precisonNum)));          // 靶值
        ui->QcInfoTable->setItem(iRow, 8, new QTableWidgetItem(QString::number(stuQcInfo.dQcSD, 'f', precisonNum)));                 // SD
        ui->QcInfoTable->setItem(iRow, 9, new QTableWidgetItem(QString::number(stuQcInfo.dQcCV, 'f', precisonNum)));                 // CV%
        ui->QcInfoTable->setItem(iRow, 10, new QTableWidgetItem(QString::number(stuQcInfo.dQcCalcTargetVal, 'f', precisonNum)));     // 计算靶值
        ui->QcInfoTable->setItem(iRow, 11, new QTableWidgetItem(QString::number(stuQcInfo.dQcCalcSD, 'f', precisonNum)));            // 计算SD
        ui->QcInfoTable->setItem(iRow, 12, new QTableWidgetItem(QString::number(stuQcInfo.dQcCalcCV, 'f', precisonNum)));            // 计算CV%
        ui->QcInfoTable->setItem(iRow, 13, new QTableWidgetItem(stuQcInfo.strQcRltCount));          // 数量
        ui->QcInfoTable->setItem(iRow, 14, new QTableWidgetItem(stuQcInfo.strID));                  // 数据库主键

        // 添加数据
        ui->QcInfoTable->item(iRow, COL_INDEX_DB_NO_OF_QC_INFO_TABLE)->setData(Qt::UserRole, QVariant::fromValue<QC_DOC_CONC_INFO>(stuQcInfo));

        // 第一列插入下拉框
		GraphComboBox* pQcGraphicCombo = new GraphComboBox();
        m_mapGraphicComboBuffer.insert(iRow, pQcGraphicCombo);
		pQcGraphicCombo->setItemData(0,COMBO_INDEX_OF_GRAPHIC_EMPTY);
        pQcGraphicCombo->addItem(tr("图表1"), COMBO_INDEX_OF_GRAPHIC_1);
        pQcGraphicCombo->addItem(tr("图表2"), COMBO_INDEX_OF_GRAPHIC_2);
        pQcGraphicCombo->addItem(tr("图表3"), COMBO_INDEX_OF_GRAPHIC_3);
        pQcGraphicCombo->addItem(tr("图表4"), COMBO_INDEX_OF_GRAPHIC_4);
        //pQcGraphicCombo->SetTextAlignment(Qt::AlignCenter);
        pQcGraphicCombo->setCurrentIndex(pQcGraphicCombo->findData(COMBO_INDEX_OF_GRAPHIC_EMPTY));
        //ui->QcInfoTable->setCellWidget(iRow, COL_INDEX_GRAPHIC_INDEX_OF_QC_INFO_TABLE, pQcGraphicCombo);
		
//         if (iRow == 0)
//         {
//             pQcGraphicCombo->setCurrentIndex(pQcGraphicCombo->findData(COMBO_INDEX_OF_GRAPHIC_1));
//         }
//         else if (iRow == 1)
//         {
//             pQcGraphicCombo->setCurrentIndex(pQcGraphicCombo->findData(COMBO_INDEX_OF_GRAPHIC_2));
//         }
//         else if (iRow == 2)
//         {
//             pQcGraphicCombo->setCurrentIndex(pQcGraphicCombo->findData(COMBO_INDEX_OF_GRAPHIC_3));
//         }
//         else if (iRow == 3)
//         {
//             pQcGraphicCombo->setCurrentIndex(pQcGraphicCombo->findData(COMBO_INDEX_OF_GRAPHIC_4));
//         }

        // 连接信号槽
        connect(pQcGraphicCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(OnGraphicComboChanged()));

        // 行号自增
        ++iRow;
    }

    // 更新下拉框
    OnQcInfoTblDisplayRangeChanged();
    UpdateQcInfoCombo();

    // 表格内容居中
    SetTblTextAlign(ui->QcInfoTable, Qt::AlignCenter);

    // 表格列宽自适应
    //ResizeTblColToContent(ui->QcInfoTable);
}

///
/// @brief
///     更新项目名列表
///
/// @par History:
/// @li 4170/TangChuXian，2023年1月3日，新建函数
///
void QcGraphLJWidget::UpdateAssayNameList()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    auto AddAssayComboItems = [this](const QStringList& strAssayNameList)
    {
        // 遍历项目名列表
        ui->AssayCombo->addItems(strAssayNameList);
        for (const auto& strAssayName : strAssayNameList)
        {
            // 获取项目信息
            auto spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(strAssayName.toStdString());
            if (spAssayInfo == Q_NULLPTR)
            {
                continue;
            }

            // 获取索引
            int iIdx = ui->AssayCombo->findText(strAssayName);
            if (iIdx < 0)
            {
                continue;
            }

            // 添加到下拉框中
            ui->AssayCombo->setItemData(iIdx, spAssayInfo->assayCode);
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
    if (!gUiAdapterPtr(m_strCurDevName, m_strCurDevGroupName)->GetQcLjAssayList(strAssayNameList))
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
//     else
//     {
//         // 更新质控信息表
//         UpdateQcInfo();
// 
//         // 更新质控结果表
//         UpdateQcResult();
// 
//         // 更新质控图
//         UpdateQcGraphic();
//     }

    // 阻塞项目下拉框信号
    ui->AssayCombo->blockSignals(false);
}

///
/// @brief
///     更新质控结果
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月9日，新建函数
///
void QcGraphLJWidget::UpdateQcResult()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取原本选中的数据库主键
    std::int64_t iSelDbNo1 = UI_INVALID_DB_NO;
    std::int64_t iSelDbNo2 = UI_INVALID_DB_NO;
    std::int64_t iSelDbNo3 = UI_INVALID_DB_NO;
    std::int64_t iSelDbNo4 = UI_INVALID_DB_NO;
    QModelIndex selIndex1 = ui->QcResultTbl1->currentIndex();
    QModelIndex selIndex2 = ui->QcResultTbl2->currentIndex();
    QModelIndex selIndex3 = ui->QcResultTbl3->currentIndex();
    QModelIndex selIndex4 = ui->QcResultTbl4->currentIndex();
    if (selIndex1.isValid() && (m_pTbl1Model->item(selIndex1.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE) != Q_NULLPTR))
    {
        iSelDbNo1 = m_pTbl1Model->item(selIndex1.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE)->text().toLongLong();
    }
    if (selIndex2.isValid() && (m_pTbl2Model->item(selIndex2.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE) != Q_NULLPTR))
    {
        iSelDbNo2 = m_pTbl2Model->item(selIndex2.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE)->text().toLongLong();
    }
    if (selIndex3.isValid() && (m_pTbl3Model->item(selIndex3.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE) != Q_NULLPTR))
    {
        iSelDbNo3 = m_pTbl3Model->item(selIndex3.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE)->text().toLongLong();
    }
    if (selIndex4.isValid() && (m_pTbl4Model->item(selIndex4.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE) != Q_NULLPTR))
    {
        iSelDbNo4 = m_pTbl4Model->item(selIndex4.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE)->text().toLongLong();
    }

    // 清空图表1、2、3、4
    m_pTbl1Model->removeRows(0, m_pTbl1Model->rowCount());
    m_pTbl2Model->removeRows(0, m_pTbl2Model->rowCount());
    m_pTbl3Model->removeRows(0, m_pTbl3Model->rowCount());
    m_pTbl4Model->removeRows(0, m_pTbl4Model->rowCount());
    //m_pTbl1Model->setRowCount(DEFAULT_ROW_CNT_OF_QC_RESULT_TABLE);
    //m_pTbl2Model->setRowCount(DEFAULT_ROW_CNT_OF_QC_RESULT_TABLE);
    //m_pTbl3Model->setRowCount(DEFAULT_ROW_CNT_OF_QC_RESULT_TABLE);
    //m_pTbl4Model->setRowCount(DEFAULT_ROW_CNT_OF_QC_RESULT_TABLE);

    // 将结果数据更新到对应表格中
    auto LoadDataToTable = [this, iSelDbNo1, iSelDbNo2, iSelDbNo3, iSelDbNo4](QStandardItemModel* pModel, const QList<QC_RESULT_INFO>& qcRltList)
    {
        // 参数检查
        if (pModel == Q_NULLPTR)
        {
            return false;
        }

        // 展示精度
        QString strCurAssayName = ui->AssayCombo->currentText();
        int precisonNum = CommonInformationManager::GetInstance()->GetPrecisionNum(strCurAssayName.toStdString());

        // 遍历结果数据
        int iRow = 0;
        for (const QC_RESULT_INFO& stuQcResult : qcRltList)
        {
            // 行数自增
            if (iRow >= pModel->rowCount())
            {
                pModel->setRowCount(iRow + 1);
            }

            // 更新单元项
            pModel->setItem(iRow, 0, new QStandardItem(""));                                                                    // 是否计算
            pModel->setItem(iRow, 1, new QStandardItem(QString::number(iRow + 1)));                                             // 序号
            pModel->setItem(iRow, 2, new QStandardItem(stuQcResult.strQcTime));                                                 // 质控时间
            pModel->setItem(iRow, 3, new QStandardItem(QString::number(stuQcResult.dQcResult, 'f', precisonNum)));                                               // 结果
            pModel->setItem(iRow, 4, new QStandardItem(""));																	// 状态(代理实现)
            pModel->setItem(iRow, 5, new QStandardItem(stuQcResult.strOutCtrlRule));                                            // 失控规则
            pModel->setItem(iRow, 6, new QStandardItem(QString::number(stuQcResult.dQcTargetVal, 'f', precisonNum)));           // 靶值
            pModel->setItem(iRow, 7, new QStandardItem(QString::number(stuQcResult.dQcSD, 'f', precisonNum)));                  // SD
            pModel->setItem(iRow, 8, new QStandardItem(stuQcResult.strOperator));                                               // 操作者

            // 保存数据库主键及数据
            pModel->setItem(iRow, COL_INDEX_DB_NO_OF_QC_RESULT_TABLE, new QStandardItem(stuQcResult.strID));
            pModel->item(iRow, COL_INDEX_DB_NO_OF_QC_RESULT_TABLE)->setData(QVariant::fromValue<QC_RESULT_INFO>(stuQcResult));

            // 选中对应项
            if ((pModel == m_pTbl1Model) && (iSelDbNo1 == stuQcResult.strID.toLongLong()))
            {
                ui->QcResultTbl1->selectRow(iRow);
            }
            else if ((pModel == m_pTbl2Model) && (iSelDbNo2 == stuQcResult.strID.toLongLong()))
            {
                ui->QcResultTbl2->selectRow(iRow);
            }
            else if ((pModel == m_pTbl3Model) && (iSelDbNo3 == stuQcResult.strID.toLongLong()))
            {
                ui->QcResultTbl3->selectRow(iRow);
            }
            else if ((pModel == m_pTbl4Model) && (iSelDbNo4 == stuQcResult.strID.toLongLong()))
            {
                ui->QcResultTbl4->selectRow(iRow);
            }

            // 更新是否计算
            if (stuQcResult.bCalculated)
            {
                pModel->item(iRow, COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE)->setIcon(QIcon(":/Leonis/resource/image/icon-select.png"));
            }
            else
            {
                pModel->item(iRow, COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE)->setIcon(QIcon());
            }

            // 根据失控状态判断单元格颜色
			pModel->item(iRow, COL_INDEX_STATE_OF_QC_RESULT_TABLE)->setData(ConvertTfEnumToQString(stuQcResult.enQcState), Qt::DisplayRole);
            // 失控
            if (stuQcResult.enQcState == QC_STATE_OUT_OF_CTRL)
            {
                // 红色
                pModel->item(iRow, COL_INDEX_STATE_OF_QC_RESULT_TABLE)->setData(StatusItemDelegate::STATE_ERROR, BK_STATE_ROLE);

				// 有处理措施或失控原因则加标识
				if (!stuQcResult.strSolution.isEmpty() || !stuQcResult.strOutCtrlReason.isEmpty())
				{
					pModel->item(iRow, COL_INDEX_STATE_OF_QC_RESULT_TABLE)->setData(QPixmap(QString(OUT_CONTROL_HAND_PNG)), BK_FLAG_ROLE);
				}
            }
            else if (stuQcResult.enQcState == QC_STATE_WARNING)
            {
                // 橙色
                pModel->item(iRow, COL_INDEX_STATE_OF_QC_RESULT_TABLE)->setData(StatusItemDelegate::STATE_WARNING, BK_STATE_ROLE);
            }

            // 行号自增
            ++iRow;
        }

        // 表格内容居中
        SetTblTextAlign(pModel, Qt::AlignCenter);

        return true;
    };

    // 构造查询条件
    QC_RESULT_QUERY_COND    qryCond;

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

    // 遍历质控信息表，加载对应质控结果表
    int iTbl1ScrollRow = -1;
    int iTbl2ScrollRow = -1;
    int iTbl3ScrollRow = -1;
    int iTbl4ScrollRow = -1;
    for (int iRow = 0; iRow < ui->QcInfoTable->rowCount(); iRow++)
    {
        // 获取对应项
        QTableWidgetItem* pItem = ui->QcInfoTable->item(iRow, COL_INDEX_GRAPHIC_INDEX_OF_QC_INFO_TABLE);
        if (pItem == Q_NULLPTR)
        {
            continue;
        }

        // 获取对应下拉框
        QComboBox* pCombo = qobject_cast<QComboBox*>(ui->QcInfoTable->cellWidget(iRow, COL_INDEX_GRAPHIC_INDEX_OF_QC_INFO_TABLE));
        if (pCombo == Q_NULLPTR)
        {
            auto it = m_mapGraphicComboBuffer.find(iRow);
            pCombo = (it != m_mapGraphicComboBuffer.end()) ? it.value() : Q_NULLPTR;
        }

        // 没有找到下拉框，则返回
        if (pCombo == Q_NULLPTR)
        {
            continue;
        }

        // 如果对应图表为空，则跳过
        if (pCombo->currentData().toInt() == COMBO_INDEX_OF_GRAPHIC_EMPTY)
        {
            continue;
        }

        // 获取对应质控信息
        QC_DOC_CONC_INFO stuQcInfo = ui->QcInfoTable->item(iRow, COL_INDEX_DB_NO_OF_QC_INFO_TABLE)->data(Qt::UserRole).value<QC_DOC_CONC_INFO>();

        // 获取质控结果
        qryCond.strQcDocID = stuQcInfo.strID;
        //qryCond.__set_qcDocId();

        // 执行查询
        // 查询质控结果
        QList<QC_RESULT_INFO>   qryResp;
        if (!gUiAdapterPtr(m_strCurDevName, m_strCurDevGroupName, iModuleNo)->QueryQcRltInfo(qryCond, qryResp))
        {
            ULOG(LOG_ERROR, "QueryQcRltInfo failed.");
            return;
        }

        // 判断对应图表
        if (pCombo->currentData().toInt() == COMBO_INDEX_OF_GRAPHIC_1)
        {
            iTbl1ScrollRow = qryResp.size() - 1;
            LoadDataToTable(m_pTbl1Model, qryResp);
        }
        else if (pCombo->currentData().toInt() == COMBO_INDEX_OF_GRAPHIC_2)
        {
            iTbl2ScrollRow = qryResp.size() - 1;
            LoadDataToTable(m_pTbl2Model, qryResp);
        }
        else if (pCombo->currentData().toInt() == COMBO_INDEX_OF_GRAPHIC_3)
        {
            iTbl3ScrollRow = qryResp.size() - 1;
            LoadDataToTable(m_pTbl3Model, qryResp);
        }
        else if (pCombo->currentData().toInt() == COMBO_INDEX_OF_GRAPHIC_4)
        {
            iTbl4ScrollRow = qryResp.size() - 1;
            LoadDataToTable(m_pTbl4Model, qryResp);
        }
    }

    // 滚动到最后一条
    ui->QcResultTbl1->scrollTo(m_pTbl1Model->index(iTbl1ScrollRow, 0), QAbstractItemView::PositionAtBottom);
    ui->QcResultTbl2->scrollTo(m_pTbl2Model->index(iTbl2ScrollRow, 0), QAbstractItemView::PositionAtBottom);
    ui->QcResultTbl3->scrollTo(m_pTbl3Model->index(iTbl3ScrollRow, 0), QAbstractItemView::PositionAtBottom);
    ui->QcResultTbl4->scrollTo(m_pTbl4Model->index(iTbl4ScrollRow, 0), QAbstractItemView::PositionAtBottom);

    // 根据内容调整列宽
    ResizeTblColToContent(ui->QcResultTbl1);
    ResizeTblColToContent(ui->QcResultTbl2);
    ResizeTblColToContent(ui->QcResultTbl3);
    ResizeTblColToContent(ui->QcResultTbl4);
	ResizeQcResultTableIconColumn(ui->QcResultTbl1);
	ResizeQcResultTableIconColumn(ui->QcResultTbl2);
	ResizeQcResultTableIconColumn(ui->QcResultTbl3);
	ResizeQcResultTableIconColumn(ui->QcResultTbl4);
}

///
/// @brief
///     更新质控曲线图
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月24日，新建函数
///
void QcGraphLJWidget::UpdateQcGraphic()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 重置质控图
    ui->QcGraphic->Reset();

    // 获取是否勾选显示不可计算点和显示子点
//     bool bDisplay = false;
//     if (Hazel::GetDisplayUnCalc(bDisplay))
//     {
//         ui->QcGraphic->ui->ShowNoCalcQcCB->setChecked(bDisplay);
//     }
//     if (Hazel::GetDisplayChild(bDisplay))
//     {
//         ui->QcGraphic->ui->ShowSubPointCB->setChecked(bDisplay);
//     }

    // 判断是日间质控还是日内质控并获取对应的点集
    QMap<int, QVector<QPointF>>   mapCurvePoints;
    QMap<int, QVector<long long>> mapCurvePointsID;
    QMap<int, QVector<QwtSymbol::Style>> mapCurvePointsSymbol;
    QDate startQcDate;
    if (ui->QcXDayBtn->isChecked())
    {
        // 显示“显示子点”复选框
        // 更新靶值SD的权限限制
        std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();
        ui->QcGraphic->ui->ShowSubPointCB->setVisible(userPms->IsPermisson(PSM_IM_QC_LJ_SHOWSUBPOINT));

        // 日间质控
        GetLjPointsFromQcResultByDay(startQcDate, mapCurvePoints, &mapCurvePointsID, &mapCurvePointsSymbol);

        // 设置横坐标起始日期
        if (startQcDate.isValid())
        {
            ui->QcGraphic->SetXAxisStartDate(startQcDate);
        }
        else
        {
            ui->QcGraphic->SetXAxisStartDate(ui->QcStartDateEdit->date());
        }

        // 设置不计算点大小为8
        ui->QcGraphic->SetNoCalcPointSize(10);
    }
    else if (ui->QcXCntBtn->isChecked())
    {
        // 隐藏“显示子点”复选框
        ui->QcGraphic->ui->ShowSubPointCB->setVisible(false);

        // 日内质控
        GetLjPointsFromQcResultByCount(mapCurvePoints, &mapCurvePointsID, &mapCurvePointsSymbol);

        // 清空横坐标起始日期
        ui->QcGraphic->ClearXAxisStartDate();

        // 设置不计算点大小为8
        ui->QcGraphic->SetNoCalcPointSize(10);
    }
    else
    {
        ULOG(LOG_WARN, "%s(), unknown qc graphic type.", __FUNCTION__);
        return;
    }

    // 将质控曲线点集设置到质控图中
    SetLjPointsToQcGraphic(mapCurvePoints, &mapCurvePointsID, &mapCurvePointsSymbol);

    // 更新选中点
    UpdateQcGraphicSelPt();
}

///
/// @brief
///     更新所有质控信息
///
/// @par History:
/// @li 4170/TangChuXian，2021年10月25日，新建函数
///
void QcGraphLJWidget::UpdateAllQcInfo()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 更新质控信息
    UpdateQcInfo();

    // 更新质控结果
    UpdateQcResult();

    // 更新质控图
    UpdateQcGraphic();
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
void QcGraphLJWidget::OnQcResultUpdate(unsigned int uAssayCode)
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
bool QcGraphLJWidget::LoadQcOutCtrlInfo(QcOutCtrlDlg* pQcOutCtrlDlg)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 检查参数
    if (pQcOutCtrlDlg == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s(), invalid param.", __FUNCTION__);
        return false;
    }

    // 获取质控结果当前图标索引
    int iTabIndex = ui->QcResultTabWidget->currentIndex();
    QTableView* pResultTbl = Q_NULLPTR;
    QStandardItemModel* pResultModel = Q_NULLPTR;
    if (iTabIndex == TAB_INDEX_OF_GRAPHIC_1)
    {
        pResultTbl = ui->QcResultTbl1;
        pResultModel = m_pTbl1Model;
    }
    else if (iTabIndex == TAB_INDEX_OF_GRAPHIC_2)
    {
        pResultTbl = ui->QcResultTbl2;
        pResultModel = m_pTbl2Model;
    }
    else if (iTabIndex == TAB_INDEX_OF_GRAPHIC_3)
    {
        pResultTbl = ui->QcResultTbl3;
        pResultModel = m_pTbl3Model;
    }
    else if (iTabIndex == TAB_INDEX_OF_GRAPHIC_4)
    {
        pResultTbl = ui->QcResultTbl4;
        pResultModel = m_pTbl4Model;
    }
    else
    {
        ULOG(LOG_WARN, "%s(), unknown tab index.", __FUNCTION__);
        return false;
    }

    // 判断质控结果当前选中项是否为有效索引
    QModelIndex selIndex = pResultTbl->currentIndex();
    if (!selIndex.isValid() || (pResultModel->item(selIndex.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE) == Q_NULLPTR))
    {
        ULOG(LOG_WARN, "%s(), select invalid index!", __FUNCTION__);
        return false;
    }

    // 获取选中质控结果数据
    QC_RESULT_INFO stuQcResult = pResultModel->item(selIndex.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE)->data().value<QC_RESULT_INFO>();

    // 将结果信息加载到界面上
	pQcOutCtrlDlg->InitViewInfo(stuQcResult);

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
bool QcGraphLJWidget::SaveQcOutCtrlInfo(QcOutCtrlDlg* pQcOutCtrlDlg)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 检查参数
    if (pQcOutCtrlDlg == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s(), invalid param.", __FUNCTION__);
        return false;
    }

    // 获取质控结果当前图标索引
    int iGraphicIndex = COMBO_INDEX_OF_GRAPHIC_EMPTY;
    int iTabIndex = ui->QcResultTabWidget->currentIndex();
    QTableView* pResultTbl = Q_NULLPTR;
    QStandardItemModel* pResultModel = Q_NULLPTR;
    if (iTabIndex == TAB_INDEX_OF_GRAPHIC_1)
    {
        iGraphicIndex = COMBO_INDEX_OF_GRAPHIC_1;
        pResultTbl = ui->QcResultTbl1;
        pResultModel = m_pTbl1Model;
    }
    else if (iTabIndex == TAB_INDEX_OF_GRAPHIC_2)
    {
        iGraphicIndex = COMBO_INDEX_OF_GRAPHIC_2;
        pResultTbl = ui->QcResultTbl2;
        pResultModel = m_pTbl2Model;
    }
    else if (iTabIndex == TAB_INDEX_OF_GRAPHIC_3)
    {
        iGraphicIndex = COMBO_INDEX_OF_GRAPHIC_3;
        pResultTbl = ui->QcResultTbl3;
        pResultModel = m_pTbl3Model;
    }
    else if (iTabIndex == TAB_INDEX_OF_GRAPHIC_4)
    {
        iGraphicIndex = COMBO_INDEX_OF_GRAPHIC_4;
        pResultTbl = ui->QcResultTbl4;
        pResultModel = m_pTbl4Model;
    }
    else
    {
        ULOG(LOG_WARN, "%s(), unknown tab index.", __FUNCTION__);
        return false;
    }

    // 判断质控结果当前选中项是否为有效索引
    QModelIndex selIndex = pResultTbl->currentIndex();
    if (!selIndex.isValid() || (pResultModel->item(selIndex.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE) == Q_NULLPTR))
    {
        ULOG(LOG_WARN, "%s(), select invalid index!", __FUNCTION__);
        return false;
    }

    // 获取选中质控结果数据并更新
    QC_RESULT_INFO stuQcResult = pResultModel->item(selIndex.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE)->data().value<QC_RESULT_INFO>();

    // 获取失控原因和处理措施
    QString strReason = pQcOutCtrlDlg->ui->OutCtrlReasonEdit->toPlainText();
    QString strDeal = pQcOutCtrlDlg->ui->SolutionEdit->toPlainText();

    // 保存失控处理
    if (!gUiAdapterPtr(m_strCurDevName, m_strCurDevGroupName)->UpdateQcOutCtrlReasonAndSolution(stuQcResult, strReason, strDeal))
    {
        ULOG(LOG_ERROR, "%s(), UpdateQcOutCtrlReasonAndSolution failed!!", __FUNCTION__);
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("保存失败!")));
        pTipDlg->exec();
        return false;
    }

    // 更新缓存的质控结果数据
    pResultModel->item(selIndex.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE)->setData(QVariant::fromValue<QC_RESULT_INFO>(stuQcResult));

	// 更新标识
	if (!strDeal.isEmpty()|| !strReason.isEmpty())
	{
		pResultModel->item(selIndex.row(), COL_INDEX_STATE_OF_QC_RESULT_TABLE)->setData(QPixmap(QString(OUT_CONTROL_HAND_PNG)), BK_FLAG_ROLE);
	}
	else
	{
		pResultModel->item(selIndex.row(), COL_INDEX_STATE_OF_QC_RESULT_TABLE)->setData(QPixmap(), BK_FLAG_ROLE);
	}

    return true;
}

///
/// @brief
///     构造LJ曲线的日间质控点集
///
/// @param[out]  startDate              坐标轴起始日期
/// @param[out]  mapCurvePoints         曲线的点集映射(key为曲线ID，value为对应点集)
/// @param[out]  pMapCurvePointsID      曲线的点集映射(key为曲线ID，value为对应点集ID)
/// @param[out]  pMapCurvePointsSymbol  曲线的点集映射(key为曲线ID，value为对应点符号形状)
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月24日，新建函数
///
bool QcGraphLJWidget::GetLjPointsFromQcResultByDay(QDate& startDate,
                                                   QMap<int, QVector<QPointF>> &mapCurvePoints,
                                                   QMap<int, QVector<long long>>* pMapCurvePointsID,
                                                   QMap<int, QVector<QwtSymbol::Style>>* pMapCurvePointsSymbol)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 不为空则清空
    if (pMapCurvePointsID != Q_NULLPTR)
    {
        pMapCurvePointsID->clear();
        pMapCurvePointsID->insert(McQcLjGraphic::CURVE_ID_QC_1, QVector<long long>());
        pMapCurvePointsID->insert(McQcLjGraphic::CURVE_ID_QC_2, QVector<long long>());
        pMapCurvePointsID->insert(McQcLjGraphic::CURVE_ID_QC_3, QVector<long long>());
        pMapCurvePointsID->insert(McQcLjGraphic::CURVE_ID_QC_4, QVector<long long>());
        pMapCurvePointsID->insert(McQcLjGraphic::CURVE_ID_QC_1_SUB_PT, QVector<long long>());
        pMapCurvePointsID->insert(McQcLjGraphic::CURVE_ID_QC_2_SUB_PT, QVector<long long>());
        pMapCurvePointsID->insert(McQcLjGraphic::CURVE_ID_QC_3_SUB_PT, QVector<long long>());
        pMapCurvePointsID->insert(McQcLjGraphic::CURVE_ID_QC_4_SUB_PT, QVector<long long>());
        pMapCurvePointsID->insert(McQcLjGraphic::CURVE_ID_QC_NO_CALC, QVector<long long>());
    }

    // 不为空则清空
    if (pMapCurvePointsSymbol != Q_NULLPTR)
    {
        pMapCurvePointsSymbol->clear();
        pMapCurvePointsSymbol->insert(McQcLjGraphic::CURVE_ID_QC_1, QVector<QwtSymbol::Style>());
        pMapCurvePointsSymbol->insert(McQcLjGraphic::CURVE_ID_QC_2, QVector<QwtSymbol::Style>());
        pMapCurvePointsSymbol->insert(McQcLjGraphic::CURVE_ID_QC_3, QVector<QwtSymbol::Style>());
        pMapCurvePointsSymbol->insert(McQcLjGraphic::CURVE_ID_QC_4, QVector<QwtSymbol::Style>());
        pMapCurvePointsSymbol->insert(McQcLjGraphic::CURVE_ID_QC_1_SUB_PT, QVector<QwtSymbol::Style>());
        pMapCurvePointsSymbol->insert(McQcLjGraphic::CURVE_ID_QC_2_SUB_PT, QVector<QwtSymbol::Style>());
        pMapCurvePointsSymbol->insert(McQcLjGraphic::CURVE_ID_QC_3_SUB_PT, QVector<QwtSymbol::Style>());
        pMapCurvePointsSymbol->insert(McQcLjGraphic::CURVE_ID_QC_4_SUB_PT, QVector<QwtSymbol::Style>());
        pMapCurvePointsSymbol->insert(McQcLjGraphic::CURVE_ID_QC_NO_CALC, QVector<QwtSymbol::Style>());
    }

    // 初始化点集映射
    mapCurvePoints.clear();
    mapCurvePoints.insert(McQcLjGraphic::CURVE_ID_QC_1, QVector<QPointF>());
    mapCurvePoints.insert(McQcLjGraphic::CURVE_ID_QC_2, QVector<QPointF>());
    mapCurvePoints.insert(McQcLjGraphic::CURVE_ID_QC_3, QVector<QPointF>());
    mapCurvePoints.insert(McQcLjGraphic::CURVE_ID_QC_4, QVector<QPointF>());
    mapCurvePoints.insert(McQcLjGraphic::CURVE_ID_QC_1_SUB_PT, QVector<QPointF>());
    mapCurvePoints.insert(McQcLjGraphic::CURVE_ID_QC_2_SUB_PT, QVector<QPointF>());
    mapCurvePoints.insert(McQcLjGraphic::CURVE_ID_QC_3_SUB_PT, QVector<QPointF>());
    mapCurvePoints.insert(McQcLjGraphic::CURVE_ID_QC_4_SUB_PT, QVector<QPointF>());
    mapCurvePoints.insert(McQcLjGraphic::CURVE_ID_QC_NO_CALC, QVector<QPointF>());

    // 构造质控结果的查询条件
    QList<QC_RESULT_INFO>       qryResp;
    QC_RESULT_QUERY_COND        qryCond;

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
    qryCond.strAssayName = strCurAssayName;
    qryCond.startDate = ui->QcStartDateEdit->date();
    qryCond.endDate = ui->QcEndDateEdit->date();

    // 获取最小时间作为第一个横坐标
    boost::posix_time::ptime minQcPTime;
    QC_DOC_CONC_INFO stuQc1Info;
    QC_DOC_CONC_INFO stuQc2Info;
    QC_DOC_CONC_INFO stuQc3Info;
    QC_DOC_CONC_INFO stuQc4Info;
    for (int iRow = 0; iRow < ui->QcInfoTable->rowCount(); iRow++)
    {
        // 获取对应项
        QTableWidgetItem* pItem = ui->QcInfoTable->item(iRow, COL_INDEX_GRAPHIC_INDEX_OF_QC_INFO_TABLE);
        if (pItem == Q_NULLPTR)
        {
            continue;
        }

        // 获取对应下拉框
        QComboBox* pCombo = qobject_cast<QComboBox*>(ui->QcInfoTable->cellWidget(iRow, COL_INDEX_GRAPHIC_INDEX_OF_QC_INFO_TABLE));
        if (pCombo == Q_NULLPTR)
        {
            continue;
        }

        // 如果对应图表为空，则跳过
        if (pCombo->currentData().toInt() == COMBO_INDEX_OF_GRAPHIC_EMPTY)
        {
            continue;
        }

        // 获取对应质控信息
        QC_DOC_CONC_INFO stuQcInfo = ui->QcInfoTable->item(iRow, COL_INDEX_DB_NO_OF_QC_INFO_TABLE)->data(Qt::UserRole).value<QC_DOC_CONC_INFO>();

        // 获取质控结果
        qryCond.strQcDocID = stuQcInfo.strID;

        // 执行查询
        // 查询所有质控结果
        if (!gUiAdapterPtr(m_strCurDevName, m_strCurDevGroupName, iModuleNo)->QueryQcRltInfo(qryCond, qryResp))
        {
            ULOG(LOG_ERROR, "QueryQcRltInfo failed.");
            return false;
        }

        // 判断对应图表
        if (pCombo->currentData().toInt() == COMBO_INDEX_OF_GRAPHIC_1)
        {
            stuQc1Info = std::move(stuQcInfo);
        }
        else if (pCombo->currentData().toInt() == COMBO_INDEX_OF_GRAPHIC_2)
        {
            stuQc2Info = std::move(stuQcInfo);
        }
        else if (pCombo->currentData().toInt() == COMBO_INDEX_OF_GRAPHIC_3)
        {
            stuQc3Info = std::move(stuQcInfo);
        }
        else if (pCombo->currentData().toInt() == COMBO_INDEX_OF_GRAPHIC_4)
        {
            stuQc4Info = std::move(stuQcInfo);
        }
    }

    // 获取质控结果当前图标索引
    int iTabIndex = ui->QcResultTabWidget->currentIndex();
    QTableView* pResultTbl = Q_NULLPTR;
    QStandardItemModel* pResultModel = Q_NULLPTR;
    if (iTabIndex == TAB_INDEX_OF_GRAPHIC_1)
    {
        pResultTbl = ui->QcResultTbl1;
        pResultModel = m_pTbl1Model;
    }
    else if (iTabIndex == TAB_INDEX_OF_GRAPHIC_2)
    {
        pResultTbl = ui->QcResultTbl2;
        pResultModel = m_pTbl2Model;
    }
    else if (iTabIndex == TAB_INDEX_OF_GRAPHIC_3)
    {
        pResultTbl = ui->QcResultTbl3;
        pResultModel = m_pTbl3Model;
    }
    else if (iTabIndex == TAB_INDEX_OF_GRAPHIC_4)
    {
        pResultTbl = ui->QcResultTbl4;
        pResultModel = m_pTbl4Model;
    }
    else
    {
        ULOG(LOG_WARN, "%s(), unknown tab index.", __FUNCTION__);
        return false;
    }

    // 获取质控结果中的最小时间
    for (int i = COMBO_INDEX_OF_GRAPHIC_1; i <= COMBO_INDEX_OF_GRAPHIC_4; i++)
    {
        QStandardItemModel* pResultModel = Q_NULLPTR;
        if (i == COMBO_INDEX_OF_GRAPHIC_1)
        {
            pResultModel = m_pTbl1Model;
        }
        else if (i == COMBO_INDEX_OF_GRAPHIC_2)
        {
            pResultModel = m_pTbl2Model;
        }
        else if (i == COMBO_INDEX_OF_GRAPHIC_3)
        {
            pResultModel = m_pTbl3Model;
        }
        else if (i == COMBO_INDEX_OF_GRAPHIC_4)
        {
            pResultModel = m_pTbl4Model;
        }
        else
        {
            continue;
        }

        for (int iRow = 0; iRow < pResultModel->rowCount(); iRow++)
        {
            // 获取质控结果
            QStandardItem* pItem = pResultModel->item(iRow, COL_INDEX_DB_NO_OF_QC_RESULT_TABLE);
            if (pItem == Q_NULLPTR)
            {
                continue;
            }

            // 获取质控结果
            QC_RESULT_INFO stuQcResult = pItem->data().value<QC_RESULT_INFO>();

            // 如果不计算点不显示，则排除不计算点
            if (!ui->QcGraphic->IsShowNoCalcPoint() && !stuQcResult.bCalculated)
            {
                continue;
            }

            // 如果最小时间为空，则直接赋值
            boost::posix_time::ptime qcTime = TimeStringToPosixTime(stuQcResult.strQcTime.toStdString());
            if (minQcPTime.is_not_a_date_time())
            {
                minQcPTime = qcTime;
                continue;
            }

            // 如果质控结果中的时间比最小时间小，则替换为最小时间
            if (qcTime < minQcPTime)
            {
                minQcPTime = qcTime;
                continue;
            }
        }
    }

    // 如果最小时间为空，则直接返回
    if (minQcPTime.is_not_a_date_time())
    {
        ULOG(LOG_INFO, "%s(), Qc result is empty.", __FUNCTION__);
        return true;
    }

    // 赋值起始日期为最小质控时间日期
    startDate = QDate(minQcPTime.date().year(), minQcPTime.date().month(), minQcPTime.date().day());

    // 先获取子点和不计算点(无需连线，忽略顺序)
    for (int i = COMBO_INDEX_OF_GRAPHIC_1; i <= COMBO_INDEX_OF_GRAPHIC_4; i++)
    {
        QStandardItemModel* pResultModel = Q_NULLPTR;
        McQcLjGraphic::CURVE_ID enSubPtCurveId;
        if (i == COMBO_INDEX_OF_GRAPHIC_1)
        {
            enSubPtCurveId = McQcLjGraphic::CURVE_ID_QC_1_SUB_PT;
            pResultModel = m_pTbl1Model;
        }
        else if (i == COMBO_INDEX_OF_GRAPHIC_2)
        {
            enSubPtCurveId = McQcLjGraphic::CURVE_ID_QC_2_SUB_PT;
            pResultModel = m_pTbl2Model;
        }
        else if (i == COMBO_INDEX_OF_GRAPHIC_3)
        {
            enSubPtCurveId = McQcLjGraphic::CURVE_ID_QC_3_SUB_PT;
            pResultModel = m_pTbl3Model;
        }
        else if (i == COMBO_INDEX_OF_GRAPHIC_4)
        {
            enSubPtCurveId = McQcLjGraphic::CURVE_ID_QC_4_SUB_PT;
            pResultModel = m_pTbl4Model;
        }
        else
        {
            continue;
        }

        for (int iRow = 0; iRow < pResultModel->rowCount(); iRow++)
        {
            // 获取质控结果
            QStandardItem* pItem = pResultModel->item(iRow, COL_INDEX_DB_NO_OF_QC_RESULT_TABLE);
            if (pItem == Q_NULLPTR)
            {
                continue;
            }

            // 获取质控结果
            QC_RESULT_INFO stuQcResult = pItem->data().value<QC_RESULT_INFO>();

            // 如果时间为空，则跳过
            boost::posix_time::ptime qcTime = TimeStringToPosixTime(stuQcResult.strQcTime.toStdString());
            if (qcTime.is_not_a_date_time())
            {
                continue;
            }

            // 计算X值和Y值
            auto dateDuration = qcTime.date() - minQcPTime.date();
            double dResult = stuQcResult.dQcResult;
            double dXVal = dateDuration.days() + 1.0;
            double dYVal = (dResult - stuQcResult.dQcTargetVal) / stuQcResult.dQcSD;

            // 坐标最大绝对值3.9
            if (dYVal > 3.9)
            {
                dYVal = 3.9;
            }
            else if (dYVal < -3.9)
            {
                dYVal = -3.9;
            }

            // 如果不是计算点，则添加到不计算点中，否则添加到对应子点中
            if (!stuQcResult.bCalculated && ui->QcGraphic->IsShowNoCalcPoint())
            {
                mapCurvePoints[McQcLjGraphic::CURVE_ID_QC_NO_CALC].push_back(QPointF(dXVal, dYVal));
                if (pMapCurvePointsID != Q_NULLPTR)
                {
                    QMap<int, QVector<long long>>& mapCurvePointsID = *pMapCurvePointsID;
                    mapCurvePointsID[McQcLjGraphic::CURVE_ID_QC_NO_CALC].push_back(stuQcResult.strID.toLongLong());
                }

                if (pMapCurvePointsSymbol != Q_NULLPTR)
                {
                    QMap<int, QVector<QwtSymbol::Style>>& mapCurvePointsSymbol = *pMapCurvePointsSymbol;
                    mapCurvePointsSymbol[McQcLjGraphic::CURVE_ID_QC_NO_CALC].push_back(QwtSymbol::Ellipse);
                }
            }
            else if (stuQcResult.bCalculated && ui->QcGraphic->ui->ShowSubPointCB->isChecked())
            {
                mapCurvePoints[enSubPtCurveId].push_back(QPointF(dXVal, dYVal));
                if (pMapCurvePointsID != Q_NULLPTR)
                {
                    QMap<int, QVector<long long>>& mapCurvePointsID = *pMapCurvePointsID;
                    mapCurvePointsID[enSubPtCurveId].push_back(stuQcResult.strID.toLongLong());
                }

                if (pMapCurvePointsSymbol != Q_NULLPTR)
                {
                    QMap<int, QVector<QwtSymbol::Style>>& mapCurvePointsSymbol = *pMapCurvePointsSymbol;
                    mapCurvePointsSymbol[enSubPtCurveId].push_back(ContructSymbolStyleByOutCtrlState(stuQcResult.enQcState));
                }
            }
        }
    }

    // 获取质控1、2、3、4均线曲线点集
    // 获取质控1、2、3、4所有子点映射
    QMap<int, QList<QC_RESULT_INFO>> mapAvgQc1Curve;
    QMap<int, QList<QC_RESULT_INFO>> mapAvgQc2Curve;
    QMap<int, QList<QC_RESULT_INFO>> mapAvgQc3Curve;
    QMap<int, QList<QC_RESULT_INFO>> mapAvgQc4Curve;
    for (int i = COMBO_INDEX_OF_GRAPHIC_1; i <= COMBO_INDEX_OF_GRAPHIC_4; i++)
    {
        // 构造对应变量
        QStandardItemModel* pResultModel = Q_NULLPTR;
        QMap<int, QList<QC_RESULT_INFO>>* pMapAvgQcCurve;
        if (i == COMBO_INDEX_OF_GRAPHIC_1)
        {
            pMapAvgQcCurve = &mapAvgQc1Curve;
            pResultModel = m_pTbl1Model;
        }
        else if (i == COMBO_INDEX_OF_GRAPHIC_2)
        {
            pMapAvgQcCurve = &mapAvgQc2Curve;
            pResultModel = m_pTbl2Model;
        }
        else if (i == COMBO_INDEX_OF_GRAPHIC_3)
        {
            pMapAvgQcCurve = &mapAvgQc3Curve;
            pResultModel = m_pTbl3Model;
        }
        else if (i == COMBO_INDEX_OF_GRAPHIC_4)
        {
            pMapAvgQcCurve = &mapAvgQc4Curve;
            pResultModel = m_pTbl4Model;
        }
        else
        {
            continue;
        }

        // 获取对应图表所有子点映射的引用
        QMap<int, QList<QC_RESULT_INFO>>& mapAvgQcCurve = *pMapAvgQcCurve;

        // 遍历所有结果
        for (int iRow = 0; iRow < pResultModel->rowCount(); iRow++)
        {
            // 获取质控结果
            QStandardItem* pItem = pResultModel->item(iRow, COL_INDEX_DB_NO_OF_QC_RESULT_TABLE);
            if (pItem == Q_NULLPTR)
            {
                continue;
            }

            // 获取质控结果
            QC_RESULT_INFO stuQcResult = pItem->data().value<QC_RESULT_INFO>();

            // 如果不计算，则跳过
            if (!stuQcResult.bCalculated)
            {
                continue;
            }

            // 如果时间为空，则跳过
            boost::posix_time::ptime qcTime = TimeStringToPosixTime(stuQcResult.strQcTime.toStdString());
            if (qcTime.is_not_a_date_time())
            {
                continue;
            }

            // 计算X值和Y值
            auto dateDuration = qcTime.date() - minQcPTime.date();
            int iXVal = dateDuration.days() + 1;

            // 如果找到对应横坐标，则追加Y值，否则则插入y值
            auto it = mapAvgQcCurve.find(iXVal);
            if (it != mapAvgQcCurve.end())
            {
                it.value().push_back(stuQcResult);
            }
            else
            {
                mapAvgQcCurve.insert(iXVal, QList<QC_RESULT_INFO>());
                mapAvgQcCurve[iXVal].push_back(stuQcResult);
            }
        }
    }

    // 获取质控均线点集
    auto GetQcResultConcAvg = [](const QList<QC_RESULT_INFO>& stuResultList)
    {
        double dSum = 0.0;
        for (const QC_RESULT_INFO& stuResult : stuResultList)
        {
            double dResult = stuResult.dQcResult;
            dSum += dResult;
        }

        return dSum / stuResultList.size();
    };
    for (int i = COMBO_INDEX_OF_GRAPHIC_1; i <= COMBO_INDEX_OF_GRAPHIC_4; i++)
    {
        // 构造对应变量
        McQcLjGraphic::CURVE_ID enAvgPtCurveId;
        QMap<int, QList<QC_RESULT_INFO>>* pMapAvgQcCurve;
        if (i == COMBO_INDEX_OF_GRAPHIC_1)
        {
            enAvgPtCurveId = McQcLjGraphic::CURVE_ID_QC_1;
            pMapAvgQcCurve = &mapAvgQc1Curve;
        }
        else if (i == COMBO_INDEX_OF_GRAPHIC_2)
        {
            enAvgPtCurveId = McQcLjGraphic::CURVE_ID_QC_2;
            pMapAvgQcCurve = &mapAvgQc2Curve;
        }
        else if (i == COMBO_INDEX_OF_GRAPHIC_3)
        {
            enAvgPtCurveId = McQcLjGraphic::CURVE_ID_QC_3;
            pMapAvgQcCurve = &mapAvgQc3Curve;
        }
        else if (i == COMBO_INDEX_OF_GRAPHIC_4)
        {
            enAvgPtCurveId = McQcLjGraphic::CURVE_ID_QC_4;
            pMapAvgQcCurve = &mapAvgQc4Curve;
        }
        else
        {
            continue;
        }

        // 获取对应图表所有子点映射的引用
        QMap<int, QList<QC_RESULT_INFO>>& mapAvgQcCurve = *pMapAvgQcCurve;

        // 遍历所有结果
        for (auto it = mapAvgQcCurve.begin(); it != mapAvgQcCurve.end(); it++)
        {
            // 结果为空，则跳过
            if (it.value().empty())
            {
                continue;
            }

            // 构造横坐标和纵坐标
            double dXVal = double(it.key());
            double dResult = GetQcResultConcAvg(it.value());
            QC_RESULT_INFO stuQcResult = it.value().last();
            double dYVal = (dResult - stuQcResult.dQcTargetVal) / stuQcResult.dQcSD;

            // 坐标最大绝对值3.9
            if (dYVal > 3.9)
            {
                dYVal = 3.9;
            }
            else if (dYVal < -3.9)
            {
                dYVal = -3.9;
            }

            // 添加到质控均线中
            mapCurvePoints[enAvgPtCurveId].push_back(QPointF(dXVal, dYVal));

            // 还原符号
            QMap<int, QVector<QwtSymbol::Style>>& mapCurvePointsSymbol = *pMapCurvePointsSymbol;
            mapCurvePointsSymbol[enAvgPtCurveId].push_back(QwtSymbol::Ellipse);
        }
    }

    return true;
}

///
/// @brief
///     构造LJ曲线的日内质控点集
///
/// @param[out]  mapCurvePoints         曲线的点集映射(key为曲线ID，value为对应点集)
/// @param[out]  pMapCurvePointsID      曲线的点集映射(key为曲线ID，value为对应点集ID)
/// @param[out]  pMapCurvePointsSymbol  曲线的点集映射(key为曲线ID，value为对应点符号形状)
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月24日，新建函数
///
bool QcGraphLJWidget::GetLjPointsFromQcResultByCount(QMap<int, QVector<QPointF>> &mapCurvePoints,
                                                     QMap<int, QVector<long long>>* pMapCurvePointsID,
                                                     QMap<int, QVector<QwtSymbol::Style>>* pMapCurvePointsSymbol)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 不为空则清空
    if (pMapCurvePointsID != Q_NULLPTR)
    {
        pMapCurvePointsID->clear();
        pMapCurvePointsID->insert(McQcLjGraphic::CURVE_ID_QC_1, QVector<long long>());
        pMapCurvePointsID->insert(McQcLjGraphic::CURVE_ID_QC_2, QVector<long long>());
        pMapCurvePointsID->insert(McQcLjGraphic::CURVE_ID_QC_3, QVector<long long>());
        pMapCurvePointsID->insert(McQcLjGraphic::CURVE_ID_QC_4, QVector<long long>());
        pMapCurvePointsID->insert(McQcLjGraphic::CURVE_ID_QC_NO_CALC, QVector<long long>());
    }

    // 不为空则清空
    if (pMapCurvePointsSymbol != Q_NULLPTR)
    {
        pMapCurvePointsSymbol->clear();
        pMapCurvePointsSymbol->insert(McQcLjGraphic::CURVE_ID_QC_1, QVector<QwtSymbol::Style>());
        pMapCurvePointsSymbol->insert(McQcLjGraphic::CURVE_ID_QC_2, QVector<QwtSymbol::Style>());
        pMapCurvePointsSymbol->insert(McQcLjGraphic::CURVE_ID_QC_3, QVector<QwtSymbol::Style>());
        pMapCurvePointsSymbol->insert(McQcLjGraphic::CURVE_ID_QC_4, QVector<QwtSymbol::Style>());
        pMapCurvePointsSymbol->insert(McQcLjGraphic::CURVE_ID_QC_NO_CALC, QVector<QwtSymbol::Style>());
    }

    // 初始化点集映射
    mapCurvePoints.clear();
    mapCurvePoints.insert(McQcLjGraphic::CURVE_ID_QC_1, QVector<QPointF>());
    mapCurvePoints.insert(McQcLjGraphic::CURVE_ID_QC_2, QVector<QPointF>());
    mapCurvePoints.insert(McQcLjGraphic::CURVE_ID_QC_3, QVector<QPointF>());
    mapCurvePoints.insert(McQcLjGraphic::CURVE_ID_QC_4, QVector<QPointF>());
    mapCurvePoints.insert(McQcLjGraphic::CURVE_ID_QC_1_SUB_PT, QVector<QPointF>());
    mapCurvePoints.insert(McQcLjGraphic::CURVE_ID_QC_2_SUB_PT, QVector<QPointF>());
    mapCurvePoints.insert(McQcLjGraphic::CURVE_ID_QC_3_SUB_PT, QVector<QPointF>());
    mapCurvePoints.insert(McQcLjGraphic::CURVE_ID_QC_4_SUB_PT, QVector<QPointF>());
    mapCurvePoints.insert(McQcLjGraphic::CURVE_ID_QC_NO_CALC, QVector<QPointF>());

    // 先获取所有点和不计算点
    for (int i = COMBO_INDEX_OF_GRAPHIC_1; i <= COMBO_INDEX_OF_GRAPHIC_4; i++)
    {
        QStandardItemModel* pResultModel = Q_NULLPTR;
        McQcLjGraphic::CURVE_ID enSubPtCurveId;
        if (i == COMBO_INDEX_OF_GRAPHIC_1)
        {
            enSubPtCurveId = McQcLjGraphic::CURVE_ID_QC_1;
            pResultModel = m_pTbl1Model;
        }
        else if (i == COMBO_INDEX_OF_GRAPHIC_2)
        {
            enSubPtCurveId = McQcLjGraphic::CURVE_ID_QC_2;
            pResultModel = m_pTbl2Model;
        }
        else if (i == COMBO_INDEX_OF_GRAPHIC_3)
        {
            enSubPtCurveId = McQcLjGraphic::CURVE_ID_QC_3;
            pResultModel = m_pTbl3Model;
        }
        else if (i == COMBO_INDEX_OF_GRAPHIC_4)
        {
            enSubPtCurveId = McQcLjGraphic::CURVE_ID_QC_4;
            pResultModel = m_pTbl4Model;
        }
        else
        {
            continue;
        }

        int iPointIndex = 0;
        for (int iRow = 0; iRow < pResultModel->rowCount(); iRow++)
        {
            // 获取质控结果
            QStandardItem* pItem = pResultModel->item(iRow, COL_INDEX_DB_NO_OF_QC_RESULT_TABLE);
            if (pItem == Q_NULLPTR)
            {
                continue;
            }

            // 获取质控结果
            QC_RESULT_INFO stuQcResult = pItem->data().value<QC_RESULT_INFO>();

            // 计算X值和Y值
            double dResult = stuQcResult.dQcResult;
            double dXVal = double(iPointIndex + 1);
            double dYVal = (dResult - stuQcResult.dQcTargetVal) / stuQcResult.dQcSD;

            // 坐标最大绝对值3.9
            if (dYVal > 3.9)
            {
                dYVal = 3.9;
            }
            else if (dYVal < -3.9)
            {
                dYVal = -3.9;
            }

            // 如果不是计算点，则添加到不计算点中，否则添加到对应曲线中
            if (!stuQcResult.bCalculated && ui->QcGraphic->IsShowNoCalcPoint())
            {
                mapCurvePoints[McQcLjGraphic::CURVE_ID_QC_NO_CALC].push_back(QPointF(dXVal, dYVal));
                if (pMapCurvePointsID != Q_NULLPTR)
                {
                    QMap<int, QVector<long long>>& mapCurvePointsID = *pMapCurvePointsID;
                    mapCurvePointsID[McQcLjGraphic::CURVE_ID_QC_NO_CALC].push_back(stuQcResult.strID.toLongLong());
                }

                if (pMapCurvePointsSymbol != Q_NULLPTR)
                {
                    QMap<int, QVector<QwtSymbol::Style>>& mapCurvePointsSymbol = *pMapCurvePointsSymbol;
                    mapCurvePointsSymbol[McQcLjGraphic::CURVE_ID_QC_NO_CALC].push_back(QwtSymbol::Ellipse);
                }
            }
            else if (stuQcResult.bCalculated)
            {
                mapCurvePoints[enSubPtCurveId].push_back(QPointF(dXVal, dYVal));
                if (pMapCurvePointsID != Q_NULLPTR)
                {
                    QMap<int, QVector<long long>>& mapCurvePointsID = *pMapCurvePointsID;
                    mapCurvePointsID[enSubPtCurveId].push_back(stuQcResult.strID.toLongLong());
                }

                if (pMapCurvePointsSymbol != Q_NULLPTR)
                {
                    QMap<int, QVector<QwtSymbol::Style>>& mapCurvePointsSymbol = *pMapCurvePointsSymbol;
                    mapCurvePointsSymbol[enSubPtCurveId].push_back(ContructSymbolStyleByOutCtrlState(stuQcResult.enQcState));
                }
            }

            // 点索引自增
            ++iPointIndex;
        }
    }

    return true;
}

///
/// @brief
///     将曲线上的点集映射设置到质控图中
///
/// @param[out]  mapCurvePoints         曲线的点集映射(key为曲线ID，value为对应点集)
/// @param[out]  pMapCurvePointsID      曲线的点集映射(key为曲线ID，value为对应点集ID)
/// @param[out]  pMapCurvePointsSymbol  曲线的点集映射(key为曲线ID，value为对应点符号形状)
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月24日，新建函数
///
bool QcGraphLJWidget::SetLjPointsToQcGraphic(const QMap<int, QVector<QPointF>> &mapCurvePoints,
                                             QMap<int, QVector<long long>>* pMapCurvePointsID,
                                             QMap<int, QVector<QwtSymbol::Style>>* pMapCurvePointsSymbol)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 遍历曲线点集映射，将点集设置到对应曲线
    for (auto it = mapCurvePoints.begin(); it != mapCurvePoints.end(); it++)
    {
        QVector<long long>        vecPtID;
        QVector<QwtSymbol::Style> vecPtSymbol;

        if (pMapCurvePointsID != Q_NULLPTR)
        {
            QMap<int, QVector<long long>>& mapCurvePointsID = *pMapCurvePointsID;
            auto itPtID = mapCurvePointsID.find(it.key());
            if (itPtID != mapCurvePointsID.end())
            {
                vecPtID = itPtID.value();
            }
        }

        if (pMapCurvePointsSymbol != Q_NULLPTR)
        {
            QMap<int, QVector<QwtSymbol::Style>>& mapCurvePointsSymbol = *pMapCurvePointsSymbol;
            auto itPtSymbol = mapCurvePointsSymbol.find(it.key());
            if (itPtSymbol != mapCurvePointsSymbol.end())
            {
                vecPtSymbol = itPtSymbol.value();
            }
        }

        ui->QcGraphic->SetPoints(McQcLjGraphic::CURVE_ID(it.key()), it.value(), vecPtID, vecPtSymbol);
    }

    return true;
}

///
/// @brief
///     更新质控图选中点
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年7月4日，新建函数
///
bool QcGraphLJWidget::UpdateQcGraphicSelPt()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 质控图清空选中效果
    ui->QcGraphic->SelPointByID(McQcLjGraphic::CURVE_ID_SIZE, UI_INVALID_DB_NO);

    // 获取对应模型
    QTableView*         pQcResultTbl = Q_NULLPTR;
    QStandardItemModel* pQcResultModel = Q_NULLPTR;
    int iTabIdx = ui->QcResultTabWidget->currentIndex();
    McQcLjGraphic::CURVE_ID enCurveID = McQcLjGraphic::CURVE_ID_SIZE;
    if (iTabIdx == TAB_INDEX_OF_GRAPHIC_1)
    {
        pQcResultTbl = ui->QcResultTbl1;
        pQcResultModel = m_pTbl1Model;
        enCurveID = ui->QcXDayBtn->isChecked() ? McQcLjGraphic::CURVE_ID_QC_1_SUB_PT : McQcLjGraphic::CURVE_ID_QC_1;
    }
    else if (iTabIdx == TAB_INDEX_OF_GRAPHIC_2)
    {
        pQcResultTbl = ui->QcResultTbl2;
        pQcResultModel = m_pTbl2Model;
        enCurveID = ui->QcXDayBtn->isChecked() ? McQcLjGraphic::CURVE_ID_QC_2_SUB_PT : McQcLjGraphic::CURVE_ID_QC_2;
    }
    else if (iTabIdx == TAB_INDEX_OF_GRAPHIC_3)
    {
        pQcResultTbl = ui->QcResultTbl3;
        pQcResultModel = m_pTbl3Model;
        enCurveID = ui->QcXDayBtn->isChecked() ? McQcLjGraphic::CURVE_ID_QC_3_SUB_PT : McQcLjGraphic::CURVE_ID_QC_3;
    }
    else if (iTabIdx == TAB_INDEX_OF_GRAPHIC_4)
    {
        pQcResultTbl = ui->QcResultTbl4;
        pQcResultModel = m_pTbl4Model;
        enCurveID = ui->QcXDayBtn->isChecked() ? McQcLjGraphic::CURVE_ID_QC_4_SUB_PT : McQcLjGraphic::CURVE_ID_QC_4;
    }

    // 如果当前模型为空则返回
    if ((pQcResultModel == Q_NULLPTR) || (pQcResultTbl == Q_NULLPTR))
    {
        // 模型无效
        ULOG(LOG_WARN, "%s(), pQcResultModel == Q_NULLPTR.", __FUNCTION__);
        return false;
    }

    // 获取当前项索引
    QModelIndex index = pQcResultTbl->currentIndex();

    // 如果没有选中项或选中项是空行，则返回
    if (!index.isValid() || (pQcResultModel->item(index.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE) == Q_NULLPTR))
    {
        // 忽略
        ULOG(LOG_INFO, "%s(), select invalid index.", __FUNCTION__);
        return false;
    }

    // 在控时候，失控处理按钮禁用（警告需确认）
    QC_RESULT_INFO stuQcResult = pQcResultModel->item(index.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE)->data().value<QC_RESULT_INFO>();

    // 质控图选中对应点
    if (!stuQcResult.bCalculated)
    {
        ui->QcGraphic->SelPointByID(McQcLjGraphic::CURVE_ID_QC_NO_CALC, stuQcResult.strID.toLongLong());
    }
    else
    {
        ui->QcGraphic->SelPointByID(enCurveID, stuQcResult.strID.toLongLong());
    }

    return true;
}

///
/// @brief
///     更新打印表
///
/// @param[in]  stuQcInfo  打印样本
///
/// @par History:
/// @li 4170/TangChuXian，2022年7月7日，新建函数
///
// void QcGraphLJWidget::UpdatePrintTbl(const QC_DOC_CONC_INFO& stuQcInfo)
// {
//     ULOG(LOG_INFO, "%s()", __FUNCTION__);
//     // 清空内容
//     m_pPrintTblQcLjResult->setRowCount(0);
// 
//     // 遍历所有项目
//     int iRow = 0;                                                              // 数据行数
//     for (const QC_RESULT_INFO& stuQcResult : stuQcInfo.vecResult)
//     {
//         // 如果行号大于等于行数，则行数自动增加
//         if (iRow >= m_pPrintTblQcLjResult->rowCount())
//         {
//             m_pPrintTblQcLjResult->setRowCount(iRow + 1);
//         }
// 
//         // 设置单元格内容
//         m_pPrintTblQcLjResult->setItem(iRow, 0, new QStandardItem(QString::number(iRow + 1)));                                               // 序号
//         m_pPrintTblQcLjResult->setItem(iRow, 1, new QStandardItem(QString::fromStdString(PosixTimeToDateString(stuQcResult.ptQCDate))));     // 项目名
//         m_pPrintTblQcLjResult->setItem(iRow, 2, new QStandardItem(QString::fromStdString(stuQcResult.strResult)));                           // 结果
// 
//         // 行号自增
//         ++iRow;
//     }
// }

///
/// @brief
///     更新打印模板变量标签
///
/// @param[in]  stuQcInfo  打印样本
///
/// @par History:
/// @li 4170/TangChuXian，2022年7月7日，新建函数
///
// void QcGraphLJWidget::UpdatePrintVarStr(const QC_DOC_CONC_INFO& stuQcInfo)
// {
//     ULOG(LOG_INFO, "%s()", __FUNCTION__);
//     // 构造查询日期范围
//     QString strQcDateRange("");
//     strQcDateRange = ui->QcStartDateEdit->date().toString("yyyy/MM/dd") + " -- " + ui->QcEndDateEdit->date().toString("yyyy/MM/dd");
// 
//     // 构造映射
//     m_mapPrintTmpVarStr.insert(PRINT_TMP_VAR_LAB_ID_QC_LJ_ITEM_NAME, QString::fromStdString(stuQcInfo.strItemName));
//     m_mapPrintTmpVarStr.insert(PRINT_TMP_VAR_LAB_ID_QC_LJ_QC_NO, QString::fromStdString(stuQcInfo.strQCID));
//     m_mapPrintTmpVarStr.insert(PRINT_TMP_VAR_LAB_ID_QC_LJ_QC_SD, QString::fromStdString(stuQcInfo.strTargetSD));
//     m_mapPrintTmpVarStr.insert(PRINT_TMP_VAR_LAB_ID_QC_LJ_TARGET_VAL, QString::fromStdString(stuQcInfo.strTargetMean));
//     m_mapPrintTmpVarStr.insert(PRINT_TMP_VAR_LAB_ID_QC_LJ_QC_NAME, QString::fromStdString(stuQcInfo.strQCName));
//     m_mapPrintTmpVarStr.insert(PRINT_TMP_VAR_LAB_ID_QC_LJ_QC_CALC_SD, QString::fromStdString(stuQcInfo.strActualSD));
//     m_mapPrintTmpVarStr.insert(PRINT_TMP_VAR_LAB_ID_QC_LJ_QC_CALC_TARGET_VAL, QString::fromStdString(stuQcInfo.strActualMean));
//     m_mapPrintTmpVarStr.insert(PRINT_TMP_VAR_LAB_ID_QC_LJ_QC_LOT, QString::fromStdString(stuQcInfo.strQCLot));
//     m_mapPrintTmpVarStr.insert(PRINT_TMP_VAR_LAB_ID_QC_LJ_QC_CV, QString::fromStdString(stuQcInfo.strTargetCV));
//     m_mapPrintTmpVarStr.insert(PRINT_TMP_VAR_LAB_ID_QC_LJ_QC_CALC_CV, QString::fromStdString(stuQcInfo.strActualCV));
//     m_mapPrintTmpVarStr.insert(PRINT_TMP_VAR_LAB_ID_QC_LJ_QC_DATE_RANGE, strQcDateRange);
// 
//     // 质控LJ图结果表
//     m_mapPrintTmpVarStr.insert(PRINT_TMP_VAR_TBL_ID_QC_LJ_RESULT_TBL, LoadStrFromLanguage(CHAR_CODE::IDS_QC_LJ_GRAPHIC));
// }

///
/// @brief
///     构造符号风格通过失控类型
///
/// @param[in]  enType  失控类型
///
/// @return 符号样式
///
/// @par History:
/// @li 4170/TangChuXian，2022年7月7日，新建函数
///
QwtSymbol::Style QcGraphLJWidget::ContructSymbolStyleByOutCtrlState(QC_STATE enType)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    switch (enType)
    {
    case QC_STATE_WARNING:
        return QwtSymbol::Triangle;
        break;
    case QC_STATE_OUT_OF_CTRL:
        return QwtSymbol::XCross;
        break;
    default:
       break;
    }

    return QwtSymbol::Ellipse;
}

///
/// @brief
///     质控条件改变
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月10日，新建函数
///
void QcGraphLJWidget::OnQcCondChanged()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取信号发送者
    QComboBox* pSnderCombo = qobject_cast<QComboBox*>(sender());

    // 判断是否为项目更新
    if (pSnderCombo == ui->AssayCombo)
    {
        // 更新质控信息表
        UpdateQcInfoByAssayChanged();
    }
    else
    {
        // 更新质控信息表
        UpdateQcInfo();
    }

    // 更新质控结果表
    UpdateQcResult();

    // 更新质控图
    UpdateQcGraphic();
}

///
/// @brief
///     质控图类型改变（日内质控和日间质控切换）
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月24日，新建函数
///
void QcGraphLJWidget::OnQcGraphicTypeChanged()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 判断是日内质控还是日间质控
    if (ui->QcXDayBtn->isChecked())
    {
        // 横坐标为日间质控
        //ui->QcGraphic->ui->XAxisNameLab->setText(tr("日间质控"));
    }
    else if (ui->QcXCntBtn->isChecked())
    {
        // 横坐标为日内质控
        //ui->QcGraphic->ui->XAxisNameLab->setText(tr("日内质控"));
    }
    else
    {
        ULOG(LOG_WARN, "%s(), No qc type", __FUNCTION__);
        return;
    }

    // 更新质控图
    UpdateQcGraphic();
}

///
/// @brief
///     质控信息表中图表下拉框改变
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月10日，新建函数
///
void QcGraphLJWidget::OnGraphicComboChanged()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取信号发送者
    QComboBox* pSnderCombo = qobject_cast<QComboBox*>(sender());

    // 判断转换是否成功
    if (pSnderCombo == Q_NULLPTR)
    {
        // 参数不合法
        ULOG(LOG_ERROR, "%s(), pSnderCombo == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 获取选中项
    int iGraphicIndex = pSnderCombo->currentData().toInt();
    if (iGraphicIndex != COMBO_INDEX_OF_GRAPHIC_EMPTY)
    {
        // 质控信息表中选中图表与之相同的被重置
        for (int iRow = 0; iRow < ui->QcInfoTable->rowCount(); iRow++)
        {
            // 获取对应项
            QTableWidgetItem* pItem = ui->QcInfoTable->item(iRow, COL_INDEX_GRAPHIC_INDEX_OF_QC_INFO_TABLE);
            if (pItem == Q_NULLPTR)
            {
                continue;
            }

            // 获取对应下拉框
            QComboBox* pCombo = qobject_cast<QComboBox*>(ui->QcInfoTable->cellWidget(iRow, COL_INDEX_GRAPHIC_INDEX_OF_QC_INFO_TABLE));
            if (pCombo == Q_NULLPTR)
            {
                continue;
            }

            // 如果为索引改变下拉框，则跳过
            if (pCombo == pSnderCombo)
            {
                continue;
            }

            // 如果选中图表与改变者相同则重置
            if (pCombo->currentData().toInt() == iGraphicIndex)
            {
                pCombo->setCurrentIndex(pCombo->findData(COMBO_INDEX_OF_GRAPHIC_EMPTY));
            }
        }
    }

    // 更新质控结果
    UpdateQcResult();
    // 关联选择质控图表x信息
    ui->QcResultTabWidget->setCurrentIndex(iGraphicIndex - 1);

    // 更新质控图
    UpdateQcGraphic();
}

///
/// @brief
///     展开按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月1日，新建函数
///
void QcGraphLJWidget::OnPopBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 定义伸缩距离
    const int iStrech = 519;

    // 判断当前状态
    if (ui->PopBtn->property("pop_status").toBool())
    {
        // 处于展开状态
        ui->QcInfoTable->resize(ui->QcInfoTable->width(), ui->QcInfoTable->height() - iStrech);
        ui->line_v_3->resize(ui->line_v_3->width(), ui->line_v_3->height() - iStrech);
        ui->line_v_4->resize(ui->line_v_4->width(), ui->line_v_4->height() - iStrech);
        ui->PopBtn->move(ui->PopBtn->x(), ui->PopBtn->y() - iStrech);

        // 更新状态
        ui->PopBtn->setProperty("pop_status", false);

        // 更新文本和图标
        ui->PopBtn->setText(tr("展开"));
        ui->PopBtn->setIcon(QIcon(":/Leonis/resource/image/icon-pop-push-down.png"));
    }
    else
    {
        // 处于收起状态
        ui->QcInfoTable->resize(ui->QcInfoTable->width(), ui->QcInfoTable->height() + iStrech);
        ui->line_v_3->resize(ui->line_v_3->width(), ui->line_v_3->height() + iStrech);
        ui->line_v_4->resize(ui->line_v_4->width(), ui->line_v_4->height() + iStrech);
        ui->PopBtn->move(ui->PopBtn->x(), ui->PopBtn->y() + iStrech);

        // 更新状态
        ui->PopBtn->setProperty("pop_status", true);

        // 更新文本
        ui->PopBtn->setText(tr("收起"));
        ui->PopBtn->setIcon(QIcon(":/Leonis/resource/image/icon-pop-push-up.png"));
    }
}

///
/// @brief
///     细节按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月10日，新建函数
///
void QcGraphLJWidget::OnDetailBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取质控结果当前图标索引
    int iTabIndex = ui->QcResultTabWidget->currentIndex();
    QTableView* pResultTbl = Q_NULLPTR;
    QStandardItemModel* pResultModel = Q_NULLPTR;
    if (iTabIndex == TAB_INDEX_OF_GRAPHIC_1)
    {
        pResultTbl = ui->QcResultTbl1;
        pResultModel = m_pTbl1Model;
    }
    else if (iTabIndex == TAB_INDEX_OF_GRAPHIC_2)
    {
        pResultTbl = ui->QcResultTbl2;
        pResultModel = m_pTbl2Model;
    }
    else if (iTabIndex == TAB_INDEX_OF_GRAPHIC_3)
    {
        pResultTbl = ui->QcResultTbl3;
        pResultModel = m_pTbl3Model;
    }
    else if (iTabIndex == TAB_INDEX_OF_GRAPHIC_4)
    {
        pResultTbl = ui->QcResultTbl4;
        pResultModel = m_pTbl4Model;
    }
    else
    {
        ULOG(LOG_WARN, "%s(), unknown tab index.", __FUNCTION__);
        return;
    }

    // 判断质控结果当前选中项是否为有效索引
    QModelIndex selIndex = pResultTbl->currentIndex();
    if (!selIndex.isValid() || (pResultModel->item(selIndex.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE) == Q_NULLPTR))
    {
        ULOG(LOG_WARN, "%s(), select invalid index!", __FUNCTION__);
        return;
    }

    // 获取选中质控结果数据
    QC_RESULT_INFO stuQcResult = pResultModel->item(selIndex.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE)->data().value<QC_RESULT_INFO>();

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
void QcGraphLJWidget::OnOutCtrlBtnClicked()
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
/// @brief
///     更新靶值按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月16日，新建函数
///
void QcGraphLJWidget::OnTargetValUpdateBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 弹框询问用户是否确认更新靶值SD
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("更新靶值SD"), tr("确定依据当前质控数据更新当前的靶值、SD？"), TipDlgType::TWO_BUTTON));
        if (pTipDlg->exec() == QDialog::Rejected)
        {
            // 取消操作
            ULOG(LOG_INFO, "%s(), Canceled!", __FUNCTION__);
            return;
        }
    }

    // 获取质控结果当前图表索引
//     int iGraphicIndex = COMBO_INDEX_OF_GRAPHIC_EMPTY;
//     int iTabIndex = ui->QcResultTabWidget->currentIndex();
//     if (iTabIndex == TAB_INDEX_OF_GRAPHIC_1)
//     {
//         iGraphicIndex = COMBO_INDEX_OF_GRAPHIC_1;
//     }
//     else if (iTabIndex == TAB_INDEX_OF_GRAPHIC_2)
//     {
//         iGraphicIndex = COMBO_INDEX_OF_GRAPHIC_2;
//     }
//     else if (iTabIndex == TAB_INDEX_OF_GRAPHIC_3)
//     {
//         iGraphicIndex = COMBO_INDEX_OF_GRAPHIC_3;
//     }
//     else if (iTabIndex == TAB_INDEX_OF_GRAPHIC_4)
//     {
//         iGraphicIndex = COMBO_INDEX_OF_GRAPHIC_4;
//     }
//     else
//     {
//         ULOG(LOG_INFO, "%s(), unknown tab index.", __FUNCTION__);
//         return;
//     }
// 
//     // 获取对应图表的质控信息
//     QC_DOC_CONC_INFO stuQcInfo;
//     for (int iRow = 0; iRow < ui->QcInfoTable->rowCount(); iRow++)
//     {
//         // 获取对应项
//         QTableWidgetItem* pItem = ui->QcInfoTable->item(iRow, COL_INDEX_GRAPHIC_INDEX_OF_QC_INFO_TABLE);
//         if (pItem == Q_NULLPTR)
//         {
//             continue;
//         }
// 
//         // 获取对应下拉框
//         QComboBox* pCombo = qobject_cast<QComboBox*>(ui->QcInfoTable->cellWidget(iRow, COL_INDEX_GRAPHIC_INDEX_OF_QC_INFO_TABLE));
//         if (pCombo == Q_NULLPTR)
//         {
//             continue;
//         }
// 
//         // 如果对应图表不同，则跳过
//         if (pCombo->currentData().toInt() != iGraphicIndex)
//         {
//             continue;
//         }
// 
//         // 获取对应质控信息
//         stuQcInfo = ui->QcInfoTable->item(iRow, COL_INDEX_DB_NO_OF_QC_INFO_TABLE)->data(Qt::UserRole).value<QC_DOC_CONC_INFO>();
//         break;
//     }
// 
//     // 如果质控信息未找到，则返回
//     if (stuQcInfo.strID.isEmpty() || stuQcInfo.strQcLot.isEmpty())
//     {
//         return;
//     }

    // 获取当前选中质控品
    QModelIndex selIndex = ui->QcInfoTable->currentIndex();
    if (!selIndex.isValid())
    {
        ULOG(LOG_INFO, "%s(), Invalid index.", __FUNCTION__);
        return;
    }

    // 获取对应项
    QTableWidgetItem* pSelItem = ui->QcInfoTable->item(selIndex.row(), COL_INDEX_DB_NO_OF_QC_INFO_TABLE);
    if (pSelItem == Q_NULLPTR)
    {
        ULOG(LOG_INFO, "%s(), pSelItem == Q_NULLPTR.", __FUNCTION__);
        return;
    }

    // 获取对应的质控品信息
    QC_DOC_CONC_INFO stuQcInfo = pSelItem->data(Qt::UserRole).value<QC_DOC_CONC_INFO>();

    // 执行更新靶值SD
	::tf::ResultLong ret;
    if (!gUiAdapterPtr(m_strCurDevName, m_strCurDevGroupName)->UpdateQcTargetValSD(ret, stuQcInfo, ui->AssayCombo->currentText()))
    {
        ULOG(LOG_ERROR, "%s(), UpdateQcTargetValSD failed.", __FUNCTION__);
		QString errInfo = ret.result == tf::ThriftResult::THRIFT_RESULT_MODIFY_USEING ? tr("正在质控该项目，不能更新！") : tr("更新靶值失败!");
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(errInfo));
		pTipDlg->exec();
        return;
    }

    // 通知质控品项目更新
    UpdateQcInfo();

    // 通知质控品文档更新
    std::vector<tf::QcDocUpdate> vecQcDocUpdate;
    POST_MESSAGE(MSG_ID_QC_DOC_INFO_UPDATE, vecQcDocUpdate);
}

///
/// @brief
///     获取待打印的质控L-J信息
///
///
/// @par History:
/// @li 6889/ChenWei，2023年12月18日，新建函数
///
bool QcGraphLJWidget::GetQCLJInfo(QCLJInfo& Info)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 记录原图表选项
    QMap<QString, int> mapItemComboIndex;
    // 遍历质控信息表，加载对应质控结果表
    for (int iRow = 0; iRow < ui->QcInfoTable->rowCount(); iRow++)
    {
        // 获取对应项
        QTableWidgetItem* pItem = ui->QcInfoTable->item(iRow, COL_INDEX_DB_NO_OF_QC_INFO_TABLE);
        if (pItem == Q_NULLPTR)
        {
            continue;
        }

        // 获取对应下拉框
        QComboBox* pCombo = qobject_cast<QComboBox*>(ui->QcInfoTable->cellWidget(iRow, COL_INDEX_GRAPHIC_INDEX_OF_QC_INFO_TABLE));
        if (pCombo == Q_NULLPTR)
        {
            continue;
        }

        // 获取对应质控信息
        QC_DOC_CONC_INFO stuQcInfo = ui->QcInfoTable->item(iRow, COL_INDEX_DB_NO_OF_QC_INFO_TABLE)->data(Qt::UserRole).value<QC_DOC_CONC_INFO>();

        // 如果数据库小于0则跳过(待完善)
        if (stuQcInfo.strID.isEmpty())
        {
            continue;
        }

        // 记录项目图表映射
        mapItemComboIndex.insert(stuQcInfo.strID, pCombo->currentData().toInt());

        // 如果有选中图表，则置为不选中任何图表（清空质控图信息）
        if (pCombo->currentData().toInt() != int(COMBO_INDEX_OF_GRAPHIC_EMPTY))
        {
            pCombo->setCurrentIndex(pCombo->findData(COMBO_INDEX_OF_GRAPHIC_EMPTY));
        }
    }

    // 当前选中项切换到图表1
    // 获取质控品信息
    // 获取当前选中质控品
    QModelIndex selIndex = ui->QcInfoTable->currentIndex();
    if (!selIndex.isValid())
    {
        ULOG(LOG_INFO, "%s(), Invalid index.", __FUNCTION__);
        return false;
    }

    // 获取对应项
    QTableWidgetItem* pSelItem = ui->QcInfoTable->item(selIndex.row(), COL_INDEX_DB_NO_OF_QC_INFO_TABLE);
    if (pSelItem == Q_NULLPTR)
    {
        ULOG(LOG_INFO, "%s(), pSelItem == Q_NULLPTR.", __FUNCTION__);
        return false;
    }

    // 获取对应的质控品信息
    QC_DOC_CONC_INFO stuQcInfo = pSelItem->data(Qt::UserRole).value<QC_DOC_CONC_INFO>();

    // 获取对应下拉框
    QComboBox* pCombo = qobject_cast<QComboBox*>(ui->QcInfoTable->cellWidget(selIndex.row(), COL_INDEX_GRAPHIC_INDEX_OF_QC_INFO_TABLE));
    if (pCombo == Q_NULLPTR)
    {
        return false;
    }
    int iComboIdx = pCombo->currentData().toInt();
    int iOldTabIndex = ui->QcResultTabWidget->currentIndex();
    pCombo->setCurrentIndex(pCombo->findData(COMBO_INDEX_OF_GRAPHIC_1));

    // 获取引用
    QCLJInfo& info = Info;

    // 导出图片
    ui->QcGraphic->SetPrintMode(true, McQcLjGraphic::PRINT_CURVE_QC_1);
    QImage printImg(ui->QcGraphic->ui->QcGraphic->size(), QImage::Format_RGB32);
    ui->QcGraphic->ui->QcGraphic->render(&printImg);
    ui->QcGraphic->SetPrintMode(false);

    // 将图片转换为字符串
    QByteArray ba;
    QBuffer buff(&ba);
    buff.open(QIODevice::WriteOnly);
    printImg.save(&buff, "PNG");
    QString strPrintImg(ba.toBase64());
    info.strLJImage = strPrintImg.toStdString();

    // 获取图表索引
    McQcLjGraphic::PRINT_CURVE enPrintCurve;
    McQcLjGraphic::PRINT_CURVE enNoCalcCurve;
    int iTabIndex = ui->QcResultTabWidget->currentIndex();
    QStandardItemModel* pResultModel = Q_NULLPTR;
    if (iTabIndex == TAB_INDEX_OF_GRAPHIC_1)
    {
        enPrintCurve = McQcLjGraphic::PRINT_CURVE_QC_1;
        pResultModel = m_pTbl1Model;
    }
    else if (iTabIndex == TAB_INDEX_OF_GRAPHIC_2)
    {
        enPrintCurve = McQcLjGraphic::PRINT_CURVE_QC_2;
        pResultModel = m_pTbl2Model;
    }
    else if (iTabIndex == TAB_INDEX_OF_GRAPHIC_3)
    {
        enPrintCurve = McQcLjGraphic::PRINT_CURVE_QC_3;
        pResultModel = m_pTbl3Model;
    }
    else if (iTabIndex == TAB_INDEX_OF_GRAPHIC_4)
    {
        enPrintCurve = McQcLjGraphic::PRINT_CURVE_QC_4;
        pResultModel = m_pTbl4Model;
    }
    else
    {
        return false;
    }

    // 查询质控结果
    // 构造查询条件
    QC_RESULT_QUERY_COND    qryCond;

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
    qryCond.strQcDocID = stuQcInfo.strID;
    //qryCond.__set_qcDocId();

    // 执行查询
    // 查询质控结果
    QList<QC_RESULT_INFO>   qryResp;
    if (!gUiAdapterPtr(m_strCurDevName, m_strCurDevGroupName, iModuleNo)->QueryQcRltInfo(qryCond, qryResp))
    {
        ULOG(LOG_ERROR, "QueryQcRltInfo failed.");
        return false;
    }

    // 构造查询时间区间
    // 设置起止日期
    QDate startDate = ui->QcStartDateEdit->date();
    QDate endDate = ui->QcEndDateEdit->date();
    QString strDateRange = startDate.toString(UI_DATE_FORMAT) + " - " + endDate.toString(UI_DATE_FORMAT);

    // 构造质控信息
    info.strModelName = (m_strCurDevGroupName + m_strCurDevName).toStdString();                 // 模块
    info.strItemName = ui->AssayCombo->currentText().toStdString();                             // 项目名称
    auto assayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(ui->AssayCombo->currentText().toStdString());
    if (assayInfo == nullptr)
    {
		ULOG(LOG_ERROR, "%s(), assayInfo is empty.", __FUNCTION__);
		return false;
    }
	info.strItemName = assayInfo->printName;

    // 显示精度
    int pricisionNum = CommonInformationManager::GetInstance()->GetPrecisionNum(assayInfo->assayName);

    info.strQCDate = strDateRange.toStdString();                        // 质控日期
    info.strQCID = stuQcInfo.strQcNo.toStdString();                     // 质控品编号
    info.strQCName = stuQcInfo.strQcName.toStdString();                 // 质控品名称
    info.strQcBriefName = stuQcInfo.strQcBriefName.toStdString();       // 质控品简称
    info.strQcSourceType = stuQcInfo.strQcSourceType.toStdString();     // 质控品类型
    info.strQcLevel = stuQcInfo.strQcLevel.toStdString();				// 质控品水平
    info.strQCLot = stuQcInfo.strQcLot.toStdString();                   // 质控品批号
    info.strTargetMean = QString::number(stuQcInfo.dQcTargetVal, 'f', pricisionNum).toStdString();        // 目标靶值
    info.strTargetSD = QString::number(stuQcInfo.dQcSD, 'f', pricisionNum).toStdString();                 // 目标标准方差(SD)
    info.strTargetCV = QString::number(stuQcInfo.dQcCV, 'f', pricisionNum).toStdString();                 // 目标变异系数（CV）
    info.strActualMean = QString::number(stuQcInfo.dQcCalcTargetVal, 'f', pricisionNum).toStdString();    // 计算靶值
    info.strActualSD = QString::number(stuQcInfo.dQcCalcSD, 'f', pricisionNum).toStdString();             // 计算标准方差(SD)
    info.strActualCV = QString::number(stuQcInfo.dQcCalcCV, 'f', pricisionNum).toStdString();             // 计算变异系数（CV）
    info.strQcRltCount = stuQcInfo.strQcRltCount.toStdString();                        // 数量
    info.strChart = QString::number(iComboIdx).toStdString();                          // 图表序号

    // 判断是否显示不计算点
    bool bShowNoCalcPt = ui->QcGraphic->IsShowNoCalcPoint();

    // 遍历数据
    int iRow = 0;
    for (const QC_RESULT_INFO& stuQcResult : qryResp)
    {
        // 如果不显示不计算点，则忽略不计算的结果
        if (!bShowNoCalcPt && !stuQcResult.bCalculated)
        {
            continue;
        }

        // 构造打印数据
        QCResult result;
        result.strDisplayIndex = QString::number(iRow + 1).toStdString();                                // 显示序号
        result.strQCDate = stuQcResult.strQcTime.toStdString();         // 质控时间
        result.strResult = QString::number(stuQcResult.dQcResult, 'f', pricisionNum).toStdString();   // 结果值
        info.vecResult.push_back(result);

        // 行号自增
        ++iRow;
    }

    // 判断是否有数据需要打印
    if (info.vecResult.empty())
    {
        ULOG(LOG_INFO, "%s(), data is empty.", __FUNCTION__);
        return false;
    }

    // 还原表格
    // 遍历质控信息表，加载对应质控结果表
    for (int iRow = 0; iRow < ui->QcInfoTable->rowCount(); iRow++)
    {
        // 获取对应项
        QTableWidgetItem* pItem = ui->QcInfoTable->item(iRow, COL_INDEX_DB_NO_OF_QC_INFO_TABLE);
        if (pItem == Q_NULLPTR)
        {
            continue;
        }

        // 获取对应下拉框
        QComboBox* pCombo = qobject_cast<QComboBox*>(ui->QcInfoTable->cellWidget(iRow, COL_INDEX_GRAPHIC_INDEX_OF_QC_INFO_TABLE));
        if (pCombo == Q_NULLPTR)
        {
            continue;
        }

        // 获取对应质控信息
        QC_DOC_CONC_INFO stuQcInfo = ui->QcInfoTable->item(iRow, COL_INDEX_DB_NO_OF_QC_INFO_TABLE)->data(Qt::UserRole).value<QC_DOC_CONC_INFO>();
        // 如果数据库小于0则跳过(待完善)
        if (stuQcInfo.strID.isEmpty())
        {
            continue;
        }

        // 查找选中项
        auto it = mapItemComboIndex.find(stuQcInfo.strID);
        if (it == mapItemComboIndex.end())
        {
            continue;
        }
        int iCbIdx = it.value();

        // 如果对应图表未改变，则跳过
        if (pCombo->currentData().toInt() == iCbIdx)
        {
            continue;
        }

        // 恢复
        pCombo->blockSignals(true);
        pCombo->setCurrentIndex(pCombo->findData(iCbIdx));
        pCombo->blockSignals(false);
    }

    // 恢复tab
    ui->QcResultTabWidget->setCurrentIndex(iOldTabIndex);

    // 刷新质控结果
    UpdateQcResult();
    UpdateQcGraphic();
    return true;
}

///
/// @brief
///     获取待打印的质控L-J信息
///
///
/// @par History:
/// @li 6889/ChenWei，2023年12月18日，新建函数
///
bool QcGraphLJWidget::GetQCDailyInfo(QCDailyInfo& Info, QDate QcDate)
{
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

    // 利用日期查询结果
    // 构造查询条件
    QC_RESULT_QUERY_COND qryCond;
    qryCond.startDate = QcDate;
    qryCond.endDate = QcDate;

    // 执行查询
    // 查询质控结果
    QList<QC_RESULT_INFO>   qryResp;
    if (!gUiAdapterPtr(m_strCurDevName, m_strCurDevGroupName, iModuleNo)->QueryQcRltInfo(qryCond, qryResp))
    {
        ULOG(LOG_ERROR, "QueryQcRltInfo failed.");
        return false;
    }

    // 如果没有结果返回
    if (qryResp.isEmpty())
    {
        ULOG(LOG_INFO, "Empty data.");
        return false;
    }

    // 获取质控品信息
    // 获取当前选中质控品
    QModelIndex selIndex = ui->QcInfoTable->currentIndex();
    if (!selIndex.isValid())
    {
        ULOG(LOG_INFO, "%s(), Invalid index.", __FUNCTION__);
        return false;
    }

    // 获取对应项
    QTableWidgetItem* pSelItem = ui->QcInfoTable->item(selIndex.row(), COL_INDEX_DB_NO_OF_QC_INFO_TABLE);
    if (pSelItem == Q_NULLPTR)
    {
        ULOG(LOG_INFO, "%s(), pSelItem == Q_NULLPTR.", __FUNCTION__);
        return false;
    }

    // 展示精度
    QString strCurAssayName = ui->AssayCombo->currentText();
    int precisonNum = CommonInformationManager::GetInstance()->GetPrecisionNum(strCurAssayName.toStdString());

    // 遍历结果
    for (const QC_RESULT_INFO& stuRlt : qryResp)
    {
        // 获取质控品信息
        std::shared_ptr<tf::QcDoc> spQcDocInfo = DataManagerQc::GetInstance()->GetQcDocByID(stuRlt.strQcDocID.toLongLong());
        if (spQcDocInfo == Q_NULLPTR)
        {
            continue;
        }

        QCDailyResult Result;
        Result.strQCDate = stuRlt.strQcTime.toStdString();                  ///< 质控时间
        Result.strQCID = spQcDocInfo->sn;                                   ///< 质控品编号
        Result.strQCName = spQcDocInfo->name;                               ///< 质控品名称
        Result.strQCBriefName = spQcDocInfo->shortName;                     ///< 质控品简称
        Result.strQCSourceType = ConvertTfEnumToQString(tf::SampleSourceType::type(spQcDocInfo->sampleSourceType)).toStdString();   ///< 质控品类型
        Result.strQCLevel = QString::number(spQcDocInfo->level).toStdString();///< 质控品水平
        Result.strQCLot = spQcDocInfo->lot;                                 ///< 质控品批号
        Result.strItemName = stuRlt.strAssayName.toStdString();             ///< 项目名称
        Result.strResult = QString::number(stuRlt.dQcResult, 'f', precisonNum).toStdString();   ///< 结果值
        Result.strTargetMean = QString::number(stuRlt.dQcTargetVal, 'f', precisonNum).toStdString();    ///< 靶值
        Result.strMaxRange = QString::number(stuRlt.dQcTargetVal + (stuRlt.dQcSD * 3), 'f', precisonNum).toStdString();;                ///< 参考范围高值(3SD)
        Result.strMinRange = QString::number(stuRlt.dQcTargetVal + (stuRlt.dQcSD * (-3)), 'f', precisonNum).toStdString();;                ///< 参考范围低值(-3SD)
        Result.strBreakRule = stuRlt.strOutCtrlRule.toStdString();          ///< 失控规则

        // 单位
        if (gUiAdapterPtr()->GetDeviceClasssify(m_strCurDevName, m_strCurDevGroupName) == DEVICE_CLASSIFY_IMMUNE)
        {
            Result.strUnit = ImQcItemRltDetailDlg::GetUnit(stuRlt.strRltDetailID.toLongLong()).toStdString();
        }

        Info.vecResult.push_back(Result);
    }

    return true;
}

///
/// @brief
///     打印按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2022年7月29日，新建函数
/// @li 6889/ChenWei，2023年3月31日，修改函数
/// @li 4170/TangChuXian，2023年4月11日，真实数据填充
///
void QcGraphLJWidget::OnPrintBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
	// 判空提示
	if (ui->QcInfoTable->rowCount() <= 0)
	{
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("没有可以打印的数据!")));
		pTipDlg->exec();
		return;
	}

    std::shared_ptr<QcPrintSetDlg> pSetDlg = std::make_shared<QcPrintSetDlg>();
    if (pSetDlg->exec() == QDialog::Rejected)
    {
        return;
    }

    if (pSetDlg->isSelectPrintLJ()) // 打印LJ图
    {
        // 构造质控信息
        QCLJInfo info;
        info.strInstrumentModel = "i 6000";                                                         // 仪器型号
        QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
        info.strPrintTime = strPrintTime.toStdString();                                             // 打印时间

        if (!GetQCLJInfo(info))
        {
            // 弹框提示导出完成
            std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("没有可以打印的数据!")));
            pTipDlg->exec();
            return;
        }

        // 调用打印接口
        ULOG(LOG_INFO, "GetJsonString");
        std::string strInfo = GetJsonString(info);
        ULOG(LOG_INFO, "Print datas : %s", strInfo);
        std::string unique_id;
        int irect = printcom::printcom_async_print(strInfo, unique_id);
        ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
    }
    else
    {
        QDate QcDate = pSetDlg->getQcDate();
        QCDailyInfo Info;
        Info.strInstrumentModel = "i 6000";                                                         // 仪器型号
        QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
        Info.strPrintTime = strPrintTime.toStdString();                                             // 打印时间
        if (!GetQCDailyInfo(Info, QcDate))
        {
            std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("没有可以打印的数据!")));
            pTipDlg->exec();
            return;
        }

        // 调用打印接口
        std::string strInfo = GetJsonString(Info);
        ULOG(LOG_INFO, "Print datas : %s", strInfo);
        std::string unique_id;
        int irect = printcom::printcom_async_print(strInfo, unique_id);
        ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);

    }
 
    // 弹框提示打印结果
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("数据已发送到打印机!")));
    pTipDlg->exec();
}

///
/// @brief
///     导出按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月11日，新建函数
///
void QcGraphLJWidget::OnExportBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 判空提示
	if (ui->QcInfoTable->rowCount() <= 0)
	{
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("没有可以导出的数据!")));
		pTipDlg->exec();
		return;
	}

    // 构造质控信息
    QCLJInfo Info;
    if (!GetQCLJInfo(Info))
    {
        // 弹框提示导出完成
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("没有可以导出的数据!")));
        pTipDlg->exec();
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
        QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportQCLJInfo.lrxml";
        int irect = printcom::printcom_async_assign_export(strInfo, strFilepath.toStdString(), strUserTmplatePath.toStdString(), unique_id);
        ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
        bRect = true;
    }
    else
    {
        std::shared_ptr<FileExporter> pFileEpt(new FileExporter());
        bRect = pFileEpt->ExportLJInfo(Info, strFilepath);
    }
    
    // 弹框提示导出
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(bRect ? tr("导出完成!") : tr("导出失败！")));
    pTipDlg->exec();
}

///
/// @brief 质控信息表被选中
///
/// @param[in]  curIndex    当前索引
/// @param[in]  prevIndex   之前选中的所有
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年9月20日，新建函数
///
void QcGraphLJWidget::OnQcInfoTableItemSelChanged(const QModelIndex& curIndex, const QModelIndex& prevIndex)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	// 空行，数量为0，计算SD为0, 更新靶值按钮置灰
    if (!curIndex.isValid())
    {
        ui->TargetValUpdateBtn->setEnabled(false);
        ui->export_btn->setEnabled(false);
        ui->PrintBtn->setEnabled(false);
    }

	// 空行
	if (ui->QcInfoTable->item(curIndex.row(), COL_INDEX_DB_NO_OF_QC_INFO_TABLE) == Q_NULLPTR )
	{
		ui->TargetValUpdateBtn->setEnabled(false);
        ui->export_btn->setEnabled(false);
        ui->PrintBtn->setEnabled(false);
		return;
	}

	// 判断计算SD和数量
	if (ui->QcInfoTable->item(curIndex.row(), COL_INDEX_CALCUL_SD_OF_QC_INFO_TABLE) == Q_NULLPTR ||
		ui->QcInfoTable->item(curIndex.row(), COL_INDEX_CALCUL_SD_OF_QC_INFO_TABLE)->text().toDouble() == 0 ||
		ui->QcInfoTable->item(curIndex.row(), COL_INDEX_NUM_OF_QC_INFO_TABLE) == Q_NULLPTR ||
		ui->QcInfoTable->item(curIndex.row(), COL_INDEX_NUM_OF_QC_INFO_TABLE)->text().toDouble() == 0)
	{
		ui->TargetValUpdateBtn->setEnabled(false);
	}
	else
	{
		ui->TargetValUpdateBtn->setEnabled(true);
	}
	
    ui->export_btn->setEnabled(true);
    ui->PrintBtn->setEnabled(true);
}

///
/// @brief
///     质控结果表单元格被点击
///
/// @param[in]  clickIndex   被点击的索引
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月16日，新建函数
///
void QcGraphLJWidget::OnQcResultTblItemClicked(const QModelIndex& clickIndex)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 检查参数
    if (!clickIndex.isValid() || (clickIndex.column() != COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE))
    {
        // 参数不合法
        ULOG(LOG_WARN, "%s(), invalid index", __FUNCTION__);
        return;
    }

    // 没有权限直接返回
    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();
    if (!userPms->IsPermisson(PSM_IM_QC_LJ_CHECKCALC))
    {
        // 没有权限操作
        ULOG(LOG_WARN, "%s(), no permission", __FUNCTION__);
        return;
    }

    // 获取信号发送者
    QTableView* pTblView = qobject_cast<QTableView*>(sender());

    // 判断转换是否成功
    if (pTblView == Q_NULLPTR)
    {
        // 参数不合法
        ULOG(LOG_WARN, "%s(), invalid index", __FUNCTION__);
        return;
    }

    // 获取点击项
    int iGraphicIndex = COMBO_INDEX_OF_GRAPHIC_EMPTY;
    if (pTblView == ui->QcResultTbl1)
    {
        iGraphicIndex = COMBO_INDEX_OF_GRAPHIC_1;
    }
    else if (pTblView == ui->QcResultTbl2)
    {
        iGraphicIndex = COMBO_INDEX_OF_GRAPHIC_2;
    }
    else if (pTblView == ui->QcResultTbl3)
    {
        iGraphicIndex = COMBO_INDEX_OF_GRAPHIC_3;
    }
    else if (pTblView == ui->QcResultTbl4)
    {
        iGraphicIndex = COMBO_INDEX_OF_GRAPHIC_4;
    }
    else
    {
        ULOG(LOG_WARN, "%s(), unknown tab index.", __FUNCTION__);
        return;
    }

    // 获取模型
    QStandardItemModel* pResultModel = qobject_cast<QStandardItemModel*>(pTblView->model());
    if (pResultModel == Q_NULLPTR)
    {
        // 参数不合法
        ULOG(LOG_WARN, "%s(), pResultModel == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 选中空行则返回
    if (pResultModel->item(clickIndex.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE) == Q_NULLPTR)
    {
        // 选中空行
        ULOG(LOG_INFO, "%s(), pResultModel == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 获取选中质控结果数据
    QC_RESULT_INFO stuQcResult = pResultModel->item(clickIndex.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE)->data().value<QC_RESULT_INFO>();

    // 如果是否计算未改变，则忽略
    bool bCalcChecked = !pResultModel->item(clickIndex.row(), COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE)->icon().isNull();
    if (stuQcResult.bCalculated == bCalcChecked)
    {
        // 忽略点击
        ULOG(LOG_INFO, "%s(), check state matched.", __FUNCTION__);
        pResultModel->item(clickIndex.row(), COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE)->setIcon(bCalcChecked ? QIcon() : QIcon(":/Leonis/resource/image/icon-select.png"));
        bCalcChecked = !bCalcChecked;
    }

    // 执行更新计算点
    if (!gUiAdapterPtr(m_strCurDevName, m_strCurDevGroupName)->UpdateQcCalcPoint(stuQcResult, bCalcChecked))
    {
        ULOG(LOG_ERROR, "%s(), UpdateQcResultExclude failed!!", __FUNCTION__);
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("操作失败!")));
        pTipDlg->exec();
        pResultModel->item(clickIndex.row(), COL_INDEX_CALCULATE_OF_QC_RESULT_TABLE)->setIcon(bCalcChecked ? QIcon() : QIcon(":/Leonis/resource/image/icon-select.png"));
        return;
    }

    // 更新表格内容
    pResultModel->item(clickIndex.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE)->setData(QVariant::fromValue<QC_RESULT_INFO>(stuQcResult));

    // 表格内容居中
    //SetTblTextAlign(ui->QcInfoTable, Qt::AlignCenter);

    // 更新质控信息表
    QTimer::singleShot(0, this, SLOT(UpdateQcInfo()));

    // 更新质控图
    QTimer::singleShot(0, this, SLOT(UpdateQcGraphic()));
}

///
/// @brief
///     质控结果表选中项改变
///
/// @par History:
/// @li 4170/TangChuXian，2021年11月22日，新建函数
///
void QcGraphLJWidget::OnQcResultItemSelChanged()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 失控处理按钮禁能
    ui->OutCtrlBtn->setEnabled(false);
    ui->DetailBtn->setEnabled(false);

    // 质控图清空选中效果
    ui->QcGraphic->SelPointByID(McQcLjGraphic::CURVE_ID_SIZE, UI_INVALID_DB_NO);

    // 获取对应模型
    QTableView*         pQcResultTbl   = Q_NULLPTR;
    QStandardItemModel* pQcResultModel = Q_NULLPTR;
    int iTabIdx = ui->QcResultTabWidget->currentIndex();
    McQcLjGraphic::CURVE_ID enCurveID = McQcLjGraphic::CURVE_ID_SIZE;
    if (iTabIdx == TAB_INDEX_OF_GRAPHIC_1)
    {
        pQcResultTbl = ui->QcResultTbl1;
        pQcResultModel = m_pTbl1Model;
        enCurveID = ui->QcXDayBtn->isChecked() ? McQcLjGraphic::CURVE_ID_QC_1_SUB_PT : McQcLjGraphic::CURVE_ID_QC_1;
    }
    else if (iTabIdx == TAB_INDEX_OF_GRAPHIC_2)
    {
        pQcResultTbl = ui->QcResultTbl2;
        pQcResultModel = m_pTbl2Model;
        enCurveID = ui->QcXDayBtn->isChecked() ? McQcLjGraphic::CURVE_ID_QC_2_SUB_PT : McQcLjGraphic::CURVE_ID_QC_2;
    }
    else if (iTabIdx == TAB_INDEX_OF_GRAPHIC_3)
    {
        pQcResultTbl = ui->QcResultTbl3;
        pQcResultModel = m_pTbl3Model;
        enCurveID = ui->QcXDayBtn->isChecked() ? McQcLjGraphic::CURVE_ID_QC_3_SUB_PT : McQcLjGraphic::CURVE_ID_QC_3;
    }
    else if (iTabIdx == TAB_INDEX_OF_GRAPHIC_4)
    {
        pQcResultTbl = ui->QcResultTbl4;
        pQcResultModel = m_pTbl4Model;
        enCurveID = ui->QcXDayBtn->isChecked() ? McQcLjGraphic::CURVE_ID_QC_4_SUB_PT : McQcLjGraphic::CURVE_ID_QC_4;
    }

    // 如果当前模型为空则返回
    if ((pQcResultModel == Q_NULLPTR) || (pQcResultTbl == Q_NULLPTR))
    {
        // 模型无效
        ULOG(LOG_WARN, "%s(), pQcResultModel == Q_NULLPTR.", __FUNCTION__);
        return;
    }

    // 获取当前项索引
    QModelIndex index = pQcResultTbl->currentIndex();

    // 如果没有选中项或选中项是空行，则返回
    if (!index.isValid() || (pQcResultModel->item(index.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE) == Q_NULLPTR))
    {
        // 忽略
        ULOG(LOG_INFO, "%s(), select invalid index.", __FUNCTION__);
        return;
    }

    // 在控时候，失控处理按钮禁用（警告需确认）
    QC_RESULT_INFO stuQcResult = pQcResultModel->item(index.row(), COL_INDEX_DB_NO_OF_QC_RESULT_TABLE)->data().value<QC_RESULT_INFO>();
    if (stuQcResult.enQcState == QC_STATE_OUT_OF_CTRL)
    {
        ui->OutCtrlBtn->setEnabled(true);
    }

    ui->DetailBtn->setEnabled(true);

    // 质控图选中对应点
    if (!stuQcResult.bCalculated)
    {
        ui->QcGraphic->SelPointByID(McQcLjGraphic::CURVE_ID_QC_NO_CALC, stuQcResult.strID.toLongLong());
    }
    else
    {
        ui->QcGraphic->SelPointByID(enCurveID, stuQcResult.strID.toLongLong());
    }

    // 质控质控时间和结果
    QStringList strTipTextList;
    strTipTextList << tr("结果：") + QString::number(stuQcResult.dQcResult) << tr("时间：") + ToCfgFmtDateTime(stuQcResult.strQcTime);
    ui->QcGraphic->SetSelPointTipText(strTipTextList);
}

///
/// @brief
///     质控图
///
/// @par History:
/// @li 4170/TangChuXian，2021年12月15日，新建函数
///
void QcGraphLJWidget::OnQcGraphicShowPointCBClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 更新点是否显示
//     if (!Hazel::SetDisplayUnCalc(ui->QcGraphic->IsShowNoCalcPoint()))
//     {
//         // 执行失败
//         ALOGE << "Hazel::SetDisplayUnCalc failed.";
//     }
// 
//     if (!Hazel::SetDisplayChild(ui->QcGraphic->ui->ShowSubPointCB->isChecked()))
//     {
//         // 执行失败
//         ALOGE << "Hazel::SetDisplayChild failed.";
//     }

    // 更新质控图
    UpdateQcGraphic();
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
void QcGraphLJWidget::OnQcGraphicSelPtChanged(McQcLjGraphic::CURVE_ID enCurveID, long long iPtID)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取对应模型
    QTableView*         pQcResultTbl = Q_NULLPTR;
    QStandardItemModel* pQcResultModel = Q_NULLPTR;
    int iTabIdx = ui->QcResultTabWidget->currentIndex();
    if (iTabIdx == TAB_INDEX_OF_GRAPHIC_1)
    {
        pQcResultTbl = ui->QcResultTbl1;
        pQcResultModel = m_pTbl1Model;
    }
    else if (iTabIdx == TAB_INDEX_OF_GRAPHIC_2)
    {
        pQcResultTbl = ui->QcResultTbl2;
        pQcResultModel = m_pTbl2Model;
    }
    else if (iTabIdx == TAB_INDEX_OF_GRAPHIC_3)
    {
        pQcResultTbl = ui->QcResultTbl3;
        pQcResultModel = m_pTbl3Model;
    }
    else if (iTabIdx == TAB_INDEX_OF_GRAPHIC_4)
    {
        pQcResultTbl = ui->QcResultTbl4;
        pQcResultModel = m_pTbl4Model;
    }

    // 如果当前模型为空则返回
    if ((pQcResultModel == Q_NULLPTR) || (pQcResultTbl == Q_NULLPTR))
    {
        // 模型无效
        ULOG(LOG_INFO, "%s(), pQcResultModel == Q_NULLPTR.", __FUNCTION__);
        return;
    }

    // 无效点，清空当前选中项
    if (enCurveID == McQcLjGraphic::CURVE_ID_SIZE)
    {
        pQcResultTbl->setCurrentIndex(QModelIndex());
        return;
    }

    // 根据曲线ID判断图表索引
    if (enCurveID == McQcLjGraphic::CURVE_ID_QC_1_SUB_PT || enCurveID == McQcLjGraphic::CURVE_ID_QC_1)
    {
        iTabIdx = TAB_INDEX_OF_GRAPHIC_1;
        pQcResultTbl = ui->QcResultTbl1;
        pQcResultModel = m_pTbl1Model;
    }
    else if (enCurveID == McQcLjGraphic::CURVE_ID_QC_2_SUB_PT || enCurveID == McQcLjGraphic::CURVE_ID_QC_2)
    {
        iTabIdx = TAB_INDEX_OF_GRAPHIC_2;
        pQcResultTbl = ui->QcResultTbl2;
        pQcResultModel = m_pTbl2Model;
    }
    else if (enCurveID == McQcLjGraphic::CURVE_ID_QC_3_SUB_PT || enCurveID == McQcLjGraphic::CURVE_ID_QC_3)
    {
        iTabIdx = TAB_INDEX_OF_GRAPHIC_3;
        pQcResultTbl = ui->QcResultTbl3;
        pQcResultModel = m_pTbl3Model;
    }
    else if (enCurveID == McQcLjGraphic::CURVE_ID_QC_4_SUB_PT || enCurveID == McQcLjGraphic::CURVE_ID_QC_4)
    {
        iTabIdx = TAB_INDEX_OF_GRAPHIC_4;
        pQcResultTbl = ui->QcResultTbl4;
        pQcResultModel = m_pTbl4Model;
    }
    else if (enCurveID == McQcLjGraphic::CURVE_ID_QC_NO_CALC)
    {
        // 不计算点
        auto IsContainQcResult = [iPtID](QStandardItemModel* pModel)
        {
            // 查找对应ID的选项
            for (int iRow = 0; iRow < pModel->rowCount(); iRow++)
            {
                // 空行则跳过
                if (pModel->item(iRow, COL_INDEX_DB_NO_OF_QC_RESULT_TABLE) == Q_NULLPTR)
                {
                    continue;
                }

                // 获取数据
                QC_RESULT_INFO stuQcResult = pModel->item(iRow, COL_INDEX_DB_NO_OF_QC_RESULT_TABLE)->data().value<QC_RESULT_INFO>();

                // ID匹配则选中
                if (stuQcResult.strID.toLongLong() == iPtID)
                {
                    return true;
                }
            }

            return false;
        };

        // 判断点在哪个图标
        if (IsContainQcResult(m_pTbl1Model))
        {
            iTabIdx = TAB_INDEX_OF_GRAPHIC_1;
            pQcResultTbl = ui->QcResultTbl1;
            pQcResultModel = m_pTbl1Model;
        }
        else if (IsContainQcResult(m_pTbl2Model))
        {
            iTabIdx = TAB_INDEX_OF_GRAPHIC_2;
            pQcResultTbl = ui->QcResultTbl2;
            pQcResultModel = m_pTbl2Model;
        }
        else if (IsContainQcResult(m_pTbl3Model))
        {
            iTabIdx = TAB_INDEX_OF_GRAPHIC_3;
            pQcResultTbl = ui->QcResultTbl3;
            pQcResultModel = m_pTbl3Model;
        }
        else if (IsContainQcResult(m_pTbl4Model))
        {
            iTabIdx = TAB_INDEX_OF_GRAPHIC_4;
            pQcResultTbl = ui->QcResultTbl4;
            pQcResultModel = m_pTbl4Model;
        }
        else
        {
            pQcResultTbl->setCurrentIndex(QModelIndex());
            return;
        }
    }
    else
    {
        // 无效曲线，清空选中点
        pQcResultTbl->setCurrentIndex(QModelIndex());
        return;
    }

    // 结果表tab跳转
    ui->QcResultTabWidget->setCurrentIndex(iTabIdx);

    // 选中对应ID的选项
    for (int iRow = 0; iRow < pQcResultModel->rowCount(); iRow++)
    {
        // 空行则跳过
        if (pQcResultModel->item(iRow, COL_INDEX_DB_NO_OF_QC_RESULT_TABLE) == Q_NULLPTR)
        {
            continue;
        }

        // 获取数据
        QC_RESULT_INFO stuQcResult = pQcResultModel->item(iRow, COL_INDEX_DB_NO_OF_QC_RESULT_TABLE)->data().value<QC_RESULT_INFO>();

        // ID匹配则选中
        if (stuQcResult.strID.toLongLong() == iPtID)
        {
            pQcResultTbl->selectRow(iRow);
            break;
        }
    }
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
void QcGraphLJWidget::OnReqPrintTmpVarData(int iVarID, int iSeries, int iItemOffset)
{
//     ULOG(LOG_INFO, "%s()", __FUNCTION__);
//     // 检测参数
//     if (iSeries != UI_PRINT_SERIES_QC_LJ_GRAPHIC || iItemOffset < 0)
//     {
//         // 系列号不匹配
//         ULOG(LOG_INFO, "%s(), iSeries do not match", __FUNCTION__);
//         return;
//     }
// 
//     // 获取图表索引
//     int iComboIdx = -1;
//     int iTabIndex = ui->QcResultTabWidget->currentIndex();
//     if (iTabIndex == TAB_INDEX_OF_GRAPHIC_1)
//     {
//         iComboIdx = COMBO_INDEX_OF_GRAPHIC_1;
//     }
//     else if (iTabIndex == TAB_INDEX_OF_GRAPHIC_2)
//     {
//         iComboIdx = COMBO_INDEX_OF_GRAPHIC_2;
//     }
//     else if (iTabIndex == TAB_INDEX_OF_GRAPHIC_3)
//     {
//         iComboIdx = COMBO_INDEX_OF_GRAPHIC_3;
//     }
//     else if (iTabIndex == TAB_INDEX_OF_GRAPHIC_4)
//     {
//         iComboIdx = COMBO_INDEX_OF_GRAPHIC_4;
//     }
//     else
//     {
//         ALOGW << "iTabIndex do not match";
//         return;
//     }
// 
//     // 获取当前质控信息
//     QC_DOC_CONC_INFO stuQcInfo;
//     for (int iRow = 0; iRow < ui->QcInfoTable->rowCount(); iRow++)
//     {
//         // 获取对应项
//         QTableWidgetItem* pItem = ui->QcInfoTable->item(iRow, COL_INDEX_GRAPHIC_INDEX_OF_QC_INFO_TABLE);
//         if (pItem == Q_NULLPTR)
//         {
//             continue;
//         }
// 
//         // 获取对应下拉框
//         QComboBox* pCombo = qobject_cast<QComboBox*>(ui->QcInfoTable->cellWidget(iRow, COL_INDEX_GRAPHIC_INDEX_OF_QC_INFO_TABLE));
//         if (pCombo == Q_NULLPTR)
//         {
//             continue;
//         }
// 
//         // 如果对应图表不匹配，则跳过
//         if (pCombo->currentData().toInt() != iComboIdx)
//         {
//             continue;
//         }
// 
//         // 获取对应质控信息
//         stuQcInfo = ui->QcInfoTable->item(iRow, COL_INDEX_DB_NO_OF_QC_INFO_TABLE)->data(Qt::UserRole).value<QC_DOC_CONC_INFO>();
//         break;
//     }
// 
//     // 更新打印模板变量映射表
//     UpdatePrintVarStr(stuQcInfo);
// 
//     // 设置返回值
//     auto it = m_mapPrintTmpVarStr.find(iVarID);
//     if (it == m_mapPrintTmpVarStr.end())
//     {
//         return;
//     }
//     PrintDataMng::SetRetOfDataVal(iVarID, it.value());
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
void QcGraphLJWidget::OnReqPrintTmpVarTblHeaderData(int iVarID, int iSeries, int iItemOffset)
{
//     ULOG(LOG_INFO, "%s()", __FUNCTION__);
//     // 检测参数
//     if (iSeries != UI_PRINT_SERIES_QC_LJ_GRAPHIC || iItemOffset < 0)
//     {
//         // 系列号不匹配
//         ALOGI << "iSeries do not match";
//         return;
//     }
// 
//     // 判断表格类型
//     if (iVarID != PRINT_TMP_VAR_TBL_ID_QC_LJ_RESULT_TBL)
//     {
//         // 系列号不匹配
//         ALOGI << "iVarID do not match";
//         return;
//     }
// 
//     // 获取表头
//     QMap<int, QString> mapHeader;
//     for (int i = 0; i < m_pPrintTblQcLjResult->columnCount(); i++)
//     {
//         QStandardItem* pItem = m_pPrintTblQcLjResult->horizontalHeaderItem(i);
//         if (pItem == Q_NULLPTR)
//         {
//             continue;
//         }
// 
//         mapHeader.insert(i, pItem->text());
//     }
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
void QcGraphLJWidget::OnReqPrintTmpVarTblItemData(int iVarID, int iSeries, int iRow, int iCol, int iItemOffset)
{
//     ULOG(LOG_INFO, "%s()", __FUNCTION__);
//     // 检测参数
//     if (iSeries != UI_PRINT_SERIES_QC_LJ_GRAPHIC || iItemOffset < 0)
//     {
//         // 系列号不匹配
//         ALOGI << "iSeries do not match";
//         return;
//     }
// 
//     // 判断表格类型
//     if (iVarID != PRINT_TMP_VAR_TBL_ID_QC_LJ_RESULT_TBL)
//     {
//         // 系列号不匹配
//         ALOGI << "iVarID do not match";
//         return;
//     }
// 
//     // 获取图表索引
//     int iComboIdx = -1;
//     int iTabIndex = ui->QcResultTabWidget->currentIndex();
//     if (iTabIndex == TAB_INDEX_OF_GRAPHIC_1)
//     {
//         iComboIdx = COMBO_INDEX_OF_GRAPHIC_1;
//     }
//     else if (iTabIndex == TAB_INDEX_OF_GRAPHIC_2)
//     {
//         iComboIdx = COMBO_INDEX_OF_GRAPHIC_2;
//     }
//     else if (iTabIndex == TAB_INDEX_OF_GRAPHIC_3)
//     {
//         iComboIdx = COMBO_INDEX_OF_GRAPHIC_3;
//     }
//     else if (iTabIndex == TAB_INDEX_OF_GRAPHIC_4)
//     {
//         iComboIdx = COMBO_INDEX_OF_GRAPHIC_4;
//     }
//     else
//     {
//         ALOGW << "iTabIndex do not match";
//         return;
//     }
// 
//     // 获取当前质控信息
//     QC_DOC_CONC_INFO stuQcInfo;
//     for (int iRow = 0; iRow < ui->QcInfoTable->rowCount(); iRow++)
//     {
//         // 获取对应项
//         QTableWidgetItem* pItem = ui->QcInfoTable->item(iRow, COL_INDEX_GRAPHIC_INDEX_OF_QC_INFO_TABLE);
//         if (pItem == Q_NULLPTR)
//         {
//             continue;
//         }
// 
//         // 获取对应下拉框
//         QComboBox* pCombo = qobject_cast<QComboBox*>(ui->QcInfoTable->cellWidget(iRow, COL_INDEX_GRAPHIC_INDEX_OF_QC_INFO_TABLE));
//         if (pCombo == Q_NULLPTR)
//         {
//             continue;
//         }
// 
//         // 如果对应图表不匹配，则跳过
//         if (pCombo->currentData().toInt() != iComboIdx)
//         {
//             continue;
//         }
// 
//         // 获取对应质控信息
//         stuQcInfo = ui->QcInfoTable->item(iRow, COL_INDEX_DB_NO_OF_QC_INFO_TABLE)->data(Qt::UserRole).value<QC_DOC_CONC_INFO>();
//         break;
//     }
// 
//     // 更新打印表格
//     UpdatePrintTbl(stuQcInfo);
// 
//     // 判断行数列数是否合法
//     if (iRow >= m_pPrintTblQcLjResult->rowCount() || iCol >= m_pPrintTblQcLjResult->columnCount())
//     {
//         ALOGW << "iRow or iCol is invalid.";
//         return;
//     }
// 
//     // 设置返回值
//     PrintDataMng::SetRetOfTblItem(iVarID, m_pPrintTblQcLjResult->item(iRow, iCol)->text());
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
void QcGraphLJWidget::OnReqPrintTmpVarTblRowColCnt(int iVarID, int iSeries, int iItemOffset)
{
//     ULOG(LOG_INFO, "%s()", __FUNCTION__);
//     // 检测参数
//     if (iSeries != UI_PRINT_SERIES_QC_LJ_GRAPHIC || iItemOffset < 0)
//     {
//         // 系列号不匹配
//         ALOGI << "iSeries do not match";
//         return;
//     }
// 
//     // 判断表格类型
//     if (iVarID != PRINT_TMP_VAR_TBL_ID_QC_LJ_RESULT_TBL)
//     {
//         // 系列号不匹配
//         ALOGI << "iVarID do not match";
//         return;
//     }
// 
//     // 获取图表索引
//     int iComboIdx = -1;
//     int iTabIndex = ui->QcResultTabWidget->currentIndex();
//     if (iTabIndex == TAB_INDEX_OF_GRAPHIC_1)
//     {
//         iComboIdx = COMBO_INDEX_OF_GRAPHIC_1;
//     }
//     else if (iTabIndex == TAB_INDEX_OF_GRAPHIC_2)
//     {
//         iComboIdx = COMBO_INDEX_OF_GRAPHIC_2;
//     }
//     else if (iTabIndex == TAB_INDEX_OF_GRAPHIC_3)
//     {
//         iComboIdx = COMBO_INDEX_OF_GRAPHIC_3;
//     }
//     else if (iTabIndex == TAB_INDEX_OF_GRAPHIC_4)
//     {
//         iComboIdx = COMBO_INDEX_OF_GRAPHIC_4;
//     }
//     else
//     {
//         ALOGW << "iTabIndex do not match";
//         return;
//     }
// 
//     // 获取当前质控信息
//     QC_DOC_CONC_INFO stuQcInfo;
//     for (int iRow = 0; iRow < ui->QcInfoTable->rowCount(); iRow++)
//     {
//         // 获取对应项
//         QTableWidgetItem* pItem = ui->QcInfoTable->item(iRow, COL_INDEX_GRAPHIC_INDEX_OF_QC_INFO_TABLE);
//         if (pItem == Q_NULLPTR)
//         {
//             continue;
//         }
// 
//         // 获取对应下拉框
//         QComboBox* pCombo = qobject_cast<QComboBox*>(ui->QcInfoTable->cellWidget(iRow, COL_INDEX_GRAPHIC_INDEX_OF_QC_INFO_TABLE));
//         if (pCombo == Q_NULLPTR)
//         {
//             continue;
//         }
// 
//         // 如果对应图表不匹配，则跳过
//         if (pCombo->currentData().toInt() != iComboIdx)
//         {
//             continue;
//         }
// 
//         // 获取对应质控信息
//         stuQcInfo = ui->QcInfoTable->item(iRow, COL_INDEX_DB_NO_OF_QC_INFO_TABLE)->data(Qt::UserRole).value<QC_DOC_CONC_INFO>();
//         break;
//     }
// 
//     // 更新打印表格
//     UpdatePrintTbl(stuQcInfo);
// 
//     // 设置返回值
//     PrintDataMng::SetRetOfTblRowColCnt(iVarID, QPair<int, int>(m_pPrintTblQcLjResult->rowCount(), m_pPrintTblQcLjResult->columnCount()));
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
void QcGraphLJWidget::OnReqPrintTmpSeriesPrintCnt(int iSeries)
{
//     ULOG(LOG_INFO, "%s()", __FUNCTION__);
//     // 检测参数
//     if (iSeries != UI_PRINT_SERIES_QC_LJ_GRAPHIC)
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
void QcGraphLJWidget::OnReqPrintTmpVarImg(int iVarID, int iSeries, int iItemOffset)
{
//     ULOG(LOG_INFO, "%s()", __FUNCTION__);
//     // 检测参数
//     if (iSeries != UI_PRINT_SERIES_QC_LJ_GRAPHIC)
//     {
//         // 系列号不匹配
//         ALOGI << "iSeries do not match";
//         return;
//     }
// 
//     // 检查ID
//     if (iVarID != PRINT_TMP_VAR_IMG_ID_QC_LJ_RESULT_IMG)
//     {
//         // ID不匹配
//         ALOGI << "iSeries do not match";
//         return;
//     }
// 
//     // 获取图表索引
//     McQcLjGraphic::PRINT_CURVE enPrintCurve;
//     int iTabIndex = ui->QcResultTabWidget->currentIndex();
//     if (iTabIndex == TAB_INDEX_OF_GRAPHIC_1)
//     {
//         enPrintCurve = McQcLjGraphic::PRINT_CURVE_QC_1;
//     }
//     else if (iTabIndex == TAB_INDEX_OF_GRAPHIC_2)
//     {
//         enPrintCurve = McQcLjGraphic::PRINT_CURVE_QC_2;
//     }
//     else if (iTabIndex == TAB_INDEX_OF_GRAPHIC_3)
//     {
//         enPrintCurve = McQcLjGraphic::PRINT_CURVE_QC_3;
//     }
//     else if (iTabIndex == TAB_INDEX_OF_GRAPHIC_4)
//     {
//         enPrintCurve = McQcLjGraphic::PRINT_CURVE_QC_4;
//     }
//     else
//     {
//         ALOGW << "iTabIndex do not match";
//         return;
//     }
// 
//     // 导出图片
//     ui->QcGraphic->SetPrintMode(true, enPrintCurve);
//     QImage printImg(ui->QcGraphic->ui->QcGraphic->size(), QImage::Format_RGB32);
//     ui->QcGraphic->ui->QcGraphic->render(&printImg);
//     ui->QcGraphic->SetPrintMode(false);
// 
//     // 设置返回值
//     printImg.save("D:/i800/trunk/I 800Analyzer/test1.png", "PNG");
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
void QcGraphLJWidget::OnTabCornerIndexChanged(int iBtnIdx)
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

    // 更新项目列表
    UpdateAssayNameList();

    // 更新质控信息
    UpdateQcInfo();

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
void QcGraphLJWidget::OnQcDocInfoInfoUpdate(std::vector<tf::QcDocUpdate, std::allocator<tf::QcDocUpdate>> vQcDoc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    Q_UNUSED(vQcDoc);
    // 更新项目列表
    ui->AssayCombo->blockSignals(true);
    UpdateAssayNameList();
    ui->AssayCombo->blockSignals(false);

    // 更新质控信息
    UpdateQcInfo();

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
void QcGraphLJWidget::OnQcRltUpdate(QString devSn, int64_t qcDocId, QVector<int64_t> changedId)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 更新项目列表
    ui->AssayCombo->blockSignals(true);
    UpdateAssayNameList();
    ui->AssayCombo->blockSignals(false);

    // 更新质控信息
    UpdateQcInfo();

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
void QcGraphLJWidget::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();
    // 更新靶值SD的权限限制
    ui->QcGraphic->ui->ShowSubPointCB->setVisible(userPms->IsPermisson(PSM_IM_QC_LJ_SHOWSUBPOINT) && ui->QcXDayBtn->isChecked());
    ui->QcGraphic->ui->ShowNoCalcQcCB->setVisible(userPms->IsPermisson(PMS_QC_RESULT_SET_SHOWNOCALCULATEPT));
    ui->TargetValUpdateBtn->setVisible(userPms->IsPermisson(PMS_QC_RESULT_UPDATE_SD));
    ui->export_btn->setVisible(userPms->IsPermisson(PSM_QC_LJ_EXPORT));
}

///
/// @brief
///     更新靶值SD按钮使能状态
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月25日，新建函数
///
void QcGraphLJWidget::UpdateTargetValSDBtnEnable()
{
    ULOG(LOG_INFO, __FUNCTION__);
    // 获取当前选中项索引
    QModelIndex modIndex = ui->QcInfoTable->currentIndex();
    if (!modIndex.isValid())
    {
        ui->TargetValUpdateBtn->setEnabled(false);
        ui->export_btn->setEnabled(false);
        ui->PrintBtn->setEnabled(false);
        return;
    }

    // 判断更新靶值SD按钮是否使能
    if (ui->QcInfoTable->item(modIndex.row(), COL_INDEX_DB_NO_OF_QC_INFO_TABLE) == Q_NULLPTR)
    {
        ui->TargetValUpdateBtn->setEnabled(false);
        ui->export_btn->setEnabled(false);
        ui->PrintBtn->setEnabled(false);
        return;
    }

    // 判断计算SD和数量
    if (ui->QcInfoTable->item(modIndex.row(), COL_INDEX_CALCUL_SD_OF_QC_INFO_TABLE) == Q_NULLPTR ||
        ui->QcInfoTable->item(modIndex.row(), COL_INDEX_CALCUL_SD_OF_QC_INFO_TABLE)->text().toDouble() == 0 ||
        ui->QcInfoTable->item(modIndex.row(), COL_INDEX_NUM_OF_QC_INFO_TABLE) == Q_NULLPTR ||
        ui->QcInfoTable->item(modIndex.row(), COL_INDEX_NUM_OF_QC_INFO_TABLE)->text().toDouble() == 0)
    {
        ui->TargetValUpdateBtn->setEnabled(false);
    }
    else
    {
        ui->TargetValUpdateBtn->setEnabled(true);
    }

    ui->export_btn->setEnabled(true);
    ui->PrintBtn->setEnabled(true);
}

///
/// @brief
///     项目信息更新
///
/// @par History:
/// @li 4170/TangChuXian，2023年12月21日，新建函数
///
void QcGraphLJWidget::OnAssayInfoUpdate()
{
    ULOG(LOG_INFO, __FUNCTION__);
    // 更新项目列表
    UpdateAssayNameList();

    // 更新质控信息表
    UpdateQcInfo();

    // 更新质控结果表
    UpdateQcResult();

    // 更新质控图
    UpdateQcGraphic();
}

///
/// @brief
///     更新质控信息下拉框
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月26日，新建函数
///
void QcGraphLJWidget::UpdateQcInfoCombo()
{
    static bool s_bDisConnect = false;
    if (!m_bNeedUpdateCombo)
    {
        if (s_bDisConnect)
        {
            connect(ui->QcInfoTable->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(UpdateQcInfoCombo()));
            s_bDisConnect = false;
        }
        return;
    }

    m_bNeedUpdateCombo = false;

    // 更新下拉框显示
    // 只对特定行有效
    int iMinRow = ui->QcInfoTable->rowAt(0);
    unsigned int iMaxRow = static_cast<unsigned int>(ui->QcInfoTable->rowAt(ui->QcInfoTable->height()));
    if (iMinRow < 0)
    {
        iMinRow = 0;
    }

    // 循环遍历
    QSet<int> delRow;
    for (auto it = m_mapGraphicComboBuffer.find(iMinRow); it != m_mapGraphicComboBuffer.end(); it++)
    {
        int iRow = it.key();
        if (iRow >= iMaxRow)
        {
            break;
        }

        // 设置下拉框
        if (ui->QcInfoTable->cellWidget(iRow, COL_INDEX_GRAPHIC_INDEX_OF_QC_INFO_TABLE) != Q_NULLPTR)
        {
            continue;
        }

        // 设置
        ui->QcInfoTable->setCellWidget(iRow, COL_INDEX_GRAPHIC_INDEX_OF_QC_INFO_TABLE, it.value());
    }

    m_bNeedUpdateCombo = false;

    // 移除对应行
    for (int iDelRow : delRow)
    {
        m_mapGraphicComboBuffer.remove(iDelRow);
    }

    // 下一次触发
    QTimer::singleShot(300, this, SLOT(UpdateQcInfoCombo()));
    if (!s_bDisConnect)
    {
        disconnect(ui->QcInfoTable->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(UpdateQcInfoCombo()));
        s_bDisConnect = true;
    }
}

///
/// @brief
///     清空质控信息下拉框
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月26日，新建函数
///
void QcGraphLJWidget::ClearQcInfoCombo()
{
    ULOG(LOG_INFO, __FUNCTION__);
    for (auto it = m_mapGraphicComboBuffer.begin(); it != m_mapGraphicComboBuffer.end(); it++)
    {
        // 释放对象
        it.value()->deleteLater();
    }

    // 清空缓存
    m_mapGraphicComboBuffer.clear();
}

///
/// @brief
///     质控信息表格显示范围改变
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月26日，新建函数
///
void QcGraphLJWidget::OnQcInfoTblDisplayRangeChanged()
{
    // 设置更新标志
    m_bNeedUpdateCombo = true;
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
void QcGraphLJWidget::UpdateSubDevReadioBtnGroup(const QStringList& subDevList)
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
            if (m_strCurSubDevReadioName != devBtn->text())
            {
                m_strCurSubDevReadioName = devBtn->text();
            }
			initFirst = true;
		}

		// 添加进布局
		ui->horizontalLayout_subDevs->addWidget(devBtn);

	}
	ui->horizontalLayout_subDevs->addSpacing(30);

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
QString QcGraphLJWidget::GetCurrentSelectedDevName()
{
	return m_strCurSubDevReadioName;
}

///
/// @brief 对质控结果表带有图标列进行设置
///
/// @param[in]  pTable  需要重新设置的表格
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年12月14日，新建函数
///
void QcGraphLJWidget::ResizeQcResultTableIconColumn(QTableView* pTable)
{
	if (pTable == nullptr)
	{
		return;
	}

	pTable->setColumnWidth(4, 80);

	// 图标列不可修改
	pTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
}
