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
/// @file     ScatterPlotClass.h
/// @brief    绘图控件封装类
///
/// @author   5774/WuHongTao
/// @date     2020年6月10日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2020年6月10日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once
#include <QWidget>
#include <QSize>
#include <QList>
#include "curveDraw/common.h"

class CScatterAsideItem;
class CScatterPlotContentClass;
class CalibrateCoordinate;
class CScatterPlotViewClass;
class QResizeEvent;
class CScatterPlotSenceClass;

namespace Ui {
    class QtGuiClass;
};


class CScatterPlotClass : public QWidget
{
    Q_OBJECT

public:
    ///
    /// @brief
    ///     绘图控件构造函数
    ///
    /// @param[in]  parent  
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年6月10日，新建函数
    ///
    CScatterPlotClass(QWidget *parent = Q_NULLPTR);

    ///
    /// @brief
    ///     绘图控件析构函数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年6月10日，新建函数
    ///
    ~CScatterPlotClass();

    ///
    /// @brief 添加曲线的函数
    ///     
    ///
    /// @param[in]  data  具体的曲线的点阵list
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月21日，新建函数
    ///
    int addCurveData(curveSingleDataStore& data);

    ///
    /// @brief 移除曲线
    ///     
    ///
    /// @param[in]  curveId  曲线Id来移除曲线
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月24日，新建函数
    ///
    void removeCurve(int curveId);

    ///
    /// @brief 设置曲线属性
    ///     
    ///
    /// @param[in]  scaleInfo  
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月24日，新建函数
    ///
    void setAttribute(CoordinateDataType scaleInfo);

    ///
    /// @brief
    ///     获取曲线的坐标系
    ///
    ///
    /// @return 曲线坐标系的属性
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月4日，新建函数
    ///
    CoordinateDataType& GetCurveAttrbute();

	///
	/// @brief
	///     清除所有曲线
	///
	/// @par History:
	/// @li 5774/WuHongTao，2020年5月27日，新建函数
	///
	void clearCurve();

    ///
    /// @brief
    ///     应用程序获取选中数列
    ///
    /// @param[in]  selectList  选中的数列
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年6月5日，新建函数
    ///
    void getSelectPointerList(QList<CurveDataInfo>& selectList);

	///
	/// @brief 设置为默认视图比例
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2022年9月9日，新建函数
	///
	void setDefaultViewScale();

signals:
	///
	/// @brief 关注点
	///     
	///
	/// @param[in]  focusData 点的信息 
	///
	/// @par History:
	/// @li 5774/WuHongTao，2020年5月19日，新建函数
	///
	void focusPointer(RealDataStruct focusData);

    ///
    /// @brief
    ///     通知使用者选中的点的list
    /// @par History:
    /// @li 5774/WuHongTao，2020年6月5日，新建函数
    ///
    void CurveSelectCome();

protected:
    ///
    /// @brief 重绘事件
    ///     
    ///
    /// @param[in]  event  
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月24日，新建函数
    ///
    virtual void resizeEvent(QResizeEvent *event) override;

    ///
    /// @brief 初始化参数
    ///     
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月24日，新建函数
    ///
    void initalPara();

private:
    Ui::QtGuiClass* ui;
    CScatterPlotViewClass*          m_view;                     //控件的view
	QSize							m_oldSize;                  //记录控件上次的大小，以便进行变换
    CScatterPlotSenceClass*         m_scene;                    //控件的sence
    CalibrateCoordinate*            m_coorDinate;               //坐标系的绘制
    CScatterPlotContentClass*       m_curveDraw;                //曲线的绘制
    CScatterAsideItem*              m_aSideItem;                //小控件的句柄

public slots:
	void                            test(RealDataStruct focusData);//test
};
