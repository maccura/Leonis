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
/// @file     mccustomqwtmarker.cpp
/// @brief    自定义悬浮提示标记
///
/// @author   4170/TangChuXian
/// @date     2023年7月7日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月7日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "mccustomqwtmarker.h"
#include "uidef.h"

#include <QObject>
#include <QFontMetrics>

///
/// @brief
///     质控指示符号
///
/// @param[in]  strConc     浓度
/// @param[in]  strTextList 文本列表
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月10日，新建函数
///
QcIndicateSymbol::QcIndicateSymbol(const QStringList& strTextList, int iAlign, int textHorAlign)
    : QwtSymbol(QwtSymbol::Path),
      m_ciFontSize(16),
      m_cstrTextList(strTextList)
{
    // 文本为空则不处理
    if (strTextList.isEmpty())
    {
        return;
    }

    // 设置颜色
    QColor symbolColor(0, 0, 0, 128);

    // 设置画笔
    QPen symbolPen(symbolColor);
    symbolPen.setJoinStyle(Qt::MiterJoin);
    setPen(symbolPen);
    setBrush(symbolColor);

    // 构造轨迹
    QPainterPath symbolPenPath;
    QPolygonF triAngle;
    triAngle << QPointF(0, -16) << QPointF(-8, -27) << QPointF(8, -27);
    symbolPenPath.addPolygon(triAngle);

    // 获取文本宽度
    m_font.setFamily(FONT_FAMILY);
    m_font.setPixelSize(m_ciFontSize);
    m_font.setBold(false);
    m_font.setWeight(QFont::Thin);
    QFontMetrics fontMtcs(m_font);
    int iTextWidth = 0;
    for (const QString& strText : strTextList)
    {
        if (fontMtcs.width(strText) > iTextWidth)
        {
            iTextWidth = fontMtcs.width(strText);
        }
    }

    // 根据气泡靠左还是靠右判断水平偏移量
    int iHOffset = 0;
    if (((iAlign & Qt::AlignTop) && (iAlign & Qt::AlignLeft)) ||
        ((iAlign & Qt::AlignBottom) && (iAlign & Qt::AlignRight)))
    {
        iHOffset = -iTextWidth / 2 + 8;
    }
    else if (((iAlign & Qt::AlignTop) && (iAlign & Qt::AlignRight)) ||
        ((iAlign & Qt::AlignBottom) && (iAlign & Qt::AlignLeft)))
    {
        iHOffset = iTextWidth / 2 - 8;
    }

    // 计算矩形气泡位置和尺寸
    const int ciHMergin = 9;
    const int ciVMergin = 17;
    const int ciVSpace = 8;
    const int ciTipFrameHeight = m_ciFontSize * strTextList.size() + (ciVSpace * (strTextList.size() - 1)) + (ciVMergin * 2);
    const int ciRadius = 4;
    QPainterPath rectTipPath;
    QRectF tipFrame(-iTextWidth / 2.0 - ciHMergin + iHOffset, -27 - ciTipFrameHeight + 1, iTextWidth + (2 * ciHMergin), ciTipFrameHeight);
    rectTipPath.addRoundedRect(tipFrame, ciRadius, ciRadius);

    // 旋转
    if (iAlign & Qt::AlignBottom)
    {
        QTransform trsf;
        trsf.rotate(180);
        symbolPenPath = trsf.map(symbolPenPath);
        rectTipPath = trsf.map(rectTipPath);
        tipFrame = trsf.mapRect(tipFrame);
    }

    // 添加文字
    QPen textPen(Qt::white, 1);
    setPen(textPen);
    int iTextIdx = 0;
    for (const QString& strText : strTextList)
    {
        // 文本起始偏移位置
        int offset = 0;

        // 获取文本坐标
        // 如果是居中则需要计算偏移起始位置
        if (textHorAlign == Qt::AlignCenter)
        {
            int textAreaWid = tipFrame.right() - tipFrame.left();
            int textWidth = fontMtcs.width(strText);
            offset = (textAreaWid - textWidth) / 2;
        }
        // 居左 (居右需要再实现)
        else
        {
            offset = ciHMergin;
        }

        // 暂时使用居中和第一行来判断是否加粗显示（校准-历史数据界面第一行文字有加粗需求）
        QPointF ptText(tipFrame.left() + offset, tipFrame.top() + ciVMergin + 14 + (iTextIdx * (m_ciFontSize + ciVSpace)));
        if (textHorAlign == Qt::AlignCenter && iTextIdx == 0)
        {
            m_font.setBold(true);
            m_font.setPixelSize(m_ciFontSize + 3);
        }
        else
        {
            m_font.setBold(false);
            m_font.setPixelSize(m_ciFontSize);
        }
        // 添加文本
        rectTipPath.addText(ptText, m_font, strText);

        // 索引自增
        ++iTextIdx;
    }

    // 设置轨迹
    setPath(symbolPenPath + rectTipPath);

    // 设置原点和尺寸
    setPinPoint(QPointF(0, 0));
    //setSize(tipFrame.width(), 85);
}

