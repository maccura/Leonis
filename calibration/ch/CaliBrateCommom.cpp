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
/// @file     CaliBrateCommom.h
/// @brief    校准公共函数集合
///
/// @author   5774/WuHongTao
/// @date     2022年3月3日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月3日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "CaliBrateCommom.h"
#include "curveDraw/common.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "src/common/Mlog/mlog.h"
#include "shared/CommonInformationManager.h"

CaliBrateCommom::CaliBrateCommom()
{
}


CaliBrateCommom::~CaliBrateCommom()
{
}

///
/// @brief
///     创建曲线
///
/// @param[in]  curve  曲线信息
///
/// @return 成功返回曲线句柄否则为空
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月2日，新建函数
///
std::shared_ptr<ch::ICalibrateCurve> CaliBrateCommom::GetCurve(const ch::tf::CaliCurve& curve)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    shared_ptr<ch::ICalibrateCurve> currentCurve = nullptr;

    try
    {
        // 根据类型选择是否修改
//         if (curve.curveDataEdit.empty())
//         {
        ch::chRecoverCurve algParam(curve.curveData);
        currentCurve = ch::CreateCaliCurve(algParam);
//         }
//         else
//         {
//             currentCurve = ch::CreateCaliCurve(curve.curveDataEdit);
//         }
    }
    catch (exception& e)
    {
        ULOG(LOG_ERROR, "exception :  %s", e.what());
        return nullptr;
    }
    catch (...)
    {
        ULOG(LOG_ERROR, "exception");
        return nullptr;
    }

    if (currentCurve == nullptr)
    {
        ULOG(LOG_ERROR, "creat the caliCurve failed %s()", __FUNCTION__);
        return nullptr;
    }

    return currentCurve;
}

///
/// @brief
///     根据曲线句柄获取曲线的坐标点（离散）
///
/// @param[in]  curveHandle  曲线句柄
/// @param[out] ScatterCurve 可曲线
///
/// @return 成功true
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月3日，新建函数
///
bool CaliBrateCommom::CreatScatterCurve(curveSingleDataStore& ScatterCurve, shared_ptr<ch::ICalibrateCurve> curveHandle)
{
    bool ret = false;
    if (curveHandle == nullptr)
    {
        return ret;
    }

    // 设置曲线类型
    ScatterCurve.scatterMarkerShape = ScatterCurve.scatterMarkerShape;
    ScatterCurve.colorType = ScatterCurve.colorType;
    // 设置曲线离散状态
    ScatterCurve.IsPointerConnect = false;
    ScatterCurve.dataContent.clear();

    //临时值
    RealDataStruct tempdata;
    //获取曲线的点
    const auto& info = curveHandle->GetAlgorithmConfigs();
    auto points = info.origPoints;
    // 空白校准
    if (info.cali_mode == ch::CALI_MODE::CALI_MODE_BLANK && 
        info.spanedPoints.size() > 1 && points.size() > 1)
    {
        points[0] = info.spanedPoints[0];
    }

    //for(const auto& pointer : curveHandle->GetCalibratePoints())
    for (const auto& pointer : points)
    {
        ret = true;
        double x = std::get<0>(pointer);
        double y = std::get<1>(pointer);
        tempdata.xData = x;
        tempdata.yData = y;
        {
            tempdata.type = ScatterCurve.scatterMarkerShape;
            tempdata.color = ScatterCurve.colorType;
        }
        ScatterCurve.dataContent.append(tempdata);
    }

    return ret;
}

///
/// @brief
///     根据离散曲线获取模拟的连续曲线
///
/// @param[out] ContinueCurve 连续曲线 
/// @param[in]  ScatterCurve  离散曲线
///
/// @return 成功true
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月3日，新建函数
///
bool CaliBrateCommom::CreatContinueCurve(curveSingleDataStore& ContinueCurve, curveSingleDataStore& ScatterCurve, shared_ptr<ch::ICalibrateCurve> curveHandle)
{
    bool ret = false;
    if (ScatterCurve.dataContent.empty() || curveHandle == nullptr)
    {
        return ret;
    }

    //获取算法计算的曲线
    ContinueCurve.dataContent.clear();
    ContinueCurve.scatterMarkerShape = CALIBRATION;
    ContinueCurve.colorType = ScatterCurve.colorType;

    double firstValue = ScatterCurve.dataContent[0].xData;
    double lastValue = 0;
    double step = 0;

    //获取曲线的点
    const auto& info = curveHandle->GetAlgorithmConfigs();
    info.origPoints;
    // 空白校准
    if (info.cali_mode == ch::CALI_MODE::CALI_MODE_BLANK &&
        info.spanedPoints.size() > 1)
    {
        lastValue = std::get<0>(info.spanedPoints.back());
    }
    else if (info.cali_mode == ch::CALI_MODE::CALI_MODE_FULLDOT &&
        info.origPoints.size() > 1)
    {
        lastValue = std::get<0>(info.origPoints.back());
    }
    else
    {
        return ret;
    }

    //保证不除以0,计算步进
    if (MAXPOINTER > 0)
    {
        // lastValue = ScatterCurve.dataContent.back().xData;
        step = (lastValue - firstValue) / MAXPOINTER;
    }
    else
    {
        return ret;
    }

    //每次取多少点来画曲线
    RealDataStruct tempdata;
    for (size_t i = 0; i <= MAXPOINTER; i++)
    {
        if (i == MAXPOINTER)
        {
            tempdata.xData = lastValue;
        }
        else
        {
            tempdata.xData = firstValue + i*step;
        }

        tempdata.yData = curveHandle->CalculateAbsorbance(tempdata.xData);
        ContinueCurve.dataContent.append(tempdata);
    }

    return true;
}

