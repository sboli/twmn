#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/foreach.hpp>
#include <exception>
#include <unistd.h>

#ifndef VERSION
#define VERSION "1.2"
#endif

const std::string   DEFAULT_HOST = "127.0.0.1";
const int           DEFAULT_PORT = 9797;

/*!
  * \brief Read the port from the config file
  * \return true if the port was found
  * \param port the port to use
  */
bool read_port(int& port)
{
    const char* xdgDir = getenv("XDG_CONFIG_HOME");
    if (!xdgDir)
        return false;
    std::string path = xdgDir ;
    path += "/twmn/twmn.conf";
    std::ifstream in(path.c_str());
    if (!in)
        return false;
    boost::property_tree::ptree tree;
    boost::property_tree::ini_parser::read_ini(in, tree);
    boost::optional<int> value =  tree.get_optional<int>("main.port");
    return value ? port = *value : false;
}

/*!
  * \brief Fill tree with command line options from vm
  * \return true if any command line options was found
  */
bool populate_tree(boost::program_options::variables_map& vm, boost::property_tree::ptree& tree)
{
    boost::program_options::notify(vm);
    boost::property_tree::ptree& root = tree.add("root", "");
    boost::property_tree::ptree empty_root = root;
    BOOST_FOREACH(boost::program_options::variables_map::value_type& i, vm) {
        boost::any value = i.second.value();
        try {
            root.add(i.first, boost::any_cast<std::string>(value));
        }
        catch (const boost::bad_any_cast& e) {
            root.add(i.first, boost::any_cast<int>(value));
        }
    }
    return empty_root != root; // tree not empty
}

/*!
  * \brief Sent tree to the host
  */
void send_tree(const boost::program_options::variables_map& vm, const boost::property_tree::ptree& tree)
{
    using namespace boost::asio;
    std::ostringstream oss;
    boost::property_tree::xml_parser::write_xml(oss, tree);
    io_context ios;
    ip::udp::socket s(ios, ip::udp::endpoint(ip::udp::v4(), 0));
    int port = vm.count("port") ? vm["port"].as<int>() : 0;
    if (!port)
        if (!read_port(port))
            port = DEFAULT_PORT;
    boost::optional<std::string> host = tree.get_optional<std::string>("content.host");
    if (!host)
        host = boost::optional<std::string>(DEFAULT_HOST);
    s.send_to(buffer(oss.str()), ip::udp::endpoint(ip::address(ip::make_address_v4(*host)), port));
    ios.run();
}

int main(int argc, char** argv)
{
    namespace po = boost::program_options;
    po::options_description desc(std::string("twmn client version ") + VERSION + "\n Options");
    desc.add_options()
            ("help,h", "Show this help")
            ("icon,i", po::value<std::string>(), "The icon to use, either a predefined symbol"
                                                  " in the config file or a file path.")
            ("title,t", po::value<std::string>(), "A title for the notification.")
            ("content,c", po::value<std::string>(), "A content for this notification")
            ("host,H", po::value<std::string>(), ("The host ip address (x.x.x.x). Default is " + DEFAULT_HOST).c_str())
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
            ("remote", po::value<std::string>(), "Remote control for the daemon. Can be either activate, next, previous or hide.")
            ("version", po::value<std::string>(), "Show the version number.")
    ;
    try {
        po::variables_map vm;
        boost::property_tree::ptree tree;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        if (vm.count("help")) {
         std::cout << desc << std::endl;
	    return 0;
	}
        const bool filled = populate_tree(vm, tree);
        if (!filled && argc > 1) {
            std::string full_line;
            std::for_each(argv+1, argv+argc, (full_line += boost::lambda::_1) += " ");
            full_line.erase(full_line.length()-1);
            tree.put("root.content", full_line);
        }
        else if (!filled && argc == 1)
            throw std::runtime_error("Empty command line");
        send_tree(vm, tree);
    }
    catch (const std::exception& e) {
        std::cout << desc << std::endl;
    }
    return 0;
}