QcGraphicSelTipMaker::QcGraphicSelTipMaker() : m_enAlign(Qt::AlignTop | Qt::AlignHCenter), m_textAlign(Qt::AlignLeft)
{
    // 更新marker显示
    UpdateMakerDisplay();
}

///
/// @brief
///     设置质控时间
///
/// @param[in]  strTextList 文本列表
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月7日，新建函数
///
void QcGraphicSelTipMaker::SetQcTipContent(const QStringList& strTextList)
{
    m_strTextList = strTextList;
    UpdateMakerDisplay();
}

///
/// @brief
///     设置标签对齐方式
///
/// @param[in]  enAlignment  对齐方式
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月7日，新建函数
///
void QcGraphicSelTipMaker::setLabelAlignment(Qt::Alignment enAlignment, Qt::Alignment textAlign)
{
    m_enAlign = enAlignment;
    m_textAlign = textAlign;
    setSymbol(new QcIndicateSymbol(m_strTextList, enAlignment, textAlign));
}

///
/// @brief
///     更新标记曲线显示
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月10日，新建函数
///
void QcGraphicSelTipMaker::UpdateMakerDisplay()
{
//     // 构造qwtText
//     QString strQcResult = QObject::tr("结果：") + m_strSelPtQcConc;
//     QString strQcTime = QObject::tr("时间：") + m_strSelPtQcTime;
//     QString strContent = QString("<br>") + strQcResult + "<br>" + strQcTime + "<br>";
//     QString strRichContent = QString("<p align=\"left\" style=\"margin-left:10px;margin-right:10px;\">%1</p>");
//     QwtText markerText(strRichContent.arg(strContent));
//     QFont defFont = markerText.font();
//     defFont.setFamily(FONT_FAMILY);
//     defFont.setPixelSize(16);
//     defFont.setWeight(QFont::DemiBold);
//     markerText.setColor(QColor(0xff, 0xff, 0xff));
//     //markerText.setBorderPen(QPen(QColor(0, 0, 0, 127), 100));
//     markerText.setBorderRadius(4);
//     markerText.setFont(defFont);
//     markerText.setBackgroundBrush(QColor(0, 0, 0, 127));

    // 构造marker
    setRenderHint(QwtPlotItem::RenderAntialiased, true);
    setSymbol(new QcIndicateSymbol(m_strTextList,  m_enAlign, m_textAlign));
//    setSpacing(21);
//     setLabel(markerText);
//     setLabelAlignment(Qt::Alignment(m_enAlign));
}

