

#ifndef CUULIGHT
#define CUULIGHT

#include <exception>
#include <list>
#include <algorithm>
#include <iostream>
#include <sstream>

class cuu_exception : public std::exception
{
public:
  cuu_exception(const std::string& message) 
    : message(message) {}

  virtual ~cuu_exception() throw () {}
  
  virtual const char *what() throw () {
    return message.c_str();
  }
private:
  std::string message;
};

inline std::string 
cuu_condition_text(bool value) 
{ 
  return value ? "true" : "false"; 
}

inline std::string
cuu_error_location(const char *fileName,
                   unsigned long lineNumber,
                   const char *testName)
{
  std::stringstream buffer;
  buffer << fileName << ":" << lineNumber << ": " << testName << " ";
  return buffer.str();
}

inline std::string
cuu_fail_error_message(const char *fileName,
                       unsigned long lineNumber,
                       const char *testName,
                       const char *message)
{
  std::stringstream buffer;
  buffer << cuu_error_location(fileName, lineNumber, testName)
    << "error: " << message;
  return buffer.str();
} 

template<typename T> std::string
cuu_equals_error_message(const char *fileName, 
                         unsigned long lineNumber,
                         const char *testName,
                         T expected,
                         T actual)
{
  std::stringstream buffer;
  buffer << cuu_error_location(fileName, lineNumber, testName) 
       << "expected: <" << expected << "> but was: <" << actual << ">";
  return buffer.str();
}

inline std::string
cuu_bool_error_message(const char *fileName, 
                       unsigned long lineNumber,
                       const char *testName,
                       const char *boolToCheckText,
                       bool boolToCheck)
{
  std::stringstream buffer;
  buffer << cuu_error_location(fileName, lineNumber, testName) 
    << "expected: " << boolToCheckText << " to be " 
    << cuu_condition_text(!boolToCheck);
  return buffer.str();
}


inline void 
cuu_assert_equal(const char *expected, 
                 const char *actual, 
                 const char *fileName, 
                 unsigned long lineNumber, 
                 const char *testName) 
{
  if (!strcmp(expected, actual)) 
    return;
  throw cuu_exception(cuu_equals_error_message(fileName, lineNumber, testName, expected, actual).c_str());
}

template<typename T> void 
cuu_assert_equal(T expected, 
                 T actual, 
                 const char *fileName, 
                 unsigned long lineNumber,
                 const char *testName)
{
  if (expected == actual) 
    return;
  throw cuu_exception(cuu_equals_error_message(fileName, lineNumber, testName, expected, actual).c_str());
}

inline void 
cuu_assert_bool(bool boolToCheck, 
                bool sense,
                const char *boolToCheckText, 
                const char *fileName,
                unsigned long lineNumber,
                const char *testName)
{
  if(boolToCheck == sense)
    return;
  throw cuu_exception(cuu_bool_error_message(fileName, lineNumber, testName, boolToCheckText, boolToCheck).c_str());
}

inline void 
cuu_fail(const char *message, const char *fileName, unsigned long lineNumber, const char *testName)
{
  throw cuu_exception(cuu_fail_error_message(fileName, lineNumber, testName, message).c_str());
}

#define ASSERT_TRUE(x)\
  cuu_assert_bool((x),(true), #x, __FILE__, __LINE__, testName)

#define ASSERT_FALSE(x)\
  cuu_assert_bool((x),(false), #x, __FILE__, __LINE__, testName)

#define ASSERT_NULL(x)\
  ASSERT_TRUE((x) == 0)

#define ASSERT_NOT_NULL(x)\
  ASSERT_TRUE((x) != 0)

#define ASSERT_EQUAL(x,y)\
  cuu_assert_equal((x), (y), __FILE__, __LINE__, testName)

#define FAIL(message)\
  cuu_fail((message), __FILE__, __LINE__, testName)


class TestResultCollector
{
public:
  static void addException(cuu_exception& e) { std::cout << e.what() << std::endl; }
};

class Runner
{
public:
  virtual void run() = 0;
  virtual ~Runner() {}
};

inline void run_runner(Runner * runner) { runner->run(); }

class TestRegistry
{
  std::list<Runner *> runners;

  static TestRegistry& instance() { 
    static TestRegistry it; 
    return it; 
  }

public:
  static void addRunner(Runner *runner) { 
    instance().runners.push_back(runner); 
  }

  static void runAll() { 
    std::for_each(
      instance().runners.begin(), 
      instance().runners.end(), 
      run_runner); 
  }
};

template<typename TESTCASECLASS> class TestRunner : public Runner
{
public:
  void run() { 
    try { 
      TESTCASECLASS instance; 
    } 
    catch (cuu_exception& e) { 
      reportError(e); 
    } 
    catch (std::exception& e) { 
      reportError(std::string("Caught exception: ") + e.what());
    } 
    catch(...) {
      reportError("Caught unknown exception");
    }
  } 
private:
  void reportError(const std::string& description) {
    std::stringstream message;
    message << description  << " in " << TESTCASECLASS::testName;
    cuu_exception cuu_e(message.str());
    reportError(cuu_e);    
  }

  void reportError(cuu_exception& e) {
    TestResultCollector::addException(e);
  }
};

template<typename TESTRUNNER> class TestRegistrar
{
public:
  TestRegistrar() { TestRegistry::addRunner(new TESTRUNNER);}
};

class test {};


#define TEST(suite, test)\
class test##suite : public suite {\
public:\
  test##suite();\
  static const char *testName;\
};\
const char *test##suite::testName = "[test <" #test "> in suite <" #suite ">]";\
TestRegistrar<TestRunner<test##suite> > test##suite##registration;\
test##suite::test##suite()

#ifdef MAIN
int main() { TestRegistry::runAll(); return 0; }
#endif

#endif
