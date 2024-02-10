#include "Network.h"
#include "Client.h"
#include <stack>
#include <queue>
#include "ApplicationLayerPacket.h"

Network::Network() {

}

Network::~Network() {
    // TODO: Free any dynamically allocated memory if necessary.
}

void Network::process_commands(vector<Client> &clients, vector<string> &commands, int message_limit,
                               const string &sender_port, const string &receiver_port) {

    clientsV = clients;

    for (const string &command : commands) {
        stringstream ss(command);
        string cmd;
        ss >> cmd;

        if (cmd == "MESSAGE") {
            string sender_id, receiver_id, message;
            ss >> sender_id >> receiver_id;

            // Collect the message which might contain spaces
            getline(ss, message, '#');  // Read until #
            getline(ss, message, '#');  // Read the message content between #

            // Find the sender and receiver in clients vector
            Client *sender = nullptr;
            Client *receiver = nullptr;
            for (Client &client : clients) {
                if (client.client_id == sender_id) {
                    sender = &client;
                }
                if (client.client_id == receiver_id) {
                    receiver = &client;
                }
            }

            // Check if sender and receiver are found
            if (sender != nullptr && receiver != nullptr) {
                // Prepare and queue frames for transmission
                prepare_frames(*sender, *receiver, message, message_limit, sender_port, receiver_port);
            } else {
                cout << "Sender or receiver not found." << endl;
            }




        } else if (cmd == "SHOW_FRAME_INFO") {
            string client_id, queue_selection;
            int frame_number;
            ss >> client_id >> queue_selection >> frame_number;

            Client *client = nullptr;
            for (Client &c : clients) {
                if (c.client_id == client_id) {
                    client = &c;
                    break;
                }
            }

            if (client != nullptr) {
                if (queue_selection == "in") {
                    if (frame_number >= 0 && frame_number < client->incoming_queue.size()) {
                        printFrameInfo(*client, "in", frame_number);
                    } else {
                        string tempmessage = "Command: SHOW_FRAME_INFO " + client_id + " in " + to_string(frame_number);

                        cout << string(tempmessage.size(), '-') << endl;
                        cout << tempmessage << endl;
                        cout << string(tempmessage.size(), '-') << endl;

                        cout << "No such frame.\n";
                    }
                } else if (queue_selection == "out") {
                    if (frame_number >= 0 && frame_number < client->outgoing_queue.size()) {
                        printFrameInfo(*client, "out", frame_number);
                    } else {
                        string tempmessage = "Command: SHOW_FRAME_INFO " + client_id + " out " + to_string(frame_number);

                        cout << string(tempmessage.size(), '-') << endl;
                        cout << tempmessage << endl;
                        cout << string(tempmessage.size(), '-') << endl;

                        cout << "No such frame.\n";
                    }
                } else {
                    cerr << "Invalid queue selection: " << queue_selection << endl;
                }
            } else {
                cerr << "Client not found: " << client_id << endl;
            }




        } else if (cmd == "SHOW_Q_INFO") {
            string client_id, queue_selection;
            ss >> client_id >> queue_selection;

            Client *client = nullptr;
            for (Client &c : clients) {
                if (c.client_id == client_id) {
                    client = &c;
                    break;
                }
            }

            if (client != nullptr) {
                printQueueInfo(*client, queue_selection);
            } else {
                cout << "Client not found: " << client_id << endl;
            }




        } else if (cmd == "SEND") {
            // Process SEND command
            // For each client in the network
            cout << "-------------\n";
            cout << "Command: SEND\n";
            cout << "-------------\n";
            for (Client &sender : clients) {
                // Check the sender's outgoing queue
                while (!sender.outgoing_queue.empty()) {

                    // Dequeue the top frame
                    stack<Packet *> &outgoing_frame = sender.outgoing_queue.front();  // frame
                    stack<Packet *> frame = outgoing_frame;

                    string receiverString;
                    string realReceiverString;
                    stack<Packet *> copyFrame = outgoing_frame;
                    while (!copyFrame.empty()) {
                        stack<Packet *> packet = copyFrame;
                        if (ApplicationLayerPacket *applicationLayer = dynamic_cast<ApplicationLayerPacket *>(packet.top())) {
                            realReceiverString = applicationLayer->receiver_ID;
                            receiverString = sender.routing_table[realReceiverString];
                        }
                        copyFrame.pop();
                    }

                    Client *receiver = nullptr;
                    Client *realReceiver = nullptr;
                    for (Client &c : clients) {
                        if (c.client_id == receiverString) {
                            receiver = &c;
                        }
                        if (c.client_id == realReceiverString) {
                            realReceiver = &c;
                        }
                    }

                    PhysicalLayerPacket *packetPhyscial;

                    // Update the PhysicalLayerPacket MAC address for forwarding
                    copyFrame = outgoing_frame;
                    while (!copyFrame.empty()) {
                        stack<Packet *> packet = copyFrame;
                        if (PhysicalLayerPacket *physical_packet = dynamic_cast<PhysicalLayerPacket *>(packet.top())) {
                            // Update the receiver's MAC address for the frame
                            if (receiver != nullptr){
                                physical_packet->receiver_MAC_address = receiver->client_mac;
                            }
                            // Update the hop count
                            physical_packet->number_of_hops++;
                            packetPhyscial = physical_packet;
                        }
                        copyFrame.pop();
                    }

                    string allMessage;
                    int frameNumber;
                    Client *realSender = nullptr;

                    copyFrame = outgoing_frame;
                    while (!copyFrame.empty()) {
                        stack<Packet *> packet = copyFrame;
                        if (ApplicationLayerPacket *applicationLayer = dynamic_cast<ApplicationLayerPacket *>(packet.top())) {

                            for (Client &c : clients) {
                                if (c.client_id == applicationLayer->sender_ID) {
                                    realSender = &c;
                                    break;
                                }
                            }


                            if(applicationLayer->frame_number == 1){
                                allMessage = "";
                                allMessage += applicationLayer->message_data;
                                frameNumber = 1;
                            } else {
                                allMessage += applicationLayer->message_data;
                                frameNumber++;
                            }
                            cout << "Client "<< sender.client_id <<" sending frame #" << applicationLayer->frame_number << " to client "<< receiver->client_id << endl;
                            cout << "Sender MAC address: " << sender.client_mac << ", Receiver MAC address: " << receiver->client_mac << endl;
                            cout << "Sender IP address: " << realSender->client_ip << ", Receiver IP address: " << realReceiver->client_ip << endl;
                            cout << "Sender port number: " << sender_port << ", Receiver port number: " << receiver_port << endl;
                            cout << "Sender ID: " << realSender->client_id << ", Receiver ID: " << realReceiver->client_id << endl;
                            cout << "Message chunk carried: \"" << applicationLayer->message_data << "\"" << endl;
                            cout << "Number of hops so far: " << packetPhyscial->number_of_hops << endl;
                            cout << "--------" << endl;
                        }
                        copyFrame.pop();
                    }

                    // Move the frame to the next receiver's incoming queue
                    receiver->incoming_queue.push(sender.outgoing_queue.front());
                    sender.outgoing_queue.pop();

                    if (allMessage.find_first_of(".?!") != string::npos) {
                        receiverString = receiver->routing_table[realReceiverString];
                        Client *nextReceiver = nullptr;
                        for (Client &c : clients) {
                            if (c.client_id == receiverString) {
                                nextReceiver = &c;
                                break;
                            }
                        }

                        if (nextReceiver != nullptr){
                            // Update Log entries
                            Log log_entry(getCurrentTimestamp(), allMessage, frameNumber,packetPhyscial->number_of_hops,
                                          realSender->client_id, realReceiver->client_id, true, ActivityType::MESSAGE_FORWARDED);
                            if (realReceiver->client_id != receiver->client_id){
                                receiver->log_entries.push_back(log_entry);
                            }
                        }
                    }

                }
            }
        } else if (cmd == "RECEIVE") {
            // Process RECEIVE command
            cout << "----------------\n";
            cout << "Command: RECEIVE\n";
            cout << "----------------\n";

            string allMessage = "";

            for (Client &receiver : clients) {
                while (!receiver.incoming_queue.empty()) {
                    stack<Packet *> &incoming_frame = receiver.incoming_queue.front(); // frame
                    stack<Packet *> frame = incoming_frame;

                    string receiverString;
                    string realReceiverString;
                    stack<Packet *> copyFrame = incoming_frame;
                    while (!copyFrame.empty()) {
                        stack<Packet *> packet = copyFrame;
                        if (ApplicationLayerPacket *applicationLayer = dynamic_cast<ApplicationLayerPacket *>(packet.top())) {
                            realReceiverString = applicationLayer->receiver_ID;
                            receiverString = receiver.routing_table[realReceiverString];
                        }
                        copyFrame.pop();
                    }

                    if (receiver.client_id ==  realReceiverString){
                        receiverString = realReceiverString;
                    }

                    Client *nextReceiver = nullptr;
                    for (Client &c : clients) {
                        if (c.client_id == receiverString) {
                            nextReceiver = &c;
                            break;
                        }
                    }

                    Client *realReceiver = nullptr;
                    for (Client &c : clients) {
                        if (c.client_id == realReceiverString) {
                            realReceiver = &c;
                            break;
                        }
                    }

                    PhysicalLayerPacket *packetPhysical = nullptr;
                    string senderMacString;

                    // Update the PhysicalLayerPacket MAC address for forwarding
                    copyFrame = incoming_frame;
                    while (!copyFrame.empty()) {
                        stack<Packet *> packet = copyFrame;
                        if (PhysicalLayerPacket *physical_packet = dynamic_cast<PhysicalLayerPacket *>(packet.top())) {
                            // Update the receiver's MAC address for the frame
                            senderMacString = physical_packet->sender_MAC_address;
                            if (nextReceiver != nullptr) {
                                physical_packet->receiver_MAC_address = nextReceiver->client_mac;
                            }
                            physical_packet->sender_MAC_address = receiver.client_mac;
                            packetPhysical = physical_packet;
                        }
                        copyFrame.pop();
                    }

                    Client *sender = nullptr;
                    for (Client &c : clients) {
                        if (c.client_mac == senderMacString) {
                            sender = &c;
                            break;
                        }
                    }

                    int frameNumber;

                    copyFrame = incoming_frame;
                    while (!copyFrame.empty()) {
                        stack<Packet *> packet = copyFrame;
                        if (ApplicationLayerPacket *applicationLayer = dynamic_cast<ApplicationLayerPacket *>(packet.top())) {


                            Client *realSender = nullptr;
                            for (Client &c : clients) {
                                if (c.client_id == applicationLayer->sender_ID) {
                                    realSender = &c;
                                    break;
                                }
                            }

                            if (nextReceiver == nullptr){
                                if(applicationLayer->frame_number == 1){
                                    frameNumber = 1;
                                    allMessage = "";
                                    cout << "Client " << receiver.client_id << " receiving frame #" << frameNumber << " from client " << sender->client_id << ", but intended for client " << realReceiver->client_id << ". Forwarding... " << endl;
                                    cout << "Error: Unreachable destination. Packets are dropped after " << packetPhysical->number_of_hops << " hops!" << endl;
                                    allMessage += applicationLayer->message_data;
                                    if (allMessage.find_first_of(".?!") != string::npos) {
                                        cout << "--------" << endl;
                                        // Update Log entries
                                        Log log_entry(getCurrentTimestamp(), allMessage, frameNumber,packetPhysical->number_of_hops,
                                                      realSender->client_id, realReceiver->client_id, false, ActivityType::MESSAGE_DROPPED);
                                        receiver.log_entries.push_back(log_entry);

                                        stack<Packet*> tempStack = frame;
                                        // Print information for each layer in the frame
                                        while (!tempStack.empty()) {


                                            delete tempStack.top();
                                            tempStack.pop();
                                        }

                                    }
                                } else {
                                    frameNumber++;
                                    cout << "Client " << receiver.client_id << " receiving frame #" << frameNumber << " from client " << sender->client_id << ", but intended for client " << realReceiver->client_id << ". Forwarding... " << endl;
                                    cout << "Error: Unreachable destination. Packets are dropped after " << packetPhysical->number_of_hops << " hops!" << endl;
                                    allMessage += applicationLayer->message_data;
                                    if (allMessage.find_first_of(".?!") != string::npos) {
                                        cout << "--------" << endl;
                                        // Update Log entries
                                        Log log_entry(getCurrentTimestamp(), allMessage, frameNumber,packetPhysical->number_of_hops,
                                                      realSender->client_id, realReceiver->client_id, false, ActivityType::MESSAGE_DROPPED);
                                        receiver.log_entries.push_back(log_entry);

                                    }

                                    stack<Packet*> tempStack = frame;
                                    // Print information for each layer in the frame
                                    while (!tempStack.empty()) {


                                        delete tempStack.top();
                                        tempStack.pop();
                                    }
                                }
                                copyFrame.pop();
                                continue;
                            }


                            if(applicationLayer->frame_number == 1){

                                allMessage = "";
                                if (receiver.client_id != realReceiver->client_id){
                                    cout << "Client " << receiver.client_id << " receiving a message from client " << sender->client_id << ", but intended for client " << realReceiver->client_id << ". Forwarding... " << endl;
                                }
                                allMessage += applicationLayer->message_data;
                                frameNumber = 1;

                            } else {
                                allMessage += applicationLayer->message_data;
                                frameNumber++;
                            }


                            if (receiver.client_id == realReceiver->client_id){
                                cout << "Client " << realReceiver->client_id << " receiving frame #"<< frameNumber << " from client " << sender->client_id << ", originating from client " << applicationLayer->sender_ID << endl;
                                cout << "Sender MAC address: " << sender->client_mac << ", Receiver MAC address: " << realReceiver->client_mac << endl;
                                cout << "Sender IP address: " << realSender->client_ip << ", Receiver IP address: " << realReceiver->client_ip << endl;
                                cout << "Sender port number: " << sender_port << ", Receiver port number: " << receiver_port << endl;
                                cout << "Sender ID: " << realSender->client_id << ", Receiver ID: " << realReceiver->client_id << endl;
                                cout << "Message chunk carried: \"" << applicationLayer->message_data << "\"" << endl;
                                cout << "Number of hops so far: " << packetPhysical->number_of_hops << endl;
                                cout << "--------" << endl;
                                if (allMessage.find_first_of(".?!") != string::npos) {
                                    cout << "Client " << realReceiver->client_id << " received the message \"" << allMessage << "\" from client " << realSender->client_id << "." << endl;
                                    cout << "--------" << endl;

                                    // Update Log entries
                                    Log log_entry(getCurrentTimestamp(), allMessage, frameNumber,packetPhysical->number_of_hops,
                                                  realSender->client_id, realReceiver->client_id, true, ActivityType::MESSAGE_RECEIVED);
                                    realReceiver->log_entries.push_back(log_entry);
                                }
                                stack<Packet*> tempStack = frame;

                                // Print information for each layer in the frame
                                while (!tempStack.empty()) {


                                    delete tempStack.top();
                                    tempStack.pop();
                                }
                            } else {
                                cout << "Frame #" << frameNumber << " MAC address change: New sender MAC " << receiver.client_mac << ", new receiver MAC " << nextReceiver->client_mac << endl;
                                if (allMessage.find_first_of(".?!") != string::npos) {
                                    cout << "--------" << endl;
                                }
                            }
                        }
                        copyFrame.pop();
                    }

                    if (nextReceiver != nullptr && realReceiver->client_id != receiver.client_id) {
                        receiver.outgoing_queue.push(incoming_frame);
                    }
                    receiver.incoming_queue.pop();
                }
            }
        } else if (cmd == "PRINT_LOG") {
            // Process PRINT_LOG command
            string clientID;
            ss >> clientID;

            cout << "--------------------\n";
            cout << "Command: PRINT_LOG " << clientID << endl;
            cout << "--------------------\n";

            // Find the client based on the client ID
            Client *selectedClient = nullptr;
            for (Client &client : clients) {
                if (client.client_id == clientID) {
                    selectedClient = &client;
                    break;
                }
            }

            int logEntry = 1;

            if (selectedClient != nullptr && !selectedClient->log_entries.empty()) {
                cout << "Client " << selectedClient->client_id << " Logs:\n--------------\n";

                // Loop through the log entries and print details for each entry
                for (const Log &log : selectedClient->log_entries) {
                    cout << "Log Entry #" << logEntry << ":" << endl;
                    log.printLogDetails();
                    if (logEntry != selectedClient->log_entries.size()){
                        cout << "--------------\n";
                    }
                    logEntry++;
                }
            }


        } else {
            string tempmessage = "Command: " + command;
            cout << string(tempmessage.size(), '-') << endl;
            cout << tempmessage << endl;
            cout << string(tempmessage.size(), '-') << endl;
            cout << "Invalid command."<< endl;
        }
    }
}


