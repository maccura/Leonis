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
/// @file     MacroDefine.h
/// @brief    一些宏定义
///
/// @author   7997/XuXiaoLong
/// @date     2023年7月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7997/XuXiaoLong，2023年7月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

// 设备图片
#define		C1005_1000_PICTURE						(":/Leonis/resource/image/system-overview-device-picture-c1005-1000.png")						// c1005 1000速设备图片
#define		C1005_2000_PICTURE						(":/Leonis/resource/image/system-overview-device-picture-c1005-2000.png")						// c1005 2000速设备图片
#define		I6000_PICTURE							(":/Leonis/resource/image/system-overview-device-picture-i6000.png")							// i6000设备图片
#define		ISE1005_PICTURE							(":/Leonis/resource/image/system-overview-device-picture-ise1005.png")							// ise1005设备图片
#define		TRACK_IOM_PICTURE						(":/Leonis/resource/image/system-overview-device-picture-track-iom.png")						// 轨道IOM设备图片
#define		TRACK_DQI_PICTURE						(":/Leonis/resource/image/system-overview-device-picture-track-dqi.png")						// 轨道DQI设备图片
#define		TRACK_RWC_PICTURE						(":/Leonis/resource/image/system-overview-device-picture-track-rwc.png")						// 轨道RWC设备图片
#define		C1005_SINGLE_PICTURE					(":/Leonis/resource/image/system-overview-device-picture-c1005-single.png")						// c1005 单机版设备图片
#define		I6000_SINGLE_PICTURE					(":/Leonis/resource/image/system-overview-device-picture-i6000-single.png")						// i6000 单机版设备图片

// 设备灯状态（需要分长短）
#define		LONG_LIGHT_GREEN						(":/Leonis/resource/image/system-overview-device-long-light-green.png")							// 长状态灯-绿色
#define		LONG_LIGHT_GREEN_SCROLL					(":/Leonis/resource/image/system-overview-device-long-light-green-scroll.gif")					// 长状态灯-绿色滚动
#define		LONG_LIGHT_YELLOW						(":/Leonis/resource/image/system-overview-device-long-light-yellow.png")						// 长状态灯-黄色
#define		LONG_LIGHT_YELLOW_SCROLL				(":/Leonis/resource/image/system-overview-device-long-light-yellow-scroll.gif")					// 长状态灯-黄色滚动
#define		LONG_LIGHT_RED							(":/Leonis/resource/image/system-overview-device-long-light-red.png")							// 长状态灯-红色
#define		LONG_LIGHT_GRAY							(":/Leonis/resource/image/system-overview-device-long-light-gray.png")							// 长状态灯-灰色
#define     SHORT_LIGHT_GREEN						(":/Leonis/resource/image/system-overview-device-short-light-green.png")						// 短状态灯-绿色
#define     SHORT_LIGHT_GREEN_SCROLL				(":/Leonis/resource/image/system-overview-device-short-light-green-scroll.gif")					// 短状态灯-绿色滚动
#define     SHORT_LIGHT_YELLOW						(":/Leonis/resource/image/system-overview-device-short-light-yellow.png")						// 短状态灯-黄色
#define     SHORT_LIGHT_YELLOW_SCROLL				(":/Leonis/resource/image/system-overview-device-short-light-yellow-scroll.gif")				// 短状态灯-黄色滚动
#define     SHORT_LIGHT_RED							(":/Leonis/resource/image/system-overview-device-short-light-red.png")							// 短状态灯-红色
#define     SHORT_LIGHT_GRAY						(":/Leonis/resource/image/system-overview-device-short-light-gray.png")							// 短状态灯-灰色

