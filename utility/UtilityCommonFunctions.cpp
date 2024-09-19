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
/// @file     UtilityCommonFunctions.cpp
/// @brief    应用的通用工具函数
///
/// @author   8276/huchunli
/// @date     2023年7月25日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8276/huchunli，2023年7月25日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "UtilityCommonFunctions.h"
#include <QStandardItemModel>
#include <QTableView>
#include "shared/QComDelegate.h"
#include "thrift/DcsControlProxy.h"
#include "thrift/im/i6000/I6000LogicControlProxy.h"
#include "../../src/alg/im/IMM_ALG/ConvertHelper.h"
#include "src/common/Mlog/mlog.h"

///
/// @bref
///		获取免疫的稀释倍数列表.(如果不能从配置中获取，则采用默认列表
///
/// @par History:
/// @li 8276/huchunli, 2023年8月31日，新建函数
///
QStringList utilcomm::GetDuliRatioListIm()
{
    // 没有读取到配置，则采用默认倍率
    std::vector<int> lstDuli;
    if (!DcsControlProxy::GetInstance()->GetDilutionRatioListIm(lstDuli) || lstDuli.size() == 0)
    {
        QStringList qDuliList{ "1","2","4","5","10","20","40","50","80","100","200","400","500","800","1000","1600","2000","3000","3200","4000","6400" };
        return qDuliList;
    }

    QStringList valueList;
    for (int i = 0; i < lstDuli.size(); ++i)
    {
        valueList.append(QString::number(lstDuli[i]));
    }
    return valueList;
}

// 和算法库的json格式不一样，所以只能自己解析
bool MyJson2SegmentSpanElement(const std::string& json, im::SegmentSpanElement& outSp)
{
    try
    {
        boost::property_tree::ptree pt;
        stringstream ss(json);
        boost::property_tree::json_parser::read_json(ss, pt);

        if (pt.not_found() != pt.find("spanType"))
        {
            outSp.spanFixType = (im::SpanFixType)pt.get<unsigned>("spanType");
        }

        if (pt.not_found() != pt.find("start"))
        {
            outSp.startEnd.start = pt.get<unsigned>("start");
        }

        if (pt.not_found() != pt.find("end"))
        {
            outSp.startEnd.end = pt.get<unsigned>("end");
        }

        if (pt.not_found() != pt.find("caliStart"))
        {
            outSp.pointIndex.push_back(pt.get<unsigned>("caliStart"));
        }
        if (pt.not_found() != pt.find("caliEnd"))
        {
            outSp.pointIndex.push_back(pt.get<unsigned>("caliEnd"));
        }
    }
    catch (exception& e)
    {
        return false;
    }
    return true;
}

bool MyJson2SpanFix2Segment(const std::string& json, im::SpanFix2Segment& outSeg)
{
    outSeg.clear();
    try
    {
        boost::property_tree::ptree pt;
        stringstream ss(json);
        boost::property_tree::json_parser::read_json(ss, pt);

        for (const auto& x : pt)
        {
            stringstream ss;
            boost::property_tree::write_json(ss, x.second.get_child(""), false);
            im::SegmentSpanElement outSp;
            if (MyJson2SegmentSpanElement(ss.str(), outSp))
            {
                outSeg.push_back(outSp);
            }
        }
    }
    catch (exception& e)
    {
        return false;
    }

    return true;
}

