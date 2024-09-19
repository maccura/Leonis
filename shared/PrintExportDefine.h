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
/// @file     PrintExportDefine.h
/// @brief    免疫打印数据序列化接口
///
/// @author   6889/ChenWei
/// @date     2023年11月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 6889/ChenWei，2023年11月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <QString>
#include "src/thrift/gen-cpp/defs_types.h"

// 数据浏览样本导出信息
typedef struct SampleExportInfo
{
	QString     strSampleSendModle;     //进样模式
	QString     strSampleSourceType;    //样本源类型
	QString     strTubeType;            //样本管类型
	QString     strBarcode;             //样本条码
	QString     strSampleTypeStat;      //样本类别
    QString     strSampleNo;            //样本号
	QString     strRack	;               //架号
    QString     strPos;                 //位置
	QString		strAddReagent1Time;		//加R1时间
	QString		strAddSampleTime;		//加样本时间
	QString		strVortexTime1;			//混匀时间1
	QString		strAddReagent2Time;		//加R2时间
	QString		strVortexTime2;			//混匀时间2
    QString     strEndTestTime;         //检测完成时间
    QString     strItemName;            //项目名称
	QString     strFirstTestCounts;     //项目重复次数
    QString     strRUL;                 //信号值
	QString     strOrignialTestResult;  //原始结果    // 管理员无法导出
    QString     strTestResult;          //检测结果
	QString     strAbs;					//吸光度
	QString     strResultStatus;        //数据报警
    QString     strUnit;                //单位
	QString     strSuckVol;             //样本量
	QString     strPreDilutionFactor;   //手工稀释倍数(机外稀释)
	QString     strDilutionFactor;      //机内稀释倍数
	QString     strAIResult;            //AI识别结果
    QString     strTestModule;          //模块
	QString     strReagentDisk;         //试剂盘
	QString     strAssayCupBatchNo;     //反应杯号
    QString     strReagentBatchNo;      //试剂批号
    QString     strReagentSerialNo;     //试剂瓶号
	QString     strRgtRegisterT;        //试剂上机时间
	QString     strReagentExpTime;      //试剂失效日期
	QString     strOpenBottleExpiryTime;//开瓶有效期
	QString     strAcidityLot;			//酸性清洗液批号
	QString     strAlkalintyLot;		//碱性清洗液批号
	QString     strSampleProbeAcidityLot;//针酸性清洗液批号
	QString     strSampleProbeAlkalintyLot;//针碱性清洗液批号
	QString     strCaliName;             //校准品名称
	QString     strCaliLot;             //校准品批号
	QString     strCaliTime;            //校准时间
	QString     strAnalysisMethod;      //分析方法
	QString     strPrimaryWave;         //主波长
	QString     strDeputyWave;          //次波长
	QString     strReactionTime;        //反应时间
	QString     strDetectPoint;         //测光点
	QString     strPrimaryWaveAbs;      //主波长吸光度
	QString     strDeputyWaveAbs;       //次波长吸光度
	QString     strAllWaveAbs;          //所有波长吸光度
	QString     strCalcuLateEmf;        //计算电动势
	QString     strCaliCount;           //校准次数
	QString     strCaliLevel;           //校准水平
	QString     strConc;				//浓度
	QString     strIsLOt;				//内部标准液批号
	QString     strIsSn;				//内部标准液瓶号
	QString     strDiluentLot;			//AT缓冲液批号
	QString     strDiluentSn;			//AT缓冲液瓶号
	QString     strCaliType;            //校准方法
	QString     strCaliPoints;          //校准点数
	QString     strDilutionSampleVol;   //稀释样本量
	QString     strDiluentVol;			//稀释液量
	QString     strCalculateAbs;		//计算吸光度

	QString     strDiluentBatchNo;      //稀释液批号
    QString     strSubstrateBatchNo;    //底物液批号
    QString     strSubstrateBottleNo;   //底物液瓶号
    QString     strCleanFluidBatchNo;   //清洗缓冲液批号
    QString     strFirstCurCaliTime;    //当前工作曲线(首次)
	QString     strTestModle;           //检测模式
	QString     strOrignialRUL;         //原始信号值   // 管理员无法导出
    QString     strReTestTubeType;      //复查样本管类型
    QString     strReTestRUL;           //复查信号值
    QString     strReTestOriglRUL;      //复查原始信号值     // 管理员无法导出
    QString     strReTestResult;        //复查结果
    QString     strReOriglTestResult;   //复查原始结果      // 管理员无法导出
    QString     strReResultStatus;      // 复查结果状态
    QString     strReAIResult;          //复查AI识别结果
    QString     strReTestModule;        //模块
    QString     strRePreDilFactor;      //复查手工稀释倍数
    QString     strReDilFactor;         //复查机内稀释倍数
    QString     strReEndTestTime;       //复查完成时间
    QString     strReTestModle;         //复查检测模式
    QString     strReSampleSendModle;     //复查进样模式
    QString     strReReagentBatchNo;      //试剂批号
    QString     strReReagentSerialNo;     //试剂瓶号
    QString     strReSubBatchNo;        //底物液批号(复查)
    QString     strReSubBottleNo;       //底物液瓶号(复查)
    QString     strReCleanFluidBatchNo;   //清洗缓冲液批号
    
    QString     strReAssayCupBatchNo;       //反应杯批号(复查)
    QString     strReDiluentBatchNo;      //稀释液批号
    QString     strReRgtRegisterT;        //试剂上机时间
    QString     strReReagentExpTime;      //试剂失效日期 
    QString     strReCaliLot;             //校准品批号
    QString     strReTestCurCaliTime;    //当前工作曲线(复查)
    QString     strReTestCounts;        //复查项目重复次数

} SAMPLEEXPORTINFO;
typedef std::vector<SAMPLEEXPORTINFO> ExpSampleInfoVector;

struct SampleExportModule
{
    std::string     strExportTime;                      // 导出时间
    std::string     strTitle;                           // 标题
    std::vector<SampleExportInfo> vecResults;

    SampleExportModule()
    {
        strExportTime = "";
        strTitle = "";
    }
};

class SEMetaInfo 
{
public:
    SEMetaInfo(const QString& name, QString SampleExportInfo::*pointer, tf::UserType::type UserType)
    {
        m_strName = name;
        m_Field = pointer;
        m_UserType = UserType;
    }

    SEMetaInfo(const SEMetaInfo& rhs)
    {
        m_strName = rhs.m_strName;
        m_Field = rhs.m_Field;
        m_UserType = rhs.m_UserType;
    }

    SEMetaInfo& operator=(const SEMetaInfo& rhs)
    {
        m_strName = rhs.m_strName;
        m_Field = rhs.m_Field;
        m_UserType = rhs.m_UserType;
        return *this;
    }