// 生化/免疫各个瓶子(联机)
#define     ISE_BOTTLE_SUPPLY_RED					(":/Leonis/resource/image/system-overview-ise-bottle-supply-red.png")							// ISE-耗材-红色
#define     CH_BOTTLE_REAGENT_RED					(":/Leonis/resource/image/system-overview-ch-bottle-reagent-red.png")							// 生化-试剂-红色
#define     CH_BOTTLE_SUPPLY_RED					(":/Leonis/resource/image/system-overview-ch-bottle-supply-red.png")							// 生化-耗材-红色
#define     CH_BOTTLE_LIQUID_WASTE_RED				(":/Leonis/resource/image/system-overview-ch-bottle-liquid-waste-red.png")						// 生化-废液桶-红色
#define     IM_BOTTLE_REAGENT_RED					(":/Leonis/resource/image/system-overview-im-bottle-reagent-red.png")							// 免疫-试剂-红色
#define     IM_BOTTLE_SUPPLY_RED					(":/Leonis/resource/image/system-overview-im-bottle-supply-red.png")							// 免疫-耗材-红色
#define     IM_BOTTLE_TRASH_CAN_RED					(":/Leonis/resource/image/system-overview-im-bottle-trash-can-red.png")							// 免疫-垃圾桶-红色
#define     IM_BOTTLE_LIQUID_WASTE_RED				(":/Leonis/resource/image/system-overview-im-bottle-liquid-waste-red.png")						// 免疫-废液桶-红色
#define     ISE_BOTTLE_SUPPLY_YELLOW				(":/Leonis/resource/image/system-overview-ise-bottle-supply-yellow.png")						// ISE-耗材-黄色
#define     CH_BOTTLE_REAGENT_YELLOW				(":/Leonis/resource/image/system-overview-ch-bottle-reagent-yellow.png")						// 生化-试剂-黄色
#define     CH_BOTTLE_SUPPLY_YELLOW					(":/Leonis/resource/image/system-overview-ch-bottle-supply-yellow.png")							// 生化-耗材-黄色
#define     CH_BOTTLE_LIQUID_WASTE_YELLOW			(":/Leonis/resource/image/system-overview-ch-bottle-liquid-waste-yellow.png")					// 生化-废液桶-黄色
#define     IM_BOTTLE_REAGENT_YELLOW				(":/Leonis/resource/image/system-overview-im-bottle-reagent-yellow.png")						// 免疫-试剂-黄色
#define     IM_BOTTLE_SUPPLY_YELLOW					(":/Leonis/resource/image/system-overview-im-bottle-supply-yellow.png")							// 免疫-耗材-黄色
#define     IM_BOTTLE_TRASH_CAN_YELLOW				(":/Leonis/resource/image/system-overview-im-bottle-trash-can-yellow.png")						// 免疫-垃圾桶-黄色
#define     IM_BOTTLE_LIQUID_WASTE_YELLOW			(":/Leonis/resource/image/system-overview-im-bottle-liquid-waste-yellow.png")					// 免疫-废液桶-黄色

