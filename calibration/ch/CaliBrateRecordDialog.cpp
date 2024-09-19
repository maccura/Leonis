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

/// @file     CaliBrateRecordDialog.h
/// @brief    校准中新增弹出对话框
///
/// @author   5774/WuHongTao
/// @date     2022年2月24日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "CaliBrateRecordDialog.h"
#include "ui_CaliBrateRecordDialog.h"

#include "QVariant"
#include <QStandardItemModel>
#include <QTimer>

#include "thrift/DcsControlProxy.h"
#include "shared/QPostionEdit.h"
#include "shared/QComDelegate.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/mcpixmapitemdelegate.h"
#include "shared/tipdlg.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/CommonInformationManager.h"
#include "src/thrift/cloud/ch/gen-cpp/ch_cloud_defs_types.h"
#include "thrift/ch/ChLogicControlProxy.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "thrift/ise//IseLogicControlProxy.h"

#include "boost/shared_ptr.hpp"
#include "CaliBrateCommom.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/defs.h"
#include "src/public/ch/ChCommon.h"
#include "manager/UserInfoManager.h"

#include "SortHeaderView.h"

// 不区分试剂批号标识
#define REAGENT_LOT_EMPTY_PNG		(":/Leonis/resource/image/icon-distinguish.png")
// 选中图标
#define SELECTED_PNG				(":/Leonis/resource/image/icon-select.png")

// 下载登记表格自定义数据
#define CALIBRATE_DOWN_ASSAY_INDEX                  (Qt::UserRole + 10)                     // 项目行索引记录
#define CALIBRATE_DOWN_SELECT_INDEX                 (Qt::UserRole + 11)                     // 选择行队列索引记录
#define DEFAULT_INPUT_DECIMAL_PLACE                 6		                                // 浓度录入限制默认位数
#define ISE_CONC_INPUT_DECIMAL_PLACE                1		                                // ISE浓度录入限制位数

///
/// @brief 新增对话框的构造函数
///     
///
/// @param[in]  parent  
///
/// @par History:
/// @li 5774/WuHongTao，2020年5月9日，新建函数
///
CaliBrateRecordDialog::CaliBrateRecordDialog(QWidget *parent)
    : BaseDlg(parent),
    m_dataChanged(false),
    m_CalibrateConcMode(nullptr),
	m_CalibrateDownMode(nullptr),
    m_modeType(NEW_MODE),
    m_CaliPostionEditDialog(nullptr),
    m_press(false),
    m_iseCaliConcModel(nullptr),
    m_lastCaliType(-1),
	m_isRequesting(false),
    m_editDelegate(new PositiveNumDelegate(this))
{
    ui = new Ui::CaliBrateRecordDialog();
    ui->setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint);
    ui->snedit->setValidator(new QRegExpValidator(QRegExp(UI_SN), this));
    ui->snedit->setMaxLength(3);
    ui->lotedit->setValidator(new QRegExpValidator(QRegExp(UI_LOT), this));
    ui->lotedit->setMaxLength(7);
	ui->lineEdit__cali_lot->setValidator(new QRegExpValidator(QRegExp(UI_LOT), this));
	ui->lineEdit__cali_lot->setMaxLength(7);
    // 初始化
	Init();
}

void CaliBrateRecordDialog::OnAssayUpdated()
{
    int type = ui->cali_meterial_type->currentData().toInt();

    // 获取表格信息
    GetAssayInfoList(m_AssayItemList, type);

    // 重置表格项目
    ResetRecordAssayTable();

    // 设置代理
    SetTableViewDelegate(type);

    m_lastCaliType = type;
}

///
/// @brief
///     重置（查看/输入）对话框的table表头
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月10日，新建函数
///
void CaliBrateRecordDialog::ResetRecordAssayTable()
{
    if (m_CalibrateConcMode == nullptr)
    {
        m_CalibrateConcMode = new QToolTipModel(true, this);
        m_CalibrateConcMode->clear();
        // 升序 降序 原序
        SortHeaderView *pCaliBrateGroupHeader = new SortHeaderView(Qt::Horizontal, ui->calibrateBrowse);
        pCaliBrateGroupHeader->setStretchLastSection(true);
        ui->calibrateBrowse->setHorizontalHeader(pCaliBrateGroupHeader);
        connect(pCaliBrateGroupHeader, &SortHeaderView::SortOrderChanged, this, [this](int logicIndex, SortHeaderView::SortOrder order) {
            QTableView *view = ui->calibrateBrowse;
            //无效index或NoOrder就设置为默认未排序状态
            if (logicIndex < 0 || order == SortHeaderView::NoOrder) {
                SetAscSortByFirstColumn();
            }
            else
            {
                Qt::SortOrder qOrderFlag = order == SortHeaderView::DescOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
                int iSortRole = (logicIndex == caliBrowseColumnIndex::BROW_ENABLE_SELECT_COLUMN) ? (Qt::UserRole + 1) : Qt::DisplayRole;
                m_CalibrateConcMode->setSortRole(iSortRole);
                view->sortByColumn(logicIndex, qOrderFlag);
            }
        });
        //去掉排序三角样式
        ui->calibrateBrowse->horizontalHeader()->setSortIndicator(-1, Qt::DescendingOrder);
        //-1则还原model默认顺序
        m_CalibrateConcMode->sort(-1, Qt::DescendingOrder);

        ui->calibrateBrowse->verticalHeader()->hide();
        ui->calibrateBrowse->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->calibrateBrowse->setItemDelegate(m_editDelegate);
        ui->calibrateBrowse->setItemDelegateForColumn(caliBrowseColumnIndex::BROW_ASSAY_NAME_COLUMN, new CReadOnlyDelegate(this));
		ui->calibrateBrowse->setItemDelegateForColumn(caliBrowseColumnIndex::BROW_MAIN_UNIT_COLUMN, new CReadOnlyDelegate(this));
		ui->calibrateBrowse->setItemDelegateForColumn(caliBrowseColumnIndex::BROW_ENABLE_SELECT_COLUMN, new CheckBoxDelegate(this));
        ui->calibrateBrowse->setModel(m_CalibrateConcMode);
        ui->calibrateBrowse->setSortingEnabled(true);

        //设置表头
        QStringList headerList;
        headerList << tr("项目名称") << tr("校准品1") << tr("校准品2") << tr("校准品3") << tr("校准品4")
            << tr("校准品5") << tr("校准品6") << tr("单位") << tr("启用");
        m_CalibrateConcMode->setHorizontalHeaderLabels(headerList);
        m_CalibrateConcMode->setRowCount(3);
        m_CalibrateConcMode->setColumnCount(headerList.size());
        //ui->calibrateBrowse->horizontalHeader()->setStyleSheet("QHeaderView::section:last { color: rgb(2,91,199);}");
        m_CalibrateConcMode->setHeaderData(headerList.size() - 1, Qt::Horizontal, QColor::fromRgb(2, 91, 199), Qt::ForegroundRole);
		ui->calibrateBrowse->setColumnWidth(0, 225);
		ui->calibrateBrowse->setColumnWidth(1, 114);
		ui->calibrateBrowse->setColumnWidth(2, 114);
		ui->calibrateBrowse->setColumnWidth(3, 114);
		ui->calibrateBrowse->setColumnWidth(4, 114);
		ui->calibrateBrowse->setColumnWidth(5, 114);
		ui->calibrateBrowse->setColumnWidth(6, 114);
		ui->calibrateBrowse->setColumnWidth(7, 114);
		ui->calibrateBrowse->horizontalHeader()->setStretchLastSection(true);
		ui->calibrateBrowse->horizontalHeader()->setMinimumSectionSize(90);
    }

	if (m_CalibrateDownMode == nullptr)
	{
		m_CalibrateDownMode = new QStandardItemModel(this);
		ui->calibrateDown->verticalHeader()->hide();
		ui->calibrateDown->setSelectionBehavior(QAbstractItemView::SelectItems);
		ui->calibrateDown->setSelectionMode(QAbstractItemView::NoSelection);
		ui->calibrateDown->setEditTriggers(QAbstractItemView::NoEditTriggers);
		//ui->calibrateDown->setItemDelegate(new PositiveNumDelegate(this));

		ui->calibrateDown->setItemDelegateForColumn(caliDownColumnIndex::DOWN_CALIBRATOR_NAME_COLUMN, new CReadOnlyDelegate(this));
		ui->calibrateDown->setItemDelegateForColumn(caliDownColumnIndex::DOWN_CALIBRATOR_LOT_COLUMN, new CReadOnlyDelegate(this));
		ui->calibrateDown->setItemDelegateForColumn(caliDownColumnIndex::DOWN_EXPIRE_DATE_COLUMN, new CReadOnlyDelegate(this));
		ui->calibrateDown->setItemDelegateForColumn(caliDownColumnIndex::DOWN_ASSAY_NAME_COLUMN, new CReadOnlyDelegate(this));
		ui->calibrateDown->setItemDelegateForColumn(caliDownColumnIndex::DOWN_REAGENT_LOT_COLUMN, new CReadOnlyDelegate(this));
		ui->calibrateDown->setItemDelegateForColumn(caliDownColumnIndex::DOWN_SELECT_COLUMN, new McPixmapItemDelegate(this));
		ui->calibrateDown->setModel(m_CalibrateDownMode);

        QStringList headerList_down;
        headerList_down << tr("校准品名称") << tr("校准品批号") << tr("失效日期") << tr("项目名称") << tr("试剂批号") << tr("选择");
        m_CalibrateDownMode->setHorizontalHeaderLabels(headerList_down);
        m_CalibrateDownMode->setColumnCount(headerList_down.size());
        //ui->calibrateDown->horizontalHeader()->setStyleSheet("QHeaderView::section:last { color: rgb(2,91,199);}");
        m_CalibrateDownMode->setHeaderData(headerList_down.size() - 1, Qt::Horizontal, QColor::fromRgb(2, 91, 199), Qt::ForegroundRole);

		ui->calibrateDown->setColumnWidth(caliDownColumnIndex::DOWN_CALIBRATOR_NAME_COLUMN, 256);
		ui->calibrateDown->setColumnWidth(caliDownColumnIndex::DOWN_CALIBRATOR_LOT_COLUMN, 256);
		ui->calibrateDown->setColumnWidth(caliDownColumnIndex::DOWN_EXPIRE_DATE_COLUMN, 256);
		ui->calibrateDown->setColumnWidth(caliDownColumnIndex::DOWN_ASSAY_NAME_COLUMN, 256);
		ui->calibrateDown->setColumnWidth(caliDownColumnIndex::DOWN_REAGENT_LOT_COLUMN, 256);
		ui->calibrateDown->setColumnWidth(caliDownColumnIndex::DOWN_SELECT_COLUMN, 256);

		ui->calibrateDown->horizontalHeader()->setStretchLastSection(true);
		ui->calibrateDown->horizontalHeader()->setMinimumSectionSize(200);
		

	}

    m_CalibrateConcMode->removeRows(0, m_CalibrateConcMode->rowCount());
	m_CalibrateDownMode->removeRows(0, m_CalibrateConcMode->rowCount());

	// 去除信号关联，防止初始化过程中出现问题
	disconnect(m_CalibrateConcMode, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(updateDataList(QStandardItem*)));
	int row = 0;
    for (const auto& assayInfo : m_AssayItemList)
	{

        std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(get<0>(assayInfo));
        if (spAssayInfo == nullptr)
            continue;

		// 默认所有项目均启用
		QStandardItem* item = new QStandardItem();
		item->setData(true);
		m_CalibrateConcMode->setItem(row, caliBrowseColumnIndex::BROW_ENABLE_SELECT_COLUMN, item);

		// 设置项目名称
        QStandardItem* nameItem = new QStandardItem(get<0>(assayInfo).c_str());
        //QString&& varString = QString::number(spAssayInfo->assayCode).rightJustified(10, '0');
        m_CalibrateConcMode->setItem(row, caliBrowseColumnIndex::BROW_ASSAY_NAME_COLUMN, nameItem);
        // 存储项目编号（用户首列默认排序）
        m_CalibrateConcMode->setData(m_CalibrateConcMode->index(row, caliBrowseColumnIndex::BROW_ASSAY_NAME_COLUMN), spAssayInfo->assayCode, Qt::UserRole + 2);
        // 存储设备信息，用于赋值，避免查找
        m_CalibrateConcMode->setData(m_CalibrateConcMode->index(row, caliBrowseColumnIndex::BROW_ASSAY_NAME_COLUMN), get<1>(assayInfo), Qt::UserRole + 3);
		
		// 设置项目单位
		m_CalibrateConcMode->setItem(row, caliBrowseColumnIndex::BROW_MAIN_UNIT_COLUMN, new QStandardItem(get<2>(assayInfo)));

		row++;
    }
	connect(m_CalibrateConcMode, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(updateDataList(QStandardItem*)));
    SetAscSortByFirstColumn();
}

