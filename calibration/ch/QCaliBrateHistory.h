﻿/***************************************************************************
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
/// @file     QCaliBrateHistory.h
/// @brief    校准历史界面
///
/// @author   4170/TangChuXian
/// @date     2022年9月5日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月5日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <QWidget>
#include <QSortFilterProxyModel>
#include <map>
#include "src/alg/ch/ch_alg/ch_alg.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"

#define MAXHISTORY 50

class CalHisCurve;
class QwtPlotCurve;
class QCurveModeFilterDataModule;
class QcGraphicSelTipMaker;             // 选中效果标记
class QStandardItemModel;
class QSerialModel;
class QLabel;
class QScrollBar;
class CaliHistoryInfoCh;

namespace Ui { class QCaliBrateHistory; };

class QCaliBrateHistory : public QWidget
{
    Q_OBJECT

public:
    QCaliBrateHistory(QWidget *parent = Q_NULLPTR);
    ~QCaliBrateHistory();

    struct QLegendAttr
    {
        bool isDetach;
        QString legendTitle;
        QColor legendColor;
        QwtPlotCurve *curve;
        QLegendAttr()
            :legendTitle(""), legendColor(QColor()), curve(nullptr) {}
        QLegendAttr(const QString& title, const QColor& color)
            :legendTitle(title), legendColor(color), curve(nullptr) {}
    };

    ///
    /// @brief刷新设备试剂列表
    ///     
    ///
    /// @param[in]  devices  设备列表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月22日，新建函数
    ///
    QWidget* RefreshPage(std::vector<std::shared_ptr<const tf::DeviceInfo>>& devices);

signals:
    void sigSetAllChecked();

public slots:
    ///
    /// @brief
    ///     重新刷新界面
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月14日，新建函数
    ///
    void OnRefreshHistory();

    ///
    /// @brief 显示当前图形选中索引的校准数据
    ///
    /// @param[in]  index  图形上的索引
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月12日，新建函数
    ///
    void OnCurrentSelCalHisInfoChanged(int index);

    ///
    /// @brief 当选中了某一个点，弹出提示框
    ///
    /// @param[in]  pos  选中点的位置
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年7月18日，新建函数
    ///
    void OnSelectPos(const QPoint& pos);

    ///
    /// @brief 打印/导出
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年3月4日，新建函数
    ///
    void OnExport();
    void OnPrint();

private:
    ///
    /// @brief
    ///     清空曲线内容
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月22日，新建函数
    ///
    void ClearCurveContent();

    ///
    /// @brief
    ///     更新校准曲线表格数据
    ///
    /// @param[in]  curveInfos  校准曲线信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月22日，新建函数
    ///
    void UpdateCurveView();

    ///
    /// @brief 初始化校准量程表格
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年9月16日，新建函数
    ///
    void InitCaliDataModel();

    ///
    /// @brief
    ///     设置显示曲线的详细信息
    ///
    /// @param[in]  curveInfo  曲线信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月22日，新建函数
    ///
    void SetCurveDetail(const ch::tf::CaliCurve& curveInfo);

    ///
    /// @brief 设置控件数据
    ///
    /// @param[in]  lb  需要设置的控件
    /// @param[in]  text  数据内容
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年9月17日，新建函数
    ///
    void SetLableData(QLabel* lb, const QString& text);

    ///
    /// @brief 设置吸光度浓度表
    ///
    /// @param[in]  curveInfo  曲线信息
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年9月17日，新建函数
    ///
    void SetCaliDataModel(const ch::tf::CaliCurve& curveInfo);

    ///
    /// @brief 设置X轴底部的信息
    ///
    /// @param[in]  value  当前显示的值
    ///
    /// @return 需要转换的值
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年3月27日，新建函数
    ///
    QString SetCustomAxisInfo_X_Bottom(double value);

    ///
    /// @bref
    ///		重新加载浓度表格
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月30日，新建函数
    ///
    void RefreshTableData(int dtIdx);

    ///
    /// @brief 设置打印导出是否可用
    ///
    /// @param[in]  hasCurve  是否存在可用曲线
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年12月22日，新建函数
    ///
    void SetPrintAndExportEnabled(bool enabled); 
    
    ///
    /// @brief 获取导出数据
    ///
    /// @param[in]  strExportTextList  
    ///
    /// @return true 标识成功
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年3月5日，新建函数
    ///
    bool ExportAssayInfo(QStringList& strExportTextList);
    bool ExportCaliInfo(QStringList& strExportTextList);

    ///
    /// @brief 获取打印数据
    ///
    /// @param[in]  info  校准记录
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年3月12日，新建函数
    ///
    bool GetPrintExportInfo(CaliHistoryInfoCh& info);

private slots:
    ///
    /// @brief
    ///     选中曲线--显示曲线
    ///
    /// @param[in]  index  曲线Index
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月22日，新建函数
    ///
    void OnSelectCurveShow(QModelIndex index);

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
    /// @bref
    ///		权限变化响应
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月30日，新建函数
    ///
    void OnPermisionChanged();

protected:
    // 重写隐藏事件
    virtual void hideEvent(QHideEvent *event) override;

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
    /// @brief
    ///     显示前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月4日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     显示之后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月4日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief
    ///     重写显示事件
    ///
    /// @param[in]  event  事件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月4日，新建函数
    ///
    virtual void showEvent(QShowEvent* event) override;

    ///
    /// @brief
    ///     初始化子控件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月17日，新建函数
    ///
    void InitChildCtrl();

    ///
    /// @brief
    ///     初始化连接
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月17日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief 创建QwtCurve显示数据
    ///
    /// @param[in]  caliCurves  校准数据
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月12日，新建函数
    ///
    void CreateQwtPlotCurveData(const std::vector<ch::tf::CaliCurve>& caliCurves);

    ///
    /// @brief 创建线条基础数据
    ///
    /// @param[in]  xAxis  x坐标轴位置
    /// @param[in]  yAxis  y坐标轴位置
    /// @param[in]  brushColor  线条颜色
    ///
    /// @return 曲线对象
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月12日，新建函数
    ///
    QwtPlotCurve * CreateInitQwtPlotCurve(int xAxis, int yAxis, const QColor& brushColor);

    ///
    /// @brief 设置图例相关控件和处理函数
    ///
    /// @param[in]  legendAttrs  图例数据
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月12日，新建函数
    ///
    void SetCurveLegend(const QVector<QLegendAttr>& legendAttrs);

    ///
    /// @brief 获取图例数据
    ///
    ///
    /// @return 图例对象集合
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月12日，新建函数
    ///
    QVector<QLegendAttr>& GetLegendAttrs();

    ///
    /// @brief 获取执行id的项目测试结果
    ///
    /// @param[in]  dbId  数据库主键id
    ///
    /// @return 项目测试结果对象
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年6月21日，新建函数
    ///
    boost::optional<ch::tf::AssayTestResult> GetAssayTestResult(int64_t dbId);

    ///
    /// @brief 创建X轴自定义坐标并设置选中X轴的索引
    ///
    /// @param[in]  selIndex  
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年7月18日，新建函数
    ///
    void CreateScaleDraw_X(int selIndex = -1);

    ///
    /// @brief 显示选中点的高亮
    ///
    /// @param[in]  isShow  是否显示高亮
    /// @param[in]  level  校准品水平
    /// @param[in]  curveIndex  校准历史中的第几个数据
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年7月18日，新建函数
    ///
    void OnShowQwtCurveTip(bool isShow, int level, int curveIndex, const QPoint & pos = QPoint(-1, -1));

private:
    Ui::QCaliBrateHistory*          ui;
	QStandardItemModel*             m_CurveMode;                // 曲线数据的模式
	QCurveModeFilterDataModule*     m_CurveModeProxyModel;      // 曲线数据的模式代理
	QSerialModel*					m_caliDataMode;             // 校准数据模型数据
	std::vector<std::string>        m_strDevs;                  // 对应设备列表
	std::vector<ch::tf::CaliCurve>	m_curCaliCurves;			// 当前项目的校准曲线合集
	CalHisCurve*                    m_pCalHisCurve;             // 校准历史控件
    QScrollBar*                     m_pScrollBar;               // 滚动条

    // 选中效果
    QStringList                     m_strSelPtTextList;         // 选中点文本列表
    QcGraphicSelTipMaker*           m_pCaliSelTipMarker;        // 选中点悬浮框标志
    QwtPlotCurve*                   m_pCaliSelRing;             // 图选中效果（圆环）
	bool                            m_bInit;                    // 是否初始化

    int                             m_currentIndex;             // 当前选中的序号
};

///////////////////////////////////////////////////////////////////////////
/// @file     QCaliBrateHistory.h
/// @brief 	 排序代理
///
/// @author   7656/zhang.changjiang
/// @date      2023年4月11日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年4月11日，新建文件
///
///////////////////////////////////////////////////////////////////////////
class QCurveModeFilterDataModule : public QSortFilterProxyModel
{
public:
	QCurveModeFilterDataModule(QObject* parent = nullptr);
	~QCurveModeFilterDataModule();
	virtual bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
};