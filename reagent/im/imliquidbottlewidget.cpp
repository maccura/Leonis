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
/// @file     imliquidbottlewidget.cpp
/// @brief    储液瓶界面
///
/// @author   4170/TangChuXian
/// @date     2022年8月18日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2022年8月18日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "imliquidbottlewidget.h"
#include <QPainter>
#include <QBitmap>

// 声明静态成员函数
QMap<int, ImLiquidBottleWidget::BottleTypeInfo>    ImLiquidBottleWidget::sm_mapBottleTypeInfo;       // 储液瓶信息映射

ImLiquidBottleWidget::ImLiquidBottleWidget(QWidget *parent)
    : QWidget(parent),
      m_iBottleType(-1),
      m_dRemainRatio(0.0)
{

}

ImLiquidBottleWidget::~ImLiquidBottleWidget()
{
}

///
/// @brief
///     注册储液瓶类型信息
///
/// @param[in]  stuBottleTypeInfo  储液瓶类型信息
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年8月18日，新建函数
///
bool ImLiquidBottleWidget::RegisterBottleType(const BottleTypeInfo& stuBottleTypeInfo)
{
    // 参数检查
    if (!stuBottleTypeInfo.IsValid())
    {
        return false;
    }

    sm_mapBottleTypeInfo.insert(stuBottleTypeInfo.iBottleType, std::move(stuBottleTypeInfo));
    return true;
}

///
/// @brief
///     反注册储液瓶类型
///
/// @param[in]  iBottleType  储液瓶类型
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年8月18日，新建函数
///
bool ImLiquidBottleWidget::UnRegisterBottleType(int iBottleType)
{
    return (sm_mapBottleTypeInfo.remove(iBottleType) > 0);
}

///
/// @brief
///     设置储液瓶类型 
///
/// @param[in]  iBottleType  储液瓶类型
///
/// @par History:
/// @li 4170/TangChuXian，2022年8月18日，新建函数
///
bool ImLiquidBottleWidget::SetBottleType(int iBottleType)
{
    m_iBottleType = iBottleType;
    return true;
}

///
/// @brief
///     获取储液瓶类型
///
/// @return 储液瓶类型
///
/// @par History:
/// @li 4170/TangChuXian，2022年8月18日，新建函数
///
int ImLiquidBottleWidget::GetBottleType()
{
    return m_iBottleType;
}

///
/// @brief
///     设置液体余量
///
/// @param[in]  dRatio  1为满，0为空
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年8月18日，新建函数
///
bool ImLiquidBottleWidget::SetLiquidRemainRatio(double dRatio)
{
    m_dRemainRatio = dRatio;
    return true;
}

///
/// @brief
///     获取液体剩余量
///
/// @return 液体剩余量 1为满，0为空
///
/// @par History:
/// @li 4170/TangChuXian，2022年8月18日，新建函数
///
double ImLiquidBottleWidget::GetLiquidRemainRatio()
{
    return m_dRemainRatio;
}

///
/// @brief
///     设置余量文本
///
/// @param[in]  strRemainVol  余量文本
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月29日，新建函数
///
bool ImLiquidBottleWidget::SetRemainVolText(const QString& strRemainVol)
{
    m_strRemainVolText = strRemainVol;
    return true;
}

///
/// @brief
///     获取余量文本
///
/// @return 余量文本
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月29日，新建函数
///
const QString& ImLiquidBottleWidget::GetRemainVolText()
{
    return m_strRemainVolText;
}

///
/// @brief
///     重写重绘事件
///
/// @param[in]  event  事件
///
/// @par History:
/// @li 4170/TangChuXian，2022年8月18日，新建函数
///
void ImLiquidBottleWidget::paintEvent(QPaintEvent *event)
{
    // 基类处理
    QWidget::paintEvent(event);

    // 判断当前储液瓶类型是否有效
    if (m_iBottleType < 0)
    {
        return;
    }

    // 如果余量比例大于1.0，则置为1.0
    if (m_dRemainRatio > 1.0)
    {
        m_dRemainRatio = 1.0;
    }

    // 获取对应储液瓶类型信息
    auto it = sm_mapBottleTypeInfo.find(m_iBottleType);
    if (it == sm_mapBottleTypeInfo.end())
    {
        return;
    }

    // 设置窗口视口
    QPainter paint(this);
    paint.setWindow(0, 0, it.value().BottleImg.width(), it.value().BottleImg.height());
    paint.setViewport(0, 0, width(), height());

    // 根据剩余量计算液体区域
    QSize  liquidPixMapSize = it.value().LiquidImg.size();
    int    iMaskHeight = liquidPixMapSize.height() * (1 - m_dRemainRatio);

    // 如果储液瓶未放置，则直接画未放置文本
    if (!it.value().bPlaced)
    {
        // 设置字体(待完善)
        QFont textFont;
        QPen textPen(QColor(0x86, 0x86, 0x86, 127));
        // 设置自动换行
        QTextOption option(Qt::AlignHCenter | Qt::AlignVCenter);
        option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        textFont.setPixelSize(14);
        paint.setFont(textFont);
        paint.drawText(QRect(it.value().ptFillLiquidTopLeft.x(), it.value().ptFillLiquidTopLeft.y(),
            liquidPixMapSize.width(), liquidPixMapSize.height()), tr("未放置"), option);
        return;
    }

    // 构造余量图片
    QPixmap remainLiquidImg;
    QPoint  ptTopLeft(it.value().ptFillLiquidTopLeft.x(), it.value().ptFillLiquidTopLeft.y() + iMaskHeight);
    if (m_dRemainRatio > DBL_EPSILON)
    {
        remainLiquidImg = it.value().LiquidImg.copy(QRect(0, iMaskHeight, liquidPixMapSize.width(), liquidPixMapSize.height() - iMaskHeight));
    }

    // 画储液瓶
    paint.drawPixmap(QPoint(0, 0), it.value().BottleImg);
    paint.drawPixmap(ptTopLeft, remainLiquidImg);

    // 画余量文本
    if (m_strRemainVolText.isEmpty())
    {
        return;
    }

    // 设置字体(待完善)
    QFont textFont;
    QPen textPen(QColor(0x50, 0x50, 0x50));
    textFont.setPixelSize(14);
    paint.setFont(textFont);
    paint.drawText(it.value().ptFillLiquidTopLeft.x(), it.value().ptFillLiquidTopLeft.y(), 
        liquidPixMapSize.width(), liquidPixMapSize.height(), Qt::AlignCenter, m_strRemainVolText);
}
