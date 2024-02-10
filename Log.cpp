//
// Created by alperen on 2.10.2023.
//

#include "Log.h"

Log::Log(const string &_timestamp, const string &_message, int _number_of_frames, int _number_of_hops, const string &_sender_id,
         const string &_receiver_id, bool _success, ActivityType _type) {
    timestamp = _timestamp;
    message_content = _message;
    number_of_frames = _number_of_frames;
    number_of_hops = _number_of_hops;
    sender_id = _sender_id;
    receiver_id = _receiver_id;
    success_status = _success;
    activity_type = _type;
}

Log::~Log() {
    // TODO: Free any dynamically allocated memory if necessary.
}

void Log::printLogDetails() const {
    int message = 0;
    cout << "Activity: ";
    switch (static_cast<int>(activity_type)) {
        case 0:
            cout << "Message Received" << endl;
            message++;
            break;
        case 1:
            cout << "Message Forwarded" << endl;
            break;
        case 2:
            cout << "Message Sent" << endl;
            message++;
            break;
        case 3:
            cout << "Message Dropped" << endl;
            break;
        default:
            break;
    }
    cout << "Timestamp: " << timestamp << endl;
    cout << "Number of frames: " << number_of_frames << endl;
    cout << "Number of hops: " << number_of_hops << endl;
    cout << "Sender ID: " << sender_id << endl;
    cout << "Receiver ID: " << receiver_id << endl;
    cout << "Success: " << (success_status ? "Yes" : "No") << endl;

    if (message == 1) {
        cout << "Message: \"" << message_content << "\"" << endl;
    }
}