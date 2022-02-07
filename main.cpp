// Standard includes.
#include <iostream>
#include <vector>
#include <iostream>

// 3rd party includes.
#include <boost/asio.hpp>
// #include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

struct Packet {
    Packet(std::size_t numBytes) : mBuffer(numBytes) {}

    operator std::string() const {
        std::stringstream os;
        os << std::to_string(mBytesTransferred) << "B from " << mOriginatorEndpoint.address();
        return os.str();
    }

    std::vector<std::byte> mBuffer;
    std::size_t mBytesTransferred;
    boost::asio::ip::udp::endpoint mOriginatorEndpoint;
};

struct mDNSListener {

    std::string const address_mcast = "224.0.0.251"; // mDNS.
    static constexpr unsigned short const port_mcast = 5353;

    mDNSListener(std::string listenAddr) : mListenAddr(boost::asio::ip::address::from_string(listenAddr)),
                                           mListenEndpoint(boost::asio::ip::address::from_string(address_mcast), port_mcast),
                                           mInpSocket(mIO_context) {

        std::cout << "Opening new input socket" << std::endl;
        mInpSocket.open(mListenEndpoint.protocol()); // boost::asio::ip::udp::socket

        std::cout << "Enabling reuse_address on input socket" << std::endl;
        mInpSocket.set_option(boost::asio::ip::udp::socket::reuse_address(true));

        // std::cout << "Enabling reuse_address on input socket" << std::endl;
        // inpSocket.set_option(boost::asio::ip::multicast::hops(1), ec); // set hops

        mInpSocket.bind(mListenEndpoint);
        std::cout << "Binding to input socket (" << mListenEndpoint.address() << ":" << mListenEndpoint.port() << ")" << std::endl;

        std::cout << "Joining multicast group on input socket" << std::endl;
        mInpSocket.set_option(boost::asio::ip::multicast::join_group(boost::asio::ip::address::from_string(address_mcast).to_v4(),
                                                                     mListenAddr.to_v4()));
    }

    Packet receiveFrom() {
        Packet p(65535);
        p.mBytesTransferred = mInpSocket.receive_from(boost::asio::buffer(p.mBuffer), p.mOriginatorEndpoint);
        return p;
    }

    boost::asio::io_context mIO_context;
    boost::asio::ip::address mListenAddr;
    boost::asio::ip::udp::endpoint mListenEndpoint;
    boost::asio::ip::udp::socket mInpSocket;
};

struct mDNSSender {

    std::string const address_mcast = "224.0.0.251"; // mDNS.
    static constexpr unsigned short const port_mcast = 5353;

    mDNSSender(std::string sendFromAddr) : mSenderAddr(boost::asio::ip::address::from_string(sendFromAddr)),
                                           mSenderEndpoint(mSenderAddr, port_mcast),
                                           mOutSocket(mIO_context) {

        std::cout << "Opening output socket" << std::endl;
        mOutSocket.open(mSenderEndpoint.protocol()); // boost::asio::ip::udp::socket

        std::cout << "Setting hops on output socket" << std::endl;
        mOutSocket.set_option(boost::asio::ip::multicast::hops(1)); // set hops

        mOutSocket.bind(mSenderEndpoint);
        std::cout << "Binding to output socket (" << mOutSocket.local_endpoint().address() << ":" << mOutSocket.local_endpoint().port() << ")" << std::endl;
    }

    void sendTo(Packet const & p, boost::asio::ip::udp::endpoint ep) {
        mOutSocket.send_to(boost::asio::buffer(p.mBuffer, p.mBytesTransferred), ep);
    }

    // boost::asio::ip::udp::endpoint reflector_endpoint(reflector_addr, port_mcast);
    // boost::asio::ip::udp::socket   outSocket(io_context);

    // std::cout << "Opening output socket" << std::endl;
    // mOutSocket.open(reflector_endpoint.protocol(), ec); // boost::asio::ip::udp::socket

    // // std::cout << "Enabling reuse_address on output socket" << std::endl;
    // // outSocket.set_option(boost::asio::ip::udp::socket::reuse_address(true), ec);

    // mOutSocket.bind(reflector_endpoint);
    // // if (ec.failed()) {
    // //     std::cout << "Bind failed with message: " << ec.message() << std::endl;
    // //     return -1;
    // // }

