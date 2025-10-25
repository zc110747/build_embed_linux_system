
#include <iostream>
#include "include/mosquittopp.h"

class mqtt_device : public mosqpp::mosquittopp
{};

int main(void)
{
    mqtt_device my_mosq();

    return 0;
}