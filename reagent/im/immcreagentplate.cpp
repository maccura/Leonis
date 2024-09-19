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
/// @file     mcreagentplate.cpp
/// @brief    试剂盘控件
///
/// @author   4170/TangChuXian
/// @date     2020年4月17日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月17日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "immcreagentplate.h"
#include <QRect>
#include <QGraphicsSceneMouseEvent>
#include "immcreagentplateitem.h"
#include "immctubeslot.h"

// 索引
#define INVALID_INDEX             (-1)                                    // 无效索引

// 长度
#define INNER_CIRCLE_RADIA        (333)                                   // 内圆半径
#define CIRCLE_RING_WIDTH         (32)                                    // 圆环宽度
#define TUBE_SLOT_AREA_WIDTH      (44)                                    // 试管槽区域宽度

// 绘制区域
#define REAGENT_PLATE_RECT        QRect(0, 0, 701, 701)                   // 试剂盘绘制区域
#define FIRST_REAGENT_SLOT_POINT  QPoint(328, 17)                         // 第一个槽位绘制区域
#define REAGENT_PLATE_TEXT_RECT   QRect(271, 244, 163, 163)               // 试剂盘中心文本绘制区域

// 字体
#define FONT_FAMILLY              QString("思源黑体")                     // 字体族
#define FONT_SIZE                 (10)                                    // 字体大小
#define FONT_WIGHT                QFont::Normal                           // 字体粗细
#define FONT_COLOR                QColor(0, 0, 0)                         // 字体颜色

///
/// @brief
///     构造函数
///
/// @param[in] iCount  试剂盘槽位总数
/// @param[in] fDegree 试剂盘偏移角度
/// @param[in] parent  父窗口指针
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月14日，新建函数
///
ImMcReagentPlate::ImMcReagentPlate(int iCount, qreal fDegree, QWidget* parent)
    : QGraphicsView(parent),
      m_iTubeSlotCnt(iCount),
      m_iSelIndex(INVALID_INDEX)
{
    // 设置窗口透明
    setAttribute(Qt::WA_TranslucentBackground, true);
    setStyleSheet("border:none;background:transparent;");

    // 画槽位
    double fAngleStep = 360 / iCount;

    // 构造
    QFont textFont;
    textFont.setFamily(FONT_FAMILLY);
    textFont.setPointSize(FONT_SIZE);
    textFont.setWeight(FONT_WIGHT);

    // 创建底盘item
    m_pReagentPlateItem = new ImMcReagentPlateItem();
    m_pReagentPlateItem->SetSlotCnt(iCount);
    m_pReagentPlateItem->SetReagentPlateRect(REAGENT_PLATE_RECT);
    m_pReagentPlateItem->SetReagentPlateTextRect(REAGENT_PLATE_TEXT_RECT);
    m_pReagentPlateItem->SetFont(textFont);
    //m_pReagentPlateItem->SetText(m_strSelDetail);
    m_pReagentPlateItem->SetTextColor(FONT_COLOR);
    m_scene.addItem(m_pReagentPlateItem);

    // 创建槽位item
    for (int i = 0; i < iCount; i++)
    {
        // 添加槽位
        ImMcTubeSlot* slotItem = new ImMcTubeSlot(-fAngleStep * i);
        slotItem->setPos(FIRST_REAGENT_SLOT_POINT);
        m_scene.addItem(slotItem);

        // 坐标系变换（平移，旋转）
        QTransform transform;
        transform.translate(TUBE_SLOT_AREA_WIDTH / 2, INNER_CIRCLE_RADIA);
        transform.rotate(-fAngleStep * i);
        transform.translate(-TUBE_SLOT_AREA_WIDTH / 2, -INNER_CIRCLE_RADIA);
        slotItem->setTransform(transform);

        // 添加到容器
        m_vTubeSlot.push_back(slotItem);
    }

    // 设置窗口大小
    resize(REAGENT_PLATE_RECT.width(), REAGENT_PLATE_RECT.height());

    // 设置场景大小
    m_scene.setSceneRect(REAGENT_PLATE_RECT);

    // 为视图设置场景
    setScene(&m_scene);

    // 旋转整个视图
    QTransform transform;
    transform.translate(REAGENT_PLATE_RECT.center().x(), REAGENT_PLATE_RECT.center().y());
    transform.rotate(fDegree);
    transform.translate(-REAGENT_PLATE_RECT.center().x(), -REAGENT_PLATE_RECT.center().y());
    setTransform(transform);

    // 为场景安装事件过滤器
    m_scene.installEventFilter(this);
}

ImMcReagentPlate::~ImMcReagentPlate()
{
}

///
/// @brief
///     获取试剂盘可容纳试管总数
///
/// @param
///
/// @return 返回试剂盘可容纳试管总数
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月14日，新建函数
///
int ImMcReagentPlate::GetTubeSlotCount()
{
    return m_iTubeSlotCnt;
}

void ImMcReagentPlate::SetIndicator(const QSet<int>& selIndex)
{
    m_pReagentPlateItem->SetIndicatorSet(selIndex);
    UpdateBackNoVisible(selIndex);
    m_scene.update();
}

///
/// @brief
///     更新备用瓶编号显示
///
/// @param[in]  selIndex  指示器索引列表
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月4日，新建函数
///
void ImMcReagentPlate::UpdateBackNoVisible(const QSet<int>& selIndex)
{
    for (int iIndex = 1; iIndex <= m_vTubeSlot.size(); iIndex++)
    {
        auto it = selIndex.find(iIndex);
        m_vTubeSlot[iIndex - 1]->SetShowBackupNo(it != selIndex.end());
    }
}

