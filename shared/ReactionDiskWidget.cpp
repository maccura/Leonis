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

#include <QStyle>
#include <QPainter>
#include <QMouseEvent>
#include "ReactionDiskWidget.h"
#include "src/thrift/ch//gen-cpp/ch_types.h"

ReactionDiskWidget::ReactionDiskWidget(QWidget *parent)
    : QWidget(parent)
{
    Init();
}

ReactionDiskWidget::~ReactionDiskWidget()
{

}

bool ReactionDiskWidget::SetCupIsSelect(int cupNo)
{
    if (cupNo > 0 && cupNo <= m_mapPanItemDatas.last().cupItemDatas.last().cupNo)
    {
        for (auto& item : m_mapPanItemDatas)
        {
            for (auto& cup : item.cupItemDatas)
            {
                cup.isSelect = cup.cupNo == cupNo;
            }
        }
    }
    else
    {
        return false;
    }

    update();
    return true;
}

bool ReactionDiskWidget::SetCupStatus(int cupNo, int status)
{
    if (cupNo > 0 && cupNo <= m_mapPanItemDatas.last().cupItemDatas.last().cupNo)
    {
        for (auto& item : m_mapPanItemDatas)
        {
            for (auto& cup : item.cupItemDatas)
            {
                if (cup.cupNo == cupNo)
                {
                    cup.status = status;
                    break;
                }
            }
        }
    }
    else
    {
        return false;
    }

    update();
    return true;
}

///
/// @brief  初始化反应盘状态
///     
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年11月16日，新建函数
///
void ReactionDiskWidget::InitDiskStatus()
{
	for (auto& item : m_mapPanItemDatas)
	{
		// 全部未选中
		item.isSelect = false;

		// 杯子状态初始化
		for (auto& cup : item.cupItemDatas)
		{
			cup.isSelect = false;
			cup.status = 0;
		}
	}

	update();
}

int ReactionDiskWidget::GetCurrentItemIndex()
{
    for (auto iter = m_mapPanItemDatas.begin(); iter != m_mapPanItemDatas.end(); iter++)
    {
        if (iter->isSelect)
        {
            return iter.key();
        }
    }

    return 0;
}

ReactionDiskWidget::PanItemData ReactionDiskWidget::GetFanItemPath(double itemStartAngle)
{
    // 外圈圆的外切矩形位置
    double x = m_centerPoint.x() - m_outR;
    double y = m_centerPoint.y() - m_outR;

    // 外圈弧线起点
    QPainterPath painterPath;
    painterPath.arcMoveTo(x, y, m_outR * 2, m_outR * 2, itemStartAngle);

    PanItemData panItemData;
    // 反应盘位置
    panItemData.ps = painterPath.currentPosition().toPoint();

    //外圈弧线
    painterPath.arcTo(x, y, m_outR * 2, m_outR * 2, itemStartAngle, m_rotateAngle);

    // 内圈圆的外切矩形位置
    double innerX = m_centerPoint.x() - m_innerR;
    double innerY = m_centerPoint.y() - m_innerR;

    // 内圈圆弧

    painterPath.arcTo(innerX, innerY, m_innerR * 2, m_innerR * 2, itemStartAngle + m_rotateAngle, -m_rotateAngle);

    // 将路径连接起来
    painterPath.closeSubpath();

    // 反应盘边缘路径
    panItemData.path = painterPath;

    return std::move(panItemData);

}

void ReactionDiskWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    // 杯号圆的半径
    double cupNumR = m_outR - m_cupNumMargins;

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    for (int i = 0; i < m_sectorCount; i++)
    {
        painter.save();

        // 将坐标系原点移动到图片左上角
        painter.translate(m_mapPanItemDatas[i].ps);

        double itemStartAngle = (m_rotateAngle + m_sectorSpacingAngle)*i + m_startAngle;
        painter.rotate(-180 - itemStartAngle);

		// 扇区图形
		QImage sectorImage;
        if (m_mapPanItemDatas[i].isSelect)
        {
			sectorImage.load(":/Leonis/resource/image/ch/c1005/item-select.png");
        }
        else
        {
			sectorImage.load(":/Leonis/resource/image/ch/c1005/item.png");
		}
		painter.drawImage(0, 0, sectorImage.scaled(m_sectorSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
		

        painter.restore();

        // 绘制杯子
        for (int j = 0; j < m_cupNumInItem; j++)
        {
            painter.save();

            // 将坐标系原点移动到图片左上角
            auto& cupData = m_mapPanItemDatas[i].cupItemDatas[j];
            painter.translate(cupData.ps);
            painter.rotate(-180 - itemStartAngle - j * m_cupSpacing);

            QImage image;
            switch (cupData.status)
            {
                case ch::tf::ReactionCupStatus::CUP_STATUS_SPARE:
                    cupData.isSelect ? image.load(":/Leonis/resource/image/ch/c1005/cup-white-select.png") 
                        : image.load(":/Leonis/resource/image/ch/c1005/cup-white.png");
                    break;
                case ch::tf::ReactionCupStatus::CUP_STATUS_ADD_SAMPLE_OR_R1:
                    cupData.isSelect ? image.load(":/Leonis/resource/image/ch/c1005/cup-green-select.png")
                        : image.load(":/Leonis/resource/image/ch/c1005/cup-green.png");
                    break;
                case ch::tf::ReactionCupStatus::CUP_STATUS_ADD_R2:
                    cupData.isSelect ? image.load(":/Leonis/resource/image/ch/c1005/cup-yellow-select.png")
                        : image.load(":/Leonis/resource/image/ch/c1005/cup-yellow.png");
                    break;
                case ch::tf::ReactionCupStatus::CUP_STATUS_WAIT_CLEAN:
                    cupData.isSelect ? image.load(":/Leonis/resource/image/ch/c1005/cup-grey-select.png")
                        : image.load(":/Leonis/resource/image/ch/c1005/cup-grey.png");
                    break;
                case ch::tf::ReactionCupStatus::CUP_STATUS_DISABLE:
                    cupData.isSelect ? image.load(":/Leonis/resource/image/ch/c1005/cup-red-select.png")
                        : image.load(":/Leonis/resource/image/ch/c1005/cup-red.png");
                    break;
                default:
                    break;
            }

            painter.drawImage(0, 0, image);
            painter.restore();
        }

        // 绘制杯号
        painter.save();
        QPen pen;
        if (m_mapPanItemDatas[i].isSelect)
        {
            pen.setBrush(m_cupNumSelectColor);
        }
        else
        {
            pen.setBrush(m_cupNumColor);
        }

        painter.setPen(pen);
        painter.setFont(QFont(m_cupNumFontFamily, m_cupNumFontSize));

        QPainterPath painterPath;

        // 扇区起始杯号位置
        painterPath.arcMoveTo(m_centerPoint.x() - cupNumR, m_centerPoint.y() - cupNumR,
            cupNumR * 2, cupNumR * 2, itemStartAngle + m_cupSpacing * 2);
        QPoint p1 = painterPath.currentPosition().toPoint();

        // 绘制杯号
        painter.drawText(QRect(p1.x() - m_cupNumRectWidth / 2, p1.y() - m_cupNumRectWidth / 2, m_cupNumRectWidth, m_cupNumRectWidth),
            Qt::AlignCenter, QString::number(m_mapPanItemDatas[i].cupItemDatas[0].cupNo));

        // 扇区结束杯号位置
        painterPath.arcMoveTo(m_centerPoint.x() - cupNumR, m_centerPoint.y() - cupNumR, cupNumR * 2, cupNumR * 2,
            itemStartAngle + (m_cupNumInItem - 1) * m_cupSpacing);
        p1 = painterPath.currentPosition().toPoint();

        // 绘制杯号
        painter.drawText(QRect(p1.x() - m_cupNumRectWidth / 2, p1.y() - m_cupNumRectWidth / 2, m_cupNumRectWidth, m_cupNumRectWidth),
            Qt::AlignCenter, QString::number(m_mapPanItemDatas[i].cupItemDatas[m_cupNumInItem - 1].cupNo));

        painter.restore();
    }
}

void ReactionDiskWidget::mousePressEvent(QMouseEvent* event)
{
    int lastselect = -1;
    int selectIndex = -1;
    if (event->button() == Qt::LeftButton)
    {
        for (int i=0; i<m_mapPanItemDatas.size(); i++)
        {
            auto& item = m_mapPanItemDatas[i];

            // 缓存上次选中位置
            if (item.isSelect)
            {
                lastselect = i;
            }

            item.isSelect = item.path.contains(event->pos());

            if (item.isSelect)
            {
                selectIndex = i;
            }
        }
    }

    // 当前点击位置不在扇区中,不变更选中区域
    if ((selectIndex == -1) && (lastselect != -1))
    {
        m_mapPanItemDatas[lastselect].isSelect = true;
    }
    else
    {
        emit SelectItemChanged(selectIndex);
    }

    update();
}

void ReactionDiskWidget::Init()
{
    QFile styleFile(":/Leonis/resource/document/deviceView.qss");
    styleFile.open(QIODevice::ReadOnly | QIODevice::Text);
    if (styleFile.isOpen())
    {
        setStyleSheet(styleFile.readAll());
        style()->unpolish(this);
        style()->polish(this);
    }

    // 单个扇区的角度
    m_rotateAngle = (360.0 / m_sectorCount) - m_sectorSpacingAngle;
    // 内圈圆的半径
    m_innerR = m_outR - m_sectorClickedWidth;
    // 反应杯的间距
    m_cupSpacing = (m_rotateAngle - m_cupAngle - m_cupToSectorMargins * 2) / (m_cupNumInItem - 1);

    // 杯子所在圆的半径
    double cupR = m_outR - m_cupToOutRoundMargins;

    for (int i = 0; i < m_sectorCount; i++)
    {
        double itemStartAngle = (m_rotateAngle + m_sectorSpacingAngle)*i + m_startAngle;
        m_mapPanItemDatas[i] = GetFanItemPath(itemStartAngle);

        // 初始化杯子
        for (int j = 0; j < m_cupNumInItem; j++)
        {
            QPainterPath painterPath;
            painterPath.arcMoveTo(m_centerPoint.x() - cupR, m_centerPoint.y() - cupR,
                cupR * 2, cupR * 2, itemStartAngle + m_cupToSectorMargins + j * m_cupSpacing);

            CupItemData data;
            data.ps = painterPath.currentPosition().toPoint();
            data.cupNo = (i*m_cupNumInItem) + (j + 1);
            m_mapPanItemDatas[i].cupItemDatas.push_back(std::move(data));
        }
    }

    // 默认选中第一个item
    m_mapPanItemDatas[0].isSelect = true;

    emit SelectItemChanged(1);
}