///
/// @brief
///     判断校准品中的成分设置是否一致
///
/// @param[in]  caliDocs  校准品列表
/// @param[in]  cgType    校准文档组的类型
///
/// @return true表示一直，反之则不是
///
/// @par History:
/// @li 5774/WuHongTao，2020年10月30日，新建函数
///
bool CaliBrateRecordDialog::IsCompostionEqual(QList<ch::tf::CaliDoc>& caliDocs, int type)
{
    //成分类型
    QStringList compostionList;
	// 保存某个项目最新的浓度值（用于判定是否按照梯度输入浓度）
    std::map<int32_t, double> concMap;

	// 设置最大的level
	int maxLevel = 0;

    //循环的判断是否是否保存
    for(const auto& caliDoc : caliDocs)
    {
		//以第一个校准的成份类型作为标准，以后的每一个水平的校准品都必须一致，否则校准品组有问题
        if (compostionList.isEmpty())
        {
			// 依次添加每个校准品类型
			for_each(caliDoc.compositions.begin(), caliDoc.compositions.end(), [&](auto compostion) {
			// 添加本类型的校准品到队列
			compostionList.append(std::to_string(compostion.assayCode).c_str()); 
			concMap.insert(pair<int32_t, double>(compostion.assayCode, compostion.conc)); });
        }
        else
        {
            // 判断逻辑成分相等的逻辑：
			// 前因： 1.由于6个校准品，用户可能输入非连续设置每个水平的浓度，所以
			//          当前没有设置浓度，则继续循环，判断后续水平有没有设置浓度
			//        2.成分为空，只表示当前水平没有设置浓度，不表示后面的水平没有设置浓度，
			// 结果： 1.当前水平浓度为空，但是后面水平有设置浓度，则返回false，不让用户保存
			//        2.当前水平设置的浓度个数和别的浓度的数量不一致，则返回false，不让用户保存
			//        3.同一种试剂浓度值没有从小到大，则返回false，不让用户保存
			//  	  4.当前水平浓度为空，后面水平也为空，则记录最大的浓度水平，最后删除多余的临时校准文档
            if (caliDoc.compositions.empty())
            {
				// 因为已经默认生成六个临时校准品，所以可能会出现校准品成为为空
				// 后续统计连续设置的校准品数量，用于判断校准品是否连续输入，以及清除非有效的校准品
                continue;
            }
			else if (caliDoc.compositions.size() != compostionList.size())
			{
				return false;
			}

			// 非连续录入校准品的情况，不能保存
			if (caliDoc.level - maxLevel != 1)
				return false;

            for (const auto& compostion : caliDoc.compositions)
            {
				// 保证测试项目必须一致
                if (!compostionList.contains(std::to_string(compostion.assayCode).c_str()))
                {
                    return false;
                }

                if ((type == ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_XQXJ) || (type == ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NY))
                {
                    // 对于ise校准品，浓度必须有大到小
                    if (concMap[compostion.assayCode] <= compostion.conc)
                    {
                        return false;
                    }
                }
                else
                {
                    // 对于比色校准品，浓度必须有小到大
                    if (concMap[compostion.assayCode] >= compostion.conc)
                    {
                        return false;
                    }
                }
				
				concMap[compostion.assayCode] = compostion.conc;
            }
		}

        // 第一个校准文档就没有项目，直接返回
        if (compostionList.isEmpty())
            return false;

        // 最大水平自增
        if (!compostionList.isEmpty())
            maxLevel++;
    }

	//录入信息不能为空
	if (maxLevel == 0)
		return false;

	//清除多余的校准文档
	while (caliDocs.size() > maxLevel)
	{
		caliDocs.removeLast();
	}

    return true;
}

///
/// @brief 对话框的初始化操作
///     
/// @par History:
/// @li 5774/WuHongTao，2020年5月9日，新建函数
///
void CaliBrateRecordDialog::Init()
{	
	ui->groupBox_input->setStyleSheet("#groupBox_input{ border: 1px solid rgb(181,181,181);}");
	ui->groupBox_out->setStyleSheet("#groupBox_out{ border: 1px solid rgb(181,181,181);}");
	ui->groupBox_screen->setStyleSheet("#groupBox_out{ border: 1px solid rgb(181,181,181);}");
	ui->calibrate_water->setStyleSheet("#calibrate_water{ border: 1px solid rgb(181,181,181);}");
    ui->cali_meterial_type->setCurrentIndex(0);
    //设置表头
    ResetRecordAssayTable();
    //清空数据(预留清空功能)
    //connect(ui->clear_btn, SIGNAL(clicked()), this, SLOT(clearContent()));
	// 查询按钮（下载登记）
	connect(ui->btn_query, SIGNAL(clicked()), this, SLOT(OnQueryBtnClicked()));
	// 重置按钮（下载登记）
	connect(ui->btn_reset_cal, SIGNAL(clicked()), this, SLOT(OnResetBtnClicked()));
    //初始化按钮
    connect(ui->yes, SIGNAL(clicked()), this, SLOT(addNewCaliIntoDatabase()));
    //退出
	connect(ui->no, &QPushButton::clicked, this, [=] {
		CleanBeforeQuit();
		this->close();
	});

	// 填充校准品类型组合框
    if (!CommonInformationManager::GetInstance()->GetDeviceFromType({ tf::DeviceType::DEVICE_TYPE_C1000, tf::DeviceType::DEVICE_TYPE_C200 }).empty())
    {
        ui->cali_meterial_type->addItem(tr("生化校准品"), ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_OTHER);		// 校准品类型->生化校准品(CAIL_METERIAL_TYPE_CHER)
    }

    if (!CommonInformationManager::GetInstance()->GetDeviceFromType({ tf::DeviceType::DEVICE_TYPE_ISE1005 }).empty())
    {
        ui->cali_meterial_type->addItem(tr("ISE血清校准品"), ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_XQXJ);		// 校准品类型->ISE血清校准品(CAIL_METERIAL_TYPE_ISE_SERUM)
	    ui->cali_meterial_type->addItem(tr("ISE尿液校准品") , ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NY);		// 校准品类型->ISE尿液校准品(CAIL_METERIAL_TYPE_ISE_URINE)
    }

    connect(ui->cali_meterial_type, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=](int index) {
        if (IsValidCaliTypeChanged())
        {
            OnChangeCalibrateType(index);

            m_addCaliDocs.clear();
            ch::tf::CaliDoc caliDocTmp;

            // 清空历史存储的数据
            m_disableAssayCodes.clear();

            //生成临时校准文档
            for (size_t i = 1; i <= MAX_CALI_DOC_CNT; i++)
            {
                caliDocTmp.compositions.clear();
                caliDocTmp.lot = ui->lotedit->text().toStdString();
                // 默认为无效类型
                caliDocTmp.__set_tubeType(tf::TubeType::TUBE_TYPE_INVALID);
                caliDocTmp.level = i;
                m_addCaliDocs.append(caliDocTmp);
            }

            if (m_modeType == MODIFY_MODE)
            {
                for (const auto& doc : m_detailDocs)
                {
                    m_addCaliDocs[doc.level - 1] = doc;
                    m_addCaliDocs[doc.level - 1].compositions.clear();
                }
            }

            updateMainCalibrateView(m_addCaliDocs);
        }
    });

	connect(ui->radioButton_scanModel, &QRadioButton::clicked, this, [=]() {
		setInputMode(InputType::Input_Scan);
	});
	connect(ui->radioButton_manualModel, &QRadioButton::clicked, this, [=]() {
		setInputMode(InputType::Input_Manual);
	});
	connect(ui->radioButton_downModel, &QRadioButton::clicked, this, [=]() {
		setInputMode(InputType::Input_Down);
	});

	// 选择改变
	connect(ui->calibrateDown, SIGNAL(pressed(const QModelIndex&)), this, SLOT(OnCalibrateDownSelectedChanged(const QModelIndex&)));
	
    // 监听项目更新
    REGISTER_HANDLER(MSG_ID_ASSAY_CODE_MANAGER_UPDATE, this, OnAssayUpdated);
}


///
/// @brief 退出之前清理
///
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
///
void CaliBrateRecordDialog::CleanBeforeQuit()
{
	// 清除扫描检测
	/*if (m_barCodeTimer && m_barCodeTimer->isActive())
	{
		m_barCodeTimer->stop();
		m_barCodeTimer.reset();
		m_barCodeTimer = nullptr;
	}*/

	// 清除扫描框的事件
	if (m_inputType == InputType::Input_Scan)
	{
		//disconnect(ui->scanEdit, SIGNAL(textChanged(const QString&)), this, SLOT(OnBarCodeEditChanged(const QString&)));
		disconnect(ui->scanEdit, SIGNAL(editingFinished()), this, SLOT(OnBarFinished()));
	}
}