// 生化/免疫各个瓶子(单机)
#define     CH_SINGLE_REAGENT_GREEN					(":/Leonis/resource/image/system-overview-ch-single-reagent-green.png")							// 生化-试剂-绿色
#define     CH_SINGLE_SUPPLY_GREEN					(":/Leonis/resource/image/system-overview-ch-single-supply-green.png")							// 生化-耗材-绿色
#define     CH_SINGLE_LIQUID_WASTE_GREEN			(":/Leonis/resource/image/system-overview-ch-single-liquid-waste-green.png")					// 生化-废液桶-绿色
#define     IM_SINGLE_REAGENT_GREEN					(":/Leonis/resource/image/system-overview-im-single-reagent-green.png")							// 免疫-试剂-绿色
#define     IM_SINGLE_SUPPLY_GREEN					(":/Leonis/resource/image/system-overview-im-single-supply-green.png")							// 免疫-耗材-绿色
#define     IM_SINGLE_TRASH_CAN_GREEN				(":/Leonis/resource/image/system-overview-im-single-trash-can-green.png")						// 免疫-垃圾桶-绿色
#define     IM_SINGLE_LIQUID_WASTE_GREEN			(":/Leonis/resource/image/system-overview-im-single-liquid-waste-green.png")					// 免疫-废液桶-绿色
#define     CH_SINGLE_REAGENT_RED					(":/Leonis/resource/image/system-overview-ch-single-reagent-red.png")							// 生化-试剂-红色
#define     CH_SINGLE_SUPPLY_RED					(":/Leonis/resource/image/system-overview-ch-single-supply-red.png")							// 生化-耗材-红色
#define     CH_SINGLE_LIQUID_WASTE_RED				(":/Leonis/resource/image/system-overview-ch-single-liquid-waste-red.png")						// 生化-废液桶-红色
#define     IM_SINGLE_REAGENT_RED					(":/Leonis/resource/image/system-overview-im-single-reagent-red.png")							// 免疫-试剂-红色
#define     IM_SINGLE_SUPPLY_RED					(":/Leonis/resource/image/system-overview-im-single-supply-red.png")							// 免疫-耗材-红色
#define     IM_SINGLE_TRASH_CAN_RED					(":/Leonis/resource/image/system-overview-im-single-trash-can-red.png")							// 免疫-垃圾桶-红色
#define     IM_SINGLE_LIQUID_WASTE_RED				(":/Leonis/resource/image/system-overview-im-single-liquid-waste-red.png")						// 免疫-废液桶-红色
#define     CH_SINGLE_REAGENT_YELLOW				(":/Leonis/resource/image/system-overview-ch-single-reagent-yellow.png")						// 生化-试剂-黄色
#define     CH_SINGLE_SUPPLY_YELLOW					(":/Leonis/resource/image/system-overview-ch-single-supply-yellow.png")							// 生化-耗材-黄色
#define     CH_SINGLE_LIQUID_WASTE_YELLOW			(":/Leonis/resource/image/system-overview-ch-single-liquid-waste-yellow.png")					// 生化-废液桶-黄色
#define     IM_SINGLE_REAGENT_YELLOW				(":/Leonis/resource/image/system-overview-im-single-reagent-yellow.png")						// 免疫-试剂-黄色
#define     IM_SINGLE_SUPPLY_YELLOW					(":/Leonis/resource/image/system-overview-im-single-supply-yellow.png")							// 免疫-耗材-黄色
#define     IM_SINGLE_TRASH_CAN_YELLOW				(":/Leonis/resource/image/system-overview-im-single-trash-can-yellow.png")						// 免疫-垃圾桶-黄色
#define     IM_SINGLE_LIQUID_WASTE_YELLOW			(":/Leonis/resource/image/system-overview-im-single-liquid-waste-yellow.png")					// 免疫-废液桶-黄色

// 报警按钮
#define     WARN_STATES_RED_FLICKER					(":/Leonis/resource/image/icon-sys-red-flicker.gif")											// 报警按钮-红色闪烁
#define     WARN_STATES_RED							(":/Leonis/resource/image/icon-sys-red.png")													// 报警按钮-红色
#define     WARN_STATES_YELLOW_FLICKER				(":/Leonis/resource/image/icon-sys-yellow-flicker.gif")											// 报警按钮-黄色闪烁
#define     WARN_STATES_YELLOW						(":/Leonis/resource/image/icon-sys-yellow.png")													// 报警按钮-黄色
#define     WARN_STATES_WHITE_FLICKER				(":/Leonis/resource/image/icon-sys-white-flicker.gif")											// 报警按钮-白色闪烁
#define     WARN_STATES_GRAY						(":/Leonis/resource/image/icon-sys-gray.png")													// 报警按钮-灰色

// 新报警
#define     NEW_WARN_LEFT_RED						(":/Leonis/resource/image/icon-new-warn-left-red.png")											// 新报警按钮-左-红色
#define     NEW_WARN_LEFT_PINK						(":/Leonis/resource/image/icon-new-warn-left-pink.gif")											// 新报警按钮-左-粉色
#define     NEW_WARN_RIGHT_RED						(":/Leonis/resource/image/icon-new-warn-right-red.png")											// 新报警按钮-右-红色
#define     NEW_WARN_RIGHT_PINK						(":/Leonis/resource/image/icon-new-warn-right-pink.png")										// 新报警按钮-右-粉色
