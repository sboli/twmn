#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/tuple/tuple.hpp>
#include <exception>
#include <unistd.h>

std::string DEFAULT_HOST = "127.0.0.1";
const int   DEFAULT_PORT = 9797;

/*!
  * \brief Read the port from the config file
  * \return true if the port was found
  * \param port the port to use
  */
bool read_port(int& port)
{
    std::string path = getenv("XDG_CONFIG_HOME");
    path += "/twmn/twmn.conf";
    std::ifstream in(path.c_str());
    if (!in)
        return false;
    boost::property_tree::ptree tree;
    boost::property_tree::ini_parser::read_ini(in, tree);
    boost::optional<int> value =  tree.get_optional<int>("main.port");
    return value ? port = *value : false;
}

int main(int argc, char** argv)
{
    namespace po = boost::program_options;
    po::options_description desc("Options");
    desc.add_options()
            ("help,h", "Show this help")
            ("icon,i", po::value<std::string>(), "The icon to use, either a predefined symbol"
                                                  " in the config file or a file path.")
            ("title,t", po::value<std::string>(), "A title for the notification.")
            ("content,c", po::value<std::string>(), "A content for this notification")
            ("host,H", po::value<std::string>()->default_value(DEFAULT_HOST), ("The host ip address (x.x.x.x). Default is " + DEFAULT_HOST).c_str())
            ("port,p", po::value<int>(), "The port to use. Default setting is loaded from the config file.")
            ("layout,l", po::value<std::string>(), "The layout to use. The name of a configuration file.")
            ("size,s", po::value<int>(), "The height of this notification. If not specified the configuration file value is used.")
            ("pos", po::value<std::string>(), "Position of this notification. Either top_right, top_left, bottom_right or bottom_left.")
            ("fn", po::value<std::string>(), "The font to use for this notification.")
            ("fs", po::value<std::string>(), "The font size for this notification.")
            ("duration,d", po::value<int>(), "How long this notification remains visible")
            ("sc", po::value<std::string>(), "A command to play a specific sound for this notification")
            ("bg", po::value<std::string>(), "The background color")
            ("fg", po::value<std::string>(), "The foreground color, which is the font color.")
            ("id", po::value<int>(), "A message id. You should have given an id to the first message in order to modify it")
            ("aot", "Always on top. Specially on fullscreen applications, default.")
            ("ac", po::value<std::string>(), "A command to run when the notification is activated.")
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
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }
    try {
        std::ostringstream out;
        {
            boost::property_tree::ptree tree;
            boost::property_tree::ptree& root = tree.add("root", "");
            if (vm.count("content"))        root.add("content", vm["content"].as<std::string>());
            if (vm.count("icon"))           root.add("icon", vm["icon"].as<std::string>());
            if (vm.count("title"))          root.add("title", vm["title"].as<std::string>());
            if (vm.count("layout"))         root.add("layout", vm["layout"].as<std::string>());
            if (vm.count("size"))           root.add("size", vm["size"].as<int>());
            if (vm.count("pos"))            root.add("pos", vm["pos"].as<std::string>());
            if (vm.count("fn"))             root.add("fn", vm["fn"].as<std::string>());
            if (vm.count("fs"))             root.add("fs", vm["fs"].as<std::string>());
            if (vm.count("duration"))       root.add("duration", vm["duration"].as<int>());
            if (vm.count("sc"))             root.add("sc", vm["sc"].as<std::string>());
            if (vm.count("bg"))             root.add("bg", vm["bg"].as<std::string>());
            if (vm.count("fg"))             root.add("fg", vm["fg"].as<std::string>());
            if (vm.count("id"))             root.add("id", vm["id"].as<int>());
            if (vm.count("aot"))            root.add("aot", true);
            if (vm.count("ac"))             root.add("ac", vm["ac"].as<std::string>());
            boost::property_tree::xml_parser::write_xml(out, tree);
        }
        io_service ios;
        ip::udp::socket s(ios, ip::udp::endpoint(ip::udp::v4(), 0));
        int port = vm.count("port") ? vm["port"].as<int>() : DEFAULT_PORT;
        if (!vm.count("port"))
            read_port(port);
        s.send_to(buffer(out.str()), ip::udp::endpoint(ip::address(ip::address_v4::from_string(vm["host"].as<std::string>())), port));
        ios.run();
    }
    catch (std::exception& e) {
        std::cout << "here you are" << e.what() << std::endl;
    }
    return 0;
}
