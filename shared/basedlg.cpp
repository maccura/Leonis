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
/// @file     basedlg.cpp
/// @brief    通用基础对话框
///
/// @author   4170/TangChuXian
/// @date     2020年4月24日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "basedlg.h"
#include <QImage>
#include <QLabel>
#include <QTimer>
#include <QScreen>
#include <QApplication>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QStyle>

// 对话框
#define DLG_DEFAULT_SIZE                QSize(1000, 500)                        // 对话框默认尺寸
#define DLG_DEFAULT_MARGIN              (0)                                     // 对话框默认内距
#define DLG_BACKGROUND_COLOR            QColor(0, 0, 0, 0)                      // 对话框背景色

// 标题栏
#define TITLE_LAB_HEIGHT                (70)                                    // 标题栏高度
#define TITLE_LAB_COLOR                 QColor(43, 129, 234, 0)                 // 标题栏颜色

// 内容栏
#define CONTENT_FRAME_COLOR             QColor(255, 255, 255, 0)                // 内容栏颜色

// 字体
#define TITLE_FONT_FAMILLY              QString("思源黑体")                     // 字体族
#define TITLE_FONT_SIZE                 (19)                                    // 字体大小
#define TITLE_FONT_WIGHT                QFont::DemiBold                         // 字体粗细
#define TITLE_FONT_COLOR                QColor(255, 255, 255, 0)                // 字体颜色

// 对象名（提供qss定位对应控件）
#define BASE_DLG_BACKGROUND_OBJ_NAME    QString("base_dlg_backgound_frame")     // 对话框背景Frame
#define BASE_DLG_TITLE_OBJ_NAME         QString("base_dlg_title_lab")           // 对话框标题栏label
#define BASE_DLG_CONTENT_OBJ_NAME       QString("base_dlg_content_frame")       // 对话框内容栏Frame

// 声明静态成员变量
QPoint              BaseDlg::sm_ptCenter;               // 对话框中心点

BaseDlg::BaseDlg(QWidget *parent)
    : QDialog(parent),
      m_bIsMousePress(false),
      m_mousePressPoint(0, 0)
{
    // 对话框默认为模态
    setModal(true);

    // 设置窗口无边框且为对话框
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);

    // 设置窗口透明
    setAttribute(Qt::WA_TranslucentBackground, true);

    // 设置对话框大小
    resize(DLG_DEFAULT_SIZE);

    // 初始化成员变量
    m_backgroundFrame = new QFrame();
    m_pTitleLabel     = new QLabel();
    m_pContentFrame   = new QFrame();

    // 设置对象名
    m_backgroundFrame->setObjectName(BASE_DLG_BACKGROUND_OBJ_NAME);
    m_pTitleLabel->setObjectName(BASE_DLG_TITLE_OBJ_NAME);
    m_pContentFrame->setObjectName(BASE_DLG_CONTENT_OBJ_NAME);

    // 设置窗口背景
    QPalette pal(m_backgroundFrame->palette());
    pal.setBrush(QPalette::Window, QBrush(DLG_BACKGROUND_COLOR));
    m_backgroundFrame->setAutoFillBackground(true);
    m_backgroundFrame->setPalette(pal);

    // 为标题栏安装事件过滤器
    m_pTitleLabel->installEventFilter(this);

    QGridLayout* gLayout = new QGridLayout(this);
    gLayout->addWidget(m_backgroundFrame);
    gLayout->setMargin(0);
    gLayout->setSpacing(0);

    // 设置布局
    QVBoxLayout* vLayout = new QVBoxLayout(m_backgroundFrame);
    vLayout->addWidget(m_pTitleLabel);
    vLayout->addWidget(m_pContentFrame);
    vLayout->setMargin(DLG_DEFAULT_MARGIN);
    vLayout->setSpacing(0);

    // 初始化标题栏
    InitTitleLab();

    // 初始化内容栏
    InitContentLab();
}

BaseDlg::~BaseDlg()
{
}

///
/// @brief 设置标题名
///     
/// @param[in]  strTitlename  标题名
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月30日，新建函数
///
void BaseDlg::SetTitleName(const QString& strTitlename)
{
    m_pTitleLabel->setText(strTitlename);
}

///
/// @brief 获取标题名
///     
/// @par History:
/// @li 4170/TangChuXian，2020年4月30日，新建函数
///
QString BaseDlg::GetTitleName()
{
    return m_pTitleLabel->text();
}

///
/// @brief
///     设置中心点
///
/// @param[in]  ptCenter  中心点
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月9日，新建函数
///
void BaseDlg::SetCenterPoint(const QPoint& ptCenter)
{
    sm_ptCenter = ptCenter;
}

///
/// @brief
///     获取中心点
///
/// @return 显示中心点坐标
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月30日，新建函数
///
QPoint BaseDlg::GetCenterPoint()
{
    return sm_ptCenter;
}

///
/// @brief 初始化标题栏
///     
/// @par History:
/// @li 4170/TangChuXian，2020年4月24日，新建函数
///
void BaseDlg::InitTitleLab()
{
    // 设置标题栏位置大小
    m_pTitleLabel->setFixedHeight(TITLE_LAB_HEIGHT);
    m_pTitleLabel->resize(width(), m_pTitleLabel->height());
    m_pTitleLabel->move(0, 0);

    // 设置默认标题
    m_pTitleLabel->setText("");

    // 设置字体
    QFont titleFont = m_pTitleLabel->font();
    titleFont.setFamily(TITLE_FONT_FAMILLY);
    titleFont.setPointSize(TITLE_FONT_SIZE);
    titleFont.setWeight(TITLE_FONT_WIGHT);
    m_pTitleLabel->setFont(titleFont);

    // 设置对齐方式
    m_pTitleLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    // 设置字体颜色和背景
    QPalette pal(m_pTitleLabel->palette());
    pal.setColor(QPalette::WindowText, TITLE_FONT_COLOR);
    pal.setBrush(QPalette::Window, QBrush(TITLE_LAB_COLOR));
    m_pTitleLabel->setAutoFillBackground(true);
    m_pTitleLabel->setPalette(pal);
}

