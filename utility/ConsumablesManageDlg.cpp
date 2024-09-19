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
/// @file     ConsumablesManageDlg.cpp
/// @brief    耗材管理配置弹窗
///
/// @author   7915/LeiDingXiang
/// @date     2023年2月2日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年2月2日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "ConsumablesManageDlg.h"
#include "ui_ConsumablesManageDlg.h"

#include <boost/optional.hpp>
#include "src/thrift/im/i6000/gen-cpp/i6000_types.h"
#include "thrift/im/ImLogicControlProxy.h"
#include "thrift/DcsControlProxy.h"

#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "shared/CommonInformationManager.h"
#include "shared/QComDelegate.h"
#include "manager/DictionaryQueryManager.h"
#include "manager/UserInfoManager.h"

#include "src/common/Mlog/mlog.h"
#include "src/common/common.h"
#include "src/common/TimeUtil.h"
#include "src/public/SerializeUtil.hpp"
#include "src/public/DictionaryKeyName.h"
#include "src/public/ConfigSerialize.h"
#include "src/public/im/ImConfigDefine.h"
#include "src/public/ch/ChConfigSerialize.h"
#include "src/public/im/ImConfigSerialize.h"
#include "src/public/ise/IseConfigSerialize.h"

ConsumablesManageDlg::ConsumablesManageDlg(QWidget *parent)
    : BaseDlg(parent)
    , m_model(Q_NULLPTR)
    , m_ImModel(Q_NULLPTR)
    , ui(new Ui::ConsumablesManageDlg)
{
    ui->setupUi(this);
    InitBeforeShow();
}

ConsumablesManageDlg::~ConsumablesManageDlg()
{
}

///
/// @brief  初始化窗体信息
///
/// @param[in]    void
///
/// @return void
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年2月2日，新建函数
///
void ConsumablesManageDlg::InitBeforeShow()
{
    //设置标题
    SetTitleName(tr("耗材管理"));

    // 设置标签鼠标事件穿透
    ui->label->setAttribute(Qt::WA_TransparentForMouseEvents);

    // 免疫保存按钮
    connect(ui->save_btn, &QPushButton::clicked, this, [&]()
    {
        if (m_ImModel->SaveData())
        {
            close();
        }
    });
    // 生化保存按钮
    connect(ui->save_btn_2, &QPushButton::clicked, this, [&]() 
    {
        if (m_model->SaveData())
        {
            close();
        }
    });

    // 免疫关闭按钮
    connect(ui->cancel_btn, &QPushButton::clicked, this, [&]() {        
        //更新显示数据
        m_ImModel->UpdataShowData();
        close(); 
    });

    // 生化关闭按钮
    connect(ui->cancel_btn_2, &QPushButton::clicked, this, [&]() {
        m_model->UpdateDate();
        close(); 
    });
    
    ui->tableView->verticalHeader()->hide();
    ui->tableView->setItemDelegateForColumn((int)QSupplyManageModel::COLSAMPLE::SAMPLE_ACIDITY, new CheckBoxDelegate(this));
    ui->tableView->setItemDelegateForColumn((int)QSupplyManageModel::COLSAMPLE::SAMPLE_ALKALINITY, new CheckBoxDelegate(this));
    ui->tableView->setItemDelegateForColumn((int)QSupplyManageModel::COLSAMPLE::CUP_ACIDITY, new CheckBoxDelegate(this));
    ui->tableView->setItemDelegateForColumn((int)QSupplyManageModel::COLSAMPLE::CUP_ALKALINITY, new CheckBoxDelegate(this));
	//设置水平、垂直控件滚动条取消
	ui->im_tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui->im_tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->im_tableView->verticalHeader()->hide();
    ui->im_tableView->setItemDelegateForColumn(2, new CheckBoxDelegate(this));
    ui->im_tableView->setItemDelegateForColumn(3, new CheckBoxDelegate(this));
    ui->im_tableView->setItemDelegateForColumn(4, new CheckBoxDelegate(this));
    ui->im_tableView->setItemDelegateForColumn(5, new CheckBoxDelegate(this));

    if (m_model == Q_NULLPTR)
    {
        m_model = new QSupplyManageModel();
        ui->tableView->setModel(m_model);

        ui->tableView->setColumnWidth((int)QSupplyManageModel::COLSAMPLE::SEQ, 60);
        ui->tableView->setColumnWidth((int)QSupplyManageModel::COLSAMPLE::NAME, 140);
        ui->tableView->horizontalHeader()->setSectionResizeMode((int)QSupplyManageModel::COLSAMPLE::SAMPLE_ACIDITY, QHeaderView::Stretch);
        ui->tableView->horizontalHeader()->setSectionResizeMode((int)QSupplyManageModel::COLSAMPLE::SAMPLE_ALKALINITY, QHeaderView::Stretch);
    }

    if (m_ImModel == Q_NULLPTR)
    {
        m_ImModel = new QImSupplyManageModel();
        ui->im_tableView->setModel(m_ImModel);
        ui->im_tableView->setColumnWidth(0, 60);
        ui->im_tableView->setColumnWidth(1, 140);
		ui->im_tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
		ui->im_tableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
		ui->im_tableView->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
		ui->im_tableView->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);

        //悬浮显示代理（不可用，待更改）
        //ui->im_tableView->setItemDelegateForColumn(0, new ConsumablesItemTableDelegate());
        //ui->im_tableView->setItemDelegateForColumn(1, new ConsumablesItemTableDelegate());
        //ui->im_tableView->setItemDelegateForColumn(2, new ConsumablesItemTableDelegate());
        //ui->im_tableView->setItemDelegateForColumn(3, new ConsumablesItemTableDelegate());
        //ui->im_tableView->setItemDelegateForColumn(4, new ConsumablesItemTableDelegate());
        //ui->im_tableView->setItemDelegateForColumn(5, new ConsumablesItemTableDelegate());

        QHeaderView *horizontalHeader = ui->im_tableView->horizontalHeader();
        for (int column = 0; column < horizontalHeader->count(); ++column)
        {
            //QString title = horizontalHeader->model()->headerData(i, Qt::Horizontal).toString();
            //悬浮显示 待实现
        }

        ui->im_tableView->horizontalHeader()->setTextElideMode(Qt::ElideRight);
        ui->im_tableView->setMouseTracking(true);
    }

    // 设置生化免疫切换按钮是否可见
    SOFTWARE_TYPE type = CommonInformationManager::GetInstance()->GetSoftWareType();
    if (type == CHEMISTRY )
    {
        ui->tabWidget->removeTab(1);
    } 
    else if(type == IMMUNE)
    {
        ui->tabWidget->removeTab(0);
    }

    // 重新设置界面布局
    if (ui->tabWidget->count() < 2)
    {
        int hight = ui->tabWidget->tabBar()->height();
        ui->label->move(ui->label->x(), ui->label->y() - hight + 2);
        ui->tabWidget->setFixedHeight(ui->tabWidget->height() - hight);
        this->setFixedHeight(this->height() - hight);
    }

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

