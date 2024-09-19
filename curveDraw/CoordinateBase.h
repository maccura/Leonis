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
/// @file     ScatterPlotCoordinatesClass.h
/// @brief    坐标系的绘制(X,Y轴)
///
/// @author   5774/WuHongTao
/// @date     2020年4月24日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2020年4月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once
#include <QGraphicsObject>
#include "common.h"
#define STEP 0.05

class QGraphicsSceneMouseEvent;

class CoordinateBase :
    public QGraphicsObject
{
    Q_OBJECT


public:
    ///
    /// @brief 构造函数
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月13日，新建函数
    ///
    CoordinateBase(int xaAisLength, int yaAisLength);

    ///
    /// @brief 析构函数
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月13日，新建函数
    ///
    ~CoordinateBase();

    ///
    /// @brief 返回图形的绘制区域
    /// @return 绘制区域的大小
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月17日，新建函数
    ///
    virtual QRectF boundingRect() const;

    ///
    /// @brief 添加曲线
    ///     
    /// @param[in]  data  曲线的数据
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月17日，新建函数
    ///   
    void addCurveData(int dataId, curveSingleDataStore& data);

    ///
    /// @brief 设置坐标系的属性，比如隔离带的大小，宽度等等
    ///     
    /// @param[in]  scaleInfo  
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月17日，新建函数
    ///    
    void setAttribute(CoordinateDataType scaleInfo);

    ///
    /// @brief 获取坐标系的刻度信息等等
    ///     
    /// @return 刻度信息的数据结构
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月24日，新建函数
    ///   
    CoordinateDataType getCoordinateInformation();

    ///
    /// @brief 删除曲线
    ///     
    /// @param[in]  curveId  对应的曲线ID
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月24日，新建函数
    ///  
    void removeCurve(int curveId);

    ///
    /// @brief 获取X轴坐标轴的刻度信息
    ///     
    /// @return x轴坐标轴的刻度信息结构体
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月24日，新建函数
    ///
    QMap<qreal, DataPosAndValue> getXcoorDinatePostion();

    ///
    /// @brief 获取y轴坐标轴的刻度信息
    ///     
    /// @return y轴坐标轴的刻度信息结构体
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月24日，新建函数
    QMap<qreal, DataPosAndValue> getYcoorDinatePostion();

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
    ///     获取坐标系范围
    ///
    /// @return 分别代表上下左右
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年11月24日，新建函数
    ///
	CoordinateSize GetCoordinatesize();

	///
	/// @brief 设置为默认视图比例
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2022年9月9日，新建函数
	///
	void setDefaultViewScale();
signals:
    ///
    /// @brief
    ///     更新曲线
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年7月14日，新建函数
    ///
    void updateCurveData();
protected:
    ///
    /// @brief 鼠标的左键相应事件
    ///     
    /// @param[in]  event  相应的事件信息，可以获取位置信息等等
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月17日，新建函数
    ///
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

    ///
    /// @brief
    ///     鼠标释放事件
    ///
    /// @param[in]  event  
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年8月4日，新建函数
    ///
    void mouseReleaseEvent(QGraphicsSceneMouseEvent  *event);

public slots:
    ///
    /// @brief
    ///     拖动曲线
    ///
    /// @param[in]  posDistance  拖动距离
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年11月24日，新建函数
    ///
    void dragCurveMap(qreal& posDistance);

protected slots:
    ///
    /// @brief
    ///     处理滚轮事件
    ///
    /// @param[in]  wheel  true:正向转;false:反向转
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年7月14日，新建函数
    ///
    void wheelEvent(bool wheel);
private:
    ///
    /// @brief
    ///     计算显示的面积
    ///
    /// @param[in]  displayPercent  面积百分比
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年7月13日，新建函数
    ///
    void ShowRealMap(qreal displayPercent, qreal blance);

    ///
    /// @brief 计算X轴的坐标刻度
    ///     
    /// @param[in]  distanceOfvalue  X轴的最大值和最小值之差
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月17日，新建函数
    ///
    void computXcoordiante(qreal distanceOfvalue);

    ///
    /// @brief 计算Y轴的坐标刻度
    ///     
    /// @param[in]  distanceOfvalue  Y轴的最大值和最小值之差
    /// @param[in]  maxValue  最大值Y
    /// @param[in]  minValue  最小值Y
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月17日，新建函数
    ///   
    void computYcoordiante(qreal distanceOfvalue, qreal maxValue, qreal minValue);

    ///
    /// @brief 计算所有的坐标位置
    ///     
    /// @param[in]  data  所有曲线的数据，因为坐标系是根据曲线来计算的
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月17日，新建函数
    ///  
    void computCoordinatePostion(allCurveDataStore& data);

    ///
    /// @brief
    ///     坐标系的初始化
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年5月29日，新建函数
    ///
    void initalCoodinate();

    ///
    /// @brief
    ///     获取所给曲线组的X轴和Y轴的最大最小值,和X,Y轴的刻度队列
    ///
    /// @param[in]  curveGroup  曲线组
    /// @param[in]  PostionGroup  初始X,Y轴的刻度队列
    ///
    /// @return 最大最小值
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年11月26日，新建函数
    ///
    CoordinateSize CacMaxMinValueAndPostionGroup(allCurveDataStore& curveGroup, QMultiMap<qreal, DataPosAndValue>& PostionGroup);

protected:
    int                             Xaxis;                      // x轴长度
    int                             Yaxis;                      // y轴长度
    CoordinateDataType              ScaleCoordinateSystem;      // 坐标系的信息
    QMultiMap<qreal, DataPosAndValue>XaxisPostionGroup;         // 绘制坐标系的具体值
    QMultiMap<qreal, DataPosAndValue>YaxisPostionGroup;         // 绘制坐标系的具体值
    allCurveDataStore               CurveMap;                   // 绘制数据
    allCurveDataStore               RawCurMap;                  // 绘制全图
    qreal                           YDvalue;                    // y轴的最大最小值的相差
    qreal                           XDvalue;                    // x轴的最大最小值的相差
    qreal                           DisplayPrecent;             // 显示面积
    qreal                           StartPos;                   // 拖动图形的开始点位
    qreal                           DistanceMove;               // 距离移动比例
    qreal                           XmaxValue;                  // x轴的最大值
    qreal                           XminValue;                  // x轴的最小值
    qreal                           YmaxValue;                  // y轴的最大值
    qreal                           YminValue;                  // y轴的最小值
    qreal                           Blance;
};