    boost::asio::io_context mIO_context;
    boost::asio::ip::address mSenderAddr;
    boost::asio::ip::udp::endpoint mSenderEndpoint;
    boost::asio::ip::udp::socket mOutSocket;
};

void listenLoop(mDNSListener & listener) {
    std::cout << "Listening for datagrams..." << std::endl;
    for (;;) {
        auto const packet = listener.receiveFrom();

        if (packet.mOriginatorEndpoint.address().to_v4() != listener.mListenAddr.to_v4()) {
            std::cout << "Saw packet " << std::string(packet) << " (i.e. _not_ " << listener.mListenAddr.to_v4() << ")" << std::endl;
        } else {
            std::cout << "Saw packet originating from us" << std::endl;
        }
    }
}

void reflectLoop(mDNSListener & listener, mDNSSender & sender) {

    std::string const address_mcast = "224.0.0.251"; // mDNS.
    constexpr unsigned short const port_mcast = 5353;

    std::cout << "Listening for datagrams..." << std::endl;
    for (;;) {
        auto const packet = listener.receiveFrom();

        if (packet.mOriginatorEndpoint.address().to_v4() != listener.mListenAddr.to_v4()) {
            std::cout << "Saw packet " << std::string(packet) << " (i.e. _not_ " << listener.mListenAddr.to_v4() << ")" << std::endl;
            std::cout << "Will forward to " << address_mcast << ":" << port_mcast << " on " << sender.mSenderAddr.to_v4() << std::endl;
            sender.sendTo(packet, boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(address_mcast), port_mcast));
        } else {
            std::cout << "Saw packet originating from us" << std::endl;
        }
    }
}


