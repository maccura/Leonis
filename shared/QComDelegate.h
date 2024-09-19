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
/// @file     QComDelegate.h
/// @brief    各类通用代理类
///
/// @author   5774/WuHongTao
/// @date     2022年5月13日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月13日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <iostream>
#include <sstream>
#include <QStyledItemDelegate>
#include <QPixmap>
#include <QDateEdit>
#include <QToolTip>
#include <QPushButton>
#include <QComboBox>

#define     VIEW_ITEM_DISABLE_ROLE        Qt::UserRole + 10           // 单元禁能的规则

class ComBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
	///
	/// @brief 下拉框代理构造函数
	///
	/// @param[in]  parent	   父节点
	/// @param[in]  valueList  combox的初始值
	///
	/// @return 
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月13日，新建函数
	///
	ComBoxDelegate(QObject *parent, QStringList valueList);

    ///
    /// @brief
    ///     创建combox的widgets
    ///
    /// @param[in]  parent  父节点
    /// @param[in]  option  创建选项，显示的一些细节和控制
    /// @param[in]  index  model中的位置
    ///
    /// @return 创建的combox的widget指针
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年5月29日，新建函数
    ///
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    ///
    /// @brief
    ///     设置editor的显示值
    ///
    /// @param[in]  editor  editor的空间Handler
    /// @param[in]  index  位置
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年5月29日，新建函数
    ///
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    ///
    /// @brief
    ///     更新Modle的对应值
    ///
    /// @param[in]  editor  位置对应的控件
    /// @param[in]  model  模式
    /// @param[in]  index  位置
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年5月29日，新建函数
    ///
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    ///
    /// @brief
    ///     更新的区域，控件
    ///
    /// @param[in]  editor  控件指针
    /// @param[in]  option  绘制选项
    /// @param[in]  index  位置
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年5月29日，新建函数
    ///
    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
	QStringList						m_valueList;                //combox的选择的List
};


class LineEditDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:

	///
	/// @brief
	///     构造函数
	///
	/// @param[in]  parent  父节点
	///
	/// @par History:
	/// @li 5774/WuHongTao，2020年5月29日，新建函数
	///
	LineEditDelegate(QObject *parent);

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
	///     创建combox的widgets
	///
	/// @param[in]  parent  父节点
	/// @param[in]  option  创建选项，显示的一些细节和控制
	/// @param[in]  index  model中的位置
	///
	/// @return 创建的LineEdit的widget指针
	///
	/// @par History:
	/// @li 5774/WuHongTao，2020年5月29日，新建函数
	///
	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
		const QModelIndex &index) const override;

	///
	/// @brief
	///     设置editor的显示值
	///
	/// @param[in]  editor  editor的空间Handler
	/// @param[in]  index  位置
	///
	/// @par History:
	/// @li 5774/WuHongTao，2020年5月29日，新建函数
	///
	void setEditorData(QWidget *editor, const QModelIndex &index) const override;

	///
	/// @brief
	///     更新Modle的对应值
	///
	/// @param[in]  editor  位置对应的控件
	/// @param[in]  model  模式
	/// @param[in]  index  位置
	///
	/// @par History:
	/// @li 5774/WuHongTao，2020年5月29日，新建函数
	///
	void setModelData(QWidget *editor, QAbstractItemModel *model,
		const QModelIndex &index) const override;

	///
	/// @brief
	///     更新的区域，控件
	///
	/// @param[in]  editor  控件指针
	/// @param[in]  option  绘制选项
	/// @param[in]  index  位置
	///
	/// @par History:
	/// @li 5774/WuHongTao，2020年5月29日，新建函数
	///
	void updateEditorGeometry(QWidget *editor,
		const QStyleOptionViewItem &option, const QModelIndex &index) const override;

signals:
    void returnPressed();
    void textChanged(const QString &text);

public:
    void SetValidateFunc(std::function<bool(const QString&)> func);

private:
    std::function<bool(const QString&)> m_validateFunc; // 
};

// 输入限制代理类
class InputLimitDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    ///
    /// @brief
    ///     构造函数
    ///
    /// @param[in]  strRegValidator     正则表达式（输入约束）
    /// @param[in]  parent              父对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年10月7日，新建函数
    ///
    InputLimitDelegate(QString strRegValidator, QObject *parent = Q_NULLPTR);

    ///
    /// @brief
    ///     创建combox的widgets
    ///
    /// @param[in]  parent  父节点
    /// @param[in]  option  创建选项，显示的一些细节和控制
    /// @param[in]  index  model中的位置
    ///
    /// @return 创建的LineEdit的widget指针
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年10月7日，新建函数
    ///
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    ///
    /// @brief
    ///     设置editor的显示值
    ///
    /// @param[in]  editor  editor的空间Handler
    /// @param[in]  index  位置
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年10月8日，新建函数
    ///
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    ///
    /// @brief
    ///     更新Modle的对应值
    ///
    /// @param[in]  editor  位置对应的控件
    /// @param[in]  model  模式
    /// @param[in]  index  位置
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年10月8日，新建函数
    ///
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

