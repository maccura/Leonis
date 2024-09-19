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

#include <QGraphicsObject>
#include <memory>
#include <QFlags>
#include <boost/optional.hpp>
#include <QFont>
#include <QGraphicsView>
#include <QGraphicsScene>

class QEvent;


class QProPertyWidget :public QWidget
{
    Q_OBJECT
    // 宽度
    Q_PROPERTY(int fan_widths READ WidthFans WRITE SetFanWidths DESIGNABLE true)
    // 高度
    Q_PROPERTY(int fan_heights READ HeightFans WRITE SetFanHeights DESIGNABLE true)
    // 上面点位置
    Q_PROPERTY(int fan_tops READ TopFans WRITE SetFanTops DESIGNABLE true)
    // 左边点位置
    Q_PROPERTY(int fan_lefts READ LeftFans WRITE SetFanLefts DESIGNABLE true)

    Q_PROPERTY(int fan_startXPointer READ FanStartXPointer WRITE SetFanStartXPointer DESIGNABLE true)
    Q_PROPERTY(int fan_startYPointer READ FanStartYPointer WRITE SetFanStartYPointer DESIGNABLE true)
    Q_PROPERTY(int fan_tranXPointer READ TranStartXPointer WRITE SetTranStartXPointer DESIGNABLE true)
    Q_PROPERTY(int fan_tranYPointer READ TranStartYPointer WRITE SetTranStartYPointer DESIGNABLE true)
    Q_PROPERTY(bool visable READ Visable WRITE SetVisable DESIGNABLE true)

    // 宽度
    Q_PROPERTY(int widths READ Widths WRITE SetWidths DESIGNABLE true)
    // 高度
    Q_PROPERTY(int heights READ Heights WRITE SetHeights DESIGNABLE true)
    // 上面点位置
    Q_PROPERTY(int tops READ Tops WRITE SetTops DESIGNABLE true)
    // 左边点位置
    Q_PROPERTY(int lefts READ Lefts WRITE SetLefts DESIGNABLE true)
    // 状态绘制宽度
    Q_PROPERTY(int statewidths READ StateWidths WRITE SetStateWidths DESIGNABLE true)
    // 状态绘制高度
    Q_PROPERTY(int stateheights READ StateHeights WRITE SetStateHeights DESIGNABLE true)
    // 状态绘制上面点位置
    Q_PROPERTY(int statetops READ StateTops WRITE SetStateTops DESIGNABLE true)
    // 状态绘制左边点位置
    Q_PROPERTY(int statelefts READ StateLefts WRITE SetStateLefts DESIGNABLE true)
    // 备用编号绘制宽度
    Q_PROPERTY(int sparewidths READ SpareWidths WRITE SetSpareWidths DESIGNABLE true)
    // 备用编号绘制高度
    Q_PROPERTY(int spareheights READ SpareHeights WRITE SetSpareHeights DESIGNABLE true)
    // 备用编号绘制上面点位置
    Q_PROPERTY(int sparetops READ SpareTops WRITE SetSpareTops DESIGNABLE true)
    // 备用编号绘制左边点位置
    Q_PROPERTY(int sparelefts READ SpareLefts WRITE SetSpareLefts DESIGNABLE true)

//     // 试剂瓶屏蔽状态图片
//     Q_PROPERTY(QString deactivity READ Deactivity WRITE SetDeactivity DESIGNABLE true)
//     // 试剂瓶警告状态图片
//     Q_PROPERTY(QString warningstate READ Warningstate WRITE SetWarningstate DESIGNABLE true)
//     // 试剂瓶提示状态图片
//     Q_PROPERTY(QString notestate READ Notestate WRITE SetNotestate DESIGNABLE true)
//     // 试剂瓶备用状态图片
//     Q_PROPERTY(QString sparestate READ Sparestate WRITE SetSparestate DESIGNABLE true)
//     // 试剂瓶在用状态图片
//     Q_PROPERTY(QString activity READ Activity WRITE SetActivity DESIGNABLE true)
//     // 试剂瓶空状态图片
//     Q_PROPERTY(QString emptystate READ Emptystate WRITE SetEmptystate DESIGNABLE true)
//     // 试剂瓶选中状态图片
//     Q_PROPERTY(QString selectstate READ Selectstate WRITE SetSelectstate DESIGNABLE true)
//     // 试剂瓶开放试剂图片
//     Q_PROPERTY(QString openstate READ Openstate WRITE SetOpenstate DESIGNABLE true)