///////////////////////////////////////////////////////////////////////////
/// @file     mccustomqwtmarker.h
/// @brief 	  统计分析提示消息气泡
///
/// @author   7656/zhang.changjiang
/// @date      2023年7月24日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年7月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////
StatisticsIndicateSymbol::StatisticsIndicateSymbol(const QStringList& strTextList, int iAlign, int textHorAlign)
	: QwtSymbol(QwtSymbol::Path),
	m_ciFontSize(16),
	m_cstrTextList(strTextList)
{
	// 文本为空则不处理
	if (strTextList.isEmpty())
	{
		return;
	}

	// 设置颜色
	QColor symbolColor(0, 0, 0, 128);

	// 设置画笔
	QPen symbolPen(symbolColor);
	symbolPen.setJoinStyle(Qt::MiterJoin);
	setPen(symbolPen);
	setBrush(symbolColor);

	// 构造轨迹
	QPainterPath symbolPenPath;
// 	QPolygonF triAngle;
// 	triAngle << QPointF(16, 0) << QPointF(27, -8) << QPointF(27, 8);
// 	symbolPenPath.addPolygon(triAngle);

	// 获取文本宽度
	m_font.setFamily(FONT_FAMILY);
	m_font.setPixelSize(m_ciFontSize);
	m_font.setBold(false);
	m_font.setWeight(QFont::Thin);
	QFontMetrics fontMtcs(m_font);
	int iTextWidth = 0;
	for (const QString& strText : strTextList)
	{
		if (fontMtcs.width(strText) > iTextWidth)
		{
			iTextWidth = fontMtcs.width(strText);
		}
	}

	// 根据气泡靠左还是靠右判断水平偏移量
	int iHOffset = 0;
	if (((iAlign & Qt::AlignTop) && (iAlign & Qt::AlignLeft)) ||
		((iAlign & Qt::AlignBottom) && (iAlign & Qt::AlignRight)))
	{
		iHOffset = -iTextWidth / 2 + 8;
	}
	else if (((iAlign & Qt::AlignTop) && (iAlign & Qt::AlignRight)) ||
		((iAlign & Qt::AlignBottom) && (iAlign & Qt::AlignLeft)))
	{
		iHOffset = iTextWidth / 2 - 8;
	}

	// 计算矩形气泡位置和尺寸
	const int ciHMergin = 9;
	const int ciVMergin = 17;
	const int ciVSpace = 8;
	const int ciTipFrameHeight = m_ciFontSize * strTextList.size() + (ciVSpace * (strTextList.size() - 1)) + (ciVMergin * 2);
	const int ciRadius = 4;
	QPainterPath rectTipPath;
	//QRectF tipFrame(-iTextWidth / 2.0 - ciHMergin + iHOffset, -27 - ciTipFrameHeight + 1, iTextWidth + (2 * ciHMergin) + 50, ciTipFrameHeight);
	QRectF tipFrame(27, -ciTipFrameHeight / 4 , iTextWidth + 2 * ciHMergin, ciTipFrameHeight);
	rectTipPath.addRoundedRect(tipFrame, ciRadius, ciRadius);

	// 旋转
// 	if (iAlign & Qt::AlignBottom)
// 	{
// 		QTransform trsf;
// 		trsf.rotate(180);
// 		symbolPenPath = trsf.map(symbolPenPath);
// 
// 		QTransform trsf1;
// 		trsf1.translate(-27 * 2 - iTextWidth - 2 * ciHMergin - 27, 0);
// 		rectTipPath = trsf1.map(rectTipPath);
// 		tipFrame = trsf1.mapRect(tipFrame);
// 	}

	// 添加文字
	QPen textPen(Qt::white, 1);
	setPen(textPen);
	int iTextIdx = 0;
	for (const QString& strText : strTextList)
	{
		// 文本起始偏移位置
		int offset = 0;

		// 获取文本坐标
		// 如果是居中则需要计算偏移起始位置
		if (textHorAlign == Qt::AlignCenter)
		{
			int textAreaWid = tipFrame.right() - tipFrame.left();
			int textWidth = fontMtcs.width(strText);
			offset = (textAreaWid - textWidth) / 2;
		}
		// 居右
		else if (textHorAlign == Qt::AlignRight)
		{
			int textAreaWid = tipFrame.right() - tipFrame.left();
			int textWidth = fontMtcs.width(strText);
			offset = textAreaWid - textWidth - ciHMergin;
		}
		// 居左
		else
		{
			offset = ciHMergin;
		}

		// 暂时使用居中和第一行来判断是否加粗显示（校准-历史数据界面第一行文字有加粗需求）
		QPointF ptText(tipFrame.left() + offset, tipFrame.top() + ciVMergin + 14 + (iTextIdx * (m_ciFontSize + ciVSpace)));

		if (textHorAlign == Qt::AlignCenter && iTextIdx == 0)
		{
			m_font.setBold(true);
			m_font.setPixelSize(m_ciFontSize + 3);
		}
		else
		{
			m_font.setBold(false);
			m_font.setPixelSize(m_ciFontSize);
		}
		// 添加文本
		rectTipPath.addText(ptText, m_font, strText);

		// 索引自增
		++iTextIdx;
	}

	// 设置轨迹
	setPath((symbolPenPath + rectTipPath).simplified());

	// 设置原点和尺寸
	setPinPoint(QPointF(0, 0));
	//setSize(tipFrame.width(), 85);
}


StatisticsGraphicSelTipMarker::StatisticsGraphicSelTipMarker()
{
	// 更新marker显示
	UpdateMakerDisplay();
}

///
///  @brief 设置文本
///
///
///  @param[in]   strTextList  文本列表
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月24日，新建函数
///
void StatisticsGraphicSelTipMarker::SetTipContent(const QStringList & strTextList)
{
	m_strTextList = strTextList;
	UpdateMakerDisplay();
}

///
/// @brief 设置标签对齐方式
///     
///
/// @param[in]  enAlignment  对齐方式
/// @param[in]  textAlign  文字在气泡中的对齐方式
///
/// @par History:
///  @li 7656/zhang.changjiang，2023年7月24日，新建函数
///
void StatisticsGraphicSelTipMarker::SetLabelAlignment(Qt::Alignment enAlignment, Qt::Alignment textAlign)
{
	m_enAlign = enAlignment;
	m_textAlign = textAlign;
	setSymbol(new StatisticsIndicateSymbol(m_strTextList, enAlignment, textAlign));
}

///
/// @brief
///     更新标记曲线显示
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月10日，新建函数
///
void StatisticsGraphicSelTipMarker::UpdateMakerDisplay()
{
	setRenderHint(QwtPlotItem::RenderAntialiased, true);
	setSymbol(new StatisticsIndicateSymbol(m_strTextList, m_enAlign, m_textAlign));
}