    SEMetaInfo(SEMetaInfo&& rhs) noexcept
    {
        m_strName = rhs.m_strName;
        m_Field = rhs.m_Field;
        m_UserType = rhs.m_UserType;
        rhs.m_Field = nullptr;
    }

    SEMetaInfo& operator=(SEMetaInfo&& rhs) noexcept
    {
        m_strName = rhs.m_strName;
        m_Field = rhs.m_Field;
        m_UserType = rhs.m_UserType;
        rhs.m_Field = nullptr;
        return *this;
    }

    ~SEMetaInfo()
    {
    }

public:
    QString m_strName;
    QString SampleExportInfo::* m_Field;
    tf::UserType::type          m_UserType;     // 用户权限
};

// 项目信息
typedef struct ItemInfo
{
    std::string     strIndex;                   ///< 序号
    std::string     strItemName;                ///< 项目名称
    std::string     strShortName;               ///< 简称
    std::string     strUnit;                    ///< 单位
    std::string     strResult;                  ///< 结果
    std::string     strResultState;             ///< 结果状态（“↓”或者“↑”）
    std::string     strRefRange;                ///< 参考范围
    std::string     strModule;                  ///< 检测模块

    ItemInfo()
    {
        strIndex = " ";
        strItemName = " ";
        strShortName = " ";
        strUnit = " ";
        strResult = " ";
        strResultState = " ";
        strRefRange = " ";
        strModule = " ";
    };

} ITEMINFO;

// 样本信息
typedef struct SampleInfo
{
    std::string     strPatientName;             ///< 姓名
    std::string     strGender;                  ///< 性别
    std::string     strAge;                     ///< 年龄
    std::string     strSourceType;              ///< 样本类型
    std::string     strSequenceNO;              ///< 样本号
    std::string     strSampleID;                ///< 样本ID
    std::string     strCaseNo;                  ///< 病历号
    std::string     strDepartment;              ///< 科室
    std::string     strDoctor;                  ///< 主治医师
    std::string     strClinicalDiagnosis;       ///< 临床诊断
    std::string     strInpatientWard;           ///< 病区
    std::string     strBedNo;                   ///< 病床号
    std::string     strCompTime;                ///< 检测完成时间
    
    std::string     strInspectionPersonnel;     ///< 检测者
    std::string     strDateAudit;               ///< 审核时间
    std::string     strReviewers;               ///< 审核者
    std::string     strInstrumentModel;         // 仪器型号

    std::string     strTitle;                   // 标题
    std::string     strNotes;                   // 备注
    std::string     strStateMent;               // 声明
    std::string     strPrintTime;               // 打印时间

    std::vector<ITEMINFO> vecItems;
    SampleInfo()
    {
        strPatientName = "";
        strGender = "";
        strAge = "";
        strSourceType = "";
        strSequenceNO = "";
        strSampleID = "";
        strCaseNo = "";
        strDepartment = "";
        strDoctor = "";
        strBedNo = "";
        strCompTime = "";
        strInpatientWard = "";
        strClinicalDiagnosis = "";
        strInspectionPersonnel = "";
        strDateAudit = "";
        strReviewers = "";
        strInstrumentModel = "";
        strTitle = "";
        strNotes = "";
        strStateMent = "";
        strPrintTime = "";
    }

} SAMPLEINFO;
typedef std::vector<SAMPLEINFO> SampleInfoVector;

// 项目结果
typedef struct ItemResult
{
    std::string     strSequenceNO;                      ///< 样本号
    std::string     strSampleID;                        ///< 样本条码
    std::string     strItemName;                        ///< 项目名称
    std::string     strResult;                          ///< 检测结果
    std::string     strCompTime;                        ///< 检测完成时间
    std::string     strModule;                          ///< 检测模块

    ItemResult()
    {
        strSequenceNO = "";
        strSampleID = "";
        strItemName = "";
        strResult = "";
        strCompTime = "";
        strModule = "";
    }
} ITEMRESULT;

// 按样本打印
typedef struct SimpleSampleInfo
{
    std::string     strInstrumentModel;                 // 仪器型号
    std::string     strPrintTime;                       // 打印时间
    std::string     strTitle;                           // 标题
    std::vector<ItemResult> vecItemResults;

    SimpleSampleInfo()
    {
        strInstrumentModel = "";
        strPrintTime = "";
        strTitle = "";
    }

} SIMPLESAMPLEINFO;
typedef std::vector<SimpleSampleInfo> SimpleSampleInfoVector;

// 项目样本结果
// 结果类型
typedef enum emResultMode
{
    PRINT_RESULT_MODE_OTHER = 0x00,
    PRINT_RESULT_MODE_FIRST = 0x01,        ///< 首次
    PRINT_RESULT_MODE_RERUN = 0x02,        ///< 复查
    PRINT_RESULT_MODE_ALL = 0x03,          ///< 首次和复查
} EM_RESULT_MODE;

typedef struct ItemSampleResult
{
    std::string     strSampleID;                            // 样本条码
    std::string     strSampleNumber;                        // 样本号
    std::string     strItemName;                            // 项目名称
    std::string     strResult;                              // 检测结果
    std::string     strUnit;                                // 单位
    std::string     strRLU;                                 // 信号值
    std::string     strDetectionTime;                       // 检测完成时间
    //std::string     strReagentLot;                          // 试剂批号
    //std::string     strReagentValidityDate;                 // 试剂有效期
    //std::string     strModule;                              // 检测模块

    ItemSampleResult()
    {
        strSampleNumber = "";
        strSampleID = "";
        strItemName = "";
        strResult = "";
        strUnit = "";
        strRLU = "";
        strDetectionTime = "";
       /* strReagentLot = "";
        strReagentValidityDate = "";
        strModule = "";*/
    };
}ITEMSAMPLERESULT;
typedef std::vector<ITEMSAMPLERESULT> ItemSampleResultVector;

typedef struct ItemSampleInfo
{
    std::string strFactory;                         // 试剂厂家
    std::string strInstrumentModel;                 // 仪器型号
    std::string strTemperature;                     // 实验室温度
    std::string strHumidness;                       // 实验室湿度
    std::string strPrintTime;                       // 打印时间
    std::string strTitle;                           // 标题
    ItemSampleResultVector vecResult;               // 样本结果
    ItemSampleInfo()
    {
        strFactory = "";
        strInstrumentModel = "";
        strTemperature = "";
        strHumidness = "";
        strPrintTime = "";
        strTitle = "";
    };
}ITEMSAMPLEINFO;
typedef std::vector<ITEMSAMPLEINFO> ItemSampleInfoVector;

