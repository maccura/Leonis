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

#include "CoordinateBase.h"
#include <QGraphicsSceneMouseEvent> 
#include "shared/uidef.h"

///
/// @brief 构造函数
/// @par History:
/// @li 5774/WuHongTao，2020年4月13日，新建函数
///
CoordinateBase::CoordinateBase(int xaAisLength, int yaAisLength)
{
    Xaxis = xaAisLength;//x轴长度
    Yaxis = yaAisLength;//y轴长度

    ScaleCoordinateSystem.xaAis = xaAisLength;
    ScaleCoordinateSystem.YaAis = yaAisLength;
    ScaleCoordinateSystem.height = 30;
    ScaleCoordinateSystem.tapeStrip = 8;
    ScaleCoordinateSystem.width = 40;
    ScaleCoordinateSystem.scaleNumber = 5;
    ScaleCoordinateSystem.scaleType = 0;
    // 坐标轴范围
    XmaxValue = -MAXNUMBER;
    XminValue = MAXNUMBER;
    YmaxValue = -MAXNUMBER;
    YminValue = MAXNUMBER;
    Blance = 0;
    //初始化范围
    DisplayPrecent = 1;
    //初始移动比例的大小
    DistanceMove = 0;
    //初始化坐标系
	initalCoodinate();
}

///
/// @brief 删除曲线
///     
///
/// @param[in]  curveId  对应的曲线ID
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2020年4月24日，新建函数
///
void CoordinateBase::removeCurve(int curveId)
{
    CurveMap.remove(curveId);
    RawCurMap.remove(curveId);

    //如若数据不为空，则重新计算曲线，重新绘制
    if (!CurveMap.isEmpty())
    {
        computCoordinatePostion(CurveMap);
    }
    else
    {
        // 数据为空，则清除坐标系数据，刷新
        XaxisPostionGroup.clear();
        YaxisPostionGroup.clear();

    }

    update();
}

///
/// @brief 设置坐标系的属性，比如隔离带的大小，宽度等等
///     
/// @param[in]  scaleInfo  
/// @par History:
/// @li 5774/WuHongTao，2020年4月17日，新建函数
///
void CoordinateBase::setAttribute(CoordinateDataType scaleInfo)
{
    ScaleCoordinateSystem = scaleInfo;
    //改变了坐标系属性以后，需要重新计算整个坐标系
    computCoordinatePostion(CurveMap);
    update();
}

///
/// @brief 添加曲线
///     
/// @param[in]  data  曲线的数据
/// @par History:
/// @li 5774/WuHongTao，2020年4月17日，新建函数
///
void CoordinateBase::addCurveData(int dataId,curveSingleDataStore& data)
{
    // 保存原始数据
    RawCurMap.insert(dataId, data);
    curveSingleDataStore tmpLine;
    tmpLine = data;
    tmpLine.dataContent.clear();

    //DisplayPrecent范围比校在0.1-1之间
    if (DisplayPrecent > 1)
    {
        DisplayPrecent = 1;
    }

    if (DisplayPrecent < STEP)
    {
        DisplayPrecent = STEP;
    }

    ShowRealMap(DisplayPrecent, Blance);
}

///
/// @brief 析构函数
/// @par History:
/// @li 5774/WuHongTao，2020年4月13日，新建函数
///
CoordinateBase::~CoordinateBase()
{

}

///
/// @brief 返回图形的绘制区域
/// @return 绘制区域的大小
///
/// @par History:
/// @li 5774/WuHongTao，2020年4月17日，新建函数
///
QRectF CoordinateBase::boundingRect()const
{
    return QRectF(0, 0, Xaxis, Yaxis);
}

