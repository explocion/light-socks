#include "dns_client.h"

DNSClient::DNSClient(const std::string &name)
    : dns_server(AF_INET, SOCK_STREAM), dns_server_addr(name, DNS_PORT) {
  dns_server.set_option(SOL_SOCKET, SO_REUSEADDR, 1);
  dns_server.connect(dns_server_addr);
}