///
/// @brief
///     根据结果，得到主曲线次曲线和主-次曲线
///
/// @param[in]  result  测试结果
/// @param[out]  CurveList  曲线列表，依次是主曲线，次曲线，主-次曲线
///
/// @return 成功true
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月3日，新建函数
///
bool CaliBrateCommom::GetScatterFromResult(ch::tf::AssayTestResult& result, std::vector<std::shared_ptr<curveSingleDataStore>>& CurveList)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 根据情况寻找主次波长
    if (result.primarySubWaves.size() <= 0)
    {
        return false;
    }

    shared_ptr<curveSingleDataStore> primWaveCurve = std::make_shared<curveSingleDataStore>();
    primWaveCurve->IsPointerConnect = false;
	primWaveCurve->colorType = ZERO;

    RealDataStruct tempdata;
    for (auto detectPoint : result.detectPoints)
    {
        // 添加主波长相关点信息
        tempdata.xData = detectPoint.pointSN;
        tempdata.yData = detectPoint.ods[result.primarySubWaves[0]];
        tempdata.type = CIRCLE;
        tempdata.color = ZERO;
        primWaveCurve->dataContent.append(tempdata);
    }
    CurveList.push_back(primWaveCurve);

    // 仅有主波长
    if (result.primarySubWaves.size() <= 1)
    {
        return true;
    }

    // 主次波长都有
    shared_ptr<curveSingleDataStore> secondWaveCurve = std::make_shared<curveSingleDataStore>();
    secondWaveCurve->IsPointerConnect = false;
	secondWaveCurve->colorType = ONE;

    shared_ptr<curveSingleDataStore> primSecondWaveCurve = std::make_shared<curveSingleDataStore>();
    primSecondWaveCurve->IsPointerConnect = false;
	primSecondWaveCurve->colorType = TWO;

    for (auto detectPoint : result.detectPoints)
    {
        // 添加次波长相关点信息
        tempdata.xData = detectPoint.pointSN;
        tempdata.yData = detectPoint.ods[result.primarySubWaves[1]];
        tempdata.type = CIRCLE;
        tempdata.color = ONE;
        secondWaveCurve->dataContent.append(tempdata);

        // 添加主-次波长相关点信息
        tempdata.yData = detectPoint.ods[result.primarySubWaves[0]] - detectPoint.ods[result.primarySubWaves[1]];
        tempdata.type = CIRCLE;
        tempdata.color = TWO;
        primSecondWaveCurve->dataContent.append(tempdata);
    }
    CurveList.push_back(secondWaveCurve);
    CurveList.push_back(primSecondWaveCurve);

    return true;
}

///
/// @brief
///     获取校准方法
///
/// @param[in]  cureveHandel  校准曲线的数据句柄
///
/// @return 校准方法字符串
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月23日，新建函数
///
QString CaliBrateCommom::GetCalibrateMethod(shared_ptr<ch::ICalibrateCurve> cureveHandel)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    QString method = "";
    if (cureveHandel == nullptr)
    {
        return method;
    }

    const auto& type = cureveHandel->GetAlgorithmConfigs();
    switch (type.analysis_method)
    {
        // 速率A法
        case ch::ANALYSIS_METHOD::ANALYSIS_METHOD_RATE_A:
            method = QObject::tr("rateA");
            break;

        // 一点法
        case ch::ANALYSIS_METHOD::ANALYSIS_METHOD_ONE_POINT:
            method = QObject::tr("onePointer");
            break;

        // 两点法
        case ch::ANALYSIS_METHOD::ANALYSIS_METHOD_TWO_POINTS:
            method = QObject::tr("twoPointer");
            break;

        // 两点速率法
        case ch::ANALYSIS_METHOD::ANALYSIS_METHOD_TWO_POINTS_RATE:
            method = QObject::tr("twoPointRate");
            break;

        // 速率A带样品空白修正法
        case ch::ANALYSIS_METHOD::ANALYSIS_METHOD_RATE_A_SAMPLE_BLANK:
            method = QObject::tr("rateA with sample blank");
            break;

        default:
            break;
    }

    return method;
}

