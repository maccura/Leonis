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

#pragma once

#include <QWidget>
#include <QTimer>
#include <QObject>
#include <qwt_plot_grid.h>
#include "boost/optional.hpp"
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include "src/thrift/ise/gen-cpp/ise_types.h"

class QwtPlotMarker;
class QwtPlotCurve;
class QwtPlotMagnifier;
class QwtPlotPanner;
class QwtSymbol;
class QLineEdit;

namespace Ui {
    class QAssayResultView;
};


class QReactionCurveData : public QObject
{
	Q_OBJECT
public:
	QReactionCurveData();
	~QReactionCurveData() {};
	struct DetectCurveData
	{
		QVector<double> detectPointers;
		QVector<double> absorbValues;
		QColor			color;
		QColor			markerColor;
		QString			curveName;
		QwtPlot			*qwtPlot;
	};

	double GetAvge(QVector<double>& resultVec);
	bool showDataDetail(double x, QString showMessage);
	void showMarker(double x, double y);
	QString GetCurrentWaveAbsorbs(double x);
	void showCurve(DetectCurveData& curveData);
	void reset();
private:
	std::shared_ptr<QwtPlotMarker>  m_plotMarker;
	std::shared_ptr<QwtPlotMarker>  m_plotInnerMarker;
	std::shared_ptr<QwtPlotCurve>	m_curves;
	std::shared_ptr<QwtPlotMarker>  m_plotMessageMarker;
	std::shared_ptr<QwtPlotMarker>  m_plotMessageMarkerText;
	QPixmap							m_markerPixmap;
	QPixmap							m_markerTextPixmap;
	DetectCurveData					m_curveData;
};

class CustomGrid : public QwtPlotGrid
{
public:
	CustomGrid() : QwtPlotGrid()
	{
		segment1Color = Qt::yellow;
		segment2Color = Qt::blue;
	}

	void setSegmentColors(const QColor& color1, const QColor& color2)
	{
		segment1Color = color1;
		segment2Color = color2;
	}

	void addSegment(double start, double end)
	{
		m_shiledMap.push_back(std::make_pair(start, end));
	}

	bool clearSegment()
	{
		m_shiledMap.clear();
		return true;
	}

    virtual void draw(QPainter *p, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRectF &canvasRect) const;

private:
	QColor segment1Color;
	QColor segment2Color;
	std::vector<std::pair<double, double>> m_shiledMap;
};

class QAssayResultView : public QWidget
{
    Q_OBJECT

public:
    QAssayResultView(QWidget *parent = Q_NULLPTR);
    ~QAssayResultView();

	struct DetectCurve
	{
		QVector<double> detectPointers;
		QVector<double> absorbValues;
		QColor color;
		QString curveName;
	};

    enum class DataMode
    {
        CHMODE,             ///< 生化
        LHIMODE,            ///< 血清指数
        ISEMODE             ///< ISE
    };

    ///
    /// @brief 设置项目结果id
    ///
    /// @param[in]  reuslt		当前显示的结果Id
	/// @param[in]  testItem    当前显示的项目信息
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月17日，新建函数
    ///
    void SetAssayResult(int64_t reuslt, tf::TestItem& testItem);

    ///
    /// @brief 设置当前显示的testItem
    ///
    /// @param[in]  testItem  当前item
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年6月16日，新建函数
    ///
    void SetCurrentTestItem(tf::TestItem& testItem);

	///
	/// @brief 获取当前显示的testItem（数据浏览）
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年8月14日，新建函数
	///
	const tf::TestItem& GetCurrentTestItem();

	///
	/// @brief 返回是否被设置为复查
	///
	///
	/// @return true:复查
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年9月20日，新建函数
	///
	const bool GetIsReset() { return m_isReset; };
	void ClearReset() { m_isReset = false; };

	///
	/// @brief 获取当前结果的测试完成时间
	///
	///
	/// @return 完成时间
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年9月25日，新建函数
	///
	QString GetTestTime();

    ///
    /// @brief 将当前页面复查结果设置为复查结果
    ///
    ///
    /// @return true:设置成功，false:设置失败
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年6月16日，新建函数
    ///
    bool SetCurrentResult();

	///
	/// @brief 比较时间是否过期
	///
	/// @param[in]  leftDate  左时间
	/// @param[in]  rightDate 右时间
	///
	/// @return leftDate > rightDate = true
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年3月22日，新建函数
	///
	bool IsExpired(const std::string& leftDate, const std::string& rightDate);

