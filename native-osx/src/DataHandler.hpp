#ifndef __DataHandler__
#define __DataHandler__

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <iostream>
#include "Constants.h"

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

using websocketpp::lib::thread;
using websocketpp::lib::mutex;
using websocketpp::lib::unique_lock;
using websocketpp::lib::condition_variable;

enum action_type {
    SUBSCRIBE,
    UNSUBSCRIBE,
    MESSAGE
};

struct action {
    action(action_type t, connection_hdl h) : type(t), hdl(h) {}
    action(action_type t, server::message_ptr m) : type(t), msg(m) {}
    
    action_type type;
    websocketpp::connection_hdl hdl;
    server::message_ptr msg;
};

class DataHandler {
public:
    DataHandler();
    ~DataHandler();
    
    static void toggleWebAppsHandler(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo);

    void run();
    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);
    void on_message(connection_hdl hdl, server::message_ptr msg);
    void process_messages();
    void send_message(std::string str);
    void notifyNumClients(int numClients);
    void toggleWebApps(int toggleWebAppsValue);
private:
    typedef std::set<connection_hdl> con_list;

    bool enableData;
    server m_server;
    con_list m_connections;
    std::queue<action> m_actions;
    
    mutex m_action_lock;
    mutex m_connection_lock;
    condition_variable m_action_cond;
};
#endif /* defined(__DataHandler__) */