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
/// @file     qcgraphljwidget.h
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
#pragma once
#include <QWidget>
#include <QComboBox>
#include "src/thrift/gen-cpp/defs_types.h"
#include "uidcsadapter/adaptertypedef.h"
#include "mcqcljgraphic.h"
#include "PrintExportDefine.h"

namespace Ui { class QcGraphLJWidget; };

// 前置声明
class QStandardItemModel;                       // 标准数据模型
class QStandardItem;                            // 标准数据模型单元项
class StatusItemDelegate;                       // 状态代理
class QcOutCtrlDlg;                             // 质控失控处理对话框
class QButtonGroup;
class QTableView;

///
/// @bref
///		图表选择下拉控件
///
class GraphComboBox : public QComboBox
{
	Q_OBJECT

public:
	GraphComboBox(QWidget *parent = Q_NULLPTR);
	~GraphComboBox();

	void paintEvent(QPaintEvent *e);                // 绘制事件
};

class QcGraphLJWidget : public QWidget
{
    Q_OBJECT

public:
    QcGraphLJWidget(QWidget *parent = Q_NULLPTR);
    ~QcGraphLJWidget();

protected:
    ///
    /// @brief
    ///     界面显示前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年9月8日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     显示之后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年9月8日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief
    ///     初始化字符串资源
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年1月17日，新建函数
    ///
    void InitStrResource();

    ///
    /// @brief
    ///     初始化子控件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年9月8日，新建函数
    ///
    void InitChildCtrl();

    ///
    /// @brief
    ///     初始化连接
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年9月8日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief
    ///     初始化打印表
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年7月7日，新建函数
    ///
    void InitPrintTbl();

    ///
    /// @brief
    ///     窗口显示事件
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年9月8日，新建函数
    ///
    void showEvent(QShowEvent *event) override;

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
    bool LoadQcOutCtrlInfo(QcOutCtrlDlg* pQcOutCtrlDlg);

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
    bool SaveQcOutCtrlInfo(QcOutCtrlDlg* pQcOutCtrlDlg);

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
    bool GetLjPointsFromQcResultByDay(QDate& startDate,
                                      QMap<int, QVector<QPointF>> &mapCurvePoints,
                                      QMap<int, QVector<long long>>* pMapCurvePointsID = Q_NULLPTR,
                                      QMap<int, QVector<QwtSymbol::Style>>* pMapCurvePointsSymbol = Q_NULLPTR);

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
    bool GetLjPointsFromQcResultByCount(QMap<int, QVector<QPointF>> &mapCurvePoints,
                                        QMap<int, QVector<long long>>* pMapCurvePointsID = Q_NULLPTR,
                                        QMap<int, QVector<QwtSymbol::Style>>* pMapCurvePointsSymbol = Q_NULLPTR);

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
    bool SetLjPointsToQcGraphic(const QMap<int, QVector<QPointF>> &mapCurvePoints,
                                QMap<int, QVector<long long>>* pMapCurvePointsID = Q_NULLPTR,
                                QMap<int, QVector<QwtSymbol::Style>>* pMapCurvePointsSymbol = Q_NULLPTR);

    ///
    /// @brief
    ///     更新质控图选中点
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年7月4日，新建函数
    ///
    bool UpdateQcGraphicSelPt();

    ///
    /// @brief
    ///     更新打印表
    ///
    /// @param[in]  stuQcInfo  打印样本
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年7月7日，新建函数
    ///
//     void UpdatePrintTbl(const UI::UI_QC_INFO& stuQcInfo);
// 
//     ///
//     /// @brief
//     ///     更新打印模板变量标签
//     ///
//     /// @param[in]  stuQcInfo  打印样本
//     ///
//     /// @par History:
//     /// @li 4170/TangChuXian，2022年7月7日，新建函数
//     ///
//     void UpdatePrintVarStr(const UI::UI_QC_INFO& stuQcInfo);

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
    QwtSymbol::Style ContructSymbolStyleByOutCtrlState(QC_STATE enType);

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
	void UpdateSubDevReadioBtnGroup(const QStringList& subDevList);

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
	QString GetCurrentSelectedDevName();

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
	void ResizeQcResultTableIconColumn(QTableView* pTable);