vector<Client> Network::read_clients(const string &filename) {
    vector<Client> clients;
    ifstream input_file(filename);

    if (input_file.is_open()) {
        int num_clients;
        input_file >> num_clients;

        for (int i = 0; i < num_clients; ++i) {
            string id, ip, mac;
            input_file >> id >> ip >> mac;
            clients.emplace_back(Client(id, ip, mac));
        }

        input_file.close();
    } else {
        cerr << "Unable to open file: " << filename << endl;
    }

    return clients;
}

void Network::read_routing_tables(vector<Client> &clients, const string &filename) {
    ifstream input_file(filename);

    if (input_file.is_open()) {
        int client_index = 0;
        string line;

        while (getline(input_file, line)) {
            if (line == "-") {
                client_index++;
                continue;
            }

            stringstream ss(line);
            string dest_client_id, nearest_neighbor_id;
            ss >> dest_client_id >> nearest_neighbor_id;

            if (client_index < clients.size()) {
                clients[client_index].routing_table[dest_client_id] = nearest_neighbor_id;
            }
        }

        input_file.close();
    } else {
        cerr << "Unable to open file: " << filename << endl;
    }
}

vector<string> Network::read_commands(const string &filename) {
    vector<string> commands;
    ifstream input_file(filename);

    if (input_file.is_open()) {
        int num_commands;
        input_file >> num_commands;
        input_file.ignore(); // Ignore newline

        string line;
        while (getline(input_file, line)) {
            commands.push_back(line);
        }

        input_file.close();
    } else {
        cerr << "Unable to open file: " << filename << endl;
    }

    return commands;
}