private:
    // 输入限制
    QString         m_strRegValidator;            // 正则表达式
};

class CheckBoxDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	///
	/// @brief 初始化函数
	///
	/// @param[in]  parent  父类
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年6月14日，新建函数
	///
	CheckBoxDelegate(QObject *parent = 0);

    ///
    /// @brief 初始化函数
    ///
    /// @param[in]  path            勾选状态的图片路径
    /// @param[in]  parent          父类
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年5月9日，新建函数
    ///
    CheckBoxDelegate(QString path, QObject *parent = 0);

    CheckBoxDelegate(bool base = false, QObject *parent = 0);

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
	/// @li 8090/YeHuaNing，2022年8月29日，新建函数
	///
	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;

	///
	/// @brief
	///     处理鼠标点击事件
	///
	/// @param[in]  event  鼠标事件
	/// @param[in]  model  数据模型
	/// @param[in]  option  绘制选项
	/// @param[in]  index  位置
	///
	/// @return true成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2020年10月10日，新建函数
	///
	bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

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
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex & index) const;

    
    ///
    /// @brief  设置单元格的背景颜色
    ///     
    /// @param[in]  bkColor  背景颜色
    /// @par History:
    /// @li 7951/LuoXin，2023年5月9日，新建函数
    ///
    void setSelectedBkColor(QColor bkColor) { selectedBkColor = bkColor; };

private:
	QPixmap							selectedPixmap;				///< 选中的时候显示的图片
    QColor                          selectedBkColor;            ///< 单元格选中时的背景色
    bool                            isBase;                     ///< 是否原始对话框
};

// 浮点数输入过滤器，重写validate函数
class ChDoubleValidator : public QDoubleValidator
{
public:
	ChDoubleValidator(QObject *parent = Q_NULLPTR, int decimals = 6, int numbers = 14)
		: QDoubleValidator(parent)
        , m_decimals(decimals)
        , m_numbers(numbers)
	{
	}


	State validate(QString &input, int &pos) const
	{
        if (m_decimals == 0 && input.endsWith("."))
            return QValidator::Invalid;

        const QStringList&& list = input.split('.');
		// 如果输入为空，也可以输入
		if (input.isEmpty())
			return QValidator::Acceptable;
        else if (input.indexOf("00") == 0 || input.indexOf("-00") == 0)
            return QValidator::Invalid;
        // 限制使用科学计数、首位为小数点、负号和小数点连续输入
        else if ((input.indexOf('e') >= 0)
              || (input.indexOf('.') == 0)
              || (input.indexOf("-.") == 0)
              || (input.indexOf("+") == 0)
              || (input.indexOf(',') >= 0))
            return QValidator::Invalid;
        // 限制整数不能超过m_numbers位
        else if (!list.empty() && list.front().size() > m_numbers)
            return QValidator::Invalid;
        // 限制总位数不能超过 m_numbers ，或者小数不能超过6位
        else if ((list.size()> 1 && list.front().size() + list.back().size() > m_numbers)
                || (list.size()> 1 && list.back().size() > m_decimals))
            return QValidator::Invalid;

		return QDoubleValidator::validate(input, pos);
	}

    ///
    /// @brief 验证是否是有效数字
    // （需要配合ChDoubleValidator一起使用，只对此验证器不能防御的情况进行判断）
    ///
    /// @param[in]  text  字符串
    ///
    /// @return true： 有效数字  false：非有效数字
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年4月21日，新建函数
    ///
    static bool IsValidNumber(const QString& text)
    {
        int lastPos = text.length() - 1;

        // 使用被过滤器的前提条件下
        // 只输入了 '-' 或者最后一位是 '0' 则判断为非数字
        if ((text.lastIndexOf('.') == lastPos)
          ||(text.lastIndexOf('-') == lastPos))
        {
            return false;
        }

        return true;
    }

    ///
    /// @brief 根据校验规则，获取正确的数字字符串（此函数不校验输入值是否为有效数值）
    ///
    /// @param[in]  src  原始字符串
    ///
    /// @return 校验后的有效值
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年4月21日，新建函数
    ///
    //QString GetValidNumber(const QString& src)
    //{
        // 方案1：通过有效位来，截取字符串，已舍弃
        //if (src.isEmpty())
        //    return src;

        //const QStringList& list = src.split('.');
        //int listSize = list.size();
        //int integerCnt = 0;
        //int decimalCnt = 0;

        //if (listSize == 1)
        //{
        //    integerCnt = list.front().size();
        //}
        //else if (listSize == 2)
        //{
        //    integerCnt = list.front().size();
        //    decimalCnt = list.back().size();
        //}
        //// 不符合规则，直接返回自身
        //else
        //{
        //    return src;
        //}

