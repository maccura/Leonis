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
#pragma once
#include <tuple>
#include <QVector>
#include <QPoint>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include "shared/basedlg.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include "src/thrift/cloud/ch/gen-cpp/ch_cloud_defs_types.h"

class QStandardItemModel;
class QStandardItem;
class QPostionEdit;
class QCaliRecordFilterDataModule;
class PositiveNumDelegate;

namespace Ui {
    class CaliBrateRecordDialog;
};

//对话框的模式
enum SHOWMODE
{
    NEW_MODE = 0,
	MODIFY_MODE,
    MODE_END
};

//录入方式
enum InputType
{
	Input_Manual = 1,   //手动输入
	Input_Scan,			//扫描输入
	Input_Down,			//下载输入
	Input_Modify		//修改输入
};


//校准品类型
/*enum CaliMeterialType {
	CAIL_METERIAL_TYPE_CHER = 0,		// 校准品类型->生化校准品(CAIL_METERIAL_TYPE_CHER)
	CAIL_METERIAL_TYPE_ISE_SERUM = 1,	// 校准品类型->ISE血清校准品(CAIL_METERIAL_TYPE_ISE_SERUM)
	CAIL_METERIAL_TYPE_ISE_URINE		// 校准品类型->ISE尿液校准品(CAIL_METERIAL_TYPE_ISE_URINE)
};*/

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

// 下载登记缓存数据结构体
struct CaliDocSaveStru
{
	ch::tf::CaliDocGroup			docGroup;	// 校准文档组
	std::vector < ch::tf::CaliDoc>	docVec;		// 校准文档列表
};

class CaliBrateRecordDialog :
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
    CaliBrateRecordDialog(QWidget *parent = Q_NULLPTR);

    ///
    /// @brief 析构函数
    ///     
    /// @par History:
    /// @li 5774/WuHongTao，2020年5月9日，新建函数
    ///
    ~CaliBrateRecordDialog();

    ///
    /// @brief 对话框的展示模式
    ///     
    ///
    /// @param[in]  mode  展示模式
    ///
    /// @par History:
    /// @li 8580/GongZhiQiang，2023年7月6日，新建函数
    ///
    void setMode(const SHOWMODE& mode);

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
	void detailShow(QList<ch::tf::CaliDoc>&, ch::tf::CaliDocGroup& grouPinfo);

	///
	/// @brief
	///     显示新增校准品组界面（包括手动输入和扫描输入）
	///
	/// @param[in]  &  InputType 新增输入校准品的方式
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2021年8月24日，新建函数
	///
	void setInputMode(const InputType&);

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
    void getData(QList<ch::tf::CaliDoc>& dataDoc, SHOWMODE& modetype);

    ///
    /// @brief
    ///     重置对话框状态
    /// @par History:
    /// @li 5774/WuHongTao，2020年10月16日，新建函数
    ///
    void reset();

protected:
    // 重写隐藏事件
    virtual void hideEvent(QHideEvent *event) override;

signals:
    ///
    /// @briefm 对话框关闭的时候，发出的信号
    ///     
    /// @par History:
    /// @li 5774/WuHongTao，2020年5月9日，新建函数
    ///
    void closeWindow(int64_t);

	///
	/// @briefm 生成一个不重复的序列号
	///
	/// @param[in]  lot  当前编辑批号
	/// @param[in]  sn  生成的序列号
	///     
	/// @par History:
	/// @li 8090/YeHuaning，2022年8月21日，新建函数
	///
	void getNewSn(const std::string& lot, std::string& sn);

	///
	/// @brief 查询结果信号
	///
	/// @param[in]  isSuccess  是否成功
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年4月8日，新建函数
	///
	void searchResult(bool isSuccess);

private slots:
    ///
    /// @brief 添加新样品
    ///     
    /// @par History:
    /// @li 5774/WuHongTao，2020年5月9日，新建函数
    ///
    void addNewCaliIntoDatabase();

    ///
    /// @brief
    ///     更改日期
    ///
    /// @param[in]  date  日期参数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年4月19日，新建函数
    ///
    void dateModified(const QDate &date);

    ///
    /// @brief
    ///     数据信息有更新
    ///
    /// @param[in]  item  更新数据的新控件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年6月28日，新建函数
    ///
    void updateDataList(QStandardItem* item);

    ///
    /// @brief
    ///     改变校准品位置
    ///
    /// @param[in]  index  索引
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月11日，新建函数
    ///
    void OnChangeCalibratePostion(QModelIndex index);

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
    void OnRefreshPostion();

    ///
    /// @brief 校准品组类型变更时，表格内容需要变化
    ///
    /// @param[in]  index  变化后的索引
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月16日，新建函数
    ///
    void OnChangeCalibrateType(int index);

	///
	/// @brief 扫描输入框内容改变槽函数
	///
	/// @param[in]  barCode  输入内容
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
	///
	void OnBarCodeEditChanged(const QString& barCode);

	///
	/// @brief 扫描间隙时间节点到达
	///
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
	///
	void OnBarCodeTimerEvent();

	///
	/// @brief 扫描输入完成槽函数
	///
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
	///
    void OnBarFinished();

    ///
    /// @brief 项目配置发生变化
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年6月9日，新建函数
    ///
    void OnAssayUpdated();

	///
	/// @brief 下载登记查询按钮点击
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月15日，新建函数
	///
	void OnQueryBtnClicked();

	///
	/// @brief 下载登记重置按钮点击
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月15日，新建函数
	///
	void OnResetBtnClicked();

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
	void OnCalibrateDownSelectedChanged(const QModelIndex& modelIndex);

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
	void OnSearchHander(bool isSuccess);

