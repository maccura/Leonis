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
/// @file     UserInfoManager.h
/// @brief    记录当前登录的用户信息， 用于界面显示的权限控制
///
/// @author   8276/huchunli
/// @date     2023年3月27日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8276/huchunli，2023年3月27日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <mutex>
#include <memory>
#include <string>
#include <set>
#include <boost/noncopyable.hpp>
#include "UserInfoAuthorityDefine.h"
#include "src/thrift/gen-cpp/defs_types.h"

class UserInfoManager : public boost::noncopyable
{
public:
    static std::shared_ptr<UserInfoManager> GetInstance();

    ///
    /// @bref
    ///		登录软件时初始化当前用户信息
    ///
    /// @param[in] currUserInfo 当前用户
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年3月27日，新建函数
    ///
    void InitUserInfo(const std::string& currUserName);

    ///
    /// @brief
    ///     获取当前登录用户信息
    ///
    /// @return 当前登录用户信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年5月9日，新建函数
    ///
    std::shared_ptr<tf::UserInfo> GetLoginUserInfo();

    ///
    /// @bref
    ///		用户是否已经登录
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年4月3日，新建函数
    ///
    inline bool AlreadyLogin()
    {
        return m_currUserInfo != nullptr;
    }

    ///
    /// @bref
    ///		是否允许对应的权限操作
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年6月26日，新建函数
    ///
    inline bool IsPermisson(int permissonId)
    {
        return m_currUserInfo != nullptr ? m_currPermission.find(permissonId) != m_currPermission.end() : true;
    }

    ///
    /// @bref
    ///		是否允许对应的权限显示
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年6月26日，新建函数
    ///
    inline bool IsPermissionShow(int permissionId)
    {
        return m_currUserInfo != nullptr ? m_currPermissionShow.find(permissionId) != m_currPermissionShow.end() : true;
    }

    ///
    /// @bref
    ///		判断维护项目是否有操作权限
    ///
    /// @param[in] signleMaintType 单项维护类型
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月22日，新建函数
    ///
    bool IsPermissonMaint(tf::MaintainItemType::type signleMaintType);
    bool IsPermissonMaintShow(tf::MaintainItemType::type signleMaintType);


    ~UserInfoManager();

private:
    UserInfoManager();

    ///
    /// @bref
    ///		初始化全部的权限信息
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年7月29日，新建函数
    ///
    void InitPermitTableFromCode();

    ///
    /// @bref
    ///		从本地表获取对应角色的权限信息
    ///
    /// @param[in] userType 角色类型
    /// @param[in] perType 权限类型
    /// @param[out] vecPermission 权限信息
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年7月29日，新建函数
    ///
    void GetRolePermitFromLocal(tf::UserType::type userType, tf::PermissionType::type perType, std::set<int>& vecPermission);


    // 当前类的单例对象
    static std::shared_ptr<UserInfoManager> s_instance;
    static std::mutex m_userMgrMutx;

private:
    std::shared_ptr<tf::UserInfo> m_currUserInfo;                       // 当前登录的用户
    std::set<int> m_currPermission;                                     // 角色权限表-操作
    std::set<int> m_currPermissionShow;                                 // 角色权限表-显示

    std::map<tf::MaintainItemType::type, int> m_maintItemPermission;    // 维护项对应的权限ID

    // 记录所有的角色权限，因为升级时不能修改数据库数据，暂时不再使用数据库的权限数据，直接使用代码证记录
    std::map<tf::UserType::type, std::map<tf::PermissionType::type, std::string>> m_wholePermit;
};


///
/// @bref
///		没有登录时直接返回的判断
///
#define SEG_NO_LOGIN_RETURN do{ if(!UserInfoManager::GetInstance()->AlreadyLogin()) {return; }}while(0)

///
/// @bref
///		注册权限变化函数，以及在注测前运行一次该函数
///
#define SEG_REGIST_PERMISSION(obj, funcName) do{obj->funcName(); REGISTER_HANDLER(MSG_ID_SYSTEM_PERMISSTION_CHANGED, obj, funcName);} while (0);
