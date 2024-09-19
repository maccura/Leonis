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
/// @file     uidef.h
/// @brief    ui通用宏定义
///
/// @author   4170/TangChuXian
/// @date     2020年6月9日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月9日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#define UI_RESULT_PRECISION                         (2)                                     // UI编辑框保留结果精度(小数位数)
#define UI_QC_TARGET_SD_CV_PRECISION                (3)                                     // UI显示质控靶值SD和CV保留结果精度(小数位数)
#define FONT_FAMILY                                 "Source Han Sans CN Regular"            // 字体类型
#define FONT_SRC_PATH                               "https://developer.huawei.com/consumer/cn/doc/design-guides-V1/font-0000001157868583-V1" // 字体源码地址
#define LEONIS_SRC_PATH                             "https://github.com/maccura/leonis"     // 软件源码地址
#define QT_SRC_PATH                                 "https://www.gnu.org/licenses"          // Qt源码地址
#define UI_INVALID_DB_NO                            (long long)(-1)                         // 无效数据库主键
#define UI_REAGENT_BARCODE_LENGTH                   (24)                                    // 试剂条码长度
#define FONT_SIZE_GENERAL                           (18)                                    // 字体大小
#define C1005_METER_POINT_MAX                       (34)                                    // 生化1005测光点最大值
#define C1005_METER_POINT_MIN                       (1)                                     // 生化1005测光点最小值

#define HEADER_EDITABLE_COLUNM_FONT_COLOR           QColor(0x02, 0x5b, 0xc7)                // 表头可编辑列的字体颜色
#define UI_TEXT_COLOR_HEADER_DEFAULT                QColor(0x28, 0x28, 0x28)                // 表头默认字体颜色
#define UI_TEXT_COLOR_HEADER_MARK                   QColor(0x02, 0x5b, 0xc7)                // 表头标记字体颜色

#define UI_DATE_TIME_FORMAT                         ("yyyy-MM-dd hh:mm:ss")                 // 界面日期时间格式
#define UI_DATE_FORMAT                              ("yyyy-MM-dd")                          // 界面日期格式
#define UI_TIME_FORMAT                              ("hh:mm:ss")                            // 界面时间格式

// 单元格数据role
#define UI_ITEM_ROLE_DEFAULT_SORT                   (Qt::UserRole + 50)                     // 默认排序
#define UI_ITEM_ROLE_ICON_SORT                      (Qt::UserRole + 51)                     // 图标排序
#define UI_ITEM_ROLE_SEQ_NO_SORT                    (Qt::UserRole + 52)                     // 序号排序
#define UI_ITEM_ROLE_SEQ_CHECKBOX_SORT              (Qt::UserRole + 53)                     // 选择框排序

#define CUSTOM_EVENT_BOTTOM_BTN_SHOWN               (QEvent::User + 1)                      // 底部按钮完成显示事件
#define CUSTOM_EVENT_RADIO_BTN_SHOWN                (QEvent::User + 2)                      // 单选按钮完成显示事件
#define CUSTOM_EVENT_CHECK_BTN_SHOWN                (QEvent::User + 3)                      // 复选按钮完成显示事件
#define CUSTOM_EVENT_LABEL_SHOWN                    (QEvent::User + 4)                      // 标签完成显示事件