///
/// @brief 计算Y轴的坐标刻度
///     
/// @param[in]  distanceOfvalue  Y轴的最大值和最小值之差
/// @param[in]  maxValue  最大值Y
/// @param[in]  minValue  最小值Y
/// @par History:
/// @li 5774/WuHongTao，2020年4月17日，新建函数
///
void CoordinateBase::computYcoordiante(qreal distanceOfvalue,qreal maxValue, qreal minValue)
{
    //如果是0个刻度点，直接退出，不计算，因为没法计算，不合理
    if (ScaleCoordinateSystem.scaleNumber == 0)
    {
        return;
    }

    //y轴是按照距离等分来设计，计算每一次的增加距离
    DataPosAndValue tmpData;
    qreal dealt = distanceOfvalue / ScaleCoordinateSystem.scaleNumber;
    //计算每一次的增加值
    qreal dealty = qreal(Yaxis - ScaleCoordinateSystem.startyDistance - 2 * ScaleCoordinateSystem.tapeStrip - ScaleCoordinateSystem.height) / ScaleCoordinateSystem.scaleNumber;
    for (int i = 0; i < ScaleCoordinateSystem.scaleNumber; i++)
    {
        tmpData.hieght = dealty;
        tmpData.xPos = ScaleCoordinateSystem.startxDistance;
        tmpData.yPos = ScaleCoordinateSystem.startyDistance + ScaleCoordinateSystem.tapeStrip + dealty*i;
        tmpData.width = ScaleCoordinateSystem.width;
        tmpData.valueText = maxValue - (i)*dealt;
        YaxisPostionGroup.insert(tmpData.valueText, tmpData);
    }

    //最后的值
    tmpData.xPos = ScaleCoordinateSystem.startxDistance;
    tmpData.yPos = tmpData.yPos + dealty;
    tmpData.hieght = Yaxis - tmpData.yPos;
    tmpData.width = ScaleCoordinateSystem.width;
    tmpData.valueText = minValue;
    YaxisPostionGroup.insert(tmpData.valueText, tmpData);

	//上部隔离带的绘制
    tmpData.hieght = ScaleCoordinateSystem.tapeStrip;
    tmpData.xPos = ScaleCoordinateSystem.startxDistance;
    tmpData.yPos = ScaleCoordinateSystem.startyDistance;
    tmpData.width = ScaleCoordinateSystem.width;
    tmpData.specialPoint = true;
    tmpData.valueText = MAXNUMBER;
	YaxisPostionGroup.insert(tmpData.valueText, tmpData);

    //下部位置的绘制
    tmpData.hieght = ScaleCoordinateSystem.height;
    tmpData.xPos = ScaleCoordinateSystem.startxDistance;
    tmpData.yPos = Yaxis - ScaleCoordinateSystem.height;
    tmpData.width = ScaleCoordinateSystem.width;
    tmpData.specialPoint = true;
    tmpData.valueText = MAXNUMBER;
    YaxisPostionGroup.insert(tmpData.valueText, tmpData);
}

///
/// @brief 获取坐标系的刻度信息等等
///     
/// @return 刻度信息的数据结构
///
/// @par History:
/// @li 5774/WuHongTao，2020年4月24日，新建函数
///
CoordinateDataType CoordinateBase::getCoordinateInformation()
{
    return ScaleCoordinateSystem;
}

///
/// @brief 获取X轴坐标轴的刻度信息
///     
/// @return x轴坐标轴的刻度信息结构体
///
/// @par History:
/// @li 5774/WuHongTao，2020年4月24日，新建函数
///
QMap<qreal, DataPosAndValue> CoordinateBase::getXcoorDinatePostion()
{
    return XaxisPostionGroup;
}

///
/// @brief 获取y轴坐标轴的刻度信息
///     
/// @return y轴坐标轴的刻度信息结构体
///
/// @par History:
/// @li 5774/WuHongTao，2020年4月24日，新建函数
QMap<qreal, DataPosAndValue> CoordinateBase::getYcoorDinatePostion()
{
    return YaxisPostionGroup;
}

///
/// @brief
///     清除所有曲线
///
/// @par History:
/// @li 5774/WuHongTao，2020年5月27日，新建函数
///
void CoordinateBase::clearCurve()
{
    CurveMap.clear();
    RawCurMap.clear();
    XaxisPostionGroup.clear();
    YaxisPostionGroup.clear();
    DisplayPrecent = 1;
    Blance = 0;
    update();
}

///
/// @brief
///     获取坐标系范围
///
/// @return x轴最大，x轴最小，Y轴最大，Y轴最小
///
/// @par History:
/// @li 5774/WuHongTao，2020年11月24日，新建函数
///
CoordinateSize CoordinateBase::GetCoordinatesize()
{
    return boost::make_tuple(XmaxValue, XminValue, YmaxValue, YminValue);
}

///
/// @brief 设置为默认视图比例
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月9日，新建函数
///
void CoordinateBase::setDefaultViewScale()
{
	DisplayPrecent = 1;
	Blance = 0;
	ShowRealMap(DisplayPrecent, Blance);
}

///
/// @brief
///     处理滚轮事件
///
/// @param[in]  wheel  true:正向转;false:反向转
///
/// @par History:
/// @li 5774/WuHongTao，2020年7月14日，新建函数
///
void CoordinateBase::wheelEvent(bool wheel)
{
    //如果不允许放大缩小，则直接返回
    if (!ScaleCoordinateSystem.scaleNeed)
    {
        return;
    }

    //根据滚轮方向做放大还是缩小
    if (wheel)
    {
        DisplayPrecent = DisplayPrecent - STEP;
    }
    else
    {
        DisplayPrecent = DisplayPrecent + STEP;
    }

    ShowRealMap(DisplayPrecent, Blance);
}