// 试剂信息
typedef struct ReagentItem
{
    long                            lRecordId;                  // 数据库对应ID
    unsigned int                    uAssayCode;                 // 项目通道号
    std::string                     strReagentPos;              // 试剂位置
    std::string                     strReagentName;             // 项目名称
    std::string                     strValidityTestNum;         // 可用测试数(若为稀释液，则表示稀释液规格 ul)
    std::string                     strResidualTestNum;         // 剩余测试数（试剂余量小于等于报警值时“剩余测试数”单元格黄色填充，余量为0时“剩余测试数”单元格红色填充）
    std::string                     strUseStatus;               // 使用状态（包括空、正在使用、备用3种。试剂用完，即“剩余测试数”为“0”，则使用状态为空）
    std::string                     strCaliStatus;              // 校准状态（空、未校准、正在校准、校准成功、校准失败5种，与该瓶试剂最后一次校准的状态保持一致。未校准过但有批工作曲线的试剂校准状态显示为空；未校准过且没有任何工作曲线的试剂校准状态显示为未校准。未校准、校准失败时“校准状态”单元格红色填充）
    std::string                     strReagentLot;              // 试剂批号
    std::string                     strReagentSN;               // 试剂序号
    std::string                     strCaliValidityDays;        // 校准曲线有效期（2. 该瓶试剂当前工作曲线是否过期，即“当前曲线有效期”是否为“B：0”或“L：0”，若是，则试剂信息界面“当前曲线有效期”单元格红色填充，试剂概况界面该瓶试剂为警告颜色）
    std::string                     strBottleValidityDays;      // 开瓶有效天数（开瓶过期时“开瓶有效期”单元格红色填充）
    std::string                     strReagentValidityDays;     // 试剂有效天数(稀释液稳定天数)（有效期过期时“试剂有效期”单元格红色填充）
    std::string                     strReagentValidityDate;     // 试剂有效期
    std::string                     strRegisterDate;            // 注册日期(实为"上机时间"或"扫描时间")(1. 若该瓶试剂未校准过，有批曲线，距上机时间超过7天，则该瓶试剂校准过期，试剂信息界面“上机时间”单元格红色填充，试剂概况界面该瓶试剂为警告颜色；若该瓶试剂校准过，距上次校准成功超过7天，则该瓶试剂校准过期，试剂信息界面“上机时间”单元格红色填充，试剂概况界面该瓶试剂为警告颜色。)
    ReagentItem()
    {
        lRecordId = 0;
        uAssayCode = 0;
        strReagentPos = "";
        strReagentName = "";
        strValidityTestNum = "";
        strResidualTestNum = "";
        strUseStatus = "";
        strCaliStatus = "";
        strReagentLot = "";
        strReagentSN = "";
        strCaliValidityDays = "";
        strBottleValidityDays = "";
        strReagentValidityDays = "";
        strReagentValidityDate = "";
        strRegisterDate = "";
    }

}REAGENTITEM;
typedef std::vector<REAGENTITEM> ReagentItemVector;

typedef struct ReagentInfo
{
    std::string strInstrumentModel;                 // 仪器型号
    std::string strPrintTime;                       // 打印时间
    std::string strTitle;                           // 标题
    ReagentItemVector vecReagent;

    ReagentInfo()
    {
        strInstrumentModel = "";
        strPrintTime = "";
        strTitle = "";
    }

}REAGENTINFO;

///     需求计算
typedef struct RequireCalcItem
{
    std::string strName;                    // 名称
    std::string strModuleRemain;            // 模块余量分布
    std::string strDynamicCalc;             // 动态计算
    std::string strReqVol;                  // 需求量
    std::string strRemainVol;               // 当前余量
    std::string strReqLack;                 // 需求缺口
    std::string strBottleSpeciffic;         // 瓶规格
    std::string strReqBottleCount;          // 需载入瓶数
};

typedef struct RequireCalcInfo
{
    std::string         strPrintTime;                       // 打印时间
    std::string         strTitle;                           // 标题

    std::vector<RequireCalcItem> ItemVector;
};

///     耗材信息
///
typedef struct SupplyItem
{
    std::string                     strType;                   // 类型
    std::string                     strLotNumber;              // 批号
    std::string                     strSerialNumber;           // 序列号
    std::string                     strRemainQuantity;         // 剩余量    
    std::string                     strExpirationDate;         // 失效日期
    std::string                     strLoadDate;               // 加载日期
    std::string                     strUsageStatus;            // 使用状态
    std::string                     strOpenEffectDays;         // 开瓶有效期
    std::string                     strModule;                 // 模块
    SupplyItem()
    {
        strType = "";
        strLotNumber = "";
        strSerialNumber = "";
        strRemainQuantity = "";
        strExpirationDate = "";
        strLoadDate = "";
        strUsageStatus = " ";
        strOpenEffectDays = " ";
        strModule = " ";
    }
}SUPPLY_ITEM;
typedef std::vector<SUPPLY_ITEM> SupplyItemVector;

typedef struct SupplyInfo
{
    std::string         strInstrumentModel;                 // 仪器型号
    std::string         strPrintTime;                       // 打印时间
    std::string         strTitle;                           // 标题
    SupplyItemVector    vecSupply;

    SupplyInfo()
    {
        strInstrumentModel = "";
        strPrintTime = "";
        strTitle = "";
    }
}SUPPLYINFO;

// 校准结果数据
typedef struct CaliResultData
{
    std::string strLevel;       // 校准品水平
    std::string strSign;          // 信号值
    std::string strConc;          // 浓度值

    CaliResultData()
    {
        strLevel = " ";
        strSign = " ";
        strConc = " ";
    }
}CALIRESULTDATA;

///
/// @brief
///     校准品测试信息
///
typedef struct CaliRLUInfo
{
    std::string                     strCalibrator;             ///< 校准品
    std::string                     strConc;                   ///< 浓度
    std::string                     strRLU1;                   ///< RLU1
    std::string                     strRLU2;                   ///< RLU2
    std::string                     strRLU;                    ///< RLU
    std::string                     strCV;                     ///< CV（CV,K1,K2,K如果在应用项目校准设置中，设置了非0-0，则保留4位小数显示）
    std::string                     strK1;                     ///< K1
    std::string                     strK2;                     ///< K2
    std::string                     strK;                      ///< K

    CaliRLUInfo()
    {
        strCalibrator = "";
        strConc = "";
        strRLU1 = "";
        strRLU2 = "";
        strRLU = "";
        strCV = "";
        strK1 = "";
        strK2 = "";
        strK = "";
    }
} CALI_RLU_INFO;