QSupplyManageModel::QSupplyManageModel()
{
    UpdateDate();

    m_heads.clear();
    m_heads << tr("序号") << tr("模块名称") 
        << CH_CUP_DETERGENT_ACIDITY_NAME + tr("(台面)")
        << CH_CUP_DETERGENT_ALKALINITY_NAME + tr("(台面)") 
        << CH_CUP_DETERGENT_ACIDITY_NAME + tr("(柜门)")
        << CH_CUP_DETERGENT_ALKALINITY_NAME + tr("(柜门)");
}

bool QSupplyManageModel::setData(const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/)
{
    if (role != (Qt::UserRole + 1))
    {
        return false;
    }

    // 获取编辑的行和列
    const int col = index.column();
    const int row = index.row();

    if (row < 0 || row >= m_data.size())
    {
        return false;
    }

    auto& data = m_data[row];
    switch (col)
    {
        case (int)COLSAMPLE::SAMPLE_ACIDITY:
            data.acidityUp = !data.acidityUp;
            break;
        case (int)COLSAMPLE::SAMPLE_ALKALINITY:
            data.alkalinityUp = !data.alkalinityUp;
            break;
        case (int)COLSAMPLE::CUP_ACIDITY:
            data.acidityLower = !data.acidityLower;
            break;
        default:
            break;
    }
  
    return true;
}

bool QSupplyManageModel::SaveData()
{
    const auto& CIM = CommonInformationManager::GetInstance();

    // 设备运行合维护中禁止修改参数
    if (CIM->IsExistDeviceRuning() || CIM->HasDeviceEqualStatus(::tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN))
    {
        TipDlg(tr("设备运行或维护中禁止修改参数")).exec();
        return false;
    }

    for (auto& data : m_data)
    {
        ::tf::DeviceInfo di;
        di.__set_deviceSN(data.sn);

        auto dev = CIM->GetDeviceInfo(data.sn);
        if (dev == nullptr)
        {
            TipDlg(tr("保存失败")).exec();
            return false;
        }

        if (dev->deviceType == tf::DeviceType::DEVICE_TYPE_ISE1005)
        {
            IseDeviceOtherInfo idoi;
            if (!DecodeJson(idoi, dev->otherInfo))
            {
                TipDlg(tr("保存失败")).exec();
                return false;
            }

            idoi.acidity = data.acidityUp;
            idoi.alkalinity = data.alkalinityUp;
           
            std::string info;
            if (!Encode2Json(info, idoi))
            {
                TipDlg(tr("保存失败")).exec();
                return false;
            }

            di.__set_otherInfo(info);
        }
        else
        {
            ChDeviceOtherInfo cdoi;
            if (!DecodeJson(cdoi, dev->otherInfo))
            {
                TipDlg(tr("保存失败")).exec();
                return false;
            }

            cdoi.acidity = data.acidityLower;
            cdoi.alkalinity = data.alkalinityLower;

            std::string info;
            if (!Encode2Json(info, cdoi))
            {
                TipDlg(tr("保存失败")).exec();
                return false;
            }

            di.__set_otherInfo(info);
        }

        if (!CIM->ModifyDeviceInfo(di))
        {
            TipDlg(tr("保存失败")).exec();
            return false;
        }
    }

    return true;
}

