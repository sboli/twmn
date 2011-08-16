#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include <boost/asio.hpp>

int main(int argc, char** argv)
{
    std::string title, content, icon, host;
    int port;
    namespace po = boost::program_options;
    po::options_description desc("Options");
    desc.add_options()
	("help,h", "Show this help")
	("icon,i", po::value<std::string>(&icon)->default_value(std::string()), "The icon to use, either a predefined symbol"
										" in the config file or a file path.")
	("title,t", po::value<std::string>(&title)->default_value(std::string()), "A title for the notification.")
	("content,c", po::value<std::string>(&content), "A content for this notification")
	("host,H", po::value<std::string>(&host)->default_value("127.0.0.1"), "The host ip address (x.x.x.x). ")
	("port,p", po::value<int>(&port)->default_value(9797), "The port to use. ")
	;
    po::variables_map vm;
    try {
	po::store(po::parse_command_line(argc, argv, desc), vm);
    }
    catch (...) {
	std::cout << desc << std::endl;
	return 0;
    }
    using namespace boost::asio;
    po::notify(vm);
    io_service ios; 
    ip::udp::socket s(ios, ip::udp::endpoint(ip::udp::v4(), 0));
    try {
    s.send_to(buffer(vm["icon"].as<std::string>() + '|' + vm["title"].as<std::string>() + '|' + vm["content"].as<std::string>()),
		ip::udp::endpoint(ip::address(ip::address_v4::from_string(host)), port));
    }
    catch (...) {
	std::cout << desc << std::endl;
    }
    ios.run();
    return 0;
}
