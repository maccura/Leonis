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

#pragma once
#include "UserInfoManager.h"
#include "UserInfoRoleAuthorityList.h"
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "thrift/DcsControlProxy.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "src/common/StringUtil.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/common.h"

///
/// @brief 初始化单例对象
///
std::shared_ptr<UserInfoManager> UserInfoManager::s_instance = nullptr;

///
/// @brief 初始化单例锁
///
std::mutex UserInfoManager::m_userMgrMutx;


UserInfoManager::UserInfoManager()
{
    m_currUserInfo = nullptr;
    
    // 单项维护类型与权限ID的对应关系
    std::map<tf::MaintainItemType::type, int> tempItemPermissio{
        {tf::MaintainItemType::type::MAINTAIN_ITEM_RESET,               PSM_MANTI_INSTRRESET},  // 仪器复位   
        {tf::MaintainItemType::type::MAINTAIN_ITEM_REAGENT_SCAN,		PSM_MANTI_REAGENTSCAN}, // 试剂扫描
        {tf::MaintainItemType::type::MAINTAIN_ITEM_REA_VORTEX,          PSM_MANTI_REAGENTVOTI}, // 试剂混匀
        {tf::MaintainItemType::type::MAINTAIN_ITEM_FILL_PIP,            PSM_MANTI_WHOLTUBFILL}, // 整机管路填充
        {tf::MaintainItemType::type::MAINTAIN_ITEM_RESET_AFTER_TEST,    PSM_MANTI_TESTENDMANT}, // 测试结束维护
        {tf::MaintainItemType::type::MAINTAIN_ITEM_LOAD_FILL_SUBS,      PSM_MANTI_SUBTRTUBFILL }, //底物管路填充
        {tf::MaintainItemType::type::MAINTAIN_ITEM_MAG_FILL_PIP,        PSM_MANTI_MGCDEVTUBFILL }, // 磁分离管路填充
        {tf::MaintainItemType::type::MAINTAIN_ITEM_CLEAN_TRACK,         PSM_MANTI_CLEANSAMPLESHIFT }, // 清除样本架
        {tf::MaintainItemType::type::MAINTAIN_ITEM_CLEAN_CUP_TRACK,     PSM_MANTI_CLEANRECTTUB }, // 清除反应杯
        {tf::MaintainItemType::type::MAINTAIN_ITEM_CHANGE_USESTATUSE_FILL_SUBS, PSM_MANTI_SUBSTOPTUBMANT }, // 底物停用管路维护
        {tf::MaintainItemType::type::MAINTAIN_ITEM_AUTO_SHUTDOWN,       PSM_MANTI_INSTPOWEROFF }, // 仪器关机
        {tf::MaintainItemType::type::MAINTAIN_ITEM_CLEAR_FILL_PIPE_CUPS, PSM_MANTI_CLEANTUBFILLTUB }, // 清除管路填充管
        {tf::MaintainItemType::type::MAINTAIN_ITEM_WASH_PIP,            PSM_MANTI_WHOLTUBWASH }, // 整机管路清洗
        {tf::MaintainItemType::type::MAINTAIN_ITEM_SUBSTRATE_WASH_PIP,  PSM_MANTI_SUBTUBWASH }, // 底物管路清洗
        {tf::MaintainItemType::type::MAINTAIN_ITEM_MAG_WASH_PIP,        PSM_MANTI_MGCDEVTUBWASH }, // 磁分离管路清洗
        {tf::MaintainItemType::type::MAINTAIN_ITEM_WASH_SAMPLE_NEEDLE,  PSM_MANTI_SAMPLENDWASH }, // 样本针清洗
        {tf::MaintainItemType::type::MAINTAIN_ITEM_WASH_REGENT_NEEDLE,  PSM_MANTI_REAGENTNDWASH }, //试剂针清洗
        {tf::MaintainItemType::type::MAINTAIN_ITEM_FILL_WASH_POOL,      PSM_MANTI_FULLNDPOOL }, //洗针池灌注
        {tf::MaintainItemType::type::MAINTAIN_ITEM_CONFECT_LIQUID_CHECK, PSM_MANTI_LIQDISPESELFCKONLINE }, // 在线配液自检
        {tf::MaintainItemType::type::MAINTAIN_ITEM_CONFECT_LIQUID_DRAIN, PSM_MANTI_LIQDISPESELFCLRONLINE }, // 在线配液排空
        {tf::MaintainItemType::type::MAINTAIN_ITEM_DRAIN_PIP,           PSM_MANTI_WHOLETUBCLR }, // 整机管路排空
        {tf::MaintainItemType::type::MAINTAIN_ITEM_UPDATE_FILL_PIPE_CUPS, PSM_MANTI_ADDTUBFILLTUB }, // 添加管路填充管
        {tf::MaintainItemType::type::MAINTAIN_ITEM_SOAK_SAMPLE_PIPE,    PSM_MANTI_SMPNDSPECWASH }, // 样本针特殊清洗
        {tf::MaintainItemType::type::MAINTAIN_ITEM_SOAK_WASH_PIPE,      PSM_MANTI_STRATNDSPECWASH }, // 抽液针特殊清洗
        {tf::MaintainItemType::type::MAINTAIN_ITEM_SOAK_SAMPLE_WASH_PIPE, PSM_MANTI_WHOLENDSPECWASH }, // 整机针特殊清洗
        {tf::MaintainItemType::type::MAINTAIN_ITEM_SOAK_SAMPLE_WASH_PIPE_BY_PURE, PSM_MANTI_WHOLENDSOAK }, // 整机针浸泡
        {tf::MaintainItemType::type::MAINTAIN_ITEM_SAMPLE_PLUG_NEEDLE_SELF_TEST, PSM_MANTI_SMPDNBLOCKSCK }, // 样本针堵针自检
        { tf::MaintainItemType::type::MAINTAIN_ITEM_MAG_1_PLUG_NEEDLE_SELF_TEST, PSM_MANTI_MGC1BLOCKSLFCK }, // 第一磁分离堵针自检
        { tf::MaintainItemType::type::MAINTAIN_ITEM_MAG_2_PLUG_NEEDLE_SELF_TEST, PSM_MANTI_MGC2BLOCKSLFCK }, // 第二磁分离堵针自检
        { tf::MaintainItemType::type::MAINTAIN_ITEM_MAG_1_INJECT_VOL_SELF_TEST, PSM_MANTI_MGC1DVINJCVSLCK }, // 第一磁分离注液量自检
        { tf::MaintainItemType::type::MAINTAIN_ITEM_MAG_2_INJECT_VOL_SELF_TEST, PSM_MANTI_MGC2DVINJCVSLCK }, // 第二磁分离注液量自检
        { tf::MaintainItemType::type::MAINTAIN_ITEM_SUB_A_INJECT_VOL_SELF_TEST, PSM_MANTI_SUBAINJCSLFCK }, // 底物A注液量自检
        { tf::MaintainItemType::type::MAINTAIN_ITEM_SUB_B_INJECT_VOL_SELF_TEST, PSM_MANTI_SUBBINJECSLFCK }, // 底物B注液量自检
        { tf::MaintainItemType::type::MAINTAIN_ITEM_AUTO_FAULT_REPAIR,  PSM_MANTI_ERRORAUTOREPARE }, // 故障自动修复
        { tf::MaintainItemType::type::MAINTAIN_ITEM_REAG1_INJECT_VOL_SELF_TEST, PSM_MANTI_REAGENTND1_VOL_SCHECK }, // 第一试剂臂注液量自检
        { tf::MaintainItemType::type::MAINTAIN_ITEM_REAG2_INJECT_VOL_SELF_TEST, PSM_MANTI_REAGENTND2_VOL_SCHECK }  // 第二试剂臂注液量自检
    };
    m_maintItemPermission = tempItemPermissio;

    // 从代码中初始化所有的权限列表
    InitPermitTableFromCode();
}

