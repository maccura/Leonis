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
/// @file     ScatterPlotContentClass.h
/// @brief    具体曲线的绘制
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

#include "ScatterPlotContentClass.h"
#include <boost/timer/timer.hpp>
#include <QPainter>
#include "CoordinateBase.h"
#include "Triangle.h"
#include "Circle.h"
#include "Start.h"
#include "Sharp.h"
#include "SQuare.h"
#include "ScatterAsideItem.h"
#include "shared/uidef.h"

///
/// @brief 构造函数，绘制曲线
///     
/// @param[in]  coordinateClass  需要对应的坐标系的对象
///
/// @par History:
/// @li 5774/WuHongTao，2020年4月24日，新建函数
///
CScatterPlotContentClass::CScatterPlotContentClass(CoordinateBase *coordinateClass, CScatterAsideItem* Item)
{
    m_coorDinateObject = coordinateClass;
    m_controlItem = Item;
    m_calibrateCurveLine = 0;
    m_scaleInfo = m_coorDinateObject->getCoordinateInformation();
    m_xaAis = m_scaleInfo.xaAis;//x轴长度
    m_YaAis = m_scaleInfo.YaAis;//y轴长度
    m_needRefresh = false;
    m_imageSize = 12;
    m_curveCount = 0;
    m_curveId = 0;
    m_currentSelectPointerId = -1;

    //1:三角形
    shared_ptr<CSharp> triangleObj(new CTriangle(m_imageSize, m_imageSize));
    m_sharpList.append(triangleObj);
    //2：正方形
    shared_ptr<CSharp> square(new CSQuare(m_imageSize, m_imageSize));
    m_sharpList.append(square);
    //3：圆形
    shared_ptr<CSharp> cicrcle(new CCircle(m_imageSize, m_imageSize));
    m_sharpList.append(cicrcle);
    //4:星形
    shared_ptr<CSharp> start(new CStart(9, 5));
    m_sharpList.append(start);

    //不能除0
    if (m_YaAis != 0)
    {
        m_scale = double(m_xaAis) / double(m_YaAis);
    }
    else
    {
        m_scale = 1;
    }

    //初始化颜色列表
    initalColorList();
}

///
/// @brief 设置坐标系的属性
///     
///
/// @param[in]  scaleInfo  属性值的结构体
/// @par History:
/// @li 5774/WuHongTao，2020年4月21日，新建函数
///
void CScatterPlotContentClass::setAttribute(CoordinateDataType scaleInfo)
{
    m_scaleInfo = scaleInfo;
    if (m_coorDinateObject != NULL)
    {
        m_coorDinateObject->setAttribute(scaleInfo);
    }

    if (m_controlItem != nullptr)
    {
        m_scaleInfo.xaAis = m_xaAis;//x轴长度
        m_scaleInfo.YaAis = m_YaAis;//y轴长度
        m_controlItem->setAttribute(m_scaleInfo);
    }
    updateCurveData(m_dataContent);
}

///
/// @brief
///     获取当前坐标系的属性设置
///
///
/// @return 坐标系属性数据结构
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月4日，新建函数
///
CoordinateDataType& CScatterPlotContentClass::GetAxisAttribute()
{
    return m_scaleInfo;
}

///
/// @brief
///     尺寸变化的比例
///
/// @param[in]  wscale  宽度尺寸变化
/// @param[in]  hscale  高度尺寸变化
///
/// @par History:
/// @li 5774/WuHongTao，2020年5月29日，新建函数
///
void CScatterPlotContentClass::setScale(double wscale, double hscale)
{
    m_wscale = wscale;
    m_hscale = hscale;
    for (shared_ptr<CSharp> sharp : m_sharpList)
    {
        sharp->updateScaleSharp(sharp->m_width / wscale, sharp->m_height / hscale);
    }
    m_controlItem->setScale(wscale, hscale);
}

///
/// @brief
///     清除所有曲线
///
/// @par History:
/// @li 5774/WuHongTao，2020年5月27日，新建函数
///
void CScatterPlotContentClass::clearCurve()
{
    m_dataContent.clear();
    m_allScatterPointer.clear();
    m_calibrationCurve.clear();
    //去掉选中线
    m_needRefresh = false;
    m_currentSelectPointerId = -1;
	//调用此函数刷新一次，否则只清除数据，不增加新曲线时，焦点线仍然会显示
	updateCurveInfo();
    update();
}