void QSupplyManageModel::UpdateDate()
{
    m_data.clear();

    // 联机才显示ISE
    std::vector<::tf::DeviceType::type> deviceTypes;
    if (DictionaryQueryManager::GetInstance()->GetPipeLine())
    {
        deviceTypes.emplace_back(tf::DeviceType::DEVICE_TYPE_ISE1005);
    }

    deviceTypes.emplace_back(tf::DeviceType::DEVICE_TYPE_C1000);
    for (const auto& dev : CommonInformationManager::GetInstance()->GetDeviceFromType(deviceTypes))
    {
        ViewData data;
        data.name = dev->groupName + dev->name;
        data.sn = dev->deviceSN;
        data.deviceType = dev->deviceType;
        if (dev->deviceType == tf::DeviceType::DEVICE_TYPE_ISE1005)
        {
            IseDeviceOtherInfo di;
            if (DecodeJson(di, dev->otherInfo))
            {
                data.acidityUp = di.acidity;
                data.alkalinityUp = di.alkalinity;
            }
        }
        else
        {
            ChDeviceOtherInfo di;
            if (DecodeJson(di, dev->otherInfo))
            {
                data.acidityLower = di.acidity;
                data.alkalinityLower = di.alkalinity;
            }
        }

        m_data.emplace_back(std::move(data));
    }
}

Qt::ItemFlags QSupplyManageModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

int QSupplyManageModel::rowCount(const QModelIndex &parent) const
{
    return m_data.size();
}

int QSupplyManageModel::columnCount(const QModelIndex &parent) const
{
    return m_heads.size();
}

QVariant QSupplyManageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    int row = index.row();
    int column = index.column();

    if (row < 0 || row >= m_data.size()
        || column < 0 || column >= m_heads.size())
    {
        return QVariant();
    }

    auto data = m_data[row];
    if (role == Qt::DisplayRole)
    {
        switch (column)
        {
            case static_cast<int>(COLSAMPLE::SEQ):
                return row + 1;
                break;
            case static_cast<int>(COLSAMPLE::NAME):
                return QString::fromStdString(data.name);
                break;
            default:
                return QVariant();
                break;
        }
    }
    else if (role == VIEW_ITEM_DISABLE_ROLE)
    {
        switch (column)
        {
            case static_cast<int>(COLSAMPLE::SAMPLE_ACIDITY):
                return data.deviceType == tf::DeviceType::DEVICE_TYPE_C1000 ? QString("#f6f6f6") : QVariant();
                break;
            case static_cast<int>(COLSAMPLE::SAMPLE_ALKALINITY):
                return data.deviceType == tf::DeviceType::DEVICE_TYPE_C1000 ? QString("#f6f6f6") : QVariant();
                break;
            case static_cast<int>(COLSAMPLE::CUP_ACIDITY):
                return data.deviceType == tf::DeviceType::DEVICE_TYPE_ISE1005 ? QString("#f6f6f6") : QVariant();
                break;
            case static_cast<int>(COLSAMPLE::CUP_ALKALINITY):
                return data.deviceType == tf::DeviceType::DEVICE_TYPE_ISE1005 ? QString("#f6f6f6") : QVariant();
                break;
            default:
                return QVariant();
                break;
        }
    }

    // 对齐方式
    else if (role == Qt::TextAlignmentRole) 
    {
        return Qt::AlignCenter;
    }
    // 特殊处理，选择项
    else if (role == (Qt::UserRole + 1))
    {
        switch (column)
        {
            case (int)COLSAMPLE::SAMPLE_ACIDITY:
                return data.acidityUp;
            case (int)COLSAMPLE::SAMPLE_ALKALINITY:
                return data.alkalinityUp;
            case (int)COLSAMPLE::CUP_ACIDITY:
                return data.acidityLower;
            case (int)COLSAMPLE::CUP_ALKALINITY:
                return data.alkalinityLower;
            default:
                return QVariant();
        }
    }

    return QVariant();    
}

