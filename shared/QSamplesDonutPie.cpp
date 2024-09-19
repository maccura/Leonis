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
/// @file     QSamplesDonutPie.h
/// @brief 	  饼状图
///
/// @author   7656/zhang.changjiang
/// @date      2023年7月18日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年7月18日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QSamplesDonutPie.h"
#include "ui_QSamplesDonutPie.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/thrift/gen-cpp/statis_types.h"

#define RATE (1.414)                                             // 内圈最大正方形边长和内圈半径的比例
#define ROUND(X) (int((X) + 0.5))                                // 四舍五入
#define  FLOAT_IS_EQUAL(f1, f2) (std::abs((f1) - (f2)) < 1e-9)   // 浮点数相等判断

QSamplesDonutPie::QSamplesDonutPie(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::QSamplesDonutPie();
	ui->setupUi(this);
	InitBeforeShow();
}

QSamplesDonutPie::~QSamplesDonutPie()
{
	delete ui;
}

///
///  @brief 设置数据
///
///
///  @param[in]   data  数据列表
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
///
void QSamplesDonutPie::SetData(const QVector<double>& data)
{
	// 检查样本量数量是否大于数组，防止越界
	if (INDEX_MAX > data.size())
	{
		return;
	}
	m_pie->SetData(data);
	double total = m_pie->GetTotal();
	bool totalNumIsValid = (!FLOAT_IS_EQUAL(total, 0) || total < 0);

	ui->label_nor_num->setText(QString::number(data[INDEX_NORMAL_SAMPLE]));
    int normalRadio = ROUND(data[INDEX_NORMAL_SAMPLE] * 100.0 / total);
	ui->label_nor_per->setText(totalNumIsValid ? QString("%1%").arg(normalRadio) : "0%");

	ui->label_emer_num->setText(QString::number(data[INDEX_EMERGE_SAMPLE]));
    int emerRadio = ROUND(data[INDEX_EMERGE_SAMPLE] * 100.0 / total);
	ui->label_emer_per->setText(totalNumIsValid ? QString("%1%").arg(emerRadio) : "0%");

	ui->label_qc_num->setText(QString::number(data[INDEX_QC_SAMPLE]));
    int qcRadio = ROUND(data[INDEX_QC_SAMPLE] * 100.0 / total);
	ui->label_qc_per->setText(totalNumIsValid ? QString("%1%").arg(qcRadio) : "0%");

	ui->label_cali_num->setText(QString::number(data[INDEX_CALI_SAMPLE]));
	ui->label_cali_per->setText(totalNumIsValid ? QString("%1%").arg(100 - normalRadio - emerRadio - qcRadio) : "0%");
}

///
/// @brief  设置数据
///
/// @param[in]  pieData  饼状图数据
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
///
void QSamplesDonutPie::SetData(const::tf::SampleStatisPieData & pieData)
{
	ui->label_nor_num->setText(QString::number(pieData.totalNormalSamples));
	ui->label_nor_per->setText(QString("%1%").arg(pieData.totalNormalSamplesRatio));

	ui->label_emer_num->setText(QString::number(pieData.totalEmerSamples));
	ui->label_emer_per->setText(QString("%1%").arg(pieData.totalEmerSamplesRatio));

	ui->label_qc_num->setText(QString::number(pieData.totalQcSamples));
	ui->label_qc_per->setText(QString("%1%").arg(pieData.totalQcSamplesRatio));

	ui->label_cali_num->setText(QString::number(pieData.totalCaliSamples));
	ui->label_cali_per->setText(QString("%1%").arg(pieData.totalCaliSamplesRatio));

	QVector<double> vecPieData;
	vecPieData.append(pieData.totalNormalSamples);
	vecPieData.append(pieData.totalEmerSamples);
	vecPieData.append(pieData.totalQcSamples);
	vecPieData.append(pieData.totalCaliSamples);
	m_pie->SetData(vecPieData);
}

///
///  @brief 根据索引值修改数据
///
///
///  @param[in]   index  索引值
///  @param[in]   num    数据
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
///
void QSamplesDonutPie::SetDataByIndex(SAMPLE_INDEX index, const double num)
{

	if (INDEX_MAX > m_pie->Getdata().size())
	{
		return;
	}
	m_pie->SetDataByIndex(index,num);
	double per = num / m_pie->GetTotal() * 100.0;
	switch (index)
	{
	case QSamplesDonutPie::INDEX_NORMAL_SAMPLE:
	{
		ui->label_nor_num->setText(QString::number(num));
		ui->label_nor_per->setText(QString::number((int)per));
	}
	break;
	case QSamplesDonutPie::INDEX_EMERGE_SAMPLE:
	{
		ui->label_emer_num->setText(QString::number(num));
		ui->label_emer_per->setText(QString::number((int)per));
	}
	break;
	case QSamplesDonutPie::INDEX_QC_SAMPLE:
	{
		ui->label_qc_num->setText(QString::number(num));
		ui->label_qc_per->setText(QString::number((int)per));
	}
	break;
	case QSamplesDonutPie::INDEX_CALI_SAMPLE:
	{
		ui->label_cali_num->setText(QString::number(num));
		ui->label_cali_per->setText(QString::number((int)per));
	}
	break;
	default:
		break;
	}
	m_pie->SetDataByIndex(index, num);
}

