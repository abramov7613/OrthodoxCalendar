### OrthodoxCalendar (Церковный Православный календарь)

Это реализация православного церковного календаря на c++.

Подробноe описание интерфейса см. в файле oxc.h или через doxygen.

#### Требования

* компилятор стандарта c++20
* cmake version >= 3.16
при первом запуске cmake требуется интернет, т.к. CPM подгружает зависимости с github

#### Пример использования:
```
mkdir some_dir
cd some_dir
git clone https://github.com/abramov7613/OrthodoxCalendar.git
touch main.cpp
touch CMakeLists.txt
```

main.cpp
```c++
#include <iostream>
#include "oxc.h"

int main(int argc, char** argv)
{
	if(argc<2) {
		std::cout << "использование:\n" << argv[0] << " <число года>" << std::endl;
		return -1;
	}
	try
	{
		oxc::OrthodoxCalendar calendar;
		auto stable_days = calendar.get_alldates_with(argv[1], oxc::dvana10_nep_prazd);
		auto unstable_days = calendar.get_alldates_with(argv[1], oxc::dvana10_per_prazd);
		auto great_days = calendar.get_alldates_with(argv[1], oxc::vel_prazd);
		std::cout << argv[1] << " год\nДвунадесятые переходящие праздники:\n"
			<< calendar.get_description_for_dates(unstable_days.value())
			<< "\nДвунадесятые непереходящие праздники:\n"
			<< calendar.get_description_for_dates(stable_days.value())
			<< "\nВеликие праздники:\n"
			<< calendar.get_description_for_dates(great_days.value()) << std::endl;
	}
	catch(const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return -1;
	}
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

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=MinSizeRel
cmake --build build
```

