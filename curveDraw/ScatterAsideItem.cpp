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
/// @file     ScatterAsideItem.h
/// @brief    小控件的绘制层
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
#include "ScatterAsideItem.h"
#include <QPainter>
#include "QPen"
#include <QGraphicsSceneMouseEvent> 

///
/// @brief
///     小控件的构造函数
///
/// @param[in]  scaleInfo  
///
/// @par History:
/// @li 5774/WuHongTao，2020年6月10日，新建函数
///
CScatterAsideItem::CScatterAsideItem(CoordinateDataType scaleInfo)
{
    m_scaleInfo = scaleInfo;
    m_wscale = 0.5;
    m_hscale = 0.5;
    m_needRefresh = false;
    //设置鼠标的移动事件可行
    setAcceptHoverEvents(true);
    m_selectPointerSeries.clear();
}

///
/// @brief
///     小控件的析构函数
///
/// @par History:
/// @li 5774/WuHongTao，2020年6月10日，新建函数
///
CScatterAsideItem::~CScatterAsideItem()
{
}

///
/// @brief 控件的响应范围
///     
///
///
/// @return 控件的响应范围,矩形对象
///
/// @par History:
/// @li 5774/WuHongTao，2020年4月24日，新建函数
///
QRectF CScatterAsideItem::boundingRect() const
{
    return QRectF(m_scaleInfo.width, 0, m_scaleInfo.xaAis - m_scaleInfo.startxDistance, m_scaleInfo.YaAis);
}

///
/// @brief 设置散列点样式
///     
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2020年4月14日，新建函数
///
void CScatterAsideItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget /*= 0*/)
{
    QPen pen;
    pen.setColor(Qt::black);
    pen.setWidth(0);
    painter->save();
    painter->setPen(pen);
    QPointF lastPointer;
    QPointF curentPointer;
    QPointF myLinkPointer;//当使用连线的时候

    //设置透明度,绘制阴影面积
    painter->setOpacity(0.2);
    painter->setBrush(Qt::green);
    {
        painter->drawRect(m_rect);
    }
    painter->restore();

    //绘制关注点
    painter->setRenderHint(QPainter::Antialiasing, true);
    if (m_needRefresh)
    {
        QPointF pointer;
        pointer.setX(m_focuspointer.xPos);
        pointer.setY(m_focuspointer.yPos);
        drawExpressLine(painter, pointer, 1);

        //只有当类型是0的时候，才绘制详细信息
//         if (m_scaleInfo.scaleType == 1)
//         {
//             drawRectText(painter, pointer);
//         }
    }
    return;
}

///
/// @brief 设置坐标系的属性
///     
///
/// @param[in]  scaleInfo  属性值的结构体
/// @par History:
/// @li 5774/WuHongTao，2020年4月21日，新建函数
///
void CScatterAsideItem::setAttribute(CoordinateDataType scaleInfo)
{
    m_scaleInfo = scaleInfo;
    update();
}

///
/// @brief
///     更新曲线的点信息
///  
/// @param[in]  ScatterPointer  曲线点信息
///
/// @par History:
/// @li 5774/WuHongTao，2020年6月4日，新建函数
///
void CScatterAsideItem::refreshCurvePointer(QMap<int, QList<CurveDataInfo>>& ScatterPointer)
{
    m_pointerList = ScatterPointer;
    updateFocusInfo();
}

///
/// @brief 鼠标响应事件
///     
///
/// @param[in]  event  
/// @par History:
/// @li 5774/WuHongTao，2020年4月21日，新建函数
///
void CScatterAsideItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_startPos = event->pos().x();
    m_rect.setCoords(0, 0, 0, 0);
    m_selected = true;
    update();
    //将事件继续往下传递
