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
/// @file     devise.h
/// @brief    适配器类型定义
///
/// @author   4170/TangChuXian
/// @date     2022年12月12日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月12日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once
#include <stdint.h>
#include <QString>
#include <QDate>
#include <QVector>
#include <QMap>

// 数据码
#define  DATA_CODE_ASSAY_NAME_LIST              (0x00000001)        // 项目名列表

#define  DATA_CODE_QC_DOC                       (0x04000001)        // 质控文档
#define  DATA_CODE_QC_APPLY                     (0x04000002)        // 质控申请
#define  DATA_CODE_DEFAULT_QC                   (0x04000003)        // 默认质控
#define  DATA_CODE_BACKUP_RGNT_QC               (0x04000004)        // 备用瓶质控
#define  DATA_CODE_QC_CONC_INFO                 (0x04000005)        // 质控靶值信息
#define  DATA_CODE_QC_RLT_INFO                  (0x04000006)        // 质控结果信息
#define  DATA_CODE_QC_TARGET_VAL_SD             (0x04000007)        // 质控靶值SD
#define  DATA_CODE_QC_REASON_SOLUTION_SINGLE    (0x04000008)        // 单质控失控原因和处理错误
#define  DATA_CODE_QC_REASON_SOLUTION_TWIN      (0x04000009)        // 联合质控失控原因和处理错误
#define  DATA_CODE_QC_CALCULATE_POINT_SINGLE    (0x0400000A)        // 单质控计算点
#define  DATA_CODE_QC_CALCULATE_POINT_TWIN      (0x0400000B)        // 联合质控计算点
#define  DATA_CODE_QC_TWIN_RLT_INFO             (0x0400000C)        // 联合质控结果信息
#define  DATA_CODE_QC_RLT_ASSAY_LIST_SINGLE     (0x0400000D)        // 单质控结果项目列表
#define  DATA_CODE_QC_RLT_ASSAY_LIST_TWIN       (0x0400000E)        // 联合质控结果项目列表

// 数据操作类型
enum DATA_OPERATE_TYPE
{
    DATA_OPERATE_TYPE_QUERY = 0,                                    // 查询
    DATA_OPERATE_TYPE_MODIFY,                                       // 修改
    DATA_OPERATE_TYPE_ADD,                                          // 增加
    DATA_OPERATE_TYPE_DELETE                                        // 删除
};

// 质控状态
enum QC_STATE
{
    QC_STATE_NONE = 0,                                              // 无状态
    QC_STATE_IN_CTRL,                                               // 在控
    QC_STATE_WARNING,                                               // 警告
    QC_STATE_OUT_OF_CTRL                                            // 失控
};

// 设备类别
enum DEVICE_CLASSIFY
{
    DEVICE_CLASSIFY_OTHER = 0,                                      // 其他
    DEVICE_CLASSIFY_IMMUNE = 1,                                     // 免疫
    DEVICE_CLASSIFY_CHEMISTRY = 2,                                  // 生化
    DEVICE_CLASSIFY_ISE = 4,                                        // 电解质
    DEVICE_CLASSIFY_BLOOD = 8,                                      // 血球
    DEVICE_CLASSIFY_URINE = 16,                                     // 尿液
    DEVICE_CLASSIFY_CRUOR = 32                                      // 凝血
};

// 质控申请信息
typedef struct tagQcApplyInfo
{
    bool                    bSelected;                                      // 是否选择
    int                     iModuleNo;                                      // 模块号
    int                     iBackupRgntPos;                                 // 备用瓶试剂位置
    QString                 strDevName;                                     // 设备名
    QString                 strDevGrpName;                                  // 设备的组名
    QString                 strQcDocID;                                     // 质控品ID
    QString                 strID;                                          // ID标识
    QString                 strManApplyID;                                  // 质控手工申请ID标识
    QString                 strDefApplyID;                                  // 质控默认申请ID标识
    QString                 strModuleName;                                  // 模块名称
    QString                 strPos;                                         // 位置
    QString                 strAssayName;                                   // 项目名称
    QString                 strAssayCode;                                   // 项目编号
    QString                 strRgntUseStatus;                               // 使用状态
    QString                 strRgntNo;                                      // 试剂瓶号
    QString                 strRgntLot;                                     // 试剂批号
    QString                 strQcNo;                                        // 质控编号
    QString                 strQcName;                                      // 质控名称
    QString                 strQcBriefName;                                 // 质控品简称
    QString                 strQcSourceType;                                // 质控品类型
    QString                 strQcLevel;                                     // 质控品水平
    QString                 strQcLot;                                       // 质控品批号
    QString                 strQcReason;                                    // 质控原因
    QString                 strQcExpDate;                                   // 失效日期
} QC_APPLY_INFO;

// 质控靶值信息查询条件
typedef struct tagQcDocConcInfoQueryCond
{
    QString                 strAssayName;                                   // 项目名称
    QDate                   startDate;                                      // 开始时间
    QDate                   endDate;                                        // 结束时间
} QC_CONC_INFO_QUERY_COND;

