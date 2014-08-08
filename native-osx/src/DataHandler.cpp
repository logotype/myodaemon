#include <stdio.h>
#include "DataHandler.hpp"

void DataHandler::toggleWebAppsHandler(CFNotificationCenterRef center, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo) {
    
    NSDictionary *userInfoDictionary = (__bridge NSDictionary*)userInfo;
    int toggleWebAppsValue = (int)[[userInfoDictionary objectForKey:@"toggleWebApps"] integerValue];
    (static_cast<DataHandler *>(observer))->toggleWebApps(toggleWebAppsValue);
}

DataHandler::DataHandler() {
    
    enableData = TRUE;

    // Listen for notifications from Settings
    CFNotificationCenterAddObserver(CFNotificationCenterGetLocalCenter(), this, DataHandler::toggleWebAppsHandler, (CFStringRef)kToggleWebApps, NULL, CFNotificationSuspensionBehaviorDeliverImmediately);
    
    // Initialize Asio Transport
    m_server.init_asio();
    
    // Disable logging for now
    m_server.clear_access_channels(websocketpp::log::alevel::all);
    
    // Register handler callbacks
    m_server.set_open_handler(bind(&DataHandler::on_open,this,::_1));
    m_server.set_close_handler(bind(&DataHandler::on_close,this,::_1));
    m_server.set_message_handler(bind(&DataHandler::on_message,this,::_1,::_2));
};

DataHandler::~DataHandler() {
    CFNotificationCenterRemoveEveryObserver(CFNotificationCenterGetLocalCenter(), this);
}

void DataHandler::run() {
    // listen on specified port
    //uint16_t port
    m_server.listen(6450);
    
    // Start the server accept loop
    m_server.start_accept();
    
    // Start the ASIO io_service run loop
    try {
        boost::this_thread::sleep(boost::posix_time::milliseconds(1));
        m_server.run();
    } catch (const std::exception & e) {
        std::cout << "WebSocket++ exception: " << e.what() << std::endl;
    } catch (websocketpp::lib::error_code e) {
        std::cout << "WebSocket++ error_code: " << e.message() << std::endl;
    } catch (...) {
        std::cout << "WebSocket++ other exception: " << std::endl;
    }
}

void DataHandler::on_open(connection_hdl hdl) {
    if (!enableData) {
        return;
    }
    unique_lock<mutex> lock(m_action_lock);
    std::cout << "on_open" << std::endl;

    // Calculate number of connections and notify
    int numClients = 1;
    con_list::iterator it;
    for (it = m_connections.begin(); it != m_connections.end(); ++it) {
        numClients++;
    }
    notifyNumClients(numClients);

    m_actions.push(action(SUBSCRIBE,hdl));
    lock.unlock();
    m_action_cond.notify_one();
}

void DataHandler::on_close(connection_hdl hdl) {
    unique_lock<mutex> lock(m_action_lock);
    std::cout << "on_close" << std::endl;

    // Calculate number of connections and notify
    int numClients = 0;
    con_list::iterator it;
    for (it = m_connections.begin(); it != m_connections.end(); ++it) {
        numClients++;
    }
    notifyNumClients(numClients - 1);

    m_actions.push(action(UNSUBSCRIBE,hdl));
    lock.unlock();
    m_action_cond.notify_one();
}

