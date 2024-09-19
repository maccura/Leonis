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

#include "QReagentItem.h"
#include <QPainter>
#include <QImage>
#include <QStyle>
#include <QGraphicsSceneMouseEvent>
#include <QFile>
#include <QEvent>

#define CIRCLEDEGREE 360
#define INVALID      (-1)                                    // 无效值

const QRectF onuse_rect = QRectF(15, 84, 11, 11);           // 在用图标●位置

QPlateItem::QPlateItem(QGraphicsView *parent /*= Q_NULLPTR*/, ItemType type /*= ItemType::BOTTLE*/)
{

}

QReagentItem::QReagentItem(QGraphicsView *parent, ItemType type)
    : QPlateItem(parent, type)
{
    d_data = std::make_shared<PrivateData>();

	// 屏蔽状态
	m_mapStateImages[ReagentState::DEACTIVATE] = {
		QImage(":/Leonis/resource/image/ch/c1005/reagentMaskSelect.png"),
		QImage(":/Leonis/resource/image/ch/c1005/masked.png"),
		QImage(":/Leonis/resource/image/ch/c1005/supply-masked.png") };

	// 空瓶状态
	m_mapStateImages[ReagentState::NONE] = {
		QImage(":/Leonis/resource/image/ch/c1005/reagentEmptyBottleSelect.png"),
		QImage(":/Leonis/resource/image/ch/c1005/empty-bottle.png") ,
		QImage(":/Leonis/resource/image/ch/c1005/supply-empty-bottle.png") };

	// 警告状态
	m_mapStateImages[ReagentState::WARNING] = {
		QImage(":/Leonis/resource/image/ch/c1005/reagentWarnSelect.png"),
		QImage(":/Leonis/resource/image/ch/c1005/warn.png"),
		QImage(":/Leonis/resource/image/ch/c1005/supply-warn.png") };

	// 提醒状态
	m_mapStateImages[ReagentState::NOTICE] = {
		QImage(":/Leonis/resource/image/ch/c1005/reagentNoteSelect.png"),
		QImage(":/Leonis/resource/image/ch/c1005/note.png"),
		QImage(":/Leonis/resource/image/ch/c1005/supply-note.png") };

	// 备用状态
	m_mapStateImages[ReagentState::SPARE] = {
		QImage(":/Leonis/resource/image/ch/c1005/reagentBackupSelect.png"),
		QImage(":/Leonis/resource/image/ch/c1005/backup.png"),
		QImage(":/Leonis/resource/image/ch/c1005/supply-backup.png") };

	// 在用状态
	m_mapStateImages[ReagentState::AVAILABLE] = {
		QImage(":/Leonis/resource/image/ch/c1005/reagentSelect.png"),
		QImage(":/Leonis/resource/image/ch/c1005/use.png"),
		QImage(":/Leonis/resource/image/ch/c1005/supply-use.png") };

	// 空状态
	m_mapStateImages[ReagentState::EMPTY] = {
		QImage(":/Leonis/resource/image/ch/c1005/reagentEmptySelect.png"),
		QImage(":/Leonis/resource/image/ch/c1005/empty.png"),
		QImage(":/Leonis/resource/image/ch/c1005/empty.png") };

	// 开放试剂
	m_mapStateImages[ReagentState::OPEN] = {
		QImage(":/Leonis/resource/image/ch/c1005/reagentSelect.png"),
		QImage(":/Leonis/resource/image/ch/c1005/lock.png"),
		QImage("") };

	// 扫描失败
	m_mapStateImages[ReagentState::SCANFAIL] = {
		QImage(":/Leonis/resource/image/ch/c1005/reagentWarnSelect.png"),
		QImage(":/Leonis/resource/image/ch/c1005/fail.png"),
		QImage(":/Leonis/resource/image/ch/c1005/fail.png") };
}

QReagentItem::~QReagentItem()
{
}