QVariant QSupplyManageModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    if (section < 0 || section > m_heads.size())
    {
        return QVariant();
    }

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) 
    {
        return m_heads.at(section);
    }

    if ((section > 1 ) && (role == Qt::ForegroundRole) && (orientation == Qt::Horizontal))
    {
        return QColor(UI_HEADER_FONT_COLOR);
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

///
/// @brief  重写模型表格设置
///
/// @param[in]    QModelIndex：模型索引
///
/// @return int ： Qt::ItemFlags 单元格设置
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年7月11日，新建函数
///
Qt::ItemFlags QImSupplyManageModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QImSupplyManageModel::QImSupplyManageModel()
: m_bCurConfectIsEmbty{ true }
, m_bCurConfectIsMix{ true }
, m_SupLot{ "" }
, m_SupSerial{ "" }
{
    m_data.clear();
    std::vector<tf::DeviceType::type> queryType;
    // 普通模式
    queryType.push_back(tf::DeviceType::DEVICE_TYPE_I6000);
    // 若不为i6000单机
    if (!CommonInformationManager::GetInstance()->GetImisSingle())
    {
        queryType.push_back(tf::DeviceType::DEVICE_TYPE_I6000);
    }

    std::vector<std::shared_ptr<const tf::DeviceInfo>> cDevs = CommonInformationManager::GetInstance()->GetDeviceFromType(queryType);
    for (const auto& devItem : cDevs)
    {
        m_data.push_back(std::make_shared<tf::DeviceInfo>(*devItem));
    }

    for (int i = 0; i < m_data.size(); i++)
    {
        ImDeviceOtherInfo washSet;
        // 回写值到耗材信息中
        if (!DecodeJson(washSet, m_data[i]->otherInfo))
        {
            return;
        }
        //初始化保存设置
        m_VecConsumSet.push_back(washSet);
    }

    m_heads.clear();
    m_heads << tr("序号") << tr("模块名称") << tr("底物液组1") << tr("底物液组2") << tr("清洗缓冲液1") << tr("清洗缓冲液2");
}

///
/// @brief  获取当前设备状态
///
/// @param[in]    m_DeviceSn DeviceSn设备号
///
/// @return ::tf::DeviceWorkState::type 设备状态
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年2月6日，新建函数
///
int32_t QImSupplyManageModel::GetCurDeviceStat(std::string m_DeviceSn)
{
    // 查询当前所有设备状态
    tf::DeviceInfoQueryResp devInfoResp;
    tf::DeviceInfoQueryCond devInfoCond;

    // 执行查询
    if (!DcsControlProxy::GetInstance()->QueryDeviceInfo(devInfoResp, devInfoCond))
    {
        ULOG(LOG_WARN, "QueryDeviceInfo failed!");
    }
    // 结果为空则返回
    if (devInfoResp.lstDeviceInfos.empty())
    {
        ULOG(LOG_WARN, "devInfoResp.lstDeviceInfos.empty()!");
        //返回未知设备状态
        return ::tf::DeviceWorkState::DEVICE_STATUS_DISCONNECT;
    }

    for (auto m_DeviceInfoVec : devInfoResp.lstDeviceInfos)
    {
        //暂时只判断免疫设备的状态，其余设备直接可以测试
        if (m_DeviceSn != m_DeviceInfoVec.deviceSN)
        {
            continue;
        }
        return m_DeviceInfoVec.status;
    }

    return ::tf::DeviceWorkState::DEVICE_STATUS_DISCONNECT;
}

///
/// @brief  是否执行底物填充
///
/// @param[in]  deviceSN  设备序列号
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2024年7月8日，新建函数
///
bool QImSupplyManageModel::IsNeedToFillPipe(const std::string& deviceSN)
{
	::tf::LatestMaintainInfoQueryResp ret;
	::tf::LatestMaintainInfoQueryCond cond;
	cond.__set_deviceSN(deviceSN);
	if (!DcsControlProxy::GetInstance()->QueryLatestMaintainInfo(ret, cond))
	{
		return false;
	}
	if (ret.lstLatestMaintainInfo.empty())
	{
		ULOG(LOG_ERROR, "%s() : retGroup.lstLatestMaintainInfo.empty()", __FUNCTION__);
		return true;
	}

	bool foundStop = false;
	boost::posix_time::ptime stopSubTime = TimeStringToPosixTime("1970-01-01 00:00:00");
	bool foundFill = false;
	boost::posix_time::ptime fillPipeTime = TimeStringToPosixTime("1970-01-01 00:00:00");

	for (const auto & it : ret.lstLatestMaintainInfo)
	{
		for (const auto & mier : it.resultDetail)
		{
			auto tempTime = TimeStringToPosixTime(mier.exeTime);
			if (mier.itemType == ::tf::MaintainItemType::type::MAINTAIN_ITEM_CHANGE_USESTATUSE_FILL_SUBS && stopSubTime < tempTime)
			{
				foundStop = true;
				stopSubTime = tempTime;
			}
			if ((mier.itemType == ::tf::MaintainItemType::type::MAINTAIN_ITEM_FILL_PIP || mier.itemType == ::tf::MaintainItemType::type::MAINTAIN_ITEM_LOAD_FILL_SUBS)&&
				mier.result == ::tf::MaintainResult::type::MAINTAIN_RESULT_SUCCESS && fillPipeTime < tempTime)
			{
				foundFill = true;
				fillPipeTime = tempTime;
			}
		}
	}

	if (!foundStop && !foundFill)
	{
		return true;
	}

	return stopSubTime > fillPipeTime;
}

///
/// @brief  获取指定维护项目类型的单项维护组id
///
/// @param[in]  itemType  维护项类型
///
/// @return 单项维护组id -1表示获取失败
///
/// @par History:
/// @li 7656/zhang.changjiang，2024年2月22日，新建函数
///
int QImSupplyManageModel::GetSingleMaintGroupId(const ::tf::MaintainItemType::type itemType)
{
	::tf::MaintainGroupQueryResp retGroup;
	::tf::MaintainGroupQueryCond miqcGroup;
	miqcGroup.__set_groupType(::tf::MaintainGroupType::type::MAINTAIN_GROUP_SINGLE);
	if (!DcsControlProxy::GetInstance()->QueryMaintainGroup(retGroup, miqcGroup))
	{
		return -1;
	}
	if (retGroup.lstMaintainGroups.empty())
	{
		ULOG(LOG_ERROR, "%s() : retGroup.lstMaintainGroups.empty()", __FUNCTION__);
		return -1;
	}
	
	for (int i = 0; i < retGroup.lstMaintainGroups.size(); i++)
	{
		if (retGroup.lstMaintainGroups[i].items.empty())
		{
			continue;
		}

		if (retGroup.lstMaintainGroups[i].items.front().itemType == itemType)
		{
			return retGroup.lstMaintainGroups[i].id;
		}
	}

	return -1;
}

///
/// @brief  是否可以进行底物液启/停用
///
/// @param[in]    DevSn:当前设备序列号 PreBtnStu:即将更改的状态
///
/// @return 
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年3月11日，新建函数
///
bool QImSupplyManageModel::IsChangeSub(const std::string& DevSn, const bool& PreBtnStu)
{
    //判断设备状态
    if (::tf::DeviceWorkState::DEVICE_STATUS_STANDBY != GetCurDeviceStat(DevSn))
    {
        //若点击checkbox按钮后为启用给出提示
        if (PreBtnStu)
        {
            ULOG(LOG_INFO, "The device can't start test,cause its reagent cover is open!");
            std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("手工启用底物液组操作只能在仪器待机状态下进行。"), TipDlgType::TWO_BUTTON));
            pTipDlg->exec();
        }
        else
        {
            ULOG(LOG_INFO, "The device can't start test,cause its reagent cover is open!");
            std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("手工停用底物液组操作只能在仪器待机状态下进行。"), TipDlgType::TWO_BUTTON));
            pTipDlg->exec();
        }
        return false;
    }
    return true;
}

