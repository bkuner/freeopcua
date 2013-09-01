/// @author Alexander Rykovanov 2013
/// @email rykovanov.as@gmail.com
/// @brief Test of opc ua binary handshake.
/// @license GNU LGPL
///
/// Distributed under the GNU LGPL License
/// (See accompanying file LICENSE or copy at
/// http://www.gnu.org/licenses/lgpl.html)
///

#include "opcua_protocol_addon_test.h"

#include <opc/common/addons_core/addon_manager.h>
#include <opc/ua/client/remote_connection.h>
#include <opc/ua/server/addons/builtin_server.h>

#include <src/opcua_protocol_factory.h>
#include "../../address_space/tests/address_space_registry_test.h"
#include "../../endpoint_services/tests/endpoints_services_test.h"
#include "../../services_registry/tests/services_registry_test.h"
#include "../../standard_namespace/tests/standard_namspace_test.h"
#include "../../builtin_server/tests/builtin_server_test.h"

#include <chrono>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iostream>
#include <thread>

using namespace testing;

class OpcUaProtocolAddonTest : public Test
{
public:
  void SetUp()
  {
    Addons = Common::CreateAddonsManager();

    OpcUa::Test::RegisterAddressSpace(*Addons);
    OpcUa::Test::RegisterStandardNamespace(*Addons);
    OpcUa::Test::RegisterBuiltinServerAddon(*Addons);
    OpcUa::Test::RegisterOpcTcpAddon(*Addons);
    OpcUa::Test::RegisterEndpointsServicesAddon(*Addons);
    OpcUa::Test::RegisterServicesRegistry(*Addons);

    Addons->Start();
  }

  void TearDown()
  {
    Addons->Stop();
    Addons.reset();
  }

protected:
  std::unique_ptr<Common::AddonsManager> Addons;
};

TEST_F(OpcUaProtocolAddonTest, Loads)
{
  ASSERT_TRUE(static_cast<bool>(Addons->GetAddon(OpcUa::Server::OpcUaProtocolAddonID)));
}

TEST_F(OpcUaProtocolAddonTest, CanGetComputerWhichOpensAndClosesSecureChannel)
{
  std::shared_ptr<OpcUa::Server::BuiltinServerAddon> computerAddon = Common::GetAddon<OpcUa::Server::BuiltinServerAddon>(*Addons, OpcUa::Server::TcpServerAddonID);
  ASSERT_TRUE(static_cast<bool>(computerAddon));
  std::shared_ptr<OpcUa::Remote::Computer> computer = computerAddon->GetComputer();
  ASSERT_TRUE(static_cast<bool>(computer));
  computer.reset();
}

TEST_F(OpcUaProtocolAddonTest, CanListEndpoints)
{
  std::shared_ptr<OpcUa::Server::BuiltinServerAddon> computerAddon = Common::GetAddon<OpcUa::Server::BuiltinServerAddon>(*Addons, OpcUa::Server::TcpServerAddonID);
  std::shared_ptr<OpcUa::Remote::Computer> computer = computerAddon->GetComputer();
  std::shared_ptr<OpcUa::Remote::EndpointServices> endpoints = computer->Endpoints();
  std::vector<OpcUa::EndpointDescription> desc;
  ASSERT_NO_THROW(desc = endpoints->GetEndpoints(OpcUa::EndpointsFilter()));
  ASSERT_EQ(desc.size(), 1);
  endpoints.reset();
  computer.reset();
}

TEST_F(OpcUaProtocolAddonTest, CanFindServers)
{
  std::shared_ptr<OpcUa::Server::BuiltinServerAddon> computerAddon = Common::GetAddon<OpcUa::Server::BuiltinServerAddon>(*Addons, OpcUa::Server::TcpServerAddonID);
  std::shared_ptr<OpcUa::Remote::Computer> computer = computerAddon->GetComputer();
  std::shared_ptr<OpcUa::Remote::EndpointServices> endpoints = computer->Endpoints();
  std::vector<OpcUa::ApplicationDescription> servers;
  ASSERT_NO_THROW(servers = endpoints->FindServers(OpcUa::FindServersParameters()));
  ASSERT_EQ(servers.size(), 1);
  endpoints.reset();
  computer.reset();
}

TEST_F(OpcUaProtocolAddonTest, CanBrowseRootFolder)
{
  std::shared_ptr<OpcUa::Server::BuiltinServerAddon> computerAddon = Common::GetAddon<OpcUa::Server::BuiltinServerAddon>(*Addons, OpcUa::Server::TcpServerAddonID);
  std::shared_ptr<OpcUa::Remote::Computer> computer = computerAddon->GetComputer();
  std::shared_ptr<OpcUa::Remote::ViewServices> views = computer->Views();

  OpcUa::Remote::BrowseParameters params;
  params.Description.NodeToBrowse = OpcUa::ObjectID::RootFolder;
  params.Description.Direction = OpcUa::BrowseDirection::Forward;
  params.Description.ReferenceTypeID = OpcUa::ReferenceID::Organizes;
  params.Description.IncludeSubtypes = true;
  params.Description.NodeClasses = OpcUa::NODE_CLASS_OBJECT;
  params.Description.ResultMask = OpcUa::REFERENCE_ALL;
  std::vector<OpcUa::ReferenceDescription> referencies = views->Browse(params);
  ASSERT_EQ(referencies.size(), 3);

  views.reset();
  computer.reset();
}

TEST_F(OpcUaProtocolAddonTest, CanCreateSession)
{
  std::shared_ptr<OpcUa::Server::BuiltinServerAddon> computerAddon = Common::GetAddon<OpcUa::Server::BuiltinServerAddon>(*Addons, OpcUa::Server::TcpServerAddonID);
  std::shared_ptr<OpcUa::Remote::Computer> computer = computerAddon->GetComputer();

  OpcUa::Remote::SessionParameters session;
  session.ClientDescription.Name.Text = "opcua client";
  session.SessionName = "opua command line";
  session.EndpointURL = "opc.tcp://localhost:4841";
  session.Timeout = 1000;

  ASSERT_NO_THROW(computer->CreateSession(session));
  ASSERT_NO_THROW(computer->ActivateSession());
  ASSERT_NO_THROW(computer->CloseSession());

  computer.reset();
}

TEST_F(OpcUaProtocolAddonTest, ManipulateSubscriptions)
{
  std::shared_ptr<OpcUa::Server::BuiltinServerAddon> computerAddon = Common::GetAddon<OpcUa::Server::BuiltinServerAddon>(*Addons, OpcUa::Server::TcpServerAddonID);
  std::shared_ptr<OpcUa::Remote::Computer> computer = computerAddon->GetComputer();
  std::shared_ptr<OpcUa::Remote::SubscriptionServices> subscriptions = computer->Subscriptions();

  OpcUa::SubscriptionParameters params;
  params.MaxNotificationsPerPublish = 3;
  params.Priority = 0;
  params.PublishingEnabled = true;
  params.RequestedLifetimeCount = 3;
  params.RequestedMaxKeepAliveCount = 3;
  params.RequestedPublishingInterval = 1000;

  OpcUa::SubscriptionData data;
  ASSERT_NO_THROW(data = subscriptions->CreateSubscription(params));

  subscriptions.reset();
  computer.reset();
}
