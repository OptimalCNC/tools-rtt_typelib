#include "TypelibMarshallerBase.hpp"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <typeinfo>

#include <rtt/types/TemplateTypeInfo.hpp>
#include <rtt/types/TypeInfoRepository.hpp>

#include <typelib/registry.hh>
#include <typelib/typemodel.hh>

namespace
{
    struct TypeWithoutTransport
    {
        int value;
    };

    struct TypeWithTypelibTransport
    {
        int value;
    };

    struct TypeWithWrongTransport
    {
        int value;
    };

    class WrongTransporter : public RTT::types::TypeTransporter
    {
    public:
        RTT::base::ChannelElementBase::shared_ptr createStream(
            RTT::base::PortInterface*, RTT::ConnPolicy const&, bool) const override
        {
            return RTT::base::ChannelElementBase::shared_ptr();
        }
    };

    class TestTypelibMarshaller : public orogen_transports::TypelibMarshallerBase
    {
    public:
        TestTypelibMarshaller(Typelib::Registry const& registry)
            : TypelibMarshallerBase(
                true,
                "/rtt_typelib_slash_fallback",
                "rtt_typelib_slash_fallback",
                registry)
        {
        }

        void setTypelibSample(Handle*, uint8_t*, bool = true) override {}
        void setOrocosSample(Handle*, void*, bool = true) override {}
        void refreshOrocosSample(Handle*) override {}
        void refreshTypelibSample(Handle*) override {}
        uint8_t* releaseOrocosSample(Handle*) override { return 0; }
        Handle* createSample() override { return createHandle(); }
        void deleteOrocosSample(Handle*) override {}
        void deleteTypelibSample(Handle*) override {}
        RTT::base::DataSourceBase::shared_ptr getDataSource(Handle*) override
        {
            return RTT::base::DataSourceBase::shared_ptr();
        }
        void writeDataSource(RTT::base::DataSourceBase&, Handle const*) override {}
        bool readDataSource(RTT::base::DataSourceBase&, Handle*) override
        {
            return false;
        }
    };

    bool expectRuntimeErrorContains(std::string const& label,
        std::string const& expected,
        void (*call)())
    {
        try
        {
            call();
        }
        catch (std::runtime_error const& e)
        {
            std::string const message = e.what();
            if (message.find(expected) != std::string::npos)
                return true;

            std::cerr << label << ": unexpected runtime_error message: "
                      << message << std::endl;
            return false;
        }
        catch (std::exception const& e)
        {
            std::cerr << label << ": unexpected exception type: "
                      << typeid(e).name() << ": " << e.what() << std::endl;
            return false;
        }

        std::cerr << label << ": expected runtime_error containing: "
                  << expected << std::endl;
        return false;
    }

    void lookupEmptyType()
    {
        orogen_transports::getMarshallerFor("");
    }

    void lookupMissingType()
    {
        orogen_transports::getMarshallerFor("rtt_typelib_missing_type");
    }

    void lookupTypeWithoutTransport()
    {
        RTT::types::TypeInfoRepository::shared_ptr types =
            RTT::types::TypeInfoRepository::Instance();
        types->addType(new RTT::types::TemplateTypeInfo<TypeWithoutTransport, false>(
            "rtt_typelib_no_transport"));

        orogen_transports::getMarshallerFor("rtt_typelib_no_transport");
    }

    void lookupTypeWithWrongTransport()
    {
        RTT::types::TypeInfoRepository::shared_ptr types =
            RTT::types::TypeInfoRepository::Instance();
        types->addType(new RTT::types::TemplateTypeInfo<TypeWithWrongTransport, false>(
            "rtt_typelib_wrong_transport"));

        RTT::types::TypeInfo* type_info =
            types->type("rtt_typelib_wrong_transport");
        type_info->addProtocol(
            orogen_transports::TYPELIB_MARSHALLER_ID,
            new WrongTransporter);

        orogen_transports::getMarshallerFor("rtt_typelib_wrong_transport");
    }

    bool lookupSlashFallbackUsesUnqualifiedTypeName()
    {
        RTT::types::TypeInfoRepository::shared_ptr types =
            RTT::types::TypeInfoRepository::Instance();
        types->addType(new RTT::types::TemplateTypeInfo<TypeWithTypelibTransport, false>(
            "rtt_typelib_slash_fallback"));

        Typelib::Registry registry;
        registry.add(new Typelib::Numeric(
            "/rtt_typelib_slash_fallback",
            sizeof(TypeWithTypelibTransport),
            Typelib::Numeric::SInt));

        RTT::types::TypeInfo* type_info =
            types->type("rtt_typelib_slash_fallback");
        auto* marshaller = new TestTypelibMarshaller(registry);
        type_info->addProtocol(
            orogen_transports::TYPELIB_MARSHALLER_ID,
            marshaller);

        orogen_transports::TypelibMarshallerBase* result =
            orogen_transports::getMarshallerFor("/rtt_typelib_slash_fallback");
        if (result == marshaller)
            return true;

        std::cerr << "slash fallback returned an unexpected marshaller"
                  << std::endl;
        return false;
    }
}

int main()
{
    bool ok = true;

    ok &= expectRuntimeErrorContains("empty type lookup",
        "not registered in the RTT type system", &lookupEmptyType);
    ok &= expectRuntimeErrorContains("missing type lookup",
        "type rtt_typelib_missing_type is not registered in the RTT type system",
        &lookupMissingType);
    ok &= expectRuntimeErrorContains("type without transport",
        "type rtt_typelib_no_transport is registered in the RTT type system, but does not have a typelib transport",
        &lookupTypeWithoutTransport);
    ok &= expectRuntimeErrorContains("wrong transport",
        "the transport object registered as typelib transport for type rtt_typelib_wrong_transport is not a TypelibMarshallerBase",
        &lookupTypeWithWrongTransport);
    ok &= lookupSlashFallbackUsesUnqualifiedTypeName();

    return ok ? 0 : 1;
}
