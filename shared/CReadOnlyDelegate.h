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
/// @file     CReadOnlyDelegate.h
/// @brief    只读
///
/// @author   5774/WuHongTao
/// @date     2020年5月29日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2020年5月29日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QStandardItemModel>
#include <QLineEdit>
#include <QTabWidget>
#include <QLabel>

class QPainter;
class QEvent;
class QPushButton;
class QTableView;

#define DELEGATE_POSITION_OFFSET_POS (1) // 位置显示代理中，图标数据的存储位置相对于Qt::UserRole的偏移
#define DELEGATE_COLOR_OFFSET_POS 5 // 只读单元格代理中，颜色数据的存储位置相对于Qt::UserRole的偏移
#define DELEGATE_DATATIME_EXPIRE 10 // 只读其存在过去判断的单元格代理

enum ColorType
{
	UPPERCOLOR,			 ///< 高于标准的颜色值
	LOWERCOLOR,          ///< 低于标准的颜色值
	NORMALCOLOR          ///< 正常标准的颜色值
};

class RotatedTabBar : public QTabBar
{
public:
	RotatedTabBar(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void enterEvent(QEvent* event) override;
	void leaveEvent(QEvent* event) override;

private:
	int hoverIndex;
};

// 右上角R标记的tabbra
class RFlagTabWidget;
class RFlagTabBar : public QTabBar
{
public:
	///
	/// @brief 构造函数
	///
	/// @param[in]  parent  对应tabWidget指针
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年10月11日，新建函数
	///
	RFlagTabBar(RFlagTabWidget* parent) 
	{
		m_parentRFlagTabWidget = parent;
	}

	///
	/// @brief 设置带flag的标志的索引，该索引是tabwidget中的所有tab中对应的索引值
	///
	/// @param[in]  vecFlag  索引容器
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年10月11日，新建函数
	///
	void SetFlag(const std::vector<int>& vecFlag)
	{
		m_vecFlag = vecFlag;
	}

protected:
    ///
    /// @brief 绘图事件
    ///
    /// @param[in]  event  
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7702/WangZhongXin，2023年10月11日，新建函数
    ///
    void paintEvent(QPaintEvent* event) override;

private:
	std::vector<int> m_vecFlag;				// flag索引容器		
	RFlagTabWidget* m_parentRFlagTabWidget; // 对应RFlagTabWidget指针
};

// 右上角翻页控件
class TabPageCorner : public QWidget 
{
	Q_OBJECT
public:
	TabPageCorner(QWidget* parent = nullptr);
signals:
	///
	/// @brief 上一页信号
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年10月11日，新建函数
	///
	void SignalPrePage();

	///
	/// @brief 下一页信号
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年10月11日，新建函数
	///
	void SignalNextPage();
public:
	///
	/// @brief 设置当前页和总页数，1-开始计数
	///
	/// @param[in]  curPage		当前页数
	/// @param[in]  totalPage	总页数
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年10月11日，新建函数
	///
	void SetPage(int curPage, int totalPage);
private:
	QPushButton* m_prePageButton;	// 上一页按钮
	QPushButton* m_nextPageButton;	// 下一页按钮
	QLabel* m_curPageLabel;			// 当前页显示label
	QLabel* m_totalPageLabel;		// 总页数显示label
};

class RFlagTabWidget : public QTabWidget
{
	Q_OBJECT
public:
	explicit RFlagTabWidget(QWidget* parent= nullptr);

	///
	/// @brief 设置带flag的标志的索引，该索引是tabwidget中的所有tab中对应的索引值
	///
	/// @param[in]  vecFlag  索引容器
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年10月11日，新建函数
	///
	void SetFlag(const std::vector<int>& vecFlag);

	///
	/// @brief 根据在容器内的索引获取在当前页内的索引和页索引
	///
	/// @param[in]		index			tab在容器内的索引
	/// @param[out]		indexInPage		当前页内的索引
	/// @param[out]		pageIndex		页索引
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年10月11日，新建函数
	///
	bool GetPageIndex(int index, int& indexInPage, int& pageIndex);

	///
	/// @brief 获取当前页索引
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年10月11日，新建函数
	///
	int GetCurPageIndex();