///
/// @brief 添加新样品
///      
/// @par History:
/// @li 5774/WuHongTao，2020年5月9日，新建函数
///
void CaliBrateRecordDialog::addNewCaliIntoDatabase()
{
    /*if (CommonInformationManager::GetInstance()->IsExistDeviceRuning(tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY))
    {
        // 删除之前先弹出一个对话框，然后进行选择
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("提示"), tr("生化分析仪处于运行状态，不能添加或修改校准品！"), TipDlgType::SINGLE_BUTTON));
        return;
    }*/

	// 如果是下载登记，则用另外一个接口保存
	if (m_inputType == Input_Down)
	{
		SaveCalibrateDownData();
		return;
	}
    
    // 校准品名称进行简单检查，不能只包含（'\t', '\n', '\v', '\f', '\r', and ' '）
    const QString& groupName = ui->nameEdit->text();
    if (!groupName.isEmpty() && groupName.trimmed().isEmpty())
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("保存失败"), tr("校准品名称不能全输入不可见字符。")));
        pTipDlg->exec();
        return;
    }

    // 首先确定是否需要插入
    if (m_addCaliDocs.empty() || ui->nameEdit->text().isEmpty() || ui->lotedit->text().isEmpty() || ui->snedit->text().isEmpty() || ui->dateEdit->isNull())
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("保存失败"), tr("校准品名称、批号、编号、失效日期不能为空。")));
        pTipDlg->exec();
        return;
    }

	ch::tf::CaliDocQueryCond queryCond;
	// 校准品批号查询条件
    queryCond.__set_lot(ui->lotedit->text().toStdString());

	// 存储编号
	std::string sn;
	
	if (m_inputType == Input_Manual)
	{
		// 生成一个新的编号
		// emit getNewSn(ui->lotedit->text().toStdString(), sn);
        sn = ui->snedit->text().toStdString();
	}
	else if(m_inputType == Input_Scan)
	{
		sn = m_lastScanInfo.sn;
	}
	else
	{
		sn = ui->snedit->text().toStdString();
	}

    // 校验编号是否为3位、批号是否为7位
    if (sn.length() != 3 || ui->lotedit->text().length() != 7)
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("保存失败"), tr("编号必须为3位，批号必须为7位！")));
        pTipDlg->exec();
        return;
    }

	// 校准品编号查询条件
	queryCond.__set_sn(sn);

    // 查询
    ch::tf::CaliDocQueryResp resp;
    ch::LogicControlProxy::QueryCaliDoc(resp, queryCond);
    // 查询失败,返回
    if (resp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "failed to query");
        return;
    }

    // 是否存在对应的校准品，有的话，直接返回不能添加
    if (!resp.lstCaliDocs.empty() && (m_modeType == NEW_MODE ||
        (m_modeType == MODIFY_MODE && m_groupDb != resp.lstCaliDocs[0].caliDocGroupId)))
    {
        ULOG(LOG_ERROR, "the calibrate has existed");
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("保存失败"), tr("存在相同批号、编号校准品组。")));
        pTipDlg->exec();
        return;
    }

    // 判断校准品中每个成分是否设置一致
    int type = ui->cali_meterial_type->currentData().toInt();
    if(!IsCompostionEqual(m_addCaliDocs, type))
    {
        ULOG(LOG_ERROR, "IsCompostionEqual run error");
        if ((type == ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_XQXJ) || (type == ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NY))
        {
            std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("保存失败"), tr("未录入校准品，或者浓度未按照从大到小进行填写，或者每个项目的校准品数量填写不一致")));
            pTipDlg->exec();
        }
        else
        {
            std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("保存失败"), tr("未录入校准品，或者浓度未按照从小到大进行填写，或者每个项目的校准品数量填写不一致")));
            pTipDlg->exec();
        }
        return;
    }

    // 查询校准品中是否存在正在校准的项目
    // 有正在校准的项目，则不能修改校准品组
    if (m_modeType == SHOWMODE::MODIFY_MODE)
    {
        bool isAssayDoing = false;
        // ISE校准品组，查询ISE模块校准情况
        if ((type == ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_XQXJ) || (type == ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NY))
        {
            ::ise::tf::IseModuleInfoQueryCond qryCond;
            ::ise::tf::IseModuleInfoQueryResp qryResp;

            ::ise::LogicControlProxy::QueryIseModuleInfo(qryResp, qryCond);
            if (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
            {
                ULOG(LOG_ERROR, "QueryIseModuleInfo failed.");
            }

            for (const auto& iseModule : qryResp.lstIseModuleInfos)
            {
                if (iseModule.caliStatus != tf::CaliStatus::CALI_STATUS_DOING)
                    continue;

                isAssayDoing = true;
                break;
            }
        }
        // 查询生化校准情况
        else
        {
            // 获取在机或者注册的项目
            ::ch::tf::ReagentGroupQueryResp qryResp;
            ::ch::tf::ReagentGroupQueryCond qryCond;
            qryCond.__set_beDeleted(false);

            if (!ch::c1005::LogicControlProxy::QueryReagentGroup(qryResp, qryCond)
                || qryResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
            {
                ULOG(LOG_ERROR, "QueryReagentGroup failed");
            }

            if (!m_addCaliDocs.empty() && !m_addCaliDocs[0].compositions.empty())
            {
                for (auto& r : qryResp.lstReagentGroup)
                {
                    // 排除异常项目和非校准申请中项目
                    if (r.assayCode < 0 || r.posInfo.pos <= 0 || r.caliStatus != tf::CaliStatus::CALI_STATUS_DOING)
                        continue;

                    for (const auto& comp : m_addCaliDocs[0].compositions)
                    {
                        if (comp.assayCode == r.assayCode)
                        {
                            isAssayDoing = true;
                            break;
                        }
                    }
                }
            }
        }

        if (isAssayDoing)
        {       
            std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("保存失败"), tr("校准品组中存在正在校准的项目，禁止修改！")));
            pTipDlg->exec();
            return;
        }
    }

    // ISE三个水平都需要录入（DCS确认需要此防御逻辑，电解质设备需要三个浓度都录入）
    if ((type == ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_XQXJ) || (type == ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NY))
    {
        if (!m_addCaliDocs.isEmpty() && 
             (m_addCaliDocs[0].compositions.size() < m_CalibrateConcMode->rowCount() 
                 // 现在只要求2个水平浓度
              || m_addCaliDocs.size() <= 1))
        {
            std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("保存失败"), tr("电解质所有项目都必须完整录入浓度")));
            pTipDlg->exec();
            return;
        }
    }

    // 初始化校准品1的数据
    if (ui->calibrate_water->isChecked() && m_addCaliDocs.size() > 1)
    {
        if (m_addCaliDocs[0].level == 1 && m_addCaliDocs[1].__isset.rack)
        {
            m_addCaliDocs[0].__set_rack(m_addCaliDocs[1].rack);
            m_addCaliDocs[0].__set_pos(0 - m_addCaliDocs[1].pos);
        }
    }

    // 可能之前是系统水，现在改为非系统水了，取消以前的位置设置
    if (!ui->calibrate_water->isChecked() && 
        m_addCaliDocs[0].__isset.pos && 
        m_addCaliDocs[0].pos < 0)
    {
        m_addCaliDocs[0].__set_rack("");
        m_addCaliDocs[0].__set_pos(0);
    }

	//校准品组--设置公共变量
    ch::tf::CaliDocGroup caliGroup;
	// 设置名称
	caliGroup.__set_name(ui->nameEdit->text().toStdString());
	// 设置批号
	caliGroup.__set_lot(ui->lotedit->text().toStdString());
	// 设置编号
	caliGroup.__set_sn(sn);
	// 设置有效期
	caliGroup.__set_expiryTime(ui->dateEdit->date().toString("yyyy-MM-dd 00:00:00").toStdString());
	// 设置校准品1系统水
	caliGroup.__set_systemWater(ui->calibrate_water->isChecked() && ui->calibrate_water->isVisible());

    switch (m_inputType)
    {
        //手动输入
        case Input_Manual:
            caliGroup.__set_registType(tf::DocRegistType::REG_MANUAL);
            break;
        //扫描输入
        case Input_Scan:
            caliGroup.__set_registType(tf::DocRegistType::REG_SCAN);
            break;
        //下载输入
        case Input_Down:
            caliGroup.__set_registType(tf::DocRegistType::REG_DOWNLOAD);
            break;
        //修改输入
        case Input_Modify:
            break;
        default:
            break;
    }

	// 设置是否是ise校准品, 以及样本类型
	if (type == ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_XQXJ ||
        type == ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NY)
	{
		// 当前是ise校准品，并设置样本类型
		caliGroup.__set_iseType(true);
		caliGroup.__set_sampleSourceType(type);
	}
	else
	{
		// 当前不是ise校准品
		caliGroup.__set_iseType(false); 
	}

	// 设置禁用的项目列表
	//if(!m_disableAssayCodes.empty())
		caliGroup.__set_disableAssayCodes(m_disableAssayCodes);

    //首先在数据库中建立一个数据库项目，然后将本项目的db传递给后面的校准文档
    if (m_modeType != MODIFY_MODE)
    {
        //校准品关键信息
        ch::tf::CaliDocKeyInfo caliKeyInfo;
		std::vector<ch::tf::CaliDocKeyInfo> caliKeyInfos;
        //循环填充组信息
		for_each(m_addCaliDocs.begin(), m_addCaliDocs.end(), [&](auto& caliDoc) {
			//校准品的组名称
			caliKeyInfo.__set_level(caliDoc.level);
			caliKeyInfo.__set_lot(ui->lotedit->text().toStdString());
			caliDoc.sn = sn;
			caliKeyInfo.__set_sn(sn);
			caliKeyInfos.push_back(caliKeyInfo);
			});

		// 设置校准组关键信息
		caliGroup.__set_caliDocKeyInfos(caliKeyInfos);
		::tf::ResultLong ret;
        //具体存储操作
        ch::LogicControlProxy::AddCaliDocGroup(ret, caliGroup);
        //添加成功
        if (::tf::ThriftResult::THRIFT_RESULT_SUCCESS == ret.result)
        {
            m_groupDb = ret.value;
            ULOG(LOG_INFO, "succed to execute addCaliDocGroup");

            // 更新操作日志
            QString record = tr("添加了校准品组，校准品组名称：") + QString::fromStdString(caliGroup.name) \
                + tr("，校准品组批号：") + QString::fromStdString(caliGroup.lot) \
                + tr("，校准品组编号：") + QString::fromStdString(caliGroup.sn);

            AddOperateLog(record.toStdString(), tf::OperationType::ADD);
        }
        else
        {
            ULOG(LOG_ERROR, "Failed to addCaliDocGroup");
            TipDlg(tr("操作失败"), tr("添加校准品失败，已登记相同批号、编号校准品组！"), TipDlgType::SINGLE_BUTTON).exec();
            return;
        }
    }

    //清除之前的数据，以备修改的时候使用
    caliGroup.caliDocKeyInfos.clear();
    bool needModify = false;
    //校准品关键信息
    ch::tf::CaliDocKeyInfo caliKeyInfo;
    //循环的将消息发送到数据中存起来
    for(ch::tf::CaliDoc& caliDoc : m_addCaliDocs)
    {
        //添加对应的db号
        caliDoc.__set_caliDocGroupId(m_groupDb);
        //保存数据库成功则添加到组中
        if (saveCaliData(caliDoc, caliGroup))
        {
            //校准品的组名称
            caliGroup.__set_name(ui->nameEdit->text().toStdString());
            caliKeyInfo.__set_level(caliDoc.level);
            caliKeyInfo.__set_lot(caliDoc.lot);
            caliKeyInfo.__set_sn(caliDoc.sn);
            caliGroup.caliDocKeyInfos.push_back(caliKeyInfo);
        }
        else
        {
            //只要有一次存储不成功，则需要重新修改数据库(校准组的)
            needModify = true;
        }
    }

	// 有需要删除的校准文档，执行删除
	if (m_addCaliDocs.size() < m_detailDocs.size())
	{
		for (int start = m_addCaliDocs.size(); start < m_detailDocs.size(); ++start)
		{
			deleteCaliData(m_detailDocs.at(start), caliGroup);
		}
	}
    
    //修改group的信息,查看的时候
    if (m_modeType == MODIFY_MODE || needModify ==  true)
    {
        //修改的时候，加上db号
        caliGroup.__set_id(m_groupDb);
		caliGroup.__set_caliDocKeyInfos(caliGroup.caliDocKeyInfos);
        bool ret = ch::LogicControlProxy::ModifyCaliDocGroup(caliGroup);

        //添加成功
        if (ret)
        {
            ULOG(LOG_INFO, "succed to execute modify calibrate group");
        }
        else
        {
            ULOG(LOG_ERROR, "Failed to add the modify calibrate group");
        }
    }

    // 获取修改的操作日志
    if (m_modeType == MODIFY_MODE)
    {
        const auto& concLog = GetModifyCaliInfoLog(m_caligroup, caliGroup, m_detailDocs, m_addCaliDocs);
        // 有变更
        if (!concLog.empty())
        {
            const auto& title = tr("编辑了校准品组[id:%1,名称：%2]:\n").arg(QString::number(m_caligroup.id), QString::fromStdString(m_caligroup.name)).toStdString();

            AddOperateLog((title + concLog), tf::OperationType::MOD);
        }
        else
        {
            ULOG(LOG_INFO, "No changed info");
        }
        POST_MESSAGE(MSG_ID_CH_CALI_SET_UPDATE, tf::UpdateType::UPDATE_TYPE_MODIFY, caliGroup, m_addCaliDocs);
    }
    
	CleanBeforeQuit();
    this->close();
    m_saveCaliDocs = m_addCaliDocs;
    m_addCaliDocs.clear();
    ResetRecordAssayTable();
    //发出关闭对话框的消息
    emit closeWindow(m_groupDb);
}

///
/// @brief
///     更改日期
///
/// @param[in]  date  日期参数
///
/// @par History:
/// @li 5774/WuHongTao，2021年4月19日，新建函数
///
void CaliBrateRecordDialog::dateModified(const QDate &date)
{
    //dataModified("null");
}

///
/// @brief
///     数据信息有更新
///
/// @param[in]  item  更新数据的新控件
///
/// @par History:
/// @li 5774/WuHongTao，2020年6月28日，新建函数
///
void CaliBrateRecordDialog::updateDataList(QStandardItem* item)
{
    ULOG(LOG_INFO, "%s(). col:%d, text is %s.", __FUNCTION__, item->column(), item->text().toStdString());

	// 过滤 “启动”列和“单位”列的选择状态变化消息
	if (m_CalibrateConcMode->indexFromItem(item).column() == m_CalibrateConcMode->columnCount() - 1)
	{
		setDisableAssayCode(item);
		return;
	}
	// 判断当前行是否有内容
	bool hasContent = hasContentInRow(item);
    int  level  = item->data(Qt::UserRole + 1).toInt();

	// 获取名称的字符串
	string assayName = m_CalibrateConcMode->data(m_CalibrateConcMode->index(item->index().row(), 0)).toString().toStdString();
	// 根据校准品名称获取对应的校准品信息
	std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(assayName);
	if (spAssayInfo == nullptr)
	{
		ULOG(LOG_ERROR, "Can not find the assay item, assayName=%s", assayName);
		return;
	}

    for (ch::tf::CaliDoc& caliDocInformation : m_addCaliDocs)
    {
		// 如果当前行没有内容，则清除历史保存的数据
		if (!hasContent)
		{
			auto it = caliDocInformation.compositions.begin();
			while (it != caliDocInformation.compositions.end())
			{
				if (it->assayCode == spAssayInfo->assayCode)
				{
					// swap会造成每个校准品里的assaycode顺序不一致，目前来看没有影响（可以优化为erase，但时间增加)
					swap(*it, caliDocInformation.compositions.back());
					caliDocInformation.compositions.pop_back();
					break;
				}
				++it;
			}

			continue;
		}

		// 根据条件筛选对应的校准品
		float conc = item->data(Qt::EditRole).toFloat();

		if (caliDocInformation.level != level || conc < 0)
		{
			continue;
		}

		bool emptyItem = item->text().isEmpty();

		// 如果内容为空，则删除该浓度数据
		if (emptyItem)
		{
			auto itCom = caliDocInformation.compositions.begin();
			while (itCom != caliDocInformation.compositions.end())
			{
				if (itCom->assayCode == spAssayInfo->assayCode)
				{
					swap(*itCom, caliDocInformation.compositions.back());
					caliDocInformation.compositions.pop_back();
					//完事，直接返回
					return;
				}
				++itCom;
			}
		}
		// 更新当前浓度的值
		else
		{
			//更新成分信息
			for (ch::tf::CaliComposition& compostion : caliDocInformation.compositions)
			{
				if (compostion.assayCode != spAssayInfo->assayCode)
				{
					continue;
				}
				//更新浓度信息
				compostion.conc = conc;
				//完事，直接返回
				return;
			}

			ch::tf::CaliComposition comPostion;
			comPostion.__set_conc(conc);
			// 设置设备类型
			comPostion.__set_deviceType(::tf::DeviceType::type(m_CalibrateConcMode->data(m_CalibrateConcMode->index(item->index().row(), caliBrowseColumnIndex::BROW_ASSAY_NAME_COLUMN), Qt::UserRole + 3).toInt()));
			comPostion.__set_assayCode(spAssayInfo->assayCode);
			//说明没找到对应的成分信息，则添加
			caliDocInformation.compositions.push_back(comPostion);
			//退出
			break;
		}
    }
}

///
/// @brief
///     改变校准品位置
///
/// @param[in]  index  索引
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月11日，新建函数
///
void CaliBrateRecordDialog::OnChangeCalibratePostion(QModelIndex index)
{
    // 必须满足位置的编辑条件
    if (!index.isValid() || index.row() != 0 || index.column() < 1)
    {
        return;
    }

    if (m_CaliPostionEditDialog == nullptr)
    {
        m_CaliPostionEditDialog = new QPostionEdit(this);
        connect(m_CaliPostionEditDialog, SIGNAL(ModifyCaliPostion()), this, SLOT(OnRefreshPostion()));
    }
    m_CaliPostionEditDialog->SetCaliDoc(m_addCaliDocs, index.column() - 1);
    m_CaliPostionEditDialog->show();
}

///
/// @brief
///     刷新校准修改位置
///
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月11日，新建函数
///
void CaliBrateRecordDialog::OnRefreshPostion()
{
    if (m_CaliPostionEditDialog == nullptr)
    {
        return;
    }

	auto docs = m_CaliPostionEditDialog->GetCaliDoc();
	if (docs)
	{
		updateMainCalibrateView(docs.value());
	}
	else
	{
		QList<ch::tf::CaliDoc> dataShow;
		updateMainCalibrateView(dataShow);
	}
}

///
/// @brief 校准品组类型变更时，表格内容需要变化
///
/// @param[in]  index  变化后的索引
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月16日，新建函数
///
void CaliBrateRecordDialog::OnChangeCalibrateType(int index)
{
    int type = ui->cali_meterial_type->currentData().toInt();

    if (IsValidCaliTypeChanged())
    {
        // 获取表格信息
        GetAssayInfoList(m_AssayItemList, type);

        // 重置表格项目
        ResetRecordAssayTable();

        // 设置代理
        SetTableViewDelegate(type);
    }

    m_lastCaliType = type;
}