bool utilcomm::ParseCaliJson(const std::string& json, CaliCurvParam& outCaliParam)
{
    try
    {
        boost::property_tree::ptree pt;
        stringstream ss(json);
        boost::property_tree::json_parser::read_json(ss, pt);

        if (pt.not_found() != pt.find("immType"))
        {
            outCaliParam.isQuantita = ((im::IMM_TYPE)pt.get<unsigned>("immType") == im::IMM_TYPE::QUANTITATE);
        }
        if (pt.not_found() != pt.find("calibratePoints"))
        {
            stringstream ss;
            boost::property_tree::write_json(ss, pt.get_child("calibratePoints"), false);
            outCaliParam.calibratePoints = std::make_shared<im::CalibratePoints>(Json2CalibratePoints(ss.str()));
        }
        if (pt.not_found() != pt.find("fit2Segment"))
        {
            stringstream ss;
            boost::property_tree::write_json(ss, pt.get_child("fit2Segment"), false);
            outCaliParam.fit2Segment = std::make_shared<im::Fit2Segment>(Json2Fit2Segment(ss.str()));
        }
        if (pt.not_found() != pt.find("cutoffParams"))
        {
            stringstream ss;
            boost::property_tree::write_json(ss, pt.get_child("cutoffParams"), false);
            outCaliParam.cutoff = std::make_shared<im::CutOffParams>(Json2CutOffParams(ss.str()));
        }
        if (pt.not_found() != pt.find("spanFix2Segment"))
        {
            stringstream ss;
            boost::property_tree::write_json(ss, pt.get_child("spanFix2Segment"), false);
            im::SpanFix2Segment ousf;
            if (MyJson2SpanFix2Segment(ss.str(), ousf))
            {
                outCaliParam.spanFix2Segment = std::make_shared<im::SpanFix2Segment>(ousf);
            }
        }
    }
    catch (exception& e)
    {
        ULOG(LOG_WARN, "Failed to parse cali json, %s.", e.what());
        return false;
    }

    return true;
}

bool utilcomm::ModifyCaliJson(std::string& json, const CaliCurvParam& caliParam)
{
    try
    {
        boost::property_tree::ptree pt;
        stringstream ss(json);
        boost::property_tree::json_parser::read_json(ss, pt);

        auto typeIter = pt.find("immType");
        if (pt.not_found() != typeIter)
        {
            typeIter->second.put_value(caliParam.isQuantita);
        }

        // 组装calibratePoints节点
        if (caliParam.calibratePoints != nullptr)
        {
            auto calibratePointsIter = pt.find("calibratePoints");
            if (pt.not_found() != calibratePointsIter)
            {
                pt.erase(calibratePointsIter->first);

                boost::property_tree::ptree ptCaliPointer;
                for (const auto& x : *(caliParam.calibratePoints))
                {

                    boost::property_tree::ptree ptSub;
                    ptSub.put<double>("c", x.first);
                    ptSub.put<double>("abs", x.second);

                    ptCaliPointer.push_back(make_pair("", ptSub));

                }
                pt.add_child("calibratePoints", ptCaliPointer);
            }
        }

        // 组装fit2Segment节点
        if (caliParam.fit2Segment != nullptr)
        {
            auto fit2SegmentIter = pt.find("fit2Segment");
            if (pt.not_found() != fit2SegmentIter)
            {
                pt.erase(fit2SegmentIter->first);

                boost::property_tree::ptree ptFit;
                boost::property_tree::ptree ptSub;

                for (auto x : *(caliParam.fit2Segment))
                {
                    ptSub.put<unsigned>("fitType", x.fitType);
                    ptSub.put<unsigned>("start", x.startEnd.start);
                    ptSub.put<unsigned>("end", x.startEnd.end);
                    ptFit.push_back(make_pair("", ptSub));

                    ptSub.clear();
                }

                pt.add_child("fit2Segment", ptFit);
            }
        }

        // 组装cutoffParams节点
        if (caliParam.cutoff != nullptr)
        {
            auto cutoffParamsIter = pt.find("cutoffParams");
            if (pt.not_found() != cutoffParamsIter)
            {
                pt.erase(cutoffParamsIter->first);

                boost::property_tree::ptree ptCut;

                const auto& cp = *(caliParam.cutoff);
                ptCut.put<double>("A1", cp.A1);
                ptCut.put<double>("A2", cp.A2);
                ptCut.put<double>("B1", cp.B1);
                ptCut.put<double>("B2", cp.B2);
                ptCut.put<double>("C", cp.C);
                ptCut.put<double>("upperSidePositive", cp.upperSidePositive);
                ptCut.put<double>("withUpperEqual", cp.withUpperEqual);
                ptCut.put<double>("withLowerEqual", cp.withLowerEqual);
                ptCut.put<double>("lower", cp.lower);
                ptCut.put<double>("upper", cp.upper);

                pt.add_child("cutoffParams", ptCut);
            }
        }

        // 组装spanFix2Segment节点
        if (caliParam.spanFix2Segment != nullptr)
        {
            auto spfIter = pt.find("spanFix2Segment");
            if (pt.not_found() != spfIter)
            {
                pt.erase(spfIter->first);

                boost::property_tree::ptree ptSeg;
                for (const auto& x : *(caliParam.spanFix2Segment))
                {
                    boost::property_tree::ptree ptSItem;
                    ptSItem.put<unsigned>("spanType", x.spanFixType);
                    ptSItem.put<unsigned>("start", x.startEnd.start);
                    ptSItem.put<unsigned>("end", x.startEnd.end);
                    if (x.pointIndex.size() > 0)
                    {
                        ptSItem.put<unsigned>("caliStart", x.pointIndex[0]);
                    }
                    if (x.pointIndex.size() > 1)
                    {
                        ptSItem.put<unsigned>("caliEnd", x.pointIndex[1]);
                    }
                    ptSeg.push_back(make_pair("", ptSItem));
                }

                pt.add_child("spanFix2Segment", ptSeg);
            }
        }

        stringstream wtss;
        boost::property_tree::write_json(wtss, pt, false);
        json = wtss.str(); // 修改原来的json
    }
    catch (exception& e)
    {
        ULOG(LOG_WARN, "Failed to parse cali json, %s.", e.what());
        return false;
    }

    return true;
}