void Network::prepare_frames(Client &sender, Client &receiver, const string &message, int message_limit, string sender_port,
                             string receiver_port) {
    // Split the message into chunks to fit into frames
    vector<string> message_chunks;
    int message_length = message.size();
    int start = 0;

    // Logic to split message into chunks of appropriate size
    while (start < message_length) {
        int chunk_size = min(message_limit, message_length - start);
        string chunk = message.substr(start, chunk_size);
        message_chunks.push_back(chunk);
        start += chunk_size;
    }

    // Create frames for each message chunk and queue them into sender's outgoing queue
    int frame_number = 0;
    // Determine the next receiver based on sender's routing table
    Client next_receiver = getNextReceiver(sender, receiver);
    for (const string &chunk : message_chunks) {
        ++frame_number;
        // Create ApplicationLayerPacket for the frame
        ApplicationLayerPacket *app_packet = new ApplicationLayerPacket(0, sender.client_id, receiver.client_id, chunk);
        // Create frames and prepare for transmission
        prepare_frame(sender, next_receiver, receiver, app_packet, frame_number, sender_port, receiver_port);
        app_packet->frame_number = frame_number;
    }

    printMessageOutput(sender.client_mac,next_receiver.client_mac,sender.client_ip,
                       receiver.client_ip,sender_port,receiver_port,
                       sender.client_id,receiver.client_id,message_chunks,receiver);

    Log log_entry(getCurrentTimestamp(), message, frame_number, 0 , sender.client_id,
                  receiver.client_id, true, ActivityType::MESSAGE_SENT);

    sender.log_entries.push_back(log_entry);

}