void QReagentItem::SetPropertyWidget(const QProPertyWidget& property)
{
    d_data->area = property.m_bottleArea;
    d_data->textarea = property.m_spareArea;
    d_data->statearea = property.m_stateArea;
}

qreal QReagentItem::Angle() const
{
    if (d_data != nullptr)
    {
        return d_data->reagentInfo.fAngle;
    }

    return 0;
}

void QReagentItem::SetAngle(const qreal angle)
{
    if (d_data == nullptr)
    {
        return;
    }

    d_data->reagentInfo.fAngle = angle;
    update();
}

QFlags<QReagentItem::ReagentState> QReagentItem::State() const
{
    if (d_data == nullptr)
    {
        return QFlags<QReagentItem::ReagentState>();
    }

    return d_data->reagentInfo.states;
}

void QReagentItem::SetState(const QFlags<ReagentState> states)
{
    if (d_data == nullptr)
    {
        return;
    }

    d_data->reagentInfo.states = states;
    update();
}

bool QReagentItem::Select() const
{
    if (d_data == nullptr)
    {
        return false;
    }

    return d_data->reagentInfo.bSelect;
}

void QReagentItem::SetSelect(bool select)
{
    if (d_data == nullptr)
    {
        return;
    }

    d_data->reagentInfo.bSelect = select;
    update();
}

boost::optional<QReagentItem::ReagentInfo> QReagentItem::ItemInfo() const
{
    if (d_data == nullptr)
    {
        return boost::none;
    }

    return d_data->reagentInfo;
}

void QReagentItem::SetItemInfo(const ReagentInfo& itemInfo)
{
    if (d_data == nullptr)
    {
        return;
    }

    d_data->reagentInfo = itemInfo;
    update();
}

QRectF QReagentItem::Area() const
{
    if (d_data == nullptr)
    {
        return QRectF();
    }

    return d_data->area;
}

bool QReagentItem::SetArea(const QRectF& area)
{
    if (d_data == nullptr)
    {
        return false;
    }

    d_data->area = area;
    update();
    return true;
}

qreal QReagentItem::Width()
{
    return d_data->width;
}

bool QReagentItem::SetWidth(const qreal width)
{
    d_data->width = width;
    update();
    return true;
}

QRectF QReagentItem::boundingRect() const
{
    if (d_data == nullptr)
    {
       return QRectF();
    }

   return d_data->area;
}

void QReagentItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget /*= Q_NULLPTR*/)
{
    if (painter == Q_NULLPTR || option == Q_NULLPTR 
        || d_data == Q_NULLPTR)
    {
        return;
    }

    // 反走样
    painter->setRenderHint(QPainter::Antialiasing, true);

	ReagentState rState = ReagentState::EMPTY;
	// 扫描失败
	if ((d_data->reagentInfo.states.testFlag(ReagentState::SCANFAIL)))
	{
		rState = ReagentState::SCANFAIL;
	}
    // 屏蔽状态
    else if ((d_data->reagentInfo.states.testFlag(ReagentState::DEACTIVATE)))
    {
		rState = ReagentState::DEACTIVATE;
    }
    // 空瓶状态
    else if ((d_data->reagentInfo.states.testFlag(ReagentState::NONE)))
    {
		rState = ReagentState::NONE;
    }
    // 警告状态
    else if ((d_data->reagentInfo.states.testFlag(ReagentState::WARNING)))
    {
		rState = ReagentState::WARNING;
    }
    // 提醒状态
    else if ((d_data->reagentInfo.states.testFlag(ReagentState::NOTICE)))
    {
		rState = ReagentState::NOTICE;
    }
    // 备用状态
    else if ((d_data->reagentInfo.states.testFlag(ReagentState::SPARE)))
    {
		rState = ReagentState::SPARE;
    }
    // 在用状态
    else if ((d_data->reagentInfo.states.testFlag(ReagentState::AVAILABLE)))
    {
		rState = ReagentState::AVAILABLE;
    }
    else if((d_data->reagentInfo.states.testFlag(ReagentState::EMPTY)))
    {
		rState = ReagentState::EMPTY;
    }

	// 瓶子图片
	QImage itemStateImg = d_data->reagentInfo.isReagent ? m_mapStateImages[rState].reagentImg : m_mapStateImages[rState].suppliesImg;
    painter->drawImage(d_data->area, itemStateImg);

    // 状态计算
    auto backupState = ((d_data->reagentInfo.states.testFlag(ReagentState::SPARE)||
        d_data->reagentInfo.states.testFlag(ReagentState::AVAILABLE))&&
        d_data->reagentInfo.states.testFlag(ReagentState::SAMEGROUP) &&
        !d_data->reagentInfo.states.testFlag(ReagentState::DEACTIVATE));

    // 选中
    if (d_data->reagentInfo.bSelect)
    {
        painter->drawImage(d_data->area, m_mapStateImages[rState].selectImg);
    }

    // 是否开放试剂
    if (d_data->reagentInfo.isReagent && (d_data->reagentInfo.states.testFlag(ReagentState::OPEN)) && !(d_data->reagentInfo.states.testFlag(ReagentState::EMPTY)))
    {
        painter->drawImage(d_data->statearea, m_mapStateImages[ReagentState::OPEN].reagentImg);
    }

    // 备用状态--编号绘制
    if (backupState && !(d_data->reagentInfo.states.testFlag(ReagentState::EMPTY))
		&& d_data->reagentInfo.seqNumber > 0)
    {
        painter->drawText(d_data->textarea, QString::number(d_data->reagentInfo.seqNumber));
    }

	// 在用状态--绘制●
	if (backupState && d_data->reagentInfo.seqNumber <= 0)
	{
		painter->setPen(Qt::NoPen);
		painter->setBrush(QColor(69, 159, 255));
		painter->drawEllipse(onuse_rect);
		painter->setBrush(QColor());
	}
}

QPlate::QPlate(QGraphicsView *parent /*= Q_NULLPTR*/, QPlateItem::ItemType type /*= ItemType::PLATE*/)
    :QPlateItem(parent, type)
{
    d_data = std::make_shared<PrivateData>();
}

bool QPlate::SetPlateRect(const QRectF& plateRect)
{
    if (d_data == Q_NULLPTR)
    {
        return false;
    }

    update();
    d_data->plateRect = plateRect;
    return true;
}

bool QPlate::SetTextAttr(const boost::optional<QRectF>& textRect, boost::optional<QFont>& textfont, boost::optional<QColor>& textColor)
{
    if (d_data == Q_NULLPTR)
    {
        return false;
    }

    if (textRect)
    {
        d_data->textRect = textRect.value();
    }

    if (textfont)
    {
        d_data->textFont = textfont.value();
    }

    if (textColor)
    {
        d_data->textColor = textColor.value();
    }

    update();
    return true;
}

bool QPlate::SetTextContent(QString& text)
{
    if (d_data == Q_NULLPTR)
    {
        return false;
    }

    d_data->textContent = text;
    update();
    return true;
}

void QPlate::SetPropertyWidget(const QProPertyWidget& property)
{
    d_data->plateRect = property.m_plateArea;
    d_data->textRect = property.m_plateArea;
    d_data->textFont = property.font();
}

QRectF QPlate::boundingRect() const
{
    return d_data == Q_NULLPTR ? QRectF() : d_data->plateRect;
}


void QPlate::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget /*= Q_NULLPTR*/)
{
    if (d_data == Q_NULLPTR)
    {
        return;
    }

    // 反走样
    painter->setRenderHint(QPainter::Antialiasing, true);
    // 画试剂盘
    QImage img(QString(":/Leonis/resource/image/ch/c1005/reagent-disk-bk.png"));
    painter->drawImage(d_data->plateRect, img);

    // 画文本
    // 设置画笔字体
    QColor oldColor = painter->pen().color();
    QPen myPen = painter->pen();
    painter->setPen(myPen);
    painter->setFont(d_data->textFont);
    painter->drawText(d_data->textRect, Qt::AlignHCenter | Qt::AlignVCenter, d_data->textContent);
    painter->setPen(oldColor);
}

