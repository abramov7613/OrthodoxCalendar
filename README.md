### OrthodoxCalendar (Церковный Православный календарь)

Это реализация православного церковного календаря на c++.

Подробноe описание интерфейса см. в файле oxc.h или через doxygen.

#### Требования

* компилятор стандарта c++20
* cmake version >= 3.16
при первом запуске cmake требуется доступ к https://github.com,
  т.к. CPM подгружает библиотеку boost::multiprecision в качестве зависимости

#### Пример использования:
```shell
mkdir some_dir
```
```shell
cd some_dir
```
```shell
git clone https://github.com/abramov7613/OrthodoxCalendar.git
```
```shell
touch main.cpp
```
```shell
touch CMakeLists.txt
```

main.cpp
```c++
#include <iostream>
#include <string>
#include <regex>
#include "oxc.h"

int main(int argc, char** argv)
{
  auto print_usage = [argv](){
    std::cout << "использование: " << argv[0] << " YEAR [-t {G,J,M}]\n\n"
                 "  YEAR             число года (значение > 2)\n"
                 "  -t {G,J,M}       тип календаря для числа года:\n"
                 "                       G - Григорианский (по умолчанию)\n"
                 "                       J - Юлианский\n"
                 "                       M - Ново-Юлианский\n" << std::endl;
  };
  std::string year;
  auto calendar_type = oxc::Grigorian;
  if(argc<2) {
    print_usage();
    return -1;
  } else {
    std::string cli_args;
    for(int i=1; i<argc; ++i) cli_args += std::string(argv[i]) + ' ';
    std::regex re_pattern("(\\d+)(\\s+-t(=| )?(J|G|M))?\\s*");
    std::smatch matches;
    if(std::regex_match(cli_args, matches, re_pattern)) {
      year = matches[1].str();
      std::string type = matches[4].str();
      if(type == "J") calendar_type = oxc::Julian;
      else if(type == "M") calendar_type = oxc::Milankovic;
    } else {
      print_usage();
      return -1;
    }
  }
  try
  {
    oxc::OrthodoxCalendar calendar;
    auto easter_date = calendar.pascha(year, calendar_type);
    auto stable_days = calendar.get_alldates_with(year, oxc::dvana10_nep_prazd, calendar_type);
    auto unstable_days = calendar.get_alldates_with(year, oxc::dvana10_per_prazd, calendar_type);
    auto great_days = calendar.get_alldates_with(year, oxc::vel_prazd, calendar_type);
    std::string datefmt;
    switch(calendar_type) {
      case oxc::Julian:
        datefmt = "%Jd %JM %JY г.";
        break;
      case oxc::Milankovic:
        datefmt = "%Md %MM %MY г.";
        break;
      default:
        datefmt = "%Gd %GM %GY г.";
    }
    if(!easter_date) {
      std::cout << "в указанном году православная пасха не празднуется!" ;
    } else {
      std::cout << calendar.get_description_for_date(easter_date, datefmt) ;
    }
    std::cout << "\n\n"
      << "Двунадесятые переходящие праздники:\n"
      << calendar.get_description_for_dates(unstable_days, datefmt) << "\n\n"
      << "Двунадесятые непереходящие праздники:\n"
      << calendar.get_description_for_dates(stable_days, datefmt) << "\n\n"
      << "Великие праздники:\n"
      << calendar.get_description_for_dates(great_days, datefmt) << std::endl;
  }
  catch(const std::exception& e)
  {
    std::cout << e.what() << std::endl;
    return -1;
  }
  return 0;
}
```

CMakeLists.txt
```
cmake_minimum_required(VERSION 3.16)
project(main)
add_subdirectory(OrthodoxCalendar)
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} oxc)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_include_directories(${PROJECT_NAME} PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/OrthodoxCalendar"
)
```

```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=MinSizeRel
```
```shell
cmake --build build
```

