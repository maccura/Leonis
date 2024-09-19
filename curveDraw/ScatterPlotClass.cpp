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

#include "ScatterPlotClass.h"
#include "ui_QtGuiClass.h"

#include "QResizeEvent"
#include "ScatterAsideItem.h"
#include "ScatterPlotSenceClass.h"
#include "CalibrateCoordinate.h"
#include "ScatterPlotViewClass.h"
#include "ScatterPlotContentClass.h"

///
/// @brief
///     绘图控件构造函数
///
/// @param[in]  parent  
///
/// @par History:
/// @li 5774/WuHongTao，2020年6月10日，新建函数
///
CScatterPlotClass::CScatterPlotClass(QWidget *parent)
    : QWidget(parent)
{
    ui = new Ui::QtGuiClass();
    ui->setupUi(this);
	//边框的距离
	int distance = 0;
    m_scene = new CScatterPlotSenceClass;
    //设置场景类的大小
    m_scene->setSceneRect(QRectF(0, 0, this->size().width()- distance, this->size().height()- distance));
    //坐标系的类的生成
    m_coorDinate = new CalibrateCoordinate(this->size().width()- distance, this->size().height()- distance);
    //小工具的生成
    m_aSideItem = new CScatterAsideItem(m_coorDinate->getCoordinateInformation());
    //设置透明度
    m_aSideItem->setOpacity(1);
    //绘图类的生成
    m_curveDraw = new CScatterPlotContentClass(m_coorDinate, m_aSideItem);
	//传递点击消息外部
	connect(m_aSideItem, SIGNAL(focusPointer(RealDataStruct)), this, SIGNAL(focusPointer(RealDataStruct)));
    connect(m_aSideItem, SIGNAL(CurveSelectCome()), this, SIGNAL(CurveSelectCome()));
    connect(m_aSideItem, SIGNAL(dragCurveSignal(qreal&)), m_coorDinate, SLOT(dragCurveMap(qreal&)));
    connect(m_coorDinate, &CalibrateCoordinate::yAxisMaxWidthChanged, this, [this](int width) {
        CoordinateDataType attr = m_curveDraw->GetAxisAttribute();
        if (attr.startxDistance != (width - attr.width))
        {
            attr.startxDistance = (width - attr.width);
            m_curveDraw->setAttribute(attr);
        }
    });
    //场景中加入坐标系控件
    m_scene->addItem(m_coorDinate);
    //加入小工具控件
    m_scene->addItem(m_aSideItem);
    //场景中加入绘图控件
    m_scene->addItem(m_curveDraw);
    //当坐标轴的状态改变的时候通知，曲线同步改变
    connect(m_coorDinate, SIGNAL(updateCurveData()), m_curveDraw, SLOT(updateCurveInfo()));

    //生成view对象
    m_view = new CScatterPlotViewClass(this);
    m_view->setMouseTracking(true);
    this->setMouseTracking(true);
    //view通知滚轮事件
    connect(m_view, SIGNAL(mouseWheelZoom(bool)), m_coorDinate, SLOT(wheelEvent(bool)));
    //设置对应的场景
    m_view->setScene(m_scene);
	//去掉滑动条
	m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	//设置窗口大小
    m_view->resize(this->size().width() - distance, this->size().height()- distance);
    //初始化
    initalPara();
}

///
/// @brief 初始化参数
///     
///
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2020年4月24日，新建函数
///
void CScatterPlotClass::initalPara()
{
	//初始化控件最开始的大小
	m_oldSize.setHeight(this->size().height());
	m_oldSize.setWidth(this->size().width());

    CoordinateDataType tmpSetData;
    tmpSetData.height = 20;//高度
    tmpSetData.tapeStrip = 10;//隔离栏的宽度
    tmpSetData.width = 25;//Y轴的刻度宽度
    tmpSetData.scaleNumber = 8;//显示的刻度数目
    tmpSetData.startxDistance = 0;//起点位置
    tmpSetData.startyDistance = 0;
    tmpSetData.scaleType = 0;
    tmpSetData.retainedNumber = 0;
    tmpSetData.needRefreshYScale = false;
    tmpSetData.maxYsacle = 4;
    tmpSetData.minYscale = -4;
    tmpSetData.xaAis = this->size().width();
    tmpSetData.xaAis =  this->size().height();
    tmpSetData.xListScale.append(1);
    tmpSetData.xListScale.append(2);
	tmpSetData.xListScale.append(3);
	tmpSetData.xListScale.append(4);
    tmpSetData.maxOutOfControlValue = INT_MAX;
    tmpSetData.minOutOfControlValue = INT_MIN;
    tmpSetData.unit = QString("SD");//单位
    tmpSetData.needRefreshXScale = false;
    tmpSetData.scaleNeed = true;
    tmpSetData.selectMode = ITEMSELCTED;
    m_curveDraw->setAttribute(tmpSetData);
}