QReagentView::QReagentView(QWidget* parent /*= Q_NULLPTR*/)
    :QGraphicsView(parent),
    m_plate(Q_NULLPTR),
    m_fan(Q_NULLPTR),
    m_currentIndex(INVALID)
{
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    d_data = std::make_shared<PrivateData>();
    static QProPertyWidget propertySave;
    static bool once = [&]() {
        QFile styleFile(":/Leonis/resource/document/deviceView.qss");
        styleFile.open(QIODevice::ReadOnly | QIODevice::Text);
        if (styleFile.isOpen())
        {
            QString setstylesheet(styleFile.readAll());
            propertySave.setStyleSheet(setstylesheet);
            styleFile.close();
            propertySave.style()->unpolish(&propertySave);
            propertySave.style()->polish(&propertySave);
        }
        return true;
    }();

    d_data->fanRect = propertySave.m_fanRect;
    d_data->startPoint = propertySave.m_startPoint;
    d_data->translatePoint = propertySave.m_translatePoint;
    d_data->isVisable = propertySave.m_isVisable;
    d_data->fontAttr = propertySave.font();
    d_data->shiftAngle = propertySave.ShiftAngle();
    d_data->innerNumber = propertySave.InnerNumber();
    d_data->innerRadius = 32;
    d_data->innerStartPointer = QPoint(propertySave.InnerStartXPointer(), propertySave.InnerStartYPointer());
    d_data->outerNumber = propertySave.OuterNumber();
    d_data->outerRadius = propertySave.OuterRadius();
    d_data->outerStartPointer = QPoint(propertySave.OuterStartXPointer(), propertySave.OuterStartYPointer());
    d_data->paintArea = propertySave.m_plateArea;
    d_data->width = propertySave.ItemWidth();

    auto total = d_data->innerNumber + d_data->outerNumber;
    if (d_data->innerNumber < 0 || d_data->outerNumber < 0
        || total <= 0)
    {
        return;
    }

    // 内部角度
    double innerDgree = 0.0;
    if (d_data->innerNumber > 0)
    {
        innerDgree = double(CIRCLEDEGREE) / double(d_data->innerNumber);
    }

    // 外部角度
    auto outerDgree = 0.0;
    if (d_data->outerNumber > 0)
    {
        outerDgree = double(CIRCLEDEGREE) / double(d_data->outerNumber);
    }

    // 创建底盘item
    m_plate = new QPlate(this);
    m_plate->SetPropertyWidget(propertySave);
    m_scene.addItem(m_plate);

    //m_fan = new QFanItem(this);
    //m_fan->SetPropertyWidget(propertySave);
    //m_fan->setPos(d_data->startPoint);
    //m_scene.addItem(m_fan);

    // 创建槽位item
    int innerIndex = 0;
    int outerIndex = 0;
    for (int i = 0; i < total; i++)
    {
        QTransform transform;
        QReagentItem* plotItem = new QReagentItem(this);
        plotItem->SetPropertyWidget(propertySave);
        // 内圈位置
        if (i >= d_data->outerNumber)
        {
            plotItem->setPos(d_data->innerStartPointer);
            transform.translate(d_data->width / 2, (d_data->outerRadius - propertySave.Scale()));
            transform.rotate(d_data->shiftAngle + -innerDgree * innerIndex++);
            transform.translate(-d_data->width / 2, -(d_data->outerRadius - propertySave.Scale()));

        }
        // 外圈位置
        else
        {
            plotItem->setPos(d_data->outerStartPointer);
            transform.translate(d_data->width / 2, d_data->outerRadius);
            transform.rotate(d_data->shiftAngle + -outerDgree * outerIndex++);
            transform.translate(-d_data->width / 2, -d_data->outerRadius);
        }
        plotItem->setTransform(transform);
        m_scene.addItem(plotItem);
        m_items.push_back(plotItem);
    }

    // 设置窗口大小
    resize(d_data->paintArea.width(), d_data->paintArea.height());
    // 设置场景大小
    m_scene.setSceneRect(d_data->paintArea);
    // 为视图设置场景
    setScene(&m_scene);

    // 旋转整个视图
    QTransform transform;
    transform.translate(d_data->paintArea.center().x(), d_data->paintArea.center().y());
    transform.rotate(0);
    transform.translate(-d_data->paintArea.center().x(), -d_data->paintArea.center().y());
    setTransform(transform);

    // 为场景安装事件过滤器
    m_scene.installEventFilter(this);
}