int utilcomm::GetSuckVol(const std::vector<::im::tf::SuckReagent>& suckList)
{
    if (suckList.size() == 0)
    {
        return -1;
    }

    int suckVol = 0;
    for (const ::im::tf::SuckReagent& sItem : suckList)
    {
        suckVol += sItem.suckVol;
    }
    return suckVol;
}

void utilcomm::AssignReagentSuckTb(QWidget* parent, QTableView* pView, QStandardItemModel* tbModel,
    int startIdx, const std::vector<::im::tf::reactReagInfo>& suckList)
{
#define ROW_AMOUNT 3 // 总共有3个试剂腔
    tbModel->removeRows(0, tbModel->rowCount());

    // 试剂子位置赋值
    int assignCnt = 0;
    for (const ::im::tf::reactReagInfo& rItem : suckList)
    {
        if (rItem.reactStep != startIdx)
        {
            continue;
        }
        for (const ::im::tf::SuckReagent& sItem : rItem.listSuckReagents)
        {
            tbModel->appendRow({
                new QStandardItem(""),
                new QStandardItem(QString::number(sItem.suckVol)) });

            QModelIndex posIdx = tbModel->index(assignCnt, 0);
            CenterComboBox *posComb = new CenterComboBox(nullptr);
            posComb->addItems({ "", "1", "2", "3" });
            pView->setIndexWidget(posIdx, posComb);
            posComb->setCurrentText(QString::number(sItem.suckPos));
            posComb->installEventFilter(parent);

            assignCnt++;
        }
    }

    // 末尾赋予空行
    for (int i = assignCnt; i < ROW_AMOUNT; ++i)
    {
        tbModel->appendRow({ new QStandardItem(""), new QStandardItem("") });
    }
}