// 校准结果信息
typedef struct CaliResultInfo
{
    std::string     strDeviceNum;               ///< 仪器编号（为机身号）
    std::string     strSoftVersion;             ///< 软件版本号
    std::string     strModelName;               ///< 模块名称

    std::string     strName;                    ///< 项目名称
    std::string     strCalibrateDate;           ///< 校准日期
    std::string     strCalibratorLot;           ///< 校准品批号
    std::string     strCalibratorPos;           ///< 校准品位置
    std::string     strReagentLot;              ///< 试剂批号
    std::string     strReagentSN;               ///< 试剂瓶号
    std::string     strSubstrateLot;            ///< 底物液批号
    std::string     strSubstrateSN;             ///< 底物液瓶号
    std::string     strCleanFluidLot;           ///< 清洗缓冲液批号
    std::string     strCleanFluidSN;            ///< 清洗缓冲液编号
    std::string     strCupLot;                  ///< 反应杯批号
    std::string     strCupSN;                   ///< 反应杯序列号

    std::string     strCutoff;                  ///< cutoff值（定量项目留空）
    std::string     strAlarm;                   ///< 告警
    std::string     strCaliResult;              ///< 校准结果
    std::string     strOperater;                ///< 检测者
    std::string     strCaliCurve;               ///< 校准曲线
    std::string     strCaliCurveImage;          ///< 校准曲线(图片)
    std::string     strPrintTime;               ///< 打印时间
    std::string     strTitle;                   ///< 标题

    std::vector<CaliResultData>     vecCaliResultData;          ///< 校准水平对应数据
    std::vector<CaliRLUInfo>        vecCaliRLUInfo;             ///< 校准结果的信息值信息

    CaliResultInfo()
    {
        strName = "";
        strCalibratorLot = "";
        strReagentLot = "";
        strReagentSN = "";
        strSubstrateLot = "";
        strSubstrateSN = "";
        strCleanFluidLot = "";
        strCleanFluidSN = "";
        strCupLot = "";
        strCupSN = "";
        strDeviceNum = "";
        strSoftVersion = "";
        strCutoff = "";
        strCaliResult = "";
        strCalibrateDate = "";
        strOperater = "";
        strPrintTime = "";
        strTitle = "";
        strAlarm = "";
    }
}CALIRESULTINFO;

typedef struct ChCaliParameters
{
    std::string	strCaliParameter;	//校准参数
    std::string	strParameterValue;	//结果
}CHCALIPARAMETERS;

typedef struct ChCaliResultData
{
    std::string	strLevel;	        //校准品水平
    std::string	strCalibratorPos;	//校准品位置
    std::string	strFirstCup;	    //首次杯号
    std::string	strFirstCupPreAssay;//首次同杯前反应
    std::string	strSecCup;	        //第二次杯号
    std::string	strSecCupPreAssay;	//第二次同杯前反应
    std::string	strAbs1;	        //吸光度1
    std::string	strAbs2;	        //吸光度2
    std::string	strAvgAbs;	        //吸光度均值
    std::string	strConc;	        //浓度
}CHCALIRESULTDATA;

// 生化校准结果
typedef struct CaliResultInfoCh {
    std::string	strModelName;	    //模块
    std::string	strItemName;	    //项目名称
    std::string	strUnit;	        //单位
    std::string	strDeviceNum;	    //仪器编号
    std::string	strSoftVersion;	    //软件版本
    std::string	strReagentLot;	    //试剂批号
    std::string	strReagentSN;	    //试剂瓶号
    std::string	strCalibratorLot;	//校准品批号
    std::string	strCalibrateDate;	//校准时间
    std::string	strAcidityLot;	    //酸性清洗液批号
    std::string	strAlkalinityLot;	//碱性清洗液批号
    std::string	strCaliType;	    //校准方法
    std::string	strCaliMode;	    //执行方法
    std::string	strAlarm;	        //数据报警
    std::string	strExportBy;	    //导出人
    std::string	strPrintBy;	        //打印人
    //std::string	strCaliCurveImage;	//校准曲线(图片)
    std::string	strPrintTime;	    //打印时间
    std::string	strExportTime;	    //导出时间

    std::vector<ChCaliParameters> vecCaliParameters;
    std::vector<ChCaliResultData> vecCaliResultData;
}CALIRESULTINFOCH;

typedef struct IseCaliResultData
{
    std::string strAssayName;       //项目名称
    std::string strLowSamleEmf;     //低值
    std::string strLowBaseEmf;      //低值基准
    std::string strHighSamleEmf;    //高值
    std::string strHighBaseEmf;     //高值基准
    std::string strSlope;           //斜率
    std::string strDilutionRatio;   //稀释倍数
    std::string strAlarm;           //数据报警
}ISECALIRESULTDATA;

//ISE校准结果
typedef struct CaliResultInfoIse {
    std::string strReagentName;     //项目名称
    std::string strPos;             //位置
    std::string strCalibrateDate;   //校准时间
    std::string strUnit;            //单位
    std::string strCalibratorLot;   //校准品批号
    std::string strCalibrator1;     //高浓度校准品位置
    std::string strCalibrator2;     //低浓度校准品位置
    std::string strIsLot;           //内部标准液批号
    std::string strDiluLot;         //AT缓冲液批号
    std::string strDeviceNum;       //仪器编号
    std::string strSoftVersion;     //软件版本号
    std::string strCaliResult;      // 校准结果（成功/失败）
    std::string strTitle;           //校准结果(标题)
    std::string strModelName;       //模块
    std::string strPrintTime;       //打印时间
    std::string strExportTime;      //导出时间
    std::string strPrintBy;         //打印人
    std::string strExportBy;        //导出人

    std::vector<IseCaliResultData> vecCaliResultData; //校准数据
}CALIRESULTINFOISE;

// 校准品反应数据
typedef struct CaliCurveLvData
{
    std::string strPonitIndex; //测光点
    std::string strPrimaryWave; //主波长
    std::string strSubWave; //次波长
    std::string strWaveDiff; //主-次波长
}CALICURVELVDATA;

// 反应曲线次序数据
typedef struct CaliCurveTimesData
{
    std::string strCaliSeqNo; //校准品次序：
    //std::string strReactCurveImage; //反应曲线(图片)

    std::vector<CaliCurveLvData> vecCaliLvData; //校准品反应数据	
}CALICURVETIMESDATA;

// 生化反应曲线
typedef struct ReactCurveData
{
    std::string strTitle; //标题
    std::string strReagentName; //项目名称:
    std::string strModelName; //模块:
    std::string strCaliName; //校准品名称:
    std::string strReagentLot; //试剂批号:
    std::string strReagentSN; //试剂瓶号:
    std::string strCaliMode; //执行方法:
    std::string strPrintTime; //打印时间：
    std::string strExportTime; //导出时间：

    std::vector<CALICURVETIMESDATA> vecCaliTimesData; //反应曲线次序数据
    
}REACTCURVEDATA;

