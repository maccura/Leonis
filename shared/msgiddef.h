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
/// @file     msgiddef.h
/// @brief    消息id定义
///
/// @author   4170/TangChuXian
/// @date     2020年4月28日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月28日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////
/// 主界面
#define MSG_ID_STARTUP_VIDEO_FINISHED                     (0x0001)          // 开机动画播放结束
#define MSG_ID_LOGIN_SUCCESSED                            (0x0002)          // 登录成功
#define MSG_ID_CURRENT_MODEL_NAME                         (0x0003)          // 状态栏当前模块
#define MSG_ID_SHOW_ERROR_TEXT                            (0x0004)          // 显示错误提示信息

/// 进度条
#define MSG_ID_PROGRESS_DIALOG_OPEN                       (0x0005)          // 显示进度条窗口
#define MSG_ID_PROGRESS_DIALOG_CHANGE                     (0x0006)          // 进度更新
#define MSG_ID_PROGRESS_DIALOG_CLOSE                      (0x0007)          // 关闭进度条窗口
#define MSG_ID_PRINT_PROGRESS_FINISH                      (0x0008)          // 打印完毕

//////////////////////////////////////////////////////////////////////////
/// 工作界面
#define MSG_ID_SAMPLE_INFO_UPDATE                         (0x0101)          // 样本信息更新
#define MSG_ID_SAMPLE_TEST_ITEM_UPDATE                    (0x0102)          // 样本测试项目信息更新
#define MSG_ID_DATA_BROWSE_SAMPLE_INFO_SELECTED           (0x0103)          // 数据浏览界面某一条样本信息被选中
#define MSG_ID_ASSAY_SELECT_SAMPLE_INFO_SELECTED          (0x0104)          // 项目选择界面某一条样本信息被选中
#define MSG_ID_SAMPLE_AND_ITEM_COLUMN_DISPLAY_UPDATE      (0x0105)          // 显示顺序更新
#define MSG_ID_SAMPLE_SHOWSET_UPDATE                      (0x0106)          // 样本显示要求更新（默认样本类型，结果显示等）
#define MSG_ID_SAMPLE_LIS_UPDATE						  (0x0107)          // 样本上传LIS状态更新
#define MSG_ID_SAMPLE_AUDIT_UPDATE_DATABROWSE			  (0x0108)          // 数据浏览界面样本审核状态更新
#define MSG_ID_SAMPLE_AUDIT_UPDATE_HIS					  (0x0109)          // 历史数据寂寞样本审核状态更新
#define MSG_ID_WORK_PAGE_SET_DISPLAY_UPDATE               (0x010a)          // 工作页面设置更新
#define MSG_ID_WORK_PAGE_RETEST_RESULT_UPDATE			  (0x010b)          // 工作页面结果详情中复查结果更新
#define MSG_ID_HIS_RETEST_RESULT_UPDATE			          (0x010c)          // 历史数据结果详情中复查结果更新
#define MSG_ID_WORK_ASSAY_STATUS_UPDATE			          (0x010d)          // 工作页面项目状态更新