    // 试剂盘宽度
    Q_PROPERTY(int platewidths READ PlateWidths WRITE SetPlateWidths DESIGNABLE true)
    // 试剂盘高度
    Q_PROPERTY(int plateheights READ PlateHeights WRITE SetPlateHeights DESIGNABLE true)
    // 试剂盘上面点位置
    Q_PROPERTY(int platetops READ PlateTops WRITE SetPlateTops DESIGNABLE true)
    // 试剂盘左边点位置
    Q_PROPERTY(int platelefts READ PlateLefts WRITE SetPlateLefts DESIGNABLE true)
    // 试剂盘偏移角度
    Q_PROPERTY(int shiftAngle READ ShiftAngle WRITE SetShiftAngle DESIGNABLE true)
    // 内圈试剂数目
    Q_PROPERTY(int innerNumber READ InnerNumber WRITE SetInnerNumber DESIGNABLE true)
    // 外圈试剂数目
    Q_PROPERTY(int outerNumber READ OuterNumber WRITE SetOuterNumber DESIGNABLE true)
    // 外圈半径
    Q_PROPERTY(int outerRadius READ OuterRadius WRITE SetOuterRadius DESIGNABLE true)
    // 尺度比例
    Q_PROPERTY(int scale READ Scale WRITE SetScale DESIGNABLE true)
    // 控件宽度，计算旋转
    Q_PROPERTY(int itemwidth READ ItemWidth WRITE SetItemWidth DESIGNABLE true)
    // 内圈起点x
    Q_PROPERTY(int innerStartXPointer READ InnerStartXPointer WRITE SetInnerStartXPointer DESIGNABLE true)
    // 内圈起点y
    Q_PROPERTY(int innerStartYPointer READ InnerStartYPointer WRITE SetInnerStartYPointer DESIGNABLE true)
    // 内圈起点x
    Q_PROPERTY(int outerStartXPointer READ OuterStartXPointer WRITE SetOuterStartXPointer DESIGNABLE true)
    // 内圈起点y
    Q_PROPERTY(int outerStartYPointer READ OuterStartYPointer WRITE SetOuterStartYPointer DESIGNABLE true)
public:
     QProPertyWidget(QWidget* parent = 0):QWidget(parent){};
    ~QProPertyWidget() {};

    int WidthFans() { return m_fanRect.width(); };
    int HeightFans() { return m_fanRect.height(); };
    int TopFans() { return m_fanRect.top(); };
    int LeftFans() { return m_fanRect.left(); };
    void SetFanWidths(int widths) { m_fanRect.setWidth(widths); };
    void SetFanHeights(int heights) { m_fanRect.setHeight(heights); };
    void SetFanTops(int tops) { m_fanRect.setTop(tops); };
    void SetFanLefts(int lefts) { m_fanRect.setLeft(lefts); };
    int FanStartXPointer() { return m_startPoint.x(); };
    int FanStartYPointer() { return m_startPoint.y(); };
    void SetFanStartXPointer(int startx) { m_startPoint.setX(startx); };
    void SetFanStartYPointer(int starty) { m_startPoint.setY(starty); };
    int TranStartXPointer() { return m_translatePoint.x(); };
    int TranStartYPointer() { return m_translatePoint.y(); };
    void SetTranStartXPointer(int startx) { m_translatePoint.setX(startx); };
    void SetTranStartYPointer(int starty) { m_translatePoint.setY(starty); };
    bool Visable() { return m_isVisable; };
    void SetVisable(bool visable) { m_isVisable = visable; };

    // 试剂瓶的绘制位置和大小
    int Widths(){ return m_bottleArea.width(); };
    int Heights() { return m_bottleArea.height(); };
    int Tops() { return m_bottleArea.top(); };
    int Lefts() { return m_bottleArea.left(); };
    void SetWidths(int widths) { m_bottleArea.setWidth(widths); };
    void SetHeights(int heights) { m_bottleArea.setHeight(heights); };
    void SetTops(int tops) { m_bottleArea.setTop(tops); };
    void SetLefts(int lefts) { m_bottleArea.setLeft(lefts); };