// 生化校准品反应数据	
struct ChCaliHisLvData
{
    std::string strCalibrator; //校准品
    std::string strCalibratorPos; //校准品位置
    std::string strFirstCup; //首次杯号
    std::string strFirstCupPreAssay; //首次同杯前反应
    std::string strSecCup; //第二次杯号
    std::string strSecCupPreAssay; //第二次同杯前反应
    std::string strAbs1; //吸光度1
    std::string strAbs2; //吸光度2
    std::string strAvgAbs; //吸光度均值
    std::string strConc; //浓度
};

struct ChCaliHisTimesData
{
    std::string strCaliSeqNo; //校准次序：
    std::string strCalibratorLot; //校准品批号
    std::string strReagentLot; //试剂批号
    std::string strReagentSN; //试剂瓶号
    std::string strCalibrateDate; //校准时间
    std::string strCaliResult; //校准结果
    std::string strCaliMode; //执行方法
    std::string strAlarm; //数据报警
    std::vector<ChCaliHisLvData> vecCaliLvData; // 生化校准品反应数据
};

// 生化校准历史
struct CaliHistoryInfoCh
{
    std::string strTitle; //校准历史
    std::string strName; //项目名称:
    std::string strUnit; //单位:
    std::string strExportTime; //导出时间:
    std::string strPrintTime; //打印时间:
    std::string strModelName; //模块：
    std::vector<ChCaliHisTimesData> grpTimesData; //曲线次序数据
};

// ISE校准品反应数据
struct IseCaliHisLvData {
    std::string strCalibrator; //校准品
    std::string strConc; //浓度
    std::string strEmf; //电动势
    std::string strBase; //电动势
};

struct IseCaliHisTimesData
{
    std::string strCaliSeqNo; //校准次序：
    std::string strCalibratorLot; //校准品批号
    std::string IsLot; //内部标准液批号
    std::string IsSn; //内部标准液瓶号
    std::string strDiluLot; //AT缓冲液批号
    std::string strDiluSn; //AT缓冲液瓶号
    std::string strCalibrateDate; //校准时间
    std::string strCaliResult; //校准结果:
    std::string strAlarm; //数据报警
    std::string strSlop; //斜率值
    std::vector<IseCaliHisLvData> vecCaliLvData; // ISE校准品反应数据
};

// ISE校准历史
struct CaliHistoryInfoIse
{
    std::string strTitle; //校准历史
    std::string strName; //项目名称:
    std::string strUnit; //单位:
    std::string strExportTime; //导出时间:
    std::string strPrintTime; //打印时间:
    std::string strPos; //位置：
    std::vector<IseCaliHisTimesData> vecCaliTimesData; // ISE校准品反应数据
};

// 校准架概况
typedef struct CaliRack
{
    std::string strItemName;        // 项目名称
    std::string strCaliName;        // 校准品名称
    std::string strCaliLot;         // 校准品批号
    std::string strExpirationDate;  // 失效日期
    std::string strCalibrator1;     // 校准品1
    std::string strPos1;            // 位置1
    std::string strCalibrator2;     // 校准品2
    std::string strPos2;            // 位置2
    std::string strCalibrator3;     // 校准品3
    std::string strPos3;            // 位置3
    std::string strCalibrator4;     // 校准品4
    std::string strPos4;            // 位置4
    std::string strCalibrator5;     // 校准品5
    std::string strPos5;            // 位置5
    std::string strCalibrator6;     // 校准品6
    std::string strPos6;            // 位置6
}CALIRACK;

typedef struct CaliRackInfo 
{
    std::string             strInstrumentModel;                 // 仪器型号
    std::string             strPrintTime;                       // 打印时间
    std::string             strTitle;                           // 标题
    std::vector<CaliRack>   vecCaliRack;

    CaliRackInfo()
    {
        strInstrumentModel = "";
        strPrintTime = "";
        strTitle = "";
    }

}CALIRACKINFO;

// 校准历史
typedef struct CaliHistoryInfo
{
    std::string     strName;                    ///< 项目名称
    std::string     strCalibrateDate;           ///< 校准时间
    std::string     strModelName;               ///< 模块
    std::string     strCalibratorLot;           ///< 校准品批号
    std::string     strReagentLot;              ///< 试剂批号
    std::string     strReagentSN;               ///< 试剂瓶号
    std::string     strRegisterDate;            ///< 试剂上机时间
    std::string     strSubstrateLot;            ///< 底物液批号
    std::string     strSubstrateSN;             ///< 底物液瓶号
    std::string     strCleanFluidLot;           ///< 清洗缓冲液批号
    std::string     strCleanFluidSN;            ///< 清洗缓冲液瓶号
    std::string     strCupLot;                  ///< 反应杯批号
    std::string     strCupSN;                   ///< 反应杯序列号

    std::string     strCalibrator1;             // 校准品1
    std::string     strConc1;                   // Cal-1浓度
    std::string     strSignalValue1;            // Cal-1主标信号值
    std::string     strC1_RUL1;                 // Cal-1 RLU1
    std::string     strC1_RUL2;                 // Cal-1 RLU2
    std::string     strC1_RUL;                  // Cal-1 RLU

    std::string     strCalibrator2;             // 校准品2
    std::string     strConc2;                   // Cal-2浓度
    std::string     strSignalValue2;            // Cal-2主标信号值
    std::string     strC2_RUL1;                 // Cal-2 RLU1
    std::string     strC2_RUL2;                 // Cal-2 RLU2
    std::string     strC2_RUL;                  // Cal-2 RLU

    std::string     strFailureReason;           // 失败原因
    std::string     strInstrumentModel;         // 仪器型号
    std::string     strPrintTime;               // 打印时间
    std::string     strTitle;                   // 标题

    CaliHistoryInfo()
    {
        strName = "";
        strCalibrateDate = "";
        strModelName = "";
        strCalibratorLot = "";
        strReagentLot = "";
        strReagentSN = "";
        strRegisterDate = "";
        strSubstrateLot = "";
        strSubstrateSN = "";
        strCleanFluidLot = "";
        strCleanFluidSN = "";
        strCupLot = "";
        strCupSN = "";

        strCalibrator1 = "";
        strConc1 = "";
        strSignalValue1 = "";
        strC1_RUL1 = "";
        strC1_RUL2 = "";
        strC1_RUL = "";

        strCalibrator2 = "";
        strConc2 = "";
        strSignalValue2 = "";
        strC2_RUL1 = "";
        strC2_RUL2 = "";
        strC2_RUL = "";
    }

}CALIHISTORYINFO;