///
/// @brief
///     析构函数
///
/// @par History:
/// @li 5774/WuHongTao，2020年5月29日，新建函数
///
CScatterPlotContentClass::~CScatterPlotContentClass()
{
    m_sharpList.clear();
}

///
/// @brief
///     初始化颜色表
/// @par History:
/// @li 5774/WuHongTao，2020年5月29日，新建函数
///
void CScatterPlotContentClass::initalColorList()
{
	m_colorList.append(QColor(59, 143, 246));
	m_colorList.append(QColor(250, 144, 22));
	m_colorList.append(QColor(65, 196, 106));
	m_colorList.append(QColor(255, 0, 0));
	m_colorList.append(QColor(156, 102, 31));
	m_colorList.append(QColor(255, 153, 18));
	m_colorList.append(QColor(176, 23, 31));
	m_colorList.append(QColor(85, 102, 0));
	m_colorList.append(QColor(255, 127, 80));
	m_colorList.append(QColor(255, 215, 0));
    m_colorList.append(QColor(69, 159, 255));
    m_colorList.append(QColor(164, 164, 164));
    m_colorList.append(QColor(31, 181, 88));
}

///
/// @brief 绘制函数
///     
/// @param[in]  painter  画笔
/// @param[in]  option   绘画设置选项
/// @param[in]  widget  
///
/// @par History:
/// @li 5774/WuHongTao，2020年4月21日，新建函数
///
void CScatterPlotContentClass::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget /* = 0 */)
{
    QPen pen;
    pen.setColor(Qt::black);
    pen.setWidth(0);
    painter->save();
    painter->setPen(pen);
    QBrush oldBrush = painter->brush();
    QPointF lastPointer;
    QPointF curentPointer;
    QPointF myLinkPointer;//当使用连线的时候
    painter->setRenderHint(QPainter::Antialiasing, true);

    //绘制平滑的曲线--函数计算数据
    qreal maxX = 0;
    for (QList<CurveDataInfo> sigalLine : m_calibrationCurve)
    {
        drawCalibrationLine(painter, sigalLine);
        if (sigalLine.last().xPos > maxX)
        {
            maxX = sigalLine.last().xPos;
        }
    }

    bool IsFirstPointer = false;//确定是否第一个点
    for (QList<CurveDataInfo> curvePointerList : m_allScatterPointer)
    {
        IsFirstPointer = false;//确定是否第一个点，每次更换曲线的时候，复位
        for (CurveDataInfo point : curvePointerList)
        {

            //绘制一边的曲线{带形状的点}
            {
                if (point.type < CALIBRATION)
                {
                    curentPointer.setX(point.xPos - m_sharpList[point.type]->m_width / 2);
                    curentPointer.setY(point.yPos - m_sharpList[point.type]->m_height / 2);
                    

                    QColor colorUse;
                    if (point.color >= m_colorList.size())
                    {
                        colorUse = m_colorList[0];
                    }
                    else
                    {
                        colorUse = m_colorList[point.color];
                    }

                    if (!point.saveRawData.hollow)
                        painter->setBrush(colorUse);
                    else
                    {
                        //painter->setBrush(Qt::white);
                        pen.setWidth(1);
                    }

                    pen.setColor(colorUse);
                    painter->setPen(pen);
                    QPainterPath sharp;
                    sharp = m_sharpList[point.type]->m_sharp;
                    sharp.translate(curentPointer);
                    painter->drawPath(sharp);                   
                    if (!point.tips.empty())
                    {
                        painter->setBrush(Qt::white);
                        pen.setWidth(1);
                        painter->setPen(pen);
                        int step = 10;
                        if (curentPointer.x() + 35 > maxX)
                        {
                            step = -30;
                        }
                        painter->drawRect(curentPointer.x() + step, curentPointer.y(), 27, 10);
                        QFont font;
                        font.setPixelSize(8);
                        font.setFamily(FONT_FAMILY);
                        painter->setFont(font);
                        pen.setColor(colorUse);
                        painter->setPen(pen);
                        painter->drawText(QPoint(curentPointer.x() + step + 2, curentPointer.y() +8), QString::fromStdString(point.tips));
                    }
                    painter->setBrush(oldBrush);
                }

                myLinkPointer.setX(point.xPos);
                myLinkPointer.setY(point.yPos);
            }

            //如果是需要连线，则做特殊处理
            if (point.IsConnected)
            {
                if (IsFirstPointer == false)
                {
                    IsFirstPointer = true;
                }
                else
                {
                    myLinkPointer.setX(point.xPos);
                    myLinkPointer.setY(point.yPos);
                    painter->setRenderHint(QPainter::Antialiasing, true);		//防锯齿
                    painter->drawLine(lastPointer, myLinkPointer);
                }
                lastPointer = myLinkPointer;
            }
        }
    }
    painter->restore();
};