///
/// @brief  是否可以进行清洗缓冲液启/停用
///
/// @param[in]    DevSn:当前设备序列号 PreBtnStu:即将更改的状态 Btn:清洗缓冲液1/清洗缓冲液2
///
/// @return 
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年2月4日，新建函数
///
bool QImSupplyManageModel::IsChangeConfectLiquid(const std::string& DevSn, const bool& PreBtnStu, const int& Btn)
{

    // 清洗缓冲液位置为空处理
    if (!QueryCurConfectIsEmpty(Btn, m_bCurConfectIsEmbty))
    {
        return false;
    }

    //清洗缓冲液正在配液处理
    if (!QueryCurConfectIsMix(m_bCurConfectIsMix))
    {
        return false;
    }

    //仪器不为待机或清洗缓冲液正在使用给出提示
    if (::tf::DeviceWorkState::DEVICE_STATUS_STANDBY != GetCurDeviceStat(DevSn))
    {
        //若点击checkbox按钮后为启用给出提示
        if (PreBtnStu)
        {
            ULOG(LOG_INFO, "The device can't start test,cause its reagent cover is open!");
            std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("手工启用清洗缓冲液操作只能在仪器待机状态下进行。"), TipDlgType::TWO_BUTTON));
            pTipDlg->exec();
        }
        else
        {
            ULOG(LOG_INFO, "The device can't start test,cause its reagent cover is open!");
            std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("手工停用清洗缓冲液操作只能在仪器待机状态下进行。"), TipDlgType::TWO_BUTTON));
            pTipDlg->exec();
        }
        return false;
    }
    // 当前清洗缓冲液是配液状态禁止切换
    else if (m_bCurConfectIsMix)
    {
        //若点击checkbox按钮后为启用给出提示
        if (PreBtnStu)
        {
            ULOG(LOG_INFO, "The device can't start test,cause its reagent cover is open!");
            std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("当前清洗缓冲液正在配液，手工启用清洗缓冲液失败。"), TipDlgType::TWO_BUTTON));
            pTipDlg->exec();
        }
        else
        {
            ULOG(LOG_INFO, "The device can't start test,cause its reagent cover is open!");
            std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("当前清洗缓冲液正在配液，手工停用清洗缓冲液失败。"), TipDlgType::TWO_BUTTON));
            pTipDlg->exec();
        }
        return false;
    }
    return true;
}