///
/// @brief
///     存储数据到数据库
///
/// @param[in]  data  需要存储的数据
/// @param[in]  caliGroup  校准品组
///
/// @return 保存成功,则返回true，错误则返回false
///
/// @par History:
/// @li 5774/WuHongTao，2020年6月29日，新建函数
///
bool CaliBrateRecordDialog::saveCaliData(ch::tf::CaliDoc& data, const ch::tf::CaliDocGroup& caliGroup)
{
    ch::tf::CaliDoc caliDoc;
	data.lot = ui->lotedit->text().toStdString();
    data.sn = ui->snedit->text().toStdString();
	caliDoc.__set_lot(caliGroup.lot);
    caliDoc.__set_level(data.level);
    caliDoc.__set_caliDocGroupId(data.caliDocGroupId);
    caliDoc.__set_tubeType(data.tubeType);
    caliDoc.__set_rack(data.rack);
    caliDoc.__set_pos(data.pos);
    caliDoc.__set_sn(caliGroup.sn);
	caliDoc.__set_oneDimensionalCode(GetCaliDocOneDimensionalCode(caliDoc.sn, caliDoc.lot, caliDoc.level));
    if (m_modeType == MODIFY_MODE && data.level <= m_detailDocs.size())
    {
        caliDoc.__set_id(data.id);
    }

    std::vector<ch::tf::CaliComposition> vecComps;
    ch::tf::CaliComposition comp;
    for (ch::tf::CaliComposition caliCompstion : data.compositions)
    {
        comp.__set_assayCode(caliCompstion.assayCode);
		comp.__set_deviceType(caliCompstion.deviceType);
        comp.__set_conc(caliCompstion.conc);
        vecComps.push_back(comp);
    }
    caliDoc.__set_compositions(vecComps);


    if (m_modeType != MODIFY_MODE)
    {

        ::tf::ResultLong ret;
        //具体存储操作
        ch::LogicControlProxy::AddCaliDoc(ret, caliDoc);

        //添加成功
        if (::tf::ThriftResult::THRIFT_RESULT_SUCCESS == ret.result)
        {
            data.__set_id(ret.value);
            ULOG(LOG_INFO, "succed to execute SystemConfig::LoadConfig()");
            return true;
        }
        else
        {
            ULOG(LOG_ERROR, "Failed to add the calibrationSample saveCaliData");
            return false;
        }
    }
    else
    {
		bool retFlag = false;
		if (data.level <= m_detailDocs.size())
			retFlag = ch::LogicControlProxy::ModifyCaliDoc(caliDoc);
		else
		{
			::tf::ResultLong ret;
			ch::LogicControlProxy::AddCaliDoc(ret, caliDoc);
			retFlag = ::tf::ThriftResult::THRIFT_RESULT_SUCCESS == ret.result;
		}

        //添加成功
        if (retFlag)
        {
            ULOG(LOG_INFO, "succed to execute modify sample");
            return true;
        }
        else
        {
            ULOG(LOG_ERROR, "Failed to add the modify sample");
            return false;
        }
    }
}

///
/// @brief
///     删除数据
///
/// @param[in]  data  需要删除的数据
/// @param[in]  caliGroup  校准品组
///
/// @return 删除成功,则返回true，错误则返回false
///
/// @par History:
/// @li 8090/YeHuaning，2022年8月26日，新建函数
///
bool CaliBrateRecordDialog::deleteCaliData(const ch::tf::CaliDoc& data, const ch::tf::CaliDocGroup& caliGroup)
{
	::ch::tf::CaliDocQueryCond qrnd;
	qrnd.__set_level(data.level);
	qrnd.__set_lot(data.lot);
	qrnd.__set_sn(data.sn);

	bool retFlag = ch::LogicControlProxy::DeleteCaliDoc(qrnd);
	//删除成功
	if (retFlag)
	{
		ULOG(LOG_INFO, "succed to execute modify sample");
		return true;
	}
	else
	{
		ULOG(LOG_ERROR, "Failed to add the modify sample");
		return false;
	}
}

///
/// @brief
///     获取项目信息的List
///
/// @param[in]  assayList  项目信息list
///
/// @par History:
/// @li 5774/WuHongTao，2020年6月28日，新建函数
///
void CaliBrateRecordDialog::GetAssayInfoList(QVector<tuple<string, int, QString>>& assayList, int type)
{
    assayList.clear();

	// 设置项目表格初始化信息
    if (type == tf::SampleSourceType::SAMPLE_SOURCE_TYPE_OTHER)
    {
        ChAssayIndexUniqueCodeMaps AssayMap;
        CommonInformationManager::GetInstance()->GetChAssayMaps(::tf::DeviceType::DEVICE_TYPE_INVALID, AssayMap);
        
        //获取项目信息list
        for (const auto& assay : AssayMap)
        {
            // 不显示血清指数(S.IND)相关信息
            if (assay.second == nullptr || assay.second->assayCode == ::ch::tf::g_ch_constants.ASSAY_CODE_SIND)
            {
                continue;
            }
			
            assayList.append(make_tuple(assay.first, 
										assay.second->deviceType, 
										CommonInformationManager::GetInstance()->GetAssayMainUnit(assay.second->assayCode)
										));
        }
    }
    else if ((type == ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_XQXJ) || (type == ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NY))
    {
        //获取ISE项目信息
        for (const auto& iseAssay : CommonInformationManager::GetInstance()->GetIseAssayIndexCodeMaps())
        {
            // ise项目信息
            auto spIseAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(iseAssay.first);
            if (!spIseAssayInfo)
            {
                continue;
            }

            assayList.append(make_tuple(spIseAssayInfo->assayName,
										iseAssay.second->deviceType,
										CommonInformationManager::GetInstance()->GetAssayMainUnit(iseAssay.second->assayCode)
										));
        }
    }
}

///
/// @brief
///     设置试剂校准屏蔽列表
///
/// @param[in]  item  变化的项目
///
/// @par History:
/// @li 8090/YeHuaNing，2022年8月25日，新建函数
///
void CaliBrateRecordDialog::setDisableAssayCode(const QStandardItem * item)
{
	//bool hasContent = hasContentInRow(item);
	int colCnt = m_CalibrateConcMode->columnCount();
    int curRow = item->index().row();
	int assayCode = m_CalibrateConcMode->index(curRow, caliBrowseColumnIndex::BROW_ASSAY_NAME_COLUMN).data(Qt::UserRole + 2).toInt();

	std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);

	if (spAssayInfo != nullptr)
	{
		if (/*hasContent &&*/ !m_CalibrateConcMode->item(curRow, colCnt - 1)->data().toBool())
		{
			m_disableAssayCodes.insert(spAssayInfo->assayCode);
		}
		else
		{
			m_disableAssayCodes.erase(spAssayInfo->assayCode);
		}
	}
}

///
/// @brief
///     获取当前行是否有设置浓度信息
///
/// @param[in]  item  变化的项目
///
/// @par History:
/// @li 8090/YeHuaNing，2022年8月25日，新建函数
///
bool CaliBrateRecordDialog::hasContentInRow(const QStandardItem* item) const
{
	int colCnt = m_CalibrateConcMode->columnCount();
	int curRow = item->index().row();

	bool hasContent = false;

	// 判断当前行是否有浓度设置
	for (int startCol = 1; startCol < colCnt - 1; ++startCol)
	{
		if (m_CalibrateConcMode->item(curRow, startCol) != nullptr && 
            !m_CalibrateConcMode->item(curRow, startCol)->text().isEmpty())
		{
			hasContent = true;
			break;
		}
	}

	return hasContent;
}

///
/// @brief 设置表格的读写方式
///
/// @param[in]  type  校准品类型
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月20日，新建函数
///
void CaliBrateRecordDialog::SetTableViewDelegate(int type)
{
    if (type == tf::SampleSourceType::SAMPLE_SOURCE_TYPE_OTHER)
    {
        m_editDelegate->SetDecimalsPlace(6);
        ui->calibrate_water->setVisible(true);
		ui->line_h_tableTop->setVisible(false);
		// 调整界面
		ui->calibrateBrowse->move(ui->calibrateBrowse->x(), ui->calibrate_water->y()+ ui->calibrate_water->height());

        for (size_t i = BROW_CALIBRATOR_3_COLUMN; i < m_CalibrateConcMode->columnCount() - 1; ++i)
        {
			// 单位列
			if (i == caliBrowseColumnIndex::BROW_MAIN_UNIT_COLUMN) continue;

            ui->calibrateBrowse->setItemDelegateForColumn(i, ui->calibrateBrowse->itemDelegateForColumn(2));	
        }
    }
    else if ((type == ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_XQXJ) || (type == ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NY))
    {
        m_editDelegate->SetDecimalsPlace(1);
        ui->calibrate_water->setVisible(false);
		ui->line_h_tableTop->setVisible(true);
		// 调整界面
		ui->calibrateBrowse->move(ui->calibrateBrowse->x(), ui->calibrate_water->y());
		ui->line_h_tableTop->move(ui->calibrateBrowse->x(), ui->calibrateBrowse->y());
		ui->line_v_3->move(ui->line_v_3->x(), ui->calibrateBrowse->y());
		ui->line_v_4->move(ui->line_v_4->x(), ui->calibrateBrowse->y());
		ui->line_v_3->resize(ui->line_v_3->width(), ui->line_h_5->y() - ui->line_h_tableTop->y());
		ui->line_v_4->resize(ui->line_v_4->width(), ui->line_h_5->y() - ui->line_h_tableTop->y());

        for (size_t i = BROW_CALIBRATOR_3_COLUMN; i < m_CalibrateConcMode->columnCount() - 1; ++i)
        {
			// 单位列
			if (i == caliBrowseColumnIndex::BROW_MAIN_UNIT_COLUMN) continue;

            ui->calibrateBrowse->setItemDelegateForColumn(i, new CReadOnlyDelegate(this));
        }
    }
}

///
/// @brief 当前校准品类型切换后是否需要做数据处理
///
/// @return true:数据需要处理  false:不需要处理
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月20日，新建函数
///
bool CaliBrateRecordDialog::IsValidCaliTypeChanged()
{
    int type = ui->cali_meterial_type->currentData().toInt();

    // Ise校准品类型之间进行切换，不清理表格，尽量保留原有内容
    return !((m_lastCaliType == ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_XQXJ) && (type == ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NY)
        || (m_lastCaliType == ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NY) && (type == ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_XQXJ));
}

void CaliBrateRecordDialog::CreateSystemWaterDocs()
{
    if (!m_addCaliDocs.empty())
    {
        // 删除多余的文档
        while (m_addCaliDocs.size() > 1)
        {
            m_addCaliDocs.pop_back();
        }

        for (int i = 0; i < m_CalibrateConcMode->rowCount(); ++i)
        {
            // 获取名称的字符串
            string assayName = m_CalibrateConcMode->data(m_CalibrateConcMode->index(i, 0)).toString().toStdString();
            // 根据校准品名称获取对应的校准品信息
            std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(assayName);
            if (spAssayInfo == nullptr)
            {
                ULOG(LOG_ERROR, "Can not find the assay item, assayName=%s", assayName);
                continue;
            }

            if (!m_CalibrateConcMode->item(i, m_CalibrateConcMode->columnCount() - 1)->data().toBool())
            {
                m_disableAssayCodes.insert(spAssayInfo->assayCode);
            }
            else
            {
                ch::tf::CaliComposition  composition;
                composition.__set_assayCode(spAssayInfo->assayCode);
                composition.__set_conc(0);
                tf::DeviceType::type type = tf::DeviceType::type(m_CalibrateConcMode->item(i, caliBrowseColumnIndex::BROW_ASSAY_NAME_COLUMN)->data(Qt::UserRole + 3).toInt());
                composition.__set_deviceType(type);
                m_addCaliDocs[0].compositions.push_back(std::move(composition));
            }
        }
    }
}

ch::tf::CaliDoc CaliBrateRecordDialog::CreateTempCaliDoc(int level)
{
    ch::tf::CaliDoc caliDocTmp;
    caliDocTmp.compositions.clear();
    caliDocTmp.sn = ui->snedit->text().toStdString();
    caliDocTmp.lot = ui->lotedit->text().toStdString();
    // 默认为无效类型
    caliDocTmp.__set_tubeType(tf::TubeType::TUBE_TYPE_INVALID);
    caliDocTmp.level = level;
    return caliDocTmp;
}

///
/// @brief 析构函数
///     
/// @par History:
/// @li 5774/WuHongTao，2020年5月9日，新建函数
///
CaliBrateRecordDialog::~CaliBrateRecordDialog()
{
}