/*    QGraphicsObject::mousePressEvent(event);*/
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
void CScatterAsideItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    qreal distancenew = (event->pos().x() - m_startPos);
    qreal distance = abs(event->pos().x() - m_startPos);
    m_selectPointerSeries.clear();
    m_selected = false;
    emit dragCurveSignal(distancenew);

    //当距离很小的时候，不认为是在做选中操作
    if (distance <= m_scaleInfo.imageSize)
    {
        IsSelected(event->pos());
    }
    else
    {
        //ITEMSELCTED模式下面没有必要继续
        if (m_scaleInfo.selectMode == ITEMSELCTED)
        {
            return;
        }

        //如果不在曲线范围以内，直接返回
        if (!m_pointerList.contains(m_focuspointer.saveRawData.curveId))
        {
            return;
        }

        //找到需要计算的线
        qreal endPostion = event->pos().x();
        for (CurveDataInfo selectPointer : m_pointerList[m_focuspointer.saveRawData.curveId])
        {
            if (m_startPos < endPostion)
            {
                if ((selectPointer.xPos >= m_startPos) && (selectPointer.xPos <= endPostion))
                {
                    m_selectPointerSeries.append(selectPointer);
                }
            }
            else
            {
                if ((selectPointer.xPos >= endPostion) && (selectPointer.xPos <= m_startPos))
                {
                    m_selectPointerSeries.append(selectPointer);
                }
            }
        }
    }

    if (!m_selectPointerSeries.isEmpty())
    {
        //如若表中有数据，则通知应用程序取走
        emit CurveSelectCome();
    }
/*    QGraphicsObject::mouseReleaseEvent(event);*/
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
void CScatterAsideItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_scaleInfo.selectMode != ITEMSELCTED)
    {
        qreal x2 = event->pos().x();
        qreal y1 = m_scaleInfo.startyDistance;
        qreal y2 = m_scaleInfo.YaAis - m_scaleInfo.height;
        if (std::abs(x2 - m_startPos) > m_scaleInfo.imageSize / 2)
        {
            m_rect.setCoords(m_startPos, y1, x2, y2);
            update();
        }
    }
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
void CScatterAsideItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    return;
}

///
/// @brief
///     鼠标悬浮事件
///
/// @param[in]  event  
///
/// @par History:
/// @li 5774/WuHongTao，2020年6月10日，新建函数
///
void CScatterAsideItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_scaleInfo.selectMode != ITEMSELCTED)
    {
        IsSelected(event->pos());
    }
}