///
/// @brief 获取校准类型
///
/// @param[in]  cureveHandel  校准曲线的数据句柄
///
/// @return 校准类型字符串
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月29日，新建函数
///
QString CaliBrateCommom::GetCalibrateType(shared_ptr<ch::ICalibrateCurve> cureveHandel)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	QString caliType = "";
	if (cureveHandel == nullptr)
	{
		return caliType;
	}

	const auto& type = cureveHandel->GetAlgorithmConfigs();
	switch (type.calibration_type)
	{
		case ch::CALIBRATION_TYPE::CALIBRATION_TYPE_ONE_PONIT_LINEAR:
			caliType = QObject::tr("一点线性法");
			break;
		case ch::CALIBRATION_TYPE::CALIBRATION_TYPE_TWO_POINTS_LINEAR:
			caliType = QObject::tr("两点线性法");
			break;
		case ch::CALIBRATION_TYPE::CALIBRATION_TYPE_MULTI_POINTS_LINEAR:
			caliType = QObject::tr("多点线性法");
			break;
		case ch::CALIBRATION_TYPE::CALIBRATION_TYPE_LOGIT_LOG3P:
			caliType = QObject::tr("logitlog3点法");
			break;
		case ch::CALIBRATION_TYPE::CALIBRATION_TYPE_LOGIT_LOG4P:
			caliType = QObject::tr("logitlog4点法");
			break;
		case ch::CALIBRATION_TYPE::CALIBRATION_TYPE_LOGIT_LOG5P:
			caliType = QObject::tr("logitlog5点法");
			break;
		case ch::CALIBRATION_TYPE::CALIBRATION_TYPE_EXPONENT:
			caliType = QObject::tr("指数法");
			break;
		case ch::CALIBRATION_TYPE::CALIBRATION_TYPE_SPLINE:
			caliType = QObject::tr("样条曲线法");
			break;
		case ch::CALIBRATION_TYPE::CALIBRATION_TYPE_AKIMA:
			caliType = QObject::tr("Akima");
			break;
        case ch::CALIBRATION_TYPE::CALIBRATION_TYPE_PCHIP:
            caliType = QObject::tr("Pchip");
            break; 
		default:
			break;
	}

	return caliType;
}

QString CaliBrateCommom::GetCaliBrateMode(int caliMode)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    QString strCaliMode = "";
    switch (caliMode)
    {
        case tf::CaliMode::CALI_MODE_BLANK:
            strCaliMode = QObject::tr("空白");
            break;
        case tf::CaliMode::CALI_MODE_TWO_POINT:
            strCaliMode = QObject::tr("两点");
            break;
        case tf::CaliMode::CALI_MODE_SPAN_POINT:
            strCaliMode = QObject::tr("量程");
            break;
        case tf::CaliMode::CALI_MODE_FULLDOT:
            strCaliMode = QObject::tr("全点");
            break;
        case tf::CaliMode::CALI_MODE_INTELLIGENT:
            strCaliMode = QObject::tr("智能");
            break;
        default:
            break;
    }

    return strCaliMode;
}

///
/// @brief
///     获取校准类型来源
///
/// @param[in]  reagentInfo  试剂信息
///
/// @return 试剂校准类型来源字符串
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月17日，新建函数
///
QString CaliBrateCommom::GetCaliBrateType(const ch::tf::ReagentGroup& reagentInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    QString showMsg;
    // 项目继承
    if (reagentInfo.caliCurveSource == tf::CurveSource::CURVE_SOURCE_ASSAY)
    {
        showMsg = "project";
    }
    // 批继承
    else if (reagentInfo.caliStatus == tf::CurveSource::CURVE_SOURCE_LOT)
    {
        showMsg = "lot";
    }
    // 瓶曲线
    else if (reagentInfo.caliStatus == tf::CurveSource::CURVE_SOURCE_BOTTLE)
    {
        showMsg = "bottle";
    }
    // 未校准
    else if (reagentInfo.caliStatus == tf::CurveSource::CURVE_SOURCE_NONE)
    {
        showMsg = "NoCali";
    }

    return showMsg;
}

///
/// @brief
///     获取试剂
///
/// @param[in]  reagentInfo  试剂信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月9日，新建函数
///
QString CaliBrateCommom::GetCaliBrateStatus(const ch::tf::ReagentGroup& reagentInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    QString showMsg;
    // 备用
    if (reagentInfo.usageStatus == tf::UsageStatus::USAGE_STATUS_BACKUP)
    {
        showMsg = "backup";
    }
    // 当前
    else if (reagentInfo.usageStatus == tf::UsageStatus::USAGE_STATUS_CURRENT)
    {
        showMsg = "current";
    }

    return showMsg;
}