///
/// @brief  查询当前清洗缓冲液是否为空
///
/// @param[in]    int CurGroupNum 当前组号
///               bool bConfectIsEmpty  是否为空
/// @return bool :true 查询成功 false 查询失败
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年6月14日，新建函数
///
bool QImSupplyManageModel::QueryCurConfectIsEmpty(const int& CurGroupNum, bool &bConfectIsEmpty)
{
    // 构造查询条件和查询结果
    ::im::tf::SuppliesInfoTableQueryResp qryResp;
    ::im::tf::SuppliesInfoTableQueryCond qryCond;
    qryCond.__set_supType(im::tf::SuppliesType::SUPPLIES_TYPE_CONCENTRATE_LIQUID);
    qryCond.__set_isLoad(1);
    qryCond.__set_groupNum(CurGroupNum);

    // 查询所有耗材信息
    bool bRet = ::im::LogicControlProxy::QuerySuppliesInfo(qryResp, qryCond);
    if (!bRet || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "QueryReagentInfo failed.");
        return false;
    }

    if (qryResp.lstSuppliesInfos.empty())
    {
        // 当前位置没有上机的清洗缓冲液
        bConfectIsEmpty = true;
    }
    else
    {
        // 存储批号和瓶号用于后续是否在配液判断
        m_SupLot = qryResp.lstSuppliesInfos[0].supLot;
        m_SupSerial = qryResp.lstSuppliesInfos[0].supSerial;
        bConfectIsEmpty = false;
    }
    return true;
}

///
/// @brief  查询当前清洗缓冲液是否在配液
///
/// @param[in]    
///               bool bConfectIsEmpty  是否为空
/// @return bool :true 查询成功 false 查询失败
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年6月14日，新建函数
///
bool QImSupplyManageModel::QueryCurConfectIsMix(bool &m_bCurConfectIsMix)
{
    // 判断操作的清洗缓冲液是否为在用状态,构造查询条件和查询结果查询配液桶
    ::im::tf::SuppliesInfoTableQueryResp qryResp;
    ::im::tf::SuppliesInfoTableQueryCond qryCond;
    // 配液桶只有一条记录
    qryCond.__set_supType(im::tf::SuppliesType::SUPPLIES_TYPE_CONFECT_LIQUID);
    qryCond.__set_isLoad(1);
    // 查询配液桶信息
    bool bRet = ::im::LogicControlProxy::QuerySuppliesInfo(qryResp, qryCond);
    if (!bRet || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "QueryReagentInfo failed.");
        return false;
    }
    // 若当前位置有上机清洗缓冲液且数据库中存在配液桶信息则判断是否在配液
    if (!m_bCurConfectIsEmbty && !qryResp.lstSuppliesInfos.empty())
    {
        // 当前的配液状态
        m_bCurConfectIsMix = qryResp.lstSuppliesInfos[0].usingStatus == im::tf::SuppliesUsingStatus::SUPPLIES_USING_STATUS_ON_LINE ? true : false;
    }
    else
    {
        //否则当前为不配液
        m_bCurConfectIsMix = false;
    }
    return true;
}

///
/// @brief  保存按钮被点击
///
/// @param[in]    void
///
/// @return void
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年2月2日，新建函数
///
bool QImSupplyManageModel::OnSaveBtnClicked(const std::shared_ptr<tf::DeviceInfo>& DevDate)
{
    //仅待机状态可以进行传输
    if (::tf::DeviceWorkState::DEVICE_STATUS_STANDBY != GetCurDeviceStat(DevDate->deviceSN))
    {
        //提示不能保存
        ULOG(LOG_INFO, "The device can't start test,cause its reagent cover is open!");
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("仪器处于非待机状态，保存失败。"), TipDlgType::TWO_BUTTON));
        pTipDlg->exec();
        return false;
    }

    ImDeviceOtherInfo washSet;
    if (!DecodeJson(washSet, DevDate->otherInfo))
    {
        TipDlg(tr("提示"), tr("保存失败")).exec();
        ULOG(LOG_ERROR, "DecodeJson ImDeviceOtherInfo Failed");
        return false;
    }

    //传输当前选择的模块号和选项到DCS层
    TransferChoseItem(DevDate->deviceSN, washSet);

    ULOG(LOG_INFO, "Device Supply Change %s ,bConfectLiquid1:%d,bConfectLiquid2:%d,bSubstrate1:%d,bSubstrate2:%d", DevDate->deviceSN.c_str(),int(washSet.bConfectLiquid1),
        int(washSet.bConfectLiquid2), int(washSet.bSubstrate1), int(washSet.bSubstrate2));

    //发送更新消息给其它界面
    POST_MESSAGE(MSG_ID_CONSUMABLES_MANAGE_UPDATE, QString::fromStdString(DevDate->deviceSN), washSet);

    return true;
}

///
/// @brief  传输选中的模块号和选项
///
/// @param[in]    
///
/// @return 
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年2月3日，新建函数
///
bool QImSupplyManageModel::TransferChoseItem(const std::string& DeviceSn,const ImDeviceOtherInfo& imDevInfo)
{
    //组装传输数据
    im::tf::UserConsumablesState m_UserConsumablesState;
    m_UserConsumablesState.__set_MsgType(::im::tf::UtilityMsgType::MSG_TYPE_CONSUMABLES_INFOS);
    m_UserConsumablesState.__set_DeviceSn(DeviceSn);
    m_UserConsumablesState.__set_bSubstrate1(imDevInfo.bSubstrate1);
    m_UserConsumablesState.__set_bSubstrate2(imDevInfo.bSubstrate2);
    m_UserConsumablesState.__set_bConfectLiquid1(imDevInfo.bConfectLiquid1);
    m_UserConsumablesState.__set_bConfectLiquid2(imDevInfo.bConfectLiquid2);
    //发送用户耗材管理配置
    im::LogicControlProxy::UpdateUtilityUserChose(m_UserConsumablesState);

    return true;
}