// 校准设置记录
struct CaliRecord 
{
    std::string     strIndex;           // 序号
    std::string     strCaliName;        // 校准品名称
    std::string     strCaliLot;         // 校准品批号
    std::string     strExpirationDate;  // 失效日期
    std::string     strCalibrator1;     // 校准品1
    std::string     strCalibrator2;     // 校准品2
    std::string     strCalibrator3;     // 校准品3
    std::string     strCalibrator4;     // 校准品4
    std::string     strCalibrator5;     // 校准品5
    std::string     strCalibrator6;     // 校准品6
};

struct CaliRecordInfo
{
    std::string     strInstrumentModel;         // 仪器型号
    std::string     strPrintTime;               // 打印时间
    std::string     strTitle;                   // 标题
    std::vector<CaliRecord> vecRecord;

    CaliRecordInfo()
    {
        strInstrumentModel = "";
        strPrintTime = "";
        strTitle = "";
    }
};

// 生化校准设置记录
struct ChCaliRecord : public CaliRecord
{
    std::string     strCaliAssays;      // 校准项目（生化）
};

struct ChCaliRecordInfo
{
    std::string     strInstrumentModel;         // 仪器型号
    std::string     strPrintTime;               // 打印时间
    std::string     strTitle;                   // 标题
    std::vector<ChCaliRecord> vecRecord;

    ChCaliRecordInfo()
    {
        strInstrumentModel = "";
        strPrintTime = "";
        strTitle = "";
    }
};

// 单日质控结果
struct QCDailyResult
{
    std::string     strQCDate;                  ///< 质控时间
    std::string     strQCID;                    ///< 质控品编号
    std::string     strQCName;                  ///< 质控品名称
    std::string     strQCBriefName;             ///< 质控品简称
    std::string     strQCSourceType;            ///< 质控品类型
    std::string     strQCLevel;					///< 质控品水平
    std::string     strQCLot;                   ///< 质控品批号
    std::string     strItemName;                ///< 项目名称
    std::string     strResult;                  ///< 结果值
    std::string     strUnit;                    ///< 单位
    std::string     strTargetMean;              ///< 靶值
    std::string     strMaxRange;                ///< 参考范围高值(3SD)
    std::string     strMinRange;                ///< 参考范围低值(-3SD)
    std::string     strBreakRule;               ///< 失控规则

    QCDailyResult()
    {
        strQCDate = "";
        strQCID = "";
        strQCName = "";
        strQCBriefName = "";
        strQCSourceType = "";
        strQCLevel = "";
        strQCLot = "";
        strItemName = "";
        strResult = "";
        strUnit = "";
        strTargetMean = "";
        strMaxRange = "";
        strMinRange = "";
        strBreakRule = "";
    }
};

struct QCDailyInfo
{
    std::string                     strInstrumentModel;        // 仪器型号
    std::string                     strPrintTime;              // 打印时间
    std::string                     strTitle;                  // 标题

    std::vector<QCDailyResult>           vecResult;                 ///< 质控结果
    QCDailyInfo()
    {
        strInstrumentModel = "";
        strPrintTime = "";
        strTitle = "";
    }
};

// 质控结果
struct QCResult
{
    std::string     strDisplayIndex;            ///< 显示序号
    std::string     strQCDate;                  ///< 质控时间
    std::string     strResult;                  ///< 结果值

    QCResult()
    {
        strResult = "";
        strQCDate = "";
        strDisplayIndex = "";
    }
};

// 质控LJ图信息
typedef struct QCLJInfo
{
    std::string                     strModelName;              ///< 模块名称
    std::string                     strItemName;               ///< 项目名称
    std::string                     strQCDate;                 ///< 质控日期
    std::string                     strQCID;                   ///< 质控品编号
    std::string                     strQCName;                 ///< 质控品名称
    std::string                     strQcBriefName;            ///< 质控品简称
    std::string                     strQcSourceType;		   ///< 质控品类型
    std::string                     strQcLevel;				   ///< 质控品水平
    
    std::string                     strQCLot;                  ///< 质控品批号
    std::string                     strTargetMean;             ///< 目标靶值
    std::string                     strTargetSD;               ///< 目标标准方差(SD)
    std::string                     strTargetCV;               ///< 目标变异系数（CV）
    std::string                     strActualMean;             ///< 计算靶值
    std::string                     strActualSD;               ///< 计算标准方差(SD)
    std::string                     strActualCV;               ///< 计算变异系数（CV）
    std::string                     strQcRltCount;             ///< 数量
    
    std::string                     strLJ;                     ///< L-J图
    std::string                     strLJImage;                ///< L-J图片

    std::string                     strQCAnalysis;             ///< 质控分析
    std::string                     strPrinter;                ///< 打印人

    std::string                     strInstrumentModel;        // 仪器型号
    std::string                     strPrintTime;              // 打印时间
    std::string                     strTitle;                  // 标题

    std::string                     strChart;                  // 图表
    std::vector<QCResult>           vecResult;                 ///< 质控结果

    QCLJInfo()
    {
        strItemName = "";
        strQCName = "";
        strQcBriefName = "";
        strQcSourceType = "";
        strQcLevel = "";
        strQCID = "";
        strQCLot = "";
        strTargetMean = "";
        strTargetSD = "";
        strTargetCV = "";
        strActualMean = "";
        strActualSD = "";
        strActualCV = "";
        strLJ = "";
        strLJImage = "";
        strQCAnalysis = "";
        strPrinter = "";
        strInstrumentModel = "";
        strPrintTime = "";
        strTitle = "";
    }
}QC_LJ_INFO;


// 质控YD图信息
typedef struct QCYDInfo
{
    std::string                     strItemName;               ///< 项目名称
    std::string                     strTimeQuantum;            ///< 质控日期
    std::string                     strModelName;              ///< 模块
    std::string                     strTP;                     ///< T-P图
    std::string                     strTPImage;                ///< T-P图片
    std::string                     strQCAnalysis;             ///< 质控分析
    std::string                     strPrinter;                ///< 打印人
    std::string                     strInstrumentModel;        ///< 仪器型号
    std::string                     strPrintTime;              ///< 打印时间
    std::string                     strTitle;                  ///< 标题
    std::vector<QCResult>           vecResult;                 ///< 质控结果

    QCYDInfo()
    {
        strItemName = "";
        strTimeQuantum = "";
        strModelName = "";
        strTP = "";
        strTPImage = "";
        strQCAnalysis = "";
        strPrinter = "";
        strInstrumentModel = "";
        strPrintTime = "";
        strTitle = "";
    }
} QC_YD_INFO;

