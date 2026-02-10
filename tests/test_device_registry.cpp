#define BOOST_TEST_MODULE DeviceRegistryTest
#include <boost/test/included/unit_test.hpp>
#include <boost/test/tools/output_test_stream.hpp>

#include <devices/deviceregistry.h>

class TestDevice: public Device {
public:
    TestDevice() : Device("TestDevice") {}

    std::vector<Malfunction> createMalfunctions() override { return std::vector<Malfunction>(); }
};

struct DeviceRegistryFixture  {
    DeviceRegistryFixture() {
        DeviceRegistry::registerType<TestDevice>("Device1");
        DeviceRegistry::registerType<TestDevice>("Device2");
    }
    ~DeviceRegistryFixture() = default;
};

BOOST_FIXTURE_TEST_SUITE(DeviceRegistrySuite, DeviceRegistryFixture)

BOOST_AUTO_TEST_CASE(test_registration_device) {
    BOOST_CHECK(DeviceRegistry::create("Device1") != nullptr);
    BOOST_CHECK(DeviceRegistry::create("Device3") == nullptr);
}

BOOST_AUTO_TEST_CASE(test_create_multiply_devices) {
    auto device1 = DeviceRegistry::create("Device1");
    auto device2 = DeviceRegistry::create("Device2");

    BOOST_REQUIRE(device1 != nullptr);
    BOOST_REQUIRE(device2 != nullptr);

    // boost::test_tools::output_test_stream output;
    // output << device1->getName() << std::endl;
    // // Проверить что вывелось
    // std::cout << "DEBUG: " << output.str();  // Вывести в реальный cout
    
    BOOST_CHECK_EQUAL(device1->getName(), "TestDevice");
    BOOST_CHECK_EQUAL(device2->getName(), "TestDevice");
    
    // Проверяем что это разные объекты
    BOOST_CHECK(device1.get() != device2.get());
}

BOOST_AUTO_TEST_SUITE_END()