void Network::prepare_frame(Client &sender, Client &next_receiver, Client &receiver, ApplicationLayerPacket *app_packet, int frame_number,  string sender_port,
                            string receiver_port) {

    // Create PhysicalLayerPacket for the frame
    PhysicalLayerPacket *physical_packet = new PhysicalLayerPacket(3, sender.client_mac, receiver.client_mac);
    physical_packet->sender_MAC_address = sender.client_mac;
    physical_packet->receiver_MAC_address = next_receiver.client_mac;
    physical_packet->number_of_hops = 0;
    physical_packet->frame_number = frame_number;

    // Create NetworkLayerPacket for the frame
    NetworkLayerPacket *network_packet = new NetworkLayerPacket(2, sender.client_ip, receiver.client_ip);
    network_packet->sender_IP_address = sender.client_ip;
    network_packet->receiver_IP_address = receiver.client_ip;
    network_packet->frame_number = frame_number;

    // Create TransportLayerPacket for the frame
    TransportLayerPacket *transport_packet = new TransportLayerPacket(1, sender.client_id, receiver.client_id);
    transport_packet->sender_port_number = sender_port;
    transport_packet->receiver_port_number = receiver_port;
    transport_packet->frame_number = frame_number;

    // Create the Frame using these packets
    Packet *packets[] = {app_packet, transport_packet, network_packet, physical_packet};
    queue<stack<Packet *>> outgoing_queue;

    stack<Packet *> packet_stack;
    for (Packet *packet : packets) {
        packet_stack.push(packet);
    }
    outgoing_queue.push(packet_stack);

    while (!outgoing_queue.empty()) {
        sender.outgoing_queue.push(outgoing_queue.front());
        outgoing_queue.pop();
    }
}

