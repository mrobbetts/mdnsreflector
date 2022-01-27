// Standard includes.
#include <iostream>

// 3rd party includes.
#include <boost/asio.hpp>
// #include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

int main(int argc, char** argv) {

    if (argc < 2) {
        std::cout << "Args!" << std::endl;
        abort();
    }

    // std::string address_listen = "10.2.10.132";
    std::string address_listen = argv[1];
    std::string address_sender = argv[2];

    std::cout << "Will listen for mDNS packets on " << address_listen << " and rebroadcast on " << address_sender << std::endl;

    std::string const address_mcast = "224.0.0.251"; // mDNS.
    unsigned short const port_mcast = 5353;

    boost::system::error_code ec;
    boost::asio::ip::address listen_addr = boost::asio::ip::address::from_string(address_listen, ec);
    boost::asio::ip::address sender_addr = boost::asio::ip::address::from_string(address_sender, ec);

    boost::asio::ip::address mcast_addr = boost::asio::ip::address::from_string(address_mcast, ec);

    boost::asio::io_context io_context;

    /////
    // Input socket.

    boost::asio::ip::udp::endpoint listen_endpoint(mcast_addr, port_mcast);
    boost::asio::ip::udp::socket   inpSocket(io_context);

    std::cout << "Opening new input socket" << std::endl;
    inpSocket.open(listen_endpoint.protocol(), ec); // boost::asio::ip::udp::socket

    std::cout << "Enabling reuse_address on input socket" << std::endl;
    inpSocket.set_option(boost::asio::ip::udp::socket::reuse_address(true), ec);

    // std::cout << "Binding to input socket" << std::endl;
    inpSocket.bind(listen_endpoint, ec);
    if (ec.failed()) {
        std::cout << "Bind failed with message: " << ec.message() << std::endl;
        return -1;
    }
    std::cout << "Binding to input socket (" << listen_endpoint.address() << ":" << listen_endpoint.port()
                                            // << inpSocket.local_endpoint().address() << ":" << inpSocket.local_endpoint().port()
                                            //  << ", "
                                            //  << inpSocket.remote_endpoint().address() << ":" << inpSocket.remote_endpoint().port()
                                             << ")" << std::endl;

    std::cout << "Joining multicast group on input socket" << std::endl;
    inpSocket.set_option(boost::asio::ip::multicast::join_group(mcast_addr.to_v4(),
                                                                listen_addr.to_v4()),
                         ec);

    /////
    // Output socket.

    boost::asio::ip::udp::endpoint sender_endpoint(sender_addr, port_mcast);
    boost::asio::ip::udp::socket   outSocket(io_context);

    std::cout << "Opening output socket" << std::endl;
    outSocket.open(sender_endpoint.protocol(), ec); // boost::asio::ip::udp::socket

    // std::cout << "Enabling reuse_address on output socket" << std::endl;
    // outSocket.set_option(boost::asio::ip::udp::socket::reuse_address(true), ec);

    outSocket.bind(sender_endpoint, ec);
    if (ec.failed()) {
        std::cout << "Bind failed with message: " << ec.message() << std::endl;
        return -1;
    }

    std::cout << "Binding to output socket (" << outSocket.local_endpoint().address() << ":" << outSocket.local_endpoint().port() << ")" << std::endl;

    // std::cout << "Joining multicast group on output socket" << std::endl;
    // outSocket.set_option(boost::asio::ip::multicast::join_group(mcast_addr.to_v4(), listen_addr.to_v4()), ec);

    std::cout << "Listening for datagrams..." << std::endl;
    for (;;) {
        char buffer[65536];
        // boost::asio::ip::udp::endpoint sender;
        // std::size_t bytes_transferred = inpSocket.receive_from(boost::asio::buffer(buffer), sender);
        // socket.send_to(boost::asio::buffer(buffer, bytes_transferred), sender);
        // std::cout << "Packet received (" << std::to_string(bytes_transferred) << "B), from " << sender.address() << ": " << sender.port() << std::endl;

        boost::asio::ip::udp::endpoint sender_endpoint;
        std::size_t bytes_transferred = inpSocket.receive_from(boost::asio::buffer(buffer), sender_endpoint);

        if (sender_endpoint.address() != listen_addr) {
            std::cout << "Will send " << std::to_string(bytes_transferred) << "B to " << listen_endpoint.address() << ": " << listen_endpoint.port() << " on " << sender_endpoint.address() << std::endl;
        } else {
            std::cout << "Filtering out packet originating from us" << std::endl;
        }
        // std::cout << "Will send " << std::to_string(bytes_transferred) << "B to " << listen_endpoint.address() << ": " << listen_endpoint.port() << " on " << sender_endpoint.address() << std::endl;

        // boost::asio::ip::udp::endpoint sender(mcast_addr, port_mcast);
        outSocket.send_to(boost::asio::buffer(buffer, bytes_transferred), listen_endpoint);
    }

    return 0;
}