void ConsumablesManageDlg::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    // 若为生化或生免联机则显示生化
    if (CommonInformationManager::GetInstance()->GetSoftWareType() == CHEMISTRY
        || CommonInformationManager::GetInstance()->GetSoftWareType() == CHEMISTRY_AND_IMMUNE)
    {
        // 生化耗材配置
        if (UserInfoManager::GetInstance()->IsPermisson(PSM_ISE_SAMPLENEEDLE_WASHCFG))
        {
            // 工程师权限应该显示两页
            if (ui->tabWidget->tabBar()->count() < 2)
            {
                ui->tabWidget->insertTab(0, ui->tab_ch, tr("生化"));
            }
        }
        else
        {
            // 低权限应该显示一页
            if (ui->tabWidget->tabBar()->count() == 2)
            {
                ui->tabWidget->removeTab(0);
            }
        }
    }
}

bool QImSupplyManageModel::setData(const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/)
{
    if (role != (Qt::UserRole + 1))
    {
        return false;
    }

    // 获取编辑的行和列
    const int col = index.column();
    const int row = index.row();

    if (row < 0 || row >= m_data.size()
        || col < 0 || col >= m_heads.size())
    {
        return false;
    }

    // 只有选择列可以被改动
    if ( col != 5 && col != 4 &&  col != 3 && col != 2)
    {
        return false;
    }

    auto& data = m_data[row];

    if (data == Q_NULLPTR)
    {
        return false;
    }

    ImDeviceOtherInfo washSet;
    if (!DecodeJson(washSet, data->otherInfo))
    {
        ULOG(LOG_ERROR, "DecodeJson ImDeviceOtherInfo Failed");
    }

    // 底物液1
    if (col == 2)
    {
        if (IsChangeSub(data->deviceSN, !washSet.bSubstrate1))
        {
            washSet.bSubstrate1 = !washSet.bSubstrate1;
        }
        else
        {
            return false;
        }
    }

    // 底物液2
    if (col == 3)
    {
        if (IsChangeSub(data->deviceSN, !washSet.bSubstrate2))
        {
            washSet.bSubstrate2 = !washSet.bSubstrate2;
        }
        else
        {
            return false;
        }
    }

    // 清洗缓冲液1
    if (col == 4)
    {
        if (IsChangeConfectLiquid(data->deviceSN, !washSet.bConfectLiquid1, 1))
        {
            washSet.bConfectLiquid1 = !washSet.bConfectLiquid1;
        }
        else
        {
            return false;
        }
    }

    // 清洗缓冲液2
    if (col == 5)
    {
        if (IsChangeConfectLiquid(data->deviceSN, !washSet.bConfectLiquid2, 2))
        {
            washSet.bConfectLiquid2 = !washSet.bConfectLiquid2;
        }
        else
        {
            return false;
        }
    }

    // 回写值到耗材信息中
    std::string xml;
    if (!Encode2Json(xml, washSet))
    {
        return false;
    }

    beginResetModel();
    data->__set_otherInfo(xml);
    endResetModel();
    return true;
}

///
/// @brief  重写模型数据显示
///
/// @param[in]    index：当前行列 role：数据角色类型
///
/// @return QVariant
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年7月11日，新建函数
///
QVariant QImSupplyManageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    int row = index.row();
    int column = index.column();

    if (row < 0 || row >= m_data.size()
        || column < 0 || column >= m_heads.size())
    {
        return QVariant();
    }

    auto data = m_data[row];
    if (data == Q_NULLPTR)
    {
        return QVariant();
    }
    if (role == Qt::DisplayRole)
    {
        switch (column)
        {
        case 0:
            return row + 1;
            break;
        case 1:
            return QString::fromStdString(data->groupName + data->name);
            break;
        case 2:
            return QVariant();
            break;
        case 3:
            return QVariant();
            break;
        case 4:
            return QVariant();
            break;
        case 5:
            return QVariant();
            break;
        default:
            return QVariant();
            break;
        }
    }
    // 对齐方式
    else if (role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
    }
    // 特殊处理，选择项
    else if (role == (Qt::UserRole + 1))
    {
        if (column != 2 && column != 3 && column != 4 && column != 5)
        {
            return false;
        }

        ImDeviceOtherInfo washSet;
        if (!DecodeJson(washSet, data->otherInfo))
        {
            return false;
        }

        bool select = false;
        if (column == 2)
        {
            select = washSet.bSubstrate1;
        }
        else if (column == 3)
        {
            select = washSet.bSubstrate2;
        }
        else if (column == 4)
        {
            select = washSet.bConfectLiquid1;
        }
        else
        {
            select = washSet.bConfectLiquid2;
        }

        if (select)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return QVariant();
    }
}