///
/// @brief 判断点是否被选中了
///     
///
/// @param[in]  point  点的位置
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2020年4月21日，新建函数
bool CScatterAsideItem::IsSelected(const QPointF point)
{
    bool ret = false;

    //在所有曲线中查找。选择的点是否在，绘制点附近{计算要落在范围内}
    for (QList<CurveDataInfo> curvePointerList : m_pointerList)
    {
        for (CurveDataInfo destPointer : curvePointerList)
        {
            //x轴的范围的计算
            if ((point.x() < (destPointer.xPos + m_scaleInfo.imageSize / 2)) && (point.x() > (destPointer.xPos - m_scaleInfo.imageSize / 2)))
            {
                //满足x轴范围的计算的同时，必须也满足Y轴的范围
                if ((point.y() < (destPointer.yPos + m_scaleInfo.imageSize / 2)) && (point.y() > (destPointer.yPos - m_scaleInfo.imageSize / 2)))
                {
                    m_focuspointer.xPos = (destPointer.xPos);
                    m_focuspointer.yPos = (destPointer.yPos);

                    m_focuspointer.saveRawData.curveId = destPointer.saveRawData.curveId;
                    //发出信号，告诉选中了那个数据
                    emit focusPointer(destPointer.saveRawData);
                    //刷新控制flag
                    m_needRefresh = true;
                    ret = true;
                    break;
                }
            }
        }
    }

    //根据结果确定是否需要刷新
    if (ret == false)
    {
        m_needRefresh = false;
    }
    update();
    return ret;
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
void CScatterAsideItem::setScale(double wscale, double hscale)
{
    m_wscale = wscale;
    m_hscale = hscale;
}

///
/// @brief
///     应用程序获取选中数列
///
/// @param[in]  selectList  选中的数列
///
/// @par History:
/// @li 5774/WuHongTao，2020年6月5日，新建函数
///
void CScatterAsideItem::getSelectPointerList(QList<CurveDataInfo>& selectList)
{
    //偷走
    selectList = std::move(m_selectPointerSeries);
}

///
/// @brief 绘制关注线
///     
///
/// @param[in]  painter  画笔
/// @param[in]  point  中心点
/// @param[in]  position  绘制的曲线的左右，暂时默认是左边

/// @par History:
/// @li 5774/WuHongTao，2020年4月24日，新建函数
///
void CScatterAsideItem::drawExpressLine(QPainter * painter, QPointF point, int position)
{
    QPen pen;
    pen.setColor(Qt::black);
    pen.setWidthF(0);

    int tmpCircleSize = m_scaleInfo.imageSize;
    painter->save();

    //绘制关注点的圆形范围
    painter->setPen(pen);
    painter->drawEllipse(point, tmpCircleSize / m_wscale, tmpCircleSize / m_hscale);

    //当类型是虚线的时候，不需要关注线
//     if (m_scaleInfo.scaleType == 1)
//     {
//         return;
//     }

    //关注线的左起点
    QPointF xStart(m_scaleInfo.startxDistance + m_scaleInfo.width, point.y());
    //关注线的左边的结束点
    QPointF pointLeft;
    pointLeft.setX(point.x() - tmpCircleSize / m_wscale);
    pointLeft.setY(point.y());
    painter->drawLine(xStart, pointLeft);

    //上关注点的起点和结束点的计算
    QPointF yStart(point.x(), m_scaleInfo.startyDistance);
    QPointF yEnd(point.x(), m_scaleInfo.YaAis - m_scaleInfo.height);
    QPointF pointUp;
    pointUp.setX(point.x());
    pointUp.setY(point.y() - tmpCircleSize / m_hscale);
    painter->drawLine(yStart, pointUp);

    //下关注点的开始点和结束点
    QPointF pointDown;
    pointDown.setX(point.x());
    pointDown.setY(point.y() + tmpCircleSize / m_hscale);
    painter->drawLine(pointDown, yEnd);
    painter->restore();
}

///
/// @brief
///     绘制实时数据，当鼠标在点的范围的时候
///
/// @param[in]  painter  绘制句柄
/// @param[in]  point  绘制位置
///
/// @par History:
/// @li 5774/WuHongTao，2020年6月5日，新建函数
///
void CScatterAsideItem::drawRectText(QPainter * painter, QPointF point)
{
    QRectF rect;
    QPen pen;
    QFont font;
    font.setPixelSize(4);
    pen.setColor(Qt::black);
    pen.setWidthF(0);

    //设置绘制字体的大小
    int tmpCircleSize = m_scaleInfo.imageSize;
    painter->save();
    painter->setFont(font);
    painter->setPen(pen);
    painter->setBrush(Qt::white);
    rect.setRect(point.x()+ tmpCircleSize/2, point.y()- 1*tmpCircleSize,4*tmpCircleSize / m_wscale, 2*tmpCircleSize / m_hscale);
    painter->drawRoundedRect(rect, 1.0, 1.0);
    painter->drawText(rect.x(), rect.y()+5, QString::number(m_focuspointer.saveRawData.yData));

    //设置绘制字体的时候，长方形的大小
    QPointF pointRight;
    QPointF pointLeft;
    pointLeft.setX(point.x() + tmpCircleSize / m_wscale);
    pointLeft.setY(point.y());

    //延长线指示线的绘制(上)
    pointRight.setX(point.x() + tmpCircleSize / 2);
    pointRight.setY(point.y() - 0.3 * tmpCircleSize);
    painter->drawLine(pointLeft, pointRight);

    //延长线指示线的绘制(下)
    pointRight.setX(point.x() + tmpCircleSize / 2);
    pointRight.setY(point.y() - 0.8 * tmpCircleSize);
    painter->drawLine(pointLeft, pointRight);

    painter->restore();
}

///
/// @brief
///     当曲线发生变化的时候，更新focus的位置信息
///
/// @par History:
/// @li 5774/WuHongTao，2020年6月16日，新建函数
///
void CScatterAsideItem::updateFocusInfo()
{
    //如果曲线已经被删除，对应的关注线也需要被删除
    if (m_pointerList.find(m_focuspointer.saveRawData.curveId) == m_pointerList.end())
    {
        m_needRefresh = false;
        return;
    }

    //如果曲线没有被删除，则重新找到新的位置
    for (CurveDataInfo everyPointer : m_pointerList[m_focuspointer.saveRawData.curveId])
    {
        if ((everyPointer.xPos == m_focuspointer.xPos)&&(everyPointer.yPos == m_focuspointer.yPos))
        {
            //更新选中点的位置
            m_focuspointer = everyPointer;
        }
    }
}