    ///
    /// @brief
    ///     获取待打印的质控L-J信息
    ///
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年12月18日，新建函数
    ///
    bool GetQCLJInfo(QCLJInfo& Info);

    ///
    /// @brief
    ///     获取待打印的质控L-J信息
    ///
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年12月18日，新建函数
    ///
    bool GetQCDailyInfo(QCDailyInfo& Info, QDate QcDate);

protected Q_SLOTS:
    ///
    /// @brief
    ///     重绘控件视图
    ///
    /// @par History:
    /// @li 5220/SunChangYan，2021年11月5日，新建函数
    ///
    //void RepaintCtrlViews();

    ///
    /// @brief
    ///     更新设备列表
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月19日，新建函数
    ///
    void UpdateDevNameList();

    ///
    /// @brief
    ///     更新质控信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年9月9日，新建函数
    ///
    void UpdateQcInfo();

    ///
    /// @brief
    ///     切换项目导致更新质控信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年11月21日，新建函数
    ///
    void UpdateQcInfoByAssayChanged();

    ///
    /// @brief
    ///     更新项目名列表
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年1月3日，新建函数
    ///
    void UpdateAssayNameList();

    ///
    /// @brief
    ///     更新质控结果
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年9月9日，新建函数
    ///
    void UpdateQcResult();

    ///
    /// @brief
    ///     更新质控曲线图
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年9月24日，新建函数
    ///
    void UpdateQcGraphic();

    ///
    /// @brief
    ///     更新所有质控信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年10月25日，新建函数
    ///
    void UpdateAllQcInfo();

    ///
    /// @brief
    ///     质控结果更新
    ///
    /// @param[in]  uAssayCode  项目编号
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年10月27日，新建函数
    ///
    void OnQcResultUpdate(unsigned int uAssayCode);

    ///
    /// @brief
    ///     质控条件改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年9月10日，新建函数
    ///
    void OnQcCondChanged();

    ///
    /// @brief
    ///     质控图类型改变（日内质控和日间质控切换）
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年9月24日，新建函数
    ///
    void OnQcGraphicTypeChanged();

    ///
    /// @brief
    ///     质控信息表中图表下拉框改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年9月10日，新建函数
    ///
    void OnGraphicComboChanged();

    ///
    /// @brief
    ///     展开按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年4月1日，新建函数
    ///
    void OnPopBtnClicked();

    ///
    /// @brief
    ///     细节按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年9月10日，新建函数
    ///
    void OnDetailBtnClicked();

    ///
    /// @brief
    ///     失控处理按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年9月13日，新建函数
    ///
    void OnOutCtrlBtnClicked();

    ///
    /// @brief
    ///     更新靶值按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年9月16日，新建函数
    ///
    void OnTargetValUpdateBtnClicked();

    ///
    /// @brief
    ///     打印按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年7月29日，新建函数
    ///
    void OnPrintBtnClicked();

    ///
    /// @brief
    ///     导出按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年5月11日，新建函数
    ///
    void OnExportBtnClicked();

	///
	/// @brief 质控信息表被选中
	///
	/// @param[in]  curIndex    当前索引
	/// @param[in]  prevIndex   之前选中的所有
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年9月20日，新建函数
	///
	void OnQcInfoTableItemSelChanged(const QModelIndex& curIndex, const QModelIndex& prevIndex);

    ///
    /// @brief
    ///     质控结果表单元格被点击
    ///
    /// @param[in]  clickIndex   被点击的索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年9月16日，新建函数
    ///
    void OnQcResultTblItemClicked(const QModelIndex& clickIndex);

    ///
    /// @brief
    ///     质控结果表选中项改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年11月22日，新建函数
    ///
    void OnQcResultItemSelChanged();