void utilcomm::GetReagentSuckFromTb(QTableView* pView, QStandardItemModel* tbModel, 
    int startIdx, std::vector<::im::tf::reactReagInfo>& suckList)
{
    // 获取一行的数据
    auto funcGetValue = [&](int rowIdx, int64_t& pos, int64_t& vol){
        auto posItem = qobject_cast<CenterComboBox*>(pView->indexWidget(tbModel->index(rowIdx, 0)));
        if (posItem != Q_NULLPTR && !posItem->currentText().isEmpty())
        {
            pos = posItem->currentText().toInt();
        }
        QStandardItem* suckVol = tbModel->item(rowIdx, 1);
        if (suckVol != Q_NULLPTR)
        {
            vol = suckVol->text().toInt();
        }
    };

    int rowIdx = 0;
    for (::im::tf::reactReagInfo& rItem : suckList)
    {
        if (rItem.reactStep != startIdx)
        {
            continue;
        }
        for (::im::tf::SuckReagent& sItem : rItem.listSuckReagents)
        {
            funcGetValue(rowIdx, sItem.suckPos, sItem.suckVol);
            rowIdx++;
        }
    }
}

bool utilcomm::CaliCurvCache::Init(int iAssayCode)
{
    m_curvs.clear();
    m_curvsMap.clear();

    // 查询曲线
    ::im::tf::CaliCurveQueryResp respCurv;
    ::im::tf::CaliCurveQueryCond condCurv;
    condCurv.__set_assayCode(iAssayCode);
    condCurv.__set_masterCurveID(0);
    if (!::im::i6000::LogicControlProxy::QueryCaliCurve(respCurv, condCurv) ||
        respCurv.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS || respCurv.lstCurveConfigs.size() == 0)
    {
        ULOG(LOG_WARN, "Failed to query calicurv.");
        return false;
    }

    for (const ::im::tf::CaliCurve& cv : respCurv.lstCurveConfigs)
    {
        std::shared_ptr<::im::tf::CaliCurve> pCurv = std::make_shared<::im::tf::CaliCurve>(cv);
        m_curvs.push_back(pCurv);

        std::string strKey = (cv.testMode == 1) ? (cv.calibratorLot + "-STAT") : cv.calibratorLot; // 区分STAT
        m_curvsMap.insert(std::pair<std::string, std::shared_ptr<::im::tf::CaliCurve>>(strKey, pCurv));
    }

    return true;
}

void utilcomm::CaliCurvCache::GetLotList(std::vector<std::string>& vecLots)
{
    vecLots.clear();

    std::map<std::string, std::shared_ptr<::im::tf::CaliCurve>>::iterator it = m_curvsMap.begin();
    for (; it != m_curvsMap.end(); ++it)
    {
        vecLots.push_back(it->first);
    }
}

bool utilcomm::CaliCurvCache::GetLateastCurve(std::shared_ptr<im::tf::CaliCurve>& outCurv, std::string& strLot)
{
    strLot.clear();
    if (outCurv != nullptr)
    {
        outCurv.reset();
        outCurv = nullptr;
    }

    // 以曲线生成时间，取最新的一条
    std::map<std::string, std::string> mapCurv; // <time, lotkey>
    std::map<std::string, std::shared_ptr<::im::tf::CaliCurve>>::iterator it = m_curvsMap.begin();
    for (; it != m_curvsMap.end(); ++it)
    {
        mapCurv.insert(std::pair<std::string, std::string>(it->second->caliExcuteDate, it->first));
    }

    if (mapCurv.size() != 0)
    {
        std::map<std::string, std::shared_ptr<::im::tf::CaliCurve>>::iterator fIter = m_curvsMap.find(mapCurv.rbegin()->second);
        if (fIter != m_curvsMap.end())
        {
            outCurv = fIter->second;
            strLot = fIter->first;

            return true;
        }
    }

    return false;
}

std::shared_ptr<im::tf::CaliCurve> utilcomm::CaliCurvCache::GetCurveByLot(const std::string& strLot)
{
    std::map<std::string, std::shared_ptr<::im::tf::CaliCurve>>::iterator it = m_curvsMap.find(strLot);
    return it == m_curvsMap.end() ? nullptr : it->second;
}
