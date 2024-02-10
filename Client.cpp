//
// Created by alperen on 27.09.2023.
//

#include "Client.h"

Client::Client(string const& _id, string const& _ip, string const& _mac) {
    client_id = _id;
    client_ip = _ip;
    client_mac = _mac;
}

ostream &operator<<(ostream &os, const Client &client) {
    os << "client_id: " << client.client_id << " client_ip: " << client.client_ip << " client_mac: "
       << client.client_mac << endl;
    return os;
}

Client::~Client() {
    log_entries.clear();

    routing_table.clear();

    while (!incoming_queue.empty()) {
        stack<Packet*> packetStack = incoming_queue.front();
        incoming_queue.pop();

        while (!packetStack.empty()) {
            delete packetStack.top();
            packetStack.pop();
        }
    }

    while (!outgoing_queue.empty()) {
        stack<Packet*> packetStack = outgoing_queue.front();
        outgoing_queue.pop();

        while (!packetStack.empty()) {
            delete packetStack.top();
            packetStack.pop();
        }
    }
}


void Client::print()  {
    cout << "Client ID: " << client_id << ", IP: " << client_ip << ", MAC: " << client_mac << endl;
    cout << "Routing Table:" << endl;
    for (const auto &entry: routing_table) {
        cout << entry.first << " -> " << entry.second << endl;
    }
}