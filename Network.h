#ifndef NETWORK_H
#define NETWORK_H

#include <vector>
#include <iostream>
#include "Packet.h"
#include "Client.h"

using namespace std;

class Network {
public:
    Network();
    ~Network();

    // Executes commands given as a vector of strings while utilizing the remaining arguments.
    void process_commands(vector<Client> &clients, vector<string> &commands, int message_limit, const string &sender_port,
                     const string &receiver_port);

    // Initialize the network from the input files.
    vector<Client> read_clients(string const &filename);
    void read_routing_tables(vector<Client> & clients, string const &filename);
    vector<string> read_commands(const string &filename);

    void printClients(const vector<Client> &clients) const;

    void printCommands(const vector<string> &commands) const;

    void prepare_frames(Client &sender, Client &receiver, const string &message, int message_limit, string sender_port,
                   string receiver_port);

    void prepare_frame(Client &sender, Client &next_receiver, Client &receiver, ApplicationLayerPacket *app_packet, int frame_number, string sender_port,
                       string receiver_port);

    bool checkReceiverInRoutingTable(const Client &sender, const Client &receiver);

    void printMessageOutput(const string &sender_mac, const string &receiver_mac, const string &sender_ip,
                            const string &receiver_ip, const string &sender_port, const string &receiver_port,
                            const string &sender_id, const string &receiver_id, const vector<string> &message_chunks,
                            const Client &client);

    Client getNextReceiver(const Client &sender, const Client &receiver) const;

    vector<Client> clientsV;

    string getCurrentTimestamp();

    void printFrameInfo(const Client &client, const string &queue_name, int frame_number);

    void printPacketInfo(Packet *packet);

    void printQueueInfo(const Client &client, const string &queue_selection);
};

#endif  // NETWORK_H
