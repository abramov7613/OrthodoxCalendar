OrthodoxCalendar - это реализация православного церковного календаря в виде модуля для c++20.

Подробноe описание интерфейса см. в файле oxc.h

Пример использования:
```
#include "oxc.h"
#include <iostream>

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