///
/// @brief 返回界面的课操作范围
///     
/// @return 操作范围的矩形描述
///
/// @par History:
/// @li 5774/WuHongTao，2020年4月21日，新建函数
///
QRectF CScatterPlotContentClass::boundingRect()const
{
    return QRectF(0, 0, m_xaAis, m_YaAis);
};

///
/// @brief 删除对应id的1d曲线
///     
/// @param[in]  curveId  具体的id号
///
/// @par History:
/// @li 5774/WuHongTao，2020年4月24日，新建函数
///
void CScatterPlotContentClass::removeCurve(int curveId)
{
    m_dataContent.remove(curveId);
    //如果删除的曲线有对应的选中线，去掉
    if (m_currentSelectPointerId == curveId)
    {
        m_needRefresh = false;
        m_currentSelectPointerId = -1;
    }

    //不等于空才可以删
    if (!m_dataContent.isEmpty())
    {
        if (m_coorDinateObject != NULL)
        {
            m_coorDinateObject->removeCurve(curveId);
        }
    }
    else
    {
        m_allScatterPointer.clear();
        m_calibrationCurve.clear();

        //去掉选中线
        m_needRefresh = false;
        m_currentSelectPointerId = -1;
    }

    //更新曲线
    updateCurveData(m_dataContent);
}

///
/// @brief 添加曲线的函数
///     
/// @param[in]  data  具体的曲线的点阵list
/// @par History:
/// @li 5774/WuHongTao，2020年4月21日，新建函数
///
int CScatterPlotContentClass::addCurveData(curveSingleDataStore& data)
{
    ++m_curveId;
    data.curveId = m_curveId;
    m_dataContent.insert(m_curveId, data);

    if (m_coorDinateObject != NULL)
    {
        m_coorDinateObject->addCurveData(m_curveId, data);
    }

    updateCurveData(m_dataContent);
    return m_curveId;
}