//////////////////////////////////////////////////////////////////////////
/// 试剂界面
#define MSG_ID_REAGENT_INFO_UPDATE                        (0x0201)          // 生化试剂、耗材信息更新(来自于dcs，只需CommonInformationManager关注)
#define MSG_ID_IM_REAGENT_INFO_UPDATE                     (0x0202)          // 免疫试剂信息更新
#define MSG_ID_IM_DILUENT_INFO_UPDATE                     (0x0203)          // 免疫稀释液信息更新
#define MSG_ID_IM_SUPPLY_INFO_UPDATE                      (0x0204)          // 免疫耗材信息更新
#define MSG_ID_REAGENT_LOADER_UPDATE                      (0x0205)          // 试剂加载器状态更新
#define MSG_ID_REAGENT_SCAN_POS_START                     (0x0206)          // 试剂指定位置扫描开始
#define MSG_ID_REAGENT_PLATE_INFO_UPDATE                  (0x0207)          // 试剂盘信息更新（开关盖、角度）
#define MSG_ID_REAGENT_LOAD_STATUS_UPDATE                 (0x0208)          // 试剂在线加载机构使用状态更新
#define MSG_ID_REAGENT_TAKE_MESSAGE						  (0x0209)          // 拿走试剂在线加载机构上的试剂盒消息
#define MSG_ID_REACTION_CUP_INFO_UPDATE                   (0x020A)          // 反应杯信息更新
#define MSG_ID_ISE_INFO_UPDATE							  (0x020B)          // ISE的耗材信息更新(来自于dcs，只需CommonInformationManager关注)
#define MSG_ID_MANAGER_REAGENT_LOADER_UPDATE              (0x020C)          // 通用消息管理器试剂加载器状态更新
#define MSG_ID_MANAGER_UNALLOCATE_ASSAYS_UPDATE           (0x020D)          // 通用消息管理器未分配项目列表信息更新（CommonInformationManager -> 试剂页面）
#define MSG_ID_ASSAY_MASK_UPDATE						  (0x020E)          // 项目遮蔽消息更新
#define MSG_ID_MANUAL_HANDL_REAG						  (0x020F)          // 申请更换试剂结果消息
#define MSG_ID_CH_REAGENT_SUPPLY_INFO_UPDATE              (0x0210)          // 生化试剂、耗材信息更新(CommonInformationManager抛出，其他界面关注该消息)
#define MSG_ID_ISE_SUPPLY_INFO_UPDATE					  (0x0211)          // ISE的耗材信息更新(CommonInformationManager抛出，其他界面关注该消息)
#define MSG_ID_CH_WASTE_CNT_UPDATE						  (0x0212)          // 生化的废液桶余量更新(CommonInformationManager抛出，其他界面关注该消息)
#define MSG_ID_IM_RGNT_LOADER_RESET_FINISH				  (0x0213)          // 免疫装载装置复位结束


//////////////////////////////////////////////////////////////////////////
/// 校准界面
#define MSG_ID_CALISELECT_STATUS_UPDATE                   (0x0301)          // 校准选择状态更新
#define MSG_ID_CALI_GRP_INFO_UPDATE                       (0x0302)          // 校准品组信息更新
#define MSG_ID_ISE_CALI_APP_UPDATE                        (0X0303)          // 电解质校准申请信息更新
#define MSG_ID_IM_CALI_GRP_UPDATE                         (0X0304)          // 免疫校准品信息更新
#define MSG_ID_IM_CALI_END                                (0X0305)          // 免疫校准结束
#define MSG_ID_CH_CALI_SET_UPDATE                         (0X0305)          // 校准设置页面数据变更（包括新增、删除、修改校准品组或者校准文档项目）
#define MSG_ID_IM_CALI_SET_UPDATE                         (0X0305)          // 校准设置页面数据变更（包括新增、删除、修改校准品组或者校准文档项目）

//////////////////////////////////////////////////////////////////////////
/// 质控界面
#define MSG_ID_QCAPP_INFO_UPDATE                          (0x0401)          // 质控申请信息更新
#define MSG_ID_IM_QCAPP_INFO_UPDATE                       (0x0402)          // 免疫质控申请信息更新
#define MSG_ID_QC_DOC_INFO_UPDATE                         (0x0403)          // 质控文档信息更新
#define MSG_ID_QC_TAB_CORNER_DISPLAY_UPDATE               (0x0404)          // 质控tab界面右上角设备显示更新
#define MSG_ID_QC_TAB_CORNER_INDEX_UPDATE                 (0x0405)          // 质控tab界面右上角选中设备索引更新
#define MSG_ID_QC_TAB_CORNER_QRY_COND_RESET               (0x0406)          // 质控tab界面右上角查询条件重置
#define MSG_ID_QC_RESULT_UPDATE                           (0x0407)          // 质控结果发生更新
#define MSG_ID_QC_RULE_UPDATE                             (0x0408)          // 质控规则更新