void QSamplesDonutPie::SetColors(const QVector<QColor>& colors)
{
	m_pie->SetColors(colors);
}

///
///  @brief 显示前初始化
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月13日，新建函数
///
void QSamplesDonutPie::InitBeforeShow()
{
	ui->label_nor_icon->setPixmap(QPixmap(":/Leonis/resource/image/icon_normal_samples.png"));
	ui->label_emer_icon->setPixmap(QPixmap(":/Leonis/resource/image/icon_emer_samples.png"));
	ui->label_cali_icon->setPixmap(QPixmap(":/Leonis/resource/image/icon_cail.png"));
	ui->label_qc_icon->setPixmap(QPixmap(":/Leonis/resource/image/icon_qc.png"));

	m_pie = new QDonutPie();
	QVBoxLayout *vLayout = new QVBoxLayout(ui->plot);
	vLayout->setMargin(0);
	vLayout->addWidget(m_pie);
	ui->plot->setLayout(vLayout);

	//pie->SetMargin(40);
	m_pie->SetAutoAdaption(false);
	m_pie->SetOuterRadius(135);
	m_pie->SetByOuter(false);
	m_pie->SetInnerRadius(100);
	m_pie->SetCenterAutoAdaption(true);
}

QDonutPie::QDonutPie(QWidget * parent)
	: QWidget(parent)
	, m_iMargin(0)
	, m_iOuterRadius(0)
	, m_iInnerRadius(0)
	, m_dRate(0.8)
	, m_bAutoAdaption(true)
	, m_bSetByOuter(true)
	, m_bCenterAutoAdaption(true)
	, m_center(QPoint(0, 0))
	, m_colors({ QColor(27, 125, 245, 255), QColor(31, 181, 88, 255), QColor(124, 63, 209, 255), QColor(19, 191, 191, 255) })
{
	// 创建 QLabel 控件

	m_pWidget = new QWidget(this);
	//widget->setStyleSheet("background-color: rgb(0, 170, 127);");

	QVBoxLayout *verticalLayout;

	verticalLayout = new QVBoxLayout(m_pWidget);
	verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
	m_pOrderNumLabel = new QLabel("600000", m_pWidget);
	m_pOrderNumLabel->setObjectName(QStringLiteral("label_order_num"));
	m_pOrderNumLabel->setAlignment(Qt::AlignCenter);

	verticalLayout->addWidget(m_pOrderNumLabel);

	m_pOrderNumLabelText = new QLabel(tr("样本量"), m_pWidget);
	m_pOrderNumLabelText->setObjectName(QStringLiteral("label_orderNum_text"));
	m_pOrderNumLabelText->setAlignment(Qt::AlignCenter);

	verticalLayout->addWidget(m_pOrderNumLabelText);
}


void QDonutPie::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);

	
	// 设置外半径
	int outerRadius = m_bAutoAdaption ? qMin(width(), height()) / 2 - m_iMargin : m_iOuterRadius;

	// 设置内半径，控制圆环的宽度
	int innerRadius = m_bSetByOuter ? outerRadius * m_dRate : m_iInnerRadius;

	// 设置圆心
	QPoint center = m_bCenterAutoAdaption ? QPoint(width() / 2, height() / 2) : m_center;

	// 设置控件位置
	int widgetWidth = innerRadius * RATE;
	int widgetHeight = widgetWidth;
	m_pWidget->setGeometry(center.x() - (int)widgetWidth / 2 -30, center.y() - (int)widgetHeight / 2, widgetWidth + 60, widgetHeight);

	// 计算数据总和
	double total = GetTotal();
	m_pOrderNumLabel->setText(QString::number(total));

	// 如果没有数据
	if (total <= 0)
	{
		painter.setPen(Qt::gray);
		painter.setBrush(Qt::gray);
		painter.drawEllipse(center, outerRadius, outerRadius);
		painter.setPen(Qt::white);
		painter.setBrush(Qt::white);
		painter.drawEllipse(center, innerRadius, innerRadius);
		return;
	}

	// 如果只有一组数据
	if (IsSingleType())
	{
		int index = GetSingleTypeIndex();
		if (index < 0 || index > m_data.size())
		{
			return;
		}
		painter.setBrush(m_colors[index % m_colors.size()]);
		painter.drawEllipse(center, outerRadius, outerRadius);
		painter.setBrush(Qt::white);
		painter.drawEllipse(center, innerRadius, innerRadius);
		return;
	}

	// 如果有多组数据
	// 起始角度
	int startAngle = 0;
	QRect outerRect = QRect(center.x() - outerRadius, center.y() - outerRadius, outerRadius * 2, outerRadius * 2);
	QRect innerRect = QRect(center.x() - innerRadius, center.y() - innerRadius, innerRadius * 2, innerRadius * 2);
	for (int i = 0; i < m_data.size(); ++i) 
	{
		// 计算起始角度
		double angle = m_data[i] * 360.0 / total;
		int alen = i == m_data.size() - 1 ? 360 * 16 - int(startAngle * 16) : (angle * 16);

		// 绘制扇形
		painter.setBrush(m_colors[i % m_colors.size()]);
		painter.setPen(Qt::white);
		painter.drawPie(outerRect, int(startAngle * 16), alen);

		// 绘制内半径扇形，创建圆环效果
		// 设置内圈背景色
		painter.setBrush(Qt::white);
		painter.setPen(Qt::white);
		painter.drawPie(innerRect, int(startAngle * 16), alen);

		// 更新起始角度
		startAngle += angle;
	}
}

