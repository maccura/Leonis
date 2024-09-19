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

#pragma once

#include <QMap>
#include <QWidget>
#include <memory>
#include <QPainter>
#include <QPainterPath>

class ReactionDiskWidget : public QWidget
{
    Q_OBJECT

        // 圆环中心点X坐标
        Q_PROPERTY(double centerX READ CenterX WRITE SetCenterX DESIGNABLE true)
        // 圆环中心点Y坐标
        Q_PROPERTY(double centerY READ CenterY WRITE SetCenterY DESIGNABLE true)
        // 扇区个数
        Q_PROPERTY(int sectorCount READ SectorCount WRITE SetSectorCount DESIGNABLE true)
        // 扇区的间隔角度
        Q_PROPERTY(double sectorSpacingAngle READ SectorSpacingAngle WRITE SetSectorSpacingAngle DESIGNABLE true)
        // 扇区按钮的宽度
        Q_PROPERTY(double sectorClickedWidth READ SectorClickedWidth WRITE SetSectorClickedWidth DESIGNABLE true)
		// 扇区的宽度
		Q_PROPERTY(int sectorWidth READ SectorWidth WRITE SetSectorWidth DESIGNABLE true)
		// 扇区的高度
		Q_PROPERTY(int sectorHeight READ SectorHeight WRITE SetSectorHeight DESIGNABLE true)
        // 1号扇区的起始角度（三点钟方向为0度,逆时针为正）
        Q_PROPERTY(double startAngle READ StartAngle WRITE SetStartAngle DESIGNABLE true)
        // 最大圆半径
        Q_PROPERTY(double outR READ OutR WRITE SetOutR DESIGNABLE true)
        // 反应杯距离扇区的边距的度数
        Q_PROPERTY(double cupToSectorMargins READ CupToSectorMargins WRITE SetCupToSectorMargins DESIGNABLE true)
        // 杯子到外圈的距离
        Q_PROPERTY(double cupToOutRoundMargins READ CupToOutRoundMargins WRITE SetCupToOutRoundMargins DESIGNABLE true)
        // 单个扇区中的反应杯个数
        Q_PROPERTY(int cupNumInItem READ CupNumInItem WRITE SetCupNumInItem DESIGNABLE true)
        // 一个杯子所占大圆的角度
        Q_PROPERTY(double cupAngle READ CupAngle WRITE SetCupAngle DESIGNABLE true)
        // 杯号显示区域到外圆的距离
        Q_PROPERTY(int cupNumMargins READ CupNumMargins WRITE SetCupNumMargins DESIGNABLE true)
        // 杯号显示区域矩形的宽度
        Q_PROPERTY(double cupNumRectWidth READ CupNumRectWidth WRITE SetCupNumRectWidth DESIGNABLE true)
        // 杯号的字体
        Q_PROPERTY(QString cupNumFontFamily READ CupNumFontFamily WRITE SetCupNumFontFamily DESIGNABLE true)
        // 杯号的字体的大小
        Q_PROPERTY(int cupNumFontSize READ CupNumFontSize WRITE SetCupNumFontSize DESIGNABLE true)
        // 杯号未选中的字体颜色
        Q_PROPERTY(QColor cupNumColor READ CupNumColor WRITE SetCupNumColor DESIGNABLE true)
        // 杯号选中的字体颜色
        Q_PROPERTY(QColor cupNumSelectColor READ CupNumSelectColor WRITE SetCupNumSelectColor DESIGNABLE true)
  
    // 杯子的数据
    struct	CupItemData
    {
        int						cupNo = -1;			// 杯号
        int						status = 0;		    // 状态
        bool					isSelect = false;   // 是否选中
        QPoint					ps;					// 杯子的位置
    };

    // 扇区的数据
    struct PanItemData
    {
        QPoint					ps;					// 扇区位置
        QPainterPath			path;				// 扇区边缘路径
        bool				    isSelect = false;   // 是否选中
        QVector<CupItemData>	cupItemDatas;		// 扇区内杯子的数据
    };

public:
    ReactionDiskWidget(QWidget *parent = Q_NULLPTR);
    ~ReactionDiskWidget();