// 正则表达式
#define UI_REG_USER_PASSWORD                        "^([^\u4e00-\u9fa5\\s]{0,10})$"         // 用户密码
#define UI_REG_USER_PASSWORD_ANY_LENGTH             "^([^\u4e00-\u9fa5\\s]+)$"              // 用户密码无长度限制
#define UI_REG_LOGIN_NAME                           "^(([0-9]|[a-z]|[A-Z]){0,10})$"         // 登录名
#define UI_REG_USER_NAME                            "^(.{0,20})$"                           // 用户名
#define UI_DEFAULT_DOUBLE_MIX                       (99999999.99)                           // 用户默认最大可输入小数
#define	UI_REG_REQ_VAL                              "^\\d{0,10}(ml)?$"                      // 非负整数ml(需求计算需求量)
#define	UI_REG_POSITIVE_INT							"[0-9]+$"								// 非负整数
#define	UI_REG_POSITIVE_INT_NOT_ZERO_1000           "^([1-9][0-9][0-9]{0,1}|1000)$"         // 1000以内的正整数
#define UI_REG_BEAD_MIX_CNT                         "^(([1-9]\\d{0,1})|(1[0-1]\\d)|(120))$" // 磁珠混匀次数(1~120)
#define	UI_REG_FLOAT								"^\\d{1,8}(\\.\\d{1,2})?$"				// 浮点数（整数位八位，小数位4位）
#define	UI_REG_POSITIVE_FLOAT						"^\\d+(\\.\\d+)?$"						// 非负浮点数
#define	UI_REG_FLOAT_RANG						    "-?\\d{0,8}(\\.\\d{0,4})?"				// -99999999.9999到99999999.9999
#define UI_REG_SAMPLE_NUM                           "^([1-9]\\d{0,11})$"                    // 样本号(1~999999999999)
#define UI_REG_SAMPLE_BAR_NUM                       "^([1-9]\\d{0,24})$"                    // 样本条码出数字的情况下（1-25）
#define UI_REG_SAMPLE_BAR                           "^([0-9A-Za-z-]{1,25})$"                // 样本条码(1~25位)
#define UI_REG_SAMPLE_BAR_ASCII                     "^([\\x00-\\x7F]{1,25})$"               // 样本条码(1~25位){最新要求，要求条码是ASCII码}
#define UI_REG_POS_NUM_OF_RACK                      "^([1-9]|10)$"                          // 样本架上的位置号(1~10)
#define UI_LOT                                      "^([0-9]*)$"                            // 试剂批号
#define UI_SN                                       "^([0-9]*)$"                            // 试剂瓶号
#define UI_BATCH                                    "^([1]\\d{0,2}|[1-9]\\d{0,1}|200$)"     // 批量输入的最大数目1-200
#define UI_MANUAL_DILUTION                          "^([2-9]|[1-9]\\d{1,8})$"               // 手工稀释的限制(1~999999999)
#define UI_DILUTION                                 "^([3-9]|[1-4][0-9]|50)$"               // 自动稀释
#define UI_REG_RACK_NUM                             "^([1-9]\\d{0,4})$"                     // 架号(1~9999)
#define UI_REG_BATCH_REGISTER_REPEAT_TIME           "^([1-9]\\d{0,2}|1000)$"                // 批量登记重复次数(1~1000)
#define UI_REG_PRE_DILLUTE                          "^([1-9]\\d{0,3})$"                     // 预稀释倍数(1~9999)
#define UI_REG_REAGENT_SLOT_INDEX                   "^(([1-9]|1\\d|20|\\s|-|,){1,16})$"     // 试剂槽位索引(1~20)
#define UI_REG_REAGENT_BARCODE                      "^(\\d{24})$"                           // 试剂条码（24位数字）
#define UI_REG_REQ_CALC_CIRCLE                      "^([1-9]\\d{0,2})$"                     // 动态计算周期(1~999)
#define UI_REG_REQ_CALC_ADJUST_PERCENT              "^([1-9]\\d{0,2})$"                     // 批量调整动态计算值百分比(1~999)
#define UI_PORT										"^([0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$"				// 端口
#define UI_IP										"\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b"	// IP
#define UI_MAC										"^([0-9A-Fa-f]{2}[-]){5}([0-9A-Fa-f]{2})$"	                                                        // MAC地址
#define UI_TIME										"((([0-9]{3}[1-9]|[0-9]{2}[1-9][0-9]{1}|[0-9]{1}[1-9][0-9]{2}|[1-9][0-9]{3})\\-(((0[13578]|1[02])\\-(0[1-9]|[12][0-9]|3[01]))|((0[469]|11)\\-(0[1-9]|[12][0-9]|30))|(02\\-(0[1-9]|[1][0-9]|2[0-8]))))|((([0-9]{2})(0[48]|[2468][048]|[13579][26])|((0[48]|[2468][048]|[3579][26])00))\\-02\\-29))\\s([0-1][0-9]|2[0-3]):([0-5][0-9]):([0-5][0-9])$"	// 时间
#define UI_DATE										"((([0-9]{3}[1-9]|[0-9]{2}[1-9][0-9]{1}|[0-9]{1}[1-9][0-9]{2}|[1-9][0-9]{3})\\-(((0[13578]|1[02])\\-(0[1-9]|[12][0-9]|3[01]))|((0[469]|11)\\-(0[1-9]|[12][0-9]|30))|(02\\-(0[1-9]|[1][0-9]|2[0-8]))))|((([0-9]{2})(0[48]|[2468][048]|[13579][26])|((0[48]|[2468][048]|[3579][26])00))\\-02\\-29))$"
#define UI_ALARM_CODE                               "^\\d+-\\d+-\\d+$"