//////////////////////////////////////////////////////////////////////////
/// 应用界面
#define MSG_ID_PROFILE_PARAM_UPDATE                       (0x0501)          // 组合项目参数更新
#define MSG_ID_ASSAY_SYSTEM_UPDATE                        (0x0503)          // 应用系统配置更新
#define MSG_ID_DETECTION_UPDATE                           (0x0504)          // 检测设置更新
#define MSG_ID_ASSAY_SAMPLE_UPDATE                        (0x0506)          // 项目参数样本源改变
#define MSG_ID_DICTIONARY_UPDATE                          (0x0507)          // 数据字典更新
#define MSG_ID_DISPLAY_SET_UPDATE                         (0x0508)          // 显示设置更新
#define MSG_ID_CALC_ASSAY_UPDATE                          (0x0509)          // 计算项目更新
#define MSG_ID_CUSTOM_SET_UPDATE                          (0x050a)          // 自定义设置更新
#define MSG_ID_BUCKET_SET_UPDATE                          (0x050b)          // 废液桶设置更新
#define MSG_ID_CONSUMABLES_MANAGE_UPDATE                  (0X050c)          // 耗材管理更新
#define MSG_ID_USER_DEF_ASSAY_PARAM_UPDATE                (0X050d)          // 项目设置用户自定义项目参数更新
#define MSG_ID_RGNT_ALARM_VOL_UPDATE                      (0X050e)          // 试剂报警值更新
#define MSG_ID_SPL_ALARM_VAL_UPDATE                       (0X050f)          // 耗材报警值更新
#define MSG_ID_PATINET_DATA_DICT_ITEM_UPDATE              (0X0510)          // 数据字典数据项更新
#define MSG_ID_PATINET_DATA_DICT_INFO_UPDATE              (0X0511)          // 数据字典数据信息更新
#define MSG_ID_PATINET_INFO_POSITION_UPDATE               (0X0512)          // 患者信息数据更新
#define MSG_ID_DETECT_MODE_UPDATE                         (0x0513)          // 检测模式更新
#define MSG_ID_EXPORT_SET_UPDATE                          (0x0514)          // 导出设置更新
#define MSG_ID_UTILITY_VERSION_UPDATE                     (0x0515)          // 应用页面的参数版本更新
#define MSG_ID_UTILITY_FUNCTION_MNG_UPDATE                (0x0516)          // 应用功能管理更新
#define MSG_ID_CONSUMABLE_CHANGE_LOG_UPDATE				  (0x0517)          // 试剂耗材更换日志界面更新
#define MSG_ID_CALI_LINE_EXPIRE_CONFIG				      (0x0518)          // 应用-提示设置-校准提示更新
#define MSG_ID_UTILITY_ADD_ASSAY    				      (0x0519)          // 应用-项目设置-新增项目
#define MSG_ID_DEBUGPRO_RESULT                            (0x051A)          // 应用-流程调试-结果
#define MSG_ID_IM_NODE_DBG_MODULE_UPDATE                  (0x051B)          // 应用-免疫节点调试-调试模块更新
#define MSG_ID_ASSAY_UNIT_UPDATE                          (0x051C)          // 应用-项目设置-当前单位修改

//////////////////////////////////////////////////////////////////////////  
/// UI公用组件
#define MSG_ID_ASSAY_CODE_MANAGER_UPDATE                  (0x0601)          // 项目编码管理器更新
#define MSG_ID_NEW_DAY_ARRIVED                            (0x0602)          // 新的一天开始
#define MSG_ID_SYSTEM_DATE_MANUAL_CHANGED                 (0x0603)          // 系统时间手动更改
#define MSG_ID_SYSTEM_PERMISSTION_CHANGED                 (0x0604)          // 当前用户权限更改
#define MSG_ID_ASSAY_SELECT                               (0x0605)          // 选中项目列表中的指定名字的项目

//////////////////////////////////////////////////////////////////////////
/// 后台通知
#define MSG_ID_DEVS_STATUS_CHANGED                        (0x0701)          // 设备状态改变
#define MSG_ID_DEVS_CAN_APPEND_TEST                       (0x0702)          // 设备可追加测试状态更新
#define MSG_ID_DEVICE_MASK_UPDATE		                  (0x0703)			// 设备屏蔽状态更新
#define MSG_ID_DEVICE_OTHER_INFO_UPDATE		              (0x0704)			// 设备其他信息状态更新
#define MSG_ID_DCS_REPORT_STATUS_CHANGED                  (0x0705)          // 下位机上报设备状态改变
#define MSG_ID_OPTICAL_ACCURA_RESULT                      (0x0706)          // 光准结果返回
#define MSG_ID_DEVICE_STATUS_INFO						  (0x0707)          // 设备状态信息上报（温度、计数、液路、单元信息）
#define MSG_ID_MAINTENANCE_RESULT                         (0x0708)          // 维护结果返回
#define MSG_ID_SUPPLIES_INFO_UPDATE                       (0x0709)          // 耗材登记完成
#define MSG_ID_DEVS_FAULT_INFO                            (0x070A)          // 通知UI更新故障日志
#define MSG_ID_DEVS_DIS_ALARM                             (0x070B)          // 通知UI当前告警消失
#define MSG_ID_UPDATE_ASSAY_TEST_BTN                      (0x070C)          // 更新开始测试按钮状态
#define MSG_ID_CONTINUE_RUN								  (0x070D)          // 通知UI继续测试(加样停、待机下)
#define MSG_ID_LIS_CONNECTION_STATUS					  (0x070E)          // LIS连接状态更新
#define MSG_ID_RACK_RECYCLE_UPDATE						  (0x070F)          // 样本架回收消息
#define MSG_ID_ALARM_SHIELD_CLEAR_ALL				      (0x0710)          // 报警屏蔽清空
#define MSG_ID_MANAGER_UPDATE_SUPPLY				      (0x0711)          // 通用消息管理器更新耗材报警门限值
#define MSG_ID_MANAGER_UPDATE_REAGENT				      (0x0712)          // 通用消息管理器更新试剂报警门限值
#define MSG_ID_TEMPERATURE_ERR                            (0x0713)          // 温度异常
#define MSG_ID_TRACT_STATE_UPDATE						  (0x0714)          // 轨道状态更新消息
#define MSG_ID_SENSOR_STATUS_UPDATE						  (0x0715)          // 传感器状态更新消息
#define MSG_ID_ALARM_READED						          (0x0716)          // 告警信息已读
#define MSG_ID_UPDATE_STOP_MAINTAIN_TYPE                  (0X0717)          // 通知关机维护类型变更为周维护
#define MSG_ID_RGT_ALARM_READ_FLAG_UPDATE                 (0X0718)          // 试剂耗材余量不足报警已读标志更新
#define MSG_ID_ALARM_DETAIL_UPDATE						  (0x0719)          // 告警信息的详情更新