///
/// @brief 计算映射的点的位置
///     
/// @param[in]  curveData  所有曲线的容器
/// @par History:
/// @li 5774/WuHongTao，2020年4月21日，新建函数
///
void CScatterPlotContentClass::updateCurveData(allCurveDataStore& curveData)
{
    //如果值为空，直接返回
    if (curveData.empty())
    {
        m_controlItem->refreshCurvePointer(m_allScatterPointer);
        return;
    }
    m_calibrationCurve.clear();

    CurveDataInfo tmpDataReal;
    QList<CurveDataInfo> scatterCurve;//单根曲线的点
    m_allScatterPointer.clear();
    m_curveCount = 0;//先设置为0

    CoordinateSize coordinateSize = m_coorDinateObject->GetCoordinatesize();

    qreal xmax = coordinateSize.get<0>();
    qreal xmin = coordinateSize.get<1>();
    qreal ymax = coordinateSize.get<2>();
    qreal ymin = coordinateSize.get<3>();

    //获取坐标系的刻度值
    QMap<qreal, DataPosAndValue> xCoord = m_coorDinateObject->getXcoorDinatePostion();
    QMap<qreal, DataPosAndValue> yCoord = m_coorDinateObject->getYcoorDinatePostion();

    // 坐标系为空直接返回
    if (xCoord.empty() || yCoord.empty())
    {
        return;
    }

    qreal lastYValue = m_coorDinateObject->getYcoorDinatePostion().first().valueText;
    qreal lastYPos = m_coorDinateObject->getYcoorDinatePostion().first().yPos;

    qreal lastValue = m_coorDinateObject->getXcoorDinatePostion().first().valueText;
    qreal lastPos = m_coorDinateObject->getXcoorDinatePostion().first().xPos;

    //计算每一个点，在对应坐标系中的位置
    for (curveSingleDataStore singleData : curveData)
    {
        int type = singleData.scatterMarkerShape;
        scatterCurve.clear();
        QList<CurveDataInfo> calibrateTmpData;
        calibrateTmpData.clear();

        //每一根曲线的坐标位置的计算
        for (RealDataStruct pointerInfo : singleData.dataContent)
        {
            // 点的值在x轴范围之外
            if (pointerInfo.xData > xmax || pointerInfo.xData < xmin)
            {
                continue;
            }

            // 点的值在y轴范围之外
            if (pointerInfo.yData > ymax || pointerInfo.yData < ymin)
            {
                continue;
            }

            //1：首先将原始数据保存
            tmpDataReal.type = pointerInfo.type;
            tmpDataReal.color = singleData.colorType;
            tmpDataReal.value = pointerInfo.yData;
            tmpDataReal.saveRawData = pointerInfo;//保存原始数据
            tmpDataReal.saveRawData.curveId = singleData.curveId;
            tmpDataReal.tips = pointerInfo.tip;

            //是否在范围之内
            bool IsRange = false;
            //是否是坐标系的第一个点
            bool IsFirstPostion = true;
            //计算X的坐标系值
            for (DataPosAndValue posAndValue : xCoord)
            {
                // 如果是用于绘制边隔离带的值
                if (posAndValue.specialPoint)
                {
                    continue;
                }

                //根据刻度的位置计算，点的位置
                if (posAndValue.valueText >= pointerInfo.xData)
                {
                    //是否在范围内的标志
                    IsRange = true;

                    if (posAndValue.valueText == pointerInfo.xData)
                    {
                        tmpDataReal.xPos = posAndValue.xPos;
                    }
                    else
                    {
                        tmpDataReal.xPos = posAndValue.xPos - (posAndValue.valueText - pointerInfo.xData) * (posAndValue.xPos - lastPos) / (posAndValue.valueText - lastValue);
                    }
                    break;
                }
            }

            //如果找不到对应的位置，则返回
            if (!IsRange)
            {
                continue;
            }

            IsRange = false;
            //计算Y的坐标系值
            for (DataPosAndValue posAndValue : yCoord)
            {
                // 如果是用于绘制边隔离带的值
                if (posAndValue.specialPoint)
                {
                    continue;
                }

                //根据y的刻度计算每一个y的位置
                if (posAndValue.valueText >= pointerInfo.yData)
                {
                    IsRange = true;
                    if (posAndValue.valueText == pointerInfo.yData)
                    {
                        tmpDataReal.yPos = posAndValue.yPos;
                    }
                    else
                    {
                        tmpDataReal.yPos = posAndValue.yPos + (posAndValue.valueText - pointerInfo.yData) * (abs(posAndValue.yPos - lastYPos)) / (posAndValue.valueText - lastYValue);
                    }
                    break;
                }
                lastYPos = posAndValue.yPos;
                lastYValue = posAndValue.valueText;
            }

            //如果找不到对应的位置，则返回
            if (!IsRange)
            {
                continue;
            }

            //确定是一般的曲线{由形状组成，通常只有几十个}，平滑曲线{基本上200个以上}
            tmpDataReal.IsConnected = singleData.IsPointerConnect;
            if (singleData.scatterMarkerShape == CALIBRATION)
            {
                calibrateTmpData.append(tmpDataReal);
            }
            else
            {
                scatterCurve.append(tmpDataReal);
            }
        }

        if (singleData.scatterMarkerShape == CALIBRATION)
        {
            m_calibrationCurve.insert(singleData.curveId, calibrateTmpData);
        }
        else
        {
            m_allScatterPointer.insert(singleData.curveId, scatterCurve);
        }
        m_curveCount++;
    }

    //将重新计算的值刷新到小控件的对象中
    m_controlItem->refreshCurvePointer(m_allScatterPointer);
}