void UserInfoManager::InitPermitTableFromCode()
{
    // 操作员
    std::map<tf::PermissionType::type, std::string> operatorPermit = {
        { tf::PermissionType::type::PERMISSION_OPERATE,  GetContainerJoinString(vecPermissionOpteratorOp, ",") },
        { tf::PermissionType::type::PERMISSION_SHOW, GetContainerJoinString(vecPermissionOpteratorSh, ",") }
    };
    m_wholePermit[tf::UserType::USER_TYPE_GENERAL] = operatorPermit;

    // 管理员
    std::map<tf::PermissionType::type, std::string> adminPermit = {
        { tf::PermissionType::type::PERMISSION_OPERATE, GetContainerJoinString(vecPermissionAdminOp, ",") },
        { tf::PermissionType::type::PERMISSION_SHOW, GetContainerJoinString(vecPermissionAdminSh, ",") }
    };
    m_wholePermit[tf::UserType::type::USER_TYPE_ADMIN] = adminPermit;

    // 工程师
    std::map<tf::PermissionType::type, std::string> engPermit = {
        { tf::PermissionType::type::PERMISSION_OPERATE, GetContainerJoinString(vecPermissionEngOp, ",") },
        { tf::PermissionType::type::PERMISSION_SHOW, GetContainerJoinString(vecPermissionEngSh, ",") }
    };
    m_wholePermit[tf::UserType::type::USER_TYPE_ENGINEER] = engPermit;

    // 最高权限
    std::map<tf::PermissionType::type, std::string> topPermit = {
        { tf::PermissionType::type::PERMISSION_OPERATE, GetContainerJoinString(vecPermissionTopOp, ",") },
        { tf::PermissionType::type::PERMISSION_SHOW, GetContainerJoinString(vecPermissionTopSh, ",") }
    };
    m_wholePermit[tf::UserType::type::USER_TYPE_TOP] = topPermit;
}

