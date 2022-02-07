// Standard includes.
#include <iostream>

// 3rd party includes.
#include <boost/asio.hpp>
// #include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

int main(int argc, char** argv) {

    bool reflector = false;
    if (argc < 2) {
        std::cout << "Args!" << std::endl;
        abort();
    } else if (argc == 2) {
        std::cout << "In listener mode." << std::endl;
    } else {
        std::cout << "In reflector mode." << std::endl;
        reflector = true;
    }

    // std::string address_listen = "10.2.10.132";
    std::string address_listen = argv[1];
    std::string address_sender = argv[2];

    std::cout << "Will listen for mDNS packets on " << address_listen << " and rebroadcast on " << address_sender << std::endl;

    // std::string const address_mcast = "224.0.0.251"; // mDNS.

    boost::system::error_code ec;
    boost::asio::ip::address listener_addr  = boost::asio::ip::address::from_string(address_listen, ec);
    boost::asio::ip::address reflector_addr = boost::asio::ip::address::from_string(address_sender, ec);

    boost::asio::ip::address mcast_addr = boost::asio::ip::address::from_string("224.0.0.251", ec);

    boost::asio::io_context io_context;

    /////
    // Input socket.

    constexpr unsigned short const port_mcast = 5353;
    boost::asio::ip::udp::endpoint listen_endpoint(mcast_addr, port_mcast);
    // boost::asio::ip::udp::endpoint listen_endpoint(listener_addr, port_mcast);
    boost::asio::ip::udp::socket   inpSocket(io_context);

    std::cout << "Opening new input socket" << std::endl;
    inpSocket.open(listen_endpoint.protocol(), ec); // boost::asio::ip::udp::socket

    std::cout << "Enabling reuse_address on input socket" << std::endl;
    inpSocket.set_option(boost::asio::ip::udp::socket::reuse_address(true), ec);
    if (ec) {
        std::cout << "set_option(reuse_address) failed with message: " << ec.message() << std::endl;
        return ec.value();
    }

    inpSocket.set_option(boost::asio::ip::multicast::hops(1), ec); // set hops
	if (ec) {
        std::cout << "set_option(hops) failed with message: " << ec.message() << std::endl;
        return ec.value();
    }

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
                                                                listener_addr.to_v4()),
                         ec);

    /////
    // Output socket.

    boost::asio::ip::udp::endpoint reflector_endpoint(reflector_addr, port_mcast);
    boost::asio::ip::udp::socket   outSocket(io_context);

    std::cout << "Opening output socket" << std::endl;
    outSocket.open(reflector_endpoint.protocol(), ec); // boost::asio::ip::udp::socket

    // std::cout << "Enabling reuse_address on output socket" << std::endl;
    // outSocket.set_option(boost::asio::ip::udp::socket::reuse_address(true), ec);

    outSocket.bind(reflector_endpoint, ec);
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

        boost::asio::ip::udp::endpoint originator_endpoint;
        std::size_t bytes_transferred = inpSocket.receive_from(boost::asio::buffer(buffer), originator_endpoint);

        if (originator_endpoint.address().to_v4() != listener_addr.to_v4()) {
            // std::cout << "Will forward " << std::to_string(bytes_transferred) << "B from " << originator_endpoint.address() << " to " << originator_endpoint.address() << ":" << listen_endpoint.port() << " on " << reflector_endpoint.address() << std::endl;
            std::cout << "Will forward " << std::to_string(bytes_transferred) << "B from " << originator_endpoint.address() << " (i.e. _not_ " << listener_addr.to_v4() << ") to " << listen_endpoint.address() << ":" << listen_endpoint.port() << " on " << outSocket.local_endpoint().address() << std::endl;
        } else {
            std::cout << "Filtering out packet originating from us" << std::endl;
        }
        // std::cout << "Will send " << std::to_string(bytes_transferred) << "B to " << listen_endpoint.address() << ": " << listen_endpoint.port() << " on " << sender_endpoint.address() << std::endl;

        // boost::asio::ip::udp::endpoint sender(mcast_addr, port_mcast);
        if (reflector) {
            outSocket.send_to(boost::asio::buffer(buffer, bytes_transferred), listen_endpoint);
        }
    }

    return 0;
}