#include "dns_client.h"

DNSClient::DNSClient(const std::string &name, std::u16 port)
    : dns_server(AF_INET, SOCK_STREAM), dns_server_addr(name, port) {
  dns_server.set_option(SOL_SOCKET, SO_REUSEADDR, 1);
  dns_server.connect(dns_server_addr);
}

std::pair<DNSHeader, DNSRecord>
DNSClient::query_name(const std::string &name) const {
  DNSHeader header;
  std::memset(&header, 0, sizeof(header));
  header.QDCOUNT = 1;
  auto buffer = DNSHeader::encode(header);
  this->dns_server.send_all(buffer.c_str(), buffer.length());
  DNSQuestion question;
  question.QTYPE = 1;
  question.QCLASS = 1;
  strncpy(question.QNAME, name.c_str(), name.length());
  buffer = DNSQuestion::encode(question);
  std::u32 length = htonl(buffer.length());
  this->dns_server.send_all(&length, 4);
  this->dns_server.send_all(buffer.c_str(), length);
  this->dns_server.sized_receive(&length, 4, MSG_WAITALL);
  length = ntohl(length);
  buffer.resize(length);
  this->dns_server.sized_receive(buffer.data(), length, MSG_WAITALL);
  header = DNSHeader::decode(buffer);
  this->dns_server.sized_receive(&length, 4, MSG_WAITALL);
  length = ntohl(length);
  buffer.resize(length);
  this->dns_server.sized_receive(buffer.data(), length, MSG_WAITALL);
  auto record = DNSRecord::decode(buffer);
  return std::make_pair(std::move(header), std::move(record));
}