void UserInfoManager::GetRolePermitFromLocal(tf::UserType::type userType, tf::PermissionType::type perType, std::set<int>& vecPermission)
{
    vecPermission.clear();

    auto userIt = m_wholePermit.find(userType);
    if (userIt != m_wholePermit.end())
    {
        auto permitTypeIter = userIt->second.find(perType);
        if (permitTypeIter != userIt->second.end())
        {
            std::vector<std::string> tempPerm;
            boost::algorithm::split(tempPerm, permitTypeIter->second, boost::is_any_of(","));
            for (const std::string& strPer : tempPerm)
            {
                int iTarg = -1;
                if (!stringutil::IsInteger(strPer, iTarg))
                {
                    ULOG(LOG_ERROR, "Invalid perssion string:%s.", strPer.c_str());
                    continue;
                }

                vecPermission.insert(iTarg);
            }
        }
    }
}

UserInfoManager::~UserInfoManager()
{

}

std::shared_ptr<UserInfoManager> UserInfoManager::GetInstance()
{
    if (s_instance == nullptr)
    {
        std::unique_lock<std::mutex> autoLock(m_userMgrMutx);
        if (s_instance == nullptr)
        {
            s_instance.reset(new UserInfoManager());
        }
    }

    return s_instance;
}

void UserInfoManager::InitUserInfo(const std::string& currUserName)
{
    ULOG(LOG_INFO, __FUNCTION__);
    
    if (m_currUserInfo != nullptr)
    {
        m_currUserInfo.reset();
        m_currUserInfo = nullptr;
    }

#if _DEBUG
    // 如果用户名为空，则设置为最高权限
    if (currUserName.empty())
    {
        tf::UserInfoQueryResp qryResp;
        tf::UserInfoQueryCond qryCond;
        qryCond.__set_type(tf::UserType::USER_TYPE_TOP);

        // 查询最高权限用户
        if (!DcsControlProxy::GetInstance()->QueryAllUserInfo(qryResp, qryCond) || (qryResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS))
        {
            ULOG(LOG_ERROR, "Failed to QueryAllUserInfo().");
            return;
        }

        // 查询结果为空，则返回
        if (qryResp.lstUserInfos.empty())
        {
            ULOG(LOG_INFO, "qryResp.lstUserInfos.empty().");
            return;
        }

        // 设置登录用户
        m_currUserInfo = std::make_shared<tf::UserInfo>(qryResp.lstUserInfos[0]);
    }
    else
    {
        DcsControlProxy::GetInstance()->QueryUserInfo(currUserName, m_currUserInfo);
    }
#else
    DcsControlProxy::GetInstance()->QueryUserInfo(currUserName, m_currUserInfo);
#endif

    // 初始化当前用户的权限范围
    if (m_currUserInfo == nullptr)
    {
        ULOG(LOG_ERROR, "Failed to get current UserInfo.");
        return;
    }
    // 获取角色对应的权限
    // 由于软件升级时不再更新该数据库内容信息，所以弃用数据库中存储的权限信息，采用硬编码的信息进行替换
#if 0
    DcsControlProxy::GetInstance()->GetRolePermision(m_currUserInfo->type, tf::PermissionType::type::PERMISSION_OPERATE, m_currPermission);
    DcsControlProxy::GetInstance()->GetRolePermision(m_currUserInfo->type, tf::PermissionType::type::PERMISSION_SHOW, m_currPermissionShow);
#else
    GetRolePermitFromLocal(m_currUserInfo->type, tf::PermissionType::type::PERMISSION_OPERATE, m_currPermission);
    GetRolePermitFromLocal(m_currUserInfo->type, tf::PermissionType::type::PERMISSION_SHOW, m_currPermissionShow);
#endif

    // 通知当前用户权限更新
    POST_MESSAGE(MSG_ID_SYSTEM_PERMISSTION_CHANGED);
}