///
/// @brief
///     传递详细信息的数据
///
/// @param[in]  &  详细信息的数据
/// @param[in]  grouPinfo  组信息
///
/// @par History:
/// @li 5774/WuHongTao，2020年7月1日，新建函数
///
void CaliBrateRecordDialog::detailShow(QList<ch::tf::CaliDoc>& caliDoc, ch::tf::CaliDocGroup& groupInfo)
{
    // 扫描录入的校准品不允许修改
    if ((m_inputType == InputType::Input_Modify) && groupInfo.registType == tf::DocRegistType::REG_SCAN)
    {
        ui->calibrateBrowse->setEditTriggers(QTableView::NoEditTriggers);
        ui->snedit->setEnabled(false);
        ui->lotedit->setEnabled(false);
        ui->cali_meterial_type->setEnabled(false);
        ui->dateEdit->setEnabled(false);
    }
    else
    {
        ui->calibrateBrowse->setEditTriggers(QTableView::DoubleClicked);
        ui->snedit->setEnabled(true);
        ui->lotedit->setEnabled(true);
        ui->cali_meterial_type->setEnabled(true);
        ui->dateEdit->setEnabled(true);
    }

    m_addCaliDocs.clear();
    // 内容空，不需要显示
    if (caliDoc.empty())
    {
        return;
    }

	// 保存当前校准文档
	m_detailDocs = caliDoc;

	// 取消信号连接，避免数据更新过程中出现异常
	disconnect(m_CalibrateConcMode, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(updateDataList(QStandardItem*)));
	// 控件处理设置
	ui->calibrate_water->setChecked(groupInfo.systemWater);
	
	// 非ise校准品，设置校准品类型为生化校准品
	if (!groupInfo.iseType)
	{
		ui->cali_meterial_type->setCurrentIndex(0);
	}
	else
	{
		// 设置当前ise校准品类型
		for (int i = 0; i < ui->cali_meterial_type->count(); ++i)
		{
			if (ui->cali_meterial_type->itemData(i).toInt() == groupInfo.sampleSourceType)
			{
				ui->cali_meterial_type->setCurrentIndex(i);
				break;
			}
		}
	}
	connect(m_CalibrateConcMode, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(updateDataList(QStandardItem*)));

    // 保存不能测试的项目，后续显示和修改状态后进行同步
    m_disableAssayCodes = groupInfo.disableAssayCodes;
    m_modeType = MODIFY_MODE;
    //组的db
    m_groupDb = groupInfo.id;
    m_caligroup = groupInfo;
    //m_addCaliDocs = caliDoc;
    ui->nameEdit->setText(groupInfo.name.c_str());
    ui->snedit->setText(QString(groupInfo.sn.c_str()));
    ui->lotedit->setText(QString(groupInfo.lot.c_str()));
    //ui->numberEdit->setText(QString::number(caliDoc.size()));
	//ui->cali_sn_edit->setText(groupInfo.sn.c_str());
    //ui->numberEdit->setDisabled(true);
    // 设置当前时间
	ui->dateEdit->setDate(QDateTime::fromString(QString::fromStdString( groupInfo.expiryTime), "yyyy-MM-dd hh:mm:ss").date());
	m_addCaliDocs.clear();

	
	const ch::tf::CaliDoc& firstDoc = caliDoc.at(0);
	//生成临时校准文档
	for (size_t i = 1; i <= MAX_CALI_DOC_CNT; i++)
	{
        ch::tf::CaliDoc caliDocTmp;
		caliDocTmp = firstDoc;
		caliDocTmp.level = i;
        caliDocTmp.rack = "";
        caliDocTmp.__isset.rack = false;
        caliDocTmp.pos = -1;
        caliDocTmp.__isset.pos = false;
		caliDocTmp.compositions.clear();
		m_addCaliDocs.append(caliDocTmp);
	}

	for (const auto& doc : caliDoc)
	{
		m_addCaliDocs[doc.level - 1] =  doc;
	}
    m_lastCaliType = -1;
    OnChangeCalibrateType(ui->cali_meterial_type->currentIndex());

    //更新数据到界面
    updateMainCalibrateView(m_addCaliDocs);
}

///
/// @brief
///     显示新增校准品组界面（包括手动输入和扫描输入）
///
/// @param[in]  &  InputType 新增输入校准品的方式
///
/// @par History:
/// @li 8090/YeHuaNing，2021年8月24日，新建函数
///
void CaliBrateRecordDialog::setInputMode(const InputType & type)
{
	switch (type)
	{
	case InputType::Input_Manual:	// 手动输入
		this->SetManualInputMode();
		break;
	case InputType::Input_Scan:		// 扫描输入
		this->SetScanInputMode();
		break;
	case InputType::Input_Down:		// 下载输入
		this->SetDownInputMode();
		break;
	case InputType::Input_Modify:	// 修改
		this->SetModifyInputMode();
		break;
	default:
		break;
	}	
}

///
/// @brief 手动输入模式
///
///
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
///
void CaliBrateRecordDialog::SetManualInputMode()
{
	// 设置模式
	m_inputType = InputType::Input_Manual;
	// 初始化界面
	ui->stackedWidget->setCurrentWidget(ui->scan_hand_page);
	ui->scan->hide();
	ui->label_errorshow->hide();
	ui->scanEdit->hide();
	ui->calibrate_water->setChecked(false);
	ui->cali_meterial_type->setCurrentIndex(-1);
	ui->dateEdit->setNull();

	ui->nameEdit->setDisabled(false);
	ui->snedit->setDisabled(false);
    ui->lotedit->setDisabled(false);
	ui->cali_meterial_type->setDisabled(false);
	ui->dateEdit->setDisabled(false);
	ui->calibrate_water->setDisabled(false);
	ui->label_symbol->setVisible(false);
	ui->label_symbolName->setVisible(false);

	// 初始化控件和变量
    ui->nameEdit->setText("");
    ui->snedit->setText("");
	ui->lotedit->setText("");
	ui->yes->setText(tr("确定"));
    // 手工录入可以编辑浓度
    ui->calibrateBrowse->setEditTriggers(QTableView::DoubleClicked);

	// 清空历史存储的数据
	m_modeType = NEW_MODE;
	m_addCaliDocs.clear();
	m_disableAssayCodes.clear();

	//生成临时校准文档
	for (size_t i = 1; i <= MAX_CALI_DOC_CNT; i++)
	{
		ch::tf::CaliDoc caliDocTmp;
		caliDocTmp.compositions.clear();
        caliDocTmp.sn = ui->snedit->text().toStdString();
		caliDocTmp.lot = ui->lotedit->text().toStdString();
		// 默认为无效类型
		caliDocTmp.__set_tubeType(tf::TubeType::TUBE_TYPE_INVALID);
		caliDocTmp.level = i;
		m_addCaliDocs.append(std::move(caliDocTmp));
	}

	m_lastCaliType = -1;
	OnChangeCalibrateType(ui->cali_meterial_type->currentIndex());

	//更新数据到界面
	updateMainCalibrateView(m_addCaliDocs);
}

///
/// @brief 下载输入模式
///
///
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
///
void CaliBrateRecordDialog::SetDownInputMode()
{
	// 设置模式
	m_inputType = InputType::Input_Down;

	// 初始化界面
	ui->stackedWidget->setCurrentWidget(ui->download_page);
	ui->label_symbol->setVisible(true);
	ui->label_symbolName->setVisible(true);
	ui->yes->setText(tr("下载"));
	
	// 清空下载登记界面相关内容
	OnResetBtnClicked();
}

///
/// @brief 扫描输入模式
///
///
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
///
void CaliBrateRecordDialog::SetScanInputMode()
{
	// 设置模式
	m_inputType = InputType::Input_Scan;
	// 初始化界面
	ui->stackedWidget->setCurrentWidget(ui->scan_hand_page);
	ui->scan->show();
	ui->scanEdit->setFocus();
	ui->scanEdit->setText("");
	ui->label_errorshow->setText("");
	ui->label_errorshow->show();
	ui->scanEdit->show();
	ui->calibrate_water->setChecked(false);
	ui->cali_meterial_type->setCurrentIndex(-1);
	ui->dateEdit->setNull();

	ui->nameEdit->setDisabled(true);
    ui->nameEdit->setText("");
    ui->snedit->setDisabled(true);
    ui->snedit->setText("");
	ui->lotedit->setDisabled(true);
	ui->lotedit->setText("");
	ui->cali_meterial_type->setDisabled(true);
	ui->calibrate_water->setDisabled(false);
	ui->dateEdit->setDisabled(true);
	if (!ui->radioButton_scanModel->isChecked())  ui->radioButton_scanModel->setChecked(true);
	ui->label_symbol->setVisible(false);
	ui->label_symbolName->setVisible(false);

	ui->yes->setText(tr("确定"));
    // 扫描登记不可以编辑
    ui->calibrateBrowse->setEditTriggers(QTableView::NoEditTriggers);

	// 初始化相关缓存变量
	m_modeType = NEW_MODE;
	m_addCaliDocs.clear();
	m_disableAssayCodes.clear();

	// 初始化扫描相关信号
	InitScanInputSignal();

	//生成临时校准文档
	for (size_t i = 1; i <= MAX_CALI_DOC_CNT; i++)
	{
		ch::tf::CaliDoc caliDocTmp;
		caliDocTmp.compositions.clear();
        caliDocTmp.sn = ui->snedit->text().toStdString();
		caliDocTmp.lot = ui->lotedit->text().toStdString();
		// 默认为无效类型
		caliDocTmp.__set_tubeType(tf::TubeType::TUBE_TYPE_INVALID);
		caliDocTmp.level = i;
		m_addCaliDocs.append(std::move(caliDocTmp));
	}

	m_lastCaliType = -1;
	OnChangeCalibrateType(ui->cali_meterial_type->currentIndex());

	//更新数据到界面
	updateMainCalibrateView(m_addCaliDocs);

}

///
/// @brief 修改输入模式
///
///
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
///
void CaliBrateRecordDialog::SetModifyInputMode()
{
	// 设置模式
	m_inputType = InputType::Input_Modify;
	
}

///
/// @brief 初始化扫描相关信号
///
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
///
void CaliBrateRecordDialog::InitScanInputSignal()
{
	ULOG(LOG_INFO, __FUNCTION__);

	// 扫描流程控制初始化
	ScanControlInit();

	// 扫描检测定时器开启
	//m_barCodeTimer = std::shared_ptr<QTimer>(new QTimer(this));
	//connect(m_barCodeTimer.get(), SIGNAL(timeout()), this, SLOT(OnBarCodeTimerEvent()));
	//m_barCodeTimer->setInterval(800);
	//m_barCodeTimer->start();

	// 扫描输入框内容变化检测开启
	//connect(ui->scanEdit, SIGNAL(textChanged(const QString&)), this, SLOT(OnBarCodeEditChanged(const QString&)));
	connect(ui->scanEdit, SIGNAL(editingFinished()), this, SLOT(OnBarFinished()));
}

///
/// @brief 扫描流程控制初始化
///
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
///
void CaliBrateRecordDialog::ScanControlInit()
{
	// 清空上一次解析的二维码信息
	m_lastScanInputText.clear();

	// 清空二维码缓存
	m_QRcodes.clear();

}

///
/// @brief 初始化扫描UI
///
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年12月18日，新建函数
///
void CaliBrateRecordDialog::InitScanUI()
{
	// 清空输入框
	ui->nameEdit->clear();
	ui->snedit->clear();
	ui->lotedit->clear();
	ui->cali_meterial_type->setCurrentIndex(-1);
	ui->dateEdit->setNull();

	// 初始化输入表格
	ResetRecordAssayTable();

}

void CaliBrateRecordDialog::SetAscSortByFirstColumn()
{
    // 第一列按位置升序排列
    m_CalibrateConcMode->setSortRole(Qt::UserRole + 2);
    ui->calibrateBrowse->sortByColumn(caliBrowseColumnIndex::BROW_ASSAY_NAME_COLUMN, Qt::AscendingOrder);
    //去掉排序三角样式
    ui->calibrateBrowse->horizontalHeader()->setSortIndicator(-1, Qt::DescendingOrder);
}

///
/// @brief 扫描间隙时间节点到达
///
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
///
void CaliBrateRecordDialog::OnBarCodeTimerEvent()
{
	QString currentScanText = ui->scanEdit->text();
	PaserScanQRCodeInterface(currentScanText);
}

///
/// @brief 扫描输入框内容改变槽函数
///
/// @param[in]  barCode  输入内容
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
///
void CaliBrateRecordDialog::OnBarCodeEditChanged(const QString& barCode)
{
	PaserScanQRCodeInterface(barCode);
}

///
/// @brief 扫描输入完成槽函数
///
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
///
void CaliBrateRecordDialog::OnBarFinished()
{
	QString currentScanText = ui->scanEdit->text();

	// 遍历解决，以分号进行切片，然后遍历解析
	auto strList = currentScanText.split('\"', QString::SkipEmptyParts);

	// 遍历解析，防止粘包
	for (auto str : strList)
	{
		PaserScanQRCodeInterface(str);
	}
}

///
/// @brief 解析进入接口
///
/// @param[in]  barCode 扫描输入框中的数据
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
///
void CaliBrateRecordDialog::PaserScanQRCodeInterface(const QString& qrCode)
{
	// 非扫描模式or输入为空退出or与上次解析一致
	if (m_inputType != InputType::Input_Scan ||
		qrCode.isEmpty() ||
		qrCode == m_lastScanInputText)
	{
		return;
	}

	// 禁止操作
	ui->yes->setEnabled(false);
	ui->no->setEnabled(false);

	// 解析二维码
	PaserScanQRCodeToUi(qrCode);

	// 暂存本次解析内容，避免重复解析
	m_lastScanInputText = qrCode;

	// 解除禁止
	ui->yes->setEnabled(true);
	ui->no->setEnabled(true);
}

