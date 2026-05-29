#include "TypelibMarshallerBase.hpp"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <typeinfo>

int main()
{
    try
    {
        orogen_transports::getMarshallerFor("");
    }
    catch (std::runtime_error const& e)
    {
        std::string const message = e.what();
        if (message.find("not registered in the RTT type system") != std::string::npos)
            return 0;

        std::cerr << "unexpected runtime_error message: " << message << std::endl;
        return 1;
    }
    catch (std::exception const& e)
    {
        std::cerr << "unexpected exception type: " << typeid(e).name()
                  << ": " << e.what() << std::endl;
        return 1;
    }

    std::cerr << "expected getMarshallerFor to reject an empty type name"
              << std::endl;
    return 1;
}