        //QString dest;
        //// 如果数字区域已经大于了有效位数，就返回前段
        //if (integerCnt > m_numbers)
        //    dest = list.front().left(m_numbers);
        //// 总位数超过 m_numbers
        //else if (integerCnt + decimalCnt > m_numbers)
        //{
        //    dest = list.front() + "." + list.back().left(m_numbers - integerCnt);
        //}
        //// 小数不能超过m_decimals
        //else if (decimalCnt > m_decimals)
        //    dest = list.front() + "." + list.back().left(m_decimals);

        //// 移除小数部分末尾的0
        //if (m_decimals > 0)
        //    dest.remove(QRegExp("0*$"));

        //return dest;
    //}

    ///
    /// @brief 根据校验规则，获取正确的数字字符串（此函数不校验输入值是否为有效数值）
    ///
    /// @param[in]  src  原始字符串
    ///
    /// @return 校验后的有效值
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年4月21日，新建函数
    ///
    QString GetValidNumber(const QString& doubleToString, double number);

private:
    int                             m_decimals;                 ///< 小数位数
    int                             m_numbers;                  ///< 全部位数（整数+小数，整数输入过多，则占用小数的位数）
};

// 正数代理
class PositiveNumDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:

    ///
    /// @brief
    ///     构造函数
    ///
    /// @param[in]  parent  父节点
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年1月5日，新建函数
    ///
    PositiveNumDelegate(QObject *parent);

    ///
    /// @brief
    ///     绘制displaymode的时候的函数
    ///
    /// @param[in]  painter  图形绘制句柄
    /// @param[in]  option  选项
    /// @param[in]  index  索引
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2020年9月21日，新建函数
    ///
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex & index) const;

    ///
    /// @brief
    ///     创建combox的widgets
    ///
    /// @param[in]  parent  父节点
    /// @param[in]  option  创建选项，显示的一些细节和控制
    /// @param[in]  index  model中的位置
    ///
    /// @return 创建的LineEdit的widget指针
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年1月5日，新建函数
    ///
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;

    ///
    /// @brief
    ///     设置editor的显示值
    ///
    /// @param[in]  editor  editor的空间Handler
    /// @param[in]  index  位置
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年1月5日，新建函数
    ///
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    ///
    /// @brief
    ///     更新Modle的对应值
    ///
    /// @param[in]  editor  位置对应的控件
    /// @param[in]  model  模式
    /// @param[in]  index  位置
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年1月5日，新建函数
    ///
    void setModelData(QWidget *editor, QAbstractItemModel *model,
        const QModelIndex &index) const override;

    ///
    /// @brief
    ///     更新的区域，控件
    ///
    /// @param[in]  editor  控件指针
    /// @param[in]  option  绘制选项
    /// @param[in]  index  位置
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年1月5日，新建函数
    ///
    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const override;


	void SetValidateFunc(std::function<bool(const QString&)> func);

    ///
    /// @brief 设置小数最大位数
    ///
    /// @param[in]  decimalsPlace  小数位数(1-6位)
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年8月28日，新建函数
    ///
    void SetDecimalsPlace(int decimalsPlace);

signals:
    void textChanged(const QString &text);

private:
	std::function<bool(const QString&)> m_validateFunc;
    int                             m_decimalsPlace;
};

class QBaseDateEdit : public QDateEdit
{
    Q_OBJECT
public:
    explicit QBaseDateEdit(QWidget* parent = nullptr);
    explicit QBaseDateEdit(const QDate& date, QWidget* parent = nullptr);
    ~QBaseDateEdit();

    /// @brief 设置日期时间为空
    void setNull();
    /// @brief 判断日期时间是否为空
    bool isNull();
    /// @brief 根据设置的格式使用字符串设置日期时间；如果字符串为空，则日期时间为空
    void setDateTimeStr(const QString& dateTimeStr);
    /// @brief 根据设置的格式获取日期时间字符串；如果日期时间为空，则字符串为空
    QString getDateTimeStr(bool isStart = false);

private:
    void initControl();
    QValidator::State validate(QString& input, int& pos) const override;
    virtual QString textFromDateTime(const QDateTime& dt) const;
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

    private slots:
    void slots_clicked(const QDate& date);

private:
    QDate minDate;
};

class CustomButton : public QPushButton
{
public:
    CustomButton(const QString& text, QWidget* parent = nullptr) : QPushButton(text, parent) {}

protected:
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override
    {
        Q_UNUSED(event);

        QToolTip::hideText();
    }
};

///
/// @bref
///		显示内容在居中的ComboBox控件（复用自i800）
///
class CenterComboBox : public QComboBox
{
    Q_OBJECT

public:
    CenterComboBox(QWidget *parent = Q_NULLPTR);
    ~CenterComboBox();

    void SetTextAlignment(Qt::Alignment flag);      // 设置菜单类容居中
    void paintEvent(QPaintEvent *e);                // 绘制事件
};
