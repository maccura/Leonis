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
/// @file     common.h
/// @brief    图形绘制模块的公共参数
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
#include "QVariant"
#include <QList>
#include <QMap>
#include "boost/tuple/tuple.hpp"

#define MAXNUMBER 2147483647
typedef boost::tuple<qreal, qreal, qreal, qreal> CoordinateSize;

//小控件类型
enum ITEMTOOLMODE
{
    ITEMSELCTED = 0,
    ITEMHOVER
};

//曲线类型
enum CURVETYPE
{
    TRIANGLE = 0,
    SQUARE,
    CIRCLE,
	START,
	CALIBRATION,
	CURVEND
};

enum CURVECOLOR
{
	ZERO = 0,
	ONE,
	TWO,
	THREE,
	FOUR,
	FIVE,
	SIX,
	SEVEN,
	EIGHT,
	NINE,
	TEN,
    ELEVEN,
	TYPEEND
};

//存储数据的地方
struct RealDataStruct
{
    qreal                           xData;                      //x值
    qreal                           yData;                      //y值
    CURVETYPE                       type;                       //点的形状
    CURVECOLOR                      color;                      //点的颜色
	int	                            curveId;                    //曲线的内部id
    double                          conc;                       //浓度
	void*                           userData;                   //用户数据
    bool                            hollow;                     //是否是空心形状
    std::string                     tip;                        //提示信息，空不显示
    RealDataStruct()
    {
        type = TRIANGLE;
        color = FOUR;
        hollow = false;
    }
};
Q_DECLARE_METATYPE(RealDataStruct);

//曲线的点的信息
struct CurveDataInfo
{
    qreal                           xPos;                       //x的位置
    qreal                           yPos;                       //y的位置
    qreal                           value;                      //点的值
    CURVETYPE                       type;                       //点的形状
    CURVECOLOR                      color;                      //点的颜色
    bool                            IsConnected;                //是否需要连线
    RealDataStruct                  saveRawData;                //保存原始数据
    std::string                     tips;                       //提示
    CurveDataInfo()//初始化
    {
        xPos = 0;
        yPos = 0;
        type = TRIANGLE;
        color = FOUR;
    }
};

struct DataPosAndValue
{
    qreal                           xPos;                       // x的位置
    qreal                           yPos;                       // y的位置
    qreal                           width;                      // 坐标刻度的宽度
    qreal                           hieght;                     // 坐标刻度的高度
    qreal                           valueText;                  // 坐标的标记值
    bool                            specialPoint;               // 专用于绘制坐标系
    int                             direction;                  // 坐标轴方向
    DataPosAndValue()
    {
        xPos = 0;
        yPos = 0;
        width = 1;
        hieght = 1;
        valueText = 1;
        specialPoint = false;
        direction = 1;
    }
};

struct curveSingleDataStore
{
    CURVETYPE                       scatterMarkerShape;         //曲线的类型
	CURVECOLOR                      colorType;                  //线的颜色
    bool                            IsPointerConnect;           //点之间是否连线
	int                             curveId;                    //曲线id
    QList<RealDataStruct>           dataContent;                //保存绘制点的值
    curveSingleDataStore()
    {
        scatterMarkerShape = CIRCLE;
        colorType = EIGHT;
        IsPointerConnect = false;
        curveId = -1;
        dataContent.clear();
    }
};

typedef QMap<int,curveSingleDataStore> allCurveDataStore;

//坐标系传入参数
struct CoordinateDataType
{
    int                             xaAis;                      //x轴长度
    int                             YaAis;                      //y轴长度
    int                             tapeStrip;                  //隔离带大小
    int                             retainedNumber;             //保留几位有效数
    int                             startxDistance;             //曲线开始绘制位置x
    int                             startyDistance;             //曲线开始绘制位置y
    int                             height;                     //坐标系的刻度的高度
    int                             width;                      //坐标系的刻度的宽度
    int                             scaleNumber;                //Y轴的刻度数目
    ITEMTOOLMODE                    selectMode;                 //坐标轴中选中使用的方式
    bool                            scaleNeed;                  //是否允许放大缩小
    int                             scaleType;                  //坐标系类型,是虚线类型，还是刻度长方形的类型
    bool                            needRefreshXScale;          //是否需要更新X坐标系的刻度,可以根据xListScale的list来增加点
    QList<qreal>                    xListScale;                 //增加的X轴的点
    bool                            needRefreshYScale;          //是否需要更新Y坐标系的刻度,maxYsacle与minYscale最大小值
    qreal                           maxYsacle;                  //y轴刻度最大值
    qreal                           minYscale;                  //y轴刻度最小值
    QString                         unit;                       //Y轴的坐标单位
    bool                            needConnectPoint;           //是否需要把线连起来
    qreal                           minOutOfControlValue;       //失控点的最小值
    qreal                           maxOutOfControlValue;       //失控点的最大值
	qreal                           linewidth;                  //线宽
    qreal                           imageSize;                  //基本元素大小

	CoordinateDataType()
	{
        selectMode = ITEMSELCTED;
        imageSize = 12;
		height = 20;                                            //高度
		tapeStrip = 10;                                         //隔离栏的宽度
		width = 40;                                             //Y轴的刻度宽度
		scaleNumber = 8;                                        //显示的刻度数目
		startxDistance = 0;                                     //起点位置
		startyDistance = 0;
		scaleType = 0;
		needRefreshYScale = false;
		maxYsacle = 4;
		minYscale = -4;
		linewidth = 0;
		xListScale.append(1);
		xListScale.append(2);
		xListScale.append(3);
		xListScale.append(4);
		maxOutOfControlValue = MAXNUMBER;
		minOutOfControlValue = -MAXNUMBER;
		unit = QString("SD");                                   //单位
		needRefreshXScale = false;
        scaleNeed = false;
	}
};
