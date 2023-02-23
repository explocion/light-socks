#pragma once

#include "DNSHeader.h"
#include "DNSQuestion.h"
#include "DNSRecord.h"

#include "socket.h"

class DNSClient {
public:
  Socket dns_server;
  IPv4Addr dns_server_addr;

  static constexpr std::u16 DNS_PORT = 53;

  DNSClient(const std::string &name, std::u16 port = DNS_PORT);
  std::pair<DNSHeader, DNSRecord> query_name(const std::string &name) const;
};
