/*
sudo apt-get install libmosquitto-dev
sudo apt-get install libmosquittopp-dev
sudo apt-get install mosquitto
mosquitto -c /etc/mosquitto/mosquitto.conf &
*/
#include <iostream>
#include <memory>
#include <functional>
#include <thread>
#include <string>
#include <string.h>
#include "mosquittopp.h"

typedef struct
{
    std::string id;
    std::string host;
    int port;
    std::string sub_topic;
    int keepalive{60};
    int qos{1};
}mqtt_info;

using mosqpp::mosquittopp;

class mqtt_device : public mosquittopp
{
public:
    /// \brief constructor
    /// \param info -- info used to initialize the mqtt service.
    mqtt_device(const mqtt_info &info, std::function<void(char *ptr, int size)> handler)
    :mosquittopp(info.id.c_str())
    {
        info_ = info;
        func_handler_ = handler;
        memset(buffer_, 0, sizeof(buffer_));
    }

    /// \brief destructor
    ~mqtt_device() {
    };

    /// \brief on_connect
    /// - This method is used do mqtt connect
    /// \param rc -- state for mqtt connect, 0 is success.
    void on_connect(int rc) {
        if (rc == 0)
        {
            /* Only attempt to subscribe on a successful connect. */
            subscribe(NULL, info_.sub_topic.c_str(), info_.qos);
            is_connet_ = true;
        }
    }

    /// \brief on_message
    /// - This method is used do mqtt message receive
    /// \param message -- memssage when receive from subscribe topic
    void on_message(const struct mosquitto_message *message) {
        if (!strcmp(message->topic, info_.sub_topic.c_str()))
        {
            /* Copy N-1 bytes to ensure always 0 terminated. */
            memcpy(buffer_, message->payload, message->payloadlen);
            buffer_[message->payloadlen] = '\0';
            if (func_handler_)
            {
                func_handler_(buffer_, message->payloadlen);
            }
        }
    }

    /// \brief on_subscribe
    /// - This method is used do subscribe success.
    /// \param mid --
    /// \param qos_count --
    /// \param granted_qos --
    void on_subscribe(int mid, int qos_count, const int *granted_qos) {
        std::cout << "Subscribe succeeded." << std::endl;
    }

    /// \brief publish_msg
    /// - This method is publish msg str.
    /// \param str -- memssage when receive by subscribe
    /// \return publish msg process.
    int publish_msg(const std::string &topic, int qos, const char* ptr, int size) {
        int ret = 0;

        if (is_connet_ && size > 0) {
            ret = publish(NULL, topic.c_str(), size, ptr, qos);
        }
        
        return ret;
    }

    /// \brief start
    /// - This method is used to start mqtt process.
    bool start() {
        thread_ = std::thread(std::bind(&mqtt_device::mqtt_run, this));
        thread_.detach();

        return true;
    }
private:
    /// \brief mqtt_run
    /// - mqtt loop run thread.
    void mqtt_run() {
        mosqpp::lib_init();

        std::cout << "mqtt run..." << std::endl;
        
        connect(info_.host.c_str(), info_.port, info_.keepalive);
    
        while (1)
        {
            loop_forever();
        }
    
        mosqpp::lib_cleanup();
    }

private:
    /// \brief info_
    /// - info used to store mqtt config.
    mqtt_info info_;

    /// \brief is_connet_
    /// - wheather mqtt success connect.
    bool is_connet_{false};

    /// \brief thread_
    /// - mqtt run thread object.
    std::thread thread_;

    /// \brief buffer_
    /// - buffer store subscription information.
    char buffer_[512];

    /// \brief buffer_
    /// - buffer store subscription information.
    std::function<void(char *ptr, int size)> func_handler_;
};

int main(int argc, char *argv[])
{
    mqtt_info mqtt_process_info = {
        id: "mqtt_client",
        host: "172.27.83.254",
        port:1883,
        sub_topic:"/info/mp_topic",
        keepalive:60,
        qos:1
    };

    std::shared_ptr<mqtt_device> mqtt_client = std::make_shared<mqtt_device>(mqtt_process_info, [](char *ptr, int size) {
        std::cout << "recv msg: " << ptr << std::endl;
    });

    mqtt_client->start();

    while(1)
    {
        mqtt_client->publish_msg("test", 1, "hello", 5);
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    return 0;
}