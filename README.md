# mdnsreflector

A trivial mDNS reflector using Boost::Asio.

## Usage

`$ mdnsreflector listen-subnet reflect-from-ip`

The `listen-subnet` might be something like `10.2.10.0/24`, and the `reflect-from-ip` might be something
like `10.2.11.1`. This will listen on the 10.2.10 subnet and rebroadcast all mDNS packets from the
10.2.11.1 interface to its subnet.