// 质控靶值信息
typedef struct tagQcDocConcInfo
{
    QString                 strID;                                          // ID标识
    QString                 strQcNo;                                        // 质控品编号
    QString                 strQcName;                                      // 质控品名称
    QString                 strQcBriefName;                                 // 质控品简称
    QString                 strQcSourceType;                                // 质控品类型
    QString                 strQcLevel;                                     // 质控品水平
    QString                 strQcLot;                                       // 质控品批号
    double					dQcTargetVal;									// 靶值
	double					dQcSD;											// SD
	double					dQcCV;											// CV%
	double					dQcCalcTargetVal;								// 计算靶值
	double					dQcCalcSD;										// 计算SD
	double					dQcCalcCV;										// 计算CV%
    QString                 strQcRltCount;                                  // 数量
} QC_DOC_CONC_INFO;

// 质控结果信息查询条件
typedef struct tagQcResultQueryCond
{
    QString                 strAssayName;                                   // 项目名称
    QString                 strQcDocID;                                     // 质控品ID
    QDate                   startDate;                                      // 开始时间
    QDate                   endDate;                                        // 结束时间
} QC_RESULT_QUERY_COND;

// 质控结果信息
typedef struct tagQcResultInfo
{
    bool                    bCalculated;                                    // 是否计算
    QString                 strID;                                          // ID标识
    QString                 strQcDocID;                                     // 质控文档ID
    QString                 strAssayName;                                   // 项目名
    QString                 strRltDetailID;                                 // 结果详情ID
    QString                 strQcTime;                                      // 质控时间
    double                  dQcResult;                                      // 结果
    QC_STATE                enQcState;                                      // 状态
	QString                 strQcReason;                                    // 质控原因
    QString                 strOutCtrlRule;                                 // 失控规则
    double                  dQcTargetVal;                                   // 靶值
    double                  dQcSD;                                          // 标准差
    QString                 strOutCtrlReason;                               // 失控原因
    QString                 strSolution;                                    // 处理措施
    QString                 strOperator;                                    // 操作者
} QC_RESULT_INFO;

// 联合质控结果信息
typedef struct tagTwinQcResultInfo
{
    bool                    bCalculated;                                    // 是否计算
    bool                    bIsX;                                           // 是否是X
    int                     iUnionIdx;                                      // 联合质控索引
    QString                 strID;                                          // ID标识
    QString                 strRltDetailID;                                 // 结果详情ID
    QString                 strQcTime;                                      // 质控时间
    double                  dQcResult;                                      // 结果
    QC_STATE                enQcState;                                      // 状态
	QString                 strQcReason;                                    // 质控原因
    QString                 strOutCtrlRule;                                 // 失控规则
    QString                 strQcNo;                                        // 质控品编号
    QString                 strQcName;                                      // 质控品名称
	QString                 strQcBriefName;                                 // 质控品简称
    QString                 strQcSourceType;                                // 质控品类型
    QString                 strQcLevel;                                     // 质控品水平
    QString                 strQcLot;                                       // 质控品批号
    double                  dQcTargetVal;                                   // 靶值
    double                  dQcSD;                                          // 标准差
    QString                 strOutCtrlReason;                               // 失控原因
    QString                 strSolution;                                    // 处理措施
    QString                 strOperator;                                    // 操作者
} TWIN_QC_RESULT_INFO;

// 耗材需求量信息
typedef struct tagSplReqVolInfo
{
    long long               lReqID;                                         // 耗材需求表数据库主键
    int                     iSplCode;                                       // 耗材编码
    bool                    bIsRgnt;                                        // 是否是试剂
    int                     iAssayClassify;                                 // 设备类别
    QString                 strSplName;                                     // 耗材名称
    QMap<QString, int>      mapDevResidual;                                 // 模块余量分布
    QString                 strDevResidual;                                 // 模块余量分布
    QVector<int>            vecIReqDynCalcVal;                              // 动态计算(周一到周天)
    QVector<QString>        vecStrReqDynCalcVal;                            // 动态计算(周一到周天，带单位)
    QVector<int>            vecIReqVal;                                     // 需求量（周一到周天）
    QVector<QString>        vecStrReqVal;                                   // 需求量(周一到周天，有单位则带单位)
    int                     iCurResidual;                                   // 当前余量
    QString                 strCurResidual;                                 // 当前余量(带单位)
    QVector<int>            vecILackingVal;                                 // 需求缺口(周一到周天)
    QVector<QString>        vecStrLackingVal;                               // 需求缺口(周一到周天，带单位)
    int                     iBottleSpec;                                    // 瓶规格
    QString                 strBottleSpec;                                  // 瓶规格(带单位)
    QVector<QString>        vecStrBottleLacking;                            // 需载入瓶数(周一到周天)
} SPL_REQ_VOL_INFO;