///
/// @brief 解析扫描框输入内容并更新到界面，
///        二维码公共信息用最后一张二维码
///
/// @param[in]  barCode 扫描输入框中的数据
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
///
void CaliBrateRecordDialog::PaserScanQRCodeToUi(const QString& qrCode)
{
	ULOG(LOG_INFO, __FUNCTION__);

	// 参数检查
	if (qrCode.isEmpty())	return;

	// 解析本次扫入的二维码
	::ch::tf::CaliGroupQRcode tempQRcode;
	if (!ch::LogicControlProxy::ParseCaliQRcode(tempQRcode, qrCode.toStdString())
		|| tempQRcode.lot.empty())
	{
		// 解析失败需要清空之前的信息
		if (m_QRcodes.isEmpty())
		{
			// 初始化信息
			InitScanUI();

			SetScanStatusNoticeInfo(false, tr("二维码解析错误！"));
		}
		else
		{
			// 如果是多页则提示某一页解析失败
			SetScanStatusNoticeInfo(false, tr("第%1页二维码解析错误！").arg(m_QRcodes.size() + 1));
		}
		
		return;
	}

	// 二维码信息范围校验(测试项目)，暂不对机型进行判断
	if (tempQRcode.assayClassify != tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY)
	{
		SetScanStatusNoticeInfo(false, tr("二维码类型不符合！"));
		return;
	}

	// 二维码同组校验,如果队列为空，则是第一张
	if (m_QRcodes.isEmpty())
	{
		// 第一页判断
		if (tempQRcode.currentPage != 1)
		{
			SetScanStatusNoticeInfo(false, tr("请扫描第1页！"));
			return;
		}

		// 初始化扫描UI信息
		InitScanUI();

		// 添加第一页
		m_QRcodes.append(tempQRcode);
	}
	else
	{
		// 与第一张比较
		if (   tempQRcode.lot != m_QRcodes.first().lot
			|| tempQRcode.sn  != m_QRcodes.first().sn
			|| tempQRcode.totalPage != m_QRcodes.first().totalPage)
		{
			SetScanStatusNoticeInfo(false, tr("二维码不是同一组！"));
			return;
		}

		// 与上一张比较
		if (tempQRcode.currentPage != m_QRcodes.last().currentPage + 1)
		{
			SetScanStatusNoticeInfo(false, tr("二维码顺序错误！"));
			return;
		}

		// 添加进缓存队列
		
		m_QRcodes.append(tempQRcode);
	}

	// 更新数据到界面
	if (!UpdateScanQRInfoToUI(tempQRcode))
	{
		SetScanStatusNoticeInfo(false, tr("二维码信息错误！"));
		return;
	}

	// 解析成功提示信息
	if (tempQRcode.currentPage == tempQRcode.totalPage)
	{
		// 扫描成功
		SetScanStatusNoticeInfo(true, tr("扫描完成！"));

		// 初始化控制，以备下一次扫描
		ScanControlInit();
	}
	else
	{
		SetScanStatusNoticeInfo(true, tr("扫描成功，总页数：") + QString::number(tempQRcode.totalPage) +
			tr("  当前页码： ") + QString::number(tempQRcode.currentPage));
	}

}

///
/// @brief 设置界面扫描状态
///
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
///
void CaliBrateRecordDialog::SetScanStatusNoticeInfo(bool isSuccess, QString noticeInfo)
{
	ui->label_errorshow->setText(noticeInfo);

	if (isSuccess)
	{
		// 清空扫描输入框
		ui->scanEdit->clear();
		ui->label_errorshow->setStyleSheet("color:green;font-size:16;");
	}
	else
	{
		ui->label_errorshow->setStyleSheet("color:red;font-size:16;");
	}
}

///
/// @brief 更新扫描内容到界面
///
/// @param[in]  QRcodeInfo 二维码信息
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
///
bool CaliBrateRecordDialog::UpdateScanQRInfoToUI(const ::ch::tf::CaliGroupQRcode& QRcodeInfo)
{
	ULOG(LOG_INFO, __FUNCTION__);

	// 如果是第一张则设置基本信息
	if (QRcodeInfo.currentPage == 1)
	{
		// 更新校准品组名称
		ui->nameEdit->setText(QString::fromStdString(QRcodeInfo.name));
		// 更新校准品类型
		ui->cali_meterial_type->setCurrentIndex(QRcodeInfo.calibratorType);
		// 更新校准品编号
		ui->snedit->setText(QString::fromStdString(QRcodeInfo.sn));
		// 更新校准品批号
		ui->lotedit->setText(QString::fromStdString(QRcodeInfo.lot));
		// 设置失效日期（生产日期+保质期（天））
		QDate expiringDate;
		if (!expiringDate.setDate(QRcodeInfo.year, QRcodeInfo.month, QRcodeInfo.day))
		{
			ULOG(LOG_ERROR, "Set expiring date error!");
			return false;
		}
		expiringDate = expiringDate.addDays(QRcodeInfo.expiration);
		ui->dateEdit->setDate(expiringDate);

		// 校准品1是否使用系统水
		ui->calibrate_water->setChecked(QRcodeInfo.composite);
	}
	
	// 更新校准文档列表（纵向描述）,不能大于MAX_CALI_DOC_CNT
	if (QRcodeInfo.compositions.size() > MAX_CALI_DOC_CNT || QRcodeInfo.levelNum > MAX_CALI_DOC_CNT)
	{
		ULOG(LOG_ERROR, "QRcodeInfo's compositions error!");
		return false;
	}

	// 遍历水平
	for (auto cdsIter = QRcodeInfo.compositions.cbegin(); cdsIter != QRcodeInfo.compositions.cend(); cdsIter++)
	{
		//水平判断
		if (cdsIter->first > MAX_CALI_DOC_CNT)
		{
			ULOG(LOG_ERROR, "compositions's level error!");
			return false;
		}

		// 遍历成分
		for (auto cdisIter = cdsIter->second.cbegin(); cdisIter != cdsIter->second.cend(); cdisIter++)
		{
			// 找到该列的某个项目（行）
			std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(cdisIter->assayCode);
			if (spAssayInfo == nullptr)
			{
				continue;
			}
			// 设置浓度
			auto aimItemList = m_CalibrateConcMode->findItems(QString::fromStdString(spAssayInfo->assayName),Qt::MatchExactly,0);

			if (aimItemList.size() != 1)
			{
				ULOG(LOG_ERROR, "m_CalibrateConcMode item error!");
				return false;
			}
			m_CalibrateConcMode->item(aimItemList.first()->row(), cdsIter->first)->setText(QString::number(cdisIter->conc, 'f'));
		}
	} 

	// 扫描流程控制初始化
	//ScanControlInit();

	// 缓存此次解析信息
	m_lastScanInfo = QRcodeInfo;

	return true;
}


///
/// @brief
///     获取校准数据
///
/// @param[in]  dataDoc  校准数据列表
/// @param[in]  modetype  对话框类型
///
/// @par History:
/// @li 5774/WuHongTao，2021年5月13日，新建函数
///
void CaliBrateRecordDialog::getData(QList<ch::tf::CaliDoc>& dataDoc, SHOWMODE& modetype)
{
    dataDoc = m_saveCaliDocs;
    modetype = m_modeType;
    m_saveCaliDocs.clear();
}

///
/// @brief
///     重置对话框状态
/// @par History:
/// @li 5774/WuHongTao，2020年10月16日，新建函数
///
void CaliBrateRecordDialog::reset()
{
    // 重新获取项目数据
    m_AssayItemList.clear();
    ResetRecordAssayTable();
    //GetAssayInfoList(m_AssayItemList);
    ui->nameEdit->clear();
    ui->snedit->clear();
    ui->lotedit->clear();
    //ui->numberEdit->clear();
	//ui->cali_sn_edit->clear();
}

void CaliBrateRecordDialog::hideEvent(QHideEvent * event)
{
    SetAscSortByFirstColumn();

    QWidget::hideEvent(event);
}

///
/// @brief 对话框的展示模式
///     
///
/// @param[in]  mode  展示模式
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年7月6日，新建函数
///
void CaliBrateRecordDialog::setMode(const SHOWMODE& mode)
{
	m_modeType = mode;

	switch (mode)
	{
	case SHOWMODE::NEW_MODE:
	{
		SetTitleName(tr("登记"));
		ResetRecordAssayTable();

	}break;

	case SHOWMODE::MODIFY_MODE:
	{
		// 初始化界面
		SetTitleName(tr("修改"));
		ui->stackedWidget->setCurrentWidget(ui->scan_hand_page);
		ui->scan->setVisible(false);
		ui->scanEdit->setVisible(false);
		ui->selectWidget->setVisible(false);
		ui->groupBox_out->setVisible(false);
		ui->label_errorshow->setVisible(false);
		ui->label_symbol->setVisible(false);
		ui->label_symbolName->setVisible(false);

		int resizeHight = ui->selectWidget->height() / 2;
		// 移动控件
		ui->stackedWidget->move(ui->stackedWidget->x(), ui->stackedWidget->y() - resizeHight);
		ui->line_h_5->move(ui->line_h_5->x(), ui->line_h_5->y() + resizeHight);
		ui->line_v_3->move(ui->line_v_3->x(), ui->calibrateBrowse->y());
		ui->line_v_4->move(ui->line_v_4->x(), ui->calibrateBrowse->y());

		// 调整控件大小
		ui->stackedWidget->resize(ui->stackedWidget->width(), ui->stackedWidget->height() + resizeHight);
		ui->groupBox_input->resize(ui->groupBox_input->width(), ui->groupBox_input->height() + resizeHight);
		ui->calibrateBrowse->resize(ui->calibrateBrowse->width(), ui->calibrateBrowse->height() + resizeHight);
		ui->line_v_3->resize(ui->line_v_3->width(), ui->line_h_5->y() - ui->line_h_tableTop->y());
		ui->line_v_4->resize(ui->line_v_4->width(), ui->line_h_5->y() - ui->line_h_tableTop->y());


	}break;

	default:
		break;
	}

}

///
/// @brief 更新对话框的显示列表
///     
/// @param[in]  dataShow  需要显示的参数
///
/// @par History:
/// @li 5774/WuHongTao，2020年5月9日，新建函数
///
void CaliBrateRecordDialog::updateMainCalibrateView(QList<ch::tf::CaliDoc>& dataShow)
{
    int row = 0;//行数目
	int columnCnt = m_CalibrateConcMode->columnCount();

    disconnect(m_CalibrateConcMode, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(updateDataList(QStandardItem*)));
	m_CalibrateConcMode->removeRows(0, m_CalibrateConcMode->rowCount());
	ui->calibrateBrowse->setItemDelegateForColumn(caliBrowseColumnIndex::BROW_ENABLE_SELECT_COLUMN, new CheckBoxDelegate(this));
	ui->calibrateBrowse->setItemDelegateForColumn(caliBrowseColumnIndex::BROW_MAIN_UNIT_COLUMN, new CReadOnlyDelegate(this));

    //如果没有项目，则直接返回
    if (m_AssayItemList.empty())
    {
        return;
    }

    //刷新成分信息
    for (auto assayInfo : m_AssayItemList)
    {
		// 根据项目名称获取项目信息
		std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(get<0>(assayInfo));
		if (spAssayInfo == nullptr)
		{
			ULOG(LOG_ERROR, "GetAssayCode error");
			continue;
		}
        // 设置项目名称
        QStandardItem* nameItem = new QStandardItem(get<0>(assayInfo).c_str());
        m_CalibrateConcMode->setItem(row, caliBrowseColumnIndex::BROW_ASSAY_NAME_COLUMN, nameItem);
        m_CalibrateConcMode->setData(m_CalibrateConcMode->index(row, caliBrowseColumnIndex::BROW_ASSAY_NAME_COLUMN), spAssayInfo->assayCode, Qt::UserRole + 2);
        // 存储设备信息，用于赋值，避免查找
        m_CalibrateConcMode->setData(m_CalibrateConcMode->index(row, caliBrowseColumnIndex::BROW_ASSAY_NAME_COLUMN), get<1>(assayInfo), Qt::UserRole + 3);
        bool selected = true;
		
		if (m_modeType != MODIFY_MODE)
		{
			// 新增时，默认勾选；修改时，根据以前的配置
			QStandardItem* item = new QStandardItem;
			item->setData(selected);
			m_CalibrateConcMode->setItem(row, caliBrowseColumnIndex::BROW_ENABLE_SELECT_COLUMN, item);
		}
		//根据禁用情况显示是否勾选
		else
		{
			if (m_disableAssayCodes.find(spAssayInfo->assayCode) != m_disableAssayCodes.end())
                selected = false;

            // 新增时，默认勾选；修改时，根据以前的配置
            QStandardItem* item = new QStandardItem;
            item->setData(selected);
            m_CalibrateConcMode->setItem(row, caliBrowseColumnIndex::BROW_ENABLE_SELECT_COLUMN, item);
		}
		
		int column = caliBrowseColumnIndex::BROW_CALIBRATOR_1_COLUMN;//列数
        // ISE校准品浓度默认为1
        int decimalPlace = 1;
        for(auto caliDoc : dataShow)
        {
            //项目浓度信息
            QStandardItem* tmp = new QStandardItem(QString(""));
            for (const auto& compostionVar : caliDoc.compositions)
            {
				// 如果使用系统水，则将校准品1设置为空白，否则设置浓度值
                if (compostionVar.assayCode == spAssayInfo->assayCode)
                {
                    if (m_lastCaliType != tf::SampleSourceType::SAMPLE_SOURCE_TYPE_XQXJ &&
                        m_lastCaliType != tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NY)
                        decimalPlace = spAssayInfo->decimalPlace;

                    QString strConc = QString::number(compostionVar.conc, 'f', decimalPlace);
                    /*if (strConc.contains("."))
                    {
                        while (strConc.endsWith("0"))
                        {
                            strConc.chop(1);
                        }

                        while (strConc.endsWith("."))
                        {
                            strConc.chop(1);
                        }
                    }*/
					tmp->setText(strConc);
                    break;
                }
            }

            m_CalibrateConcMode->setItem(row, column++, tmp);
			// 存储水平号
            m_CalibrateConcMode->setData(tmp->index(), caliDoc.level, Qt::UserRole + 1);

			// 目前界面只支持6个水平
			if (column > caliBrowseColumnIndex::BROW_CALIBRATOR_6_COLUMN) break;
        }

		// 设置单位
		m_CalibrateConcMode->setItem(row, caliBrowseColumnIndex::BROW_MAIN_UNIT_COLUMN, new QStandardItem(get<2>(assayInfo)));

        row++;
    }

    //展示数据以后，跟踪修改
    connect(m_CalibrateConcMode, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(updateDataList(QStandardItem*)));
    //强行启动重新绘制
    //this->resize(this->width() - 1, this->height() - 1);
    SetAscSortByFirstColumn();
	emit m_CalibrateConcMode->layoutChanged();
}