///
/// @brief
///     获取试剂是开放或者封闭
///
/// @param[in]  reagentInfo  试剂信息
///
/// @return 试剂类型的字符串
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月9日，新建函数
///
QString CaliBrateCommom::GetCaliBrateReagentType(const ch::tf::ReagentGroup& reagentInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    QString showMsg;
    // 开放试剂
    if (reagentInfo.openReagent)
    {
        showMsg = "open";
    }
    // 封闭试剂
    else
    {
        showMsg = "close";
    }

    return showMsg;
}

///
/// @brief
///     获取校准品的位置和杯类型
///
/// @param[in]  doc  校准品信息
///
/// @return 位置和类型字符串
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月24日，新建函数
///
QString CaliBrateCommom::GetPosAndCupType(ch::tf::CaliDoc& doc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    QString PostionAndtype = "";

    if (doc.rack.empty())
    {
        PostionAndtype = QObject::tr("位置");
    }
    else
    {
        //rack + postion
        PostionAndtype = QString::fromStdString(doc.rack) + "-" + QString::number(doc.pos);
    }

    //连接字符&
    PostionAndtype += "&";
    switch (doc.tubeType)
    {
    case ::tf::TubeType::type::TUBE_TYPE_NORMAL:
        PostionAndtype += QObject::tr("标准");
        break;
    case ::tf::TubeType::type::TUBE_TYPE_MICRO:
        PostionAndtype += QObject::tr("微量");
        break;
    case ::tf::TubeType::type::TUBE_TYPE_INVALID:
        PostionAndtype += QObject::tr("无效");
        break;
    case ::tf::TubeType::type::TUBE_TYPE_ORIGIN:
        PostionAndtype += QObject::tr("样本原始管");
        break;
    default:
        PostionAndtype += QObject::tr("类型");
        break;
    }

    return PostionAndtype;
}

///
/// @brief 根据条件获取曲线组
///
/// @param[in]  CurveCond  条件组
///
/// @return 曲线组
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月18日，新建函数
///
std::vector<std::shared_ptr<ch::tf::CaliCurve>> CaliBrateCommom::GetCurvesByCond(const std::vector<::ch::tf::CaliCurveQueryCond>& CurveCond)
{
	std::vector<std::shared_ptr<ch::tf::CaliCurve>> repVec;
	if (CurveCond.empty())
	{
		return repVec;
	}

	for (const auto& cond : CurveCond)
	{
		// 查询曲线数据
		::ch::tf::CaliCurveQueryResp CurveRsp;
		ch::c1005::LogicControlProxy::QueryCaliCurve(CurveRsp, cond);
		if (CurveRsp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
		{
			return repVec;
		}

		// 添加项目曲线
		for (const auto& caliCurve : CurveRsp.lstCaliCurves)
		{
			auto findCurve = std::find_if(repVec.begin(), repVec.end(), [&](const auto& curve)->bool { return (curve->id == caliCurve.id); });
			if (findCurve == repVec.end() && !caliCurve.isEditCurve)
			{
				repVec.push_back(make_shared<ch::tf::CaliCurve>(caliCurve));
			}
		}
	}

	return repVec;
}

std::map<int64_t, std::vector<std::string>> CaliBrateCommom::GetCurveCupInfos(const ch::tf::CaliCurve & curve)
{
    std::map<int64_t, std::vector<std::string>> cupInfos;

    for (const auto& result : curve.assayTestResults)
    {
        ch::tf::AssayTestResultQueryResp qryResp;
        ch::tf::AssayTestResultQueryCond qryCond;
        qryCond.__set_id(result.second);

        if (!ch::c1005::LogicControlProxy::QueryAssayTestResult(qryResp, qryCond) || qryResp.lstAssayTestResult.empty())
        {
            ULOG(LOG_ERROR, "QueryAssayTestResult failed or lstAssayTestResult is empty");
            continue;
        }

        std::string strPreAssay;
        if (qryResp.lstAssayTestResult[0].previousAssayCode > 0)
        {
            auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(qryResp.lstAssayTestResult[0].previousAssayCode);
            if (spAssay != nullptr)
                strPreAssay = spAssay->assayName;
            else
                strPreAssay = std::move(to_string(qryResp.lstAssayTestResult[0].previousAssayCode));
        }

        if (cupInfos[result.first].empty())
        {
            cupInfos[result.first].resize(4);
            cupInfos[result.first][0] = std::move(to_string(qryResp.lstAssayTestResult[0].cupSN));
            cupInfos[result.first][1] = std::move(strPreAssay);
        }
        else
        {
            cupInfos[result.first][2] = std::move(to_string(qryResp.lstAssayTestResult[0].cupSN));
            cupInfos[result.first][3] = std::move(strPreAssay);
        }
    }

    return cupInfos;
}