int main(int argc, char** argv) {

    // bool reflector = false;
    if (argc < 2) {
        std::cout << "Args!" << std::endl;
        abort();
    } else if (argc == 2) {
        std::cout << "In listener mode." << std::endl;
        auto listener = mDNSListener(std::string(argv[1]));
        listenLoop(listener);

    } else {
        std::cout << "In reflector mode." << std::endl;
        auto listener = mDNSListener(std::string(argv[1]));
        auto sendener = mDNSSender(std::string(argv[1]));

        reflectLoop(listener, sendener);
    }

    // std::string address_listen = argv[1];

    // // std::string const address_mcast = "224.0.0.251"; // mDNS.
    // boost::system::error_code ec;
    // boost::asio::ip::address listener_addr  = boost::asio::ip::address::from_string(address_listen, ec);



    // std::string address_sender = argv[2];
    // boost::asio::ip::address reflector_addr = boost::asio::ip::address::from_string(address_sender, ec);

    // std::cout << "Will listen for mDNS packets on " << address_listen << " and rebroadcast on " << address_sender << std::endl;



    // boost::asio::ip::address mcast_addr = boost::asio::ip::address::from_string("224.0.0.251", ec);

    // boost::asio::io_context io_context;

    // /////
    // // Input socket.

    // constexpr unsigned short const port_mcast = 5353;
    // boost::asio::ip::udp::endpoint listen_endpoint(mcast_addr, port_mcast);
    // // boost::asio::ip::udp::endpoint listen_endpoint(listener_addr, port_mcast);
    // boost::asio::ip::udp::socket   inpSocket(io_context);

    // std::cout << "Opening new input socket" << std::endl;
    // inpSocket.open(listen_endpoint.protocol(), ec); // boost::asio::ip::udp::socket

    // std::cout << "Enabling reuse_address on input socket" << std::endl;
    // inpSocket.set_option(boost::asio::ip::udp::socket::reuse_address(true), ec);
    // if (ec) {
    //     std::cout << "set_option(reuse_address) failed with message: " << ec.message() << std::endl;
    //     return ec.value();
    // }

    // inpSocket.set_option(boost::asio::ip::multicast::hops(1), ec); // set hops
	// if (ec) {
    //     std::cout << "set_option(hops) failed with message: " << ec.message() << std::endl;
    //     return ec.value();
    // }

    // // std::cout << "Binding to input socket" << std::endl;
    // inpSocket.bind(listen_endpoint, ec);
    // if (ec.failed()) {
    //     std::cout << "Bind failed with message: " << ec.message() << std::endl;
    //     return -1;
    // }
    // std::cout << "Binding to input socket (" << listen_endpoint.address() << ":" << listen_endpoint.port()
    //                                         // << inpSocket.local_endpoint().address() << ":" << inpSocket.local_endpoint().port()
    //                                         //  << ", "
    //                                         //  << inpSocket.remote_endpoint().address() << ":" << inpSocket.remote_endpoint().port()
    //                                          << ")" << std::endl;

    // std::cout << "Joining multicast group on input socket" << std::endl;
    // inpSocket.set_option(boost::asio::ip::multicast::join_group(mcast_addr.to_v4(),
    //                                                             listener_addr.to_v4()),
    //                      ec);

    /////
    // Output socket.

    // boost::asio::ip::udp::endpoint reflector_endpoint(reflector_addr, port_mcast);
    // boost::asio::ip::udp::socket   outSocket(io_context);

    // std::cout << "Opening output socket" << std::endl;
    // outSocket.open(reflector_endpoint.protocol(), ec); // boost::asio::ip::udp::socket

    // // std::cout << "Enabling reuse_address on output socket" << std::endl;
    // // outSocket.set_option(boost::asio::ip::udp::socket::reuse_address(true), ec);

    // outSocket.bind(reflector_endpoint, ec);
    // if (ec.failed()) {
    //     std::cout << "Bind failed with message: " << ec.message() << std::endl;
    //     return -1;
    // }

    // std::cout << "Binding to output socket (" << outSocket.local_endpoint().address() << ":" << outSocket.local_endpoint().port() << ")" << std::endl;

    // std::cout << "Joining multicast group on output socket" << std::endl;
    // outSocket.set_option(boost::asio::ip::multicast::join_group(mcast_addr.to_v4(), listen_addr.to_v4()), ec);

    // std::cout << "Listening for datagrams..." << std::endl;
    // for (;;) {
    //     char buffer[65536];
    //     // boost::asio::ip::udp::endpoint sender;
    //     // std::size_t bytes_transferred = inpSocket.receive_from(boost::asio::buffer(buffer), sender);
    //     // socket.send_to(boost::asio::buffer(buffer, bytes_transferred), sender);
    //     // std::cout << "Packet received (" << std::to_string(bytes_transferred) << "B), from " << sender.address() << ": " << sender.port() << std::endl;

    //     // boost::asio::ip::udp::endpoint originator_endpoint;
    //     // std::size_t bytes_transferred = inpSocket.receive_from(boost::asio::buffer(buffer), originator_endpoint);
    //     // std::size_t bytes_transferred = listener.mInpSocket.receive_from(boost::asio::buffer(buffer), originator_endpoint);
    //     auto const packet = listener.receiveFrom();

    //     if (packet.mOriginatorEndpoint.address().to_v4() != listener.mListenAddr.to_v4()) {
    //     // if (originator_endpoint.address().to_v4() != listener_addr.to_v4()) {
    //         // std::cout << "Will forward " << std::to_string(bytes_transferred) << "B from " << originator_endpoint.address() << " to " << originator_endpoint.address() << ":" << listen_endpoint.port() << " on " << reflector_endpoint.address() << std::endl;
    //         std::cout << "Will forward " << std::to_string(bytes_transferred) << "B from " << packet.mOriginatorEndpoint.address() << " (i.e. _not_ " << listener.mListenAddr.to_v4() << ") to " << listen_endpoint.address() << ":" << listen_endpoint.port() << " on " << outSocket.local_endpoint().address() << std::endl;
    //     } else {
    //         std::cout << "Filtering out packet originating from us" << std::endl;
    //     }
    //     // std::cout << "Will send " << std::to_string(bytes_transferred) << "B to " << listen_endpoint.address() << ": " << listen_endpoint.port() << " on " << sender_endpoint.address() << std::endl;

    //     // boost::asio::ip::udp::endpoint sender(mcast_addr, port_mcast);
    //     if (reflector) {
    //         outSocket.send_to(boost::asio::buffer(buffer, bytes_transferred), listen_endpoint);
    //     }
    // }

    return 0;
}