    double CenterX() { return m_centerPoint.x(); }
    void SetCenterX(double cx) { m_centerPoint.setX(cx); }
    double CenterY() { return m_centerPoint.y(); }
    void SetCenterY(double cy) { m_centerPoint.setY(cy); }
    int SectorCount() { return m_sectorCount; }
    void SetSectorCount(int sc) { m_sectorCount = sc; }
    double SectorSpacingAngle() { return m_sectorSpacingAngle; }
    void SetSectorSpacingAngle(double ssa) { m_sectorSpacingAngle = ssa; }
    double SectorClickedWidth() { return m_sectorClickedWidth; }
    void SetSectorClickedWidth(double scw) { m_sectorClickedWidth = scw; }
	int SectorWidth() { return m_sectorSize.width(); }
	void SetSectorWidth(int sw) { m_sectorSize.setWidth(sw); }
	int SectorHeight() { return m_sectorSize.height(); }
	void SetSectorHeight(int sh) { m_sectorSize.setHeight(sh); }
    double StartAngle() { return m_startAngle; }
    void SetStartAngle(double sa) { m_startAngle = sa; }
    double OutR() { return m_outR; }
    void SetOutR(double r) { m_outR = r; }
    double CupToSectorMargins() { return m_cupToSectorMargins; }
    void SetCupToSectorMargins(double ctsm) { m_cupToSectorMargins = ctsm; }
    double CupToOutRoundMargins() { return m_cupToOutRoundMargins; };
    void SetCupToOutRoundMargins(double ctm) { m_cupToOutRoundMargins = ctm; };
    int CupNumInItem() { return m_cupNumInItem; }
    void SetCupNumInItem(int cni) { m_cupNumInItem = cni; }
    double CupAngle() { return m_cupAngle; }
    void SetCupAngle(double ca) { m_cupAngle = ca; }
    double CupNumMargins() { return m_cupNumMargins; }
    void SetCupNumMargins(double cnm) { m_cupNumMargins = cnm; }
    double CupNumRectWidth() { return m_cupNumRectWidth; }
    void SetCupNumRectWidth(double cnrw) { m_cupNumRectWidth = cnrw; }
    QString CupNumFontFamily() { return m_cupNumFontFamily; }
    void SetCupNumFontFamily(QString cnff) { m_cupNumFontFamily = cnff; }
    int CupNumFontSize() { return m_cupNumFontSize; }
    void SetCupNumFontSize(int cnfs) { m_cupNumFontSize = cnfs; }
    QColor CupNumColor() { return m_cupNumColor; }
    void SetCupNumColor(QColor cnc) { m_cupNumColor = cnc; }
    QColor CupNumSelectColor() { return m_cupNumSelectColor; }
    void SetCupNumSelectColor(QColor cnsc) { m_cupNumSelectColor = cnsc; }

    ///
    /// @brief 设置当前选中的的杯子
    ///     
    /// @param[in]  cupNo  杯号
    ///
    /// @return 成功返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年8月14日，新建函数
    ///
    bool SetCupIsSelect(int cupNo);

    ///
    /// @brief 设置杯子状态
    ///     
    /// @param[in]  cupNo  杯号
    /// @param[in]  status  杯子状态 参考 ::tf::ReactionCupStatus
    ///
    /// @return 成功返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年8月14日，新建函数
    ///
    bool SetCupStatus(int cupNo, int status);

    
    ///
    /// @brief  获取当前选中扇区
    ///     
    /// @return 当前选中扇区
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年8月17日，新建函数
    ///
    int GetCurrentItemIndex();

	///
	/// @brief  初始化反应盘状态
	///     
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年11月16日，新建函数
	///
	void InitDiskStatus();

protected:

    void paintEvent(QPaintEvent *event) override;

    void mousePressEvent(QMouseEvent* event) override;

    void Init();

    // 获取扇区的边缘路径
    PanItemData GetFanItemPath(double itemStartAngle);

signals:

    // 当前选中的扇区改变
    void SelectItemChanged(int panItemNo);

     protected Q_SLOTS:
    // 	
    // void OnClicked();

public:
    QMap<int, PanItemData>						m_mapPanItemDatas;

    //动态属性
    QPoint                                      m_centerPoint;          // 圆心
    int                                         m_sectorCount;          // 扇区个数
    double                                      m_sectorSpacingAngle;   // 扇区的间隔角度
    double                                      m_sectorClickedWidth;   // 扇区按钮的宽度
	QSize										m_sectorSize;			// 扇区的尺寸（width和height）
    double                                      m_startAngle;           // 1号扇区的起始角度（三点钟方向为0度,逆时针为正）
    double                                      m_outR;                 // 最大圆半径
    double                                      m_cupToSectorMargins;   // 反应杯距离扇区的边距的度数
    int                                         m_cupNumInItem;         // 单个扇区中的反应杯个数
    double                                      m_cupToOutRoundMargins; // 杯子到外圈的距离
    double                                      m_cupAngle;             // 一个杯子所占大圆的角度
    double                                      m_cupNumMargins;        // 杯号显示区域到外圆的距离
    double                                      m_cupNumRectWidth;      // 杯号显示区域矩形的宽度
    QString                                     m_cupNumFontFamily;     // 杯号的字体
    int                                         m_cupNumFontSize;       // 杯号的字体的大小
    QColor                                      m_cupNumColor;          // 杯号未选中的字体颜色
    QColor                                      m_cupNumSelectColor;    // 杯号选中的字体颜色

    // 普通成员变量
    double                                      m_rotateAngle;          // 单个扇区的角度
    double                                      m_innerR;               // 内圈圆的半径
    double                                      m_cupSpacing;           // 反应杯的间距
};