///
/// @brief
///     拖动曲线
///
/// @param[in]  posDistance  拖动距离
///
/// @par History:
/// @li 5774/WuHongTao，2020年11月24日，新建函数
///
void CoordinateBase::dragCurveMap(qreal& posDistance)
{
    //只有在整个图小于1的时候，才能拖动，否则是全图
    if (DisplayPrecent >= 1)
    {
        return;
    }

    //避免崩溃
    if (DisplayPrecent <= 0)
    {
        return;
    }
    //虚拟总长度
    qreal virtualDistance = (Xaxis - ScaleCoordinateSystem.startxDistance - ScaleCoordinateSystem.width) / DisplayPrecent;

    //避免除0
    if (virtualDistance <= 0)
    {
        return;
    }

    if (posDistance == 0)
    {
        return;
    }

    qreal blance = Blance - posDistance / virtualDistance;
    qreal LeftStartPercent = (1 - DisplayPrecent) / 2;
    qreal RightEndPercent = LeftStartPercent + DisplayPrecent;

    if ((LeftStartPercent + blance) < 0 || (RightEndPercent + blance) < 0)
    {
        Blance = -LeftStartPercent;
    }
    else if ((LeftStartPercent + blance) > 1 || (RightEndPercent + blance) > 1)
    {
        Blance = 1 - RightEndPercent;
    }
    else
    {
        Blance = blance;
    }

    ShowRealMap(DisplayPrecent, Blance);
}

///
/// @brief
///     计算显示的面积
///
/// @param[in]  displayPercent  面积百分比
///
/// @par History:
/// @li 5774/WuHongTao，2020年7月13日，新建函数
///
void CoordinateBase::ShowRealMap(qreal displayPercent, qreal blance)
{
    DisplayPrecent = displayPercent;
    CurveMap.clear();

    //范围为0.1-1
    if (DisplayPrecent > 1)
    {
        DisplayPrecent = 1;
        Blance = 0;
    }

    if (DisplayPrecent < STEP)
    {
        DisplayPrecent = STEP;
    }

    // 获取最大值
    QMultiMap<qreal, DataPosAndValue> tmpPostion;
    CoordinateSize maxminValue;
    maxminValue = CacMaxMinValueAndPostionGroup(RawCurMap, tmpPostion);
    qreal xmaxValue = maxminValue.get<0>();

    // 调整图形的显示
    qreal LeftStartPercent = (1 - DisplayPrecent) / 2;
    qreal RightEndPercent = LeftStartPercent + DisplayPrecent;

    LeftStartPercent = LeftStartPercent + Blance;
    RightEndPercent = DisplayPrecent + LeftStartPercent;
    // 获取显示范围
    qreal start = xmaxValue * LeftStartPercent;
    qreal end = xmaxValue* RightEndPercent;

    curveSingleDataStore tmpLine;
    //根据新的范围来计算值
    for (auto line : RawCurMap)
    {
        //如若等于0，则继续下一个
        if (line.dataContent.size() == 0)
        {
            continue;
        }

        tmpLine = line;
        tmpLine.dataContent.clear();        
        for (size_t i = 0; i < line.dataContent.size(); i++)
        {
            if (line.dataContent[i].xData < start || line.dataContent[i].xData > end)
            {
                continue;
            }

            tmpLine.dataContent.append(line.dataContent[i]);
        }
        CurveMap.insert(tmpLine.curveId, tmpLine);
    }

    //更新坐标数据
    computCoordinatePostion(CurveMap);
    update();

    //通知内容更新曲线信息
    emit updateCurveData();
}