    // 试剂瓶状态的绘制位置
    int StateWidths() { return m_stateArea.width(); };
    int StateHeights() { return m_stateArea.height(); };
    int StateTops() { return m_stateArea.top(); };
    int StateLefts() { return m_stateArea.left(); };
    void SetStateWidths(int widths) { m_stateArea.setWidth(widths); };
    void SetStateHeights(int heights) { m_stateArea.setHeight(heights); };
    void SetStateTops(int tops) { m_stateArea.setTop(tops); };
    void SetStateLefts(int lefts) { m_stateArea.setLeft(lefts); };

    int SpareWidths() { return m_spareArea.width(); };
    int SpareHeights() { return m_spareArea.height(); };
    int SpareTops() { return m_spareArea.top(); };
    int SpareLefts() { return m_spareArea.left(); };
    void SetSpareWidths(int widths) { m_spareArea.setWidth(widths); };
    void SetSpareHeights(int heights) { m_spareArea.setHeight(heights); };
    void SetSpareTops(int tops) { m_spareArea.setTop(tops); };
    void SetSpareLefts(int lefts) { m_spareArea.setLeft(lefts); };

    int PlateWidths() { return m_plateArea.width(); };
    int PlateHeights() { return m_plateArea.height(); };
    int PlateTops() { return m_plateArea.top(); };
    int PlateLefts() { return m_plateArea.left(); };
    void SetPlateWidths(int width) { m_plateArea.setWidth(width); };
    void SetPlateHeights(int height) { m_plateArea.setHeight(height); };
    void SetPlateTops(int top) { m_plateArea.setTop(top); };
    void SetPlateLefts(int left) { m_plateArea.setLeft(left); };
    double ShiftAngle() { return m_shiftAngle; };
    void SetShiftAngle(double shiftAngle) { m_shiftAngle = shiftAngle; };
    int InnerNumber() { return m_innerNumber; };
    int OuterNumber() { return m_outerNumber; };
    void SetInnerNumber(int innerNumber) { m_innerNumber = innerNumber; };
    void SetOuterNumber(int outerNumber) { m_outerNumber = outerNumber; };
    int OuterRadius() { return m_outerRadius; };
    void SetOuterRadius(int outerRadius) { m_outerRadius = outerRadius; };
    int Scale() { return m_scale; };
    void SetScale(int scale) { m_scale = scale; };
    int ItemWidth() { return m_width; };
    void SetItemWidth(int width) { m_width = width; };
    int InnerStartXPointer() { return m_innerStartPointer.x(); };
    int InnerStartYPointer() { return m_innerStartPointer.y(); };
    int OuterStartXPointer() { return m_outerStartPointer.x(); };
    int OuterStartYPointer() { return m_outerStartPointer.y(); };
    void SetInnerStartXPointer(int innerStartx) { m_innerStartPointer.setX(innerStartx); };
    void SetInnerStartYPointer(int innerStarty) { m_innerStartPointer.setY(innerStarty); };
    void SetOuterStartXPointer(int outerStartx) { m_outerStartPointer.setX(outerStartx); };
    void SetOuterStartYPointer(int outerStarty) { m_outerStartPointer.setY(outerStarty); };
public:
    QRectF m_fanRect;               // 扇面绘制区域
    QPointF m_startPoint;           // 起点
    QPointF m_translatePoint;       // 转换点
    bool   m_isVisable;             // 是否可见
    int m_width;
    QRectF m_bottleArea;
    QRectF m_stateArea;
    QRectF m_spareArea;
    QRectF m_plateArea;
    double m_shiftAngle;
    int m_innerNumber;
    int m_outerNumber;
    int m_outerRadius;
    int m_scale;
    QPoint m_innerStartPointer;
    QPoint m_outerStartPointer;
};

class QPlateItem :public QGraphicsObject
{
    Q_OBJECT
public:
    // 控件类型
    enum class ItemType
    {
        BOTTLE,
        PLATE,
        FAN
    };

    QPlateItem(QGraphicsView *parent = Q_NULLPTR, ItemType type = ItemType::BOTTLE);
    virtual int rtti() const { return int(ItemType::BOTTLE); };
    virtual void SetPropertyWidget(const QProPertyWidget& property) = 0;

protected:
    virtual QRectF boundingRect() const  = 0;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) = 0;
private:
    QPlateItem::ItemType             m_type;                    ///< 类型
};