// 验证输入的位置信息格式是否正确(贪婪匹配)的正则
// 示例格式(数字不能以0开头，且只能两位):    12 或  12,13,60  或 12,13-15,16,17,78-79  或 2-3,4-7,8-9,99,98,96,54-57
#define UI_REAGENT_POS                              "^([1-9]\\d{0,1}(-[1-9]\\d{0,1})?)((,[1-9]\\d{0,1})|(,[1-9]\\d{0,1}-[1-9]\\d{0,1}))*$"

#define UI_REAGENT_WARNCOLOR	                    "#ffdad6"			/// 试剂列表告警颜色
#define UI_REAGENT_WARNFONT	                        "#ff565f"			/// 试剂列表告警字体颜色
#define UI_REAGENT_NOTECOLOR	                    "#ffe6ba"			/// 试剂列表提示颜色
#define UI_REAGENT_NOTEFONT	                        "#fa9016"			/// 试剂列表提示字体颜色

#define UI_ALARM_YELLOW_WARNFONT	                "#fa931c"           /// 报警列表加样停字体颜色
#define UI_ALARM_YELLOW_WARNCOLOR	                "#ffecd7"	        /// 报警列表加样停单元格颜色
#define UI_SHIELD_WARNCOLOR	                        "#adadad"	        /// 报警列表被屏蔽的行字体颜色

#define UI_HEADER_FONT_COLOR                        "#025bc7"           /// 表格可编辑列的表头字体颜色

#define UI_ASSAYPAGEMAX                             3					/// 默认项目选择页面为3
#define UI_ASSAYPAGENUM                             49					/// 每页项目数
#define UI_DEFAULT_ASSAYSELECTTB_SINGLEROW          5                   /// 界面项目选择框中单项目所占函数的默认值

#define UI_PROFILESTART                             10000				/// 组合项目的编号开始值
#define UI_DOUBLE_DECIMALS                          4                   /// UI界面可输入的小数精度
#define UI_DOUBLE_MAX_UNM                           99999999.0          /// UI界面可输入的最大小数
#define UI_INT_MAX_UNM                              99999999            /// UI界面可输入的最大整数

#define SAMPLE_MIX_MIN_VOL                          650                 /// 样本最小混匀体积，单位0.1ul
#define SAMPLE_REACTION_MIN_VOL                     750                 /// 样本最小反应体积，单位0.1ul
#define SAMPLE_REACTION_MAX_VOL                     1850                /// 样本最大反应体积，单位0.1ul
#define SAMPLE_PROBE_MIN_VOL                        10                  /// 样本针最小吸取量，单位0.1ul
#define SAMPLE_PROBE_MAX_VOL                        250                 /// 样本针最大吸取量，单位0.1ul
#define REAGENT_PROBE_MIN_VOL                       100                 /// 试剂针最小吸取量，单位0.1ul
#define REAGENT_PROBE_MAX_VOL                       1500                /// 试剂针最大吸取量，单位0.1ul
#define REAGENT_REACTION_MIN_VOL                    750                 /// 试剂最小吸取量，单位0.1ul,试剂最小反应体积75ul，bug14233
#define REAGENT_REACTION_MAX_VOL                    1000                /// 试剂最大吸取量，单位0.1ul
#define REAGENT_ALARM_MAINCODE                      90                  /// 试剂/耗材不足报警码的主报警码
#define REAGENT_ALARM_MIDCODE                       21                  /// 试剂/耗材不足报警码的中报警码

// 检查输入框是否为0
#define CheckEditorIsZero(edit)\
(!edit->text().isEmpty() && edit->text().toInt() == 0)

// 检查输入框是否为空，或者为0
#define CheckEditorIsEmptyOrZero(edit)\
(edit->text().isEmpty() || edit->text() == "0")

// 设置控件的动态属性，并刷新样式
#define SetCtrlPropertyAndUpdateQss(ctrl, name, value)\
ctrl->setProperty(name, value);\
ctrl->style()->unpolish(ctrl);\
ctrl->style()->polish(ctrl)