///
/// @brief 绘制拟合曲线
///     
///
/// @param[in]  painter  画笔
/// @param[in]  curveData  拟合点
/// @par History:
/// @li 5774/WuHongTao，2020年4月21日，新建函数
///
void CScatterPlotContentClass::drawCalibrationLine(QPainter * painter, QList<CurveDataInfo> curveData)
{
    //如果曲线没有点，不必绘制
    if (curveData.empty())
    {
        return;
    }

    //设置绘制属性
    QPen pen;
    pen.setColor(Qt::blue);
    pen.setWidth(0);
    painter->save();
    painter->setPen(pen);
    QPointF tmpPointer;
    QPainterPath  path;

    //绘制影子路径
    QPainterPath  pathShadow;
    QLinearGradient linear;
    linear.setStart(QPointF(0,0));
    linear.setFinalStop(QPointF(0, m_YaAis));
    linear.setColorAt(1, Qt::white);

    //绘制每一个点
    bool runFlag = false;
    for (CurveDataInfo var : curveData)
    {
        if (runFlag == false)
        {
            //影子的路径
            pathShadow.moveTo(var.xPos, m_YaAis-m_scaleInfo.height);
            pathShadow.lineTo(var.xPos, var.yPos);

            //正常的曲线的路径
            path.moveTo(var.xPos, var.yPos);
            runFlag = true;
            continue;
        }

        //设置颜色
        QColor colorUse;
        if (var.color >= m_colorList.size())
        {
            colorUse = m_colorList[0];
        }
        else
        {
            colorUse = m_colorList[var.color];
        }

        //设置影子的颜色
        linear.setColorAt(0, colorUse);
        //设置画笔的颜色
        pen.setColor(colorUse);
        painter->setPen(pen);

        tmpPointer.setX(var.xPos);
        tmpPointer.setY(var.yPos);

        //使用QpaiterPath来绘制
        path.lineTo(var.xPos, var.yPos);
        //添加路径点到影子中
        pathShadow.lineTo(var.xPos, var.yPos);
    }

    //最后一个点特殊处理
    pathShadow.lineTo(curveData.last().xPos, m_YaAis - m_scaleInfo.height);
    //做具体的绘制
    painter->setRenderHint(QPainter::Antialiasing, true);		//防锯齿
    painter->drawPath(path);
    //绘制影子
    pathShadow.setFillRule(Qt::FillRule::OddEvenFill);
    painter->setOpacity(0.3);
    //painter->fillPath(pathShadow, linear);

    painter->restore();
}

///
/// @brief
///   找到曲线中x和y的最大值和最小值
///
/// @param[in]  xMax  x最大值
/// @param[in]  xMin  x最小值
/// @param[in]  yMax  y最大值
/// @param[in]  yMin  y最小值
/// @param[in]  curveListData  曲线的列表
///
/// @par History:
/// @li 5774/WuHongTao，2020年6月11日，新建函数
///
void CScatterPlotContentClass::findMaxAndMin(qreal& xMax, qreal& xMin, qreal& yMax, qreal& yMin, const allCurveDataStore& curveListData)
{
    //首先找到最大值和最小值，便于计算坐标系{坐标系根据曲线的值来变动}
    for (curveSingleDataStore singleData : curveListData)
    {
        for (RealDataStruct var : singleData.dataContent)
        {
            //找到最大和最小的x的值
            if (xMax < var.xData)
            {
                xMax = var.xData;
            }

            if (var.xData < xMin)
            {
                xMin = var.xData;
            }

            //找到最大或者是最小的y值
            if (yMax < var.yData)
            {
                yMax = var.yData;
            }

            if (var.yData < yMin)
            {
                yMin = var.yData;
            }
        }
    }
}

///
/// @brief
///     更新曲线的状态
///
/// @par History:
/// @li 5774/WuHongTao，2020年7月14日，新建函数
///
void CScatterPlotContentClass::updateCurveInfo()
{
    updateCurveData(m_dataContent);
}

///
/// @brief 鼠标响应事件
///     
///
/// @param[in]  event  
/// @par History:
/// @li 5774/WuHongTao，2020年4月21日，新建函数
///
void CScatterPlotContentClass::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsObject::mousePressEvent(event);
    return;
}

///
/// @brief
///     鼠标释放事件
///
/// @param[in]  event  
///
/// @par History:
/// @li 5774/WuHongTao，2020年6月4日，新建函数
///
void CScatterPlotContentClass::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsObject::mouseReleaseEvent(event);
    return;
}

///
/// @brief
///     鼠标移动事件
///
/// @param[in]  event  
///
/// @par History:
/// @li 5774/WuHongTao，2020年6月4日，新建函数
///
void CScatterPlotContentClass::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    return;
}

///
/// @brief
///     鼠标双击事件
///
/// @param[in]  event  
///
/// @par History:
/// @li 5774/WuHongTao，2020年6月4日，新建函数
///
void CScatterPlotContentClass::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    return;
}