class QFanItem : public QPlateItem
{
    Q_OBJECT
public:
    QFanItem(QGraphicsView *parent = Q_NULLPTR, QPlateItem::ItemType type = ItemType::FAN);
    virtual int rtti() const { return int(ItemType::FAN); };
    void SetPropertyWidget(const QProPertyWidget& property);
protected:
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;

private:
    class PrivateData
    {
    public:
        QRectF   fanRect;               // 扇面绘制区域
        QPointF  startPoint;            // 起点
        QPointF  translatePoint;        // 转换点
        bool     isVisable;             // 是否可见
        PrivateData()
        {
//             fanRect = { 0,0,421,371 };
//             startPoint = {350,0};
//             translatePoint = { 35,385 };
//             isVisable = true;
        }
    };
    std::shared_ptr<PrivateData>    d_data;                    ///< 数据  
};

class QPlate : public QPlateItem
{
    Q_OBJECT
public:
    QPlate(QGraphicsView *parent = Q_NULLPTR, QPlateItem::ItemType type = ItemType::PLATE);
    virtual int rtti() const { return int(ItemType::PLATE); };
    bool SetPlateRect(const QRectF& plateRect);
    bool SetTextAttr(const boost::optional<QRectF>& textRect, boost::optional<QFont>& textfont, boost::optional<QColor>& textColor);
    bool SetTextContent(QString& text);
    void SetPropertyWidget(const QProPertyWidget& property);

protected:
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;

private:
    class PrivateData
    {
    public:
        QRectF   plateRect;             // 试剂盘绘制区域
        QRectF   textRect;              // 试剂详情绘制区域
        QFont    textFont;              // 字体
        QString  textContent;           // 文本内容
        QColor   textColor;             // 文本颜色
    };
    std::shared_ptr<PrivateData>    d_data;                    ///< 数据  
};

class QReagentItem : public QPlateItem
{
    Q_OBJECT
    Q_PROPERTY(QFlags<ReagentState> states READ State WRITE SetState )
    Q_PROPERTY(qreal angle READ Angle WRITE SetAngle)
    Q_PROPERTY(bool select READ Select WRITE SetSelect)
    Q_PROPERTY(QRectF area READ Area WRITE SetArea)
    Q_PROPERTY(qreal width READ Width WRITE SetWidth)

public:
    // 试剂状态
    enum class ReagentState
    {
        AVAILABLE = 0x0001,          // 可用
        SPARE     = 0x0002,          // 备用
        NOTICE    = 0x0004,          // 提醒
        WARNING   = 0x0008,          // 警告
        DEACTIVATE= 0x0010,          // 屏蔽
        OPEN      = 0x0020,          // 开放试剂
        SAMEGROUP = 0x0040,          // 同组标志
        NONE      = 0x0080,          // 空瓶
        EMPTY     = 0x0100,			 // 空位
		SCANFAIL  = 0x0200			 // 扫描失败
    };
    Q_DECLARE_FLAGS(ReagentStates, ReagentState)

    // 试剂信息
    struct ReagentInfo
    {
        ReagentStates states;        // 状态
        qreal        fAngle;        // 试剂偏移角度
        QRect        areaDraw;      // 试剂盘扇形绘制区域
        bool         bSelect;       // 是否被选中
        int          seqNumber;     // 备用序号
		bool		 isReagent;		// 是否是试剂

        // 初始化
        ReagentInfo()
        {
            states = ReagentState::EMPTY;
            fAngle = 0;
            areaDraw = QRect(0, 0, 0, 0);
            bSelect = false;
            seqNumber = 0;
			isReagent = true;
        }
    };

	// 试剂瓶图片
	struct Images
	{
		QImage selectImg;		// 瓶子选中图片
		QImage reagentImg;		// 试剂瓶图片
		QImage suppliesImg;		// 耗材瓶图片

		Images() {}

		// 构造函数
		Images(const QImage& img1, const QImage& img2, const QImage& img3)
			: selectImg(img1)
			, reagentImg(img2)
			, suppliesImg(img3)
		{
		}
	};

    QReagentItem(QGraphicsView *parent = Q_NULLPTR, QPlateItem::ItemType type = ItemType::BOTTLE);
    ~QReagentItem();
    void SetPropertyWidget(const QProPertyWidget& property);
    qreal Angle() const;
    void SetAngle(const qreal angle);
    QFlags<ReagentState> State() const;
    void SetState(const QFlags<ReagentState> states);
    bool Select() const;
    void SetSelect(bool select);
    boost::optional<ReagentInfo> ItemInfo() const;
    void SetItemInfo(const ReagentInfo& itemInfo);
    QRectF Area() const;
    bool SetArea(const QRectF& area);
    qreal Width();
    bool SetWidth(const qreal width);

protected:
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR);

