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

#pragma once
#include <memory>
#include "common.h"
#include <boost/lexical_cast.hpp>
#include <QGraphicsObject>

class QGraphicsSceneMouseEvent;
class QPainter;
class CSharp;
class ScatterAsideItem;
class CoordinateBase;
class CScatterAsideItem;

using namespace std;


class CScatterPlotContentClass :
    public QGraphicsObject
{
    Q_OBJECT

public:
    ///
    /// @brief 构造函数，绘制曲线
    ///     
    /// @param[in]  coordinateClass  需要对应的坐标系的对象
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月24日，新建函数
    ///   
    CScatterPlotContentClass(CoordinateBase *coordinateClass, CScatterAsideItem* Item);

    ///
    /// @brief
    ///     析构函数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年5月29日，新建函数
    ///
    ~CScatterPlotContentClass();

    ///
    /// @brief 控件的响应范围
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
    /// @param[in]  markerShape  //散列点样式
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月14日，新建函数
    ///
    virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

    ///
    /// @brief 计算映射的点的位置
    ///     
    /// @param[in]  curveData  所有曲线的容器
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月21日，新建函数
    ///
    void updateCurveData(allCurveDataStore& curveData);

    ///
    /// @brief 判断点是否被选中了
    ///     
    /// @param[in]  point  点的位置
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月21日，新建函数
    bool IsSelected(const QPointF point);

    ///
    /// @brief 添加曲线的函数
    ///     
    /// @param[in]  data  具体的曲线的点阵list
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月21日，新建函数
    ///
    int addCurveData(curveSingleDataStore& data);

    ///
    /// @brief 删除对应id的1d曲线
    ///     
    /// @param[in]  curveId  具体的id号
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月24日，新建函数
    ///
    void removeCurve(int curveId);

    ///
    /// @brief 设置坐标系的属性
    ///     
    /// @param[in]  scaleInfo  属性值的结构体
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月21日，新建函数
    ///
    void setAttribute(CoordinateDataType scaleInfo);

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
    CoordinateDataType& GetAxisAttribute();

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
    ///     清除所有曲线
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年5月27日，新建函数
    ///
    void clearCurve();

signals:
    ///
    /// @brief 关注点
    ///     
    /// @param[in]  focusData 点的信息 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年5月19日，新建函数
    ///
    void focusPointer(RealDataStruct focusData);

public slots:
    ///
    /// @brief
    ///     更新曲线的状态
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年7月14日，新建函数
    ///
    void updateCurveInfo();

protected:
    ///
    /// @brief
    ///     鼠标点击事件的处理函数
    ///
    /// @param[in]  event  具体事件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年6月10日，新建函数
    ///
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

private:
    ///
    /// @brief 绘制关注线
    ///     
    /// @param[in]  painter  画笔
    /// @param[in]  point  中心点
    /// @param[in]  position  绘制的曲线的左右，暂时默认是左边
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月24日，新建函数
    ///
    void drawExpressLine(QPainter * painter, QPointF point, int position);

    ///
    /// @brief 绘制拟合曲线
    ///     
    /// @param[in]  painter  画笔
    /// @param[in]  curveData  拟合点
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月21日，新建函数
    ///
    void drawCalibrationLine(QPainter * painter, QList<CurveDataInfo> curveData);

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
    void findMaxAndMin(qreal& xMax, qreal& xMin, qreal& yMax, qreal& yMin, const allCurveDataStore& curveListData);

    ///
    /// @brief
    ///     初始化颜色表
    /// @par History:
    /// @li 5774/WuHongTao，2020年5月29日，新建函数
    ///
    void initalColorList();

private:
    int                             m_xaAis;                    //x轴长度
    int                             m_YaAis;                    //y轴长度
    int                             m_imageSize;                //点的大小
    CoordinateDataType              m_scaleInfo;                //坐标系的信息
    allCurveDataStore               m_dataContent;              //各条曲线的位置
    bool                            m_needRefresh;
    QPointF                         m_expressPointer;           //关注点的中心位置
    QList<CurveDataInfo>            m_scatterCurve;             //散列曲线的所有点
    QMap<int ,QList<CurveDataInfo>> m_allScatterPointer;        //所有散列曲线的点,和曲线编号
	QMap<int ,QList<CurveDataInfo>> m_calibrationCurve;         //校准曲线的点
    int                             m_curveCount;               //记录曲线数目和分别曲线中点的属性
    qreal                           m_minOutOfControlValue;     //失控点的最小位置
    qreal                           m_maxOutOfControlValue;     //失控点的最大位置
    CoordinateBase*                 m_coorDinateObject;         //坐标系
    CScatterAsideItem*              m_controlItem;              //小控件层
    int                             m_curveId;                  //标识每一条曲线id
	int								m_calibrateCurveLine;       //标识每一条校准线
	double							m_wscale;                   //屏幕的比例--宽度
	double							m_hscale;                   //屏幕的比例--高度
	double							m_scale;                    //本身控件的比例
	QVector<shared_ptr<CSharp>>		m_sharpList;                //形状的列表
	QVector<QColor>					m_colorList;                //颜色列表
	int								m_currentSelectPointerId;   //当前选中线对应的曲线的id
};