///
/// @brief 下载登记查询按钮点击
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月15日，新建函数
///
void CaliBrateRecordDialog::OnQueryBtnClicked()
{
	ULOG(LOG_INFO, __FUNCTION__);

	// 获取参数
	QString caliName = ui->lineEdit_cali_name->text();
	QString caliLot = ui->lineEdit__cali_lot->text();

	// 验证空
	if (caliName.isEmpty() && caliLot.isEmpty())
	{
		TipDlg(tr("查询校准品信息时名称和批号不能同时为空，请重新输入！")).exec();
		return;
	}

	// 验证名称
	if (!caliName.isEmpty() && caliName.length() > 30)
	{
		TipDlg(tr("校准品名称长度不能超过30个字符，请重新输入！")).exec();
		return;
	}

	// 验证批号
	if (!caliLot.isEmpty() && caliLot.length() != 7)
	{
		TipDlg(tr("批号长度为7个字符，请重新输入！")).exec();
		return;
	}

	// 有效期(0:全部，1:有效期内)
	int expiry = ui->comboBox_expire_date->currentIndex();

	// 获取查询条件
	::adapter::chCloudDef::tf::QueryCloudCaliDocGroupCond qry;
	qry.__set_productAreas(adapter::tf::CloudProductArea::AREA_CHINA);
	qry.__set_productSystem(adapter::tf::CloudProductSystem::SYSTEM_CHEMISTRY);
	qry.__set_deviceType(tf::DeviceType::DEVICE_TYPE_C1000);
	qry.__set_calibratorName(caliName.toStdString());
	qry.__set_calibratorLot(caliLot.toStdString());
	qry.__set_hasExpiry(expiry);

	// 防止多次进入
	if (m_isRequesting)
	{
		return;
	}

	// 清空当前结果
	m_CalibrateDownMode->removeRows(0, m_CalibrateDownMode->rowCount());
	m_downloadDataCache.clear();

	// 创建一个新的线程来查询
	auto searchFunc = [&](::adapter::chCloudDef::tf::QueryCloudCaliDocGroupCond qccdgc) {
		
		// 初始化状态
		POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("查询中..."), true);
		connect(this, &CaliBrateRecordDialog::searchResult, this, &CaliBrateRecordDialog::OnSearchHander);
		m_isRequesting = true;

		bool ret = true;
		::adapter::chCloudDef::tf::CloudCaliDocGroupResp resp;
		if (!ch::LogicControlProxy::DownloadCaliDocGroup(resp, qccdgc) || !resp.__isset.result ||
			resp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
		{
			ret = false;
		}
		else
		{
			// 更新缓存
			m_downloadDataCache = resp.lstCaliDocGroup;
		}

		emit searchResult(ret);
	};
	std::thread searchThread(searchFunc, qry);
	searchThread.detach();
}

///
/// @brief 查询处理
///
/// @param[in]  isSuccess  是否查询成功
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年4月8日，新建函数
///
void CaliBrateRecordDialog::OnSearchHander(bool isSuccess)
{
	// 复位条件
	m_isRequesting = false;
	disconnect(this, &CaliBrateRecordDialog::searchResult, this, &CaliBrateRecordDialog::OnSearchHander);
	POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);

	// 查询失败
	if (!isSuccess)
	{
		TipDlg(tr("校准品查询失败！")).exec();
		return;
	}

	// 填写表格
	UpdateCalibrateDownData();
}

///
/// @brief 更新下载登记表格
///
/// @param[in]  vccdg  下载的数据
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月20日，新建函数
///
void CaliBrateRecordDialog::UpdateCalibrateDownData()
{
	ULOG(LOG_INFO, __FUNCTION__);

	// 参数判断
	if (m_downloadDataCache.size() <= 0)
	{
		TipDlg(tr("校准品下载为空，请重新设置查询条件！")).exec();
	}

	auto CenterAligmentItem = [&](const QString & text)->QStandardItem*{
		QStandardItem * item = new QStandardItem(text);
		item->setTextAlignment(Qt::AlignCenter);
		return item;
	};

	auto GetDateItem = [](const QString & StrDate)->QStandardItem* {

		QStandardItem * item = new QStandardItem(StrDate);
		item->setTextAlignment(Qt::AlignCenter);

		// 失效日期
		QDateTime expDate = QDateTime::fromString(StrDate, "yyyy-MM-dd hh:mm:ss");
		if (expDate.isValid())
		{
			item->setText(expDate.toString("yyyy/MM/dd"));
		}

		// 是否过期
		if (expDate.isValid() && expDate.date() <= QDate::currentDate())
		{
			item->setData(UI_REAGENT_WARNCOLOR, Qt::UserRole + 5);
			item->setData(QColor(UI_REAGENT_WARNFONT), Qt::ForegroundRole);

		}

		return item;
	};

	// 清空下载登记表格
	m_CalibrateDownMode->removeRows(0, m_CalibrateConcMode->rowCount());

	// 校准品信息
	int rowIndex = 0;
	for (const auto& docGroupInfo : m_downloadDataCache)
	{
		// 校准品组开始行
		int docStartRow = rowIndex;
		int docSpanSize = 0;

		// 校准品名称
		m_CalibrateDownMode->setItem(rowIndex, caliDownColumnIndex::DOWN_CALIBRATOR_NAME_COLUMN, CenterAligmentItem(QString::fromStdString(docGroupInfo.calibratorName)));
		// 校准品批号
		m_CalibrateDownMode->setItem(rowIndex, caliDownColumnIndex::DOWN_CALIBRATOR_LOT_COLUMN, CenterAligmentItem(QString::fromStdString(docGroupInfo.calibratorLot)));
		// 失效日期
		m_CalibrateDownMode->setItem(rowIndex, caliDownColumnIndex::DOWN_EXPIRE_DATE_COLUMN, GetDateItem(QString::fromStdString(docGroupInfo.expireTime)));

		// 项目信息
		for (const auto& assayInfo : docGroupInfo.lstAssayInfo)
		{
			// 项目信息开始行
			int assayStartRow = rowIndex;
			
			// 项目名称
			auto genInfo = CommonInformationManager::GetInstance()->GetAssayInfo(assayInfo.assayCode);
			QString assayName = (nullptr == genInfo) ? QString::number(assayInfo.assayCode) : QString::fromStdString(genInfo->printName);
			m_CalibrateDownMode->setItem(rowIndex, caliDownColumnIndex::DOWN_ASSAY_NAME_COLUMN, CenterAligmentItem(assayName));

			// 试剂信息
			QList<QVariant> assaySelectRows;
			for (const auto& reagentInfo : assayInfo.lstCaliConcInfo)
			{
				// 试剂批号
				if (reagentInfo.reagentLot.empty())
				{
					// 不区分试剂批号
					auto reaItem = CenterAligmentItem("");
					reaItem->setData(QPixmap(QString(REAGENT_LOT_EMPTY_PNG)), Qt::UserRole + 3);
					m_CalibrateDownMode->setItem(rowIndex, caliDownColumnIndex::DOWN_REAGENT_LOT_COLUMN, reaItem);
				}
				else
				{
					m_CalibrateDownMode->setItem(rowIndex, caliDownColumnIndex::DOWN_REAGENT_LOT_COLUMN, CenterAligmentItem(QString::fromStdString(reagentInfo.reagentLot)));
				}

				// 选择(默认不选中)
				auto selectItem = CenterAligmentItem("");
				selectItem->setIcon(QIcon());
				selectItem->setData(assayStartRow, CALIBRATE_DOWN_ASSAY_INDEX);
				m_CalibrateDownMode->setItem(rowIndex, caliDownColumnIndex::DOWN_SELECT_COLUMN, selectItem);
				assaySelectRows.push_back(QVariant(rowIndex));
				rowIndex++;
			}

			// 设置子类行索引列表
			m_CalibrateDownMode->item(assayStartRow, caliDownColumnIndex::DOWN_ASSAY_NAME_COLUMN)->setData(assaySelectRows, CALIBRATE_DOWN_SELECT_INDEX);

			// 项目信息合并行
			docSpanSize += assayInfo.lstCaliConcInfo.size();
			ui->calibrateDown->setSpan(assayStartRow, caliDownColumnIndex::DOWN_ASSAY_NAME_COLUMN, assayInfo.lstCaliConcInfo.size(), 1);
		}

		// 校准品信息行合并
		ui->calibrateDown->setSpan(docStartRow, caliDownColumnIndex::DOWN_CALIBRATOR_NAME_COLUMN, docSpanSize, 1);
		ui->calibrateDown->setSpan(docStartRow, caliDownColumnIndex::DOWN_CALIBRATOR_LOT_COLUMN, docSpanSize, 1);
		ui->calibrateDown->setSpan(docStartRow, caliDownColumnIndex::DOWN_EXPIRE_DATE_COLUMN, docSpanSize, 1);
	}

}

///
/// @brief 保存下载登记数据
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月20日，新建函数
///
void CaliBrateRecordDialog::SaveCalibrateDownData()
{
	ULOG(LOG_INFO, __FUNCTION__);

	// 参数判断
	if (m_CalibrateDownMode == nullptr || m_CalibrateDownMode->rowCount() == 0)
	{
		// 请至少选择一个校准文档
		TipDlg(tr("未查询校准品或查询结果为空！")).exec();
		return;
	}

	// 是否选中
	auto IsSelectedItem = [&](int rowIndex)->bool{
		auto item = m_CalibrateDownMode->item(rowIndex, caliDownColumnIndex::DOWN_SELECT_COLUMN);
		// 为空或者没有图标就判定为没选中
		if (item == nullptr || item->icon().isNull())
		{
			return false;
		}

		return true;
	};

	// 判断是否存在校准文档组
	auto IsExistCaliDocGroup = [&](const std::string& lot, const std::string& sn) ->bool {

		::ch::tf::CaliDocGroupQueryResp ResultDocGroup;
		::ch::tf::CaliDocGroupQueryCond queryDocGroupCondition;
		queryDocGroupCondition.__set_lot(lot);
		queryDocGroupCondition.__set_sn(sn);

		if (!ch::LogicControlProxy::QueryCaliDocGroup(ResultDocGroup, queryDocGroupCondition)
			|| ResultDocGroup.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
			|| ResultDocGroup.lstCaliDocGroup.empty())
		{
			return false;
		}

		return true;
	};

	// 需要缓存的数据
	std::set<std::pair<std::string, std::string>> saveCacheSet;
	std::vector<CaliDocSaveStru> saveDataCache;

	// 存在重复判断
	bool dbExistFlag = false;
	bool cacheExistFlag = false;

	// 以行索引作为映射
	int rowIndex = 0;
	for (const auto& docGroupInfo : m_downloadDataCache)
	{
		// 判断数据库是否已经有了该校准品组
		if (IsExistCaliDocGroup(docGroupInfo.calibratorLot, docGroupInfo.calibratorSn))
		{
			dbExistFlag = true;
			continue;
		}

		// 判断缓存是否已经存在
		if (saveCacheSet.find({ docGroupInfo.calibratorLot, docGroupInfo.calibratorSn }) != saveCacheSet.end())
		{
			cacheExistFlag = true;
			continue;
		}

		// 创建一个校准文档组
		ch::tf::CaliDocGroup groupTemp;

		groupTemp.__set_name(docGroupInfo.calibratorName);
		groupTemp.__set_lot(docGroupInfo.calibratorLot);
		groupTemp.__set_sn(docGroupInfo.calibratorSn);
		QDateTime expDateTime = QDateTime::fromString(QString::fromStdString(docGroupInfo.expireTime), UI_DATE_FORMAT);
		groupTemp.__set_expiryTime(expDateTime.toString(UI_DATE_TIME_FORMAT).toStdString());
		groupTemp.__set_registType(tf::DocRegistType::REG_DOWNLOAD);
		groupTemp.__set_systemWater(docGroupInfo.composite);

		// 设备类型
		tf::DeviceType::type devType = tf::DeviceType::DEVICE_TYPE_INVALID;

		// 样本类型
		switch (docGroupInfo.calibratorType)
		{
		case adapter::chCloudDef::tf::CloudCalibratorType::CALIBRATOR_TYPE_CH:
		{
			groupTemp.__set_iseType(false);
			groupTemp.__set_sampleSourceType(tf::SampleSourceType::SAMPLE_SOURCE_TYPE_OTHER);
			devType = tf::DeviceType::DEVICE_TYPE_C1000;
		}break;

		case adapter::chCloudDef::tf::CloudCalibratorType::CALIBRATOR_ISE_XQ:
		{
			groupTemp.__set_iseType(true);
			groupTemp.__set_sampleSourceType(tf::SampleSourceType::SAMPLE_SOURCE_TYPE_XQXJ);
			devType = tf::DeviceType::DEVICE_TYPE_ISE1005;
		}break;

		case adapter::chCloudDef::tf::CloudCalibratorType::CALIBRATOR_ISE_NY:
		{
			groupTemp.__set_iseType(true);
			groupTemp.__set_sampleSourceType(tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NY);
			devType = tf::DeviceType::DEVICE_TYPE_ISE1005;
		}break;

		default:
			break;
		}
		
		// 记录不同水平的校准文档和校准文档关键信息
		std::vector<ch::tf::CaliDoc> docVecTemp;
		std::vector<ch::tf::CaliDocKeyInfo> caliKeyInfosTemp;
		for (const auto& assayInfo : docGroupInfo.lstAssayInfo)
		{
			// 找到该项目被选中的水平信息
			for (const auto& reagentInfo : assayInfo.lstCaliConcInfo)
			{
				// 被选中
				if (IsSelectedItem(rowIndex))
				{
					// 水平数量判断
					if (reagentInfo.lstLevelInfo.size() > MAX_CALI_DOC_CNT)
					{
						TipDlg(tr("%1:水平数量大于6！").arg(QString::fromStdString(docGroupInfo.calibratorName))).exec();
						return;
					}

					// 遍历水平
					bool firstSelected = docVecTemp.size() == 0 ? true : false;
					for (auto levelInfo : reagentInfo.lstLevelInfo)
					{
						// 初次创建
						if (firstSelected)
						{
							// 记录文档信息
							ch::tf::CaliDoc docTemp;
							docTemp.__set_lot(docGroupInfo.calibratorLot);
							docTemp.__set_sn(docGroupInfo.calibratorSn);
							docTemp.__set_level(levelInfo.level);
							docTemp.__set_oneDimensionalCode(GetCaliDocOneDimensionalCode(docGroupInfo.calibratorSn, docGroupInfo.calibratorLot, levelInfo.level));
							docVecTemp.push_back(docTemp);

							// 记录关键信息
							ch::tf::CaliDocKeyInfo caliKeyInfo;
							caliKeyInfo.__set_level(levelInfo.level);
							caliKeyInfo.__set_lot(docGroupInfo.calibratorLot);
							caliKeyInfo.__set_sn(docGroupInfo.calibratorSn);
							caliKeyInfosTemp.push_back(caliKeyInfo);
						}

						// 判断同一个校准品不同项目水平是否一致
						auto docIter = std::find_if(docVecTemp.begin(), docVecTemp.end(), [&levelInfo](auto& doc) {return (levelInfo.level == doc.level); });
						if (docIter == docVecTemp.end())
						{
							// 不同则失败
							TipDlg(tr("同一个校准品不同项目水平数量不一致！")).exec();
							return;
						}
						
						// 读出来添加后再存进去
						auto compList = docIter->compositions;
						if (!docIter->__isset.compositions)
						{
							compList.clear();
						}

						// 根据当前水平设置成分信息
						ch::tf::CaliComposition compTemp;
						compTemp.__set_assayCode(assayInfo.assayCode);
						compTemp.__set_conc(levelInfo.conc);
						compTemp.__set_deviceType(devType);

						compList.push_back(compTemp);
						docIter->__set_compositions(compList);
					}
				}
				
				rowIndex++;
			}
		}

		// 文档列表不为空，说明该校准品被选中，则扔进缓存
		if (docVecTemp.size() > 0)
		{
			// ISE检查
			if (groupTemp.iseType)
			{
				if (docVecTemp[0].compositions.size() != 3)
				{
					// ISE需要
					TipDlg(tr("ISE必须同时包含Na、K、Cl三个项目！")).exec();
					return;
				}
			}

			// 设置关键信息
			groupTemp.__set_caliDocKeyInfos(caliKeyInfosTemp);

			// 存进缓存
			saveDataCache.push_back({ groupTemp, docVecTemp });
			// 添加记录，防止重复添加
			saveCacheSet.insert({ groupTemp.lot, groupTemp.sn });
		}
	}

	// 数据库存在
	QString dbExistStr = dbExistFlag ? tr("已跳过存在的校准品。") : "";

	// 重复勾选
	QString cacheExistStr = cacheExistFlag ? tr("已跳过重复勾选的校准品。") : "";

	// 判空
	if (saveDataCache.size() <= 0)
	{
		// 请至少选择一个校准文档
		TipDlg(tr("请至少选择一个校准品下载！") + dbExistStr).exec();
		return;
	}
	
	// 将转换后的信息存储
	if (!AddCalibrateInfoToDb(saveDataCache))
	{
		// 保存失败
		TipDlg(tr("下载失败！")).exec();
		return;
	}

	// 退出
	TipDlg(tr("下载成功！") + dbExistStr + cacheExistStr).exec();
}