private:
    class PrivateData
    {
    public:
        ReagentInfo reagentInfo;
        QRectF      area;
        QRectF      statearea;
        QRectF      textarea;
        qreal       width;
    };
    std::shared_ptr<PrivateData>                            d_data;                    ///< 数据
    std::shared_ptr<QProPertyWidget>                        m_propertyWidget;
	QMap<ReagentState, Images>								m_mapStateImages;          //< 缓存试剂瓶图片<状态,试剂瓶图片>
};

// 试剂盘视图
class QReagentView : public QGraphicsView
{
    Q_OBJECT

public:
    QReagentView(QWidget* parent = Q_NULLPTR);
    ~QReagentView();

    ///
    /// @brief 返回试剂界面的试剂总数
    ///
    ///
    /// @return 若有则，返回总数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年10月19日，新建函数
    ///
    boost::optional<int>  TotalCount();

    ///
    /// @brief 设置选中
    ///
    /// @param[in]  index  选中序号
    ///
    /// @return 设置成功则返回true
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年10月19日，新建函数
    ///
    bool SetIndex(int index);

    ///
    /// @brief 取消选中
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年10月19日，新建函数
    ///
    void CancelSelect();

    ///
    /// @brief 获取当前选中项目
    ///
    ///
    /// @return 当前选中编号，-1代表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年10月19日，新建函数
    ///
    int  Index();

    ///
    /// @brief 设置试剂盘显示信息
    ///
    /// @param[in]  strSelDetail  显示信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年10月19日，新建函数
    ///
    void SetPlateContent(const QString& strSelDetail);

    ///
    /// @brief 加载试剂
    ///
    /// @param[in]  index  试剂索引
    /// @param[in]  info   试剂信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年10月19日，新建函数
    ///
    void LoadReagent(int index, const QReagentItem::ReagentInfo& info);

    ///
    /// @brief 卸载试剂
    ///
    /// @param[in]  iIndex  试剂索引
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年10月19日，新建函数
    ///
    void UnloadReagent(int iIndex);

    ///
    /// @brief 获取对应位置的试剂信息
    ///
    /// @param[in]  index  索引
    ///
    /// @return 试剂信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年10月19日，新建函数
    ///
    boost::optional<QReagentItem::ReagentInfo> GetReagent(int index);

protected:
    void resizeEvent(QResizeEvent *event);

    ///
    /// @brief 事件过滤器
    ///
    /// @param[in]  obj    事件的目标对象
    /// @param[in]  event  事件对象
    ///
    /// @return true表示事件已处理，false表示事件未处理
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年10月19日，新建函数
    ///
    bool eventFilter(QObject *obj, QEvent *event);

    ///
    /// @brief 扇面旋转
    ///
    /// @param[in]  degree  旋转度数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年2月23日，新建函数
    ///
    void FanRotation(int degree);

Q_SIGNALS:
    ///
    /// @brief 选中索引发生改变
    ///     
    /// @param[in]  curItemIndex   当前选中项索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月16日，新建函数
    ///
    void indexChanged(int index);

private:
    class PrivateData
    {
    public:
        QFont       fontAttr;
        double      shiftAngle;
        int         outerNumber;
        int         innerNumber;
        int         outerRadius;
        int         innerRadius;
        QPoint      outerStartPointer;
        QPoint      innerStartPointer;
        QRectF      paintArea;
        qreal       width;
        QRectF      fanRect;               // 扇面绘制区域
        QPointF     startPoint;            // 起点
        QPointF     translatePoint;        // 转换点
        bool        isVisable;             // 是否可见
    };
    std::shared_ptr<PrivateData>                            d_data;                    ///< 数据

    int                     m_iTubeSlotCnt;       // 试管槽位总数
    int                     m_currentIndex;       // 选中项索引
    QString                 m_strSelDetail;       // 选中项详情
    QVector<QReagentItem*>  m_items;              // 试管槽位
    QPlate*                 m_plate;              // 试剂盘底盘
    QFanItem*               m_fan;                // 试剂盘扇面
    QGraphicsScene          m_scene;              // 内部场景
};
