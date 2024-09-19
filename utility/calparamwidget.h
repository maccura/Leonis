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
/// @file     calparamwidget.h
/// @brief    Ӧ�ý���->��Ŀ���ý���->У׼�������ý���
///
/// @author   4170/TangChuXian
/// @date     2020��8��26��
/// @version  0.1
///
/// @par Copyright(c):
///     2015 ����ҽ�Ƶ������޹�˾��All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian��2020��8��26�գ��½��ļ�
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
namespace Ui { class CalParamWidget; };

class CalParamWidget : public QWidget
{
    Q_OBJECT

public:
    CalParamWidget(QWidget *parent = Q_NULLPTR);
    ~CalParamWidget();

    ///
    /// @brief
    ///     ����ָ����Ŀ��У׼����
    ///
    /// @param[in]  db_no  ָ����Ŀ�����ݿ�����
    ///
    /// @par History:
    /// @li 4170/TangChuXian��2020��8��26�գ��½�����
    ///
    void LoadCalParam(long long db_no);

    ///
    /// @brief
    ///     ����ָ����Ŀ��У׼����
    ///
    /// @param[in]  db_no  ָ����Ŀ�����ݿ�����
    ///
    /// @par History:
    /// @li 4170/TangChuXian��2020��8��26�գ��½�����
    ///
    void SaveCalParam(long long db_no);

protected:
    ///
    /// @brief ������ʾǰ��ʼ��
    ///
    /// @par History:
    /// @li 4170/TangChuXian��2020��9��8�գ��½�����
    ///
    void InitBeforeShow();

    ///
    /// @brief ������ʾ���ʼ��
    ///
    /// @par History:
    /// @li 4170/TangChuXian��2020��8��26�գ��½�����
    ///
    void InitAfterShow();

    ///
    /// @brief
    ///     ��ʼ���źŲ�����
    ///
    /// @par History:
    /// @li 4170/TangChuXian��2020��8��26�գ��½�����
    ///
    void InitConnect();

    ///
    /// @brief
    ///     ��ʼ���ӿؼ�
    ///
    /// @par History:
    /// @li 4170/TangChuXian��2020��8��26�գ��½�����
    ///
    void InitChildCtrl();

    ///
    /// @brief ������ʾ�¼�
    ///     
    /// @param[in]  event  �¼�����
    ///
    /// @par History:
    /// @li 4170/TangChuXian��2020��8��26�գ��½�����
    ///
    void showEvent(QShowEvent *event);

    ///
    /// @brief
    ///     ��ղ�������
    ///
    /// @par History:
    /// @li 4170/TangChuXian��2020��8��26�գ��½�����
    ///
    void ClearText();

private:
    Ui::CalParamWidget      *ui;                        // UIָ�����
    bool                     m_bInit;                   // �Ƿ��Ѿ���ʼ��

    // ������Ԫ��
    friend class             AssayConfigWidget;         // ���������ô�������Ϊ��Ԫ��
};