    ///
    /// @brief
    ///     质控图
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年12月15日，新建函数
    ///
    void OnQcGraphicShowPointCBClicked();

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
    void OnQcGraphicSelPtChanged(McQcLjGraphic::CURVE_ID enCurveID, long long iPtID);

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
    void OnReqPrintTmpVarData(int iVarID, int iSeries, int iItemOffset);

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
    void OnReqPrintTmpVarTblHeaderData(int iVarID, int iSeries, int iItemOffset);

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
    void OnReqPrintTmpVarTblItemData(int iVarID, int iSeries, int iRow, int iCol, int iItemOffset);

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
    void OnReqPrintTmpVarTblRowColCnt(int iVarID, int iSeries, int iItemOffset);

    ///
    /// @brief
    ///     请求获取打印模板系列号打印份数
    ///
    /// @param[in]  iSeries      系列号
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年6月30日，新建函数
    ///
    void OnReqPrintTmpSeriesPrintCnt(int iSeries);

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
    void OnReqPrintTmpVarImg(int iVarID, int iSeries, int iItemOffset);

    ///
    /// @brief
    ///     Tab角落按钮索引改变
    ///
    /// @param[in]  iBtnIdx  按钮索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月15日，新建函数
    ///
    void OnTabCornerIndexChanged(int iBtnIdx);

    ///
    /// @brief
    ///     质控品信息更新
    ///
    /// @param[in]  vQcDoc  质控品信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年1月4日，新建函数
    ///
    void OnQcDocInfoInfoUpdate(std::vector<tf::QcDocUpdate, std::allocator<tf::QcDocUpdate>> vQcDoc);

    ///
    /// @brief
    ///     质控结果更新
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月14日，新建函数
    ///
    void OnQcRltUpdate(QString devSn, int64_t qcDocId, QVector<int64_t> changedId);

    ///
    /// @bref
    ///		响应权限变化
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月18日，新建函数
    ///
    void OnPermisionChanged();

    ///
    /// @brief
    ///     更新靶值SD按钮使能状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月25日，新建函数
    ///
    void UpdateTargetValSDBtnEnable();

    ///
    /// @brief
    ///     项目信息更新
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年12月21日，新建函数
    ///
    void OnAssayInfoUpdate();

    ///
    /// @brief
    ///     更新质控信息下拉框
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年3月26日，新建函数
    ///
    void UpdateQcInfoCombo();

    ///
    /// @brief
    ///     清空质控信息下拉框
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年3月26日，新建函数
    ///
    void ClearQcInfoCombo();

    ///
    /// @brief
    ///     质控信息表格显示范围改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年3月26日，新建函数
    ///
    void OnQcInfoTblDisplayRangeChanged();

private:
    Ui::QcGraphLJWidget           *ui;                            // UI对象
    bool                          m_bInit;                        // 是否已经初始化

    // 质控结果
    QStandardItemModel*           m_pTbl1Model;                   // 图表1模型
    QStandardItemModel*           m_pTbl2Model;                   // 图表2模型
    QStandardItemModel*           m_pTbl3Model;                   // 图表3模型
    QStandardItemModel*           m_pTbl4Model;                   // 图表4模型

    // 打印数据提供
    QStandardItemModel*           m_pPrintTblQcLjResult;          // 质控LJ结果表
    QMap<int, QString>            m_mapPrintTmpVarStr;            // 打印模板变量映射

    // 设备列表
    QStringList                   m_strDevNameList;			      // 当前设备名称列表（如果有组名则记录的是组名）
	QString                       m_strCurDevGroupName;           // 当前设备组名
    QString                       m_strCurDevName;				  // 当前设备名

	// 界面子设备
	QButtonGroup*				  m_pSubDevReadioBtnGroup;		   // 界面子设备按钮组
	QString						  m_strCurSubDevReadioName;		   // 当前选中的子设备按钮名称

    // 下拉框缓存
    QMap<int, QComboBox*>         m_mapGraphicComboBuffer;         // 质控图下拉框映射(用于提升插入效率)
    bool                          m_bNeedUpdateCombo;              // 是否需要刷新下拉框显示
};