void CScatterPlotClass::test(RealDataStruct focusData)
{

	emit this->focusPointer(focusData);
}

///
/// @brief 重绘事件
///     
///
/// @param[in]  event  
/// @par History:
/// @li 5774/WuHongTao，2020年4月24日，新建函数
///
void CScatterPlotClass::resizeEvent(QResizeEvent *event)
{
    m_view->resize(event->size().width(), event->size().height());
    float width = event->size().width();
    float height = event->size().height();
    float scaleWid = width / float(m_oldSize.width());
    float scaleheight = height / float(m_oldSize.height());

    //当为0的时候，直接返回不做处理
    if (scaleWid == 0 || scaleheight == 0)
    {
        return;
    }

	m_curveDraw->setScale(scaleWid, scaleheight);
    m_view->scale(scaleWid, scaleheight);
	m_oldSize = event->size();
}

///
/// @brief
///     绘图控件析构函数
///
/// @par History:
/// @li 5774/WuHongTao，2020年6月10日，新建函数
///
CScatterPlotClass::~CScatterPlotClass()
{
    //删除视口类
    if (m_view != NULL)
    {
        delete m_view;
    }

    //删除场景类
    if (m_scene != NULL)
    {
        delete m_scene;
    }
}

///
/// @brief 添加曲线的函数
///     
///
/// @param[in]  data  具体的曲线的点阵list
/// @par History:
/// @li 5774/WuHongTao，2020年4月21日，新建函数
///
int CScatterPlotClass::addCurveData(curveSingleDataStore& data)
{
    //增加曲线，并且范围曲线的Id
    if((m_curveDraw!= NULL) && (!data.dataContent.empty()))
    {
        return m_curveDraw->addCurveData(data);
    }
    //当曲线的句柄为空的时候
    else
    {
        return 0;
    }
}

///
/// @brief 移除曲线
///     
///
/// @param[in]  curveId  曲线Id来移除曲线
///
/// @par History:
/// @li 5774/WuHongTao，2020年4月24日，新建函数
///
void CScatterPlotClass::removeCurve(int curveId)
{
    if (m_curveDraw != NULL)
    {
        m_curveDraw->removeCurve(curveId);
    }
}

///
/// @brief 设置曲线属性
///     
///
/// @param[in]  scaleInfo  
///
/// @par History:
/// @li 5774/WuHongTao，2020年4月24日，新建函数
///
void CScatterPlotClass::setAttribute(CoordinateDataType scaleInfo)
{
    m_curveDraw->setAttribute(scaleInfo);
}

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
CoordinateDataType& CScatterPlotClass::GetCurveAttrbute()
{
    return m_curveDraw->GetAxisAttribute();
}

///
/// @brief
///     清除所有曲线
///
/// @par History:
/// @li 5774/WuHongTao，2020年5月27日，新建函数
///
void CScatterPlotClass::clearCurve()
{
	if (m_coorDinate != nullptr)
	{
		m_coorDinate->clearCurve();
	}
	if (m_curveDraw != nullptr)
	{
		m_curveDraw->clearCurve();
	}
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
void CScatterPlotClass::getSelectPointerList(QList<CurveDataInfo>& selectList)
{
    if (m_aSideItem)
    {
        m_aSideItem->getSelectPointerList(selectList);
    }
}

///
/// @brief 设置为默认视图比例
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月9日，新建函数
///
void CScatterPlotClass::setDefaultViewScale()
{
	m_coorDinate->setDefaultViewScale();
}