///
/// @brief 计算X轴的坐标刻度
///     
/// @param[in]  distanceOfvalue  X轴的最大值和最小值之差
/// @par History:
/// @li 5774/WuHongTao，2020年4月17日，新建函数
///
void CoordinateBase::computXcoordiante(qreal distanceOfvalue)
{
    //当间隔距离等于0时候，没有意义
    if (distanceOfvalue == 0)
    {
        return;
    }

    DataPosAndValue tmpData;
    //计算起位置
    qreal xPostion = ScaleCoordinateSystem.startxDistance + ScaleCoordinateSystem.tapeStrip+ ScaleCoordinateSystem.width;
    //根据x轴中的数据计算x的段数
    QMap<qreal, DataPosAndValue>::iterator iter = XaxisPostionGroup.begin();
    //此处实现第一个位置的初始化
    qreal lastPostion = iter->xPos;
    {
        iter->xPos = xPostion;
        iter->width = ScaleCoordinateSystem.width;
        iter->hieght = ScaleCoordinateSystem.height;
        iter->yPos = Yaxis - ScaleCoordinateSystem.height;
        iter->valueText = lastPostion;
        iter++;
    }

    while (iter != XaxisPostionGroup.end())
    {
        iter->width = ScaleCoordinateSystem.tapeStrip;//此处设置宽度是，为最后一个数据设置，其他的数据在计算过程中会被覆盖,只有最后一个才不会被覆盖
        /*
        每次计算宽度只能计算上次的宽度，因为只有通过本次的x值减去上次的x值才能得出宽度
        计算宽度的方法是用本次与上次的x只差相减，然后比照最大长度
        减去1和加上1的原因是为了让显示分明
        */
        (iter - 1)->width = (iter->xPos - lastPostion) / distanceOfvalue * qreal((Xaxis - ScaleCoordinateSystem.startxDistance - 2 * ScaleCoordinateSystem.tapeStrip- ScaleCoordinateSystem.width));

        xPostion = (iter - 1)->width + xPostion;//加上线宽
        lastPostion = iter->xPos;
        iter->hieght = ScaleCoordinateSystem.height;
        iter->width = Xaxis - xPostion;
        iter->valueText = iter->xPos;
        iter->xPos = xPostion;
        iter->yPos = Yaxis - ScaleCoordinateSystem.height;
        iter++;
    }

    //添加隔离带开始区域,隔离区域需要特殊处理，上面不打出刻度值
    tmpData.hieght = ScaleCoordinateSystem.height;
    tmpData.xPos = ScaleCoordinateSystem.startxDistance+ ScaleCoordinateSystem.width;
    tmpData.yPos = Yaxis - ScaleCoordinateSystem.height;
    tmpData.width = ScaleCoordinateSystem.tapeStrip;
    tmpData.specialPoint = true;
    tmpData.valueText = MAXNUMBER;
    XaxisPostionGroup.insert(tmpData.valueText, tmpData);
}

///
/// @brief 计算所有的坐标位置
///     
/// @param[in]  data  所有曲线的数据，因为坐标系是根据曲线来计算的
/// @par History:
/// @li 5774/WuHongTao，2020年4月17日，新建函数
///
void CoordinateBase::computCoordinatePostion(allCurveDataStore& data)
{
    //如果值为空，直接返回
    if (data.empty())
    {
        return;
    }

    XmaxValue = -MAXNUMBER;
    XminValue = MAXNUMBER;
    YmaxValue = -MAXNUMBER;
    YminValue = MAXNUMBER;

    XaxisPostionGroup.clear();
    YaxisPostionGroup.clear();
    CoordinateSize maxminValue;
    maxminValue = CacMaxMinValueAndPostionGroup(data, XaxisPostionGroup);
    XmaxValue = maxminValue.get<0>();
    XminValue = maxminValue.get<1>();
    YmaxValue = maxminValue.get<2>();
    YminValue = maxminValue.get<3>();

    //y轴的最大最小值的相差
    DataPosAndValue tmpData;
    if (ScaleCoordinateSystem.needRefreshYScale)
    {
        YDvalue = ScaleCoordinateSystem.maxYsacle - ScaleCoordinateSystem.minYscale;
        YminValue = ScaleCoordinateSystem.minYscale;
        YmaxValue = ScaleCoordinateSystem.maxYsacle;
    }
    else
    {
        YDvalue = YmaxValue - YminValue;
    }

    if (ScaleCoordinateSystem.needRefreshXScale)
    {
        for (qreal var : ScaleCoordinateSystem.xListScale)
        {
            //更新最大最小值
            if (XmaxValue < var)
            {
                XmaxValue = var;
            }
            if (var < XminValue)
            {
                XminValue = var;
            }
            tmpData.valueText = var;
            tmpData.xPos = var;
            XaxisPostionGroup.insert(var, tmpData);
        }
        XDvalue = XmaxValue - XminValue;
    }
    else
    {
        //x轴的最大最小值的相差
        XDvalue = XmaxValue - XminValue;
    }
    
    tmpData.valueText = XmaxValue;
    tmpData.xPos = XmaxValue;
    XaxisPostionGroup.insert(XmaxValue, tmpData);

    tmpData.valueText = XminValue;
    tmpData.xPos = XminValue;
    XaxisPostionGroup.insert(XminValue, tmpData);
    //绘制X轴
    computXcoordiante(XDvalue);
    //绘制Y轴
    computYcoordiante(YDvalue, YmaxValue, YminValue);
}