void DataHandler::on_message(connection_hdl hdl, server::message_ptr msg) {
    if (!enableData) {
        return;
    }
    // queue message up for sending by processing thread
    unique_lock<mutex> lock(m_action_lock);
    std::cout << "on_message" << std::endl;
    std::cout << msg->get_payload() << std::endl;
    
    NSError *errorInfo;
    NSString *jsonString = [NSString stringWithCString:msg->get_payload().c_str() encoding:[NSString defaultCStringEncoding]];
    NSDictionary *parsedJSON = [NSJSONSerialization JSONObjectWithData:[jsonString dataUsingEncoding:NSUTF8StringEncoding] options:kNilOptions error:&errorInfo];
    if (parsedJSON) {
        NSString *command = [parsedJSON objectForKey:@"command"];

        if([command isEqualTo:kCmdRequestDeviceInfo]) {
            CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(), (CFStringRef)kCmdRequestDeviceInfo, NULL, NULL, YES);
        }

        if([command isEqualTo:kCmdVibrate]) {

            NSArray *args = [parsedJSON objectForKey:@"args"];
            NSString *vibrateLengthString = [args objectAtIndex:0];
            
            CFDictionaryKeyCallBacks keyCallbacks = {0, NULL, NULL, CFCopyDescription, CFEqual, NULL};
            CFDictionaryValueCallBacks valueCallbacks  = {0, NULL, NULL, CFCopyDescription, CFEqual};
            CFMutableDictionaryRef dictionary = CFDictionaryCreateMutable(kCFAllocatorDefault, 1, &keyCallbacks, &valueCallbacks);
            CFDictionaryAddValue(dictionary, CFSTR("vibrateValue"), (CFStringRef)vibrateLengthString);
            CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(), (CFStringRef)kCmdVibrate, NULL, dictionary, TRUE);
        }
    }
    
    m_actions.push(action(MESSAGE,msg));
    lock.unlock();
    m_action_cond.notify_one();
}

void DataHandler::send_message(std::string str) {
    if (!enableData) {
        return;
    }
    unique_lock<mutex> lock(m_action_lock);
    //std::cout << "send_message" << std::endl;
    
    con_list::iterator it;
    for (it = m_connections.begin(); it != m_connections.end(); ++it) {
        m_server.send(*it, str, websocketpp::frame::opcode::TEXT);
    }
    lock.unlock();
    m_action_cond.notify_one();
}

void DataHandler::process_messages() {
    while(1) {
        unique_lock<mutex> lock(m_action_lock);
        
        while(m_actions.empty()) {
            m_action_cond.wait(lock);
        }
        
        action a = m_actions.front();
        m_actions.pop();
        
        lock.unlock();
        
        if (a.type == SUBSCRIBE) {
            unique_lock<mutex> lock(m_connection_lock);
            m_connections.insert(a.hdl);
        } else if (a.type == UNSUBSCRIBE) {
            unique_lock<mutex> lock(m_connection_lock);
            m_connections.erase(a.hdl);
        } else if (a.type == MESSAGE) {
            unique_lock<mutex> lock(m_connection_lock);
            
            con_list::iterator it;
            for (it = m_connections.begin(); it != m_connections.end(); ++it) {
                m_server.send(*it,a.msg);
            }
        } else {
            // undefined.
        }
    }
}

void DataHandler::notifyNumClients(int numClients) {
    CFStringRef numClientsString = CFStringCreateWithFormat(NULL, NULL, CFSTR("%d"), numClients);
    CFDictionaryKeyCallBacks keyCallbacks = {0, NULL, NULL, CFCopyDescription, CFEqual, NULL};
    CFDictionaryValueCallBacks valueCallbacks  = {0, NULL, NULL, CFCopyDescription, CFEqual};
    CFMutableDictionaryRef dictionary = CFDictionaryCreateMutable(kCFAllocatorDefault, 1, &keyCallbacks, &valueCallbacks);
    CFDictionaryAddValue(dictionary, CFSTR("numClients"), numClientsString);
    CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(), (CFStringRef)kMyoNumClients, NULL, dictionary, TRUE);
    CFRelease(numClientsString);
    CFRelease(dictionary);
}

void DataHandler::toggleWebApps(int toggleWebAppsValue) {
    std::cout << "DataHandler::toggleWebApps: " << toggleWebAppsValue << std::endl;
    switch (toggleWebAppsValue) {
        case 0:
            enableData = FALSE;
            break;
        case 1:
            enableData = TRUE;
            break;
        default:
            break;
    }
}