private:
    ///
    /// @brief
    ///     重置（查看/输入）对话框的table表头
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月10日，新建函数
    ///
    void ResetRecordAssayTable();

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
    bool IsCompostionEqual(QList<ch::tf::CaliDoc>& caliDocs, int cgType);

    ///
    /// @brief 对话框的初始化操作
    ///     
    /// @par History:
    /// @li 5774/WuHongTao，2020年5月9日，新建函数
    ///
    void Init();

    ///
    /// @brief 更新对话框的显示列表
    ///     
    /// @param[in]  dataShow  需要显示的参数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年5月9日，新建函数
    ///
    void updateMainCalibrateView(QList<ch::tf::CaliDoc>& dataShow);

    ///
    /// @brief
    ///     存储数据到数据库
    ///
	/// @param[in]  data  需要存储的数据
	/// @param[in]  caliGroup  当前校准品组
    ///
    /// @return 保存成功,则返回true，错误则返回false
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年6月29日，新建函数
    ///
	bool saveCaliData(ch::tf::CaliDoc& data, const ch::tf::CaliDocGroup& caliGroup);
	
	///
	/// @brief
	///     删除数据
	///
	/// @param[in]  data  需要删除的数据
	/// @param[in]  caliGroup  当期校准品组
	///
	/// @return 删除成功,则返回true，错误则返回false
	///
	/// @par History:
	/// @li 8090/YeHuaning，2022年8月26日，新建函数
	///
	bool deleteCaliData(const ch::tf::CaliDoc& data, const ch::tf::CaliDocGroup& caliGroup);

    ///
    /// @brief
    ///     获取项目信息的List
    ///
    /// @param[in]  assayList  项目信息list
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年6月28日，新建函数
    ///
	void GetAssayInfoList(QVector<std::tuple<std::string, int, QString>>& assayList, int type);

	///
	/// @brief
	///     设置试剂校准屏蔽列表
	///
	/// @param[in]  item  变化的项目
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2022年8月25日，新建函数
	///
	void setDisableAssayCode(const QStandardItem* item);

	///
	/// @brief
	///     获取当前行是否有设置浓度信息
	///
	/// @param[in]  item  变化的项目
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2022年8月25日，新建函数
	///
	bool hasContentInRow(const QStandardItem* item) const;

    ///
    /// @brief 设置表格的读写方式
    ///
    /// @param[in]  type  校准品类型
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月20日，新建函数
    ///
    void SetTableViewDelegate(int type);

    ///
    /// @brief 当前校准品类型切换后是否需要做数据处理
    ///
    /// @return true:数据需要处理  false:不需要处理
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月20日，新建函数
    ///
    bool IsValidCaliTypeChanged();

    ///
    /// @brief 使用系统水初始化校准文档的校准品1
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年4月20日，新建函数
    ///
    void CreateSystemWaterDocs();

    ///
    /// @brief 创建一个临时校准文档
    ///
    /// @param[in]  level  水平
    ///
    /// @return 临时文档
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年4月24日，新建函数
    ///
    ch::tf::CaliDoc CreateTempCaliDoc(int level);

	///
	/// @brief 手动输入模式
	///
	///
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
	///
	void SetManualInputMode();

	///
	/// @brief 扫描输入模式
	///
	///
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
	///
	void SetScanInputMode();

	///
	/// @brief 下载输入模式
	///
	///
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
	///
	void SetDownInputMode();

	///
	/// @brief 修改输入模式
	///
	///
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
	///
	void SetModifyInputMode();

	///
	/// @brief 初始化扫描相关信号
	///
	///
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
	///
	void InitScanInputSignal();

	///
	/// @brief 解析进入接口
	///
	/// @param[in]  barCode 扫描输入框中的数据
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
	///
	void PaserScanQRCodeInterface(const QString& qrCode);

	///
	/// @brief 解析扫描框输入内容并更新到界面
	///
	/// @param[in]  barCode 扫描输入框中的数据
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
	///
	void PaserScanQRCodeToUi(const QString& qrCode);

	///
	/// @brief 更新扫描内容到界面
	///
	/// @param[in]  QRcodeInfo 二维码信息
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
	///
	bool UpdateScanQRInfoToUI(const ::ch::tf::CaliGroupQRcode& QRcodeInfo);

	///
	/// @brief 退出之前清理
	///
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
	///
	void CleanBeforeQuit();

	///
	/// @brief 设置界面扫描状态提示信息
	///
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
	///
	void SetScanStatusNoticeInfo(bool isSuccess, QString noticeInfo);

	///
	/// @brief 扫描流程控制初始化
	///
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
	///
    void ScanControlInit();

	///
	/// @brief 初始化扫描UI
	///
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年12月18日，新建函数
	///
	void InitScanUI();

    ///
    /// @brief 按照第一列进行升序排序
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月30日，新建函数
    ///
    void SetAscSortByFirstColumn();

	///
	/// @brief 更新下载登记表格
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月20日，新建函数
	///
	void UpdateCalibrateDownData();

	///
	/// @brief 保存下载登记数据
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月20日，新建函数
	///
	void SaveCalibrateDownData();


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
	bool AddCalibrateInfoToDb(const std::vector<CaliDocSaveStru>& saveDataCache);

    ///
    /// @brief 获取修改校准品的操作日志
    ///
    /// @param[in]  oriGroup  修改前的校准品组数据
    /// @param[in]  newGroup  修改后的校准品组数据
    /// @param[in]  oriDocs   修改前的校准文档数据
    /// @param[in]  newDocs   修改后的校准文档数据
    ///
    /// @return 操作日志
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年6月26日，新建函数
    ///
    std::string GetModifyCaliInfoLog(const ch::tf::CaliDocGroup& oriGroup, const ch::tf::CaliDocGroup& newGroup,
        const QList<ch::tf::CaliDoc>& oriDocs, const QList<ch::tf::CaliDoc>& newDocs);

    ///
    /// @brief 添加操作日志
    ///
    /// @param[in]  info  日志内容
    /// @param[in]  type  操作类型
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年6月27日，新建函数
    ///
    void AddOperateLog(const std::string& info, int type);