Client Network::getNextReceiver(const Client &sender, const Client &receiver) const {
    string receiver_id = receiver.client_id;

    // Check if the receiver is directly connected to the sender
    if (sender.routing_table.find(receiver_id) != sender.routing_table.end()) {
        string next_hop = sender.routing_table.at(receiver_id);

        // Find the client corresponding to the next hop ID
        for (const Client &client : clientsV) {
            if (client.client_id == next_hop) {
                return client;
            }
        }
    }

    // If the receiver is not directly connected, return the receiver itself
    return receiver;
}

std::string Network::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

bool Network::checkReceiverInRoutingTable(const Client &sender, const Client &receiver) {
    auto it = sender.routing_table.find(receiver.client_id);
    return it != sender.routing_table.end(); // True if receiver is found in sender's routing table
}

void Network::printMessageOutput(const string& sender_mac, const string& receiver_mac,
                                 const string& sender_ip, const string& receiver_ip,
                                 const string& sender_port, const string& receiver_port,
                                 const string& sender_id, const string& receiver_id,
                                 const vector<string>& message_chunks,
                                 const Client &client) {

    int message_length = 0;
    for (const auto& chunk : message_chunks) {
        message_length += chunk.size();
    }
    message_length += (message_chunks.size() - 1); // Account for spaces between chunks

    string message_printed;

    // Print message header

    message_printed = "Command: MESSAGE " + sender_id + " " + client.client_id + " #";
    for (const auto& chunk : message_chunks) {
        message_printed += chunk;
        if (&chunk != &message_chunks.back()) {
        }
    }
    message_printed += "#";
    cout << string(message_printed.size(), '-') << endl;
    cout << message_printed << endl;
    cout << string(message_printed.size(), '-') << endl;

    // Print message content
    cout << "Message to be sent: \"";
    for (const auto& chunk : message_chunks) {
        cout << chunk;
    }
    cout << "\"\n\n";

    // Print frames
    int frame_number = 1;
    for (const auto& chunk : message_chunks) {
        cout << "Frame #" << frame_number << endl;
        cout << "Sender MAC address: " << sender_mac << ", Receiver MAC address: " << receiver_mac << endl;
        cout << "Sender IP address: " << sender_ip << ", Receiver IP address: " << receiver_ip << endl;
        cout << "Sender port number: " << sender_port << ", Receiver port number: " << receiver_port << endl;
        cout << "Sender ID: " << sender_id << ", Receiver ID: " << receiver_id << endl;
        cout << "Message chunk carried: \"" << chunk << "\"" << endl;
        cout << "Number of hops so far: 0\n";
        cout << "--------" << endl;
        ++frame_number;
    }
}