    ///
    /// @brief 设置样本信息
    ///
    /// @param[in]  sampleInfo		 样本信息
	/// @param[in]  isAllowedModify  是否允许修改结果信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年1月7日，新建函数
    ///
    void SetSampleInfo(tf::SampleInfo& sampleInfo, bool isAllowedModify = true);

    ///
    /// @brief 恢复原图
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年12月2日，新建函数
    ///
    void ResetCurve();

    ///
    /// @brief 显示ISE的结果
    ///
    /// @param[in]  iseResult  结果信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月30日，新建函数
    ///
    void SetIseAssayResult(ise::tf::AssayTestResult iseResult);

	///
	/// @brief 设置当前控件的显示字符串（当字符串长度大于控件宽度时，冒泡显示）
	///
	/// @param[in]  label			控件句柄
	/// @param[in]  strItemName		字符串
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年9月5日，新建函数
	///
	void SetItemName(QLineEdit *label, const QString& strItemName);

    ///
    /// @brief 初始化
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月17日，新建函数
    ///
    void Init();

    ///
    /// @brief 保存当前的结果修改
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月19日，新建函数
    ///
    void SaveAssayResultEdit();

Q_SIGNALS:
    void dataChanged();
	void resultChanged();

protected slots:
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

private:
    ///
    /// @brief 显示反应过程
    ///
    /// @param[in]  detectPointer  显示过程的点
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月17日，新建函数
    ///
    void ShowProcessCurve(DetectCurve& detectPointer);

    ///
    /// @brief 显示结果信息(生化)
    ///
    /// @param[in]  result  结果信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月17日，新建函数
    ///
    void DisplayResultInfo(ch::tf::AssayTestResult& result);

    ///
    /// @brief显示结果信息(血清指数)
    ///
    /// @param[in]  result  结果信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月30日，新建函数
    ///
    void DisplayLHIResultInfo(ch::tf::AssayTestResult& result);

    ///
    /// @brief 显示ISE的结果信息
    ///
    /// @param[in]  result  结果信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月30日，新建函数
    ///
    void DisplayISEResultInfo(ise::tf::AssayTestResult& result);

    ///
    /// @brief 事件处理函数
    ///
    /// @param[in]  obj   目标对象
    /// @param[in]  event 事件对象
    ///
    /// @return true表示事件已处理
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月17日，新建函数
    ///
    bool eventFilter(QObject *obj, QEvent *event) override;

	///
	/// @brief 显示当前坐标的值
	///
	/// @param[in]  xPointer  横坐标
	/// @param[in]  yPointer  纵坐标
	///
	/// @return 
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年1月27日，新建函数
	///
	void ShowTextDetail(double xPointer, double yPointer);

    ///
    /// @brief 页面显示时，处理事件
    ///
    /// @param[in]  event  事件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年4月24日，新建函数
    ///
    void showEvent(QShowEvent *event);

    ///
    /// @brief 清除界面
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月19日，新建函数
    ///
    void ClearView();

    ///
    /// @brief 显示曲线
    ///
    /// @param[in]  assayResult  结果
    ///
    /// @return true成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年12月6日，新建函数
    ///
    bool DisplayCurve(ch::tf::AssayTestResult& AssayResult);

	///
	/// @brief 显示分析方法信息
	///
	/// @param[in]  AssayResult  测试结果
	///
	/// @return true成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年6月6日，新建函数
	///
	bool DisplayAnalyPointInfo(const ch::tf::AssayTestResult& AssayResult, std::shared_ptr<CustomGrid>& spCurve);

private:
	bool							m_isReset;					///< 是否被设置为复查结果
	bool                            m_controlModify;            ///< control被选中
	bool                            m_shiftKey;					///< shiftKey被选中
	boost::optional<double>			m_startPostion;
    ch::tf::AssayTestResult         m_assayResult;
    Ui::QAssayResultView*           ui;
    QwtPlotMarker*                  m_plotMarker;
	std::shared_ptr<CustomGrid>		m_pSplitGrid;
	std::vector<std::shared_ptr<QReactionCurveData>> m_reactionDatas;
    std::vector<std::shared_ptr<QwtPlotCurve>> m_waveCurves;
    QwtPlotMagnifier*               m_plotMagnifier;
    QwtPlotPanner*                  m_plotPanner;
    tf::SampleInfo                  m_currentSample;
    tf::TestItem                    m_currentTestItem;
    DataMode                        m_mode;
	QString							m_modifyFlag;				///< 项目测试几个被修改过后的标志
    ise::tf::AssayTestResult        m_assayIseResult;           ///< ISE的结果
    QTimer                          m_timer;                    ///< 测试过程中定时更新数据
	bool							m_allowedEdit;				///< 是否允许修改
};