///
/// @brief
///     获取当前登录用户信息
///
/// @return 当前登录用户信息
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月9日，新建函数
///
std::shared_ptr<tf::UserInfo> UserInfoManager::GetLoginUserInfo()
{
    return m_currUserInfo;
}

///说明：
///1. 用户在维护保养界面能看到的单项维护根据用户权限显示
///2. 用户在维护保养 - 组合设置界面能加入组合、从组合中删除的单项维护根据用户权限进行限制
///3. 默认组合A中含工程师权限维护，普通用户&管理员权限用户在维护保养 - 组合设置界面选择组合A时在已选单项维护表中能看到，但不能删除
///4. 组合B中含工程师权限维护b，普通用户&管理员权限用户在维护详情界面能看到单项维护b，若执行组合B时维护b执行失败，普通用户&管理员权限用户可以在维护详情界面从维护b开始重新维护
///5. 工程师权限用户添加的组合维护普通用户&管理员权限用户看不到
/// 
/// 该函数用于第2、3点，在组合项目中根据用户选项disable组合中包含的单项，使其不可删除
bool UserInfoManager::IsPermissonMaint(tf::MaintainItemType::type signleMaintType)
{
    if (m_currUserInfo == nullptr)
    {
        return true;
    }
    std::map<tf::MaintainItemType::type, int>::iterator it = m_maintItemPermission.find(signleMaintType);
    if (it == m_maintItemPermission.end())
    {
        return true;
    }

    return m_currPermission.find(it->second) != m_currPermission.end();
}

///1. 用户在维护保养界面能看到的单项维护根据用户权限显示
/// 适用于单项维护列表的显示
bool UserInfoManager::IsPermissonMaintShow(tf::MaintainItemType::type signleMaintType)
{
    if (m_currUserInfo == nullptr)
    {
        return true;
    }
    std::map<tf::MaintainItemType::type, int>::iterator it = m_maintItemPermission.find(signleMaintType);
    if (it == m_maintItemPermission.end())
    {
        return true;
    }

    return m_currPermissionShow.find(it->second) != m_currPermissionShow.end();
}
