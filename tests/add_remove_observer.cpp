#include <memory>
#define BOOST_TEST_MODULE AddRemoveObservers
#include <boost/test/included/unit_test.hpp>
#include <boost/test/tools/output_test_stream.hpp>

#include <observers/observer.h>
#include <observers/penguin.h>

struct ObserverTestFixture {
    ObserverTestFixture() {
        penguin1 = std::make_shared<Penguin>(false);
        penguin2 = std::make_shared<Penguin>(false);
    }

    ~ObserverTestFixture() = default;

    std::shared_ptr<Penguin> penguin1;
    std::shared_ptr<Penguin> penguin2;
    ObserverManager observers;
};

BOOST_FIXTURE_TEST_SUITE(TestObservers, ObserverTestFixture)

BOOST_AUTO_TEST_CASE(AddObservers) {
    observers.addObserver(penguin1);
    observers.addObserver(penguin2);

    BOOST_REQUIRE_EQUAL(2, observers.countObservers());
}

BOOST_AUTO_TEST_CASE(RemoveObserver) {
    observers.addObserver(penguin1);
    observers.addObserver(penguin2);

    observers.removeObserver(penguin1);
    BOOST_REQUIRE_EQUAL(1, observers.countObservers());
}

BOOST_AUTO_TEST_SUITE_END()