//////////////////////////////////////////////////////////////////////////
/// 维护界面
#define MSG_ID_MAINTAIN_UPDATE                            (0x0801)          // 维护界面更新
#define MSG_ID_MAINTAIN_GROUP_PHASE_UPDATE                (0x0802)          // 维护界面更新维护组执行阶段
#define MSG_ID_MAINTAIN_GROUP_RESULT_UPDATE               (0x0803)          // 维护界面更新维护组执行结果
#define MSG_ID_MAINTAIN_GROUP_PROGRESS_UPDATE             (0x0804)          // 维护界面更新维护组执行进度
#define MSG_ID_MAINTAIN_GROUP_TIME_UPDATE                 (0x0805)          // 维护界面更新维护组执行时间
#define MSG_ID_MAINTAIN_ITEM_PHASE_UPDATE                 (0x0806)          // 维护界面更新维护项执行阶段
#define MSG_ID_MAINTAIN_ITEM_RESULT_UPDATE                (0x0807)          // 维护界面更新维护项执行结果
#define MSG_ID_MAINTAIN_ITEM_PROGRESS_UPDATE              (0x0808)          // 维护界面更新维护项执行进度
#define MSG_ID_MAINTAIN_ITEM_TIME_UPDATE				  (0x0809)          // 维护界面更新维护项执行时间
#define MSG_ID_MAINTAIN_ITEM_STAGE_UPDATE				  (0x0810)          // 维护界面更新维护项执行阶段
#define MSG_ID_MAINTAIN_GROUP_UPDATE					  (0x080A)          // 维护界面更新维护组
#define MSG_ID_MAINTAIN_ITEM_UPDATE				          (0x080B)          // 维护界面更新维护项
#define MSG_ID_MAINTAIN_DETECT_UPDATE                     (0x080C)          // 维护界面更新检测系统调试信号值
#define MSG_ID_MAINTAIN_GROUP_CFG_UPDATE                  (0x080D)          // 维护组设置更新
#define MSG_ID_MAINTAIN_SHOW_SET_UPDATE                   (0x080E)          // 维护界面显示设置更新
#define MSG_ID_MAINTAIN_FAIL_CAUSE						  (0x080F)          // 维护失败原因
#define MSG_ID_MAINTAIN_DATA_INIT					      (0x0812)          // 维护数据初始化

//////////////////////////////////////////////////////////////////////////
/// 系统总览界面
#define MSG_ID_SYSTEM_OVERVIEW_CLICK_RESET                (0x0901)          // 系统总览界面点击“复位”按钮
#define MSG_ID_SYSTEM_OVERVIEW_WARNSET_UPDATE			  (0x0902)			// 系统总览-报警设置更新
#define MSG_ID_WARN_UPDATE_TO_SYS_OVERVIEW			      (0x0903)			// 报警更新发送到系统总览（更新各瓶子状态）
#define	MSG_ID_DEV_COUNT_DOWN_UPDATE					  (0x0904)			// 系统总览界面设备倒计时刷新