///
/// @brief  保存数据
///
///
/// @return 
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年7月11日，新建函数
///
bool QImSupplyManageModel::SaveData()
{
	// 上次设置的数据和当前显示的数据不一致则返回
	if (m_VecConsumSet.size() != m_data.size())
	{
		ULOG(LOG_ERROR, "%s() : m_VecConsumSet.size() != m_data.size()", __FUNCTION__);
		return false;
	}

	// 获取底物填充的id
	int iGroupID = GetSingleMaintGroupId(::tf::MaintainItemType::type::MAINTAIN_ITEM_LOAD_FILL_SUBS);

	// 遍历数据并修改和执行维护
	for (int i = 0; i < m_data.size(); i++)
	{
		::tf::DeviceInfo di;
		di.__set_deviceSN(m_data[i]->deviceSN);
		di.__set_otherInfo(m_data[i]->otherInfo);

		if (!OnSaveBtnClicked(m_data[i]))
		{
			ULOG(LOG_ERROR, "%s() : modify device failed", __FUNCTION__);
			continue;
		}

		if (!CommonInformationManager::GetInstance()->ModifyDeviceInfo(di))
		{
			ULOG(LOG_ERROR, "%s() : modify device failed", __FUNCTION__);
			continue;
		}

		// 解码数据
		ImDeviceOtherInfo washSet;
		if (!DecodeJson(washSet, m_data[i]->otherInfo))
		{
			continue;
		}

		// 如果有启用底物组，就执行底物填充
		if ((m_VecConsumSet[i].bSubstrate1 || !washSet.bSubstrate1) && (m_VecConsumSet[i].bSubstrate2 || !washSet.bSubstrate2))
		{
			continue;
		}

		// 如果获取id失败
		if (iGroupID == -1)
		{
			continue;
		}

		if (IsNeedToFillPipe(m_data[i]->deviceSN))
		{
			// 构造执行维护的设备信息
			::tf::DevicekeyInfo stuTfDevKeyInfo;
			stuTfDevKeyInfo.__set_sn(m_data[i]->deviceSN);

			// 执行维护
			tf::MaintainExeParams exeParams;
			exeParams.__set_groupId(iGroupID);
			exeParams.__set_lstDev({ stuTfDevKeyInfo });
			if (!DcsControlProxy::GetInstance()->Maintain(exeParams))
			{
				ULOG(LOG_ERROR, "%s(), Maintain() failed", __FUNCTION__);
			}
		}
	}

    //保存后修改存储的初始值
    for (int i = 0; i < m_data.size(); i++)
    {
        ImDeviceOtherInfo washSet;
        // 回写值到耗材信息中
        if (!DecodeJson(washSet, m_data[i]->otherInfo))
        {
            return false;
        }
        //初始化保存设置
        m_VecConsumSet[i] = washSet;
    }

    return true;
}

///
/// @brief  更新耗材管理界面刷新数据
///
/// @param[in]    void
///
/// @return void
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年7月11日，新建函数
///
void QImSupplyManageModel::UpdataShowData()
{
    if (m_VecConsumSet.empty())
    {
        return;
    }
    for (int i = 0; i < m_data.size(); i ++)
    {
        // 回写值到耗材信息中
        std::string xml;
        if (!Encode2Json(xml, m_VecConsumSet[i]))
        {
            return;
        }
        m_data[i]->otherInfo = xml;
    }
}

///
/// @brief  重写模型数据行数
///
/// @param[in]    QModelIndex：模型索引
///
/// @return int ：行数
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年7月11日，新建函数
///
int QImSupplyManageModel::rowCount(const QModelIndex &parent) const
{
    return m_data.size();
}

///
/// @brief  重写模型数据列数
///
/// @param[in]    QModelIndex：模型索引
///
/// @return int ：列数
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年7月11日，新建函数
///
int QImSupplyManageModel::columnCount(const QModelIndex &parent) const
{
    return m_heads.size();
}

///
/// @brief  重写模型表头内容
///
/// @param[in]    section:部分  orientation:方向 role：角色
///
/// @return QVariant
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年7月11日，新建函数
///
QVariant QImSupplyManageModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    if (section < 0 || section > m_heads.size())
    {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            return m_heads.at(section);
        }
        else if(role == Qt::ForegroundRole && (section == 2 || section == 3 || section == 4 || section == 5))
        {
             return QVariant(QColor(2,91,199));
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

///////////////////////////////////////////////////////////////////////////
/// @class     ConsumablesItemTableDelegate
/// @brief 	   维护项表格提示框代理
///////////////////////////////////////////////////////////////////////////
ConsumablesItemTableDelegate::ConsumablesItemTableDelegate(QObject * parent)
    : QStyledItemDelegate(parent)
{
}

bool ConsumablesItemTableDelegate::editorEvent(QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index)
{
    if (event->type() == QEvent::MouseMove)
    {
        QString text = index.data().toString();
        int iTextWidth = option.fontMetrics.width(text) + 20;
        int icellWidth = option.rect.width();
        if (iTextWidth > icellWidth)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (option.rect.contains(mouseEvent->pos()))
            {
                QToolTip::showText(mouseEvent->globalPos(), text);
            }
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
