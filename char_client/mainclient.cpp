#include "client/client.hpp"

using namespace mychat;

int main() {
    Client client;
    client.plugin(); // connect

    client.poll();
    
    return 0;
}