///
/// @brief
///     坐标系的初始化
///
/// @par History:
/// @li 5774/WuHongTao，2020年5月29日，新建函数
///
void CoordinateBase::initalCoodinate()
{
    //暂时返回，有bug
    return;
    if (ScaleCoordinateSystem.scaleType == 0)
    {
        curveSingleDataStore dateCurve;
        RealDataStruct tempdata;
        dateCurve.scatterMarkerShape = TRIANGLE;
        dateCurve.colorType = ONE;
        dateCurve.IsPointerConnect = false;

        for (size_t i = 0; i <= 10; i++)
        {
            tempdata.xData = 10 * i;
            tempdata.yData = 100 * i;
            dateCurve.dataContent.append(tempdata);
        }
        CurveMap.insert(1, dateCurve);
        computCoordinatePostion(CurveMap);
        update();
        CurveMap.clear();
    }
    else
    {

    }
}

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
CoordinateSize CoordinateBase::CacMaxMinValueAndPostionGroup(allCurveDataStore& curveGroup, QMultiMap<qreal, DataPosAndValue>& PostionGroup)
{
    qreal xmaxValue = -MAXNUMBER;
    qreal xminValue = MAXNUMBER;
    qreal ymaxValue = -MAXNUMBER;
    qreal yminValue = MAXNUMBER;

    DataPosAndValue tmpData;
    for (curveSingleDataStore singleData : curveGroup)
    {
        for (RealDataStruct var : singleData.dataContent)
        {
            //找到最大和最小的x的值
            if (xmaxValue < var.xData)
            {
                xmaxValue = var.xData;
            }

            if (var.xData < xminValue)
            {
                xminValue = var.xData;
            }

            //找到最大或者是最小的y值
            if (ymaxValue < var.yData)
            {
                ymaxValue = var.yData;
            }

            if (var.yData < yminValue)
            {
                yminValue = var.yData;
            }

            tmpData.xPos = var.xData;
            tmpData.yPos = var.yData;

            //此处做特殊处理，当为校准曲线的时候，不需要根据他来画坐标轴，只可用他的值来确定最大最小值
            if (singleData.scatterMarkerShape != CALIBRATION)
            {
                PostionGroup.insert(var.xData, tmpData);
            }
        }
    }

    // 当Y轴最大值不足刻度数整倍时，给一个补充
    // 否则图形会出现各种奇怪的问题
    yminValue = std::floor(yminValue);
    ymaxValue = std::ceil(ymaxValue + 0.5);
    int spanY = ymaxValue - yminValue;
    if ( spanY % ScaleCoordinateSystem.scaleNumber !=  0)
    {
        ymaxValue = (int)(ScaleCoordinateSystem.scaleNumber *(spanY/ ScaleCoordinateSystem.scaleNumber + 1)) + int(yminValue);
    }
    // 返回最大最小值
    return boost::make_tuple(xmaxValue, xminValue, ymaxValue, yminValue);
}

///
/// @brief 鼠标的左键相应事件
///     
/// @param[in]  event  相应的事件信息，可以获取位置信息等等
/// @par History:
/// @li 5774/WuHongTao，2020年4月17日，新建函数
///
void CoordinateBase::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    StartPos = event->pos().x();
    return;
}

///
/// @brief
///     鼠标释放事件
///
/// @param[in]  event  
///
/// @par History:
/// @li 5774/WuHongTao，2020年8月4日，新建函数
///
void CoordinateBase::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    //只有在整个图小于1的时候，才能拖动，否则是全图
    if (DisplayPrecent >= 1)
    {
        return;
    }

    //避免崩溃
    if (DisplayPrecent <= 0)
    {
        return;
    }
    //虚拟总长度
    qreal virtualDistance = (Xaxis - ScaleCoordinateSystem.startxDistance - ScaleCoordinateSystem.width)/ DisplayPrecent;

    //避免除0
    if (virtualDistance <= 0)
    {
        return;
    }

    //计算位置之差
    qreal posDistance = event->pos().x() - StartPos;
    //移动多少位置
    if (posDistance > 10)
    {
        DistanceMove = -STEP;
    }
    else if (posDistance < -10)
    {
        DistanceMove = STEP;
    }
    else
    {
        return;
    }

    qreal centerPointer = (1 - DisplayPrecent) / 2;

    Blance = Blance + DistanceMove;

    if (Blance > centerPointer)
    {
        Blance = centerPointer;
    }

    if (Blance < -centerPointer)
    {
        Blance = -centerPointer;
    }

    ShowRealMap(DisplayPrecent, Blance);
}