private:
    Ui::CaliBrateRecordDialog*      ui;
    QList<ch::tf::CaliDoc>          m_addCaliDocs;              ///< 列表中的校准品list
    QList<ch::tf::CaliDoc>          m_saveCaliDocs;             ///< 暂存列表
    QString                         m_calibrationSampleBarcode; ///< 校准样品条码
    QStandardItemModel*             m_CalibrateConcMode;        ///< main的mode
	QStandardItemModel*             m_CalibrateDownMode;        ///< 下载登记的mode
    QPoint                          m_globalPos;                ///< 鼠标的世界坐标
    QPoint                          m_windowsPos;               ///< 窗口的世界坐标
    bool                            m_press;                    ///< 鼠标是否按下
    bool                            m_dataChanged;              ///< 数据被改动
    QVector<std::tuple<std::string, int, QString>>     m_AssayItemList;  ///< 测试项目信息(项目名称，设备类型，主单位)
    SHOWMODE                        m_modeType;                 ///< 对话框的模式
	InputType						m_inputType;				///< 输入模式
    int64_t                         m_groupDb;                  ///< 组的db信息
    ch::tf::CaliDocGroup            m_caligroup;                ///< 校准品组信息
	QPostionEdit*					m_CaliPostionEditDialog;    ///< 校准位置编辑对话框
	std::set<int>					m_disableAssayCodes;		///< 禁用校准的试剂列表
	QList<ch::tf::CaliDoc>			m_detailDocs;				///< 修改界面的输入校准文档组
    QStandardItemModel*             m_iseCaliConcModel;         ///< Ise校准品组信息
    int                             m_lastCaliType;             ///< 上一次的校准品类型

	std::shared_ptr<QTimer>         m_barCodeTimer;		        ///< 用于侦测BarCodeEdit变化
	QString							m_lastScanInputText;		///< 上一次解析的二维码信息
	::ch::tf::CaliGroupQRcode		m_lastScanInfo;				///< 上一次解析的二维码信息
	QVector<::ch::tf::CaliGroupQRcode>		m_QRcodes;		    ///< 暂存全部二维码的信息

	std::vector<adapter::chCloudDef::tf::CloudCaliDocGroup>		m_downloadDataCache;	///< 下载登记数据缓存
	bool							m_isRequesting;				///< 是否正在请求下载
    PositiveNumDelegate*            m_editDelegate;             ///< 浓度编辑代理
};
