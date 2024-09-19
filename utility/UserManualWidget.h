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
/// @file     UserManualWidget.h
/// @brief    用户手册界面
///
/// @author   6889/ChenWei
/// @date     2024年1月22日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QtWidgets/QMainWindow>
#include <vector>
#include <memory>
#include <QTimer>
namespace Ui { class UserManualWidget; };

class PdfPage;
class QGraphicsScene;
class GraphicsViewZoomer;
class BookmarkModel;
class UserManualWidget : public QMainWindow
{
    Q_OBJECT

    struct Book
    {
        std::vector<std::shared_ptr<PdfPage>> m_PdfPages;
        QGraphicsScene*      m_pDocScene;
        BookmarkModel*       m_Bookmark;
        int                  m_iCurrentPage;

        Book()
        {
            m_pDocScene = nullptr;
            m_pDocScene = nullptr;
            m_iCurrentPage = 1;
        }
    };

public:
    UserManualWidget(QWidget *parent = Q_NULLPTR);
    ~UserManualWidget();

    //     窗口显示事件
    void showEvent(QShowEvent *event) override;


private:
    // 界面显示前初始化
    void InitBeforeShow();

    // 显示之后初始化
    void InitAfterShow();

    // 初始化字符串资源
    void InitStrResource();

    // 初始化连接
    void InitConnect();

    // 初始化说明书切换按钮
    void InitSwitchoverBtn();

    // 获取文件名
    QString GetFileName(int iDocID);

    // 加载文件
    void LoadFile(QString strFileName, int iDocID);

    // 显示缩放比例
    void SetScaleLab();

    // 设置显示当前页
    void SetPageNumber();

    // 计算页面跳转位置
    QRectF calcPageShift();

    // 计算指定页面是否可见
    bool pageIsVisible(int iPageIndex);

    // 设置指定页面是否可见
    void SetpageIsVisible(int iPageIndex, bool bFlage);

    // 构建目录树
    void BuildBookmarkTree();

private slots:
    
    // 生化按钮
    void OnChClicked();

    // 免疫按钮
    void OnImClicked();

    // 响应滑块移动
    void slotSliderMoved(int value);

    // 调整页面大小
    void resizeDone();

    // 放大
    void OnZoomInClicked();

    // 缩小
    void OnZoomOutClicked();

    // 恢复
    void OnRecoverClicked();

    // 上一页
    void OnPrePageClicked();

    // 下一页
    void OnNextPageClicked();

    // 第一页
    void OnFirstPageClicked();

    // 最后一页
    void OnLastPageClicked();

    // 跳转页面
    void JumpPage();

    // 目录被点击
    void TreeItemClicked(const QModelIndex &index);

private:
    Ui::UserManualWidget *ui;
    bool                                    m_bInit;
    QTimer                                  m_resizeTimer;
    std::map<int, Book>                     m_Books;
    double                                  m_dScalePercent;
    int                                     m_iScrolValue;
};
