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
/// @file     ImCaliBrateRecordDialog.h
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
#pragma once
#include <tuple>
#include "QVector"
#include "QPoint"
#include "shared/basedlg.h"
#include "src/thrift/im/gen-cpp/im_types.h"

class QStandardItemModel;
class QStandardItem;

namespace Ui {
    class ImCaliBrateRecordDialog;
};


#define SCAN_TIMER_INTERVAL                        (100)                // 扫码定时器间隔500(ms)
#define SCAN_TIMER_MAX_OVER_TIME_COUNT             (5)                  // 扫码定时器最大超时次数

//录入方式
enum InputType
{
	Input_Scan = 1,	    //扫描输入
	Input_Down,			//下载输入
};

//浓度设置列表Column索引（calibrateBrowse）
enum caliBrowseColumnIndex {
	BROW_ASSAY_NAME_COLUMN = 0,			// 项目名称
	BROW_CALIBRATOR_1_COLUMN ,			// 校准品1
	BROW_CALIBRATOR_2_COLUMN ,			// 校准品2
	BROW_CALIBRATOR_3_COLUMN ,			// 校准品3
	BROW_CALIBRATOR_4_COLUMN ,			// 校准品4
	BROW_CALIBRATOR_5_COLUMN ,			// 校准品5
	BROW_CALIBRATOR_6_COLUMN ,			// 校准品6
	BROW_MAIN_UNIT_COLUMN ,				// 单位（使用的是主单位）
	BROW_ENABLE_SELECT_COLUMN			// 启用
};

//浓度下载登记列表Column索引（calibrateDown）
enum caliDownColumnIndex {
	DOWN_CALIBRATOR_NAME_COLUMN = 0,	// 校准品名称
	DOWN_CALIBRATOR_LOT_COLUMN,			// 校准品批号
	DOWN_EXPIRE_DATE_COLUMN ,			// 失效日期
	DOWN_ASSAY_NAME_COLUMN ,			// 项目名称
	DOWN_REAGENT_LOT_COLUMN ,			// 试剂批号
	DOWN_SELECT_COLUMN 					// 选择(下载)
};

class ImCaliBrateRecordDialog :
    public BaseDlg
{
    Q_OBJECT
public:
    ///
    /// @brief 新增对话框的构造函数
    ///     
    /// @param[in]  parent  
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年5月9日，新建函数
    ///
    ImCaliBrateRecordDialog(QWidget *parent = Q_NULLPTR);

    ///
    /// @brief 析构函数
    ///     
    /// @par History:
    /// @li 5774/WuHongTao，2020年5月9日，新建函数
    ///
    ~ImCaliBrateRecordDialog();

    void Init();
    void ResetRecordAssayTable();
    void CleanBeforeQuit();
    void setInputMode(const InputType & type);
    void SetDownInputMode();
    void SetScanInputMode();  
    void SetScanStatusNoticeInfo(bool isSuccess, QString noticeInfo);

protected Q_SLOTS:
    void addNewCaliIntoDatabase();
    void OnBarcodeChanged();
    void OnScanTimer();

signals:
    void FinishCaliInfo(const im::tf::CaliDocGroup& grp);
   
private:
    Ui::ImCaliBrateRecordDialog*    ui;
    QString                         m_calibrationSampleBarcode; ///< 校准样品条码
    QStandardItemModel*             m_CalibrateConcMode;        ///< main的mode
	QStandardItemModel*             m_CalibrateDownMode;        ///< 下载登记的mode
    QPoint                          m_globalPos;                ///< 鼠标的世界坐标
    QPoint                          m_windowsPos;               ///< 窗口的世界坐标
    bool                            m_press;                    ///< 鼠标是否按下
    QVector<std::tuple<std::string, int, QString>>     m_AssayItemList;  ///< 测试项目信息(项目名称，项目编号，主单位)

	std::shared_ptr<QTimer>         m_barCodeTimer;		        ///< 用于侦测BarCodeEdit变化
	QString							m_lastScanInputText;		///< 上一次解析的二维码信息
    ::im::tf::CaliDocGroup          m_caliGrp;                  ///< 校准组合
    InputType                       m_inputType;                ///< 输入模式

    QTimer*                         m_pScanTimer;               // 扫码定时器
    int                             m_iOverTimeCnt;             // 最大超时次数
};