QReagentView::~QReagentView()
{

}

///
/// @brief 返回试剂界面的试剂总数
///
///
/// @return 若有则，返回总数
///
/// @par History:
/// @li 5774/WuHongTao，2022年10月19日，新建函数
///
boost::optional<int> QReagentView::TotalCount()
{
    if (d_data == Q_NULLPTR)
    {
        return -1;
    }

    return d_data->innerNumber + d_data->outerNumber;
}

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
bool QReagentView::SetIndex(int index)
{
    // 判断索引值是否有效，有效则选中目标索引，取消其他索引选中
    if (index > 0 && index <= m_items.size() /*&& m_currentIndex != index*/)
    {
        // 取消旧的选中项
        if (m_currentIndex != INVALID)
        {
            m_items[m_currentIndex - 1]->SetSelect(false);
        }

        // 选中新的选中项
        m_items[index - 1]->SetSelect(true);
        m_currentIndex = index;

        // 刷新整个场景影响效率 SetSelect已经调用了这个item的update了
        //m_scene.update();
        return true;
    }

    return false;
}

///
/// @brief 取消选中
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年10月19日，新建函数
///
void QReagentView::CancelSelect()
{
    // 取消旧的选中项
    if (m_currentIndex != INVALID)
    {
        m_items[m_currentIndex - 1]->SetSelect(false);
    }

    m_currentIndex = INVALID;

    // 刷新场景
    //m_scene.update();
    // 通知选中槽位索引改变
    //emit indexChanged(m_currentIndex);
}

///
/// @brief 获取当前选中项目
///
///
/// @return 当前选中编号，-1代表
///
/// @par History:
/// @li 5774/WuHongTao，2022年10月19日，新建函数
///
int QReagentView::Index()
{
    return m_currentIndex;
}

///
/// @brief 设置试剂盘显示信息
///
/// @param[in]  strSelDetail  显示信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年10月19日，新建函数
///
void QReagentView::SetPlateContent(const QString& strSelDetail)
{
    if (m_plate == Q_NULLPTR)
    {
        return;
    }

    m_plate->SetTextContent(const_cast<QString&>(strSelDetail));
}

///
/// @brief 加载试剂
///
/// @param[in]  index  试剂索引
/// @param[in]  info   试剂信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年10月19日，新建函数
///
void QReagentView::LoadReagent(int index, const QReagentItem::ReagentInfo& info)
{
    // 判断索引值是否有效，有效则装载试剂
    if (index > 0 && index <= m_items.size())
    {
        m_items[index - 1]->SetItemInfo(info);
		// 不要刷新整个场景 影响效率 SetItemInfo已经调用了这个item的update了
        //m_scene.update();
    }
}