// 质控品记录
struct QCRecord
{
    std::string                     strDocNo;                  ///< 文档号
    std::string                     strQCID;                   ///< 质控品编号
    std::string                     strQCName;                 ///< 质控品名称
    std::string                     strQcBriefName;            ///< 质控品简称
    std::string                     strQcSourceType;		   ///< 质控品类型
    std::string                     strQcLevel;				   ///< 质控品水平
    std::string                     strQCLot;                  ///< 质控品批号
    std::string                     strExpirationDate;         ///< 失效日期
    std::string                     strRegMethod;              ///< 登记方式
    std::string                     strPos;                    ///< 位置/样本管

    QCRecord()
    {
        strDocNo = "";
        strQCID = "";
        strQCName = "";
        strQcBriefName = "";
        strQcSourceType = "";
        strQcLevel = "";
        strQCLot = "";
        strExpirationDate = "";
        strRegMethod = "";
        strPos = "";
    }
};

struct QCItemRecord
{
    std::string                     strInstrumentModel;        ///< 仪器型号
    std::string                     strPrintTime;              ///< 打印时间
    std::string                     strTitle;                  ///< 标题
    std::vector<QCRecord>             vecRecord;

    QCItemRecord()
    {
        strInstrumentModel = "";
        strPrintTime = "";
        strTitle = "";
    }
};

///
/// @brief
///     操作日志
///
struct OperationLogItem
{
    std::string strIndex;               // 序号
    std::string strUserName;            // 用户名
    std::string strOperationType;       // 操作类型
    std::string strRecord;              // 操作记录
    std::string strTime;                // 操作时间

    OperationLogItem()
    {
        strIndex = "";
        strUserName = "";
        strOperationType = "";
        strRecord = "";
        strTime = "";
    }
};

struct OperationLog
{
    std::string                     strInstrumentModel;        ///< 仪器型号
    std::string                     strPrintTime;              ///< 打印时间
    std::string                     strTitle;                  ///< 标题
    std::vector<OperationLogItem>             vecRecord;

    OperationLog()
    {
        strInstrumentModel = "";
        strPrintTime = "";
        strTitle = "";
    }
};

///
/// @brief
///     试剂（耗材）更换记录
///
struct ChangeRecord
{
    std::string strIndex;               // 序号
    std::string strModule;              // 模块
    std::string strName;                // 名称
    std::string strOperationType;       // 操作类型
    std::string strState;               // 状态
    std::string strUserName;            // 用户名
    std::string strLot;                 // 批号
    std::string strReagentSN;           // 瓶号/序列号
    std::string strTime;                // 时间

};

///
/// @brief
///     试剂（耗材）更换日志
///
struct ReagentChangeLog
{
    std::string strInstrumentModel;        ///< 仪器型号
    std::string strPrintTime;              ///< 打印时间
    std::string strTitle;                  ///< 标题

    std::vector<ChangeRecord>             vecRecord;
    ReagentChangeLog()
    {
        strInstrumentModel = "";
        strPrintTime = "";
        strTitle = "";
    }
};

//     报警信息
struct AlarmInfo
{
    std::string strIndex;                   // 序号
    std::string strModule;                  // 模块
    std::string strAlarmCode;               // 报警代码
    std::string strAlarmLevel;              // 报警级别
    std::string strAlarmName;               // 报警名称
    std::string strAlarmTime;               // 报警时间

    AlarmInfo()
    {
        strIndex = "";
        strModule = "";
        strAlarmCode = "";
        strAlarmLevel = "";
        strAlarmName = "";
        strAlarmTime = "";
    }
};

// 报警日志
struct AlarmInfoLog
{
    std::string strInstrumentModel;        ///< 仪器型号
    std::string strPrintTime;              ///< 打印时间
    std::string strTitle;                  ///< 标题

    std::vector<AlarmInfo>             vecRecord;
    AlarmInfoLog()
    {
        strInstrumentModel = "";
        strPrintTime = "";
        strTitle = "";
    }
};

///     维护日志
struct MaintenanceItem
{
    std::string     strIndex;                   ///< 序号 
    std::string     strGroupMaint;              ///< 组合维护
    std::string     strSingleMaint;             ///< 单项维护
    std::string     strMaintenanceStatus;       ///< 维护状态
    std::string     strMaintenaceDate;          ///< 维护完成时间
    std::string     strModule;                  ///< 模块
    std::string     strUserName;                ///< 用户名

    MaintenanceItem()
    {
        strIndex = "";
        strGroupMaint = "";
        strSingleMaint = "";
        strMaintenanceStatus = "";
        strMaintenaceDate = "";
        strModule = "";
        strUserName = "";
    }
};

struct MaintenanceLog
{
    std::string strInstrumentModel;        ///< 仪器型号
    std::string strPrintTime;              ///< 打印时间
    std::string strTitle;                  ///< 标题

    std::vector<MaintenanceItem>             vecRecord;
    MaintenanceLog()
    {
        strInstrumentModel = "";
        strPrintTime = "";
        strTitle = "";
    }
};

/// 杯空白测定结果
struct ChCupTestResult
{
	std::string     strCupNum;					// 杯号
	std::string     str340Value;				// 340波长值
	std::string     str380Value;				// 380波长值
	std::string     str405Value;				// 405波长值
	std::string     str450Value;				// 450波长值
	std::string     str480Value;				// 480波长值
	std::string     str505Value;				// 505波长值
	std::string     str546Value;				// 546波长值
	std::string     str570Value;				// 570波长值
	std::string     str600Value;				// 600波长值
	std::string     str660Value;				// 660波长值
	std::string     str700Value;				// 700波长值
	std::string     str750Value;				// 750波长值
	std::string     str800Value;				// 800波长值
};

struct ChCupBlankTestInfo
{
	std::string     strModel;					// 模块
	std::string     strPrintTime;               // 打印时间
	std::string     strTitle;                   // 标题
	std::string     strTestTime;                // 测定时间
	std::string     strValueType;               // 示值类型（ADC或ABS）
	std::string		strErrorCupNums;			// 异常反应杯号

	std::vector<ChCupTestResult> vecCupResult;	// 数据结果

	ChCupBlankTestInfo()
	{
		strModel = "";
		strPrintTime = "";
		strTitle = "";
	}
};

/// 杯空白计算结果
struct ChCupBlankCalculateInfo
{
	std::string     strModel;					// 模块
	std::string     strPrintTime;               // 打印时间
	std::string     strTitle;                   // 标题
	std::string     strTestTime1;               // 测定时间1
	std::string     strTestTime2;               // 测定时间2

	std::vector<ChCupTestResult> vecCupResult;	// 数据结果

	ChCupBlankCalculateInfo()
	{
		strModel = "";
		strPrintTime = "";
		strTitle = "";
	}
};

/// 光度计检查结果
struct ChPhotoCheckResult
{
	std::string     strWave;					// 波长
	std::string     strMeanValue;				// 均值
	std::string     strRangeValue;				// 极差
};

