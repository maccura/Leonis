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
/// @file     QDialogAi.h
/// @brief    AI识别结果
///
/// @author   5774/WuHongTao
/// @date     2024年03月13日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2024年03月13日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "shared/basedlg.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include <memory>

namespace Ui {
    class QDialogAi;
};


class QDialogAi : public BaseDlg
{
	Q_OBJECT

public:
	QDialogAi(QWidget *parent = Q_NULLPTR);
	void SetSample(const std::shared_ptr<tf::SampleInfo>& spSample);
	void SetAiRecognizeId(const qint64 aiRecognizeId);
	~QDialogAi();

private:
	void Clear();

	///
	/// @brief 将枚举类型转化为颜色名称
	///
	/// @param[in]  colorType  颜色枚举
	///
	/// @return 颜色名称
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年3月14日，新建函数
	///
	QString ConvertColor(int colorType);

	///
	/// @brief 将枚举转化为样本管制式名称
	///
	/// @param[in]  patternType  样本管枚举类型
	///
	/// @return 名称
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年3月14日，新建函数
	///
	QString ConvertPattern(int patternType);

private:
	Ui::QDialogAi*				ui;
};