///
/// @brief 卸载试剂
///
/// @param[in]  iIndex  试剂索引
///
/// @par History:
/// @li 5774/WuHongTao，2022年10月19日，新建函数
///
void QReagentView::UnloadReagent(int index)
{
    // 判断索引值是否有效，有效则装载试剂
    if (index > 0 && index <= m_items.size())
    {
        m_items[index - 1]->SetItemInfo(QReagentItem::ReagentInfo());
        m_currentIndex = INVALID;
		// 不要刷新整个场景 影响效率 SetItemInfo已经调用了这个item的update了
        //m_scene.update();
    }
}

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
boost::optional<QReagentItem::ReagentInfo> QReagentView::GetReagent(int index)
{
    // 判断位置是否越界
    if (index > 0 && index <= m_items.size())
    {
        return m_items[index - 1]->ItemInfo();
    }

    return boost::none;
}

void QReagentView::resizeEvent(QResizeEvent *event)
{
    if (d_data == Q_NULLPTR)
    {
        return;
    }

    // 支持缩放
    fitInView(d_data->paintArea, Qt::KeepAspectRatio);
    // 基类处理
    QGraphicsView::resizeEvent(event);
}

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
bool QReagentView::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == &m_scene && event->type() == QEvent::GraphicsSceneMousePress)
    {
        auto graphicsEvent = dynamic_cast<QGraphicsSceneMouseEvent*>(event);
        if (graphicsEvent == Q_NULLPTR)
        {
            return false;
        }

        // 点击鼠标事件
        if (graphicsEvent->button() == Qt::LeftButton)
        {
            // 检测光标下是否有item，有则置为选中
            auto items = m_scene.items(graphicsEvent->scenePos());
            int index = 0;
            for (auto& slotItem : m_items)
            {
                index++;
                // 是否选中
                if (items.contains(slotItem))
                {
					// 判断索引值是否有效，有效则选中目标索引，取消其他索引选中
					if (index > 0 && index <= m_items.size())
					{
						// 通知选中槽位索引改变
						emit indexChanged(index);
					}
					//break;
					return true;
                }
            }

            // 旋转90度 这行没必要吧？
            //static int i = 1;
            //FanRotation(i * 72);
            //(i == 6) ? i = 1 : i++;
        }
    }

    // 更新场景并转发事件
    //m_scene.update();
    return false;
}

///
/// @brief 扇面旋转
///
/// @param[in]  degree  旋转度数
///
/// @par History:
/// @li 5774/WuHongTao，2023年2月23日，新建函数
///
void QReagentView::FanRotation(int degree)
{
    if (m_fan == Q_NULLPTR)
    {
        return;
    }

    if (!d_data->isVisable)
    {
        return;
    }

    m_fan->setPos(d_data->startPoint);
    // 坐标系变换（平移，旋转）
    QTransform transform;
    transform.translate(d_data->translatePoint.x(), d_data->translatePoint.y());
    transform.rotate(degree);
    transform.translate(-(d_data->translatePoint.x()), -(d_data->translatePoint.y()));
    m_fan->setTransform(transform);
}

QFanItem::QFanItem(QGraphicsView *parent /*= Q_NULLPTR*/, QPlateItem::ItemType type /*= ItemType::FAN*/)
{
    d_data = std::make_shared<PrivateData>();
}

void QFanItem::SetPropertyWidget(const QProPertyWidget& property)
{
    d_data->fanRect = property.m_fanRect;
    d_data->startPoint = property.m_startPoint;
    d_data->translatePoint = property.m_translatePoint;
    d_data->isVisable = property.m_isVisable;
}

QRectF QFanItem::boundingRect() const
{
    return d_data == Q_NULLPTR ? QRectF() : d_data->fanRect;
}

void QFanItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget /*= Q_NULLPTR*/)
{
    if (d_data == Q_NULLPTR)
    {
        return;
    }

    painter->setRenderHint(QPainter::Antialiasing, true);
    if (d_data->isVisable)
    {
        QImage imgfan(QString(":/Leonis/resource/image/ch/c1005/newFan.png"));
        painter->drawImage(d_data->fanRect, imgfan);
    }
}