///
/// @brief 将校准品信息添加进数据库
///
/// @param[in]  saveDataCache  数据
///
/// @return true:成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月20日，新建函数
///
bool CaliBrateRecordDialog::AddCalibrateInfoToDb(const std::vector<CaliDocSaveStru>& saveDataCache)
{
	ULOG(LOG_INFO, __FUNCTION__);

	// 参数判断
	if (saveDataCache.empty())
	{
		return false;
	}

	// 遍历存储
	for (auto data : saveDataCache)
	{
		// 添加进去
		if (!ch::LogicControlProxy::AddNewCaliDocGroup(data.docGroup, data.docVec))
		{
			return false;
		}

        // 此处不抛出消息更新校准架概况，因为dcs调用了校准设置页面的校准品组更新消息
        //POST_MESSAGE(MSG_ID_CH_CALI_SET_UPDATE, tf::UpdateType::UPDATE_TYPE_ADD, data.docGroup, 
            //QList<ch::tf::CaliDoc>::fromVector(QVector<ch::tf::CaliDoc>::fromStdVector(data.docVec)));
	}

	return true;
}

std::string CaliBrateRecordDialog::GetModifyCaliInfoLog(const ch::tf::CaliDocGroup & oriGroup, 
    const ch::tf::CaliDocGroup & newGroup, const QList<ch::tf::CaliDoc>& oriDocs, const QList<ch::tf::CaliDoc>& newDocs)
{
    QStringList opLogs;
    std::map<int32_t, std::vector<std::set<double>>> concInfos;

    // 校准品类型名称
    auto funcGetSourceName = [](int32_t source, bool iseType)->QString
    {
        if (!iseType /*::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_OTHER*/)
        {
            return tr("生化校准品");
        }
        else if (source == ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_XQXJ)
        {

            return tr("ISE血清校准品");
        }
        else if (source == ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NY)
        {

            return tr("ISE尿液校准品");
        }

        return "";
    };

    // 系统水启用状态
    auto funcSysWaterEnableName = [](bool syswater)->QString {
        if (syswater)
            return tr("使用");

        return tr("不使用");
    };

    // 统计校准文档数据
    auto funcGetDocInfos = [&concInfos](const QList<ch::tf::CaliDoc>& Docs, const std::set<int32_t>& disableAssayCodes, int i) {
        for (const auto& doc : Docs)
        {
            for (const auto& com : doc.compositions)
            {
                // 没有这个项目
                if (concInfos.find(com.assayCode) == concInfos.end())
                {
                    concInfos[com.assayCode].emplace_back(std::set<double>{});
                    concInfos[com.assayCode].emplace_back(std::set<double>{});
                }

                if (i < concInfos[com.assayCode].size())
                    concInfos[com.assayCode][i].emplace(com.conc);
            }
        }

        for (const auto& disAssay : disableAssayCodes)
        {
            if (concInfos.find(disAssay) == concInfos.end())
            {
                concInfos[disAssay].emplace_back(std::set<double>{});
                concInfos[disAssay].emplace_back(std::set<double>{});
            }

            if (i < concInfos[disAssay].size())
                concInfos[disAssay][i].emplace(-1);
        }
    };

    // 获取项目对应的校准品信息数据
    auto funcAssaydocInfo = [](const std::set<double>& assayConcInfos)->QString
    {
        QString enable(tr("启用"));
        QString strCatInfo;
        for (const auto& conc : assayConcInfos)
        {
            if (conc < 0)
            {
                enable = tr("不启用");
            }
            else
            {
                strCatInfo += (QString::number(conc) + ",");
            }
        }
        strCatInfo += enable;

        return strCatInfo;
    };

    /*
        编辑了校准品：
        校准品组名称："A"修改为"B"
        AST: "" 修改为"0.25, 0.35 启用"
        ALT: "0.25 0.36 启用" 修改为 ""
        ALP: "启用"修改为"不启用"
    */
    if (oriGroup != newGroup)
    {
        // 校准品名称
        if (oriGroup.name != newGroup.name)
        {
            opLogs << QString("校准品组名称：[%1] 修改为 [%2]").arg(QString::fromStdString(oriGroup.name), QString::fromStdString(newGroup.name));
        }

        // 校准品编号
        if (oriGroup.sn != newGroup.sn)
        {
            opLogs << QString("校准品编号：[%1] 修改为 [%2]").arg(QString::fromStdString(oriGroup.sn), QString::fromStdString(newGroup.sn));
        }

        // 校准品批号
        if (oriGroup.lot != newGroup.lot)
        {
            opLogs << QString("校准品批号：[%1] 修改为 [%2]").arg(QString::fromStdString(oriGroup.lot), QString::fromStdString(newGroup.lot));
        }

        // 校准品类型
        if (oriGroup.sampleSourceType != newGroup.sampleSourceType)
        {
            opLogs << QString("校准品类型：[%1] 修改为 [%2]").arg(
                funcGetSourceName(oriGroup.sampleSourceType, oriGroup.iseType), 
                funcGetSourceName(newGroup.sampleSourceType, newGroup.iseType));
        }

        // 失效日期
        auto oriDate = QString::fromStdString(oriGroup.expiryTime).split(" ")[0];
        auto newDate = QString::fromStdString(newGroup.expiryTime).split(" ")[0];
        if (oriDate != newDate)
        {
            opLogs << QString("失效日期：[%1] 修改为 [%2]").arg(oriDate, newDate);
        }

        // 校准品1使用系统水
        if (oriGroup.systemWater != newGroup.systemWater)
        {
            opLogs << QString("校准品1使用系统水：[%1] 修改为 [%2]").arg(
                funcSysWaterEnableName(oriGroup.systemWater), 
                funcSysWaterEnableName(newGroup.systemWater));
        }
    }

    // 将原有信息都拆分出来
    funcGetDocInfos(oriDocs, oriGroup.disableAssayCodes, 0);
    funcGetDocInfos(newDocs, newGroup.disableAssayCodes, 1);

    // 获取校准文档修改信息
    for (const auto& info : concInfos)
    {
        int assayCode = info.first;
        const auto& vecInfos = info.second;
        const auto& assayName = CommonInformationManager::GetInstance()->GetAssayNameByCode(assayCode);

        // 防御一下
        if (vecInfos.size() < 2)
            continue;

        // 没有变更
        if (vecInfos[0] == vecInfos[1])
            continue;

        opLogs << tr("%1：[%2] 修改为 [%3]").arg(QString::fromStdString(assayName), funcAssaydocInfo(vecInfos[0]), funcAssaydocInfo(vecInfos[1]));
    }

    // 证明没有内容变更
    if (opLogs.isEmpty())
        return std::string();

    return opLogs.join('\n').toStdString();
}

void CaliBrateRecordDialog::AddOperateLog(const std::string & info, int type)
{
    ::tf::ResultLong _return;
    ::tf::OperationLog ol;
    auto spUser = UserInfoManager::GetInstance()->GetLoginUserInfo();
    if (spUser)
    {
        ol.__set_user(spUser->nickname);
    }
    //ol.__set_deviceSN("4");
    ol.__set_deviceClassify(tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY);
    ol.__set_operationType(tf::OperationType::type(type));
    ol.__set_operationTime(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss").toStdString());
    ol.__set_operationRecord(info);
    if (!DcsControlProxy::GetInstance()->AddOperationLog(_return, ol))
    {
        ULOG(LOG_ERROR, "AddOperationLog failed, info: %s.", info);
    }
}

///
/// @brief 下载登记重置按钮点击
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月15日，新建函数
///
void CaliBrateRecordDialog::OnResetBtnClicked()
{
	ULOG(LOG_INFO, __FUNCTION__);

	// 清空查询条件
	ui->lineEdit_cali_name->clear();
	ui->lineEdit__cali_lot->clear();
	ui->comboBox_expire_date->setCurrentIndex(1);

	// 清空下载登记表格
	m_CalibrateDownMode->removeRows(0, m_CalibrateDownMode->rowCount());

	// 清空缓存
	m_downloadDataCache.clear();
}

///
/// @brief 下载登记选择改变
///
/// @param[in]  modelIndex  操作索引
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月20日，新建函数
///
void CaliBrateRecordDialog::OnCalibrateDownSelectedChanged(const QModelIndex& modelIndex)
{
	// 参数检查
	if (!modelIndex.isValid() || !m_CalibrateDownMode->item(modelIndex.row(), caliDownColumnIndex::DOWN_SELECT_COLUMN)
		|| modelIndex.column() != caliDownColumnIndex::DOWN_SELECT_COLUMN)
	{
		return;
	}

	// 当前操作Item
	auto currentItem = m_CalibrateDownMode->item(modelIndex.row(), caliDownColumnIndex::DOWN_SELECT_COLUMN);

	// 当前状态
	bool bChecked = !currentItem->icon().isNull();
	if (bChecked)
	{
		// 设置非选中，并退出
		currentItem->setIcon(QIcon());
		return;
	}
	else
	{
		// 设置为选中，并进行互斥处理
		currentItem->setIcon(QIcon(SELECTED_PNG));
	}

	// 获取项目行索引
	auto assayRowIndex = currentItem->data(CALIBRATE_DOWN_ASSAY_INDEX);
	if (!assayRowIndex.isValid())
	{
		return;
	}

	// 获取项目行ITEM
	auto assayItem = m_CalibrateDownMode->item(assayRowIndex.toInt(), caliDownColumnIndex::DOWN_ASSAY_NAME_COLUMN);
	if (assayItem == nullptr)
	{
		return;
	}

	// 获取选择行数据List
	auto selectList = assayItem->data(CALIBRATE_DOWN_SELECT_INDEX);
	if (!selectList.isValid())
	{
		return;
	}
	
	// 当前项目所拥有的行
	auto selectListIndexs = selectList.toList();
	for (auto selectIndex : selectListIndexs)
	{
		// 合法性
		if (!selectIndex.isValid())
		{
			continue;
		}

		int rowIndex = selectIndex.toInt();

		// 如果是当前行，则不作处理
		if (rowIndex == modelIndex.row())
		{
			continue;
		}

		// 其余行取消图标
		m_CalibrateDownMode->item(rowIndex, caliDownColumnIndex::DOWN_SELECT_COLUMN)->setIcon(QIcon());
	}

}