	// 注意不能使用QTabWidget的addTab和clear，会和设计功能冲突！ add by wzx-231011
	///
	/// @brief 添加tab窗口
	///
	/// @param[in]  tab		tab窗口
	/// @param[in]  name	tab窗口名称
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年10月11日，新建函数
	///
	void AddTab(QWidget* tab,QString name);

	///
	/// @brief	清除所有tab
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年10月11日，新建函数
	///
	void Clear();

	///
	/// @brief 获取所有tab数量，注意并不是所有tab都显示出来了
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年10月11日，新建函数
	///
	int Count();

signals:
	///
	/// @brief 当前的tab页是否带Rflag，在当前页改变时触发该信号
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年12月6日，新建函数
	///
	void SignalCurrentTabIsRFlag(bool d);

private:

	///
	/// @brief 设置当前页
	///
	/// @param[in]  curPageIndex  当前页索引
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年10月11日，新建函数
	///
	void SetPage(int curPageIndex);
private:
	RFlagTabBar* m_tabBar;
	TabPageCorner* m_tabPageCorner;
	std::vector<int> m_vecFlag;				// flag索引容器		
	int m_curPageIndex;		// 当前页索引 0开始计数
	int m_totalPage;		// 总页数 1开始计数
	QVector<std::tuple<QWidget*, QString>> m_vecTabs;
};

class CustomTabWidget : public QTabWidget
{
	Q_OBJECT
public:
    explicit CustomTabWidget(QWidget* parent = nullptr);

signals:
    void currentChanged(int index);
};


// IP显示代理
class CIPDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	///
	/// @brief 构造函数
	///
	/// @param[in]  parent  
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年4月23日，新建函数
	///
	CIPDelegate(QObject *parent):QStyledItemDelegate(parent)
	{
	}

	///
	/// @brief 创建代理显示窗口
	///
	/// @param[in]  parent  
	/// @param[in]  option  
	/// @param[in]  index  
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年4月23日，新建函数
	///
	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;
};

// 端口显示代理
class CPortDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	///
	/// @brief 构造函数
	///
	/// @param[in]  parent  
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年4月23日，新建函数
	///
	CPortDelegate(QObject *parent) :QStyledItemDelegate(parent)
	{
	}

	///
	/// @brief 创建代理显示窗口
	///
	/// @param[in]  parent  
	/// @param[in]  option  
	/// @param[in]  index  
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年4月23日，新建函数
	///
	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;
};

class CReadOnlyDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:

    ///
    /// @brief
    ///     只读代理的构造函数
    ///
    /// @param[in]  parent  父节点
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年5月29日，新建函数
    ///
    CReadOnlyDelegate(QObject *parent);

    ///
    /// @brief
    ///     绘制displaymode的时候的函数
    ///
    /// @param[in]  painter  图形绘制句柄
    /// @param[in]  option  选项
    /// @param[in]  index  索引
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年9月21日，新建函数
    ///
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex & index) const;

    ///
    /// @brief
    ///     不创建editor
    ///
    /// @param[in]  parent  父节点
    /// @param[in]  option  创建选项，显示的一些细节和控制
    /// @param[in]  index  model中的位置
    ///
    /// @return NULL
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年5月29日，新建函数
    ///
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;

    ///
    /// @brief
    ///     重写显示文本（主要用于可配置的日期时间格式显示）
    ///
    /// @param[in]  value   显示值
    /// @param[in]  locale  本地
    ///
    /// @return 显示字符串
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年4月19日，新建函数
    ///
    virtual QString displayText(const QVariant &value, const QLocale &locale) const override;

};


class QCircleButton : public QStyledItemDelegate
{
    Q_OBJECT

public:
    QCircleButton(QObject *parent);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex & index) const;
};

class QResultDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    ///
    /// @brief 结果的显示
    ///
    /// @param[in]  parent  父控件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年12月6日，新建函数
    ///
    QResultDelegate(QObject *parent);

    virtual QString displayText(const QVariant &value, const QLocale &locale) const;

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex & index) const;
};


class QStatusDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    ///
    /// @brief 数据浏览中的状态delegate
    ///
    /// @param[in]  parent  
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年5月30日，新建函数
    ///
    QStatusDelegate(QObject *parent);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex & index) const;
};

class QPositionDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	///
	/// @brief 位置显示代理
	///
	/// @param[in]  parent  父控件
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年4月7日，新建函数
	///
	QPositionDelegate(QObject *parent);

	virtual QString displayText(const QVariant &value, const QLocale &locale) const;

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex & index) const;
};

// 自定义表头字体颜色
class QColorHeaderModel : public QStandardItemModel
{
public:
    QColorHeaderModel(QObject* parent = nullptr) 
        : QStandardItemModel(parent)
    {
    }

    QColorHeaderModel(std::map<int, QColor> map, QObject* parent = nullptr) 
        : QStandardItemModel(parent)
        , m_mapRowAndColor(map)
    {
    }

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void SetColunmColor(int row, QColor color);

protected:
    std::map<int, QColor>            m_mapRowAndColor;          // 设置表头字体颜色的行和对应的颜色
};

class QAutoSeqNoModel : public QStandardItemModel
{
    Q_OBJECT

public:
    QAutoSeqNoModel(Qt::AlignmentFlag textAlign = Qt::AlignVCenter, QObject* parent = nullptr)
        : m_textAlign(textAlign),
        QStandardItemModel(parent)
    {
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
        if (role == Qt::ToolTipRole)
        {
            return QStandardItemModel::data(index, Qt::DisplayRole);
        }
        else if (role == Qt::DisplayRole && index.column() == 0)
        {
            return QString("%1").arg(index.row() + 1);
        }
        else if (role == Qt::TextAlignmentRole)
            return m_textAlign;

        return QStandardItemModel::data(index, role);
    }

private:
    Qt::AlignmentFlag               m_textAlign;                /// 文字对齐方式
};

// 带清除功能的Lineedit
class LineEdit : public QLineEdit
{
public:
	///
	/// @brief 构造函数
	///
	/// @param[in]  parent  
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年8月16日，新建函数
	///
	LineEdit(QWidget* parent = nullptr);

protected:
	///
	/// @brief 窗口尺寸改变事件
	///
	/// @param[in]  event  
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年8月16日，新建函数
	///
	void resizeEvent(QResizeEvent *event) override;

	///
	/// @brief 事件过滤器
	///
	/// @param[in]  obj  
	/// @param[in]  event  
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年8月16日，新建函数
	///
	bool eventFilter(QObject *obj, QEvent *event) override;

private:
	QPushButton*			m_clearButton;			///< 清除按钮
};

// QTableView的自定义数据模型的自定义排序
class QUtilitySortFilterProxyModel :public QSortFilterProxyModel
{
public:
    QUtilitySortFilterProxyModel(QObject* obj);
    virtual bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;

    // 启用表格排序
    void SetTableViewSort(QTableView* view, const QVector<int>& vecSortColumn);

private:
    QVector<int>                m_vecSortColumn;    // 可排序的列
};

class QToolTipModel : public QStandardItemModel
{
public:
    QToolTipModel(QObject* parent = nullptr)
        : QStandardItemModel(parent)
    {
    }
    QToolTipModel(bool center = false, QObject* parent = nullptr)
        : m_bTextCenter(center)
        , QStandardItemModel(parent)
    {
    }

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (role == Qt::ToolTipRole)
        {
            return QStandardItemModel::data(index, Qt::DisplayRole);
        }
        else if (role == Qt::TextAlignmentRole)
        {
            if (m_bTextCenter)
                return Qt::AlignCenter;
        }

        return QStandardItemModel::data(index, role);
    }

private:
    bool                            m_bTextCenter;              ///< 文字是否居中
};

class ComboBoxDelegate : public QStyledItemDelegate
{
public:
    ComboBoxDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

    QWidget *createEditor(QWidget *parent,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor,
        QAbstractItemModel *model,
        const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;

    void setItemText(const QStringList& items) { m_listItemText = items; }
private:
    QStringList             m_listItemText;
};

class AutoExpriteDelegate : public CReadOnlyDelegate
{
public:
    AutoExpriteDelegate(QObject* parent = nullptr)
        :CReadOnlyDelegate(parent) {
    }
    ~AutoExpriteDelegate() {};

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex & index) const;
};