///
/// @brief 初始化内容栏
///     
/// @par History:
/// @li 4170/TangChuXian，2020年4月24日，新建函数
///
void BaseDlg::InitContentLab()
{
    // 设置内容栏位置
    m_pContentFrame->move(0, TITLE_LAB_HEIGHT);

    // 内容栏设置背景
    QPalette pal(m_pContentFrame->palette());
    pal.setBrush(QPalette::Window, QBrush(CONTENT_FRAME_COLOR));
    m_pContentFrame->setAutoFillBackground(true);
    m_pContentFrame->setPalette(pal);
}

///
/// @brief
///     事件过滤器（主要用于监听标题栏拖拽）
///
/// @param[in] obj    事件的目标对象
/// @param[in] event  事件对象
///
/// @return true表示事件已处理，false表示事件未处理
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月24日，新建函数
///
bool BaseDlg::eventFilter(QObject *obj, QEvent *event)
{
    // 支持鼠标拖拽标题栏移动
    if (obj == m_pTitleLabel)
    {
        if (event->type() == QEvent::MouseButtonPress)                          // 鼠标按下事件
        {
            QMouseEvent* pMouseEvt = dynamic_cast<QMouseEvent*>(event);         // 鼠标事件

            // 判断类型是否转换成功
            if (pMouseEvt == Q_NULLPTR)
            {
                return false;
            }

            m_bIsMousePress        = true;                                      // 鼠标已经按下
            m_mousePressPoint      = pMouseEvt->globalPos();                    // 记录鼠标的世界坐标
            m_mousePressDlgPoint   = this->frameGeometry().topLeft();           // 记录对话框的世界坐标
        }
        else if (event->type() == QEvent::MouseMove)                            // 鼠标移动事件
        {
            QMouseEvent* pMouseEvt = dynamic_cast<QMouseEvent*>(event);         // 鼠标事件

            // 判断类型是否转换成功
            if (pMouseEvt == Q_NULLPTR)
            {
                return false;
            }

            QPoint relativePos     = pMouseEvt->globalPos() - m_mousePressPoint;// 移动中的鼠标位置相对于初始位置的相对位置
            //this->move(m_mousePressDlgPoint + relativePos);                     // 然后移动窗体即可
        }
        else if (event->type() == QEvent::MouseButtonRelease)                   // 鼠标松开事件
        {
            m_bIsMousePress        = false;                                     // 鼠标已经松开
        }
    }

    return QDialog::eventFilter(obj, event);
}

///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月9日，新建函数
///
void BaseDlg::showEvent(QShowEvent *event)
{
    // 如果中心点有效，则移动到中心点
    if (!sm_ptCenter.isNull())
    {
        this->move(sm_ptCenter.x() - (width() / 2), sm_ptCenter.y() - (height() / 2));
    }

    QDialog::showEvent(event);
}

QCustomDialog::QCustomDialog(QWidget* parent /*= Q_NULLPTR*/, int times/* = 3000*/, QString showMsg /*= tr("新增成功")*/, bool isSuccess)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , m_times(times)
    , m_showMsg(showMsg)
	, m_success(isSuccess)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_AlwaysStackOnTop);
    setObjectName("flashDiglog");
    QLabel* label = new QLabel(m_showMsg, this);
    label->setAlignment(Qt::AlignCenter);
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(label);
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &QCustomDialog::close);
    timer->setSingleShot(true);
    timer->start(m_times);

    if (m_success)
    {
        label->setProperty("bks", "flashDiglog_success");
    }
	else
	{
		label->setProperty("bks", "flashDiglog_fail");
	}

    label->style()->unpolish(label);
    label->style()->polish(label);
	//SetCenter();
}

void QCustomDialog::SetCenter()
{
    // 获取中心点
    QPoint ptCenter = BaseDlg::GetCenterPoint();

    // 如果中心点有效，则移动到中心点
    if (!ptCenter.isNull())
    {
        this->move(ptCenter.x() - (width() / 2), ptCenter.y() - (height() / 2));
    }

// 	auto desktop = QApplication::desktop();
// 	if (desktop == Q_NULLPTR)
// 	{
// 		return;
// 	}
// 
// 	auto screenIndex = desktop->screenNumber(QApplication::activeWindow());
// 	auto screenArray = QApplication::screens();
// 	if (screenIndex < 0 || screenIndex >= screenArray.size())
// 	{
// 		return;
// 	}
// 
// 	auto screen = screenArray[screenIndex];
// 	if (screen == Q_NULLPTR)
// 	{
// 		return;
// 	}
// 
// 	auto screenGmometry = screen->geometry();
// 	auto x = screenGmometry.width() / 2;
// 	auto y = screenGmometry.height() / 2;
// 	this->move(x, y);
}

///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月30日，新建函数
///
void QCustomDialog::showEvent(QShowEvent *event)
{
    // 基类先处理
    QDialog::showEvent(event);

    // 显示在中心
    SetCenter();
}
