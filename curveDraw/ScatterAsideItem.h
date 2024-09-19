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

#pragma once
#include "qgraphicsitem.h"
#include "common.h"

class QPainter;
class QGraphicsSceneMouseEvent;

class CScatterAsideItem :
    public QGraphicsObject
{
    Q_OBJECT
public:

    ///
    /// @brief
    ///     小控件的构造函数
    ///
    /// @param[in]  scaleInfo  
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年6月10日，新建函数
    ///
    CScatterAsideItem(CoordinateDataType scaleInfo);

    ///
    /// @brief
    ///     小控件的析构函数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年6月10日，新建函数
    ///
    ~CScatterAsideItem();

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
    virtual QRectF boundingRect() const;

    ///
    /// @brief 设置散列点样式
    ///     
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月14日，新建函数
    ///
    virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

    ///
    /// @brief 设置坐标系的属性
    ///     
    ///
    /// @param[in]  scaleInfo  属性值的结构体
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月21日，新建函数
    ///
    void setAttribute(CoordinateDataType scaleInfo);

    ///
    /// @brief
    ///     更新曲线的点信息
    ///  
    /// @param[in]  ScatterPointer  曲线点信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年6月4日，新建函数
    ///
    void refreshCurvePointer(QMap<int, QList<CurveDataInfo>>& ScatterPointer);

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
    void setScale(double wscale, double hscale);

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
    ///     拖动曲线信号
    ///
    /// @param[in]  posDistance  
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年11月24日，新建函数
    ///
    void dragCurveSignal(qreal& posDistance);

    ///
    /// @brief
    ///     通知使用者选中的点的list
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年6月5日，新建函数
    ///
    void CurveSelectCome();
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

    ///
    /// @brief
    ///     鼠标释放事件
    ///
    /// @param[in]  event  
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年6月4日，新建函数
    ///
    void mouseReleaseEvent(QGraphicsSceneMouseEvent  *event);

    ///
    /// @brief
    ///     鼠标移动事件
    ///
    /// @param[in]  event  
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年6月4日，新建函数
    ///
    void mouseMoveEvent(QGraphicsSceneMouseEvent  *event);

    ///
    /// @brief
    ///     鼠标双击事件
    ///
    /// @param[in]  event  
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年6月4日，新建函数
    ///
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent  *event);

    ///
    /// @brief
    ///     鼠标悬浮事件
    ///
    /// @param[in]  event  
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年6月10日，新建函数
    ///
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);

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
    bool IsSelected(const QPointF point);
private:

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
        void drawExpressLine(QPainter * painter, QPointF point, int position);

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
        void drawRectText(QPainter * painter, QPointF point);

        ///
        /// @brief
        ///     当曲线发生变化的时候，更新focus的位置信息
        ///
        /// @par History:
        /// @li 5774/WuHongTao，2020年6月16日，新建函数
        ///
        void updateFocusInfo();
private:
    CoordinateDataType              m_scaleInfo;    //坐标系的信息
    QRectF                          m_rect;//阴影面积
    qreal                           m_startPos;
    bool                            m_selected;//选中
    QMap<int, QList<CurveDataInfo>> m_pointerList;//曲线点信息
    CurveDataInfo                   m_focuspointer;//关注点信息
    bool                            m_needRefresh;//是否需要刷新选中点
    double							m_wscale;//屏幕的比例--宽度
    double							m_hscale;//屏幕的比例--高度
    QList<CurveDataInfo>            m_selectPointerSeries;//选中的点的
};