void Network::printFrameInfo(const Client &client, const string &queue_name, int frame_number) {
    queue<stack<Packet *>> queue_to_check;
    if (queue_name == "in") {
        queue_to_check = client.incoming_queue;
    } else if (queue_name == "out") {
        queue_to_check = client.outgoing_queue;
    } else {
        cerr << "Invalid queue name: " << queue_name << endl;
        return;
    }

    if (frame_number >= 0 && frame_number < queue_to_check.size()) {
        queue<stack<Packet*>> temp_queue = queue_to_check;

        string tempmessage = "Command: SHOW_FRAME_INFO " + client.client_id + " " + queue_name + " " + to_string(frame_number);

        cout << string(tempmessage.size(), '-') << endl;
        cout << tempmessage << endl;
        cout << string(tempmessage.size(), '-') << endl;

        if (queue_name == "out"){
            cout << "Current Frame #" << frame_number << " on the outgoing queue of client " << client.client_id << endl;
        } else {
            cout << "Current Frame #" << frame_number << " on the incoming queue of client " << client.client_id << endl;
        }



        int current_frame = 1;
        stack<Packet*> temp_stack;
        while (!temp_queue.empty()) {
            if (current_frame == frame_number) {
                stack<Packet*> current_stack = temp_queue.front();
                while (!current_stack.empty()) {
                    Packet* packet = current_stack.top();
                    temp_stack.push(packet);
                    current_stack.pop();
                }
                break;
            }
            temp_queue.pop();
            ++current_frame;
        }

        while (!temp_stack.empty()) {
            Packet* packet = temp_stack.top();
            printPacketInfo(packet);
            temp_stack.pop();
        }


    } else {
        string tempmessage = "Command: SHOW_FRAME_INFO" + client.client_id + " " + queue_name + " " + to_string(frame_number);

        cout << string(tempmessage.size(), '-') << endl;
        cout << tempmessage << endl;
        cout << string(tempmessage.size(), '-') << endl;

        cout << "No such frame.\n";
    }
}

