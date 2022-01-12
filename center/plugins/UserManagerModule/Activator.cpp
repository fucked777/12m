/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/

//! [Activator]

#include "CppMicroServicesUtility.h"
#include "ExtendedConfig.h"
#include "GlobalData.h"
#include "PlatformInterface.h"
#include "UserListWidget.h"
#include "UserLoginWidget.h"
#include "UserManagerService.h"
#include "UserRegisterWidget.h"
#include "ZMQUtility.h"
#include <QMessageBox>
#include <UserModifyWidget.h>

using namespace cppmicroservices;

template<typename T>
static void registerNetService(BundleContext context, const char* type)
{
    auto server = std::make_shared<T>(context);
    ServiceProperties props;
    props["Type"] = std::string(type);
    context.RegisterService<T>(server, props);

    server->setConnectAddr(ZMQHelper::deaultLocalAddress());
    server->setSrcAddr(type);
    QString msg;
    if (!server->startServer(&msg))
    {
        qWarning() << __FUNCTION__ << msg;
    }
}

namespace
{
    /**
     * This class implements a bundle activator that uses the bundle
     * context to register an English language dictionary service
     * with the C++ Micro Services registry during static initialization
     * of the bundle. The dictionary service interface is
     * defined in a separate file and is implemented by a nested class.
     */
    class US_ABI_LOCAL UserManagerModuleActivator : public BundleActivator
    {
    private:
        class GuiImpl : public IGuiService
        {
        public:
            explicit GuiImpl(BundleContext context)
                : m_context(context)
            {
            }
            virtual ~GuiImpl() {}

            virtual QWidget* createWidget(const QString& subWidgetName) override
            {
                if (subWidgetName == "UserLogin")
                {
                    auto loginWidget = new UserLoginWidget(m_context);
                    if (m_context)
                    {
                        auto ref = m_context.GetServiceReference<UserManagerService>();
                        auto service = m_context.GetService(ref);
                        QObject::connect(loginWidget, &UserLoginWidget::signalUserLogin, service.get(), &UserManagerService::slotUserLogin);
                        QObject::connect(service.get(), &UserManagerService::signalUserLoginACK, loginWidget,
                                         &UserLoginWidget::slotUserLoginResponse);
                        QObject::connect(loginWidget, &UserLoginWidget::signalUserLogout, service.get(), &UserManagerService::slotUserLogout);
                        QObject::connect(service.get(), &UserManagerService::signalUserLogoutACK, loginWidget,
                                         &UserLoginWidget::slotUserLogoutResponse);
                    }
                    return loginWidget;
                }
                if (subWidgetName == "UserRegister")
                {
                    auto currentuser = GlobalData::getCurrentUser();
                    if ("2" != currentuser.identity)
                    {
                        QMessageBox::information(nullptr, QString("提示"), QString("只有管理员才能进行用户注册"), QString("确定"));
                        return nullptr;
                    }

                    auto registerWidget = new UserRegisterWidget;
                    if (m_context)
                    {
                        auto ref = m_context.GetServiceReference<UserManagerService>();
                        auto service = m_context.GetService(ref);
                        QObject::connect(registerWidget, &UserRegisterWidget::signalUserRegister, service.get(),
                                         &UserManagerService::slotUserRegister);
                        QObject::connect(service.get(), &UserManagerService::signalUserRegisterACK, registerWidget,
                                         &UserRegisterWidget::slotUserRegisterResponse);
                    }
                    return registerWidget;
                }
                if (subWidgetName == "UserList")
                {
                    auto currentuser = GlobalData::getCurrentUser();
                    if ("2" != currentuser.identity)
                    {
                        QMessageBox::information(nullptr, QString("提示"), QString("只有管理员才能查看用户列表"), QString("确定"));
                        return nullptr;
                    }

                    auto userListWidget = new UserListWidget;
                    if (m_context)
                    {
                        auto ref = m_context.GetServiceReference<UserManagerService>();
                        auto service = m_context.GetService(ref);
                        QObject::connect(userListWidget, &UserListWidget::signalGetUserList, service.get(), &UserManagerService::slotGetUserList);
                        QObject::connect(service.get(), &UserManagerService::signalGetUserListACK, userListWidget,
                                         &UserListWidget::slotGetUserListResponse);

                        QObject::connect(userListWidget, &UserListWidget::signalDeleteUser, service.get(), &UserManagerService::slotDeleteUser);
                        QObject::connect(service.get(), &UserManagerService::signalDeleteUserACK, userListWidget,
                                         &UserListWidget::slotDeleteUserResponse);

                        auto userModifyWidget = new UserModifyWidget;
                        userListWidget->initUserModifyDialog(userModifyWidget);
                        QObject::connect(userModifyWidget, &UserModifyWidget::signalModifyUser, service.get(), &UserManagerService::slotModifyUser);
                        QObject::connect(service.get(), &UserManagerService::signalModifyUserACK, userModifyWidget,
                                         &UserModifyWidget::slotModifyUserResponse);
                        QObject::connect(service.get(), &UserManagerService::signalModifyUserACK, userListWidget,
                                         &UserListWidget::slotModifyUserResponse);
                    }
                    return userListWidget;
                }
                return nullptr;
            }

        private:
            BundleContext m_context;
        };

    public:
        /**
         * Implements BundleActivator::Start(). Registers an
         * instance of a dictionary service using the bundle context;
         * attaches properties to the service that can be queried
         * when performing a service look-up.
         * @param context the context for the bundle.
         */
        void Start(BundleContext context)
        {
            auto guiService = std::make_shared<GuiImpl>(context);
            ServiceProperties props;
            props["Type"] = std::string("Widget");
            context.RegisterService<IGuiService>(guiService, props);

            registerNetService<UserManagerService>(context, "ClientUserManagerService");
        }

        /**
         * Implements BundleActivator::Stop(). Does nothing since
         * the C++ Micro Services library will automatically unregister any registered services.
         * @param context the context for the bundle.
         */
        void Stop(BundleContext /*context*/)
        {
            // NOTE: The service is automatically unregistered
        }
    };
}  // namespace

CPPMICROSERVICES_INITIALIZE_BUNDLE
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(UserManagerModuleActivator)
//![Activator]