struct ChPhotoCheckResultInfo
{
	std::string     strModel;					// 模块
	std::string     strPrintTime;               // 打印时间
	std::string     strTitle;                   // 标题
	std::string     strCurTestTime;             // 当前测定时间
	std::string     strHisTestTime;             // 历史测定时间

	std::vector<ChPhotoCheckResult> vecCurResult;	// 当前数据结果
	std::vector<ChPhotoCheckResult> vecHisResult;	// 历史数据结果

	ChPhotoCheckResultInfo()
	{
		strModel = "";
		strPrintTime = "";
		strTitle = "";
	}
};

///     反应杯历史单行的数据
struct CupHistoryItem
{
    std::string strCupNum;              // 杯号
    std::string strStatus;              // 状态
    std::string strTestTimes;           // 总测试数
    std::string strTestingAssay;        // 正在检查的项目
    std::string strTestedAssay;         // 前十次项目
};

///     反应杯历史
struct CupHistoryInfo
{
    std::string strModule;              // 模块
    std::string strPrintTime;           // 打印时间

    std::vector<CupHistoryItem>     vecRecord;
};

///     温度信息
struct TemperatureInfo
{
    std::string strModule;              // 模块
    std::string strPartName;            // 部件名称
    std::string strCurTem;              // 当前温度
    std::string strTemControlRange;     // 温控范围
    std::string strTemControlStatu;     // 温控状态
};

struct TemperatureLog
{
    std::string strInstrumentModel;        ///< 仪器型号
    std::string strPrintTime;              ///< 打印时间
    std::string strTitle;                  ///< 标题

    std::vector<TemperatureInfo>             vecRecord;
    TemperatureLog()
    {
        strInstrumentModel = "";
        strPrintTime = "";
        strTitle = "";
    }
};

// 统计单元
struct StatisticalUnit
{
    std::string strModule;              // 模块
    std::string strUnitName;            // 单元名称
    std::string strPartName;            // 部件名称
    std::string strSN;                  // 序列号
    std::string strCurUseNum;           // 当前使用数目
    std::string strAllUseNum;           // 累计使用数目
    std::string strUseUpperLimit;       // 使用上限
    std::string strResetTime;           // 重置时间
};

// 计数
struct StatisticalInfo
{
    std::string strInstrumentModel;        ///< 仪器型号
    std::string strPrintTime;              ///< 打印时间
    std::string strTitle;                  ///< 标题

    std::vector<StatisticalUnit>             vecRecord;
    StatisticalInfo()
    {
        strInstrumentModel = "";
        strPrintTime = "";
        strTitle = "";
    }
};

// 液路
struct LiquidItem
{
    std::string strModule;              // 模块
    std::string strName;                // 名称
    std::string strStatus;              // 状态
};

struct LiquidInfo
{
    std::string strPrintTime;           // 打印时间

    std::vector<LiquidItem>             vecRecord;
};

// 下位机单元
struct LowerCpUnit
{
    std::string strIndex;                   // 序号
    std::string strModule;                  // 模块
    std::string strName;                    // 名称
    std::string strState;                   // 状态
};

struct LowerCpLog
{
    std::string strInstrumentModel;        ///< 仪器型号
    std::string strPrintTime;              ///< 打印时间
    std::string strTitle;                  ///< 标题

    std::vector<LowerCpUnit>             vecRecord;
    LowerCpLog()
    {
        strInstrumentModel = "";
        strPrintTime = "";
        strTitle = "";
    }
};

// 样本量统计
struct SampleStatistics
{
    std::string strModule;                  // 模块
    std::string strReagentLot;              // 试剂批号
    std::string strItemName;                // 项目名称
    std::string strTotal;                   // 样本总量
    std::string strConvSample;              // 常规样本量
    std::string strEmSample;                // 急诊样本量
    std::string strQc;                      // 质控
    std::string strCali;                    // 校准
};

// 样本统计和
struct SampleStatisticsSum
{
    std::string strType;                    // 类型
    std::string strTotalOrder;              // 样本量（订单总量）
    std::string strConvSample;              // 常规样本量
    std::string strEmSample;                // 急诊样本量
    std::string strQc;                      // 质控
    std::string strCali;                    // 校准
    SampleStatisticsSum()
    {
        strType = "";
        strTotalOrder = "";
        strConvSample = "";
        strEmSample = "";
        strQc = "";
        strCali = "";
    }
};

struct SampleStatisticsInfo
{
    std::string strInstrumentModel;         // 仪器型号
    std::string strTitle;                   // 标题
    std::string strPrintTime;               // 打印时间
    std::string strDate;                    // 日期
    std::string strModule;                  // 模块
    std::string strReagentLot;              // 试剂批号

    std::vector<SampleStatisticsSum>          vecRecordSum;
    std::vector<SampleStatistics>             vecRecord;
    SampleStatisticsInfo()
    {
        strModule = "";
        strPrintTime = "";
        strTitle = "";
    }
};

// 试剂统计
struct ReagentStatistics
{
    std::string strIndex;                // 序号
    std::string strReagentCount;         // 试剂工作量
    std::string strSampleCount;          // 样本工作量
    std::string strRetestCount;          // 复查工作量
    std::string strQcCount;              // 质控工作量
    std::string strCaliCount;            // 校准工作量
};

struct ReagentStatisticsInfo
{
    std::string strInstrumentModel;        ///< 仪器型号
    std::string strTitle;                   // 标题
    std::string strModule;                  // 模块
    std::string strPrintTime;               // 打印时间
    std::string strDate;                    // 日期
    std::string strReagentName;             // 试剂名称
    std::string strImageTitle;              // 折线图
    std::string strImage;                   // 图片

    std::vector<ReagentStatistics>             vecRecord;

    ReagentStatisticsInfo()
    {
        strTitle = "";
        strModule = "";
        strPrintTime = "";
        strDate = "";
        strReagentName = "";
        strImageTitle = "";
        strImage = "";
    }
};

// 校准统计
struct CaliStatistics
{
    std::string strModule;                  // 模块
    std::string strItemName;                // 项目名称
    std::string strReagentLot;              // 试剂批号
    std::string strCaliNum;                 // 校准次数
};

struct CaliStatisticsInfo
{
    std::string strInstrumentModel;        ///< 仪器型号
    std::string strTitle;                   // 标题
    std::string strModule;                  // 模块
    std::string strPrintTime;               // 打印时间
    std::string strDate;                    // 日期
    std::string strItemName;                // 项目名称
    std::string strReagentLot;              // 试剂批号

    std::vector<CaliStatistics>             vecRecord;

    CaliStatisticsInfo()
    {
        strTitle = "";
        strModule = "";
        strPrintTime = "";
        strDate = "";
        strItemName = "";
        strReagentLot = "";
    }
};