void Network::printPacketInfo(Packet* packet) {
    if (PhysicalLayerPacket* physical_packet = dynamic_cast<PhysicalLayerPacket*>(packet)) {
        // Physical layer information
        cout << "Layer 3 info: Sender MAC address: " << physical_packet->sender_MAC_address
             << ", Receiver MAC address: " << physical_packet->receiver_MAC_address << endl;
        cout << "Number of hops so far: " << physical_packet->number_of_hops << endl;
    }
    if (NetworkLayerPacket* network_packet = dynamic_cast<NetworkLayerPacket*>(packet)) {
        // Network layer information
        cout << "Layer 2 info: Sender IP address: " << network_packet->sender_IP_address
             << ", Receiver IP address: " << network_packet->receiver_IP_address << endl;
    }
    if (TransportLayerPacket* transport_packet = dynamic_cast<TransportLayerPacket*>(packet)) {
        // Transport layer information
        cout << "Layer 1 info: Sender port number: " << transport_packet->sender_port_number
             << ", Receiver port number: " << transport_packet->receiver_port_number << endl;
    }
    if (ApplicationLayerPacket* app_packet = dynamic_cast<ApplicationLayerPacket*>(packet)) {
        cout << "Carried Message: \"" << app_packet->message_data << "\"" << endl;
        // Application layer information
        cout << "Layer 0 info: Sender ID: " << app_packet->sender_ID
             << ", Receiver ID: " << app_packet->receiver_ID << endl;
    }
}

void Network::printQueueInfo(const Client &client, const string &queue_selection) {
    if (queue_selection == "out") {

        string tempmessage = "Command: SHOW_Q_INFO " + client.client_id + " out";

        cout << string(tempmessage.size(), '-') << endl;
        cout << tempmessage << endl;
        cout << string(tempmessage.size(), '-') << endl;

        cout << "Client " << client.client_id << " Outgoing Queue Status\n";
        cout << "Current total number of frames: " << client.outgoing_queue.size() << endl;
    } else if (queue_selection == "in") {
        string tempmessage = "Command: SHOW_Q_INFO " + client.client_id + " in";

        cout << string(tempmessage.size(), '-') << endl;
        cout << tempmessage << endl;
        cout << string(tempmessage.size(), '-') << endl;

        cout << "Client " << client.client_id << " Incoming Queue Status\n";
        cout << "Current total number of frames: " << client.incoming_queue.size() << endl;
    } else {
        cout << "Invalid queue selection: " << queue_selection << endl;
    }
}