///
///  @brief 设置边距
///
///
///  @param[in]   iMargin  边距大小
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
///
void QDonutPie::SetMargin(const int iMargin)
{
	m_iMargin = iMargin;
}

///
///  @brief 设置外圈半径
///
///
///  @param[in]   iOuterRadius  外圈半径
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
///
void QDonutPie::SetOuterRadius(const int iOuterRadius)
{
	m_iOuterRadius = iOuterRadius;
}

///
///  @brief 设置内圈半径
///
///
///  @param[in]   iInnerRadius  内圈半径
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
///
void QDonutPie::SetInnerRadius(const int iInnerRadius)
{
	m_iInnerRadius = iInnerRadius;
}

///
///  @brief 设置饼状图是否根据窗口大小自适应
///
///
///  @param[in]   bAutoAdaption  （true 是 false 否）
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
///
void QDonutPie::SetAutoAdaption(const bool bAutoAdaption)
{
	m_bAutoAdaption = bAutoAdaption;
}

///
///  @brief 设置内圈半径按照外圈半径比例显示
///
///
///  @param[in]   bSetByOuter  
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
///
void QDonutPie::SetByOuter(const bool bSetByOuter)
{
	m_bSetByOuter = bSetByOuter;
}

///
///  @brief 设置内外半径的比例
///
///
///  @param[in]   rate  内外半径的比例
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
///
void QDonutPie::SetInnerRadiusRate(const double rate)
{
	m_dRate = rate;
}

///
///  @brief 设置圆心居中显示
///
///
///  @param[in]   bCenterAutoAdaption  （true 居中, false 手动设置）
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
///
void QDonutPie::SetCenterAutoAdaption(const bool bCenterAutoAdaption)
{
	m_bCenterAutoAdaption = bCenterAutoAdaption;
}

///
///  @brief 设置圆心
///
///
///  @param[in]   center  圆心
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
///
void QDonutPie::SetCenterPoint(const QPoint center)
{
	m_center = center;
}

///
///  @brief 设置数据
///
///
///  @param[in]   data  数据列表
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
///
void QDonutPie::SetData(const QVector<double>& data)
{
	m_data = data;
	this->update();
}

///
///  @brief 获取数据列表
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
///
QVector<double>& QDonutPie::Getdata()
{
	return m_data;
}

///
///  @brief 设置颜色
///
///
///  @param[in]   colors  颜色列表
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
///
void QDonutPie::SetColors(const QVector<QColor>& colors)
{
	m_colors = colors;
}

///
///  @brief 根据数据索引修改数据
///
///
///  @param[in]   index  数据索引
///  @param[in]   num    数据
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
///
void QDonutPie::SetDataByIndex(const int index, const double num)
{
	if (index < 0 || index >= m_data.size());
	{
		return;
	}
	m_data[index] = num;
}

///
///  @brief 获取数据总量
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
///
double QDonutPie::GetTotal()
{
	double total = 0;
	for (auto it : m_data)
	{
		total += it;
	}
	return total;
}

///
/// @brief  检查是否只有一组数据
///
///
/// @return true 只有一组 false 有多组数据
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年12月1日，新建函数
///
bool QDonutPie::IsSingleType()
{
	int typeNum = 0;
	for (auto it : m_data)
	{
		if (it > 0)
		{
			++typeNum;
		}
	}
	return (typeNum == 1 || typeNum == 0);
}

///
/// @brief  获取只有一组数据时的索引值
///
///
/// @return 有数据的索引值
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年12月1日，新建函数
///
int QDonutPie::GetSingleTypeIndex()
{
	for (int i = 0; i < m_data.size(); ++i)
	{
		if (m_data[i] > 0)
		{
			return i;
		}
	}
	return 0;
}