///
/// @brief
///     设置选中项索引
///
/// @param[in] iSelIndex 选中项索引
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月14日，新建函数
///
void ImMcReagentPlate::SetSelIndex(int iSelIndex)
{
    // 判断索引值是否有效，有效则选中目标索引，取消其他索引选中
    if (iSelIndex > 0 && iSelIndex <= m_vTubeSlot.size() && m_iSelIndex != iSelIndex)
    {
        // 取消旧的选中项
        if (m_iSelIndex != INVALID_INDEX)
        {
            m_vTubeSlot[m_iSelIndex - 1]->SetChecked(false);
        }

        // 选中新的选中项
        m_vTubeSlot[iSelIndex - 1]->SetChecked(true);
        m_iSelIndex = iSelIndex;

        // 更新指示器显示
        SetIndicator({ iSelIndex });

        // 刷新场景
        m_scene.update();

        // 通知选中槽位索引改变
        emit selectedItemChange(m_iSelIndex);

        return;
    }

    // 相同则不处理
    if (m_iSelIndex == iSelIndex)
    {
        return;
    }

    // 取消索引
    if (m_iSelIndex != INVALID_INDEX)
    {
        m_vTubeSlot[m_iSelIndex - 1]->SetChecked(false);
        m_iSelIndex = INVALID_INDEX;
        m_scene.update();
    }

    // 更新指示器显示
    QSet<int> setIndicator;
    m_pReagentPlateItem->SetIndicatorSet(setIndicator);
    UpdateBackNoVisible(setIndicator);

    // 通知选中槽位索引改变
    emit selectedItemChange(INVALID_INDEX);
}

///
/// @brief
///     获取选中项索引
///
/// @param
///
/// @return 返回试剂盘选中位置的索引
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月14日，新建函数
///
int ImMcReagentPlate::GetSelIndex()
{
    return m_iSelIndex;
}

///
/// @brief
///     设置选中项详情
///
/// @param[in] strSelDetail 选中项详情
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月14日，新建函数
///
void ImMcReagentPlate::SetSelDetail(const QString& strSelDetail)
{
    m_strSelDetail = strSelDetail;
    m_pReagentPlateItem->SetText(m_strSelDetail);
}

///
/// @brief
///     获取选中项详情
///
/// @param
///
/// @return 返回选中位置的详细信息
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月14日，新建函数
///
QString ImMcReagentPlate::GetSelDetail()
{
    return m_strSelDetail;
}

///
/// @brief
///     装载试剂
///
/// @param[in] iIndex  装载试剂的槽位索引
/// @param[in] info    装载试剂的试剂信息
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月14日，新建函数
///
void ImMcReagentPlate::LoadReagent(int iIndex, const ImReagentInfo& info)
{
    // 判断索引值是否有效，有效则装载试剂
    if (iIndex > 0 && iIndex <= m_vTubeSlot.size())
    {
        m_vTubeSlot[iIndex - 1]->LoadReagent(info);
        m_scene.update();
    }
}

///
/// @brief
///     获取试剂信息
///
/// @param[in] iIndex  装载试剂的槽位索引
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月30日，新建函数
///
const boost::optional<ImReagentInfo> ImMcReagentPlate::GetReagentInfo(int iIndex)
{
    // 判断索引值是否有效
    if (iIndex > 0 && iIndex <= m_vTubeSlot.size())
    {
        return m_vTubeSlot[iIndex - 1]->GetReagentInfo();
    }

    return boost::none;
}

///
/// @brief
///     卸载试剂
///
/// @param[in] iIndex  卸载试剂的槽位索引
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月14日，新建函数
///
void ImMcReagentPlate::UnloadReagent(int iIndex)
{
    // 判断索引值是否有效，有效则装载试剂
    if (iIndex > 0 && iIndex <= m_vTubeSlot.size())
    {
        m_vTubeSlot[iIndex - 1]->UnloadReagent();
        m_scene.update();
    }
}

///
/// @brief
///     重置试剂
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月18日，新建函数
///
void ImMcReagentPlate::ReseReagent()
{
    for (int i = 1; i <= GetTubeSlotCount(); i++)
    {
        // 重置所有试剂
        ImReagentInfo stuInfo;
        LoadReagent(i, stuInfo);
    }
}

void ImMcReagentPlate::resizeEvent(QResizeEvent *event)
{
    // 支持缩放
    fitInView(REAGENT_PLATE_RECT, Qt::KeepAspectRatio);

    // 基类处理
    QGraphicsView::resizeEvent(event);
}

///
/// @brief
///     事件过滤器（主要用于监听场景事件）
///
/// @param[in] obj    事件的目标对象
/// @param[in] event  事件对象
///
/// @return true表示事件已处理，false表示事件未处理
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月14日，新建函数
///
bool ImMcReagentPlate::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == &m_scene && event->type() == QEvent::GraphicsSceneMousePress)
    {
        QGraphicsSceneMouseEvent* graphicsEvent = dynamic_cast<QGraphicsSceneMouseEvent*>(event);
        if (graphicsEvent == Q_NULLPTR)
        {
            // 类型转换失败
            return false;
        }

        if (graphicsEvent->button() == Qt::LeftButton)
        {
            // 检测光标下是否有item，有则置为选中
            SetSelIndex(INVALID_INDEX);
            QList<QGraphicsItem*> clickedItemList = m_scene.items(graphicsEvent->scenePos());
            for(int i = 0; i < m_vTubeSlot.size(); i++) 
            {
                if (m_vTubeSlot[i]->type() != ImMcTubeSlot::ReagentSlotType || !clickedItemList.contains(m_vTubeSlot[i]))
                {
                    continue;
                }

                // 选中对应项
                SetSelIndex(i + 1);
                break;
            }
        }
    }

    // 更新场景并转发事件
    m_scene.update();
    return false;
}
